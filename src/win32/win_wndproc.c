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


#include "../client/client.h"
#include "win_local.h"
#include "glw_win.h"

WinVars_t g_wv;

static HHOOK WinHook;

/*
==================
WinKeyHook
==================
*/
static LRESULT CALLBACK WinKeyHook(int code, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT)lParam;
	switch (wParam)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if ((key->vkCode == VK_LWIN || key->vkCode == VK_RWIN) && !(Key_GetCatcher() & KEYCATCH_CONSOLE)) {
			Sys_QueEvent(0, SE_KEY, K_SUPER, qtrue, 0, NULL);
			return 1;
		}
		if (key->vkCode == VK_SNAPSHOT) {
			Sys_QueEvent(0, SE_KEY, K_PRINT, qtrue, 0, NULL);
			return 1;
		}
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if ((key->vkCode == VK_LWIN || key->vkCode == VK_RWIN) && !(Key_GetCatcher() & KEYCATCH_CONSOLE)) {
			Sys_QueEvent(0, SE_KEY, K_SUPER, qfalse, 0, NULL);
			return 1;
		}
		if (key->vkCode == VK_SNAPSHOT) {
			Sys_QueEvent(0, SE_KEY, K_PRINT, qfalse, 0, NULL);
			return 1;
		}
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
}

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL ( WM_MOUSELAST + 1 )  // message that will be supported by the OS
#endif

static UINT MSH_MOUSEWHEEL;

// Console variables that we need to access from this module
cvar_t      *vid_xpos;          // X coordinate of window position
cvar_t      *vid_ypos;          // Y coordinate of window position
cvar_t      *r_fullscreen;

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

LONG WINAPI MainWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

static qboolean s_alttab_disabled;

static void WIN_DisableAltTab( void ) {
	if ( s_alttab_disabled ) {
		return;
	}

	if ( !Q_stricmp( Cvar_VariableString( "arch" ), "winnt" ) ) {
		RegisterHotKey( 0, 0, MOD_ALT, VK_TAB );
	} else
	{
		BOOL old;

		SystemParametersInfo( SPI_SCREENSAVERRUNNING, 1, &old, 0 );
	}
	s_alttab_disabled = qtrue;
}

static void WIN_EnableAltTab( void ) {
	if ( s_alttab_disabled ) {
		if ( !Q_stricmp( Cvar_VariableString( "arch" ), "winnt" ) ) {
			UnregisterHotKey( 0, 0 );
		} else
		{
			BOOL old;

			SystemParametersInfo( SPI_SCREENSAVERRUNNING, 0, &old, 0 );
		}

		s_alttab_disabled = qfalse;
	}
}

/*
==================
WIN_DisableHook
==================
*/
void WIN_DisableHook(void)
{
	if (WinHook) {
		UnhookWindowsHookEx(WinHook);
		WinHook = NULL;
	}
}


/*
==================
WIN_EnableHook

Capture PrintScreen and Win* keys
==================
*/
void WIN_EnableHook(void)
{
#ifndef DEDICATED
	if (!WinHook)
	{
		WinHook = SetWindowsHookEx(WH_KEYBOARD_LL, WinKeyHook, g_wv.hInstance, 0);
	}
#endif
}

/*
==================
VID_AppActivate
==================
*/
static void VID_AppActivate(qboolean active)
{
	Key_ClearStates();

	IN_Activate(active);

	if (active) {
		WIN_EnableHook();
		SetWindowPos(g_wv.hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else {
		WIN_DisableHook();
		SetWindowPos(g_wv.hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}


//==========================================================================

static byte s_scantokey[128] =
{
//  0           1       2       3       4       5       6       7
//  8           9       A       B       C       D       E       F
	0,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',    13,    K_CTRL,'a',  's',      // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
	'\'',    '`',    K_SHIFT,'\\',  'z',    'x',    'c',    'v',      // 2
	'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*',
	K_ALT,' ',   K_CAPSLOCK,    K_F1, K_F2, K_F3, K_F4, K_F5,    // 3
	K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0, K_HOME,
	K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW, K_KP_PLUS,K_END, //4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11,
	K_F12,0,    0,    0,    0,    0,    0,    0,                    // 5
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0,                      // 6
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0                       // 7
};

static byte s_scantokey_german[128] =
{
//  0           1       2       3       4       5       6       7
//  8           9       A       B       C       D       E       F
	0,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '?',    '\'',    K_BACKSPACE, 9, // 0
	'q',    'w',    'e',    'r',    't',    'z',    'u',    'i',
	'o',    'p',    '=',    '+',    13,    K_CTRL, 'a',  's',      // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    '[',
	']',    '`',    K_SHIFT,'#',  'y',    'x',    'c',    'v',      // 2
	'b',    'n',    'm',    ',',    '.',    '-',    K_SHIFT,'*',
	K_ALT,' ',   K_CAPSLOCK,    K_F1, K_F2, K_F3, K_F4, K_F5,    // 3
	K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0, K_HOME,
	K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW, K_KP_PLUS,K_END, //4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             '<',              K_F11,
	K_F12,0,    0,    0,    0,    0,    0,    0,                    // 5
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0,                      // 6
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0                       // 7
};

static byte s_scantokey_french[128] =
{
//  0           1       2       3       4       5       6       7
//  8           9       A       B       C       D       E       F
	0,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    ')',    '=',    K_BACKSPACE, 9, // 0
	'a',    'z',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '^',    '$',    13,    K_CTRL, 'q',  's',      // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    'm',
	'%',    '`',    K_SHIFT,'*',  'w',    'x',    'c',    'v',      // 2
	'b',    'n',    ',',    ';',    ':',    '!',    K_SHIFT,'*',
	K_ALT,' ',   K_CAPSLOCK,    K_F1, K_F2, K_F3, K_F4, K_F5,    // 3
	K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0, K_HOME,
	K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW, K_KP_PLUS,K_END, //4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             '<',              K_F11,
	K_F12,0,    0,    0,    0,    0,    0,    0,                    // 5
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0,                      // 6
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0                       // 7
};

static byte s_scantokey_spanish[128] =
{
//  0           1       2       3       4       5       6       7
//  8           9       A       B       C       D       E       F
	0,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '\'',    '!',    K_BACKSPACE, 9, // 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',    13,    K_CTRL, 'a',  's',      // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    '=',
	'{',    '`',    K_SHIFT,'}',  'z',    'x',    'c',    'v',      // 2
	'b',    'n',    'm',    ',',    '.',    '-',    K_SHIFT,'*',
	K_ALT,' ',   K_CAPSLOCK,    K_F1, K_F2, K_F3, K_F4, K_F5,    // 3
	K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0, K_HOME,
	K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW, K_KP_PLUS,K_END, //4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             '<',              K_F11,
	K_F12,0,    0,    0,    0,    0,    0,    0,                    // 5
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0,                      // 6
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0                       // 7
};

static byte s_scantokey_italian[128] =
{
//  0           1       2       3       4       5       6       7
//  8           9       A       B       C       D       E       F
	0,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '\'',    '^',    K_BACKSPACE, 9, // 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',    13,    K_CTRL, 'a',  's',      // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    '@',
	'#',    '`',    K_SHIFT,'=',  'z',    'x',    'c',    'v',      // 2
	'b',    'n',    'm',    ',',    '.',    '-',    K_SHIFT,'*',
	K_ALT,' ',   K_CAPSLOCK,    K_F1, K_F2, K_F3, K_F4, K_F5,    // 3
	K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0, K_HOME,
	K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW, K_KP_PLUS,K_END, //4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             '<',              K_F11,
	K_F12,0,    0,    0,    0,    0,    0,    0,                    // 5
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0,                      // 6
	0,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0                       // 7
};
/*
=======
MapKey

Map from windows to quake keynums
=======
*/
static int MapKey( int key ) {
	int result;
	int modified;
	qboolean is_extended;

//	Com_Printf( "0x%x\n", key);

	modified = ( key >> 16 ) & 255;

	if ( modified > 127 ) {
		return 0;
	}

	if ( key & ( 1 << 24 ) ) {
		is_extended = qtrue;
	} else
	{
		is_extended = qfalse;
	}

	result = s_scantokey[modified];

	if ( cl_language->integer - 1 == LANGUAGE_FRENCH ) {
		result = s_scantokey_french[modified];
	} else if ( cl_language->integer - 1 == LANGUAGE_GERMAN ) {
		result = s_scantokey_german[modified];
	} else if ( cl_language->integer - 1 == LANGUAGE_ITALIAN ) {
		result = s_scantokey_italian[modified];
	} else if ( cl_language->integer - 1 == LANGUAGE_SPANISH ) {
		result = s_scantokey_spanish[modified];
	}

	if ( !is_extended ) {
		switch ( result )
		{
		case K_HOME:
			return K_KP_HOME;
		case K_UPARROW:
			return K_KP_UPARROW;
		case K_PGUP:
			return K_KP_PGUP;
		case K_LEFTARROW:
			return K_KP_LEFTARROW;
		case K_RIGHTARROW:
			return K_KP_RIGHTARROW;
		case K_END:
			return K_KP_END;
		case K_DOWNARROW:
			return K_KP_DOWNARROW;
		case K_PGDN:
			return K_KP_PGDN;
		case K_INS:
			return K_KP_INS;
		case K_DEL:
			return K_KP_DEL;
		default:
			return result;
		}
	} else
	{
		switch ( result )
		{
		case K_PAUSE:
			return K_KP_NUMLOCK;
		case 0x0D:
			return K_KP_ENTER;
		case 0x2F:
			return K_KP_SLASH;
		case 0xAF:
			return K_KP_PLUS;
		}
		return result;
	}
}

extern cvar_t* in_mouse;
extern cvar_t* in_logitechbug;

int			HotKey = 0;
int			hkinstalled = 0;

extern void SetGameDisplaySettings(void);
extern void SetDesktopDisplaySettings(void);

void Win_AddHotkey(void)
{
	UINT modifiers, vk;
	ATOM atom;

	if (!HotKey || !g_wv.hWnd || hkinstalled)
		return;

	modifiers = 0;

	if (HotKey & HK_MOD_ALT)		modifiers |= MOD_ALT;
	if (HotKey & HK_MOD_CONTROL)	modifiers |= MOD_CONTROL;
	if (HotKey & HK_MOD_SHIFT)	modifiers |= MOD_SHIFT;
	if (HotKey & HK_MOD_WIN)		modifiers |= MOD_WIN;

	vk = HotKey & 0xFF;

	atom = GlobalAddAtom(TEXT("ETMinimizeHotkey"));
	if (!RegisterHotKey(g_wv.hWnd, atom, modifiers, vk)) {
		GlobalDeleteAtom(atom);
		return;
	}
	hkinstalled = 1;
}


void Win_RemoveHotkey(void)
{
	ATOM atom;

	if (!g_wv.hWnd || !hkinstalled)
		return;

	atom = GlobalFindAtom(TEXT("ETMinimizeHotkey"));
	if (atom) {
		UnregisterHotKey(g_wv.hWnd, atom);
		GlobalDeleteAtom(atom);
		hkinstalled = 0;
	}
}


BOOL Win_CheckHotkeyMod(void) {

	if (!(HotKey & HK_MOD_XMASK))
		return TRUE;

	if ((HotKey & HK_MOD_LALT) && !GetAsyncKeyState(VK_LMENU)) return FALSE;
	if ((HotKey & HK_MOD_RALT) && !GetAsyncKeyState(VK_RMENU)) return FALSE;
	if ((HotKey & HK_MOD_LSHIFT) && !GetAsyncKeyState(VK_LSHIFT)) return FALSE;
	if ((HotKey & HK_MOD_RSHIFT) && !GetAsyncKeyState(VK_RSHIFT)) return FALSE;
	if ((HotKey & HK_MOD_LCONTROL) && !GetAsyncKeyState(VK_LCONTROL)) return FALSE;
	if ((HotKey & HK_MOD_RCONTROL) && !GetAsyncKeyState(VK_RCONTROL)) return FALSE;
	if ((HotKey & HK_MOD_LWIN) && !GetAsyncKeyState(VK_LWIN)) return FALSE;
	if ((HotKey & HK_MOD_RWIN) && !GetAsyncKeyState(VK_RWIN)) return FALSE;

	return TRUE;
}


#define TIMER_M 11
#define TIMER_T 12
static UINT uTimerM;
static UINT uTimerT;

void WIN_Minimize(void) {
	static int minimize = 0;

	if (minimize)
		return;

	minimize = 1;

#ifdef FAST_MODE_SWITCH
	// move game window to background
	if (glw_state.cdsFullscreen) {
		if (gw_active)
			SetForegroundWindow(GetDesktopWindow());
		// and wait some time before minimizing
		if (!uTimerM)
			uTimerM = SetTimer(g_wv.hWnd, TIMER_M, 50, NULL);
	}
	else {
		ShowWindow(g_wv.hWnd, SW_MINIMIZE);
	}
#else
	ShowWindow(g_wv.hWnd, SW_MINIMIZE);
#endif

	minimize = 0;
}

/*
====================
MainWndProc

main window procedure
====================
*/
LONG WINAPI MainWndProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam ) {

	static qboolean flip = qtrue;
	static qboolean focused = qfalse;
	qboolean active;
	qboolean minimized;

	if ( uMsg == MSH_MOUSEWHEEL ) {
		if ( ( ( int ) wParam ) > 0 ) {
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qtrue, 0, NULL );
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qfalse, 0, NULL );
		} else
		{
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL );
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL );
		}
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	switch ( uMsg )
	{
		// RTCWPro - raw input
#ifndef DEDICATED
	case WM_INPUT:
		IN_RawInput_MouseRead((HANDLE)lParam);
		break;
#endif
		// raw input end
	case WM_MOUSEWHEEL:
		//
		//
		// this chunk of code theoretically only works under NT4 and Win98
		// since this message doesn't exist under Win95
		//
		if ( ( short ) HIWORD( wParam ) > 0 ) {
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qtrue, 0, NULL );
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qfalse, 0, NULL );
		} else
		{
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL );
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL );
		}
		break;

	case WM_CREATE:

		g_wv.hWnd = hWnd;

		vid_xpos = Cvar_Get( "vid_xpos", "3", CVAR_ARCHIVE );
		vid_ypos = Cvar_Get( "vid_ypos", "22", CVAR_ARCHIVE );
		r_fullscreen = Cvar_Get( "r_fullscreen", "1", CVAR_ARCHIVE | CVAR_LATCH );

		MSH_MOUSEWHEEL = RegisterWindowMessage( "MSWHEEL_ROLLMSG" );
		if ( r_fullscreen->integer ) {
			WIN_DisableAltTab();
		} else
		{
			WIN_EnableAltTab();
		}

		break;
#if 0
	case WM_DISPLAYCHANGE:
		Com_DPrintf( "WM_DISPLAYCHANGE\n" );
		// we need to force a vid_restart if the user has changed
		// their desktop resolution while the game is running,
		// but don't do anything if the message is a result of
		// our own calling of ChangeDisplaySettings
		if ( com_insideVidInit ) {
			break;      // we did this on purpose
		}
		// something else forced a mode change, so restart all our gl stuff
		Cbuf_AddText( "vid_restart\n" );
		break;
#endif
	case WM_DESTROY:
		// let sound and input know about this?
		g_wv.hWnd = NULL;
		if ( r_fullscreen->integer ) {
			WIN_EnableAltTab();
		}
		break;

	case WM_CLOSE:
		Cbuf_ExecuteText( EXEC_APPEND, "quit" );
		break;

	case WM_ACTIVATE:
		active = (LOWORD(wParam) != WA_INACTIVE) ? qtrue : qfalse;
		minimized = (BOOL)HIWORD(wParam) ? qtrue : qfalse;

		// We can receive Active & Minimized when restoring from minimized state
		if (active && minimized) {
			gw_minimized = qtrue;
			break;
		}

		gw_active = active;
		gw_minimized = minimized;

		VID_AppActivate(gw_active);
		Win_AddHotkey();

		if (glw_state.cdsFullscreen) {
			if (gw_active) {
				SetGameDisplaySettings();
				if (re.SetColorMappings)
					re.SetColorMappings();
			}
			else {
				// don't restore gamma if we have multiple monitors
				if (glw_state.monitorCount <= 1 || gw_minimized)
					GLW_RestoreGamma();
				// minimize if there is only one monitor
				if (glw_state.monitorCount <= 1) {
					//if (CL_VideoRecording() == AVIDEMO_NONE || (re.CanMinimize && re.CanMinimize())) {
					if (re.CanMinimize&& re.CanMinimize()) {
						if (!gw_minimized) {
							WIN_Minimize();
						}
						SetDesktopDisplaySettings();
					}
				}
			}
		}
		else {
			if (gw_active) {
				if (re.SetColorMappings)
					re.SetColorMappings();
			}
			else {
				GLW_RestoreGamma();
			}
		}

		// after ALT+TAB, even if we selected other window we may receive WM_ACTIVATE 1 and then WM_ACTIVATE 0
		// if we set HWND_TOPMOST in VID_AppActivate() other window will be not visible despite obtained input focus
		// so delay HWND_TOPMOST setup to make sure we have no such bogus activation
		if (gw_active && glw_state.cdsFullscreen) {
			if (uTimerT) {
				KillTimer(g_wv.hWnd, uTimerT);
			}
			uTimerT = SetTimer(g_wv.hWnd, TIMER_T, 20, NULL);
		}

		SNDDMA_Activate();
		break;

	case WM_SETFOCUS:
		focused = qtrue;
		break;

	case WM_KILLFOCUS:
		//gw_active = qfalse;
		focused = qfalse;
		break;

	case WM_MOVE:
	{
		if (!gw_active || gw_minimized || !focused)
			break;

		GetWindowRect(hWnd, &g_wv.winRect);
		g_wv.winRectValid = qtrue;
		UpdateMonitorInfo(&g_wv.winRect);
		IN_UpdateWindow(NULL, qtrue);
		IN_Activate(gw_active);

		if (!glw_state.cdsFullscreen) {
			Cvar_SetIntegerValue("vid_xpos", g_wv.winRect.left);
			Cvar_SetIntegerValue("vid_ypos", g_wv.winRect.top);
			vid_xpos->modified = qfalse;
			vid_ypos->modified = qfalse;
		}
		break;
	}
	break;

	// rtcwpro - borderless window - snap edges to screen
	case WM_WINDOWPOSCHANGING:
	{
		WINDOWPLACEMENT wp;

		// set minimized flag as early as possible
		Com_Memset(&wp, 0, sizeof(wp));
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(hWnd, &wp) && wp.showCmd == SW_SHOWMINIMIZED)
			gw_minimized = qtrue;

		if (g_wv.borderless)
		{
			WINDOWPOS* pos = (LPWINDOWPOS)lParam;
			const int threshold = 10;
			HMONITOR hMonitor;
			MONITORINFO mi;
			const RECT* r;
			RECT rr;

			rr.left = pos->x;
			rr.right = pos->x + pos->cx;
			rr.top = pos->y;
			rr.bottom = pos->y + pos->cy;
			hMonitor = MonitorFromRect(&rr, MONITOR_DEFAULTTONEAREST);

			if (hMonitor)
			{
				mi.cbSize = sizeof(mi);
				GetMonitorInfo(hMonitor, &mi);
				r = &mi.rcWork;

				// snap window to current monitor borders
				if (pos->x >= (r->left - threshold) && pos->x <= (r->left + threshold))
					pos->x = r->left;
				else if ((pos->x + pos->cx) >= (r->right - threshold) && (pos->x + pos->cx) <= (r->right + threshold))
					pos->x = (r->right - pos->cx);

				if (pos->y >= (r->top - threshold) && pos->y <= (r->top + threshold))
					pos->y = r->top;
				else if ((pos->y + pos->cy) >= (r->bottom - threshold) && (pos->y + pos->cy) <= (r->bottom + threshold))
					pos->y = (r->bottom - pos->cy);

				return 0;
			}
		}
	}
	break;

// this is complicated because Win32 seems to pack multiple mouse events into
// one update sometimes, so we always check all states and look for events
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
	{
		int temp;

		temp = 0;

		if ( wParam & MK_LBUTTON ) {
			temp |= 1;
		}

		if ( wParam & MK_RBUTTON ) {
			temp |= 2;
		}

		if ( wParam & MK_MBUTTON ) {
			temp |= 4;
		}

		// RTCWPro - extra buttons
		if (wParam & MK_XBUTTON1) {
			temp |= 8;
		}

		if (wParam & MK_XBUTTON2) {
			temp |= 16;
		}
		// RTCWPro end

		IN_MouseEvent( temp );
	}
	break;

	case WM_SYSCOMMAND:
		if ( wParam == SC_SCREENSAVE ) {
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		if ( wParam == 13 ) {
			if ( r_fullscreen ) {
				Cvar_SetValue( "r_fullscreen", !r_fullscreen->integer );
				Cbuf_AddText( "vid_restart\n" );
			}
			return 0;
		}
		// fall through
	case WM_KEYDOWN:
		Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, MapKey( lParam ), qtrue, 0, NULL );
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP:
		Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, MapKey( lParam ), qfalse, 0, NULL );
		break;

	case WM_CHAR:
		Sys_QueEvent( g_wv.sysMsgTime, SE_CHAR, wParam, 0, 0, NULL );
		break;

		// rtcwpro - borderless window - move window with mouse while holding ctrl
	case WM_NCHITTEST:
		// in borderless mode - drag using client area when holding ALT
		if (g_wv.borderless && GetKeyState(VK_MENU) & (1 << 15))
			return HTCAPTION;
		break;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

/*
================
HandleEvents
================
*/
void HandleEvents(void) {
	MSG msg;

	// pump the message loop
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (GetMessage(&msg, NULL, 0, 0) <= 0) {
			Cmd_Clear();
			Com_Quit_f();
		}

		// save the msg time, because wndprocs don't have access to the timestamp
		//g_wv.sysMsgTime = msg.time;
		g_wv.sysMsgTime = Sys_Milliseconds();

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}