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

#include "g_local.h"

/*
============
Cmd_APIQuery
Call RtcwPro API
============
*/
void Cmd_APIQuery(gentity_t* ent)
{
	char command[256];
	char arg1[256];
	char arg2[256];
	char* jsonText;

	trap_Argv(1, command, sizeof(command));
	trap_Argv(2, arg1, sizeof(arg1));
	trap_Argv(3, arg2, sizeof(arg2));

	// get JSON text for the command
	jsonText = G_CreateAPIJson(command, arg1, arg2, ent->client->sess.guid);

	int clientNum = ent->client - level.clients;

	trap_HTTP_apiQuery(command, jsonText, clientNum);
}

/*
============
trap_HandleApiResponse
Handle API Response
============
*/
void trap_HandleApiResponse(int clientNum, char* response)
{
	gentity_t* ent;
	ent = g_entities + clientNum;
	if (!ent->inuse || ent->client->pers.connected != CON_CONNECTED)
	{
		G_Printf("Cmd_HandleApiResponse: Invalid client number: %i\n", clientNum);
		return;
	}

	if (g_alternatePing.integer)
	{
		if (ent->client->pers.alternatePing < 0 || ent->client->pers.alternatePing >= 999)
		{
			G_Printf("Cmd_HandleApiResponse: Invalid client number: %i\n", clientNum);
			return;
		}
	}
	else
	{
		if (ent->client->ps.ping < 0 || ent->client->ps.ping >= 999)
		{
			G_Printf("Cmd_HandleApiResponse: Invalid client number: %i\n", clientNum);
			return;
		}
	}

	if (ReadApiResultJson(response))
	{
		// response is a json string so let's just strip off the special characters and add new lines
		response = Q_StrReplace(response, "[", "");
		response = Q_StrReplace(response, "\"", "");
		response = Q_StrReplace(response, "\\", "");
		response = Q_StrReplace(response, "]", "");
		response = Q_StrReplace(response, ",", "[NL]"); // [NL] will get replaced with \n on the CG side - syscalls don't seem to like \n sent across
		response = Q_StrReplace(response, "[NL] ", "[NL]");

		CP(va("api \"%s\n\n\n\"", response));
	}
	else
		CP("api \"Invalid response from api.\n\n\n\"");
}


/*
============
G_CreateAPIJson
Create JSON to send to the API
============
*/
char* G_CreateAPIJson(char* commandText, char* arg1, char* arg2, char* callerGuid)
{
	int idnum, cnt = 0, tteam;
	gclient_t* cl;
	gentity_t* cl_ent;
	char alias[MAX_NETNAME];
	char mapName[MAX_QPATH];
	char userinfo[MAX_INFO_STRING];
	char *s, *uinfo = "", *guid = "", *team, *jsonCommand = "";

	trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));

	json_t* jdata = json_object(); // json for all queries
	
	json_object_set_new(jdata, "format", json_string(va("%s", "v1")));
	json_object_set_new(jdata, "command", json_string(va("%s", commandText))); // set the command
	json_object_set_new(jdata, "server_name", json_string(sv_hostname.string));
	json_object_set_new(jdata, "map", json_string(mapName));
	json_object_set_new(jdata, "caller", json_string(va("%s", callerGuid)));
	json_object_set_new(jdata, "matchid", json_string(va("%s", level.jsonStatInfo.match_id))); // same as MATCHID in g_json


	if (!Q_stricmp(commandText, "whois"))
	{
		json_t* players = json_object();
		json_t* playerGuid = json_object();

		time_t unixTime = time(NULL);
		s = "";
		
		for (int i = 0; i < level.numConnectedClients; i++)
		{
			idnum = level.sortedClients[i];
			cl = &level.clients[idnum];
			cl_ent = g_entities + idnum;

			SanitizeString(cl->pers.netname, alias);
			Q_CleanStr(alias);
			alias[26] = 0;

			trap_GetUserinfo(idnum, userinfo, sizeof(userinfo));

			uinfo = Info_ValueForKey(userinfo, "cg_uinfo");

			char* array[10];
			int i = 0;
			array[i] = strtok(uinfo, " ");
			while (array[i] != NULL)
				array[++i] = strtok(NULL, " ");
			guid = array[6];

			tteam = cl->sess.sessionTeam;

			if (tteam == TEAM_SPECTATOR || tteam == TEAM_FREE) team = "Spec";
			else if (tteam == TEAM_RED) team = "Axis";
			else team = "Allied";

			json_object_set_new(players, "alias", json_string(va("%s", alias)));
			json_object_set_new(players, "team", json_string(va("%s", team)));
			s = json_dumps(players, 0);
			json_object_set(playerGuid, guid, json_string(s));

			s = json_dumps(playerGuid, 0);
			json_object_set(jdata, "players", json_string(s));
			/*
			{
			 "format": "v1",
			 "command": "whois",
			 "players": {"c7594c502fdaa397f84bf7f00d3708e4": {"alias": "kk1", "team": "Spec}}"
			}

			json_t* root = json_pack("{s:s, s:s, s:{s:s: {s:s, s:s}}",
				"format", "v1", "command", commandText, "players", "properties",
				"geometry", "type", "LineString", "coordinates", jdata);

			s = json_dumps(root, JSON_INDENT(2)); 
			*/

			free(s);
		}

		char* replaceStrings = Q_StrReplace(json_dumps(jdata, 1), "\\", "");
		replaceStrings = Q_StrReplace(replaceStrings, "\"{\"", "{\"");
		replaceStrings = Q_StrReplace(replaceStrings, "\"}\"}\"", "\"}}");
		replaceStrings = Q_StrReplace(replaceStrings, "\"}\",", "\"},");
		return replaceStrings;

	}
	else
	{
		jsonCommand = va("{\"command\":\"%s\"}", commandText);
	}

	return jsonCommand;
}

/*
============
ReadApiResultJson
Check that the API response is a valid JSON
============
*/
int ReadApiResultJson(char* data)
{
	json_t* root;
	json_error_t error;

	data = Q_StrReplace(data, "\"[", "[");
	data = Q_StrReplace(data, "]\"", "]");
	data = Q_StrReplace(data, "\\", "");

	root = json_loads(data, 0, &error);
	if (!root)
	{
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		return 0;
	}
	if (!json_is_array(root))
	{
		fprintf(stderr, "error: root is not an array\n");
		json_decref(root);
		return 0;
	}

	json_decref(root);

	return 1;
}