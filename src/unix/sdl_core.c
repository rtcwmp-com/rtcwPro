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
#include "sdl_local.h"


qbool sdl_Version_AtLeast( int major, int minor, int patch )
{
	SDL_version v;
	SDL_GetVersion(&v);

	// has to be SDL 2
	if (v.major != major)
		return qfalse;

	if (v.minor < minor)
		return qfalse;

	if (v.minor > minor)
		return qtrue;

	return v.patch >= patch;
}

qbool sdl_Init()
{
	atexit(SDL_Quit);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		fprintf(stderr, "Failed to initialize SDL 2: %s\n", SDL_GetError());
		return qfalse;
	}

	SDL_version version;
	SDL_GetVersion(&version);
	printf("Opened SDL %d.%d.%d\n", version.major, version.minor, version.patch);

	// @TODO: investigate/test these?
	// SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH
	// SDL_HINT_MOUSE_RELATIVE_MODE_WARP
#if SDL_VERSION_ATLEAST(2, 0, 4)
	if (sdl_Version_AtLeast(2, 0, 4))
		SDL_SetHintWithPriority(SDL_HINT_NO_SIGNAL_HANDLERS, "1", SDL_HINT_OVERRIDE);
#endif
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_CRITICAL);
	SDL_StartTextInput();	// enables SDL_TEXTINPUT events
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

	return qtrue;
}
