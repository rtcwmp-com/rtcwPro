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

// linux_local.h: Linux-specific Quake3 header file


#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#ifndef DEDICATED
#include "../client/client.h"
#endif

void		Lin_HardRebootHandler(int argc, char** argv);
void		Lin_TrackParentProcess();
void		Lin_ConsoleInputInit();
void		Lin_ConsoleInputShutdown();
const char* Lin_ConsoleInput();

qbool		tty_Enabled();
void		tty_Hide();
history_t* tty_GetHistory();

void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );
qboolean Sys_GetPacket( netadr_t *net_from, msg_t *net_message );
qboolean	Sys_GetStreamedPacket(netadr_t* net_from, msg_t* net_message); // rtcwpro
void Sys_SendKeyEvents( void );

// Input subsystem

void IN_Init( void );
void IN_Frame( void );
void IN_Shutdown( void );


void IN_JoyMove( void );
void IN_StartupJoystick( void );

// GL subsystem
qboolean QGL_Init( const char *dllname );
void QGL_EnableLogging( qboolean enable );
void QGL_Shutdown( void );

char *strlwr( char *s );

// signals.c
void	SIG_InitChild();
void	SIG_InitParent();
void	SIG_Frame();

void sdl_Frame();
void sdl_PollEvents();
qbool sdl_Init();

extern int		q_argc;
extern char** q_argv;