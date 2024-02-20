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

#include "q_shared.h"
#include "../game/g_shared.h"
#include "qcommon.h"
#include "http.h"


size_t APIResultMessage(char* ptr, size_t size, size_t nmemb, void* userdata) {
	/* Cast the user data to an integer */
	int clientNumber = *((int*)userdata);

	/* Print the response along with the integer */
	printf("Received response (integer=%d): %.*s\n", clientNumber, (int)(size * nmemb), ptr);

	VM_Call(gvm, G_RETURN_API_QUERY_RESPONSE, clientNumber, ptr);

	/* Return the number of bytes processed */
	return size * nmemb;
}

/*
================
 RtcwPro API Server Query
================
*/
void* API_Query(char* param, char* jsonText, int clientNumber) {

	HTTP_APIInquiry_t* query_info = (HTTP_APIInquiry_t*)malloc(sizeof(HTTP_APIInquiry_t));

	if (query_info) {

		query_info->url = "";
		query_info->param = va("%s", param);
		query_info->jsonText = va("%s", jsonText);
		query_info->clientNumber = clientNumber;
		//query_info->callback = APIResultMessage;

		Threads_Create(API_HTTP_Query, query_info);
	}

	return;
}

/*
===============
serverQuery
===============
*/
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
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &query_info->clientNumber);

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

	return;
}