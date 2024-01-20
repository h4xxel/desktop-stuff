#include <sys/statvfs.h>
#include <mntent.h>

#include "dwm.h"
#include "indicator.h"

#define MENU_WIDTH 200

typedef struct MountList MountList;
struct MountList {
	char *dir;
	char usage[64];
	MountList *next;
};

static MountList *mnt = NULL;

static struct {
	Window window;
	GC gc;
	int x, y, w, h;
	int selected;
} menu={0};

static int get_mounts(MountList **mnt_list) {
	FILE *f;
	struct mntent *m;
	MountList *tmp;
	int i = 0;
	
	if(!mnt_list)
		return 0;
	
	f = setmntent(_PATH_MOUNTED, "r");
	
	while((m = getmntent(f))) {
		if(strstr(m->mnt_opts, "bind"))
			continue;
		if(m->mnt_fsname[0] != '/')
			continue;
		
		tmp = malloc(sizeof(MountList));
		tmp->dir = strdup(m->mnt_dir);
		tmp->next = *mnt_list;
		*mnt_list = tmp;
		mnt_list = &(*mnt_list)->next;
		i++;
	}
	
	endmntent(f);
	return i;
}

static void free_mounts(MountList **mnt_list) {
	MountList *tmp;
	
	if(!mnt_list)
		return;
	
	while(*mnt_list) {
		tmp = (*mnt_list)->next;
		free((*mnt_list)->dir);
		free(*mnt_list);
		*mnt_list = tmp;
	}
}

static void menu_open(Indicator *indicator) {
	int mounts;
	if((mounts = get_mounts(&mnt)) <= 0)
		return;
	
	menu.selected=-1;
	menu.x=selmon->mx+indicator->x-MENU_WIDTH+indicator->width;
	menu.y=selmon->my+bh;
	menu.w=MENU_WIDTH;
	menu.h=bh*mounts;
	menu.window=XCreateSimpleWindow(dpy, root, 
		menu.x, menu.y, menu.w, menu.h,
		1, dc.sel[ColBorder], dc.norm[ColBG]
	);
	menu.gc=XCreateGC(dpy, menu.window, 0, 0);
	XSelectInput(dpy, menu.window, ExposureMask|ButtonPressMask|PointerMotionMask);
	XDefineCursor(dpy, menu.window, cursor[CurNormal]);
	XMapRaised(dpy, menu.window);
	indicator->active = 1;
}

static void menu_close(Indicator *indicator) {
	free_mounts(&mnt);
	
	XFreeGC(dpy, menu.gc);
	XUnmapWindow(dpy, menu.window);
	XDestroyWindow(dpy, menu.window);
	indicator->active = 0;
}
int indicator_disk_init(Indicator *indicator) {
	return 0;
}

void indicator_disk_update(Indicator *indicator) {
	struct statvfs sbuf;
	unsigned long p;
	
	if(statvfs("/", &sbuf) < 0)
		return;
	
	p = 100*(sbuf.f_blocks-sbuf.f_bavail)/sbuf.f_blocks;
	
	if(p >= 95)
		sprintf(indicator->text, " <span foreground=\"red\">\uf0a0 %lu%%</span> ", p);
	else
		sprintf(indicator->text, " \uf0a0 %lu%% ", p);
}

void indicator_disk_expose(Indicator *indicator, Window window) {
	int i;
	unsigned long usage;
	struct statvfs sbuf;
	MountList *list;
	
	if(window!=menu.window)
		return;
	
	for(list = mnt, i = 0; list; list = list->next, i++) {
		if(statvfs(list->dir, &sbuf) >= 0) {
			usage = 100*(sbuf.f_blocks - sbuf.f_bavail)/sbuf.f_blocks;
			sprintf(list->usage, " %lu%% ", usage);
			
			if(usage >= 95)
				sprintf(list->usage, "<span foreground=\"red\">%lu%%</span>", usage);
			else
				sprintf(list->usage, "%lu%%", usage);
			
			indicator_draw_text(menu.window, menu.gc, 0, i*bh, MENU_WIDTH - 40, bh, dc.norm, list->dir, False);
			indicator_draw_text(menu.window, menu.gc, MENU_WIDTH - 40, i*bh, 40, bh, dc.norm, list->usage, True);
		} else {
			indicator_draw_text(menu.window, menu.gc, 0, i*bh, MENU_WIDTH - 40, bh, dc.norm, list->dir, False);
			indicator_draw_text(menu.window, menu.gc, MENU_WIDTH - 40, i*bh, 40, bh, dc.norm, "--", False);
		}
	}
}

Bool indicator_disk_haswindow(Indicator *indicator, Window window) {
	return menu.window==window?True:False;
}

void indicator_disk_mouse(Indicator *indicator, XButtonPressedEvent *ev) {
	if(ev->type != ButtonPress) {
		return;
	}
	if(ev->window==menu.window) {
		indicator_disk_expose(indicator, ev->window);
		return;
	}
	switch(ev->button) {
		case Button1:
		case Button3:
			if(indicator->active)
				menu_close(indicator);
			else
				menu_open(indicator);
			return;
		case Button4:
			break;
		case Button5:
			break;
	}
	if(indicator->active)
		indicator_disk_expose(indicator, menu.window);
}
