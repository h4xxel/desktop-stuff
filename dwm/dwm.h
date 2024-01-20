#ifndef DWM_H
#define DWM_H

#include <time.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>
#include <pango/pango.h>
#include <pango/pangoxft.h>

/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#ifndef MAX
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#endif
#ifndef MIN
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#endif
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define TEXTW(X)                (textnw(X, strlen(X)) + dc.font.height)
#define STATUS_BUF_LEN          8192

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define _NET_SYSTEM_TRAY_ORIENTATION_HORZ 0

/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY      0
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_FOCUS_IN             4
#define XEMBED_MODALITY_ON         10

#define XEMBED_MAPPED              (1 << 0)
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_WINDOW_DEACTIVATE    2

#define VERSION_MAJOR               0
#define VERSION_MINOR               0
#define XEMBED_EMBEDDED_VERSION (VERSION_MAJOR << 16) | VERSION_MINOR

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast };        /* cursor */
enum { ColBorder, ColFG, ColBG, ColBorderFloat, ColLast };              /* color */
enum { NetSupported, NetSystemTray, NetSystemTrayOP, NetSystemTrayOrientation,
	   NetWMName, NetWMState, NetWMFullscreen, NetActiveWindow, NetWMWindowType,
	   NetWMWindowTypeDialog, NetLast }; /* EWMH atoms */
enum { Manager, Xembed, XembedInfo, XLast }; /* Xembed atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast };             /* clicks */

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client {
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	unsigned int tags;
	Bool isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen, sizehints;
	Client *next;
	Client *snext;
	Monitor *mon;
	Window win;
};

typedef struct {
	int x, y, w, h;
	Drawable drawable;
	unsigned long norm[ColLast];
	unsigned long sel[ColLast];
	struct {
		XftColor norm[ColLast];
		XftColor sel[ColLast];
		XftDraw *drawable;
	} xft;
	GC gc;
	struct {
		int ascent;
		int descent;
		int height;
		PangoLayout *layout;
	} font;
	Bool pad;
} DC; /* draw context */

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;

typedef struct Pertag Pertag;
struct Monitor {
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by;               /* bar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	Bool showbar;
	Bool topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Window barwin;
	const Layout *lt[2];
	Pertag *pertag;
};

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	Bool isfloating;
	Bool sizehints;
	int monitor;
} Rule;

typedef struct Systray   Systray;
struct Systray {
	Window win;
	Client *icons;
};

typedef struct Indicator {
	Bool active;
	char text[64];
	int x;
	int width;
	int (*init)(struct Indicator *indicator);
	void (*update)(struct Indicator *indicator);
	void (*expose)(struct Indicator *indicator, Window window);
	Bool (*haswindow)(struct Indicator *indicator, Window window);
	void (*mouse)(struct Indicator *indicator, XButtonPressedEvent *ev);
	void *data;
	struct Indicator *next;
} Indicator;

extern Display *dpy;
extern Cursor cursor[];
extern Window root;
extern int bh;
extern int screen;
extern DC dc;
extern Monitor *selmon;

int textnw(const char *text, unsigned int len);
int markupnw(const char *text, unsigned int len, Bool markup);
Bool utf8isfirstbyte(char c);

void viewtag(int tag);
void quit(const Arg *arg);

int indicator_time_init(Indicator *indicator);
void indicator_time_update(Indicator *indicator);
void indicator_time_expose(Indicator *indicator, Window window);
Bool indicator_time_haswindow(Indicator *indicator, Window window);
void indicator_time_mouse(Indicator *indicator, XButtonPressedEvent *ev);

int indicator_music_init(Indicator *indicator);
void indicator_music_update(Indicator *indicator);
void indicator_music_expose(Indicator *indicator, Window window);
Bool indicator_music_haswindow(Indicator *in, Window window);
void indicator_music_mouse(Indicator *indicator, XButtonPressedEvent *ev);

int indicator_disk_init(Indicator *indicator);
void indicator_disk_update(Indicator *indicator);
void indicator_disk_expose(Indicator *indicator, Window window);
Bool indicator_disk_haswindow(Indicator *indicator, Window window);
void indicator_disk_mouse(Indicator *indicator, XButtonPressedEvent *ev);

int indicator_power_init(Indicator *indicator);
void indicator_power_update(Indicator *indicator);
void indicator_power_expose(Indicator *indicator, Window window);
Bool indicator_power_haswindow(Indicator *indicator, Window window);
void indicator_power_mouse(Indicator *indicator, XButtonPressedEvent *ev);

int indicator_upgrade_init(Indicator *indicator);
void indicator_upgrade_update(Indicator *indicator);
void indicator_upgrade_expose(Indicator *indicator, Window window);
Bool indicator_upgrade_haswindow(Indicator *indicator, Window window);
void indicator_upgrade_mouse(Indicator *indicator, XButtonPressedEvent *ev);

int indicator_logout_init(Indicator *indicator);
void indicator_logout_update(Indicator *indicator);
void indicator_logout_expose(Indicator *indicator, Window window);
Bool indicator_logout_haswindow(Indicator *indicator, Window window);
void indicator_logout_mouse(Indicator *indicator, XButtonPressedEvent *ev);

void alsa_volume(const Arg *a);
void alsa_mute_toggle(const Arg *a);
void alsa_mute_mic_toggle(const Arg *a);

#endif
