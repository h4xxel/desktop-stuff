#include <alsa/asoundlib.h>
#include "dwm.h"
#include "dbus.h"
#include "indicator.h"

#define MENU_WIDTH 256
#define TRACK_ELEMENT_LENGTH 128
#define BUTTON_W 40

enum PLAYBACK_STATUS {
	PLAYBACK_STATUS_PLAYING,
	PLAYBACK_STATUS_PAUSED,
	PLAYBACK_STATUS_STOPPED,
};

enum MEDIAPLAYER_ACTION {
	MEDIAPLAYER_ACTION_PREVIOUS,
	MEDIAPLAYER_ACTION_PLAYPAUSE,
	MEDIAPLAYER_ACTION_NEXT,
	
	MEDIAPLAYER_ACTION_PLAY,
	MEDIAPLAYER_ACTION_PAUSE,
	MEDIAPLAYER_ACTIONS,
};

typedef struct Icon Icon;
struct Icon {
	int percentage;
	const char *str;
};

static Icon icon[] = {
	{33, "\uf358"},
	{66, "\uf359"},
	{101, "\uf357"},
};

struct TRACK {
	char artist[TRACK_ELEMENT_LENGTH];
	char album[TRACK_ELEMENT_LENGTH];
	char title[TRACK_ELEMENT_LENGTH];
};

static struct {
	const char *interface;
	const char *interface_player;
	const char *base;
	const char *object;
} mpris={
	"org.mpris.MediaPlayer2",
	"org.mpris.MediaPlayer2.Player",
	"org.mpris.MediaPlayer2.",
	"/org/mpris/MediaPlayer2",
};

static struct {
	snd_mixer_t *handle;
	snd_mixer_elem_t* elem_play;
	snd_mixer_elem_t* elem_cap;
	const char* card;
	const char* mix_name_play;
	const char* mix_name_cap;
	int mix_index_play;
	int mix_index_cap;
	long minv;
	long maxv;
} alsa={
	NULL,
	NULL,
	NULL,
	"default",
	"Master",
	"Capture",
	0,
	0,
};

static struct {
	Window window;
	GC gc;
	int x, y, w, h;
	int selected;
	int button;
} menu={0};

static struct MEDIAPLAYER {
	const char *id;
	const char *name;
	struct MEDIAPLAYER *next;
} *mediaplayer=NULL;

static void mediaplayer_register(const char *id) {
	DBusMessage* msg;
	DBusMessageIter args, element;
	DBusPendingCall* pending;
	char *name=NULL;
	static const char *prop="Identity";
	struct MEDIAPLAYER **mp;
	char player[256];
	
	for(mp=&mediaplayer; *mp; mp=&((*mp)->next)) {
		if(!strcmp(id, (*mp)->id))
			return;
	}
	sprintf(player, "%s.%s", mpris.interface, id);
	if(!(msg=dbus_message_new_method_call(player, mpris.object, dbus.interface_prop, "Get")))
		goto do_register;
	dbus_message_iter_init_append(msg, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &mpris.interface)) {
		dbus_message_unref(msg);
		goto do_register;
	}
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop)) {
		dbus_message_unref(msg);
		goto do_register;
	}
	if(!dbus_connection_send_with_reply(dbus.session.connection, msg, &pending, -1)) {
		dbus_message_unref(msg);
		goto do_register;
	}
	if(!pending) {
		dbus_message_unref(msg);
		goto do_register;
	}
	dbus_connection_flush(dbus.session.connection);
	dbus_message_unref(msg);
	dbus_pending_call_block(pending);
	if (!(msg=dbus_pending_call_steal_reply(pending))) {
		dbus_pending_call_unref(pending);
		goto do_register;
	}
	dbus_pending_call_unref(pending);
	if(!dbus_message_iter_init(msg, &args)) {
		dbus_message_unref(msg);
		goto do_register;
	} else if(dbus_message_iter_get_arg_type(&args)!=DBUS_TYPE_VARIANT) {
		dbus_message_unref(msg);
		goto do_register;
	}
	
	dbus_message_iter_recurse(&args, &element);
	dbus_message_iter_get_basic(&element, &name);
	dbus_message_unref(msg);
	
	do_register:
	*mp=malloc(sizeof(struct MEDIAPLAYER));
	(*mp)->id=malloc(strlen(id)+1);
	if(name) {
		(*mp)->name=malloc(strlen(name)+1);
		strcpy((void *) (*mp)->name, name);
	} else
		(*mp)->name=NULL;
	(*mp)->next=NULL;
	strcpy((void *) (*mp)->id, id);
}

static void mediaplayer_deregister(const char *id) {
	struct MEDIAPLAYER **mp, *mp_next;
	for(mp=&mediaplayer; *mp; mp=&((*mp)->next)) {
		if(!strcmp(id, (*mp)->id)) {
			mp_next=(*mp)->next;
			free((void *) (*mp)->id);
			free((void *) (*mp)->name);
			free(*mp);
			*mp=mp_next;
			return;
		}
	}
}

static void mediaplayer_deregister_all() {
	struct MEDIAPLAYER *mp_next;
	while(mediaplayer) {
		mp_next=mediaplayer->next;
		free((void *) mediaplayer->id);
		free((void *) mediaplayer->name);
		free(mediaplayer);
		mediaplayer=mp_next;
	}
}

static struct TRACK mediaplayer_get_track(const char *id) {
	int current_type;
	DBusMessage* msg;
	DBusMessageIter args, array, dict, element, var, artist;
	DBusPendingCall* pending;
	static const char *prop="Metadata";
	char player[256];
	struct TRACK track={
		"Unknown",
		"Unknown",
		"Unknown",
	};
	
	sprintf(player, "%s%s", mpris.base, id);
	if(!(msg=dbus_message_new_method_call(player, mpris.object, dbus.interface_prop, "Get")))
		return track;
	dbus_message_iter_init_append(msg, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &mpris.interface_player)) {
		dbus_message_unref(msg);
		return track;
	}
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop)) {
		dbus_message_unref(msg);
		return track;
	}
	if(!dbus_connection_send_with_reply(dbus.session.connection, msg, &pending, -1)) {
		dbus_message_unref(msg);
		return track;
	}
	if(!pending) {
		dbus_message_unref(msg);
		return track;
	}
	dbus_connection_flush(dbus.session.connection);
	dbus_message_unref(msg);
	dbus_pending_call_block(pending);
	if(!(msg=dbus_pending_call_steal_reply(pending))) {
		dbus_pending_call_unref(pending);
		return track;
	}
	dbus_pending_call_unref(pending);
	if(!dbus_message_iter_init(msg, &args)) {
		dbus_message_unref(msg);
		return track;
	}
	
	if(dbus_message_iter_get_arg_type(&args)!=DBUS_TYPE_VARIANT) {
		dbus_message_unref(msg);
		return track;
	}
	dbus_message_iter_recurse(&args, &array);
	if(dbus_message_iter_get_arg_type(&array)!=DBUS_TYPE_ARRAY) {
		dbus_message_unref(msg);
		return track;
	}
	
	for(dbus_message_iter_recurse(&array, &dict); (current_type=dbus_message_iter_get_arg_type(&dict))!=DBUS_TYPE_INVALID; dbus_message_iter_next(&dict)) {
		char *element_key, *element_value;
		int element_type;
		dbus_message_iter_recurse(&dict, &element);
		if(dbus_message_iter_get_arg_type(&element)!=DBUS_TYPE_STRING)
			continue;
		
		dbus_message_iter_get_basic(&element, &element_key);
		dbus_message_iter_next(&element);
		if(dbus_message_iter_get_arg_type(&element)!=DBUS_TYPE_VARIANT)
			continue;
		
		dbus_message_iter_recurse(&element, &var);
		element_type=dbus_message_iter_get_arg_type(&var);
		if(element_type==DBUS_TYPE_STRING) {
			dbus_message_iter_get_basic(&var, &element_value);
			if(!strcmp(element_key, "xesam:album"))
				snprintf(track.album, TRACK_ELEMENT_LENGTH, "%s", element_value);
			else if(!strcmp(element_key, "xesam:title"))
				snprintf(track.title, TRACK_ELEMENT_LENGTH, "%s", element_value);
		} else if(element_type==DBUS_TYPE_ARRAY) {
			/*just handle one artist for now*/
			dbus_message_iter_recurse(&var, &artist);
			if(dbus_message_iter_get_arg_type(&artist)!=DBUS_TYPE_STRING)
				continue;
			dbus_message_iter_get_basic(&artist, &element_value);
			if(!strcmp(element_key, "xesam:artist"))
				snprintf(track.artist, TRACK_ELEMENT_LENGTH, "%s", element_value);
		}
	}
	dbus_message_unref(msg);
	return track;
}

static enum PLAYBACK_STATUS mediaplayer_get_status(const char *id) {
	DBusMessage* msg;
	DBusMessageIter args, var;
	DBusPendingCall* pending;
	static const char *prop="PlaybackStatus";
	char *status;
	char player[256];
	enum PLAYBACK_STATUS ret=PLAYBACK_STATUS_STOPPED;
	
	sprintf(player, "%s%s", mpris.base, id);
	if(!(msg=dbus_message_new_method_call(player, mpris.object, dbus.interface_prop, "Get")))
		return ret;
	dbus_message_iter_init_append(msg, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &mpris.interface_player)) {
		dbus_message_unref(msg);
		return ret;
	}
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop)) {
		dbus_message_unref(msg);
		return ret;
	}
	if(!dbus_connection_send_with_reply(dbus.session.connection, msg, &pending, -1)) {
		dbus_message_unref(msg);
		return ret;
	}
	if(!pending) {
		dbus_message_unref(msg);
		return ret;
	}
	dbus_connection_flush(dbus.session.connection);
	dbus_message_unref(msg);
	dbus_pending_call_block(pending);
	if(!(msg=dbus_pending_call_steal_reply(pending))) {
		dbus_pending_call_unref(pending);
		return ret;
	}
	dbus_pending_call_unref(pending);
	if(!dbus_message_iter_init(msg, &args)) {
		dbus_message_unref(msg);
		return ret;
	}
	if(dbus_message_iter_get_arg_type(&args)!=DBUS_TYPE_VARIANT) {
		dbus_message_unref(msg);
		return ret;
	}
	dbus_message_iter_recurse(&args, &var);
	if(dbus_message_iter_get_arg_type(&var)!=DBUS_TYPE_STRING) {
		dbus_message_unref(msg);
		return ret;
	}
	
	dbus_message_iter_get_basic(&var, &status);
	if(!strcmp(status, "Playing"))
		ret=PLAYBACK_STATUS_PLAYING;
	else if(!strcmp(status, "Paused"))
		ret=PLAYBACK_STATUS_PAUSED;
	
	dbus_message_unref(msg);
	return ret;
}

static void mediaplayer_raise(const char *id) {
	DBusMessage *msg;
	char player[256];
	sprintf(player, "%s%s", mpris.base, id);
	msg=dbus_message_new_method_call(player, mpris.object, mpris.interface, "Raise");
	dbus_connection_send(dbus.session.connection, msg, &dbus.session.serial);
	dbus_connection_flush(dbus.session.connection);
	dbus_message_unref(msg);
	dbus.session.serial++;
}

static void mediaplayer_action(const char *id, enum MEDIAPLAYER_ACTION action) {
	DBusMessage *msg;
	static const char *actions[]={
		"Previous",
		"PlayPause",
		"Next",
		"Play",
		"Pause",
	};
	char player[256];
	sprintf(player, "%s%s", mpris.base, id);
	msg=dbus_message_new_method_call(player, mpris.object, mpris.interface_player, actions[action]);
	dbus_connection_send(dbus.session.connection, msg, &dbus.session.serial);
	dbus_connection_flush(dbus.session.connection);
	dbus_message_unref(msg);
	dbus.session.serial++;
}

static void menu_open(Indicator *indicator) {
	int i;
	struct MEDIAPLAYER *mp;
	for(mp=mediaplayer, i=0; mp; mp=mp->next, i++);
	menu.selected=-1;
	menu.x=selmon->mx+indicator->x-MENU_WIDTH+indicator->width;
	menu.y=selmon->my+bh;
	menu.w=MENU_WIDTH;
	menu.h=bh*2+bh*5*i;
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

static void check_bus() {
	DBusMessage* msg;
	DBusMessageIter args;
	char *player, *oldowner, *newowner;
	
	while(1) {
		dbus_connection_read_write(dbus.session.connection, 0);

		if(!(msg=dbus_connection_pop_message(dbus.session.connection)))
			break;
		
		if(dbus_message_is_signal(msg, dbus.interface, "NameOwnerChanged")) {
			if(!(dbus_message_iter_init(msg, &args)&&dbus_message_iter_get_arg_type(&args)==DBUS_TYPE_STRING))
				continue;
			
			dbus_message_iter_get_basic(&args, &player);
			dbus_message_iter_next(&args);
			dbus_message_iter_get_basic(&args, &oldowner);
			dbus_message_iter_next(&args);
			dbus_message_iter_get_basic(&args, &newowner);
			if(!strncmp(player, mpris.base, strlen(mpris.base))) {
				if(!strlen(oldowner)&&strlen(newowner))
					mediaplayer_register(player+strlen(mpris.base));
				if(!strlen(newowner))
					mediaplayer_deregister(player+strlen(mpris.base));
			}
		}

		dbus_message_unref(msg);
	}
}

static long volume_get() {
	long volume;
	if(snd_mixer_selem_get_playback_volume(alsa.elem_play, 0, &volume)<0)
		return -1;
	
	volume-=alsa.minv;
	return (100L*volume)/(alsa.maxv-alsa.minv);
}

static void volume_set(long volume) {
	if(volume<0)
		volume=0;
	if(volume>100L)
		volume=100L;
	
	volume=(volume*((long) (alsa.maxv-alsa.minv)))/100L+alsa.minv;
	snd_mixer_selem_set_playback_volume(alsa.elem_play, 0, volume);
	snd_mixer_selem_set_playback_volume(alsa.elem_play, 1, volume);
}

static void mute_set(Bool mute) {
	if(snd_mixer_selem_has_playback_switch(alsa.elem_play))
		snd_mixer_selem_set_playback_switch_all(alsa.elem_play, !mute);
}

static Bool mute_get() {
	int active=1;
	if(snd_mixer_selem_has_playback_switch(alsa.elem_play))
		snd_mixer_selem_get_playback_switch(alsa.elem_play, 0, &active);
	return !active;
}

static void mute_mic_set(Bool mute) {
	if(snd_mixer_selem_has_capture_switch(alsa.elem_cap))
		snd_mixer_selem_set_capture_switch_all(alsa.elem_cap, !mute);
}

static Bool mute_mic_get() {
	int active=1;
	if(snd_mixer_selem_has_capture_switch(alsa.elem_cap))
		snd_mixer_selem_get_capture_switch(alsa.elem_cap, 0, &active);
	return !active;
}

int indicator_music_init(Indicator *indicator) {
	DBusMessage* msg;
	DBusMessageIter args, element;
	DBusPendingCall* pending;
	char *player;
	int current_type;
	
	if(!dbus.session.connection)
		return -1;
	
	if(!(msg=dbus_message_new_method_call(dbus.interface, dbus.object, dbus.interface, "ListNames")))
		return -1;
	
	if(!dbus_connection_send_with_reply(dbus.session.connection, msg, &pending, -1)) {
		dbus_connection_unref(dbus.session.connection);
		return -1;
	}
	if(!pending) {
		dbus_message_unref(msg);
		dbus_connection_unref(dbus.session.connection);
		return -1;
	}
	dbus_connection_flush(dbus.session.connection);
	dbus_message_unref(msg);
	dbus_pending_call_block(pending);

	if(!(msg=dbus_pending_call_steal_reply(pending))) {
		dbus_connection_unref(dbus.session.connection);
		return -1;
	}
	dbus_pending_call_unref(pending);

	if(!(dbus_message_iter_init(msg, &args)&&dbus_message_iter_get_arg_type(&args)==DBUS_TYPE_ARRAY)) {
		dbus_message_unref(msg);
		dbus_connection_unref(dbus.session.connection);
		return -1;
	}
	
	for(dbus_message_iter_recurse(&args, &element); (current_type=dbus_message_iter_get_arg_type(&element))!=DBUS_TYPE_INVALID; dbus_message_iter_next(&element)) {
		if(current_type!=DBUS_TYPE_STRING)
			continue;
		dbus_message_iter_get_basic(&element, &player);
		if(!strncmp(player, mpris.base, strlen(mpris.base))) {
			mediaplayer_register(player+strlen(mpris.base));
		}
	}
	dbus_message_unref(msg);
	
	dbus_bus_add_match(dbus.session.connection, "type='signal',interface='org.freedesktop.DBus',member='NameOwnerChanged'", &dbus.session.error);
	dbus_connection_flush(dbus.session.connection);
	if(dbus_error_is_set(&dbus.session.error)) { 
		dbus_connection_unref(dbus.session.connection);
		mediaplayer_deregister_all();
		return -1;
	}
	
	snd_mixer_selem_id_t *sid_play;
	snd_mixer_selem_id_t *sid_cap;
	/*init alsa*/
	snd_mixer_selem_id_alloca(&sid_play);
	snd_mixer_selem_id_set_index(sid_play, alsa.mix_index_play);
	snd_mixer_selem_id_set_name(sid_play, alsa.mix_name_play);
	
	snd_mixer_selem_id_alloca(&sid_cap);
	snd_mixer_selem_id_set_index(sid_cap, alsa.mix_index_cap);
	snd_mixer_selem_id_set_name(sid_cap, alsa.mix_name_cap);

	if((snd_mixer_open(&alsa.handle, 0))<0)
		return -1;
	if((snd_mixer_attach(alsa.handle, alsa.card))<0) {
		snd_mixer_close(alsa.handle);
		return -1;
	}
	if((snd_mixer_selem_register(alsa.handle, NULL, NULL))<0) {
		snd_mixer_close(alsa.handle);
		return -1;
	}
	if(snd_mixer_load(alsa.handle)<0) {
		snd_mixer_close(alsa.handle);
		return -1;
	}
	if(!(alsa.elem_play = snd_mixer_find_selem(alsa.handle, sid_play))) {
		snd_mixer_close(alsa.handle);
		return -1;
	}
	if(!(alsa.elem_cap = snd_mixer_find_selem(alsa.handle, sid_cap))) {
		snd_mixer_close(alsa.handle);
		return -1;
	}
	
	snd_mixer_selem_get_playback_volume_range (alsa.elem_play, &alsa.minv, &alsa.maxv);
	return 0;
}

void indicator_music_update(Indicator *indicator) {
	long int i, vol;
	char *ic;
	
	check_bus();
	snd_mixer_handle_events(alsa.handle);
	if(mute_get())
		sprintf(indicator->text, " \uf35a %li%% ", volume_get());
	else {
		vol = volume_get();
		ic = "\uf357";
		for(i = 0; i < sizeof(icon)/sizeof(Icon); i++) {
			if(vol < icon[i].percentage) {
				ic = icon[i].str;
				break;
			}
		}
		sprintf(indicator->text, " %s %li%% ", ic, vol);
	}
}

void indicator_music_expose(Indicator *indicator, Window window) {
	int i;
	int y=0, sliderw=menu.w-10*2;
	struct MEDIAPLAYER *mp;
	if(window!=menu.window)
		return;
	snd_mixer_handle_events(alsa.handle);
	
	indicator_draw_text(menu.window, menu.gc, 0, 0, menu.w, bh, menu.selected==0?dc.sel:dc.norm, mute_get()?"Unmute":"Mute", False);
	XSetForeground(dpy, menu.gc, (menu.selected==1?dc.sel:dc.norm)[ColBG]);
	XFillRectangle(dpy, window, menu.gc, 0, bh, menu.w, bh);
	XSetForeground(dpy, menu.gc, (menu.selected==1?dc.sel:dc.norm)[ColFG]);
	XDrawLine(dpy, window, menu.gc, 8, bh+bh/2, menu.w-8, bh+bh/2);
	XFillRectangle(dpy, window, menu.gc, 10-2+sliderw*volume_get()/100, bh+2, 4, bh-4);
	
	for(mp=mediaplayer, y=2*bh, i=2; mp; mp=mp->next, y+=bh*5, i+=5) {
		struct TRACK track=mediaplayer_get_track(mp->id);
		enum PLAYBACK_STATUS status=mediaplayer_get_status(mp->id);
		indicator_draw_text(menu.window, menu.gc, 0, y, menu.w, bh, i==menu.selected?dc.sel:dc.norm, mp->name?mp->name:mp->id, False);
		
		if(status==PLAYBACK_STATUS_STOPPED) {
			XSetForeground(dpy, menu.gc, dc.norm[ColBG]);
			XFillRectangle(dpy, window, menu.gc, 0, y+bh, menu.w, bh*3);
			indicator_draw_text(menu.window, menu.gc, 8, y+bh*2, menu.w, bh, dc.norm, "Playback stopped", False);
		} else {
			indicator_draw_text(menu.window, menu.gc, 8, y+bh, menu.w, bh, dc.norm, track.title, False);
			indicator_draw_text(menu.window, menu.gc, 8, y+bh*2, menu.w, bh, dc.norm, track.artist, False);
			indicator_draw_text(menu.window, menu.gc, 8, y+bh*3, menu.w, bh, dc.norm, track.album, False);
		}
		
		indicator_draw_text(menu.window, menu.gc, menu.w/2-BUTTON_W/2-BUTTON_W, y+bh*4, BUTTON_W, bh, i+4==menu.selected&&menu.button==0?dc.sel:dc.norm, "▮◀", False);
		indicator_draw_text(menu.window, menu.gc, menu.w/2-BUTTON_W/2, y+bh*4, BUTTON_W, bh, i+4==menu.selected&&menu.button==1?dc.sel:dc.norm, status==PLAYBACK_STATUS_PLAYING?"▮▮":" ▶", False);
		indicator_draw_text(menu.window, menu.gc, menu.w/2-BUTTON_W/2+BUTTON_W, y+bh*4, BUTTON_W, bh, i+4==menu.selected&&menu.button==2?dc.sel:dc.norm, "▶▮", False);
	}
	XFlush(dpy);
}

Bool indicator_music_haswindow(Indicator *in, Window window) {
	return menu.window==window?True:False;
}

void indicator_music_mouse(Indicator *indicator, XButtonPressedEvent *ev) {
	if(ev->type != ButtonPress) {
		Window w;
		int tmp;
		int x, y;
		unsigned int mask;
		XQueryPointer(dpy, menu.window, &w, &w, &tmp, &tmp, &x, &y, &mask);
		
		menu.selected=-1;
		if(x>=0&&y>=0&&x<menu.w&&y<menu.h) {
			if(mask&0x100&&y/bh==1) {
				int sliderw=menu.w-10*2;
				snd_mixer_handle_events(alsa.handle);
				volume_set(100*(x-10)/sliderw);
			}
			menu.selected=y/bh;
			menu.button=x>=menu.w/2-BUTTON_W*3/2&&x<menu.w/2+BUTTON_W*2/2?(x-(menu.w/2-BUTTON_W*3/2))/BUTTON_W:-1;
		}
		
		indicator_music_expose(indicator, menu.window);
		return;
	}
	if(ev->window==menu.window) {
		int sliderw=menu.w-10*2;
		int item=ev->y/bh;
		struct MEDIAPLAYER *mp;
		
		if(item==0) {
			snd_mixer_handle_events(alsa.handle);
			mute_set(!mute_get());
		} else if(item==1) {
			if(ev->x>=10&&ev->x<menu.w-10) {
				snd_mixer_handle_events(alsa.handle);
				volume_set(100*(ev->x-10)/sliderw);
			}
		} else {
			int i;
			for(mp=mediaplayer, i=2; mp; mp=mp->next, i+=5)
				if(item==i) {
					mediaplayer_raise(mp->id);
					indicator->active=False;
					menu_close();
					break;
				} else if(item==i+4&&menu.button>=0) {
					mediaplayer_action(mp->id, menu.button);
					indicator_music_expose(indicator, ev->window);
				}
			
			return;
		}
		indicator_music_expose(indicator, ev->window);
		
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
			snd_mixer_handle_events(alsa.handle);
			volume_set(volume_get()+6);
			break;
		case Button5:
			snd_mixer_handle_events(alsa.handle);
			volume_set(volume_get()-4);
			break;
	}
	if(indicator->active)
		indicator_music_expose(indicator, menu.window);
}

void alsa_volume(const Arg *a) {
	long vol = a->i;
	
	snd_mixer_handle_events(alsa.handle);
	vol += volume_get();
	volume_set(vol);
}

void alsa_mute_toggle(const Arg *a) {
	snd_mixer_handle_events(alsa.handle);
	mute_set(!mute_get());
}

void alsa_mute_mic_toggle(const Arg *a) {
	snd_mixer_handle_events(alsa.handle);
	mute_mic_set(!mute_mic_get());
}
