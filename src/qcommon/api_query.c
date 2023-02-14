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

#include <curl/curl.h>
#include <curl/easy.h>
#include "../qcommon/threads.h"

/*
============
HTTP_APIInquiry_t

Structure for issuing inquiries and invoking callbacks.
============
*/
typedef struct {
	char* url;
	char* param;
	char* jsonText;

	void (*callback)(char* fmt, ...);
} HTTP_APIInquiry_t;

/*
================
 RtcwPro API Server Query
================
*/
int API_Query(char* param, char* jsonText) {

	HTTP_APIInquiry_t* query_info = (HTTP_APIInquiry_t*)malloc(sizeof(HTTP_APIInquiry_t));

	if (query_info) {

		query_info->url = "";
		query_info->param = va("%s", param);
		query_info->jsonText = va("%s", jsonText);

		Threads_Create(API_HTTP_Query, query_info);
	}

	HTTP_APIInquiry_t* http_inquiry = (HTTP_APIInquiry_t*)malloc(sizeof(HTTP_APIInquiry_t));
}

/*
===============
serverQuery
===============
*/
static size_t APIResultMessage(void* ptr, size_t size, size_t nmemb, void* stream) {
	Com_Printf("%s", ptr);
}

void* API_HTTP_Query(void* args) {
	HTTP_APIInquiry_t* query_info = (HTTP_APIInquiry_t*)args;
	CURLcode ret;
	CURL* hnd;
	struct curl_slist* slist1;
	char url[256];

	Cvar_VariableStringBuffer("g_apiquery_curl_URL", url, sizeof(url));

	slist1 = NULL;
	//slist1 = curl_slist_append(slist1, query_info->matchid);
	slist1 = curl_slist_append(slist1, "x-api-key: rtcwproapikeythatisjustforbasicauthorization");

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_URL, url);
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, query_info->jsonText);

	// THIS DISABLES VERIFICATION OF CERTIFICATE AND IS INSECURE
	//   INCLUDE CERTIFICATE AND CHANGE VALUE TO 1!
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);

	curl_easy_setopt(hnd, CURLOPT_USE_SSL, CURLUSESSL_TRY);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, APIResultMessage);

	//Com_Printf(va("Pro API: Client issued API Command %s\n", query_info->param));
	ret = curl_easy_perform(hnd);

	if (ret != CURLE_OK)
	{
		Com_Printf("Query API: Curl Error return code: %s\n", curl_easy_strerror(ret));
	}

	curl_easy_cleanup(hnd);
	hnd = NULL;
	curl_slist_free_all(slist1);
	slist1 = NULL;

}