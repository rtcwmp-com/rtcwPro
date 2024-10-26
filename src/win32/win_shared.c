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


#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "win_local.h"
#include <lmerr.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

/*
================
Sys_Milliseconds
================
*/
int sys_timeBase;
int Sys_Milliseconds( void ) {
	int sys_curtime;
	static qboolean initialized = qfalse;

	if ( !initialized ) {
		sys_timeBase = timeGetTime();
		initialized = qtrue;
	}
	sys_curtime = timeGetTime() - sys_timeBase;

	return sys_curtime;
}

void Sys_Sleep(int ms)
{
	if (ms >= 1)
		Sleep(ms);
}

void Sys_MicroSleep(int us)
{
	if (us <= 50)
		return;

	us -= 50;

	LARGE_INTEGER frequency;
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);
	QueryPerformanceFrequency(&frequency);
	endTime.QuadPart += ((LONGLONG)us * frequency.QuadPart) / 1000000LL;

	// reminder: we call timeBeginPeriod(1) at init
	// Sleep(1) will generally last 1000-2000 us,
	// but in some cases quite a bit more (I've seen up to 3500 us)
	// because threads can take longer to wake up
	const LONGLONG thresholdUS = (LONGLONG)Cvar_Get("r_sleepThreshold", "2500", CVAR_ARCHIVE)->integer;
	const LONGLONG bigSleepTicks = (thresholdUS * frequency.QuadPart) / 1000000LL;

	for (;;) {
		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);
		const LONGLONG remainingTicks = endTime.QuadPart - currentTime.QuadPart;
		if (remainingTicks <= 0) {
			break;
		}
		if (remainingTicks >= bigSleepTicks) {
			Sleep(1);
		}
		else {
			YieldProcessor();
		}
	}
}

int Sys_GetUptimeSeconds(qbool parent)
{
	if (parent)
		return -1;

	FILETIME startFileTime;
	FILETIME trash[3];
	if (GetProcessTimes(GetCurrentProcess(), &startFileTime, &trash[0], &trash[1], &trash[2]) == 0)
		return -1;

	SYSTEMTIME endSystemTime;
	GetSystemTime(&endSystemTime);

	FILETIME endFileTime;
	if (SystemTimeToFileTime(&endSystemTime, &endFileTime) == 0)
		return -1;

	// 1 FILETIME unit is 100-nanoseconds
	ULARGE_INTEGER start, end;
	start.LowPart = startFileTime.dwLowDateTime;
	start.HighPart = startFileTime.dwHighDateTime;
	end.LowPart = endFileTime.dwLowDateTime;
	end.HighPart = endFileTime.dwHighDateTime;
	const int seconds = (int)((end.QuadPart - start.QuadPart) / 1e7);

	return seconds;
}

qbool Sys_HardReboot()
{
	return qfalse;
}


qbool Sys_HasRtcwProParent()
{
	return qfalse;
}

/*
================
Sys_SnapVector
================
*/
long fastftol( float f ) {
	return (long)f;
}

void Sys_SnapVector( float *v ) {
//unused
}



//============================================

char *Sys_GetCurrentUser( void ) {
	static char s_userName[1024];
	unsigned long size = sizeof( s_userName );


	if ( !GetUserName( s_userName, &size ) ) {
		strcpy( s_userName, "player" );
	}

	if ( !s_userName[0] ) {
		strcpy( s_userName, "player" );
	}

	return s_userName;
}

qbool Sys_IsDebuggerAttached()
{
	return IsDebuggerPresent();
}

void Sys_Crash(const char* message, const char* file, int line, const char* function)
{
	const ULONG_PTR args[4] = { (ULONG_PTR)message, (ULONG_PTR)file, (ULONG_PTR)line, (ULONG_PTR)function };
	RaiseException(RTCWPRO_WINDOWS_EXCEPTION_CODE, EXCEPTION_NONCONTINUABLE, ARRAY_LEN(args), args);
}

char* Sys_GetScreenshotPath(char* filename){
	char* basepath = Cvar_VariableString("fs_basepath");
	char* gamepath = Cvar_VariableString("fs_game");

	return va("%s/%s/screenshots/%s.jpg", basepath, gamepath, filename);
}