﻿/*
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
#include "client.h"

/*
====================
CL_SetGuid
====================
*/

void CL_SetGuid(void) {
	Com_ReadAuthKey(BASEGAME);
}

/*
====================
CL_SetMotd
====================
*/
void CL_SetMotd(char* message) {
	if (message != NULL) {
		Cvar_Set("cl_motdString", message);
	}
}

/*
====================
CL_ClientNeedsUpdate
====================
*/
void CL_ClientNeedsUpdate(char* response) {
	if (response != NULL) {
		Cmd_TokenizeString(response);
		if (!Q_stricmp(Cmd_Argv(0), "updtAvailable")) {
			Com_Error(ERR_FATAL, Cmd_ArgsFrom(1));
		}
	}
}
