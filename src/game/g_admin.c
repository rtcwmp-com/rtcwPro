/*
===========================================================================

wolfX GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of wolfX source code.

wolfX Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

wolfX Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with wolfX Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the wolfX Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the wolfX Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
L0 - g_admin.c

Admin functions
Administration functions and features.

Created: 17. Oct / 2012
Last Updated: 28. Apr / 2013
===========================================================================
*/



#include "g_local.h"

/*
===========
Sort tag
===========
*/
char *sortTag(gentity_t *ent) {
	char *tag;
	char n1[MAX_NETNAME];

	if (ent->client->sess.admin == ADM_1)
		tag = a1_tag.string;
	else if (ent->client->sess.admin == ADM_2)
		tag = a2_tag.string;
	else if (ent->client->sess.admin == ADM_3)
		tag = a3_tag.string;
	else if (ent->client->sess.admin == ADM_4)
		tag = a4_tag.string;
	else if (ent->client->sess.admin == ADM_5)
		tag = a5_tag.string;
	else
		tag = "";

	// No colors in tag for console prints..
	DecolorString(tag, n1);
	//SanitizeString(n1, tag);
	//Q_CleanStr(tag);
	tag[20] = 0;	 // 20 should be enough..

return tag;
}

/*
===========
Login
===========
*/
void cmd_do_login (gentity_t *ent, qboolean silent) {
	char str[MAX_TOKEN_CHARS];
	qboolean error;
	char *log;

	error = qfalse;
	trap_Argv( 1, str, sizeof( str ) );

	// Make sure user is not already logged in.
	if (!ent->client->sess.admin == ADM_NONE )
	{
		CP("print \"You are already logged in^1!\n\"");
	return;
	}
	// Prevent bogus logins
	if (( !Q_stricmp( str, "\0")) || ( !Q_stricmp( str, "")) || ( !Q_stricmp( str, "\"")) || ( !Q_stricmp( str, "none")) ) {
		CP("print \"Incorrect password^1!\n\"");
		// No log here to avoid login by error..
	return;
	}

		// Else let's see if there's a password match.
		if ( (Q_stricmp(str, a1_pass.string) == 0)
			|| (Q_stricmp(str, a2_pass.string) == 0)
			|| (Q_stricmp(str, a3_pass.string) == 0)
			|| (Q_stricmp(str, a4_pass.string) == 0)
			|| (Q_stricmp(str, a5_pass.string) == 0) ) {

			// Always start with lower level as if owner screws it up and sets the same passes for more levels, the lowest is the safest bet.
			if (Q_stricmp(str, a1_pass.string) == 0) {
				ent->client->sess.admin = ADM_1;
			} else if (Q_stricmp(str, a2_pass.string) == 0) {
				ent->client->sess.admin = ADM_2;
			} else if (Q_stricmp(str, a3_pass.string) == 0) {
				ent->client->sess.admin = ADM_3;
			} else if (Q_stricmp(str, a4_pass.string) == 0) {
				ent->client->sess.admin = ADM_4;
			} else if (Q_stricmp(str, a5_pass.string) == 0) {
				ent->client->sess.admin = ADM_5;
			} else {
				error = qtrue;
			}
				// Something went to hell..
				if (error == qtrue) {
					// User shouldn't be anything but regular so make sure..
					ent->client->sess.admin = ADM_NONE;
						CP("print \"Error has occured while trying to log you in^z!\n\"");
				return;
				}

				// We came so far so go with it..
				if (silent) {
					CP("print \"Silent Login successful^z!\n\"");
					ent->client->sess.incognito = 1; // Hide them

					// Log it
					log =va("Player %s (IP:%s) has silently logged in as %s.",
					ent->client->pers.netname, ent->client->sess.ip, sortTag(ent));
					logEntry (ADMLOG, log);
				} else {
					AP(va("chat \"^zconsole:^7 %s ^7has logged in as %s^z!\n\"", ent->client->pers.netname, sortTag(ent)));

					// Log it
					log =va("Player %s (IP:%s) has logged in as %s.",
					ent->client->pers.netname, ent->client->sess.ip, sortTag(ent));
					logEntry (ADMLOG, log);
				}
				// Make sure logged in user bypasses any spec lock instantly.
				ent->client->sess.specInvited = 3;
		return;
		// No match..
		} else {
			CP("print \"Incorrect password^1!\n\"");

			// Log it
			log =va("Player %s (IP:%s) has tried to login using password: %s",
			ent->client->pers.netname, ent->client->sess.ip, str);
			logEntry (PASSLOG, log);
		return;
		}
}

/*
===========
Logout
===========
*/
void cmd_do_logout(gentity_t *ent) {
	// If user is not logged in do nothing
	if (ent->client->sess.admin == ADM_NONE) {
		return;
	} else {
		// Admin is hidden so don't print
		if (ent->client->sess.incognito)
			CP("print \"You have successfully logged out^z!\n\"");
		else
			AP(va("chat \"^zconsole:^7 %s ^7has logged out^z!\n\"", ent->client->pers.netname));

		// Log them out now
		ent->client->sess.admin = ADM_NONE;
		// Set incognito to visible..
		ent->client->sess.incognito = 0;
		// Clear speclock
		ent->client->sess.specInvited = 0;
		// Blackouts player after logout
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
			!ent->client->sess.specInvited &&
			(teamInfo[TEAM_RED].spec_lock || teamInfo[TEAM_BLUE].spec_lock)  )
			SetTeam( ent, "s", qtrue);
		// This needs to get tested under various conditions
		else if ( teamInfo[TEAM_RED].spec_lock || teamInfo[TEAM_BLUE].spec_lock )
			G_updateSpecLock(ent->client->sess.specInvited, qtrue);

	return;
	}
}

/*********************************** FUNCTIONALITY ************************************/

/*
===========
Time for log, getstatus..
===========
*/
extern int trap_RealTime ( qtime_t * qtime );
const char *cMonths[12] = {
"Jan", "Feb", "Mar", "Apr", "May", "Jun",
"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/*
===========
Log Admin related stuff
===========
*/
void logEntry (char *filename, char *info)
{
	fileHandle_t	f;
	char *varLine;
	qtime_t		ct;
	trap_RealTime( &ct );

	if (!g_extendedLog.integer)
		return;

	strcat (info, "\r");
	trap_FS_FOpenFile( filename, &f, FS_APPEND);

	varLine=va("Time: %02d:%02d:%02d/%02d %s %d : %s \n",
				ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday,
				cMonths[ct.tm_mon], 1900+ct.tm_year, info);

	trap_FS_Write(varLine, strlen( varLine ), f);
	trap_FS_FCloseFile(f);
return;
}

/*
===========
Get client number from name
===========
*/
int ClientNumberFromNameMatch(char *name, int *matches){
	int i, textLen;
	char nm[32];
	char c;
	int index = 0;

	Q_strncpyz(nm, name, sizeof(nm));
	Q_CleanStr(nm);
	textLen = strlen(nm);
	c = *nm;

	for (i = 0; i < level.maxclients; i++)
	{
		int j, len;
		char playerName[32];

		if ((!g_entities[i].client) || (g_entities[i].client->pers.connected != CON_CONNECTED) )
			continue;

		Q_strncpyz(playerName, g_entities[i].client->pers.netname, sizeof(playerName));
		Q_CleanStr(playerName);
		len = strlen(playerName);

		for (j = 0; j < len; j++)
		{
			if (tolower(c) == tolower(playerName[j]))
			{
				if (!Q_stricmpn(nm, playerName+j, textLen))
				{
					matches[index] = i;
					index++;
					break;
				}
			}
		}
	}
return index;
}

/*
===========
Deals with ! & ?
===========
*/
void admCmds(const char *strCMD1, char *strCMD2, char *strCMD3, qboolean cmd){

	int i = 0, j=0;
	int foundcolon=0;

	while(strCMD1[i] != 0)
	{
		if(!foundcolon)
		{
			if (cmd) {
				if(strCMD1[i] == '?') {
					foundcolon = 1;
					strCMD2[i]=0;
				}
				else
					strCMD2[i]=strCMD1[i];
				i++;
			} else {
				if(strCMD1[i] == '!') {
					foundcolon = 1;
					strCMD2[i]=0;
				}
				else
					strCMD2[i]=strCMD1[i];
				i++;
			}
		}
		else
		{
			strCMD3[j++]=strCMD1[i++];
		}
	}
	if(!foundcolon)
		strCMD2[i]=0;
	strCMD3[j]=0;
}

/*
===========
Parse string (if I recall right this bit is from S4NDMoD)
===========
*/
void ParseAdmStr(const char *strInput, char *strCmd, char *strArgs)
{
	int i = 0, j=0;
	int foundspace=0;

	while(strInput[i] != 0){
		if(!foundspace){
			if(strInput[i] == ' '){
				foundspace = 1;
				strCmd[i]=0;
			}else
				strCmd[i]=strInput[i];
			i++;
		}else{
			strArgs[j++]=strInput[i++];
		}
	}
	if(!foundspace)
		strCmd[i]=0;

strArgs[j]=0;
}


/*
===========
Deals with customm commands
===========
*/
void cmdCustom(gentity_t *ent, char *cmd) {
	char *tag, *log;

	tag = sortTag(ent);

	if (!strcmp(ent->client->pers.cmd2,"")) {
		CP(va("print \"Command ^1%s ^7must have a value^z!\n\"", cmd));
	return;
	} else {
		// Rconpasswords or sensitve commands can be changed without public print..
		if (!strcmp(ent->client->pers.cmd3,"@"))
			CP(va("print \"Info: ^2%s ^7was silently changed to ^2%s^z!\n\"", cmd, ent->client->pers.cmd2));
		else
			AP(va("chat \"^zconsole:^7 %s ^7changed ^z%s ^7to ^z%s %s\n\"", tag, cmd, ent->client->pers.cmd2, ent->client->pers.cmd3));
		// Change the stuff
		trap_SendConsoleCommand( EXEC_APPEND, va("%s %s %s", cmd, ent->client->pers.cmd2, ent->client->pers.cmd3));
		// Log it
		log =va("Player %s (IP:%s) has changed %s to %s %s.",
			ent->client->pers.netname, ent->client->sess.ip, cmd, ent->client->pers.cmd2, ent->client->pers.cmd3);
		logEntry (ADMACT, log);
	return;
	}
}

/*
===========
Can't use command msg..
===========
*/
void cantUse(gentity_t *ent) {
	char alt[128];
	char cmd[128];

	admCmds(ent->client->pers.cmd1, alt, cmd, qfalse);

	CP(va("print \"Command ^1%s ^7is not allowed for your level^z!\n\"", cmd));
return;
}

/*
===========
Determine if admin level allows command
===========
*/
qboolean canUse(gentity_t *ent, qboolean isCmd) {
	char *permission="";
	char *token, *parse;
	char alt[128];
	char cmd[128];

	switch (ent->client->sess.admin) {
		case ADM_NONE:// So linux stops complaining..
			return qfalse;
		break;
		case ADM_1:
			permission = a1_cmds.string;
		break;
		case ADM_2:
			permission = a2_cmds.string;
		break;
		case ADM_3:
			permission = a3_cmds.string;
		break;
		case ADM_4:
			permission = a4_cmds.string;
		break;
		case ADM_5:
			if (a5_allowAll.integer && isCmd) // Return true if allowAll is enabled and is command.
				return qtrue;
			else
				permission = a5_cmds.string;  // Otherwise just loop thru string and see if there's a match.
		break;
	}

	admCmds(ent->client->pers.cmd1, alt, cmd, qfalse);

	if (strlen(permission)) {
		parse = permission;
		while (1) {
			token = COM_Parse(&parse);
			if (!token || !token[0])
				break;

			if (!Q_stricmp(cmd, token))	{
					return qtrue;
			}
		}
		return qfalse;
	}
return qfalse;
}

/*
===========
Global sound
===========
*/
void APSound(char *sound){
	gentity_t *ent;
	gentity_t *te;

	ent = g_entities;

	te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
	te->s.eventParm = G_SoundIndex(sound);
	te->r.svFlags |= SVF_BROADCAST;
}

/*
===========
Client sound
===========
*/
void CPSound(gentity_t *ent, char *sound){
	gentity_t *te;

	te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND );
	te->s.eventParm = G_SoundIndex(sound);
	te->s.teamNum = ent->s.clientNum;
}

/*
===========
Global sound with limited range
===========
*/
void APRSound(gentity_t *ent, char *sound){
	gentity_t   *te;

	te = G_TempEntity( ent->r.currentOrigin, EV_GENERAL_SOUND );
	te->s.eventParm = G_SoundIndex(sound);
}

/*
===========
Check if string matches IP pattern
===========
*/
void flip_it (char *s, char in, char out) {
	while (*s != 0) {
		if (*s == in)
			*s = out;
		s++;
	}
}
// It's not perfect but it helps..
qboolean IPv4Valid( char *s)
{
    int c, i, len = strlen(s);
	unsigned int d[4];
	char vrfy[16];

    if (len < 7 || len > 15)
        return qfalse;

    vrfy[0] = 0;
	flip_it(s, '*', (char)256);

    c = sscanf( s, "%3u.%3u.%3u.%3u%s",
				&d[0], &d[1], &d[2], &d[3], vrfy);

    if (c != 4 || vrfy[0])
        return qfalse;

    for (i = 0; i < 4; i++)
        if (d[i] > 256)
            return qfalse;

	flip_it(s, (char)256, '*');

return qtrue;
}


/*********************************** COMMANDS ************************************/

/*
===========
Toggle incognito
===========
*/
void cmd_incognito(gentity_t *ent) {
	if (ent->client->sess.admin == ADM_NONE)
		return;

	if (ent->client->sess.incognito == 0) {
		ent->client->sess.incognito = 1;
			CP("cp \"You are now incognito^z!\n\"2");
	return;
	} else {
		ent->client->sess.incognito = 0;
			CP("cp \"Your status is now set to visible^z!\n\"2");
	return;
	}
}

/*
===========
Ignore user
===========
*/
void cmd_ignore(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
		if (count == 0){
			CP("print \"Client not on server^z!\n\"");
		return;
	} else if (count > 1) {
		CP(va("print \"To many people with %s in their name^z!\n\"", ent->client->pers.cmd2));
	return;
	}

		for (i = 0; i < count; i++){
			if (g_entities[nums[i]].client->sess.ignored){
				CP(va("print \"Player %s ^7is already ignored^z!\n\"", g_entities[nums[i]].client->pers.netname));
			return;
			}  else
				g_entities[nums[i]].client->sess.ignored = 1;
				AP(va("chat \"^zconsole:^7 %s has ignored player %s^z!\n\"", tag, g_entities[nums[i]].client->pers.netname));
				// Log it
				log =va("Player %s (IP:%s) has Ignored user %s.",
					ent->client->pers.netname, ent->client->sess.ip, g_entities[nums[i]].client->pers.netname);
				logEntry (ADMACT, log);
			}

return;
}

/*
===========
UnIgnore user
===========
*/
void cmd_unignore(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
		if (count == 0) {
			CP("print \"Client not on server^z!\n\"");
			return;
		}
		else if (count > 1) {
			CP(va("print \"To many people with %s in their name^z!\n\"", ent->client->pers.cmd2));
			return;
		}

		for (i = 0; i < count; i++){
			if (!g_entities[nums[i]].client->sess.ignored){
				CP(va("print \"Player %s ^7is already Unignored^z!\n\"", g_entities[nums[i]].client->pers.netname));
			return;
			}  else

				g_entities[nums[i]].client->sess.ignored = 0;
				AP(va("chat \"^zconsole:^7 %s has Unignored player %s^z!\n\"", tag, g_entities[nums[i]].client->pers.netname));
				// Log it
				log =va("Player %s (IP:%s) has unignored user %s.",
					ent->client->pers.netname, ent->client->sess.ip, g_entities[nums[i]].client->pers.netname);
				logEntry (ADMACT, log);
			}

return;
}

/*
===========
Ignore user based upon client number
===========
*/
void cmd_clientIgnore( gentity_t *ent )
{
	int	player_id;
	gentity_t	*targetclient;
	char *tag, *log;

	tag = sortTag(ent);

	player_id = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( player_id == -1 ) { //
		return;
	}

	targetclient = g_entities + player_id;

	if (targetclient->client->sess.ignored ) {
		CP(va("print \"Player %s ^7is already ignored^z!\"", targetclient->client->pers.netname));
	return;
	}

	targetclient->client->sess.ignored = 1;
	AP(va("chat \"^zconsole:^7 %s has ignored player %s^z!\"", tag, targetclient->client->pers.netname));
	// Log it
	log =va("Player %s (IP:%s) has clientIgnored user %s.",
		ent->client->pers.netname, ent->client->sess.ip, targetclient->client->pers.netname);
	logEntry (ADMACT, log);

return;
}

/*
===========
UnIgnore user based upon client number
===========
*/
void cmd_clientUnignore( gentity_t *ent )
{
	int	player_id;
	gentity_t	*targetclient;
	char *tag, *log;

	tag = sortTag(ent);

	player_id = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( player_id == -1 ) { //
		return;
	}

	targetclient = g_entities + player_id;

	if (targetclient->client->sess.ignored == 0 ) {
		CP(va("print \"Player %s ^7is already unignored^z!\"", targetclient->client->pers.netname));
	return;
	}

	targetclient->client->sess.ignored = 0;
	AP(va("chat \"^zconsole:^7 %s has unignored player %s^z!\"", tag, targetclient->client->pers.netname));
	// Log it
	log =va("Player %s (IP:%s) has clientUnignored user %s.",
		ent->client->pers.netname, ent->client->sess.ip, targetclient->client->pers.netname);
	logEntry (ADMACT, log);
return;
}

/*
===========
Kick player + optional <msg>
===========
*/
void cmd_kick(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
		if (count == 0){
			CP("print \"Client not on server^z!\n\"");
		return;
		}else if (count > 1){
			CP(va("print \"To many people with %s in their name^z!\n\"", ent->client->pers.cmd2));
		return;
		}

		for (i = 0; i < count; i++){
			trap_DropClient( nums[i], va( "^3kicked by ^3%s \n^7%s", tag, ent->client->pers.cmd3));
			AP(va("chat \"^zconsole:^7 %s has kicked player %s^z! ^3%s\n\"", tag, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3));

			// Log it
			log =va("Player %s (IP:%s) has kicked user %s. %s",
				ent->client->pers.netname, ent->client->sess.ip, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3);
			logEntry (ADMACT, log);
		}
return;
}

/*
===========
Kick player based upon clientnumber + optional <msg>
===========
*/
void cmd_clientkick( gentity_t *ent) {
	int	player_id;
	gentity_t	*targetclient;
	char *tag, *log;

	tag = sortTag(ent);

	player_id = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( player_id == -1 ) {
		return;
	}

	targetclient = g_entities + player_id;

	//kick the client
	trap_DropClient( player_id, va( "^3kicked by ^3%s \n^7%s", tag, ent->client->pers.cmd3));
	AP(va("chat \"^zconsole:^7 %s has kicked player %s^z! ^3%s\n\"", tag, targetclient->client->pers.netname, ent->client->pers.cmd3));

	// Log it
	log =va("Player %s (IP:%s) has clientKicked user %s. %s",
		ent->client->pers.netname, ent->client->sess.ip, targetclient->client->pers.netname,ent->client->pers.cmd3);
	logEntry (ADMACT, log);

return;
}

/*
===========
Ban guid
===========
*/
void cmd_banGuid(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);

	if (!svx_serverStreaming.integer) {
		CP("print \"^jError: ^7!banGuid command only works on streaming servers^j!\n\"");
	return;
	}

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
		if (count == 0){
			CP("print \"Client not on server^z!\n\"");
		return;
		}else if (count > 1){
			CP(va("print \"To many people with %s in their name^z!\n\"", ent->client->pers.cmd2));
		return;
		}

		for (i = 0; i < count; i++){
			// Ban player
			trap_SendConsoleCommand(EXEC_APPEND, va("addguid %s", g_entities[nums[i]].client->sess.guid));

			// Kick player now
			trap_DropClient( nums[i], va( "^3banned by ^3%s \n^7%s", tag, ent->client->pers.cmd3));
			AP(va("chat \"^zconsole:^7 %s has banned player %s^z! ^3%s\n\"", tag, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3));

			// Log it
			log =va("Player %s (GUID: %s) has banned user %s (GUID: %s)",
				ent->client->pers.netname, ent->client->sess.guid, g_entities[nums[i]].client->pers.netname, g_entities[nums[i]].client->sess.guid);
			logEntry (ADMACT, log);
		}
return;
}

/*
===========
Ban client guid
===========
*/
void cmd_banClientGuid(gentity_t *ent) {
	int	player_id;
	gentity_t	*targetclient;
	char *tag, *log;

	tag = sortTag(ent);

	if (!svx_serverStreaming.integer) {
		CP("print \"^jError: ^7!banClientGuid command only works on streaming servers^j!\n\"");
	return;
	}

	player_id = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( player_id == -1 ) {
		return;
	}

	targetclient = g_entities + player_id;

	// Ban player
	trap_SendConsoleCommand(EXEC_APPEND, va("addguid %s", targetclient->client->sess.guid));

	// Kick the client
	trap_DropClient( player_id, va( "^3banned by ^3%s \n^7%s", tag, ent->client->pers.cmd3));
	AP(va("chat \"^zconsole:^7 %s has banned player %s^z! ^3%s\n\"", tag, targetclient->client->pers.netname, ent->client->pers.cmd3));

	// Log it
	log =va("Player %s (GUID: %s) has banned user %s (GUID: %s)",
		ent->client->pers.netname, ent->client->sess.guid, targetclient->client->pers.netname,targetclient->client->sess.guid);
	logEntry (ADMACT, log);

return;
}

/*
===========
Temp ban guid
===========
*/
void cmd_tempbanGuid(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);

	if (!svx_serverStreaming.integer) {
		CP("print \"^jError: ^7!tempbanGuid command only works on streaming servers^j!\n\"");
	return;
	}

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
	if (count == 0){
		CP("print \"Client not on server^z!\n\"");
	return;
	} else if (count > 1){
		CP(va("print \"To many people with %s in their name^z!\n\"", ent->client->pers.cmd2));
	return;
	}

	for (i = 0; i < count; i++){
		// Ban player
		trap_SendConsoleCommand(EXEC_APPEND, va("tempbanguid %s %s", g_entities[nums[i]].client->sess.guid, ent->client->pers.cmd3));

		// Kick player now
		trap_DropClient( nums[i], va( "^3Temporarily banned by ^3%s\n^7Tempban will expire in ^3%s ^7minute(s)", tag, ent->client->pers.cmd3));
		AP(va("chat \"^zconsole:^7 %s has tempbanned player %s ^7for ^z%s ^7minute(s)^z!\n\"", tag, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3));

		// Log it
		log =va("Player %s (GUID: %s) tempbanned user %s (GUID: %s) for %s minute(s).",
			ent->client->pers.netname, ent->client->sess.guid, g_entities[nums[i]].client->pers.netname, g_entities[nums[i]].client->sess.guid, ent->client->pers.cmd3 );
		logEntry (ADMACT, log);

	}
return;
}

/*
===========
Ban ip
===========
*/
void cmd_banIp(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
		if (count == 0){
			CP("print \"Client not on server^z!\n\"");
		return;
		}else if (count > 1){
			CP(va("print \"To many people with %s in their name^z!\n\"", ent->client->pers.cmd2));
		return;
		}

		for (i = 0; i < count; i++){
			// Ban player
			if (strlen(g_entities[nums[i]].client->sess.ip) > 15) { // IPv6
				trap_SendConsoleCommand(EXEC_APPEND, va("banaddr %s", g_entities[nums[i]].client->sess.ip));
			}
			else { // IPv4
				trap_SendConsoleCommand(EXEC_APPEND, va("addip %s", g_entities[nums[i]].client->sess.ip));
			}

			// Kick player now
			trap_DropClient( nums[i], va( "^3banned by ^3%s \n^7%s", tag, ent->client->pers.cmd3));
			AP(va("chat \"^zconsole:^7 %s has banned player %s^z! ^3%s\n\"", tag, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3));

			// Log it
			log =va("Player %s (IP:%s) has (IP)banned user %s",
				ent->client->pers.netname, ent->client->sess.ip, g_entities[nums[i]].client->pers.netname);
			logEntry (ADMACT, log);
		}
return;
}

/*
===========
tempBan ip
===========
*/
void cmd_tempBanIp(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
	if (count == 0){
		CP("print \"Client not on server^z!\n\"");
	return;
	}else if (count > 1){
		CP(va("print \"To many people with %s in their name^z!\n\"", ent->client->pers.cmd2));
	return;
	}

	for (i = 0; i < count; i++){
		// TempBan player
		trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %s", nums[i], ent->client->pers.cmd3 ));

		// Kick player now
		trap_DropClient( nums[i], va( "^3temporarily banned by ^3%s \n^7Tempban will expire in ^3%s ^7minute(s)", tag, ent->client->pers.cmd3));
		AP(va("chat \"^zconsole:^7 %s has tempbanned player %s ^7for ^z%s ^7minute(s)^z!\n\"", tag, g_entities[nums[i]].client->pers.netname,ent->client->pers.cmd3));

		// Log it
		log =va("Player %s (IP:%s) tempbanned user %s by IP for %s minute(s).",
			ent->client->pers.netname, ent->client->sess.ip, g_entities[nums[i]].client->pers.netname, ent->client->pers.cmd3 );
		logEntry (ADMACT, log);
	}
	return;
}

/*
===========
Add IP
===========
*/
void cmd_addIp(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);

	if (!IPv4Valid(ent->client->pers.cmd2)) {
		CP(va("print \"%s is not a valid IPv4 address!\n\"", ent->client->pers.cmd2));
		return;
	}

	// Note that this blindly accepts what ever user inputs. Not ideal..
	trap_SendConsoleCommand(EXEC_APPEND, va("addip %s", ent->client->pers.cmd2 ));
	AP(va("chat \"^zconsole:^7 %s has added IP ^z%s ^7to banned file.\n\"", tag, ent->client->pers.cmd2));

	// Log it
	log =va("Player %s (IP:%s) added IP %s to banned file.",
		ent->client->pers.netname, ent->client->sess.ip, ent->client->pers.cmd2  );
	logEntry (ADMACT, log);
return;
}

/*
===========
Rename player
===========
*/
void cmd_rename(gentity_t *ent) {
	int          clientNum;
	gclient_t	 *client;
	char *tag, *log;
	char userinfo[MAX_INFO_STRING];

	tag = sortTag(ent);

	clientNum = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( clientNum == -1 ) {
		return;
	}

	// Ugly..
	log = va("Player %s (IP:%s) has renamed user", ent->client->pers.netname, ent->client->sess.ip);

	ent = g_entities + clientNum;
	client = ent->client;

	log = va("%s %s to", log, client->pers.netname);

	// Print first..
	AP(va("chat \"^zconsole:^7 %s has renamed player %s ^7to %s^z!\n\"", tag, client->pers.netname, ConcatArgs(3)));

	// Rename..
	trap_GetUserinfo( client->ps.clientNum, userinfo, sizeof( userinfo ) );
	Info_SetValueForKey( userinfo, "name", ConcatArgs(3));
	trap_SetUserinfo( client->ps.clientNum, userinfo );
	ClientUserinfoChanged(client->ps.clientNum);

	// Log it
	log =va("%s %s", log, ConcatArgs(3));
	// Not vital..
	if (g_extendedLog.integer > 1)
		logEntry (ADMACT, log);

return;
}

/*
===========
Slap player
===========
*/
void cmd_slap(gentity_t *ent)
{
	int clientid;
	int damagetodo;
	char *tag, *log, *log2;

	tag = sortTag(ent);
	// Sort log
	log =va("Player %s (IP:%s) has slapped ",
			ent->client->pers.netname, ent->client->sess.ip);

	clientid = atoi(ent->client->pers.cmd2);
	damagetodo = 20; // do 20 damage

	if ((clientid < 0) || (clientid >= MAX_CLIENTS) )
	{
		CP("print \"Invalid client number^z!\n\"");
		return;
	}

	if ((!g_entities[clientid].client) || (level.clients[clientid].pers.connected != CON_CONNECTED))
	{
		CP("print \"Invalid client number^z!\n\"");
		return;
	}

	if (g_entities[clientid].client->sess.sessionTeam == TEAM_SPECTATOR) {
		CP("print \"You cannot slap a spectator^z!\n\"");
	return;
	}

	ent = &g_entities[clientid];

	if (ent->client->ps.stats[STAT_HEALTH] <= 20) {
		G_Damage(ent, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ZOMBIESPIT);
		AP(va("chat \"^zconsole:^7 %s ^7was slapped to death by %s^z!\n\"", ent->client->pers.netname, tag));
		player_die( ent, ent, ent, 100000, MOD_ZOMBIESPIT );

			// Log it
			log2 =va("%s to death player %s.", log, ent->client->pers.netname);
			if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
				logEntry (ADMACT, log2);
	return;
	} else {
		G_Damage(ent, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ZOMBIESPIT);
		AP(va("chat \"^zconsole:^7 %s ^7was slapped by %s^z!\n\"", ent->client->pers.netname, tag));
		// it's broadcasted globaly but only to near by players
		G_AddEvent(ent, EV_GENERAL_SOUND, G_SoundIndex("sound/multiplayer/vo_revive.wav")); // L0 - TODO: Add sound in pack...
			// Log it
			log2 =va("%s player %s.", log, ent->client->pers.netname);
			if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
				logEntry (ADMACT, log2);
	return;
	}
}

/*
===========
Kill player
===========
*/
void cmd_kill(gentity_t *ent)
{
	int clientid;
	int damagetodo;
	char *tag, *log, *log2;

	tag = sortTag(ent);
	// Sort log
	log =va("Player %s (IP:%s) has killed ",
			ent->client->pers.netname, ent->client->sess.ip);

	clientid = atoi(ent->client->pers.cmd2);
	damagetodo = 250; // Kill the player on spot


	if ((clientid < 0) || (clientid >= MAX_CLIENTS))
	{
		CP("print \"Invalid client number^z!\n\"");
		return;
	}

	if ((!g_entities[clientid].client) || (level.clients[clientid].pers.connected != CON_CONNECTED))
	{
		CP("print \"Invalid client number^z!\n\"");
		return;
	}

	if (!g_entities[clientid].client->ps.stats[STAT_HEALTH] > 0) {
		CP("print \"Player is already dead^z!\n\"");
	return;
	}

	if (g_entities[clientid].client->sess.sessionTeam == TEAM_SPECTATOR) {
		CP("print \"You cannot kill a spectator^z!\n\"");
	return;
	}

	ent = &g_entities[clientid];
	G_Damage(ent, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ZOMBIESPIT);
	AP(va("chat \"^zconsole:^7 %s ^7was killed by %s^z!\n\"", ent->client->pers.netname, tag));
	player_die( ent, ent, ent, 100000, MOD_ZOMBIESPIT );

		// Log it
		log2 =va("%s user %s.", log, ent->client->pers.netname);
		if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
			logEntry (ADMACT, log2);
return;
}

/*
===========
Lock or Unlock game

What a mess...
===========
*/
void cmd_gamelocked(gentity_t *ent, qboolean unlock) {
	char *tag, *log;

	tag = sortTag(ent);
	// Deals with unlocking
	if (unlock) {
		if (!g_gamelocked.integer) {
			CP(va("print \"Both teams are already unlocked^z!\n\""));
		return;
		} else {
			// Axis
			if (!strcmp(ent->client->pers.cmd2,"red") || !strcmp(ent->client->pers.cmd2,"axis")) {
				if (g_gamelocked.integer == 1) {
					trap_Cvar_Set( "g_gamelocked", "0" );
					AP(va("chat \"^zconsole:^7 %s has unlocked ^1Axis ^7team^z!\n\"", tag));
				} else if (g_gamelocked.integer == 3) {
					trap_Cvar_Set( "g_gamelocked", "2" );
					AP(va("chat \"^zconsole:^7 %s has unlocked ^1Axis ^7team^z!\n\"", tag));
				} else {
					CP(va("print \"^1Axis ^7team is already unlocked^z!\n\""));
				return;
				}
			}
			// Allied
			else if (!strcmp(ent->client->pers.cmd2,"blue") || !strcmp(ent->client->pers.cmd2,"allied")) {
				if (g_gamelocked.integer == 2) {
					trap_Cvar_Set( "g_gamelocked", "0" );
					AP(va("chat \"^zconsole:^7 %s has unlocked ^4Allied ^7team^z!\n\"", tag));
				} else if (g_gamelocked.integer == 3) {
					trap_Cvar_Set( "g_gamelocked", "1" );
					AP(va("chat \"^zconsole:^7 %s has unlocked ^1Allied ^7team^z!\n\"", tag));
				} else {
					CP(va("print \"^4Allied ^7team is already unlocked^z!\n\""));
				return;
				}
			// Both
			} else if (!strcmp(ent->client->pers.cmd2,"both")) {
				if (g_gamelocked.integer) {
					trap_Cvar_Set( "g_gamelocked", "0" );
					AP(va("chat \"^zconsole:^7 %s has unlocked ^3Both ^7teams^z!\n\"", tag));
				} else {
					CP(va("print \"^3Both ^7teams are already unlocked^z!\n\""));
				return;
				}
			}
		// Log it
		log =va("Player %s (IP:%s) has unlocked %s team(s).",
		ent->client->pers.netname, ent->client->sess.ip, ent->client->pers.cmd2);
		logEntry (ADMACT, log);
		}
	return;
	// Deals with locking
	} else {
		if (g_gamelocked.integer == 3) {
			CP(va("print \"Both teams are already locked^z!\n\""));
		return;
		} else {
			// Axis
			if (!strcmp(ent->client->pers.cmd2,"red") || !strcmp(ent->client->pers.cmd2,"axis")) {
				if (!g_gamelocked.integer) {
					trap_Cvar_Set( "g_gamelocked", "1" );
					AP(va("chat \"^zconsole:^7 %s has locked ^1Axis ^7team^z!\n\"", tag));
				} else if (g_gamelocked.integer == 2) {
					trap_Cvar_Set( "g_gamelocked", "3" );
					AP(va("chat \"^zconsole:^7 %s has locked ^1Axis ^7team^z!\n\"", tag));
				} else {
					CP(va("print \"^1Axis ^7team is already locked^1!\n\""));
				return;
				}
			}
			// Allied
			else if (!strcmp(ent->client->pers.cmd2,"blue") || !strcmp(ent->client->pers.cmd2,"allied")) {
				if (!g_gamelocked.integer) {
					trap_Cvar_Set( "g_gamelocked", "2" );
					AP(va("chat \"^zconsole:^7 %s has locked ^4Allied ^7team^z!\n\"", tag));
				} else if (g_gamelocked.integer == 1) {
					trap_Cvar_Set( "g_gamelocked", "3" );
					AP(va("chat \"^zconsole:^7 %s has locked ^1Allied ^7team^z!\n\"", tag));
				} else {
					CP(va("print \"^4Allied ^7team is already unlocked^z!\n\""));
				return;
				}
			// Both
			} else if (!strcmp(ent->client->pers.cmd2,"both")) {
				if (g_gamelocked.integer != 3) {
					trap_Cvar_Set( "g_gamelocked", "3" );
					AP(va("chat \"^zconsole:^7 %s has locked ^3Both ^7teams^z!\n\"", tag));
				} else {
					CP(va("print \"^3Both ^7teams are already locked^z!\n\""));
				return;
				}
			}
		// Log it
		log =va("Player %s (IP:%s) has locked %s team(s).",
		ent->client->pers.netname, ent->client->sess.ip, ent->client->pers.cmd2);
		logEntry (ADMACT, log);
		}
	return;
	}
}

/*
===========
Force user to spectators
===========
*/
void cmd_specs(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
		if (count == 0){
			CP("print \"Client not on server^z!\n\"");
		return;
	}else if (count > 1){
		CP(va("print \"To many people with %s in their name^z!\n\"", ent->client->pers.cmd2));
	return;
	}
		for (i = 0; i < count; i++){

			if (g_entities[nums[i]].client->sess.sessionTeam == TEAM_SPECTATOR){
				CP(va("print \"Player %s ^7is already a spectator^z!\n\"", g_entities[nums[i]].client->pers.netname));
			return;
			}  else
				SetTeam( &g_entities[nums[i]], "spectator", qtrue );
				AP(va("chat \"^zconsole:^7 %s has forced player %s ^7to ^3spectators^z!\n\"", tag, g_entities[nums[i]].client->pers.netname));

					// Log it
					log =va("Player %s (IP:%s) has forced user %s to spectators.",
					ent->client->pers.netname, ent->client->sess.ip, g_entities[nums[i]].client->pers.netname);
					if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
						logEntry (ADMACT, log);
			}
return;
}

/*
===========
Force user to Axis
===========
*/
void cmd_axis(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
		if (count == 0){
			CP("print \"Client not on server^z!\n\"");
		return;
	}else if (count > 1){
		CP(va("print \"To many people with %s in their name^z!\n\"", ent->client->pers.cmd2));
	return;
	}
		for (i = 0; i < count; i++){

			if (g_entities[nums[i]].client->sess.sessionTeam == TEAM_RED){
				CP(va("print \"Player %s ^7is already playing^z!\n\"", g_entities[nums[i]].client->pers.netname));
			return;
			}  else
				SetTeam( &g_entities[nums[i]], "red", qtrue );
				AP(va("chat \"^zconsole:^7 %s has forced player %s ^7to ^1Axis ^7team^z!\n\"", tag, g_entities[nums[i]].client->pers.netname));

					// Log it
					log =va("Player %s (IP:%s) has forced user %s into Axis team.",
					ent->client->pers.netname, ent->client->sess.ip, g_entities[nums[i]].client->pers.netname);
					if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
						logEntry (ADMACT, log);
			}
return;
}

/*
===========
Force user to Allied
===========
*/
void cmd_allied(gentity_t *ent) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *tag, *log;

	tag = sortTag(ent);

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
		if (count == 0){
			CP("print \"Client not on server^z!\n\"");
		return;
	}else if (count > 1){
		CP(va("print \"To many people with %s in their name^z!\n\"", ent->client->pers.cmd2));
	return;
	}
		for (i = 0; i < count; i++){

			if (g_entities[nums[i]].client->sess.sessionTeam == TEAM_BLUE){
				CP(va("print \"Player %s ^7is already playing^z!\n\"", g_entities[nums[i]].client->pers.netname));
			return;
			}  else
				SetTeam( &g_entities[nums[i]], "blue", qtrue );
				AP(va("chat \"^zconsole:^7 %s has forced player %s ^7into ^4Allied ^7team^z!\n\"", tag, g_entities[nums[i]].client->pers.netname));

					// Log it
					log =va("Player %s (IP:%s) has forced user %s into Axis team.",
					ent->client->pers.netname, ent->client->sess.ip, g_entities[nums[i]].client->pers.netname);
					if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
						logEntry (ADMACT, log);
			}
return;
}

/*
===========
Execute command
===========
*/
void cmd_exec(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);

	if (!strcmp(ent->client->pers.cmd3,"@"))
		CP(va("print \"^zInfo: ^7%s has been executed^z!\n\"", ent->client->pers.cmd2));
	else
		AP(va("print \"^zconsole:^7 %s has executed ^z%s^7 config^z.\n\"", tag, ent->client->pers.cmd2));

	trap_SendConsoleCommand( EXEC_INSERT, va("exec \"%s\"", ent->client->pers.cmd2));

	// Log it
	log =va("Player %s (IP:%s) has executed %s config.",
		ent->client->pers.netname, ent->client->sess.ip, ent->client->pers.cmd2);
	logEntry (ADMACT, log);

return;
}

/*
===========
Nextmap
===========
*/
void cmd_nextmap(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	AP(va("chat \"^zconsole:^7 %s has set ^znextmap ^7in rotation^z.\n\"", tag));
	trap_SendConsoleCommand( EXEC_APPEND, va("vstr nextmap"));

	// Log it
	log =va("Player %s (IP:%s) has set nextmap.",
		ent->client->pers.netname, ent->client->sess.ip);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

return;
}

/*
===========
Load map
===========
*/
void cmd_map(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	AP(va("chat \"^zconsole:^7 %s has loaded ^z%s ^7map^z. \n\"", tag, ent->client->pers.cmd2));
	trap_SendConsoleCommand( EXEC_APPEND, va("map %s", ent->client->pers.cmd2));

	// Log it
	log =va("Player %s (IP:%s) has loaded %s map.",
		ent->client->pers.netname, ent->client->sess.ip, g_entities->client->pers.cmd2);
	logEntry (ADMACT, log);

return;
}

/*
===========
Vstr

Loads next map in rotation (if any)
===========
*/
void cmd_vstr(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	AP(va("chat \"^zconsole:^7 %s set vstr to ^z%s^7.\n\"", tag, ent->client->pers.cmd2));
	trap_SendConsoleCommand( EXEC_APPEND, va("vstr %s", ent->client->pers.cmd2));

	// Log it
	log =va("Player %s (IP:%s) has set vstr to %s",
		ent->client->pers.netname, ent->client->sess.ip, g_entities->client->pers.cmd2);
	logEntry (ADMACT, log);

return;
}

/*
===========
Renameon/renameoff

Takes or restores ability to rename from client.
NOTE: Taking ability to rename lasts only for that round..
===========
*/
void cmd_nameHandle(gentity_t *ent, qboolean revoke) {
	int	player_id;
	gentity_t	*targetclient;
	char *tag, *log, *action;

	tag = sortTag(ent);

	player_id = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( player_id == -1 ) {
		return;
	}

	targetclient = g_entities + player_id;

	if (revoke && targetclient->client->pers.nameLocked) {
		CP(va("print \"^zError: ^7%s ^7is already name locked!\n\"", targetclient->client->pers.netname));
	return;
	} else if (!revoke && !targetclient->client->pers.nameLocked) {
		CP(va("print \"^zError: ^7%s ^7already can rename!\n\"", targetclient->client->pers.netname));
	return;
	}

	action = revoke ? "revoked" : "restored";
	AP(va("chat \"^zconsole:^7 %s has %s %s^7s ability to rename.\n\"", tag, action, targetclient->client->pers.netname));
	targetclient->client->pers.nameLocked = revoke;

	// Log it
	log =va("Player %s (GUID: %s) has %s %s^7s ability to rename",
		ent->client->pers.netname, ent->client->sess.guid, action, targetclient->client->pers.netname);
	logEntry (ADMACT, log);
return;
}



/*
===========
Center prints message to all
===========
*/
void cmd_cpa(gentity_t *ent) {
	char *s, *log;

	s = ConcatArgs(2);
	AP(va("cp \"^1ADMIN WARNING^7! \n%s\n\"", s));

	// Log it
	log =va("Player %s (IP:%s) issued CPA warning: %s",
		ent->client->pers.netname, ent->client->sess.ip, s);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

return;
}

/*
===========
Shows message to selected user in center print
===========
*/
void cmd_cp(gentity_t *ent) {
	int	player_id;
	gentity_t	*targetclient;
	char *s, *log;

	s = ConcatArgs(3);

	player_id = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( player_id == -1 ) {
		return;
	}

	targetclient = g_entities + player_id;

	// CP to user
	CPx(targetclient-g_entities, va("cp \"^1ADMIM WARNING^7! \n%s\n\"2", s));

	// Log it
	log =va("Player %s (IP:%s) issued to user %s CP warning: %s",
		ent->client->pers.netname, ent->client->sess.ip, targetclient->client->pers.netname, s);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

return;
}

/*
===========
Shows message to all in console and center print
===========
*/
void cmd_warn(gentity_t *ent) {
	char *s, *log;

	s = ConcatArgs(2);
	AP(va("cp \"^1ADMIM WARNING^7: \n%s\n\"2", s));
	AP(va("chat \"^1ADMIM WARNING^7: \n%s\n\"", s));

	// Log it
	log =va("Player %s (IP:%s) issued global warning: %s",
		ent->client->pers.netname, ent->client->sess.ip, s);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

return;
}

/*
===========
Shows message to all in console
===========
*/
void cmd_chat(gentity_t *ent) {
	char *s, *log;

	s = ConcatArgs(2);
	AP(va("chat \"^1ADMIM WARNING^7: \n%s\n\"", s));

	// Log it
	log =va("Player %s (IP:%s) issued CHAT warning: %s",
		ent->client->pers.netname, ent->client->sess.ip, s);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

return;
}
/*
===========
Cancels any vote in progress
===========
*/
void cmd_cancelvote(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	if ( level.voteTime ) {
		level.voteNo = level.numConnectedClients;
		CheckVote();
		AP(va("cp \"%s has ^3Cancelled the vote.\n\"2", tag));
		AP("chat \"^zconsole:^7 Turns out everyone voted No^z!\n\"");

		// Log it
		log =va("Player %s (IP:%s) cancelled a vote.",
		ent->client->pers.netname, ent->client->sess.ip);
		if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
			logEntry (ADMACT, log);
		return;
		}
return;
}

/*
===========
Passes any vote in progress
===========
*/
void cmd_passvote(gentity_t *ent){
	char *tag, *log;

	tag = sortTag(ent);
	if ( level.voteTime ) {
		level.voteYes = level.numConnectedClients;
		CheckVote();
		AP(va("cp \"%s has ^3Passed the vote.\n\"2", tag));
		AP("chat \"^zconsole:^7 Turns out everyone voted Yes^z!\n\"");

		// Log it
		log =va("Player %s (IP:%s) passed a vote.",
		ent->client->pers.netname, ent->client->sess.ip);
		if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
			logEntry (ADMACT, log);
		return;
	}
return;
}

/*
===========
Map restart
===========
*/
void cmd_restart(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	AP(va("chat \"^zconsole:^7 %s has ^zrestarted ^7map^z.\n\"", tag));
	trap_SendConsoleCommand( EXEC_APPEND, va("map_restart"));

	// Log it
	log =va("Player %s (IP:%s) has restarted map.",
		ent->client->pers.netname, ent->client->sess.ip);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
	logEntry (ADMACT, log);

return;
}

/*
===========
Reset match
===========
*/
void cmd_resetmatch(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	AP(va("chat \"^zconsole:^7 %s has ^zresetted ^7match^z.\n\"", tag));
	trap_SendConsoleCommand( EXEC_APPEND, va("resetmatch"));

	// Log it
	log =va("Player %s (IP:%s) has resetted match.",
		ent->client->pers.netname, ent->client->sess.ip);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

return;
}

/*
===========
Swap teams
===========
*/
void cmd_swap(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);
	AP(va("chat \"^zconsole:^7 %s has ^zswapped ^7the teams^z.\n\"", tag));
	trap_SendConsoleCommand( EXEC_APPEND, va("swap_teams"));

	// Log it
	log =va("Player %s (IP:%s) has swapped teams.",
		ent->client->pers.netname, ent->client->sess.ip);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

return;
}

/*
===========
Shuffle
===========
*/
void cmd_shuffle(gentity_t *ent) {
	char *tag, *log;
	int count=0, tmpCount, i;
	int players[MAX_CLIENTS];

	tag = sortTag(ent);
	memset(players, -1, sizeof(players));

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if ((!g_entities[i].inuse) || (level.clients[i].pers.connected != CON_CONNECTED))
			continue;

		//ignore spectators
		if ((level.clients[i].sess.sessionTeam != TEAM_RED) && (level.clients[i].sess.sessionTeam != TEAM_BLUE))
			continue;

		players[count] = i;
		count++;
	}

	tmpCount = count;

	for (i = 0; i < count; i++)
	{
		int j;

		do {
			j = (rand() % count);
		} while (players[j] == -1);

		if (i & 1)
			level.clients[players[j]].sess.sessionTeam = TEAM_BLUE;
		else
			level.clients[players[j]].sess.sessionTeam = TEAM_RED;

		ClientUserinfoChanged(players[j]);
		ClientBegin(players[j]);

		players[j] = players[tmpCount-1];
		players[tmpCount-1] = -1;
		tmpCount--;
	}

	AP(va("print \"^zconsole:^7 %s has ^zshuffled ^7teams^z.\n\"", tag));
	trap_SendConsoleCommand(EXEC_APPEND, va("resetmatch %i\n", GS_WARMUP));

	// Log it
	log =va("Player %s (IP:%s) has shuffled teams.",
		ent->client->pers.netname, ent->client->sess.ip);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

return;
}

/*
==================
Move lagged out or downloading clients to spectators
==================
*/
qboolean cmd_specs999(gentity_t *ent) {
	int i;
	qboolean moved=qfalse;
	char *tag, *log;

	tag = sortTag(ent);
	for(i = 0; i < level.maxclients; i++) {
		ent = &g_entities[i];
		if(!ent->client) continue;
		if(ent->client->pers.connected != CON_CONNECTED) continue;
		if(ent->client->ps.ping >= 999) {
			SetTeam(ent, "s", qtrue);
			moved = qtrue;
		}
	}

	if (moved==qtrue)
		AP(va("chat \"^zconsole:^7 %s moved all lagged out players to spectators^z!\n\"", tag));
	else
		CP("print \"No one to move to spectators^z!\n\"");

		// Log it
	log =va("Player %s (IP:%s) forced all 999 to spectators.",
		ent->client->pers.netname, ent->client->sess.ip);
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);

return qtrue;
}

/*
==================
Reveal location of a player.
==================
*/
void cmd_revealCamper(gentity_t *ent) {
/*
	char location[64];
	int	clientNum;
	char *tag, *log, *log2;

	tag = sortTag(ent);
	log2 = va("Player %s (IP:%s)",
		ent->client->pers.netname, ent->client->sess.ip );
	clientNum = ClientNumberFromString( ent, ent->client->pers.cmd2 );
	if ( clientNum == -1 ) {
		return; //check if target is not a client
	}

	// Give values to these
	ent = g_entities + clientNum;

	Team_GetLocationMsg(ent, location, sizeof(location), qtrue);
	AP(va("chat \"^zconsole:^7 %s has releaved that player %s^7 is hidding at ^z%s^7.\n\"", tag, ent->client->pers.netname, location));

	// Log it
	log =va("%s has revealed %s location.",log2, ent->client->pers.netname );
	if (g_extendedLog.integer >= 2) // Only log this if it is set to 2+
		logEntry (ADMACT, log);
*/
}

/*
==================
Pause
==================
*/
void cmd_pause(gentity_t *ent, qboolean fPause) {
	char* status[2] = {"^3UN", "^3"};
	char *log;

	if ( g_gamestate.integer != GS_PLAYING ) {
		CP("print \"^jError: ^7Pause can only be issued during a match!\n\"");
		return;
	}

	if (level.numPlayingClients == 0) {
		CP("print \"^jError: ^7You cannot use pause feature with no playing clients..\n\"");
		return;
	}

	if ((!level.alliedPlayers && !level.axisPlayers) && fPause) {
		CP("print \"^jError: ^7Pause can only be used when at least 1 team has a player!\n\"");
		return;
	}

	if ((PAUSE_UNPAUSING >= level.paused && !fPause) || (PAUSE_NONE != level.paused && fPause)) {
		CP(va("print \"^1Error^7: The match is already %sPAUSED!\n\"", status[fPause]));
		return;
	}

	// Trigger the auto-handling of pauses
	if (fPause) {
		G_handlePause(qtrue, (ent ? 1 + ent - g_entities : 0));
	}
	else {
		G_handlePause(qfalse, 0);
	}

	AP(va("chat \"^zconsole: ^7%s has ^3%sPAUSED ^7the match!\n\"", status[fPause], sortTag(ent)));

    if (g_gameStatslog.integer) {
        G_writeGeneralEvent(ent , ent, " ", (!fPause) ? eventUnpause : eventPause);  // might want to distinguish between player and admin here?
    }

	log = va("Player %s (IP:%s) %s",
		ent->client->pers.netname, ent->client->sess.ip, !fPause ? "resumed the match." : "paused a match.");
	if (g_extendedLog.integer)
		logEntry (ADMACT, log);
	
	return;
}

/*
==================
Speclock/unlock
==================
*/
qboolean specAlready(int team, qboolean lock) {
	if (team > 0 && team < 3) {
		if (teamInfo[team].spec_lock == lock)
			return qtrue;
		else
			return qfalse;
	} else if ( team == 3 ) {
		if ((teamInfo[TEAM_RED].spec_lock == lock) &&
			(teamInfo[TEAM_BLUE].spec_lock == lock) )
			return qtrue;
		else
			return qfalse;
	}
return qfalse;
}

void cmd_specHandle( gentity_t *ent, qboolean lock ) {
	int team;
	char *act = ((lock) ? "locked" : "unlocked");

	if (!ent->client->pers.cmd2) {
		CP(va("print \"^jError: ^7You need to specify a team!\nUse ?spec%s for help.\n\"", (lock ? "lock" : "unlock") ));
		return;
	}

#define STM(x) !(strcmp(ent->client->pers.cmd2,x))

	if (STM("both")) {
		team = 3;
	} else if ( STM("red") || STM("axis") )	{
		team = TEAM_RED;
	} else if ( STM("blue") || STM("allied") || STM("allies") ) {
		team = TEAM_BLUE;
	} else {
		CP(va("print \"^jError: ^7Unknown argument ^j%s^7!\nUse ?spec%s for help.\n\"",
			ent->client->pers.cmd2, (lock ? "lock" : "unlock") ));
		return;
	}

	if (specAlready(team, lock)) {
		CP(va("print \"%s already spec%s! \n\"",
			((team==3) ? "^3Both ^7teams are": va("Team %s is",aTeams[team]) ), act ));
		return;
	}

	// Sanity check
	if (lock) {
		if (team == TEAM_BLUE && !level.alliedPlayers) {
			CP(va("print \"%s team has no players!\n\"", aTeams[team] ));
			return;
		} else if (team == TEAM_RED && !level.axisPlayers) {
			CP(va("print \"%s team has no players!\n\"", aTeams[team] ));
			return;
		} else if (team == TEAM_SPECTATOR && (!level.axisPlayers || !level.alliedPlayers) ) {
			CP("print \"Not all teams have players^j!\n\"");
			return;
		}
	}

	if (team != 3) {
		G_updateSpecLock( team, lock );
	} else {
		G_updateSpecLock( TEAM_RED, lock );
		G_updateSpecLock( TEAM_BLUE, lock );
	}

	aTeams[team] = (team==3) ? "^3Both^7" : aTeams[team];
	AP(va("chat \"^zconsole: ^7%s has spec%s %s team%s\"", sortTag(ent), act, aTeams[team], ((team==3) ? "s" : "" )));
}

/*
===========
Un/Ready all

Ready or unready all..
===========
*/
void cmd_readyHandle( gentity_t *ent, qboolean unready ) {
	char *msg = ((unready) ? "UNREADY" : "READY");

	if (!g_tournament.integer) {
		CP("print \"Tourny mode is disabled! Command ignored..\n\"");
		return;
	}

	if (!unready) {
		if (g_gamestate.integer != GS_WARMUP) {
			CP("print \"^nREADYALL ^7command can only be used in warmup!\n\"");
			return;
		}
		G_readyStart();
	} else {
		if (g_gamestate.integer != GS_WARMUP_COUNTDOWN) {
			CP("print \"^nUNREADYALL ^7command can only be used during countdown!\n\"");
			return;
		}
		G_readyReset(qtrue);
	}

	AP(va("chat \"^zconsole: ^7%s has ^n%s ^7ALL players..\n\"", sortTag(ent), msg));
	AP(va("print \"^z>> ^7%s ^z%s ALL players..\n\"", ent->client->pers.netname));
	return;
}

/*
===========
Getstatus

Prints IP's, GUIDs and some match info..
===========
*/
void cmd_getstatus(gentity_t *ent) {
	int	j;
	// uptime
	int secs = level.time / 1000;
	int mins = (secs / 60) % 60;
	int hours = (secs / 3600) % 24;
	int days = (secs / (3600 * 24));
	int teamLocked = g_gamelocked.integer;
	qboolean teamSpecLocked=qfalse;
	qtime_t ct;
	trap_RealTime(&ct);

	if (teamInfo[TEAM_RED].spec_lock || teamInfo[TEAM_BLUE].spec_lock )
		teamSpecLocked = qtrue;

	CP(va("print \"\n^7Server: %s    ^7%02d:%02d:%02d ^d(^7%02d %s %d^d)\n\"", sv_hostname.string, ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday, cMonths[ct.tm_mon], 1900+ct.tm_year));
	CP("print \"^d-----------------------------------------------------------------------------\n\"");
	CP("print \"^7Slot : Team : Name       : ^dIP              ^7: ^dGuid            ^7: Status \n\"");
	CP("print \"^d-----------------------------------------------------------------------------\n\"");

	for ( j = 0; j <= (MAX_CLIENTS-1); j++) {

		if ( g_entities[j].client ) {
			char *team, *slot, *ip, *tag, *sortTag, *extra;
			char n1[MAX_NETNAME];
			char n2[MAX_NETNAME];
			char *guid;

			DecolorString(g_entities[j].client->pers.netname, n1);
			SanitizeString(n1, n2);
			Q_CleanStr(n2);
			n2[10] = 0;

			// player is connecting
			if ( g_entities[j].client->pers.connected == CON_CONNECTING ) {
				CP(va("print \"%3d  : >><< : %-10s : ^d>>Connecting<<  ^7:\n\"", j, n2));
			}

			// player is connected
			if ( g_entities[j].client->pers.connected == CON_CONNECTED ) {

					// Sort it :C
					sortTag = "";
					slot = va("%3d", j);
					team = (g_entities[j].client->sess.sessionTeam == TEAM_SPECTATOR) ? "^3SPEC^7" :
						(g_entities[j].client->sess.sessionTeam == TEAM_RED ? "^1Axis^7" : "^4Alld^7" );
					ip = va("%s", SanitizeClientIP(g_entities[j].client->sess.ip, (ent->client->sess.admin == ADM_NONE)));

					switch (g_entities[j].client->sess.admin) {
						case ADM_NONE:
							sortTag = "";
						break;
						case ADM_1:
							sortTag = (g_entities[j].client->sess.incognito) ? va("%s ^7*", a1_tag.string) : va("%s", a1_tag.string);
						break;
						case ADM_2:
							sortTag = (g_entities[j].client->sess.incognito) ? va("%s ^7*", a2_tag.string) : va("%s", a2_tag.string);
						break;
						case ADM_3:
							sortTag = (g_entities[j].client->sess.incognito) ? va("%s ^7*", a3_tag.string) : va("%s", a3_tag.string);
						break;
						case ADM_4:
							sortTag = (g_entities[j].client->sess.incognito) ? va("%s ^7*", a4_tag.string) : va("%s", a4_tag.string);
						break;
						case ADM_5:
							sortTag = (g_entities[j].client->sess.incognito) ? va("%s ^7*", a5_tag.string) : va("%s", a5_tag.string);
						break;
					}
					// Sort Admin tags..
					tag = "";
					extra = (g_entities[j].client->sess.admin == ADM_NONE && g_entities[j].client->sess.ignored) ?
						((g_entities[j].client->sess.ignored > 1) ? "^zIgnored" : "^3Ignored") : "";
					if (ent->client->sess.admin == ADM_NONE) {
						tag = (g_entities[j].client->sess.admin != ADM_NONE && !g_entities[j].client->sess.incognito) ? sortTag : extra;
					} else if (ent->client->sess.admin != ADM_NONE) {
						tag = (g_entities[j].client->sess.admin == ADM_NONE && g_entities[j].client->sess.ignored) ? "^1Ignored" : sortTag;
					}
					// Sort guid for streaming or non streaming servers..
					guid = (svx_serverStreaming.integer) ? g_entities[j].client->sess.guid : "GUIDS DISABLED!";
					// Specing speclocked team (This will override ignored tag but so be it..).
					if (g_entities[j].client->sess.sessionTeam == TEAM_SPECTATOR &&
						!g_entities[j].client->sess.admin &&
						teamSpecLocked ) {
						if (g_entities[j].client->sess.specInvited == 1 )
							tag = "Spec. Axis";
						else if (g_entities[j].client->sess.specInvited == 2 )
							tag = "Spec. Allies";
						else if (g_entities[j].client->sess.specInvited == 3 )
							tag = "Spec. Both";
						else if (teamSpecLocked > 0 && !g_entities[j].client->sess.specInvited )
							tag = "^nSpeclocked";
					// Hidden admins
					} else if (g_entities[j].client->sess.sessionTeam == TEAM_SPECTATOR &&
						g_entities[j].client->sess.admin &&
						g_entities[j].client->sess.incognito) {
						tag = "^zSpec. Both"; // Silver color should mark them as 'read only' eg. can't be uninvited.
					}

						// Print it now
						CP(va("print \"%-4s : %s : %-10s : ^d%-15s ^7: ^d%-15s ^7: %-12s \n\"",
							slot,
							team,
							n2,
							ip,
							guid,
							tag
						));
			}
		}
	}
	CP("print \"^d-----------------------------------------------------------------------------\n\"");
	CP( va("print \"^7Uptime: ^d%d ^7day%s ^d%d ^7hours ^d%d ^7minutes\n\"", days, (days != 1 ? "s" : ""), hours, mins));
	CP("print \"\n\"");

		CP(va("print \"^zAxis team : %-9s ^z| Allied team : %-9s\n\"",
			( ((teamLocked == 3) || (teamLocked == 1)) ? "^dLocked!" : "^7Open" ),
			( ((teamLocked == 3) || (teamLocked == 2)) ? "^dLocked!" : "^7Open" )
		));

		CP(va("print \"^zAxis specs: %-9s ^z| Allied specs: %-9s\n\"",
			( ((teamInfo[TEAM_RED].spec_lock) || (teamInfo[TEAM_SPECTATOR].spec_lock)) ? "^dLocked!" : "^7Open" ),
			( ((teamInfo[TEAM_BLUE].spec_lock) || (teamInfo[TEAM_SPECTATOR].spec_lock)) ? "^dLocked!" : "^7Open" )
		));


	CP("print \"\n\"");
return;
}

/*********************************** INTERACTIONS ************************************/
/*
===========
List commands
===========
*/
void cmd_listCmds(gentity_t *ent) {
	char *cmds;

	if (!adm_help.integer) {
		CP("print \"Admin commands list is disabled on this server^1!\n\"");
	return;
	}

	// Keep an eye on this..so it's not to big..
	cmds = "incognito list_cmds ignore unignore clientignore clientunignore kick clientkick slap kill "
		   "lock unlock specs axis allies exec nextmap map cpa cp chat warn cancelvote passvote restart "
		   "reset swap shuffle spec999 whereis pause unpause rename renameon renameoff vstr banip tempbanip addip"
		   "speclock specunlock readyall undreadyall rememberme forgetme cookies destroycookie viewcookie";

	// Shows this only if server is streaming..
	if (svx_serverStreaming.integer)
		cmds = va("%s banguid banclientguid tempbanguid", cmds );

	if (ent->client->sess.admin == ADM_1)
		CP( va("print \"^dAvailable commands are:^7\n%s\n^dUse ? for help with command. E.g. ?incognito.\n\"", a1_cmds.string ));
	else if (ent->client->sess.admin == ADM_2)
		CP( va("print \"^dAvailable commands are:^7\n%s\n^dUse ? for help with command. E.g. ?incognito.\n\"", a2_cmds.string ));
	else if (ent->client->sess.admin == ADM_3)
		CP( va("print \"^dAvailable commands are:^7\n%s\n^dUse ? for help with command. E.g. ?incognito.\n\"", a3_cmds.string ));
	else if (ent->client->sess.admin == ADM_4)
		CP( va("print \"^dAvailable commands are:^7\n%s\n^2Use ? for help with command. E.g. ?incognito.\n\"", a4_cmds.string ));
	else if (ent->client->sess.admin == 5 && !a5_allowAll.integer)
		CP( va("print \"^dAvailable commands are:^7\n%s\n^2Use ? for help with command. E.g. ?incognito.\n\"", a5_cmds.string ));
	else if (ent->client->sess.admin == 5 && a5_allowAll.integer)
		CP( va("print \"^dAvailable commands are:^7\n%s\n^dAdditinal server commands:^7\n%s\n^dUse ? for help with command. E.g. ?incognito.\n\"", cmds, a5_cmds.string ));

return;
}

/*
===========
Admin commands
===========
*/
qboolean do_cmds(gentity_t *ent) {
	char alt[128];
	char cmd[128];

	admCmds(ent->client->pers.cmd1, alt, cmd, qfalse);

	if (!strcmp(cmd,"incognito"))			{ if (canUse(ent, qtrue)) cmd_incognito(ent); else cantUse(ent);	return qtrue;}
	else if (!strcmp(cmd,"list_cmds"))		{ cmd_listCmds(ent);	return qtrue;}
	else if (!strcmp(cmd,"ignore"))			{ if (canUse(ent, qtrue)) cmd_ignore(ent);	else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"unignore"))		{ if (canUse(ent, qtrue)) cmd_unignore(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"clientignore"))	{ if (canUse(ent, qtrue)) cmd_clientIgnore(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"clientunignore"))	{ if (canUse(ent, qtrue)) cmd_clientUnignore(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"kick"))			{ if (canUse(ent, qtrue)) cmd_kick(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"clientkick"))		{ if (canUse(ent, qtrue)) cmd_clientkick(ent);	else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"slap"))			{ if (canUse(ent, qtrue)) cmd_slap(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"kill"))			{ if (canUse(ent, qtrue)) cmd_kill(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"lock"))			{ if (canUse(ent, qtrue)) cmd_gamelocked(ent, qfalse); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"unlock"))			{ if (canUse(ent, qtrue)) cmd_gamelocked(ent, qtrue); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"specs"))			{ if (canUse(ent, qtrue)) cmd_specs(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"axis"))			{ if (canUse(ent, qtrue)) cmd_axis(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"allies"))			{ if (canUse(ent, qtrue)) cmd_allied(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"exec"))			{ if (canUse(ent, qtrue)) cmd_exec(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"nextmap"))		{ if (canUse(ent, qtrue)) cmd_nextmap(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"map"))			{ if (canUse(ent, qtrue)) cmd_map(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"cpa"))			{ if (canUse(ent, qtrue)) cmd_cpa(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"cp"))			    { if (canUse(ent, qtrue)) cmd_cp(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"chat"))			{ if (canUse(ent, qtrue)) cmd_chat(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"warn"))			{ if (canUse(ent, qtrue)) cmd_warn(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"cancelvote"))		{ if (canUse(ent, qtrue)) cmd_cancelvote(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"passvote"))		{ if (canUse(ent, qtrue)) cmd_passvote(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"restart"))		{ if (canUse(ent, qtrue)) cmd_restart(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"reset"))			{ if (canUse(ent, qtrue)) cmd_resetmatch(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"swap"))			{ if (canUse(ent, qtrue)) cmd_swap(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"shuffle"))		{ if (canUse(ent, qtrue)) cmd_shuffle(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"spec999"))		{ if (canUse(ent, qtrue)) cmd_specs999(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"whereis"))		{ if (canUse(ent, qtrue)) cmd_revealCamper(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"pause"))			{ if (canUse(ent, qtrue)) cmd_pause(ent, qfalse); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"unpause"))		{ if (canUse(ent, qtrue)) cmd_pause(ent, qtrue); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"banguid"))		{ if (canUse(ent, qtrue)) cmd_banGuid(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"banclientguid"))	{ if (canUse(ent, qtrue)) cmd_banClientGuid(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"tempbanguid"))	{ if (canUse(ent, qtrue)) cmd_tempbanGuid(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"banip"))			{ if (canUse(ent, qtrue)) cmd_banIp(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"tempbanip"))		{ if (canUse(ent, qtrue)) cmd_tempBanIp(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"addip"))			{ if (canUse(ent, qtrue)) cmd_addIp(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"rename"))			{ if (canUse(ent, qtrue)) cmd_rename(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"vstr"))			{ if (canUse(ent, qtrue)) cmd_vstr(ent); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"renameon"))		{ if (canUse(ent, qtrue)) cmd_nameHandle(ent, qfalse); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"renameoff"))		{ if (canUse(ent, qtrue)) cmd_nameHandle(ent, qtrue); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"speclock"))		{ if (canUse(ent, qtrue)) cmd_specHandle(ent, qtrue); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"specunlock"))		{ if (canUse(ent, qtrue)) cmd_specHandle(ent, qfalse); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"readyall"))		{ if (canUse(ent, qtrue)) cmd_readyHandle(ent, qfalse); else cantUse(ent); return qtrue;}
	else if (!strcmp(cmd,"unreadyall"))		{ if (canUse(ent, qtrue)) cmd_readyHandle(ent, qtrue); else cantUse(ent); return qtrue;}

	// Any other command
	else if (canUse(ent, qfalse))			{ cmdCustom(ent, cmd); return qtrue; }

// It failed on all checks..
else { CP( va("print \"Command ^j%s ^7was not found^1!\n\"", cmd )); return qfalse; }

}

/*
===========
Admin help
===========
*/
typedef struct {
	char *command;
	char *help;
	char *usage;
} helpCmd_reference_t;

#define _HELP(x,y,z) {x, y, z},
/**
 * Fairly straight forward approach _HELP(COMMAND, DESCRIPTION, USAGE)
 * Alternatively, usage can be empty.
 * Add new as needed..
 */
static const helpCmd_reference_t helpInfo[] = {
	_HELP("help", "Prints help about specific command.", "?COMMAND")
	_HELP("login", "Logs you in as Administrator.", NULL)
	_HELP("@login", "Silently logs you in as Administrator.", NULL)
	_HELP("logout", "Removes you from Administrator status.", NULL)
	_HELP("incognito", "Toggles your Admin status from hidden to visible or other way around.", NULL)
	_HELP("getstatus", "Shows basic info of all connected players.", NULL)
	_HELP("list_cmds", "Shows all available commands for your level.", NULL)
	_HELP("ignore", "Takes player's ability to chat, use vsay or callvotes.", "Uses unique part of name!")
	_HELP("unignore", "Restores player's ability to chat, use vsay or call votes.", "Uses unique part of name!")
	_HELP("clientignore", "Takes player's ability to chat, callvotes or use vsay.", "Uses client slot!")
	_HELP("clientunignore", "Restores player's ability to chat, callvotes or use vsay.", "Uses client slot!")
	_HELP("kick", "Kicks player from server.", "Uses unique part of name! Optionally you can add a message.")
	_HELP("clientkick", "Kicks player from server.", "Uses client slot number! Optionally you can add a message.")
	_HELP("slap", "Slaps player and takes 20hp.", "Uses client slot number!")
	_HELP("kill", "Kills player on spot.", "Uses client slot number!")
	_HELP("lock", "Locks the team(s) so players can't join.", "Usage !lock <red/axis> <blue/allied> <both>")
	_HELP("unlock", "Unlocks the team(s) so players can join.", "Usage !unlock <red/axis> <blue/allied> <both>")
	_HELP("specs", "Forces player to spectators.", "Uses unique part of name!")
	_HELP("axis", "Forces player to Axis team.", "Uses unique part of name!")
	_HELP("allies", "Forces player to Allied team.", "Uses unique part of name!")
	_HELP("exec", "Executes server config file.", "You can use @ at the end to silently execute file, e.g. !exec server @")
	_HELP("nextmap", "Loads the nextmap.", NULL)
	_HELP("exec", "Executes config on a server. Note! Write full name.", "E.g. !exec server.cfg")
	_HELP("map", "Loads the map of your choice.", "!map mp_base")
	_HELP("cpa", "Center Prints Admin message to everyone.", "!cpa <msg>")
	_HELP("cp", "Center Prints Admin message to selected user.", "Uses client slot number!")
	_HELP("chat", "Shows warning message to all in global chat.", "!chat <msg>")
	_HELP("warn", "Shows warning message to all in global chat and center print.", "!warn <msg>")
	_HELP("cancelvote", "Cancels any vote in progress.", NULL)
	_HELP("passvote", "Passes any vote in progress.", NULL)
	_HELP("restart", "Restarts the round.", NULL)
	_HELP("reset", "Resets the match.", NULL)
	_HELP("swap", "Swaps the teams.", NULL)
	_HELP("shuffle", "Shuffles the teams.", NULL)
	_HELP("spec999", "Moves all lagged (999) players to spectators.", NULL)
	_HELP("whereis", "Reveals players location to all.", "Uses client slot number!")
	_HELP("pause", "Pauses the match in progress.", NULL)
	_HELP("unpause", "Resumes a paused match..", NULL)
	_HELP("banguid", "Bans player's GUID.", "!banguid <unique part of name>")
	_HELP("banclientguid", "Bans player's GUID.", "!banclientguid <client slot number>")
	_HELP("tempbanguid", "Temporarily bans player from server.", "!tempbanguid <unique part of name> <mins>")
	_HELP("banip", "Bans player by IP.", "!banip <unique part of name>")
	_HELP("tempbanip", "Temporarily Bans player by IP.", "!tempbanip <unique part of name> <mins>")
	_HELP("addip", "Adds IP to banned file. You can use wildcards for subrange bans.", "example - !addip 100.*.*.*")
	_HELP("rename", "Renames players.", "!rename <client slot> <new name>")
	_HELP("vstr", "Loads a level from rotation file. Note - You need to know rotation labels..", "!vstr map1")
	_HELP("renameon", "Restores ability to rename from client.", "!renameon <client number>")
	_HELP("renameoff", "Revokes ability to rename from client (lasts only that round).", "!renameoff <client number>")
	_HELP("speclock", "Locks team(s) from spectators viewing.", "!speclock <axis/red allied/blue both>")
	_HELP("specunlock", "Unlocks team(s) for spectator viewing.", "!specunlock <axis/red allied/blue both>")
	_HELP("readyall", "Sets all players as ready and starts the countdown..", NULL)
	_HELP("unreadyall", "Cancels countdown and returns back to warmup..", NULL)
	// --> Add new ones after this line..

	{NULL, NULL, NULL}
};


qboolean do_help(gentity_t *ent) {

	char alt[128];
	char cmd[128];
	unsigned int i, \
	//	aHelp=ARRAY_LEN( helpInfo );
		aHelp=sizeof(helpInfo)/sizeof(helpInfo[0]);
	const helpCmd_reference_t *hCM;
	qboolean wasUsed=qfalse;

	admCmds(ent->client->pers.cmd1, alt, cmd, qtrue);

	for ( i = 0; i < aHelp; i++ ) {
		hCM = &helpInfo[i];
		if ( NULL != hCM->command && 0 == Q_stricmp( cmd, hCM->command ) ) {
			CP(va("print \"^n%s %s %s\n\"",
				va(hCM->usage ? "Help ^7:" : "Help^7:"),
				hCM->help,
				va("%s", (hCM->usage ? va("\n^zUsage^7: %s\n", hCM->usage) : "")) ));
			wasUsed = qtrue;
		}
	}
return wasUsed;

//return qfalse;
}

/*
===========
Commands
===========
*/
qboolean cmds_admin(char cmd[MAX_TOKEN_CHARS], gentity_t *ent) {

	// We're dealing with command
	if ( Q_stricmp( cmd, "!" ) == 0 ) {
		return do_cmds(ent);
	}
	// We're dealing with help
	else if ( Q_stricmp( cmd, "?" ) == 0 ) {
		return do_help(ent);
	}

return qfalse;
}
