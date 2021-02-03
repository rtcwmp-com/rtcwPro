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
HTTP_InitString

Allocates memory for web requeest.
===============
*/
void HTTP_InitString(struct HTTP_Reply_t* s) {
	s->len = 0;
	s->ptr = malloc(s->len + 1);

	if (s->ptr == NULL) {
		Com_DPrintf("HTTP[i_s]: malloc() failed\n");
		return;
	}
	s->ptr[0] = '\0';
}

/*
===============
HTTP_ParseReply

Parses web reply. 
===============
*/
size_t HTTP_ParseReply(void* ptr, size_t size, size_t nmemb, struct HTTP_Reply_t* s) {
	size_t new_len = s->len + size * nmemb;
	char* tmp;

	tmp = realloc(s->ptr, new_len + 1);
	if (tmp == NULL) {
		Com_DPrintf("HTTP[write]: realloc() failed\n");
		return 0;
	}

	s->ptr = tmp;
	memcpy(s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}

/*
===============
HTTP_write_data

Writes data (for download).
===============
*/
size_t HTTP_WriteData(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	return written;
}

/*
===============
HTTP_GetFileSize

Returns file size.
===============
*/
int HTTP_GetFileSize(FILE* fp) {
	int prev = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);

	//go back to where we were
	fseek(fp, prev, SEEK_SET);
	return sz;
}

/*
===============
getCurrentPath

Because we're not going through Game we need to sort stuff ourself.
===============
*/
char* getCurrentPath(char* file) {
	char* path = Cvar_VariableString("fs_game");

	return (strlen(path) < 2?va("%s/%s", BASEGAME, file):va("%s/%s", path, file));
}

/*
===============
HTTP_Post

Posts a request and returns a response.
===============
*/
char* HTTP_Post(char* url, char* data) {
	CURL* handle;
	CURLcode res;
	char* response = AUTH_NO_RESPONSE;

	handle = curl_easy_init();
	if (handle) {
		struct HTTP_Reply_t s;
		HTTP_InitString(&s);
		struct curl_slist* headers = NULL;
		char* token_header = "Signature:";
		
		headers = curl_slist_append(headers, va("Mod: %s", GAMEVERSION));
#ifdef DEDICATED
		headers = curl_slist_append(headers, va("%s %s", token_header, sv_StreamingToken->string));

		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, sv_StreamingSelfSignedCert->integer);
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, sv_StreamingSelfSignedCert->integer);
#else
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, cl_StreamingSelfSignedCert->integer);
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, cl_StreamingSelfSignedCert->integer);
#endif
		curl_easy_setopt(handle, CURLOPT_URL, url);
		curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 3);
		curl_easy_setopt(handle, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, HTTP_ParseReply);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, &s);
		curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1);

		res = curl_easy_perform(handle);
		if (res != CURLE_OK) {
			Com_DPrintf("HTTP_Post[res] failed: %s\n", curl_easy_strerror(res));
		}
		else {
			response = va("%s", s.ptr);
		}
		free(s.ptr);
		curl_slist_free_all(headers);
	}
	curl_easy_cleanup(handle);

	return response;
}

/*
===============
HTTP_Get

Sends a request and returns a response.
===============
*/
char* HTTP_Get(char* url, char* data) {
	CURL* handle;
	CURLcode res;
	char* response = AUTH_NO_RESPONSE;

	handle = curl_easy_init();
	if (handle) {
		struct HTTP_Reply_t s;
		HTTP_InitString(&s);
		struct curl_slist* headers = NULL;
		char* token_header = "Signature:";

		headers = curl_slist_append(headers, va("Mod: %s", GAMEVERSION));
#ifdef DEDICATED
		headers = curl_slist_append(headers, va("%s %s", token_header, sv_StreamingToken->string));

		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, sv_StreamingSelfSignedCert->integer);
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, sv_StreamingSelfSignedCert->integer);
#else
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, cl_StreamingSelfSignedCert->integer);
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, cl_StreamingSelfSignedCert->integer);
#endif
		curl_easy_setopt(handle, CURLOPT_URL, url);
		curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
		curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(handle, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 3);
		curl_easy_setopt(handle, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, HTTP_ParseReply);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, &s);
		curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1);

		res = curl_easy_perform(handle);
		if (res != CURLE_OK) {
			Com_DPrintf("HTTP_Get[res] failed: %s\n", curl_easy_strerror(res));
		}
		else {
			response = va("%s", s.ptr);
		}
		free(s.ptr);
		curl_slist_free_all(headers);
	}
	curl_easy_cleanup(handle);

	return response;
}
