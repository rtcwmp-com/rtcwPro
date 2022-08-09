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
#include "threads.h"
#include "../server/server.h"
#include "../client/client.h"


//
// URL Mappings
//
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
#define AUTH_INVALID_GUID	"0"		// sv_AuthStrictMode = 1 or 3
#define AUTH_OK				"1"



/*
===============
HTTP_Reply

Structure for replies.
===============
*/
struct HTTP_Reply_t {
	char*	ptr;
	size_t	len;
};

/*
============
HTTP_Inquiry_t

Structure for issuing inquiries and invoking callbacks.
============
*/
typedef struct {
	char*	url;
	char*	param;
	char	userinfo[MAX_INFO_STRING];

	void (*callback)(char* fmt, ...);
} HTTP_Inquiry_t;



/*
============
Lazy way for submitting SS..
eventually use struct above
============
*/
typedef struct {
	char* address;
	char* hookid;
	char* hooktoken;
	char* waittime;
	char* datetime;
	char* name;
	char* guid;
	char* filepath;
	char* filename;
	void (*callback)(char* fmt, ...);
} SS_info_t;


typedef struct {
    char* url;
	char* filename;
    char* matchid;
	void (*callback)(char* fmt, ...);
} http_stats_t;

//
// http_main.c
//
void* HTTP_Post(void* args);
void* HTTP_Get(void* args);
char* getCurrentPath(char* file);

//
// http.c
//
void HTTP_AuthClient(char userinfo[MAX_INFO_STRING]);
#ifndef DEDICATED
void HTTP_ClientNeedsUpdate(void);
void HTTP_ClientGetMOTD(void);
#endif
void* CL_HTTP_SSUpload(void* args);
//qboolean CL_HTTP_SSUpload(char* url, char* file, char* marker, char* marker2); // reqSS

#endif // ~_S_HTTP
