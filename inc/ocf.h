#ifndef _OCF_H_
#define _OCF_H_

#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <message_port.h>
#include <bundle.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string>
#include <map>
#include <cstdlib>
#include <functional>
#include <Elementary.h>
#include <mutex>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;
namespace PH = std::placeholders;

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "ocf"

#if !defined(PACKAGE)
#define PACKAGE "org.openinterconnect.ocf"
#endif

#define ICON_DIR "/opt/usr/apps/org.openinterconnect.ocf/res/images"
#define ELM_DEMO_EDJ "/opt/usr/apps/org.openinterconnect.ocf/res/ui_controls.edj"
#define ROTARY_SELECTOR_PAGE_COUNT 1
#define ROTARY_SELECTOR_PAGE_ITEM_COUNT 7
#define TOTAL_CONTROLS 11


const char picmodes[][25] = {
		"modeStandard",
		"modeMovie",
		"modeDynamic"
};
const char audiomodes[][25] = {
		"modeStandard",
		"modeMovie",
		"modeMusic",
};



typedef enum {
    UNINITIALIZED,
    INITIALIZED,
    TVSTATUS,
    MUTE,
    VOLUME,
    PICMODE,
    AUDIOMODE,
} ocfstate_e;


struct soscondata {
	ocfstate_e ocfstate;

	int tvonoff;
	int mute;
	int volume;

	int picmode;
	int audiomode;

	char message[1024];

};

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *layout;
	Evas_Object *nf;
	Eext_Circle_Surface *circle_surface;

	Elm_Object_Item *nf_it;
	Evas_Object *event_label;
	Evas_Object *start;
	Evas_Object *stop;
	Evas_Object *progressbar;
	Evas_Object *popup;
	Evas_Object *image;

	Evas_Object *rotary_selector;

	Evas_Object *slider_layout;
	Evas_Object *slider;
	Evas_Object *scroller;
	Evas_Object *circle_scroller;

	soscondata r;

	Evas_Object *hrm;
	Evas_Object *hrm_value;


} appdata_s;

struct sockdata{
	int state;
	int param;
};




void initOCFStack(appdata_s *);
void putOCF(soscondata *);
void getOCF2(soscondata *);
void getAllValues();


void send_message(soscondata *p);

#endif
