#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "dwm.h"
#include "indicator.h"
#include "power.h"

#define MENU_WIDTH 200

static struct {
	Window window;
	GC gc;
	int x, y, w, h;
	int selected;
} menu={0};

typedef struct Icon Icon;
struct Icon {
	int percentage;
	const char *str;
};

static Icon icon[] = {
	{10, "\uf212"},
	{40, "\uf215"},
	{80, "\uf214"},
	{INT_MAX, "\uf213"},
};

static const char *state_text[] = {
	[POWER_BATTERY_STATE_UNKNOWN] = "Unknown",
	[POWER_BATTERY_STATE_CHARGING] = "Charging",
	[POWER_BATTERY_STATE_DISCHARGING] = "Discharging",
	[POWER_BATTERY_STATE_EMPTY] = "Empty",
	[POWER_BATTERY_STATE_FULL] = "Full",
	[POWER_BATTERY_STATE_PENDING_CHARGE] = "Pending Charge",
	[POWER_BATTERY_STATE_PENDING_DISCHARGE] = "Pending Discharge",
};

static PowerBattery *battery;

static void menu_open(Indicator *indicator) {
	menu.selected=-1;
	menu.x=selmon->mx+indicator->x-MENU_WIDTH+indicator->width;
	menu.y=bh;
	menu.w=MENU_WIDTH;
	menu.h=bh*4;
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

int indicator_power_init(Indicator *indicator) {
	//if(!(battery = power_battery_status()))
	//	return -1;
	
	return 0;
}

void indicator_power_update(Indicator *indicator) {
	int i, p;
	
	free(battery);
	battery = power_battery_status();
	
	if(!battery) {
		sprintf(indicator->text, " \uf211 - %% ");
		return;
	}
	
	p = lround(battery->percentage);
	
	switch(battery->state) {
		case POWER_BATTERY_STATE_CHARGING:
		case POWER_BATTERY_STATE_FULL:
		case POWER_BATTERY_STATE_UNKNOWN:
		case POWER_BATTERY_STATE_PENDING_CHARGE:
			sprintf(indicator->text, " \uf211 %i%% ", p);
			break;
	
		default:
			p = lround(battery->percentage);
			for(i = 0; i < sizeof(icon)/sizeof(Icon); i++) {
				if(p < icon[i].percentage)
					break;
			}
			if(p < icon[0].percentage)
				sprintf(indicator->text, " <span foreground=\"red\">%s %i%%</span> ", icon[0].str, p);
			else
				sprintf(indicator->text, " %s %i%% ", icon[i].str, p);
		
			break;
	}
}

void format_time(int64_t t, char *buf, const char *header, const char *footer) {
	int h, m, s;
	
	s = t % 60;
	t /= 60;
	m = t % 60;
	h = t / 60;
	
	sprintf(buf, "%s%i:%i%s", header, h, m, footer);
}

void indicator_power_expose(Indicator *indicator, Window window) {
	char text[256];
	int64_t t;
	
	if(window!=menu.window)
		return;
	
	if(!battery) {
		indicator_draw_text(menu.window, menu.gc, 0, 0, MENU_WIDTH, bh, dc.norm, "Unknown", False);
		return;
	}
	
	sprintf(text, "%s", state_text[battery->state]);
	indicator_draw_text(menu.window, menu.gc, 0, bh*0, MENU_WIDTH, bh, dc.norm, text, False);
	
	switch(battery->state) {
		case POWER_BATTERY_STATE_CHARGING:
		case POWER_BATTERY_STATE_FULL:
		case POWER_BATTERY_STATE_UNKNOWN:
		case POWER_BATTERY_STATE_PENDING_CHARGE:
			t = battery->time_to_full;
			if(!t)
				text[0] = 0;
			else
				format_time(t, text, "Time until charged: ", "");
			break;
	
		default:
			t = battery->time_to_empty;
			format_time(t, text, "Time until empty: ", "");
	}
	
	indicator_draw_text(menu.window, menu.gc, 0, bh*1, MENU_WIDTH, bh, dc.norm, text, False);
	
	sprintf(text, "Voltage: %.1f V", round(battery->voltage * 10.0)/10.0);
	indicator_draw_text(menu.window, menu.gc, 0, bh*2, MENU_WIDTH, bh, dc.norm, text, False);
	
	sprintf(text, "Battery health: %li%%", lround(battery->capacity));
	indicator_draw_text(menu.window, menu.gc, 0, bh*3, MENU_WIDTH, bh, dc.norm, text, False);
}

Bool indicator_power_haswindow(Indicator *indicator, Window window) {
	return menu.window==window?True:False;
}

void indicator_power_mouse(Indicator *indicator, XButtonPressedEvent *ev) {
	if(ev->type != ButtonPress) {
		return;
	}
	if(ev->window==menu.window) {
		indicator_power_expose(indicator, ev->window);
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
		indicator_power_expose(indicator, menu.window);
}
