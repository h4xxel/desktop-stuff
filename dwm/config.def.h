/* See LICENSE file for copyright and license details. */

/* appearance */
static const char font[]		= "Icons,DejaVu Sans, 10";
static const char normbordercolor[]	= "#3c3b37";
static const char normbgcolor[]		= "#3c3b37";
static const char normfgcolor[]		= "#bbbbbb";
//static const char selbordercolor[]	= "#3c3b37";
static const char selbordercolor[]	= "#005577";
static const char selbgcolor[]		= "#005577";
static const char selfgcolor[]		= "#eeeeee";
static const char floatnormbordercolor[]= "#3c3b37";
//static const char floatselbordercolor[]	= "#005577";
static const char floatselbordercolor[]	= "#FF0000";
static const unsigned int borderpx	= 2;		/* border pixel of windows */
static const unsigned int snap		= 32;		/* snap pixel */
static const unsigned int systrayspacing= 2;
static const Bool showsystray		= True;
static const Bool showbar		= True;
static const Bool topbar		= True;		/* False means bottom bar */
static const Bool statusmarkup      = True;     /* True means use pango markup in status message */

static const char *wmname="LG3D";

/* tagging */
//static const char *tags[] = {"web", "term", "dev", "file", "chat", "mono", "virt", "spare", "float"};
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* class      instance    title       tags mask     isfloating   sizehints   monitor */
	/*{"Gimp",     NULL,       NULL,       0,            True,        -1},*/
#if 0
	{"Firefox",  NULL,       NULL,       1,       False, True,       1},
	{"Pale moon",  NULL,       NULL,       1,       False, True,       1},
	{"Chromium-browser",  NULL,       NULL,       1,       False, True,       1},
	{"qutebrowser",  NULL,       NULL,       1,       False, True,       1},
	
	//{"Scite",  NULL,       NULL,       1<<2,       False, True,       0},
	{"Geany",  NULL,       NULL,       1<<2,       False, True,      0},
	
	{"Nemo",  NULL,       NULL,       1<<3,       False, True,       0},
	{"Nautilus",  NULL,       NULL,       1<<3,       False, True,       0},
	
	
	{"Wine",  NULL,       NULL,       1<<5,       False, True,       0},
	{"Steam",  NULL,       NULL,       1<<5,       False, True,       0},
	{"Opencpn",  NULL,       NULL,       1<<5,       False, True,       0},
	{"Gimp",  NULL,       NULL,       1<<5,       False, True,       0},
	
	{"Virtualbox",  NULL,       NULL,       1<<6,       False, True,       0},
	
	{"Thunderbird",  NULL,       NULL,       1<<7,       False, True,        0},
	
	//{"file_progress",  NULL,       NULL,       1<<8,       True, True,       0},
#endif
	{"Audacious",  NULL,       NULL,       1<<8,       False, True,       0},
	{"Update-manager",  NULL,       NULL,       1<<8,       True, True,       0},
	/*{"Update-manager",  NULL,       NULL,       0,       True, True,       -1},*/
	
	{"UXTerm", NULL, NULL, 0, False, False, -1},
	{"XTerm", NULL, NULL, 0, False, False, -1},
	{"URxvt", NULL, NULL, 0, False, False, -1},
};

/* layout(s) */
static const float mfact	= 0.55;	/* factor of master area size [0.05..0.95] */
static const int nmaster	= 1;	/* number of clients in master area */
static const Bool resizehints	= True;	/* True means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol, arrange function */
	{"\uf417", tile},	/* first entry is default */
	{"\uf41c", NULL},	/* no layout function means floating behavior */
	{"\uf419", monocle},
	{"\uf418", bstack},
	{"\uf41a", bstackhoriz},
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{MODKEY,			KEY,	view,           {.ui = 1 << TAG}}, \
	{MODKEY|ControlMask,		KEY,	toggleview,     {.ui = 1 << TAG}}, \
	{MODKEY|ShiftMask,		KEY,	tag,            {.ui = 1 << TAG}}, \
	{MODKEY|ControlMask|ShiftMask,	KEY,	toggletag,      {.ui = 1 << TAG}}

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) {.v = (const char*[]){"/bin/sh", "-c", cmd, NULL}}

/* commands */
static const char *dmenucmd[]	= {"dmenu_run", "-fn", "sans-10", "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL};
static const char *rofiappcmd[]	= {"rofi", "-show", "drun", NULL};
static const char *rofiruncmd[]	= {"rofi", "-show", "run", NULL};
static const char *rofiwindowcmd[]	= {"rofi", "-show", "window", NULL};
static const char *rofifilecmd[]	= {"rofi", "-show", "filebrowser", NULL};
static const char *termcmd[]	= {"urxvt", NULL};
static const char *webcmd[]	= {"firefox", NULL};
static const char *logoutcmd[]	= {"gnome-session-quit", NULL};
static const char *lockcmd[]	= {"dm-tool", "lock", NULL};
static const char *brigthness_up[]	= {"xbacklight", "-inc", "10", "-steps", "1", NULL};
static const char *brigthness_down[]	= {"xbacklight", "-dec", "10", "-steps", "1", NULL};
static const char *networkmanager[] = {"networkmanager_dmenu", NULL};

static Key keys[] = {
	/* modifier                     key        function        argument */
	{MODKEY,			XK_p,      spawn,          {.v = rofiappcmd}},
	{MODKEY|ShiftMask,		XK_p,      spawn,          {.v = rofiruncmd}},
	{MODKEY,			XK_o,      spawn,          {.v = rofifilecmd}},
	{MODKEY,			XK_w,      spawn,          {.v = rofiwindowcmd}},
	{MODKEY,			XK_c,      spawn,          {.v = networkmanager}},
	{MODKEY|ShiftMask,		XK_Return, spawn,          {.v = termcmd}},
	{MODKEY|ShiftMask,		XK_f, spawn,          {.v = webcmd}},
	{MODKEY,			XK_b,      togglebar,      {0}},
	{MODKEY,			XK_j,      focusstack,     {.i = +1}},
	{MODKEY,			XK_k,      focusstack,     {.i = -1}},
	{MODKEY,			XK_i,      incnmaster,     {.i = +1}},
	{MODKEY,			XK_d,      incnmaster,     {.i = -1}},
	{MODKEY,			XK_h,      setmfact,       {.f = -0.05}},
	{MODKEY,			XK_l,      setmfact,       {.f = +0.05}},
	{MODKEY,			XK_Return, zoom,           {0}},
	{MODKEY,			XK_Tab,    view,           {0}},
	{MODKEY|ShiftMask,		XK_c,      killclient,     {0}},
	{MODKEY,			XK_t,      setlayout,      {.v = &layouts[0]}},
	{MODKEY,			XK_f,      setlayout,      {.v = &layouts[1]}},
	{MODKEY,			XK_m,      setlayout,      {.v = &layouts[2]}},
	{MODKEY,			XK_s,      setlayout,      {.v = &layouts[3]}},
	{MODKEY,			XK_z,      setlayout,      {.v = &layouts[4]}},
	{MODKEY,			XK_space,  setlayout,      {0}},
	{MODKEY|ShiftMask,		XK_space,  togglefloating, {0}},
	//{MODKEY,			XK_0,      view,           {.ui = ~0}},
	//{MODKEY|ShiftMask,		XK_0,      tag,            {.ui = ~0}},
	{MODKEY,			XK_comma,  focusmon,       {.i = -1}},
	{MODKEY,			XK_period, focusmon,       {.i = +1}},
	{MODKEY|ShiftMask,		XK_comma,  tagmon,         {.i = -1}},
	{MODKEY|ShiftMask,		XK_period, tagmon,         {.i = +1}},
	TAGKEYS(XK_1, 0),
	TAGKEYS(XK_2, 1),
	TAGKEYS(XK_3, 2),
	TAGKEYS(XK_4, 3),
	TAGKEYS(XK_5, 4),
	TAGKEYS(XK_6, 5),
	TAGKEYS(XK_7, 6),
	TAGKEYS(XK_8, 7),
	TAGKEYS(XK_9, 8),
	{MODKEY|ShiftMask,		XK_r,		self_restart,	{0}},
	/*{MODKEY|ShiftMask,		XK_q,		spawn,		{.v=logoutcmd}},*/
	{MODKEY|ShiftMask,		XK_q,		quit,		{0}},
	
	
	//{0,		XF86XK_Sleep,			spawn,			{.v = lockcmd}},
	{0,		XF86XK_Favorites,			power_suspend,	{0}},
	{0,		XF86XK_AudioRaiseVolume,	alsa_volume,		{.i = +3}},
	{0,		XF86XK_AudioLowerVolume,	alsa_volume,		{.i = -3}},
	{0,		XF86XK_AudioMute,		alsa_mute_toggle,	{0}},
	{0,		XF86XK_AudioMicMute,		alsa_mute_mic_toggle,	{0}},
	
	{0,		XF86XK_MonBrightnessUp,		spawn,			{.v = brigthness_up}},
	{0,		XF86XK_MonBrightnessDown,	spawn,			{.v = brigthness_down}},
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ClkLtSymbol,          0,              Button1,        setlayout,      {0}},
	{ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]}},
	{ClkWinTitle,          0,              Button1,        focusstack,           {.i=+1}},
	{ClkWinTitle,          0,              Button3,        focusstack,           {.i=-1}},
	{ClkWinTitle,          0,              Button2,        zoom,           {0}},
	/*{ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd}},*/
	{ClkClientWin,         MODKEY,         Button1,        movemouse,      {0}},
	{ClkClientWin,         MODKEY,         Button2,        togglefloating, {0}},
	{ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0}},
	{ClkTagBar,            0,              Button1,        view,           {0}},
	{ClkTagBar,            0,              Button3,        toggleview,     {0}},
	{ClkTagBar,            MODKEY,         Button1,        tag,            {0}},
	{ClkTagBar,            MODKEY,         Button3,        toggletag,      {0}},
};

