#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "dwm.h"
#include "indicator.h"

#define MENU_WIDTH 200


static struct {
	Window window;
	GC gc;
	int x, y, w, h;
	int selected;
} menu={0};


static struct {
	pthread_t thread;
	int upgrades;
	pthread_mutex_t lock;
} upg;

static int numupgrades() {
	int upgrades = 0;
	FILE *p;
	
	//if(!(p = popen("apt-get --dry-run upgrade | grep ^Inst | wc -l", "r")))
	if(!(p = popen("pacman -Qu | wc -l", "r")))
		return 0;
	
	fscanf(p, "%i\n", &upgrades);
	pclose(p);
	
	return upgrades;
}

static void *get_upgrades(void *tmp) {
	int upgrades;
	for(;;) {
		upgrades = numupgrades();
		pthread_mutex_lock(&upg.lock);
		upg.upgrades = upgrades;
		pthread_mutex_unlock(&upg.lock);
		sleep(60*2);
	}
	return NULL;
}

static void update_manager() {
	switch(fork()) {
		case 0:
			execlp("xterm", "xterm \"sh -c 'sudo pacman -Syu'\"", NULL);
			exit(1);
		
		default:
			viewtag(9);
			break;
		
		case -1:
			return;
	}
}

static void menu_open(Indicator *indicator) {
	menu.selected=-1;
	menu.x=selmon->mx+indicator->x-MENU_WIDTH+indicator->width;
	menu.y=bh;
	menu.w=MENU_WIDTH;
	menu.h=bh*1;
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
int indicator_upgrade_init(Indicator *indicator) {
	if(pthread_mutex_init(&upg.lock, NULL) != 0)
		return -1;
	if(pthread_create(&upg.thread, NULL, get_upgrades, NULL) != 0)
		return -1;
	return 0;
}

void indicator_upgrade_update(Indicator *indicator) {
	int upgrades;
	
	pthread_mutex_lock(&upg.lock);
	upgrades = upg.upgrades;
	pthread_mutex_unlock(&upg.lock);
	if(upgrades)
		sprintf(indicator->text, " <span foreground=\"red\">⇪ %i</span> ", upgrades);
	else 
		indicator->text[0] = 0;
}

void indicator_upgrade_expose(Indicator *indicator, Window window) {
	if(window!=menu.window)
		return;
	
	indicator_draw_text(menu.window, menu.gc, 0, 0, MENU_WIDTH, bh, menu.selected == 0 ? dc.sel : dc.norm, "Open update manager", False);
}

Bool indicator_upgrade_haswindow(Indicator *indicator, Window window) {
	return menu.window==window?True:False;
}

void indicator_upgrade_mouse(Indicator *indicator, XButtonPressedEvent *ev) {
	if(ev->type != ButtonPress) {
		XMotionEvent *motev = (void *) ev;
		int x = motev->x, y = motev->y;
		
		menu.selected=-1;
		if(x>=0&&y>=0&&x<menu.w&&y<menu.h) {
			menu.selected=y/bh;
		}
		indicator_upgrade_expose(indicator, menu.window);
		return;
	}
	if(ev->window==menu.window) {
		if(menu.selected == 0) {
			indicator->active = False;
			menu_close();
			update_manager();
		}
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
		indicator_upgrade_expose(indicator, menu.window);
}
