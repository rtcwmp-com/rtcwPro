/*
===========================================================================

Return to Castle Wolfenstein multiplayer GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein multiplayer GPL Source Code (RTCW MP Source Code).  

RTCW MP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW MP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW MP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW MP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW MP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

// cg_info.c -- display information while data is being loading

#include "cg_local.h"

#if 0
#define MAX_LOADING_ITEM_ICONS      26

static int loadingItemIconCount;
static qhandle_t loadingItemIcons[MAX_LOADING_ITEM_ICONS];
#endif

/*
======================
CG_LoadingString

======================
*/
void CG_LoadingString(const char *s) {
	Q_strncpyz(cg.infoScreenText, s, sizeof(cg.infoScreenText));

	if (s && s[0] != 0) {
		CG_Printf("%s", va("LOADING... %s\n", s));   //----(SA)	added so you can see from the console what's going on

	}
	trap_UpdateScreen();
}

/*
===================
CG_LoadingItem
===================
*/
void CG_LoadingItem(int itemNum) {
#if 0 //----(SA)	Max Kaufman request that we don't show any pacifier stuff for items

	gitem_t     *item;

	item = &bg_itemlist[itemNum];

	if (item->giType == IT_KEY) { // do not show keys at level startup //----(SA)
		return;
	}

	if (item->icon && loadingItemIconCount < MAX_LOADING_ITEM_ICONS) {
		loadingItemIcons[loadingItemIconCount++] = trap_R_RegisterShaderNoMip(item->icon);
	}

	CG_LoadingString(item->pickup_name);
#endif //----(SA)	end
}

/*
===================
CG_LoadingClient
===================
*/
void CG_LoadingClient(int clientNum) {
	const char      *info;
	char            *skin;
	char personality[MAX_QPATH];
	char model[MAX_QPATH];
	char iconName[MAX_QPATH];

	if (cgs.gametype == GT_SINGLE_PLAYER  && clientNum > 0) { // for now only show the player's icon in SP games
		return;
	}

	info = CG_ConfigString(CS_PLAYERS + clientNum);

	Q_strncpyz(model, Info_ValueForKey(info, "model"), sizeof(model));
	skin = strrchr(model, '/');
	if (skin) {
		*skin++ = '\0';
	}
	else {
		skin = "default";
	}

	Com_sprintf(iconName, MAX_QPATH, "models/players/%s/icon_%s.tga", model, skin);

	// (SA) ignore player icons for the moment
	if (!(cg_entities[clientNum].currentState.aiChar)) {
		//		if ( loadingPlayerIconCount < MAX_LOADING_PLAYER_ICONS ) {
		//			loadingPlayerIcons[loadingPlayerIconCount++] = trap_R_RegisterShaderNoMip( iconName );
		//		}
	}

	Q_strncpyz(personality, Info_ValueForKey(info, "n"), sizeof(personality));
	Q_CleanStr(personality);

	if (cgs.gametype == GT_SINGLE_PLAYER) {
		trap_S_RegisterSound(va("sound/player/announce/%s.wav", personality));
	}

	CG_LoadingString(personality);
}

/*
====================
CG_DrawStats
====================
*/

typedef struct {
	char    *label;
	int YOfs;
	int labelX;
	int labelFlags;
	vec4_t  *labelColor;

	char    *format;
	int formatX;
	int formatFlags;
	vec4_t  *formatColor;

	int numVars;
} statsItem_t;

// this defines the layout of the mission stats
// NOTE: these must match the stats sent in AICast_ScriptAction_ChangeLevel()
static statsItem_t statsItems[] = {
	//{ "MISSION STATS",		110, 40,		UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW,		&colorWhite,		"",		600,	UI_SMALLFONT|UI_DROPSHADOW|UI_RIGHT,		&colorWhite,	0 },

	{ "Kills", 170, 40, UI_SMALLFONT | UI_DROPSHADOW, &colorWhite, "%3i/%3i", 600, UI_SMALLFONT | UI_DROPSHADOW | UI_RIGHT, &colorWhite, 2 },
	{ " Nazis", 40, 40, UI_EXSMALLFONT | UI_DROPSHADOW, &colorWhite, "%3i/%3i", 600, UI_EXSMALLFONT | UI_DROPSHADOW | UI_RIGHT, &colorWhite, 2 },
	{ " Monsters", 15, 40, UI_EXSMALLFONT | UI_DROPSHADOW, &colorWhite, "%3i/%3i", 600, UI_EXSMALLFONT | UI_DROPSHADOW | UI_RIGHT, &colorWhite, 2 },

	{ "Time", 30, 40, UI_SMALLFONT | UI_DROPSHADOW, &colorWhite, "%2ih %2im %2is", 600, UI_SMALLFONT | UI_DROPSHADOW | UI_RIGHT, &colorWhite, 3 },

	{ "Secrets", 30, 40, UI_SMALLFONT | UI_DROPSHADOW, &colorWhite, "%i/%i", 600, UI_SMALLFONT | UI_DROPSHADOW | UI_RIGHT, &colorWhite, 2 },

	{ "Attempts", 30, 40, UI_SMALLFONT | UI_DROPSHADOW, &colorWhite, "%i", 600, UI_SMALLFONT | UI_DROPSHADOW | UI_RIGHT, &colorWhite, 1 },

	{ NULL }
};

void CG_DrawStats(char *stats) {
	int i, y, v, j;
#define MAX_STATS_VARS  64
	int vars[MAX_STATS_VARS];
	char *str, *token;
	char *formatStr = NULL; // TTimo: init
	int varIndex;
	char string[MAX_QPATH];

	UI_DrawProportionalString(320, 120, "MISSION STATS",
		UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);

	Q_strncpyz(string, stats, sizeof(string));
	str = string;
	// convert commas to spaces
	for (i = 0; str[i]; i++) {
		if (str[i] == ',') {
			str[i] = ' ';
		}
	}

	for (i = 0, y = 0, v = 0; statsItems[i].label; i++) {
		y += statsItems[i].YOfs;

		UI_DrawProportionalString(statsItems[i].labelX, y, statsItems[i].label,
			statsItems[i].labelFlags, *statsItems[i].labelColor);

		if (statsItems[i].numVars) {
			varIndex = v;
			for (j = 0; j < statsItems[i].numVars; j++) {
				token = COM_Parse(&str);
				if (!token || !token[0]) {
					CG_Error("error parsing mission stats\n");
					return;
				}

				vars[v++] = atoi(token);
			}

			// build the formatStr
			switch (statsItems[i].numVars) {
			case 1:
				formatStr = va(statsItems[i].format, vars[varIndex]);
				break;
			case 2:
				formatStr = va(statsItems[i].format, vars[varIndex], vars[varIndex + 1]);
				break;
			case 3:
				formatStr = va(statsItems[i].format, vars[varIndex], vars[varIndex + 1], vars[varIndex + 2]);
				break;
			case 4:
				formatStr = va(statsItems[i].format, vars[varIndex], vars[varIndex + 1], vars[varIndex + 2], vars[varIndex + 3]);
				break;
			}

			UI_DrawProportionalString(statsItems[i].formatX, y, formatStr,
				statsItems[i].formatFlags, *statsItems[i].formatColor);
		}
	}
}

/*
====================
CG_DrawInformation

Draw all the status / pacifier stuff during level loading
====================
*/
void CG_DrawInformation(void) {
	const char  *s;
	const char  *info;
	qhandle_t levelshot = 0; // TTimo: init
	static int callCount = 0;
	float percentDone;

	int expectedHunk;
	char hunkBuf[MAX_QPATH];

	if (cg.snap) {
		return;     // we are in the world, no need to draw information
	}

	if (callCount) {    // reject recursive calls
		return;
	}

	callCount++;

	info = CG_ConfigString(CS_SERVERINFO);

	trap_Cvar_VariableStringBuffer("com_expectedhunkusage", hunkBuf, MAX_QPATH);
	expectedHunk = atoi(hunkBuf);


	s = Info_ValueForKey(info, "mapname");
	levelshot = trap_R_RegisterShaderNoMip(va("levelshots/%s.tga", s));
	if (!levelshot) {
		levelshot = trap_R_RegisterShaderNoMip("levelshots/unknownmap.jpg");
	}
	trap_R_SetColor(NULL);
	CG_DrawPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, levelshot);

	// show the server motd
	CG_DrawMotd();

	// show the percent complete bar
	if (expectedHunk >= 0) {
		vec2_t xy = { 200, 468 };
		vec2_t wh = { 240, 10 };

		percentDone = (float)(cg_hunkUsed.integer + cg_soundAdjust.integer) / (float)(expectedHunk);
		if (percentDone > 0.97) { // never actually show 100%, since we are not in the game yet
			percentDone = 0.97;
		}
		CG_HorizontalPercentBar(xy[0], xy[1], wh[0], wh[1], percentDone);

	}

	callCount--;
}
/*
	OSPx 
	
	Demo key actions...
*/
extern void CG_createControlsWindow(void);
extern void CG_createDemoTimelineWindow(void);

void CG_DemoClick(int key) {
	int milli = trap_Milliseconds();
	int duration;
	switch (key)
	{	
	case K_END:
		if (cgs.demoTimeline.show != SHOW_ON) {
			cgs.demoTimeline.show = SHOW_ON;
			CG_createDemoTimelineWindow();
			trap_Cvar_Set("demo_timelineWindow", "1");
		}
		else {
			cgs.demoTimeline.show = SHOW_SHUTDOWN;
			if (cg.time < cgs.demoTimeline.fadeTime) {
				cgs.demoTimeline.fadeTime = 2 * cg.time + STATS_FADE_TIME - cgs.demoTimeline.fadeTime;
			}
			else {
				cgs.demoTimeline.fadeTime = cg.time + STATS_FADE_TIME;
			}
			CG_windowFree(cg.demoTimelineWindow);
			cg.demoTimelineWindow = NULL;
			trap_Cvar_Set("demo_timelineWindow", "0");
		}
		return;

	case K_PGUP:
		CG_NDP_GoToNextFrag(qtrue);
		break;
	case K_PGDN:
		CG_NDP_GoToNextFrag(qfalse);
		break;
		
	case K_TAB:
		if (cgs.demoControlInfo.show != SHOW_ON) {
			cgs.demoControlInfo.show = SHOW_ON;
			CG_createControlsWindow();
			trap_Cvar_Set("demo_controlsWindow", "1");
		}
		else {
			cgs.demoControlInfo.show = SHOW_SHUTDOWN;
			if (cg.time < cgs.demoControlInfo.fadeTime) {
				cgs.demoControlInfo.fadeTime = 2 * cg.time + STATS_FADE_TIME - cgs.demoControlInfo.fadeTime;
			}
			else {
				cgs.demoControlInfo.fadeTime = cg.time + STATS_FADE_TIME;
			}
			CG_windowFree(cg.demoControlsWindow);
			cg.demoControlsWindow = NULL;
			trap_Cvar_Set("demo_controlsWindow", "0");
		}
		return;	
	case K_BACKSPACE:
		if (cgs.demoControlInfo.show != SHOW_ON) {
			cgs.demoControlInfo.show = SHOW_ON;
			CG_createControlsWindow();
			trap_Cvar_Set("demo_controlsWindow", "1");
		}
		else {
			cgs.demoControlInfo.show = SHOW_SHUTDOWN;
			if (cg.time < cgs.demoControlInfo.fadeTime) {
				cgs.demoControlInfo.fadeTime = 2 * cg.time + STATS_FADE_TIME - cgs.demoControlInfo.fadeTime;
			}
			else {
				cgs.demoControlInfo.fadeTime = cg.time + STATS_FADE_TIME;
			}
			CG_windowFree(cg.demoControlsWindow);
			cg.demoControlsWindow = NULL;
			trap_Cvar_Set("demo_controlsWindow", "0");
		}
		return;
	case K_SHIFT:
		if (demo_infoWindow.integer)
			trap_Cvar_Set("demo_infoWindow", "0");
		else
			trap_Cvar_Set("demo_infoWindow", "1");
		return;
	case K_CTRL:
		if (demo_showTimein.integer)
			trap_Cvar_Set("demo_showTimein", "0");
		else
			trap_Cvar_Set("demo_showTimein", "1");
		return;
	case K_MOUSE1:
		if (demo_timelineWindow.integer) {
			float percentX = (float)(cgs.cursorX-35.0f) / (float)(SCREEN_WIDTH - (GIANTCHAR_WIDTH * 2));
			int serverTimeAtPercent = (int)((float)((m_lastServerTime - m_firstServerTime) * percentX)) + m_firstServerTime;
			CG_NDP_SeekAbsolute(serverTimeAtPercent);
			return;
		}
		CG_zoomViewSet_f();
		return;
	case K_MOUSE2:
		CG_zoomViewRevert_f();
		return;
	case K_ENTER:	
		trap_Cvar_Set("cg_thirdperson", ((cg_thirdPerson.integer == 0) ? "1" : "0"));
		return;	
		
	case K_UPARROW:
		if (demo_timelineWindow.integer) {
			CG_NDP_SeekRelative(60);
			return;
		}

		if (milli > cgs.thirdpersonUpdate) {
			float range = cg_thirdPersonRange.value;

			cgs.thirdpersonUpdate = milli + DEMO_THIRDPERSONUPDATE;
			range -= ((range >= 4 * DEMO_RANGEDELTA) ? DEMO_RANGEDELTA : (range - DEMO_RANGEDELTA));
			trap_Cvar_Set("cg_thirdPersonRange", va("%f", range));
		}
		return;
	case K_DOWNARROW:
		if (demo_timelineWindow.integer) {
			CG_NDP_SeekRelative(-60);
			return;
		}
		if (milli > cgs.thirdpersonUpdate) {
			float range = cg_thirdPersonRange.value;

			cgs.thirdpersonUpdate = milli + DEMO_THIRDPERSONUPDATE;
			range += ((range >= 120 * DEMO_RANGEDELTA) ? 0 : DEMO_RANGEDELTA);
			trap_Cvar_Set("cg_thirdPersonRange", va("%f", range));
		}
		return;
	case K_RIGHTARROW:
		if (demo_timelineWindow.integer) {
			if (cgs.fKeyPressed[K_CTRL]) {
				duration = 60;
			}
			else if (cgs.fKeyPressed[K_SHIFT]) {
				duration = 3;
			}
			else {
				duration = 10;
			}
			CG_NDP_SeekRelative(duration);
			return;
		}
		if (milli > cgs.thirdpersonUpdate) {
			float angle = cg_thirdPersonAngle.value - DEMO_ANGLEDELTA;

			cgs.thirdpersonUpdate = milli + DEMO_THIRDPERSONUPDATE;
			if (angle < 0) {
				angle += 360.0f;
			}
			trap_Cvar_Set("cg_thirdPersonAngle", va("%f", angle));
		}
		return;
	case K_LEFTARROW:
		if (demo_timelineWindow.integer) {
			if (cgs.fKeyPressed[K_CTRL]) {
				duration = 60;
			}
			else if (cgs.fKeyPressed[K_SHIFT]) {
				duration = 3;
			}
			else {
				duration = 10;
			}
			CG_NDP_SeekRelative(-duration);
			return;
		}
		if (milli > cgs.thirdpersonUpdate) {
			float angle = cg_thirdPersonAngle.value + DEMO_ANGLEDELTA;

			cgs.thirdpersonUpdate = milli + DEMO_THIRDPERSONUPDATE;
			if (angle >= 360.0f) {
				angle -= 360.0f;
			}
			trap_Cvar_Set("cg_thirdPersonAngle", va("%f", angle));
		}
		return;
		
	// Timescale controls
	case K_SPACE:
		trap_Cvar_Set("timescale", "1");
		trap_SendConsoleCommand("s_stop\n");
		return;
	case K_KP_UPARROW:
		trap_Cvar_Set("timescale", va("%f", cg_timescale.value + 1.0f));
		return;
	case K_KP_DOWNARROW:
	{
		float tscale = cg_timescale.value;

		if (tscale <= 1.1f) {
			if (tscale > 0.1f) {
				tscale -= 0.1f;
			}
		}
		else { tscale -= 1.0; }
		trap_Cvar_Set("timescale", va("%f", tscale));
	}
		return;
	case K_KP_RIGHTARROW:
		trap_Cvar_Set("timescale", va("%f", cg_timescale.value + 0.1f));
		return;
	case K_KP_LEFTARROW:
		if (cg_timescale.value > 0.1f) {
			trap_Cvar_Set("timescale", va("%f", cg_timescale.value - 0.1f));
		}
		return;
	case K_MWHEELDOWN:
		if (cg_timescale.value > 0.1f) {
			trap_Cvar_Set("timescale", va("%f", cg_timescale.value - 0.1f));
			return;
	case K_MWHEELUP:
		trap_Cvar_Set("timescale", va("%f", cg_timescale.value + 0.5f));
		return;

	// Hacks
	case K_F1:
		if (cgs.wallhack)
			cgs.wallhack = qfalse;
		else
			cgs.wallhack = qtrue;
		return;
	case K_F2:
		if (cgs.showNormals) {
			trap_Cvar_Set("r_showNormals", "0");
			cgs.showNormals = qfalse;
		}
		else {
			trap_Cvar_Set("r_showNormals", "1");
			cgs.showNormals = qtrue;
		}
		return;
	case K_F3:
		if (!cgs.noChat) {
			cgs.noChat = 1;
			cgs.demoPopUpInfo.show = SHOW_ON;
			CG_createDemoPopUpWindow("All chats are ^nDISABLED", 2);
		}
		else {
			cgs.noChat = 0;
			cgs.demoPopUpInfo.show = SHOW_ON;
			CG_createDemoPopUpWindow("All chats are ^nENABLED\n", 2);
		}
		return;
	case K_F4:
		if (!cgs.noVoice) {
			cgs.noVoice = 1;
			cgs.demoPopUpInfo.show = SHOW_ON;
			CG_createDemoPopUpWindow("All ^3VOICE ^7chats are ^nDISABLED", 2);
		}
		else if (cgs.noVoice == 1) {
			cgs.demoPopUpInfo.show = SHOW_ON;
			CG_createDemoPopUpWindow("All ^3VOICE ^7chats are ^nENABLED", 2);
			cgs.noVoice = 0;
		}
		return;
	case K_MOUSE3:
		if (!cgs.freezeDemo) {
			cgs.freezeDemo = qtrue;
			trap_Cvar_Set("cl_freezeDemo", "1");
		}
		else {
			cgs.freezeDemo = qfalse;
			trap_Cvar_Set("cl_freezeDemo", "0");
		}
		return;
		}
	}
}



/*
	OSPx 

	Demo controls
*/
typedef struct {
	char *command;
	char *key;
} helpCmd_reference_t;

static const helpCmd_reference_t helpInfo[] = {
		{ "TAB",		"Show/Hide This Window" },
		{ "SHIFT",		"Show/Hide Status Window" },
		{ "CTRL",		"Show/Hide Start Timer"}, 
		{ "END",		"Show/Hide Demo Timeline"},
		{ " ",			" "},
		{ "F1",			"Toggle Wallhack" },
		{ "F2",			"Toggle ShowNormals" },
		{ "F3",			"Show/Hide Chats" },
		{ "F4",			"Show/Hide Voice chats" },
		{ " ", " " },
		{ "MOUSE 1",	"Zoom IN FOV" },
		{ "MOUSE 2",	"Zoom OUT FOV" },
		{ "MOUSE 3",	"Toggle Demo Freeze"},
		{ " ", " " },			
		{ "NUM ARROWS",	"TimeScale Slow/Fast"},
		{ "SPACE",		"Timescale reset" },
		{ "SCROLL",		"Timescale Slow/Fast" },
		{ " ", " " },		
		{ "ENTER",		"Toggle third person view"},
		{ "ARROWS",		"Third person rotation" },
		{ "",			"Timeline jump forward/back" }
	    
};

void CG_createControlsWindow(void) {
	if (cgs.demoControlInfo.show == SHOW_OFF) {
		return;
	}
	else {		
		vec4_t colorGeneralFill = { 0.1f, 0.1f, 0.1f, 0.8f };
		int i, aHelp = ARRAY_LEN(helpInfo);
		const helpCmd_reference_t *hCM;		

		if (aHelp != 0) {
			cg_window_t *sw = CG_windowAlloc(WFX_TEXTSIZING | WFX_FADEIN | WFX_FLASH | WFX_SCROLLRIGHT, 500);
			char *str;

			cg.demoControlsWindow = sw;
			if (sw == NULL) {
				return;
			}

			// Window specific
			sw->id = WID_DEMOCONTROLS;
			sw->fontScaleX = 0.7f;
			sw->fontScaleY = 0.8f;
			sw->x = -10;
			sw->y = -36;
			sw->flashMidpoint = sw->flashPeriod * 0.7f;
			memcpy(&sw->colorBackground2, colorGeneralFill, sizeof(vec4_t));

			// Pump stuff in it now
			cg.windowCurrent = sw;
			for (i = 0; i < aHelp; i++) {
				hCM = &helpInfo[i];

				if (hCM->command) {
					str = va("^n%-14s ^z%s", hCM->command, hCM->key);
					CG_printWindow((char*)str);
				}
				else {
					break;
				}
			}
		}
	}
}

void CG_createDemoTimelineWindow(void) {
	if (cgs.demoTimeline.show == SHOW_OFF) {
		return;
	}
	else {
		vec4_t colorGeneralFill = { 0.1f, 0.1f, 0.1f, 0.8f };

		cg_window_t* sw = CG_windowAlloc( WFX_FADEIN | WFX_FLASH | WFX_SCROLLUP, 500);

		cg.demoTimelineWindow = sw;
		if (sw == NULL) {
			return;
		}

		// Window specific
		sw->id = WID_DEMOTIMELINE;
		sw->fontScaleX = 0.7f;
		sw->fontScaleY = 0.8f;
		sw->x = 0;
		sw->y = 480-64;
		sw->h = 64;
		sw->w = 640;
		sw->flashMidpoint = sw->flashPeriod * 0.7f;
		memcpy(&sw->colorBackground2, colorGeneralFill, sizeof(vec4_t));

		// Pump stuff in it now
		cg.windowCurrent = sw;
		cgs.cursorY = 240;
		cgs.cursorX = 320;
	}
}

/*
	Basic info and not even a window..

	NOTE: Ugly inlines :|
*/
void CG_demoView(void) {

	if (cg.demoPlayback && demo_infoWindow.integer) {
		vec4_t colorGeneralFill = { 0.1f, 0.1f, 0.1f, 0.4f };
		vec4_t colorBorderFill = { 0.1f, 0.1f, 0.1f, 0.8f };
		char *s = va("^nWallhack: ^7%s ^n| Timescale: ^7%.1f", (cgs.wallhack ? "On" : "Off"), cg_timescale.value);
		char *ts = (cg_timescale.value != 1.0 ? "Space: Default" : "Fst/Slw: Scroll");
		int w = CG_DrawStrlen(s) * SMALLCHAR_WIDTH;
		char *s2 = (cgs.wallhack ? va("^nToggle: F1     | %s", ts) : va("^nToggle: F1      | %s", ts));

		CG_FillRect(42 - 2, 400, w + 5, SMALLCHAR_HEIGHT + 3, colorGeneralFill);
		CG_DrawRect(42 - 2, 400, w + 5, SMALLCHAR_HEIGHT + 3, 1, colorBorderFill);

		CG_DrawStringExt(42, 400, s, colorWhite, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0);
		CG_DrawStringExt(42, 420, s2, colorWhite, qfalse, qtrue, TINYCHAR_WIDTH - 1, TINYCHAR_HEIGHT - 1, 0);
	}
}

/*
	Tournament Overlay
*/
/*
void CG_tournamentOverlay(void) {
	int x, y;	
	char *str;

	if (cgs.tournamentMode == TOURNY_FULL && cg.tournamentInfo.inProgress && cg_tournamentHUD.integer) {

		// Don't draw timer if client is checking scoreboard
		if (CG_DrawScoreboard())
			return;

		x = TOURINFO_RIGHT;
		y = TOURINFO_TOP - (2 * (TOURINFO_TEXTSIZE + 1));

		// Round
		str = va("^n%d/%d", (cg.tournamentInfo.resultAxis + cg.tournamentInfo.resultAllied + 1), cg.tournamentInfo.rounds);
		CG_DrawStringExt(x - (CG_DrawStrlen(str) * (TOURINFO_TEXTSIZE - 1)) - 2,
			y + 1,
			str,
			colorWhite, qfalse, qtrue,
			TOURINFO_TEXTSIZE - 1,
			TOURINFO_TEXTSIZE - 1, 0);

		// Axis
		{	
			y += 2 * (TOURINFO_TEXTSIZE + 1);
			CG_DrawPic(TOURINFO_RIGHT - (2 * TOURINFO_TEXTSIZE), y, 2 * TOURINFO_TEXTSIZE, TOURINFO_TEXTSIZE, trap_R_RegisterShaderNoMip("ui_mp/assets/ger_flag.tga"));

			y += TOURINFO_TEXTSIZE + 1;
			str = va("W: ^7%2d", cg.tournamentInfo.resultAxis);
			CG_DrawStringExt(x - (CG_DrawStrlen(str) * (TOURINFO_TEXTSIZE - 1)) - 2,
				y,
				str,
				colorOrange, qfalse, qtrue,
				TOURINFO_TEXTSIZE - 1,
				TOURINFO_TEXTSIZE - 1, 0);

			y += TOURINFO_TEXTSIZE + 1;
			str = va("T: ^7%2d", cg.tournamentInfo.timeoutAxis);
			CG_DrawStringExt(x - (CG_DrawStrlen(str) * (TOURINFO_TEXTSIZE - 1)) - 2,
				y,
				str,
				colorOrange, qfalse, qtrue,
				TOURINFO_TEXTSIZE - 1,
				TOURINFO_TEXTSIZE - 1, 0);
			
			if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR) {
				y += TOURINFO_TEXTSIZE + 1;
				str = va("Re: ^7%2d", (int)CG_CalculateReinfTimeSpecs(TEAM_RED));
				CG_DrawStringExt(x - (CG_DrawStrlen(str) * (TOURINFO_TEXTSIZE - 1)) - 2,
					y,
					str,
					colorOrange, qfalse, qtrue,
					TOURINFO_TEXTSIZE - 1,
					TOURINFO_TEXTSIZE - 1, 0);
			}
		}

		// Allies
		{
			y += 2 * (TOURINFO_TEXTSIZE + 1);
			CG_DrawPic(TOURINFO_RIGHT - (2 * TOURINFO_TEXTSIZE), y, 2 * TOURINFO_TEXTSIZE, TOURINFO_TEXTSIZE, trap_R_RegisterShaderNoMip("ui_mp/assets/usa_flag.tga"));

			y += TOURINFO_TEXTSIZE + 1;
			str = va("W: ^7%2d", cg.tournamentInfo.resultAllied);
			CG_DrawStringExt(x - (CG_DrawStrlen(str) * (TOURINFO_TEXTSIZE - 1)) - 2,
				y,
				str,
				colorOrange, qfalse, qtrue,
				TOURINFO_TEXTSIZE - 1,
				TOURINFO_TEXTSIZE - 1, 0);

			y += TOURINFO_TEXTSIZE + 1;
			str = va("T: ^7%2d", cg.tournamentInfo.timeoutAllied);
			CG_DrawStringExt(x - (CG_DrawStrlen(str) * (TOURINFO_TEXTSIZE - 1)) - 2,
				y,
				str,
				colorOrange, qfalse, qtrue,
				TOURINFO_TEXTSIZE - 1,
				TOURINFO_TEXTSIZE - 1, 0);

			if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR) {
				y += TOURINFO_TEXTSIZE + 1;
				str = va("Re: ^7%2d", (int)CG_CalculateReinfTimeSpecs(TEAM_BLUE));
				CG_DrawStringExt(x - (CG_DrawStrlen(str) * (TOURINFO_TEXTSIZE - 1)) - 2,
					y,
					str,
					colorOrange, qfalse, qtrue,
					TOURINFO_TEXTSIZE - 1,
					TOURINFO_TEXTSIZE - 1, 0);
			}
		}
	}
}*/

