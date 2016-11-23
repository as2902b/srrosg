#include "ocf.h"
#include <app.h>

int initialized = 0;

typedef struct _item_data
{
	int index;
	Elm_Object_Item *item;
} item_data;

static char *icon_path_list[] = {
	ICON_DIR"/ocf.png",
	ICON_DIR"/tv0.png",
	ICON_DIR"/mute0.png",
	ICON_DIR"/vol.png",
	ICON_DIR"/picmode0.png",
	ICON_DIR"/audiomode0.png",
	NULL
};

static char *icon_name_list[] = {
	"OCF Demo",
	"TV Status",
	"TV Mute",
	"TV Volume",
	"Picture",
	"Sound",
	NULL
};

static void destroy_popup(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s *)data;
	if(!ad->popup) return;
	elm_popup_dismiss(ad->popup);
	if(ad->popup)evas_object_del(ad->popup);
	if(ad->image)evas_object_del(ad->image);
	ad->popup = NULL;
	ad->image = NULL;
}

void  send_message(soscondata *p)
{
	bundle *b = bundle_create();
	bundle_add_byte(b, "ocfmsg", p, sizeof(soscondata));
	message_port_send_message("org.openinterconnect.ocf", "s2port", b);
	//dlog_print(DLOG_INFO, LOG_TAG, "Message port send status = %d", ret);
}

void message_port_cb(int id, const char *remote_id, const char *remote_port, bool trusted_remote_port, bundle *message, void *data)
{
	appdata_s *ad = (appdata_s *)data;

	size_t size;
	void *ptr;

	bundle_get_byte(message, "ocfmsg", &ptr, &size);
	//dlog_print(DLOG_INFO, LOG_TAG, "message_port_cb size received = %d", size);
	soscondata *ss = (soscondata *)ptr;
	switch(ss->ocfstate){
		case INITIALIZED:{
			getAllValues();
			destroy_popup(ad, NULL, NULL);
		}
		break;
		case TVSTATUS:
			ad->r.tvonoff = ss->tvonoff;
			break;
		case MUTE:
			ad->r.mute = ss->mute;
			break;
		case VOLUME:
			ad->r.volume = ss->volume;
			break;
		case PICMODE:
			ad->r.picmode = ss->picmode;
			break;
		case AUDIOMODE:
			ad->r.audiomode = ss->audiomode;
			break;
	}
}


static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s *)data;
	dlog_print(DLOG_INFO, LOG_TAG,  "Top Item %p - %p\n", ad->nf_it, elm_naviframe_top_item_get(ad->nf));

	ad->r.ocfstate = INITIALIZED;
	destroy_popup(ad, NULL, NULL);

	if(ad->nf_it == elm_naviframe_top_item_get(ad->nf)){
		dlog_print(DLOG_INFO, LOG_TAG,  "Exiting The App");
		ui_app_exit();
	}
	else{
		elm_naviframe_item_pop_to(ad->nf_it);
		evas_object_del(ad->slider);
		evas_object_del(ad->slider_layout);
		evas_object_show(ad->rotary_selector);
		eext_rotary_object_event_activated_set(ad->rotary_selector, EINA_TRUE);
	}
}

void show_graphic_popup(appdata_s *ad, char *text, char *img, int timeout){
	ad->popup = elm_popup_add(ad->win);
	elm_object_style_set(ad->popup, "toast/circle");
	elm_popup_orient_set(ad->popup, ELM_POPUP_ORIENT_CENTER);
	evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ad->image = elm_image_add(ad->popup);
	elm_object_part_text_set(ad->popup,"elm.text", text);
	elm_image_file_set(ad->image, img, NULL);
	dlog_print(DLOG_INFO, LOG_TAG,  "*****ICON BUG %s", img);
	elm_object_part_content_set(ad->popup, "toast,icon", ad->image);

	if(timeout>0){
		elm_popup_timeout_set(ad->popup, timeout);
		evas_object_smart_callback_add(ad->popup, "timeout", destroy_popup, ad);
	}else{
		evas_object_smart_callback_del(ad->popup, "timeout", destroy_popup);
	}
	evas_object_show(ad->popup);
}



static void
_value_changed(void *data, Evas_Object *obj, void *event_info)
{
	char buffer[2000];
	appdata_s *ad = (appdata_s *)data;
	int curr = eext_circle_object_value_get(obj);

	/*
	if(strstr(ad->r.message, "PicMode")!=NULL)
		sprintf(buffer, ad->r.message, picmodes[curr]);
	else if(strstr(ad->r.message, "AudioMode")!=NULL)
		sprintf(buffer, ad->r.message, audiomodes[curr]);
		*/
	sprintf(buffer, "Volume = %d", curr);
	dlog_print(DLOG_INFO, LOG_TAG,  "%d Resetting Text to : %s", curr, buffer);
	elm_object_part_text_set(ad->slider_layout, "elm.text.slider", buffer);
	switch(ad->r.ocfstate){
		case VOLUME:
			ad->r.volume = curr;
			break;
		case PICMODE:
			ad->r.picmode = curr;
			break;
		case AUDIOMODE:
			ad->r.audiomode = curr;
			break;
	}
	putOCF(&ad->r);
}

void int_slider(appdata_s *ad, int min, int max, int curr){
	ad->slider_layout = elm_layout_add(ad->nf);
	elm_layout_file_set(ad->slider_layout, ELM_DEMO_EDJ, "slider_layout");

	char buffer[2024];


	/*if(strstr(ad->r.message, "PicMode")!=NULL)
		sprintf(buffer, ad->r.message, picmodes[curr]);
	else if(strstr(ad->r.message, "AudioMode")!=NULL){
		dlog_print(DLOG_INFO, LOG_TAG,  "Crash Message %s %d", ad->r.message, curr);
		sprintf(buffer, ad->r.message, audiomodes[curr]);
	}*/
		sprintf(buffer, "Volume = %d", curr);
	elm_object_part_text_set(ad->slider_layout, "elm.text.slider", buffer);
	evas_object_show(ad->slider_layout);

	ad->slider = eext_circle_object_slider_add(ad->slider_layout, ad->circle_surface);
	eext_circle_object_value_min_max_set(ad->slider, min, max);
	eext_circle_object_value_set(ad->slider, curr);
	elm_slider_unit_format_set(ad->slider, "%d");
	elm_slider_indicator_format_set(ad->slider, "%d");
	elm_slider_span_size_set(ad->slider, 120);
	eext_circle_object_slider_step_set(ad->slider, 1);
	eext_rotary_object_event_activated_set(ad->slider, EINA_TRUE);
	evas_object_smart_callback_add(ad->slider, "value,changed", _value_changed, ad);
	elm_naviframe_item_push(ad->nf, _("Slider"), NULL, NULL, ad->slider_layout, "empty");
}

void process_selection(int i, appdata_s *ad)
{
	char msg[1000];
	char icon[1000];
	int timeout = 5;
	sprintf(icon, "%s%s", ICON_DIR, icon_path_list[i]);

	if(i>0 && !initialized){
		show_graphic_popup(ad, "OCF Not Initialized", icon, 2);
		return;
	}
	switch(i)
	{
		case 0:
		{
			if(!initialized){
				sprintf(msg, "Initializing OCF");
				sprintf(icon, "%s/ocf.png", ICON_DIR);
				initOCFStack(ad);
				initialized = 1;
				show_graphic_popup(ad, msg, icon, 0);
			}
		}
		break;
		case 1:
		{
			ad->r.tvonoff = (ad->r.tvonoff)?0:1;
			ad->r.ocfstate = TVSTATUS;
			sprintf(icon, "%s/tv%d.png", ICON_DIR, ad->r.tvonoff);
			sprintf(msg, "Set %s To : %s", icon_name_list[i], (ad->r.tvonoff)?"Off":"On");
			show_graphic_popup(ad, msg, icon, timeout);
			putOCF(&ad->r);
		}
		break;
		case 2: //mute
		{
			ad->r.mute = (ad->r.mute==0)?1:0;
			ad->r.ocfstate = MUTE;
			sprintf(icon, "%s/mute%d.png", ICON_DIR, ad->r.mute);
			sprintf(msg, "TV Mute <br> : %s", (ad->r.mute)?"Off":"On");
			show_graphic_popup(ad, msg, icon, timeout);
			putOCF(&ad->r);
		}
		break;
		case 3:
		{
			ad->r.ocfstate = VOLUME;
			strcpy(ad->r.message, "Volume = %d");
			int_slider(ad, 0, 100, ad->r.volume);
		}
		break;
		case 4:
		{
			ad->r.ocfstate = PICMODE;
			strcpy(ad->r.message, "PicMode = %s");
			sprintf(icon, "%s/picmode%d.png", ICON_DIR, ad->r.picmode);
			show_graphic_popup(ad, msg, icon, 0);
			//FIXME : Trigger OCF API
		}
		break;
		case 5:
		{
			ad->r.ocfstate = AUDIOMODE;
			strcpy(ad->r.message, "AudioMode = %s");
			sprintf(icon, "%s/audiomode%d.png", ICON_DIR, ad->r.audiomode);
			show_graphic_popup(ad, msg, icon, 0);
			//FIXME : Trigger OCF API
		}
		break;

	}
}

static void
_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Eext_Object_Item *item;
	const char *main_text;
	const char *sub_text;

	/* Get current seleted item object */
	item = eext_rotary_selector_selected_item_get(obj);

	/* Get set text for the item */
	main_text = eext_rotary_selector_item_part_text_get(item, "selector,main_text");
	sub_text = eext_rotary_selector_item_part_text_get(item, "selector,sub_text");

	int len = sizeof(icon_name_list) / sizeof(icon_name_list[0]);
	int i = 0;
	for(i = 0; i<len; i++){
		if(!strcmp(icon_name_list[i], sub_text)){
			dlog_print(DLOG_INFO, LOG_TAG,  "Found Match %d %s, %s\n", i, main_text, sub_text);
			process_selection(i, (appdata_s *)data);
			return;
		}
	}
	return;
}

void
_item_create(Evas_Object *rotary_selector)
{
	Evas_Object *image;
	Eext_Object_Item * item;
	char buf[255] = {0};
	int i, j, k;

	for (i = 0, k = 0; i < ROTARY_SELECTOR_PAGE_COUNT; i++)
	{
		for (j = 0; j < ROTARY_SELECTOR_PAGE_ITEM_COUNT; j++)
		{
			if(k>=TOTAL_CONTROLS)
				break;

			item = eext_rotary_selector_item_append(rotary_selector);
			image = elm_image_add(rotary_selector);
			elm_image_file_set(image, icon_path_list[k], NULL);
			eext_rotary_selector_item_part_content_set(item,
															 "item,icon",
															 EEXT_ROTARY_SELECTOR_ITEM_STATE_NORMAL,
															 image);
			snprintf(buf, sizeof(buf), "SOSCON");
			eext_rotary_selector_item_part_text_set(item, "selector,main_text", buf);


			snprintf(buf, sizeof(buf), "%s", icon_name_list[k]);
			eext_rotary_selector_item_part_text_set(item, "selector,sub_text", buf);
			k++;
		}
	}
}

static void create_dial(appdata_s *ad){
	ad->circle_surface = eext_circle_surface_conformant_add(ad->conform);
	ad->layout = elm_layout_add(ad->conform);
	evas_object_size_hint_weight_set(ad->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_theme_set(ad->layout, "layout", "application", "default");
	evas_object_show(ad->layout);
	elm_object_content_set(ad->conform, ad->layout);
	ad->nf = elm_naviframe_add(ad->layout);
	elm_object_part_content_set(ad->layout, "elm.swallow.content", ad->nf);
	eext_object_event_callback_add(ad->nf, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ad->nf, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);
	ad->rotary_selector = eext_rotary_selector_add(ad->nf);
	eext_rotary_object_event_activated_set(ad->rotary_selector, EINA_TRUE);
	_item_create(ad->rotary_selector);
	//evas_object_smart_callback_add(rotary_selector, "item,selected", _item_selected_cb, ad);
	evas_object_smart_callback_add(ad->rotary_selector, "item,clicked", _item_clicked_cb, ad);
	ad->nf_it = elm_naviframe_item_push(ad->nf, _("Rotary Selector"), NULL, NULL, ad->rotary_selector, "empty");
}


static void
create_base_gui(appdata_s *ad)
{
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	create_dial(ad);
	evas_object_show(ad->win);

}

static bool
app_create(void *data)
{
	appdata_s *ad = (appdata_s *)data;
	create_base_gui(ad);
	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad;
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;


    int port = message_port_register_local_port("s2port", &message_port_cb, &ad);
    dlog_print(DLOG_INFO, LOG_TAG, "Local Message Port = %d", port);

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}
	return ret;
}
