//#include "../game/g_local.h"
#include "../game/q_shared.h"
#include "../game/g_shared.h"
#include "qcommon.h"
#include "http.h"
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
//#include <string.h>

int CL_APIQuery(char* commandText) {

    HTTP_Inquiry_t* query_info = (HTTP_Inquiry_t*)malloc(sizeof(HTTP_Inquiry_t));
    char url[256];

    Cvar_VariableStringBuffer("g_stats_curl_submit_URL", url, sizeof(url));
    if (query_info) {
        query_info->url = url;
        query_info->param = va("command: %s", commandText);

        Threads_Create(CL_HTTP_apiQuery, query_info);
    }

    HTTP_Inquiry_t* http_inquiry = (HTTP_Inquiry_t*)malloc(sizeof(HTTP_Inquiry_t));

    return;
}

// post the data to specified server (currently it is fixed but will make customizable via cvar)
void* CL_HTTP_apiQuery(void* args) {
    HTTP_Inquiry_t* query_info = (HTTP_Inquiry_t*)args;
    CURLcode ret;
    CURL* hnd;
    struct curl_slist* slist1;


    slist1 = NULL;
    //slist1 = curl_slist_append(slist1, query_info->matchid);
    slist1 = curl_slist_append(slist1, "x-api-key: rtcwproapikeythatisjustforbasicauthorization");

    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_URL, "https://rtcwproapi.donkanator.com/serverquery");
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);

    // THIS DISABLES VERIFICATION OF CERTIFICATE AND IS INSECURE
    //   INCLUDE CERTIFICATE AND CHANGE VALUE TO 1!
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);

    curl_easy_setopt(hnd, CURLOPT_USE_SSL, CURLUSESSL_TRY);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, printcurlresponse);

    Com_Printf(va("Pro API: Client issued API Command %s\n", query_info->param));
    ret = curl_easy_perform(hnd);

    if (ret != CURLE_OK)
    {
        Com_Printf("Stats API: Curl Error return code: %s\n", curl_easy_strerror(ret));
    }

    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(slist1);
    slist1 = NULL;

    return (int)ret;

}
