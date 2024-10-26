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
#include <sys/stat.h> // reqSS

/*
===============
HTTP_AuthClient

Will either return Guid or error message.
===============
*/
void HTTP_AuthClient(char userinfo[MAX_INFO_STRING]) {
	HTTP_Inquiry_t* http_inquiry = (HTTP_Inquiry_t*)malloc(sizeof(HTTP_Inquiry_t));

	if (http_inquiry) {
		http_inquiry->url = va("%s", WEB_GET_AUTH);
		http_inquiry->param = va("%s", CODENAME);
		http_inquiry->callback = SV_AuthorizeClient;
		Q_strncpyz(http_inquiry->userinfo, userinfo, sizeof(http_inquiry->userinfo));

		Threads_Create(HTTP_Post, http_inquiry);
	}
	return;
}

#if 0 //#ifndef DEDICATED
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
#endif // ~DEDICATED

static size_t read_callback(char* ptr, size_t size, size_t nmemb, void* stream)
{
	curl_off_t nread;

	size_t retcode = fread(ptr, size, nmemb, stream);

	nread = (curl_off_t)retcode;

	fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
		" bytes from file\n", nread);
	return retcode;
}
/*
===============
reqSS
===============
*/
void* CL_HTTP_SSUpload(void* args) {
	SS_info_t* SS_info = (SS_info_t*)args;
	CURL* curl;
	CURLcode res;
	struct stat file_info;
	double speed_upload, total_time;
	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;
	struct curl_slist* headerlist = NULL;

	FILE* fd;
	static const char buf[] = "Expect:";

	fd = fopen(SS_info->filepath, "rb");

	if (!fd)
	{
		Com_DPrintf("HTTP[fu]: cannot o/r\n");
		return NULL;
	}

	if (fstat(fileno(fd), &file_info) != 0)
	{
		Com_DPrintf("HTTP[fs]: cannot o/r\n");
		return NULL;
	}

	curl = curl_easy_init();

	headerlist = curl_slist_append(headerlist, SS_info->hookid);
	headerlist = curl_slist_append(headerlist, SS_info->hooktoken);
	headerlist = curl_slist_append(headerlist, SS_info->name);
    headerlist = curl_slist_append(headerlist, SS_info->guid);
	headerlist = curl_slist_append(headerlist, SS_info->waittime);
	headerlist = curl_slist_append(headerlist, SS_info->datetime);
	headerlist = curl_slist_append(headerlist, SS_info->protocol);
    //headerlist = curl_slist_append(headerlist, SS_info->ip);
	//headerlist = curl_slist_append(headerlist, va("IND: %s", GAMESTR));

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

		//curl_easy_setopt(curl, CURLOPT_URL, "http://rtcwpro.com:8118//files/0.jpg");
		//curl_easy_setopt(curl, CURLOPT_URL, va("http://rtcwpro.com:8118//files/%s.jpg",SS_info->upfname));
		curl_easy_setopt(curl, CURLOPT_URL, va("http://%s/%s.jpg", SS_info->address, SS_info->filename));
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curl, CURLOPT_READDATA, fd);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);

		//Com_Printf("CL_HTTP_SSUpload:\n address: %s\n hookid: %s\n hooktoken: %s\n waittime: %s\n cleanName: %s\n guid: %s\n filepath: %s\n filename: %s\n",
			//SS_info->address, SS_info->hookid, SS_info->hooktoken, SS_info->waittime, SS_info->name, SS_info->guid, SS_info->filepath, SS_info->filename);

		if (res != CURLE_OK)
		{
			Com_DPrintf("HTTP[res] failed: %s\n", curl_easy_strerror(res));
		}
		else
		{
			curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
//			Com_Printf("^nSpeed: ^7%.3f bytes/sec during %.3f seconds\n", speed_upload, total_time);

		}


		curl_slist_free_all(headerlist);
		curl_easy_cleanup(curl);
	}

	fclose(fd);
	remove(SS_info->filename);
	return NULL;
}




