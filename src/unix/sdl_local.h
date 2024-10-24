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

#pragma once


#define MAX_MONITOR_COUNT 16


typedef struct {
	SDL_Rect		rect;
	int				sdlIndex;
} monitor_t;

typedef struct {
	SDL_Window*		window;
	SDL_GLContext	glContext;

	monitor_t		monitors[MAX_MONITOR_COUNT];
	int				monitorCount;
	int				monitor; // indexes monitors
} glImp_t;

void sdl_Window( const SDL_WindowEvent* event );
qbool sdl_Version_AtLeast( int major, int minor, int patch );

extern glImp_t glimp;

