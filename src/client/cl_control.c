/*
sswolf - req SS functions.
Source: Nate (rtcwMP)
*/

#include "client.h"
#include "../qcommon/http.h"
#include <sys/stat.h> // reqSS
#include <time.h>
/*
================
Takes ScreenShot
================
*/
//void CL_takeSS(char* name, int quality) {
void CL_takeSS(char* name) {

	//Cbuf_ExecuteText(EXEC_NOW, va("screenshotJPEG sys %s %d\n", name, quality));
	Cbuf_ExecuteText(EXEC_NOW, va("screenshotJPEG silent %s\n", name));
}

/*
================
Generate time..
================
*/
void CL_actionGenerateTime(void) {

	int min = 600 * 1000;	// 10 mins
	int max = 12000 * 1000;	// 20 mins
	int time = rand() % max + min;

	cl.clientSSAction = cl.serverTime + time;
}

/*
================
Check if it's time to take the screenshot..
================
*/
void CL_checkSSTime(void) {

	if (!cl.clientSSAction)
	{
		CL_actionGenerateTime();
	}
	else
	{
		if (cl.serverTime >= cl.clientSSAction)
		{
			//CL_RequestedSS(45);
	//		CL_RequestedSS();
			CL_RequestedSS(" ");  // temp change

		}
	}
}

/*
================
ScreenShot request from server
================
*/
void CL_RequestedSS(char* ip) {
	char* filename = va("%d", cl.clientSSAction);
	char* file;
	SS_info_t* SS_info = (SS_info_t*)malloc(sizeof(SS_info_t));
	char* guid;
	char* name;
	int n;

	if (clc.demoplaying) {
		return;
	}

    srand(time(0));
    n = rand() % 99;

	guid = Cvar_VariableString("cl_guid");
	name = Cvar_VariableString("name");

	CL_takeSS(filename);
	CL_actionGenerateTime();

	file = getFilePath(filename);
	if (SS_info) {
		SS_info->guid = va("GUID: %s", guid);
		SS_info->name = va("NAME: %s", name);
		SS_info->ip = va("IP: %s", ip);
		SS_info->upfname = va("%i", n);
		SS_info->filename = file;

		Threads_Create(CL_HTTP_SSUpload, SS_info);
	}

}

