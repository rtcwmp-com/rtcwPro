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
#include "http.h"

/*
===============
HTTP_AuthClient

Will either return Guid or error message.
===============
*/
char* HTTP_AuthClient(char* guid) {
	//return HTTP_Post(WEB_GET_AUTH, guid);
	return "";
}

/*
===============
HTTP_ClientNeedsUpdate

Checks if client needs a update.
===============
*/
void HTTP_ClientNeedsUpdate(void) {
	HTTP_Inquiry_t* http_inquiry = (HTTP_Inquiry_t*)malloc(sizeof(HTTP_Inquiry_t));

	if (http_inquiry) {
		http_inquiry->url = va("%s", WEB_GET_UPDATE);
		http_inquiry->param = va("%s", CODENAME);
		http_inquiry->callback = CL_ClientNeedsUpdate;

		Threads_Create(HTTP_Get, http_inquiry);
	}
	return;
}

/*
===============
HTTP_ClientGetMOTD

Sets a MOTD if there's any..
===============
*/
void HTTP_ClientGetMOTD(void) {
	HTTP_Inquiry_t* http_inquiry = (HTTP_Inquiry_t*)malloc(sizeof(HTTP_Inquiry_t));

	if (http_inquiry) {
		http_inquiry->url = va("%s", WEB_GET_MOTD);
		http_inquiry->param = va("%s", CODENAME);
		http_inquiry->callback = CL_SetMotd;

		Threads_Create(HTTP_Get, http_inquiry);
	}
	return;
}
