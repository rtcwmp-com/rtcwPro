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
#include "../qcommon/threads.h"

// tjw: handle for libpthread.so
void* g_pthreads = NULL;

// tjw: pointer to pthread_create() from libpthread.so
static int (*g_pthread_create)
(pthread_t*,
	__const pthread_attr_t*,
	void* (*)(void*),
	void*) = NULL;

/*
===============
Threads_Init
===============
*/
void Threads_Init(void) {
	Com_Printf("------ Initializing Threading ------\n");

	if (g_pthreads != NULL) {
		Com_Printf("pthreads already loaded\n");
		return;
	}
	g_pthreads = dlopen(LIBPTHREAD_SO, RTLD_NOW);
	if (g_pthreads == NULL) {
		Com_Printf("could not load libpthread\n%s\n", dlerror());
		return;
	}
	Com_Printf("loaded libpthread\n");

	g_pthread_create = dlsym(g_pthreads, "pthread_create");
	if (g_pthread_create == NULL) {
		Com_Printf("could not locate pthread_create\n%s\n", dlerror());
		return;
	}
	Com_Printf("found pthread_create\n");

	Com_Printf("Threading Initialized.\n");
}

/*
===============
create_thread
===============
*/
int create_thread(void* (*thread_function)(void*), void* arguments) {
	pthread_t thread_id;

	if (g_pthread_create == NULL) {
		// tjw: pthread_create() returns non 0 for failure
		//      but I don't know what's proper here.
		return -1;
	}

	Com_DPrintf("Thread created.\n");
	return g_pthread_create(&thread_id, NULL, thread_function, arguments);
}