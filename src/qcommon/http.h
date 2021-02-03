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
#ifndef _S_HTTP
#define _S_HTTP

#include <curl/curl.h>
#include <curl/easy.h>
#ifdef DEDICATED
	#include "../server/server.h"
#else
	#include "../client/client.h"
#endif

// URL Mappings
#ifdef _DEBUG
	#define WEB_URL		"http://game.localhost"
#else 
	#define WEB_URL		"https://rtcwmp.com"
#endif // ~_DEBUG
#define WEB_GET_MOTD	WEB_URL "/api/get/motd"
#define WEB_GET_UPDATE	WEB_URL "/api/get/update"
#define WEB_GET_AUTH	WEB_URL "/api/get/auth"
#define WEB_GET_MBL		WEB_URL "/api/get/mbl"
#define WEB_UPLOAD_SS	WEB_URL "/api/post/ss"
#define WEB_UPLOAD_DEMO	WEB_URL "/api/post/demo"

// Auth Responses
#define AUTH_NO_RESPONSE	"-1"	// sv_AuthStrictMode = 2 or 3
#define AUTH_OK				"0"
#define AUTH_INVALID_GUID	"1"		// sv_AuthStrictMode = 1 or 3
#define AUTH_MAX_AGE		"2"
#define AUTH_MIN_AGE		"3"
#define AUTH_BAN_PERM		"4"
#define AUTH_BAN_TEMP		"5"


/*
===============
HTTP_Reply

Structure for replies.
===============
*/
struct HTTP_Reply_t {
	char* ptr;
	size_t len;
};

//
// http_main.c
//
char* HTTP_Post(char* url, char* data);
char* HTTP_Get(char* url, char* data);

//
// http.c
//
char* HTTP_AuthClient(char* guid);
char* HTTP_ClientNeedsUpdate(void);
char* HTTP_ClientGetMOTD(void);

#endif // ~_S_HTTP
