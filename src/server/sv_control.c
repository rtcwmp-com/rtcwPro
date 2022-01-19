/*
RTCWPro - reqSS functions.
Source: Nate (rtcwMP)
*/

#include "server.h"

/*
================
Sends request to client for screenshot
NOTE: It will be uploaded to default web server..
================
*/
//void SV_SendSSRequest(int clientNum, int quality)
void SV_SendSSRequest(int clientNum)
{
    char        *value;
    client_t    *cl;
	if (clientNum == -1 || clientNum < 0 || clientNum >= sv_maxclients->integer)
	{
		return;
	}

	cl = &svs.clients[clientNum];
	if ( !cl ) 
	{
		return;
	}

	if (cl->ping < 0 || cl->ping >= 999)
	{
		return;
	}

	value = Info_ValueForKey( cl->userinfo, "ip" );

	//SV_SendServerCommand(svs.clients + clientNum, "ssreq %d", quality);
	SV_SendServerCommand(svs.clients + clientNum, "ssreq %s", value);
}

/*
================
Requests screenshot from all connected clients
================
*/
void autoSSTime(void) {
	int i;
	client_t* cl;

	if (!sv_ssEnable->integer)
		return;

	if (svs.time < svs.ssTime)
		return;

	for (i = 0; i < sv_maxclients->integer; i++)
	{
		cl = &svs.clients[i];

		if (cl->state)
			//SV_SendSSRequest(i, sv_ssQuality->value);
			SV_SendSSRequest(i);
	}

	// Sort some stuff
	if (sv_ssMinTime->integer < 300)
		Cvar_Set("sv_ssMinTime", "300");
	else if (sv_ssMinTime->integer > 1200)
		Cvar_Set("sv_ssMinTime", "1200");

	if (sv_ssMaxTime->integer > 1500)
		Cvar_Set("sv_ssMaxTime", "1500");
	else if (sv_ssMaxTime->integer < 450)
		Cvar_Set("sv_ssMaxTime", "450");

	/*if (sv_ssQuality->integer > 100)
		Cvar_Set("sv_ssQuality", "100");
	else if (sv_ssQuality->integer < 30)
		Cvar_Set("sv_ssQuality", "30");*/

	// Set new timer
	{
		int min = sv_ssMinTime->integer * 1000;
		int max = sv_ssMaxTime->integer * 1000;
		int newTime = rand() % max + min;

		svs.ssTime = svs.time + newTime;
	}
}

