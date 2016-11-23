#include "ocf.h"


typedef std::map<std::string, std::shared_ptr<OCResource>> DiscoveredResourceMap;
DiscoveredResourceMap discoveredResources;
ocfstate_e ocfstate = UNINITIALIZED;
soscondata r;

char gs2res[][100] = {
		{"/sec/tv/switch/binary"},
		{"/sec/tv/audio"},
		{"/sec/tv/mode/picture"},
		{"/sec/tv/mode/sound"},
};

int MAX_RESOURCES = sizeof(gs2res) / sizeof(gs2res[0]);

int resCount = 0;

static FILE* client_open(const char* /*path*/, const char *mode)
{
//	/return fopen("./oic_svr_db_client.json", mode);
	return NULL;
}


void foundResource(std::shared_ptr<OCResource> resource)
{
	std::string resourceURI;
	std::string hostAddress;
	try
	{
		resourceURI = resource->uri();
		if (discoveredResources.find(resourceURI) == discoveredResources.end())
		{
			for(int i = 0; i<MAX_RESOURCES; i++){
				if (resourceURI.compare(gs2res[i]) == 0){
					discoveredResources[resourceURI] = resource;
					if(i==3){
						resCount++;
						dlog_print(DLOG_INFO, LOG_TAG,  "Resource detected %s", resourceURI.c_str());
					}
				}
			}
		}
		if(resCount==1 && ocfstate == UNINITIALIZED){
			r.ocfstate = INITIALIZED;
			ocfstate = INITIALIZED;
			send_message(&r);
		}
	}
	catch(std::exception& e)
	{
		dlog_print(DLOG_INFO, LOG_TAG,  "Exception in foundResource");
	}
}

void initOCFStack(appdata_s *ad)
{
	resCount = 0;
	std::ostringstream requestURI;
	OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };
	PlatformConfig cfg {
		OC::ServiceType::InProc,
			OC::ModeType::Client,
			"0.0.0.0",
			0,
			OC::QualityOfService::LowQos,
			&ps
	};
	OCPlatform::Configure(cfg);
	try
	{
		std::cout.setf(std::ios::boolalpha);
		requestURI << OC_RSRVD_WELL_KNOWN_URI;
		OCPlatform::findResource("", requestURI.str(), CT_DEFAULT, &foundResource);
		dlog_print(DLOG_INFO, LOG_TAG,  "Finding Resource... ");
	}catch(OCException& e)
	{
		oclog() << "Exception in main: "<<e.what();
	}
}

void onPost(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
	dlog_print(DLOG_INFO, LOG_TAG,  "Post request was successful");
	//FIXME : Check result of Post
}

void putOCF(soscondata *s)
{
	string key;
	int value;
	int i = 0;
	OCRepresentation rep;

	switch(s->ocfstate)
	{
		case TVSTATUS:
			rep.setValue("value" ,(s->tvonoff)?false:true);
			i = 0;
			break;
		case VOLUME:
			rep.setValue("volume" ,s->volume);
			i = 1;
			break;
		case MUTE:
			rep.setValue("mute" ,(s->mute)?true:false);
			i = 1;
			break;
		case PICMODE:
			rep.setValue("modes" ,picmodes[s->picmode]);
			i = 2;
			break;
		case AUDIOMODE:
			rep.setValue("modes" ,audiomodes[s->audiomode]);
			i = 3;
			break;
	}

	std::shared_ptr<OCResource> resource = discoveredResources[gs2res[i]];
	if(resource){
		dlog_print(DLOG_INFO, LOG_TAG,  "POSTING %s", gs2res[i]);
		resource->post(rep, QueryParamsMap(), &onPost);
	}

}

void printSOSCONData(soscondata *data)
{
	//dlog_print(DLOG_INFO, LOG_TAG, "What Changed %d", data->ocfstate);
	switch(data->ocfstate)
	{
		case TVSTATUS:
			dlog_print(DLOG_INFO, LOG_TAG, "TV Status %d", data->tvonoff);
			break;
		case VOLUME:
		case MUTE:
			dlog_print(DLOG_INFO, LOG_TAG, "TV Volume %d Mute : %d", data->volume, data->mute);
			break;
		case PICMODE:
			dlog_print(DLOG_INFO, LOG_TAG, "Picture Mode %d", data->picmode);
			break;
		case AUDIOMODE:
			dlog_print(DLOG_INFO, LOG_TAG, "Audio Mode %d", data->audiomode);
			break;
	}
}

void onGet(const HeaderOptions&, const OCRepresentation& rep, const int eCode)
{
	if(eCode == OC_STACK_OK)
	{
		char resUri[1000];
		bool val;
		strcpy(resUri, rep.getUri().c_str());
		std::string temp;
    	int i = 0;

//        dlog_print(DLOG_INFO, LOG_TAG, "Attribs for %s", rep.getUri().c_str());
//        for(std::map<std::string, AttributeValue>::const_iterator it = rep.getValues().begin();
//				 it != rep.getValues().end(); ++it)
//		{
//			dlog_print(DLOG_INFO, LOG_TAG, "%s", it->first.c_str());
//		}
//        dlog_print(DLOG_INFO, LOG_TAG, "-----------------------------");

		for(i = 0; i<MAX_RESOURCES; i++)
		{
			if (rep.getUri().compare(gs2res[i]) == 0){
				switch(i)
				{
					case 0:
						rep.getValue("value" ,val);
						r.tvonoff = (val)?1:0;
						r.ocfstate = TVSTATUS;
						break;

					case 1:
						rep.getValue("mute" , val);
						rep.getValue("volume" ,r.volume);
						r.mute = (val)?1:0;
						r.ocfstate = VOLUME;
						break;
					
					case 2:{
						rep.getValue("modes" , temp);
						dlog_print(DLOG_INFO, LOG_TAG, "PicMode %s", temp.c_str());
						int j = sizeof(picmodes)/sizeof(picmodes[0]);
						for(int i = 0; i<j; i++){
							if(temp.compare(picmodes[i]) == 0){
								r.picmode = i;
								break;
							}
						}
						r.ocfstate = PICMODE;
					}
					break;
					case 3:{
						rep.getValue("modes" , temp);
						dlog_print(DLOG_INFO, LOG_TAG, "SoundMode %s", temp.c_str());

						int j = sizeof(audiomodes)/sizeof(audiomodes[0]);

						for(int i = 0; i<j; i++){
							if(temp.compare(audiomodes[i]) == 0){
								r.audiomode = i;
								break;
							}
						}
						r.ocfstate = AUDIOMODE;
					}
					break;
				}
				break;
			}
		}
		printSOSCONData(&r);
		send_message(&r);
	}
	else
	{
		dlog_print(DLOG_INFO, LOG_TAG,  "GET request was UNsuccessful");
	}
}

void getOCF(std::shared_ptr<OCResource> resource)
{
	QueryParamsMap test;
	if(resource)
		resource->get(test, &onGet);
}

void getAllValues()
{
	for(int i = 0; i<MAX_RESOURCES; i++){
		dlog_print(DLOG_INFO, LOG_TAG,  "Getting Resource  %s", gs2res[i]);
		std::shared_ptr<OCResource> resource = discoveredResources[gs2res[i]];
		getOCF(resource);
	}
}
