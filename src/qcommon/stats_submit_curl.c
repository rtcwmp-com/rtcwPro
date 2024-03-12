//#include "../game/g_local.h"
#include "q_shared.h"
#include "../game/g_shared.h"
#include "qcommon.h"
#include "http.h"
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#ifndef _WIN32 
#include <unistd.h>
#endif



struct fdata {
  const char *readptr;
  long sizeline;
};
/*
=====================================
    BEGIN b64: base64 conversion originates from source below
=====================================
*/

/*********************************************************************\
MODULE NAME:    b64.c
AUTHOR:         Bob Trower 08/04/01
PROJECT:        Crypt Data Packaging
COPYRIGHT:      Copyright (c) Trantor Standard Systems Inc., 2001
NOTES:          This source code may be used as you wish, subject to
                the MIT license.  See the LICENCE section below.
                Canonical source should be at:
                    http://base64.sourceforge.net
/*********************************************************************\
*/
#define B64_DEF_LINE_SIZE   72
//#define B64_MIN_LINE_SIZE    4
#define B64_MIN_LINE_SIZE    0



/*
** Translation Table as described in RFC1113
*/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author)
*/
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** returnable errors
**
** Error codes returned to the operating system.
**
*/
#define B64_SYNTAX_ERROR        1
#define B64_FILE_ERROR          2
#define B64_FILE_IO_ERROR       3
#define B64_ERROR_OUT_CLOSE     4
#define B64_LINE_SIZE_TO_MIN    5
#define B64_SYNTAX_TOOMANYARGS  6

/*
** b64_message
**
** Gather text messages in one place.
**
*/
#define B64_MAX_MESSAGES 7
static char *b64_msgs[ B64_MAX_MESSAGES ] = {
            "b64:000:Invalid Message Code.",
            "b64:001:Syntax Error -- check help (-h) for usage.",
            "b64:002:File Error Opening/Creating Files.",
            "b64:003:File I/O Error -- Note: output file not removed.",
            "b64:004:Error on output file close.",
            "b64:005:linesize set to minimum.",
            "b64:006:Syntax: Too many arguments."
};

#define b64_message( ec ) ((ec > 0 && ec < B64_MAX_MESSAGES ) ? b64_msgs[ ec ] : b64_msgs[ 0 ])

/*
** encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
static void encodeblock( unsigned char *in, unsigned char *out, int len )
{
    out[0] = (unsigned char) cb64[ (int)(in[0] >> 2) ];
    out[1] = (unsigned char) cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ (int)(in[2] & 0x3f) ] : '=');
}


/*
** encode
**
** base64 encode a stream adding padding and line breaks as per spec.
*/
static int encode( FILE *infile, FILE *outfile, int linesize )
{

    unsigned char in[3];
    unsigned char out[4];
    int i, len, blocksout = 0;
    int retcode = 0;

    *in = (unsigned char) 0;
    *out = (unsigned char) 0;
    while( feof( infile ) == 0 ) {
        len = 0;
        for( i = 0; i < 3; i++ ) {
            in[i] = (unsigned char) getc( infile );

            if( feof( infile ) == 0 ) {
                len++;
            }
            else {
                in[i] = (unsigned char) 0;
            }
        }
        if( len > 0 ) {
            encodeblock( in, out, len );
            for( i = 0; i < 4; i++ ) {
                if( putc( (int)(out[i]), outfile ) == 0 ){
                    if( ferror( outfile ) != 0 )      {
                        perror( b64_message( B64_FILE_IO_ERROR ) );
                        retcode = B64_FILE_IO_ERROR;
                    }
                    break;
                }
            }
            blocksout++;
        }
        if( blocksout >= (linesize/4) || feof( infile ) != 0 ) {
            if( blocksout > 0 ) {
                //fprintf( outfile, "\r\n" );
            }
            blocksout = 0;
        }
    }
    return( retcode );
}


char* encode_data_b64( char *infilename ) {
  FILE *infile;

  char hpath[256];
  char game[60];

  Cvar_VariableStringBuffer( "fs_homepath", hpath, sizeof( hpath ) );
  Cvar_VariableStringBuffer( "fs_game", game, sizeof( game ) );
  //trap_Cvar_VariableStringBuffer( "fs_homepath", hpath, sizeof( hpath ) );
  //trap_Cvar_VariableStringBuffer( "fs_game", game, sizeof( game ) );
  char* outfilename = va("%s/%s/stats/stats.tmp",hpath,game);
  int linesize = B64_DEF_LINE_SIZE;
  int retcode = B64_FILE_ERROR;

// ENCODE FILE
	infile = fopen( infilename, "rb" );

   if( !infile ) {
        perror( infilename );
    }
    else {
        FILE *outfile;
            outfile = fopen( outfilename, "wb" );

        if( !outfile ) {

            perror( outfilename );
        }
        else {
              retcode = encode( infile, outfile, linesize );

              if( retcode == 0 ) {

                if (ferror( infile ) != 0 || ferror( outfile ) != 0) {
                    perror( b64_message( B64_FILE_IO_ERROR ) );
                    retcode = B64_FILE_IO_ERROR;
                }

              }
             if( outfile != stdout ) {
                if( fclose( outfile ) != 0 ) {
                    perror( b64_message( B64_ERROR_OUT_CLOSE ) );
                    retcode = B64_FILE_IO_ERROR;
                }
             }

        }
    }

    struct stat file_info;
    stat(outfilename, &file_info);
    size_t datasize = (size_t)file_info.st_size;
    FILE* data =  fopen(outfilename , "r");


    return outfilename;

}
/*  =====================================
                 END b64
    =====================================
*/

static struct fdata readfile_content(char* jsonfile) {
    char * buffer = 0;
    long length;
    struct fdata out;
    FILE *f = fopen (jsonfile, "rb");
    size_t read_length;

    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length+1);
        if (buffer)
        {
            read_length=fread (buffer, 1, length, f);
        }
        fclose (f);
        buffer[length]= '\0';
    }

    if (buffer)
    {
       out.sizeline = length;
       out.readptr = buffer;
       return out;
    }

}

size_t StatsAPIResultMessage(char* ptr, size_t size, size_t nmemb, void* userdata) {
    printf("Received response: %.*s\n", (int)(size * nmemb), ptr);
    return size * nmemb;
}

int submit_curlPost( char* jsonfile, char* matchid ) {
    char* outfile = encode_data_b64(jsonfile);   // should put this in memory rather than temp file
    http_stats_t* stats_info = (http_stats_t*)malloc(sizeof(http_stats_t));
    char url[256];

    Cvar_VariableStringBuffer( "g_stats_curl_submit_URL", url, sizeof( url ) );
    if (stats_info) {
        stats_info->url = url;
	    stats_info->matchid = va("matchid: %s", matchid);
	    stats_info->filename = outfile;

	    Threads_Create(submit_HTTP_curlPost, stats_info);
    }
}

// wait X amount of seconds to try again
void RetrySleep(int sleepInSeconds)
{
    #ifdef _WIN32 
        Sleep(sleepInSeconds * 1000); // windows uses milliseconds
    #else
        sleep(sleepInSeconds);
    #endif
}

// post the data to specified server (currently it is fixed but will make customizable via cvar)
void* submit_HTTP_curlPost(void* args) {
    http_stats_t* stats_info = (http_stats_t*)args;
    CURLcode ret;
    CURL *hnd;
    struct curl_slist *slist1;


    struct fdata fileinfo = readfile_content(stats_info->filename);



    slist1 = NULL;
    slist1 = curl_slist_append(slist1, stats_info->matchid );
    slist1 = curl_slist_append(slist1, "x-api-key: rtcwproapikeythatisjustforbasicauthorization");

    hnd = curl_easy_init();
    //curl_easy_setopt(hnd, CURLOPT_URL, "https://rtcwproapi.donkanator.com/submit");
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, &StatsAPIResultMessage);
    curl_easy_setopt(hnd, CURLOPT_URL, stats_info->url);
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, fileinfo.readptr);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)fileinfo.sizeline);

    // curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);

    // THIS DISABLES VERIFICATION OF CERTIFICATE AND IS INSECURE
    //   INCLUDE CERTIFICATE AND CHANGE VALUE TO 1!
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);

    curl_easy_setopt(hnd, CURLOPT_USE_SSL, CURLUSESSL_TRY);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);

    int retryCount = 0;
    int shouldRetry = qtrue;

    // get variables for retry count and delay
    char varStatsRetryCount[10];
    char varStatsRetryDelay[10];
    Cvar_VariableStringBuffer("g_statsRetryCount", varStatsRetryCount, sizeof(varStatsRetryCount));
    Cvar_VariableStringBuffer("g_statsRetryDelay", varStatsRetryDelay, sizeof(varStatsRetryDelay));

    int statsRetryCount = atoi(varStatsRetryCount);
    int statsRetryDelay = atoi(varStatsRetryDelay);

    // make sure we try at least once
    if (statsRetryCount < 1)
        statsRetryCount = 1;

    while (shouldRetry && retryCount < statsRetryCount) {
        Com_Printf("Stats API: Calling URL with stats payload\n");
        ret = curl_easy_perform(hnd);

        if (ret != CURLE_OK) {
            Com_Printf("Stats API: Curl Error return code: %s\n", curl_easy_strerror(ret));
            Com_Printf("Stats API: Retrying request...\n");

            retryCount++;
            RetrySleep(statsRetryDelay);  // Wait for a delay before retrying
        }
        else {
            shouldRetry = qfalse;  // Request succeeded, exit the retry loop
        }
    }

    if (shouldRetry) {
        Com_Printf("Stats API: Maximum retry limit reached. Request failed.\n");
    }

    curl_slist_free_all(slist1);
    slist1 = NULL;
    curl_easy_cleanup(hnd);
    hnd = NULL;

    remove(stats_info->filename);
    return (int)ret;

}
