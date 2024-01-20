#include <time.h>

#include "dwm.h"
#include "indicator.h"

#define MENU_WIDTH 256

static struct {
	Window window;
	GC gc;
	int x, y, w, h;
	int selected;
} menu={0};

static struct tm seltime;

static void menu_open(Indicator *indicator) {
	menu.selected=-1;
	menu.x=selmon->mx+indicator->x-MENU_WIDTH+indicator->width;
	menu.y=selmon->my+bh;
	menu.w=MENU_WIDTH;
	menu.h=bh*8;
	menu.window=XCreateSimpleWindow(dpy, root, 
		menu.x, menu.y, menu.w, menu.h,
		1, dc.sel[ColBorder], dc.norm[ColBG]
	);
	menu.gc=XCreateGC(dpy, menu.window, 0, 0);
	XSelectInput(dpy, menu.window, ExposureMask|ButtonPressMask|PointerMotionMask);
	XDefineCursor(dpy, menu.window, cursor[CurNormal]);
	XMapRaised(dpy, menu.window);
}

static void menu_close() {
	XFreeGC(dpy, menu.gc);
	XUnmapWindow(dpy, menu.window);
	XDestroyWindow(dpy, menu.window);
}

int indicator_time_init(Indicator *indicator) {
	time_t t;
	struct tm *timeinfo;
	time(&t);
	timeinfo=localtime(&t);
	memcpy(&seltime, timeinfo, sizeof(struct tm));
	return 0;
}

void indicator_time_update(Indicator *indicator) {
	time_t t;
	struct tm *timeinfo;
	char timebuf[16];
	
	time(&t);
	timeinfo=localtime(&t);
	
	strftime(timebuf, 16, "%H:%M:%S", timeinfo);
	sprintf(indicator->text, " \uf3d6 %s ", timebuf);
}

void indicator_time_expose(Indicator *indicator, Window window) {
	int day, wday;
	time_t t, selt;
	struct tm *timeinfo;
	char buf[32];
	
	if(window!=menu.window)
		return;
	
	time(&t);
	timeinfo=localtime(&t);
	selt=mktime(&seltime);
	gmtime_r(&selt, &seltime);
	if(--seltime.tm_wday<0)
		seltime.tm_wday=6;
	
	strftime(buf, sizeof(buf), "%a %d %b %Y", timeinfo);
	indicator_draw_text(menu.window, menu.gc, 0, 0, menu.w, bh, dc.norm, buf, False);
	
	for(day=seltime.tm_mday-1, wday=seltime.tm_wday-1; day>0; day--, wday--) {
		if(wday<0)
			wday=6;
		sprintf(buf, "%i", day);
		indicator_draw_text(menu.window, menu.gc, bh*seltime.tm_wday, bh, menu.w, bh, dc.norm, buf, False);
	}
}

Bool indicator_time_haswindow(Indicator *indicator, Window window) {
	return menu.window==window?True:False;
}

void indicator_time_mouse(Indicator *indicator, XButtonPressedEvent *ev) {
	if(ev->type != ButtonPress) {
		return;
	}
	if(ev->window==menu.window) {
		indicator_time_expose(indicator, ev->window);
		return;
	}
	switch(ev->button) {
		case Button1:
		case Button3:
			if((indicator->active=!indicator->active))
				menu_open(indicator);
			else
				menu_close();
			return;
		case Button4:
			break;
		case Button5:
			break;
	}
	if(indicator->active)
		indicator_time_expose(indicator, menu.window);
}
