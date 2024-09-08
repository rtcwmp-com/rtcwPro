/*
===========================================================================
Parts taken from CNQ3:
Copyright (C) 2017-2019 Gian 'myT' Schellenbaum

This file is part of RtcwPro.

RtcwPro is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

RtcwPro is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RtcwPro. If not, see <https://www.gnu.org/licenses/>.
===========================================================================
*/

#include "linux_local.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include "sdl_local.h"

static qbool 	sdl_inputActive	= qfalse;
static int		sdl_focusTime	= INT_MIN;	// timestamp of last X11 FocusIn event
static qbool	sdl_focused		= qtrue;	// does the X11 window have the focus?

static int QuakeKeyFromSDLKey( SDL_Keysym key )
{
	if (key.scancode == SDL_SCANCODE_GRAVE)
		return '`';

	const SDL_Keycode sym = key.sym;

	// these ranges map directly to ASCII chars
	if ((sym >= SDLK_a && sym <= SDLK_z) ||
		(sym >= SDLK_0 && sym <= SDLK_9))
		return (int)sym;

	// F1 to F24
	// SDL splits the values 1-12 and 13-24
	// the engine splits the values 1-15 and 16-24

	switch (sym) {
		case SDLK_F1: return K_F1;
		case SDLK_F2: return K_F2;
		case SDLK_F3: return K_F3;
		case SDLK_F4: return K_F4;
		case SDLK_F5: return K_F5;
		case SDLK_F6: return K_F6;
		case SDLK_F7: return K_F7;
		case SDLK_F8: return K_F8;
		case SDLK_F9: return K_F9;
		case SDLK_F10: return K_F10;
		case SDLK_F11: return K_F11;
		case SDLK_F12: return K_F12;
		case SDLK_F13: return K_F13;
		case SDLK_F14: return K_F14;
		case SDLK_F15: return K_F15;
		/*case SDLK_F16: return K_F16;
		case SDLK_F17: return K_F17;
		case SDLK_F18: return K_F18;
		case SDLK_F19: return K_F19;
		case SDLK_F20: return K_F20;
		case SDLK_F21: return K_F21;
		case SDLK_F22: return K_F22;
		case SDLK_F23: return K_F23;
		case SDLK_F24: return K_F24;*/
		case SDLK_UP: return K_UPARROW;
		case SDLK_DOWN: return K_DOWNARROW;
		case SDLK_LEFT: return K_LEFTARROW;
		case SDLK_RIGHT: return K_RIGHTARROW;
		case SDLK_TAB: return K_TAB;
		case SDLK_RETURN: return K_ENTER;
		case SDLK_ESCAPE: return K_ESCAPE;
		case SDLK_SPACE: return K_SPACE;
		case SDLK_BACKSPACE: return K_BACKSPACE;
		case SDLK_CAPSLOCK: return K_CAPSLOCK;
		case SDLK_LALT: return K_ALT;
		case SDLK_RALT: return K_ALT;
		case SDLK_LCTRL: return K_CTRL;
		case SDLK_RCTRL: return K_CTRL;
		case SDLK_LSHIFT: return K_SHIFT;
		case SDLK_RSHIFT: return K_SHIFT;
		case SDLK_INSERT: return K_INS;
		case SDLK_DELETE: return K_DEL;
		case SDLK_PAGEDOWN: return K_PGDN;
		case SDLK_PAGEUP: return K_PGUP;
		case SDLK_HOME: return K_HOME;
		case SDLK_END: return K_END;
		case SDLK_KP_7: return K_KP_HOME;
		case SDLK_KP_8: return K_KP_UPARROW;
		case SDLK_KP_9: return K_KP_PGUP;
		case SDLK_KP_4: return K_KP_LEFTARROW;
		case SDLK_KP_5: return K_KP_5;
		case SDLK_KP_6: return K_KP_RIGHTARROW;
		case SDLK_KP_1: return K_KP_END;
		case SDLK_KP_2: return K_KP_DOWNARROW;
		case SDLK_KP_3: return K_KP_PGDN;
		case SDLK_KP_ENTER: return K_KP_ENTER;
		case SDLK_KP_0: return K_KP_INS;
		case SDLK_KP_DECIMAL: return K_KP_DEL;
		case SDLK_KP_DIVIDE: return K_KP_SLASH;
		case SDLK_KP_MINUS: return K_KP_MINUS;
		case SDLK_KP_PLUS: return K_KP_PLUS;
		case SDLK_KP_MULTIPLY: return K_KP_STAR;
		//case SDLK_BACKSLASH: return K_BACKSLASH;
		case SDLK_PAUSE: return K_PAUSE;
		case SDLK_NUMLOCKCLEAR: return K_KP_NUMLOCK;
		case SDLK_KP_EQUALS: return K_KP_EQUALS;
		//case SDLK_MENU: return K_MENU;
		case SDLK_PERIOD: return '.';
		case SDLK_COMMA: return ',';
		case SDLK_EXCLAIM: return '!';
		case SDLK_HASH: return '#';
		case SDLK_PERCENT: return '%';
		case SDLK_DOLLAR: return '$';
		case SDLK_AMPERSAND: return '&';
		case SDLK_QUOTE: return '\'';
		case SDLK_LEFTPAREN: return '(';
		case SDLK_RIGHTPAREN: return ')';
		case SDLK_ASTERISK: return '*';
		case SDLK_PLUS: return '+';
		case SDLK_MINUS: return '-';
		case SDLK_SLASH: return '/';
		case SDLK_COLON: return ':';
		case SDLK_LESS: return '<';
		case SDLK_EQUALS: return '=';
		case SDLK_GREATER: return '>';
		case SDLK_QUESTION: return '?';
		case SDLK_AT: return '@';
		case SDLK_LEFTBRACKET: return '[';
		case SDLK_RIGHTBRACKET: return ']';
		case SDLK_UNDERSCORE: return '_';
		case SDLK_SEMICOLON: return ';';
		// not handled:
		// K_COMMAND (Apple)
		// K_POWER (Apple)
		// K_AUX1-16
		// K_WIN
		default: break;
	}

	if (sym >= 32 && sym <= 126)
		return (int)sym;

	return -1;
}



static void sdl_X11( const XEvent* event )
{
	switch (event->type) {
		case FocusIn:
			// see in_focusDelay explanation at the top
			sdl_focusTime = Sys_Milliseconds();
			sdl_focused = qtrue;
			break;

		case FocusOut:
			// set modifier keys as released to prevent
			// accidental combos such alt+enter right after
			// getting focus
			// e.g. alt gets "stuck", pressing only enter
			// does a video restart as if pressing alt+enter
			Sys_QueEvent(0, SE_KEY, K_ALT,   qfalse, 0, NULL);
			Sys_QueEvent(0, SE_KEY, K_CTRL,  qfalse, 0, NULL);
			Sys_QueEvent(0, SE_KEY, K_SHIFT, qfalse, 0, NULL);
			sdl_focused = qfalse;
			break;

		default:
			break;
	}
}

static void sdl_Key( const SDL_KeyboardEvent* event, qbool down )
{
	const int key = QuakeKeyFromSDLKey(event->keysym);
	if (key >= 0)
		Sys_QueEvent(Sys_Milliseconds(), SE_KEY, key, down, 0, NULL);

	if (down && key == K_BACKSPACE)
		Sys_QueEvent(Sys_Milliseconds(), SE_CHAR, 8, 0, 0, NULL);

	// ctrl+v
	if (down && key == 'v' && (event->keysym.mod & KMOD_CTRL) != 0)
		Sys_QueEvent(Sys_Milliseconds(), SE_CHAR, 22, 0, 0, NULL);
}


static void sdl_Text( const SDL_TextInputEvent* event )
{
	// text is UTF-8 encoded but we only care for
	// chars that are single-byte encoded
	const byte key = (byte)event->text[0];
	if (key >= 0 && key <= 0x7F)
		Sys_QueEvent(Sys_Milliseconds(), SE_CHAR, (int)key, 0, 0, NULL);
}


static void sdl_MouseMotion( const SDL_MouseMotionEvent* event )
{
	if (!sdl_inputActive)
		return;

	// SDL sometimes sends events with both values set to 0
	if ((event->xrel | event->yrel) == 0)
		return;

	Sys_QueEvent(Sys_Milliseconds(), SE_MOUSE, event->xrel, event->yrel, 0, NULL);
}


static void sdl_MouseButton( const SDL_MouseButtonEvent* event, qbool down )
{
	if (!sdl_inputActive && down)
		return;

	static const int mouseButtonCount = 5;
	static const int mouseButtons[5][2] = {
		{ SDL_BUTTON_LEFT, K_MOUSE1 },
		{ SDL_BUTTON_RIGHT, K_MOUSE2 },
		{ SDL_BUTTON_MIDDLE, K_MOUSE3 },
		{ SDL_BUTTON_X1, K_MOUSE4 },
		{ SDL_BUTTON_X2, K_MOUSE5 }
	};

	int button = -1;
	for(int i = 0; i < mouseButtonCount; ++i) {
		if (event->button == mouseButtons[i][0]) {
			button = i;
			break;
		}
	}

	if (button < 0)
		return;

	Sys_QueEvent(Sys_Milliseconds(), SE_KEY, mouseButtons[button][1], down, 0, NULL);
}


static void sdl_MouseWheel( const SDL_MouseWheelEvent* event )
{
	if (event->y == 0)
		return;

#if SDL_VERSION_ATLEAST(2, 0, 4)
	int delta = event->y;
	if (sdl_Version_AtLeast(2, 0, 4) &&
		event->direction == SDL_MOUSEWHEEL_FLIPPED)
		delta = -delta;
#else
	const int delta = event->y;
#endif

	const int key = (delta < 0) ? K_MWHEELDOWN : K_MWHEELUP;
	Sys_QueEvent(Sys_Milliseconds(), SE_KEY, key, qtrue,  0, NULL);
	Sys_QueEvent(Sys_Milliseconds(), SE_KEY, key, qfalse, 0, NULL);
}

static void sdl_Event( const SDL_Event* event )
{
	// Note that CVar checks are necessary here because event polling
	// can actually start before the main loop does,
	// i.e. CVars can be uninitialized by the time we get here.

	switch (event->type) {
	case SDL_QUIT:
		Com_Quit(0);
		break;

	case SDL_KEYDOWN:
		// the CVar check means we'll ignore all keydown events until the main loop starts
		//if (in_focusDelay != NULL && sdl_focused && Sys_Milliseconds() - sdl_focusTime >= in_focusDelay->integer)
			sdl_Key(&event->key, qtrue);
		break;

	case SDL_KEYUP:
		// always forward releases
		sdl_Key(&event->key, qfalse);
		break;

	case SDL_TEXTINPUT:
		if (sdl_focused)
			sdl_Text(&event->text);
		break;

	case SDL_MOUSEMOTION:
		if (sdl_focused)
			sdl_MouseMotion(&event->motion);
		break;

	case SDL_MOUSEBUTTONDOWN:
		if (sdl_focused)
			sdl_MouseButton(&event->button, qtrue);
		break;

	case SDL_MOUSEBUTTONUP:
		// always forward releases
		sdl_MouseButton(&event->button, qfalse);
		break;

	case SDL_MOUSEWHEEL:
		if (sdl_focused)
			sdl_MouseWheel(&event->wheel);
		break;

	case SDL_WINDOWEVENT:
		sdl_Window(&event->window);
		break;

	case SDL_SYSWMEVENT:
		{
			const SDL_SysWMmsg* msg = event->syswm.msg;
			if (msg->subsystem == SDL_SYSWM_X11)
				sdl_X11(&msg->msg.x11.event);
		}
		break;

	default:
		break;
	}
}

static qbool sdl_IsInputActive()
{
	const qbool hasFocus = (SDL_GetWindowFlags(glimp.window) & SDL_WINDOW_INPUT_FOCUS) != 0;
	if (!hasFocus)
		return qfalse;

	return qtrue;
}

void sdl_PollEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
		sdl_Event(&event);
}


void sdl_Frame()
{
	sdl_inputActive = sdl_IsInputActive();
	sdl_PollEvents();

	//SDL_SetRelativeMouseMode((sdl_inputActive && m_relative->integer) ? SDL_TRUE : SDL_FALSE);
	SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_SetWindowGrab(glimp.window, sdl_inputActive ? SDL_TRUE : SDL_FALSE);
	SDL_ShowCursor(sdl_inputActive ? SDL_DISABLE : SDL_ENABLE);
	// @NOTE: SDL_WarpMouseInWindow generates a motion event

	//S_Frame();
}


void IN_Init( void ) {
}

void IN_Shutdown( void ) {
}

void IN_Frame( void ) {
}

void IN_Activate( void ) {
}