#include "dwm.h"

int indicator_time_init(Indicator *indicator) {
	return 0;
}

void indicator_time_update(Indicator *indicator) {
	time_t rawtime;
	struct tm *timeinfo;
	char timebuf[16];

	time(&rawtime);
	timeinfo=localtime(&rawtime);

	strftime(timebuf, 16, "%H:%M:%S", timeinfo);
	sprintf(indicator->text, " ◷ %s ", timebuf);
	//indicator->width=textnw(indicator->text, strlen(indicator->text));
}

void indicator_time_expose(Indicator *indicator, Window window) {
}

void indicator_time_mouse(Indicator *indicator, unsigned int button) {
	if(button==Button1||button==Button3) {
		printf("clicked time indicator\n");
		indicator->active=!indicator->active;
	}
}