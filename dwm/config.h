/* See LICENSE file for copyright and license details. */

/* includes */
#include "layouts.c"
#include "fibonacci.c"

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const Gap default_gap        = {.isgap = 1, .realgap = 10, .gappx = 10};
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;     /* 0 means no systray */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int user_bh            = 0;        /* 0 means that dwm will calculate bar height, >= 1 means dwm will user_bh as bar height */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = {"HackNerdFontMono:size=12", "JoyPixels:pixelsize=10:antialias=true:autohint=true"};
static const char dmenufont[]       = "monospace:size=10";
//static const char col_gray1[]       = "#222222";
//static const char col_gray2[]       = "#444444";
//static const char col_gray3[]       = "#bbbbbb";
//static const char col_gray4[]       = "#eeeeee";
//static const char col_cyan[]        = "#005577";
//static const char col_orange[]      = "#ff7d52";
//static const char *colors[][3]      = {
//                   fg         bg         border   
	//[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	//[SchemeSel]  = { col_gray4, col_orange,  col_dracula  },
//};
static const char col_gray1[]       = "#282a36";
static const char col_gray2[]       = "#ffb86c";
static const char col_gray3[]       = "#ff79c6";
static const char col_gray4[]       = "#ffb86c";
static const char col_cyan[]        = "#282a36";
static const char *colors[][3]      = {
    /*               fg         bg         border   */
    [SchemeNorm] = { col_gray3, col_gray1, col_cyan },
    [SchemeSel]  = { col_gray4, col_cyan,  col_gray2  },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
//static const char *tags[] = { "???", "???", "???", "???", "???", "???", "???", "???", "???" };
//static const char *tags[] = { "???", "???", "???", "???", "???",  "???", "???", "???", "???", "???" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class        instance    title               tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Gimp",                       NULL,       NULL,    0,         1,          0,           0,        -1 },
	{ "Firefox",                    NULL,       NULL,    1 << 1,    0,          0,           0,        -1 },
	{ "LibreWolf",                    NULL,       NULL,    1 << 1,    0,          0,           0,        -1 },
	{ "St",                         NULL,       NULL,    0,         0,          1,           0,        -1 },
	{ "Alacritty",                  NULL,       NULL,    0,         0,          1,           0,        -1 },
	{ "Mailspring",                 NULL,       NULL,    1 << 8,    0,          0,           0,        -1 },
	{ "Thunderbird",                NULL,       NULL,    1 << 8,    0,          0,           0,        -1 },
    { "jetbrains-studio",           NULL,       NULL,    0,   0,    1,          0,           0,        -1 },
	{ NULL,                         NULL,       "Event Tester",     0,              0,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "HHH",      grid },
 	{ "[@]",      spiral },
 	{ "[\\]",     dwindle },
    { NULL,       NULL },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

#define STACKKEYS(MOD,ACTION) \
	{ MOD, XK_j,     ACTION##stack, {.i = INC(+1) } }, \
	{ MOD, XK_k,     ACTION##stack, {.i = INC(-1) } }, \
	{ MOD, XK_v,     ACTION##stack, {.i = 0 } },  
	/*{ MOD, XK_l,     ACTION##stack, {.i = -1 } }, \
    { MOD, XK_h,     ACTION##stack, {.i = 0 } }, */

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", NULL };

#include <X11/XF86keysym.h>

/* commands spawned when clicking statusbar, the mouse button pressed is exported as BUTTON 
static char *statuscmds[] = { "notify-send Mouse$BUTTON" };
static char *statuscmd[] = { "/bin/sh", "-c", NULL, NULL };
*/
static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_d,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY|ShiftMask,             XK_f,      incnmaster,     {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_d,      incnmaster,     {.i = -1 } },

	{ MODKEY|ShiftMask,             XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY|ShiftMask,             XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ControlMask,           XK_h,      setcfact,       {.f = +0.25} },
	{ MODKEY|ControlMask,           XK_l,      setcfact,       {.f = -0.25} },

	{ MODKEY,                       XK_space, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_q,      killclient,     {0} },

	{ MODKEY|ShiftMask,             XK_period,  cyclelayout,    {.i = +1 } },

	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },

    { MODKEY,                     XK_y,  focusmon,           {.i = -1 } },
    { MODKEY,                     XK_o, focusmon,            {.i = +1 } },
    { MODKEY|ShiftMask,           XK_y,  tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,           XK_o, tagmon,         {.i = +1 } },

	{ MODKEY|ControlMask,           XK_comma,  setgaps,        {.i = -5 } },
	{ MODKEY|ControlMask,           XK_period,  setgaps,       {.i = +5 } },
    //{ MODKEY|ShiftMask,             XK_minus,  setgaps,        {.i = GAP_RESET } },
    //{ MODKEY|ShiftMask,             XK_equal,  setgaps,        {.i = GAP_TOGGLE} },

	STACKKEYS(MODKEY,                    focus)
	STACKKEYS(MODKEY|ShiftMask,          push)
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_F2,      quit,         {0} },


    { MODKEY|ShiftMask,             XK_w,      spawn,           SHCMD("firefox-bin") },
	{ MODKEY,                       XK_w,      spawn,           SHCMD("librewolf") },
	{ MODKEY,                       XK_s,      spawn,           SHCMD("st -e ranger") },
	{ MODKEY|ShiftMask,             XK_s,      spawn,           SHCMD("pcmanfm") },
	{ MODKEY|ShiftMask,             XK_a,      spawn,           SHCMD("pavucontrol") },
	{ MODKEY|ShiftMask,             XK_t,      spawn,           SHCMD("translate") },

    {0, XF86XK_AudioMute,   spawn,  SHCMD("pulsemixer --toggle-mute; kill -44 $(pidof dwmblocks)")},
    {0, XF86XK_AudioLowerVolume,   spawn,  SHCMD("pulsemixer --change-volume -5; kill -44 $(pidof dwmblocks)")},
    {0, XF86XK_AudioRaiseVolume,   spawn,  SHCMD("pulsemixer --change-volume +5; kill -44 $(pidof dwmblocks)")},
    {0, XF86XK_AudioPlay,   spawn,  SHCMD("dbus-send --print-reply --dest=org.mpris.MediaPlayer2.spotify /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.PlayPause")},
    {0, XF86XK_AudioNext,   spawn,  SHCMD("dbus-send --print-reply --dest=org.mpris.MediaPlayer2.spotify /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Next")},
    {0, XF86XK_AudioPrev,   spawn,  SHCMD("dbus-send --print-reply --dest=org.mpris.MediaPlayer2.spotify /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Previous")},
    {0, XK_Print,   spawn,  SHCMD("scrot && notify-send -i 'notification-message-IM' 'Scrot' 'Ekran G??r??nt??s?? Al??nd??!'")},
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
#ifndef __OpenBSD__
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button1,        sigdwmblocks,   {.i = 1} },
	{ ClkStatusText,        0,              Button2,        sigdwmblocks,   {.i = 2} },
	{ ClkStatusText,        0,              Button3,        sigdwmblocks,   {.i = 3} },
	{ ClkStatusText,        0,              Button4,        sigdwmblocks,   {.i = 4} },
	{ ClkStatusText,        0,              Button5,        sigdwmblocks,   {.i = 5} },
	{ ClkStatusText,        ShiftMask,      Button1,        sigdwmblocks,   {.i = 6} },
#endif
	{ ClkStatusText,        ShiftMask,      Button3,        spawn,          SHCMD("st -e nvim ~/Projects/dwmblocks/config.h") },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
	{ ClkRootWin,		    0,	            Button2,	    togglebar,	    {0} },
};
