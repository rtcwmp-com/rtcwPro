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
L0 - g_players.c

Player's commands and features.

Created: 23. Oct / 2012
Last Updated: 28. Apr / 2013
===========================================================================
*/
#include "g_local.h"

// ************** PLAYERS
//
// Show client info
void pCmd_players(gentity_t *ent, qboolean fParam) {
	int i, idnum, max_rate, cnt = 0, tteam;
	int user_rate, user_snaps;
	gclient_t *cl;
	gentity_t *cl_ent;
	char n1[MAX_NETNAME], ready[16], ref[16], rate[256];
	char *s, *tc, *coach, userinfo[MAX_INFO_STRING];


	if (g_gamestate.integer == GS_PLAYING) {
		if (ent) {
			CP("print \"\n^3 ID^1 : ^3Player                    Nudge  Rate  MaxPkts  Snaps\n\"");
			CP("print \"^1-----------------------------------------------------------^7\n\"");
		}
		else {
			G_Printf(" ID : Player                    Nudge  Rate  MaxPkts  Snaps\n");
			G_Printf("-----------------------------------------------------------\n");
		}
	}
	else {
		if (ent) {
			CP("print \"\n^3Status^1   : ^3ID^1 : ^3Player                    Nudge  Rate  MaxPkts  Snaps\n\"");
			CP("print \"^1---------------------------------------------------------------------^7\n\"");
		}
		else {
			G_Printf("Status   : ID : Player                    Nudge  Rate  MaxPkts  Snaps\n");
			G_Printf("---------------------------------------------------------------------\n");
		}
	}

	max_rate = trap_Cvar_VariableIntegerValue("sv_maxrate");

	for (i = 0; i < level.numConnectedClients; i++) {
		idnum = level.sortedClients[i];
		cl = &level.clients[idnum];
		cl_ent = g_entities + idnum;

		SanitizeString(cl->pers.netname, n1);
		Q_CleanStr(n1);
		n1[26] = 0;
		ref[0] = 0;
		ready[0] = 0;

		// Rate info
		if (cl_ent->r.svFlags & SVF_BOT) {
			strcpy(rate, va("%s%s%s%s", "[BOT]", " -----", "       --", "     --"));
		}
		else if (cl->pers.connected == CON_CONNECTING) {
			strcpy(rate, va("%s", "^3>>> CONNECTING <<<"));
		}
		else {
			trap_GetUserinfo(idnum, userinfo, sizeof(userinfo));
			s = Info_ValueForKey(userinfo, "rate");
			user_rate = (max_rate > 0 && atoi(s) > max_rate) ? max_rate : atoi(s);
			s = Info_ValueForKey(userinfo, "snaps");
			user_snaps = atoi(s);

			strcpy(rate, va("%5d%6d%9d%7d", cl->pers.clientTimeNudge, user_rate, cl->pers.clientMaxPackets, user_snaps));
		}

		if (g_gamestate.integer != GS_PLAYING) {
			if (cl->sess.sessionTeam == TEAM_SPECTATOR || cl->pers.connected == CON_CONNECTING) {
				strcpy(ready, ((ent) ? "^5--------^1 :" : "-------- :"));
			}
			else if (cl->pers.ready || (g_entities[idnum].r.svFlags & SVF_BOT)) {
				strcpy(ready, ((ent) ? "^3(READY)^1  :" : "(READY)  :"));
			}
			else {
				strcpy(ready, ((ent) ? "NOTREADY^1 :" : "NOTREADY :"));
			}
		}

		if (cl->sess.admin && !cl->sess.incognito) {
			strcpy(ref, sortTag(ent));
		}
		/*
		if (cl->sess.coach_team) {
			tteam = cl->sess.coach_team;
			coach = (ent) ? "^3C" : "C";
		}
		else {*/
			tteam = cl->sess.sessionTeam;
			coach = " ";
		//}

		tc = (ent) ? "^7 " : " ";
		if (g_gametype.integer >= GT_WOLF) {
			if (tteam == TEAM_RED) {
				tc = (ent) ? "^1X^7" : "X";
			}
			if (tteam == TEAM_BLUE) {
				tc = (ent) ? "^4L^7" : "L";
			}
		}

		if (ent) {
			CP(va("print \"%s%s%2d%s^1:%s %-26s^7%s  ^3%s\n\"", ready, tc, idnum, coach, ((ref[0]) ? "^3" : "^7"), n1, rate, ref));
		}
		else { G_Printf("%s%s%2d%s: %-26s%s  %s\n", ready, tc, idnum, coach, n1, rate, ref); }

		cnt++;
	}

	if (ent) {
		CP(va("print \"\n^3%2d^7 total players\n\n\"", cnt));
	}
	else { G_Printf("\n%2d total players\n\n", cnt); }

	// Team speclock info
	if (g_gametype.integer >= GT_WOLF) {
		for (i = TEAM_RED; i <= TEAM_BLUE; i++) {
			if (teamInfo[i].spec_lock) {
				if (ent) {
					CP(va("print \"** %s team is speclocked.\n\"", aTeams[i]));
				}
				else { G_Printf("** %s team is speclocked.\n", aTeams[i]); }
			}
		}
	}
}
/*
===========
Get client number from name
===========
*/
/* // nihi commented out below
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
*/
/*
================
Private messages
================
*/
void cmd_pmsg(gentity_t *ent)
{
	char cmd[MAX_TOKEN_CHARS];
	char name[MAX_STRING_CHARS];
	char nameList[MAX_STRING_CHARS];
	char *msg;
	int matchList[MAX_CLIENTS];
	int count, i;

	if (!g_allowPMs.integer)	{
		CP("print \"Private messages are Disabled^1!\n\"");
		return;
	}

	if (trap_Argc() < 3) {
		trap_Argv(0, cmd, sizeof(cmd));
		CP(va("print \"^dUsage:^7  %s <match> <message>\n\"", cmd));
	return;
	}

	if (ent->client->sess.ignored) {
		if (ent->client->sess.ignored == 1)
			CP( "cp \"You are ignored^1!\n\"2" );
		else
			CP( "print \"You are ^zpermanently ^7ignored^1!\n\"" );
	return;
	}

	trap_Argv(1, name, sizeof(name));
	if (strlen(name) < 2) {
		CP("print \"You must match at least ^32 ^7characters of the name^3!\n\"");
	return;
	}

	count = ClientNumberFromNameMatch(name, matchList);
	if (count == 0) {
		CP("print \"No matching clients found\n\"");
	return;
	}

	msg = ConcatArgs(2);
    if( strlen(msg) >= 700 ){
		G_LogPrintf( "NUKER(pmsg >= 700): %s IP: %i.%i.%i.%i\n", ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], ent->client->sess.ip[3] );
	    trap_DropClient( ent-g_entities, "^7Player Kicked: ^3Nuking" );
	return;
    }

	Q_strncpyz ( nameList, "", sizeof( nameList ) );
	for (i = 0; i < count; i++)	{
		strcat(nameList, g_entities[matchList[i]].client->pers.netname);
		if (i != (count-1))
			strcat(nameList, "^7, ");

			// Pop in
			CPx(matchList[i], va("popin \"Message from %s^j!\n\"", ent->client->pers.netname));
			// Print in console..
			CPx(matchList[i], va("@print \"^zMessage from ^7%s^7: \n^3%.99s\n\"", ent->client->pers.netname, msg));

	}
	//let the sender know his message went to
	CP(va("@print \"^zMessage was sent to: ^7%s \n^zMessage: ^3%.99s\n\"", nameList, msg));
}

// Actual command
void cmd_throwKnives( gentity_t *ent ) {
	vec3_t velocity, angles, offset, org, mins, maxs;
	trace_t tr;
	gentity_t *ent2;
	gitem_t *item = BG_FindItemForWeapon( WP_KNIFE );

//	if ( g_throwKnives.integer == 0 ) {
	//	return;
//	}

//	if ( level.time < ( ent->thrownKnifeTime + 800 ) ) {
//		return;
//	}

	// If out or -1/unlimited
	//if ( ( ent->client->pers.throwingKnives == 0 ) &&
	//	 ( g_throwKnives.integer != -1 ) ) {
	//return;
	//}
/*
	AngleVectors( ent->client->ps.viewangles, velocity, NULL, NULL );
	VectorScale( velocity, 64, offset );
	offset[2] += ent->client->ps.viewheight / 2;
	VectorScale( velocity, 800, velocity );
	velocity[2] += 50 + random() * 35;
	VectorAdd( ent->client->ps.origin, offset, org );
	VectorSet( mins, -ITEM_RADIUS, -ITEM_RADIUS, 0 );
	VectorSet( maxs, ITEM_RADIUS, ITEM_RADIUS, 2 * ITEM_RADIUS );
	trap_Trace( &tr, ent->client->ps.origin, mins, maxs, org, ent->s.number, MASK_SOLID );
	VectorCopy( tr.endpos, org );

	G_Sound( ent, G_SoundIndex( "sound/weapons/knife/knife_slash1.wav" ) );
	ent2 = LaunchItem( item, org, velocity, ent->client->ps.clientNum );
	VectorCopy( ent->client->ps.viewangles, angles );
	angles[1] += 90;
	G_SetAngle( ent2, angles );
	ent2->touch = Touch_Knife;
	ent2->parent = ent;

	if ( g_throwKnives.integer > 0 ) {
		ent->client->pers.throwingKnives--;
	}

	//only show the message if throwing knives are enabled
	if ( g_throwKnives.integer > 0 ) {
		CP(va( "chat \"^zKnives left:^7 %d\" %i", ent->client->pers.throwingKnives, qfalse ));
	}

ent->thrownKnifeTime = level.time;
*/

}

/*
===================
Invite player to spectate

NOTE: Admin can still be invited..so in case logout occurs..
===================
*/
void cmd_specInvite( gentity_t *ent ) {
	int	target;
	gentity_t	*player;
	char arg[MAX_TOKEN_CHARS];
	int team=ent->client->sess.sessionTeam;

	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		if ( !teamInfo[team].spec_lock ) {
			CP( "print \"Your team isn't locked from spectators!\n\"" );
			return;
		}

		trap_Argv( 1, arg, sizeof( arg ) );
		if ( ( target = ClientNumberFromString( ent, arg ) ) == -1 ) {
			return;
		}

		player = g_entities + target;

		// Can't invite self
		if ( player->client == ent->client ) {
			CP( "print \"You can't specinvite yourself!\n\"" );
			return;
		}

		// Can't invite an active player.
		if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
			CP( "print \"You can't specinvite a non-spectator!\n\"" );
			return;
		}

		// If player it not viewing anyone, force them..
		if (!player->client->sess.specInvited &&
			!(player->client->sess.spectatorClient == SPECTATOR_FOLLOW)) {
			player->client->sess.spectatorClient = ent->client->ps.clientNum;
			player->client->sess.spectatorState = SPECTATOR_FOLLOW;
		}

		player->client->sess.specInvited |= team;

		// Notify sender/recipient
		CP( va( "print \"%s^7 has been sent a spectator invitation.\n\"", player->client->pers.netname ) );
		CPx(player-g_entities, va("cp \"%s ^7invited you to spec the %s team.\n\"2",
			ent->client->pers.netname, aTeams[team]));

	} else {CP( "print \"Spectators can't specinvite players!\n\"" );}
}
/*
=================
Hitsounds

Do it like in shrub just permanently
(A hack tied to color so one doesn't need to type it all the time..)
=================
*/
void Cmd_hitsounds(gentity_t *ent) {
	char *action = (ent->client->sess.clientFlags & CFLAGS_HITSOUNDS ? "^3Disable^7" : "^3Enable^7");
	int	flag = (ent->client->sess.clientFlags & CFLAGS_HITSOUNDS ? 0 : 1);

	CP(va("print \"Bit flag to %s Hitsounds is /color %d \nType ^3/commands bitflags^7 for explanation.\n\"", action, flag));
	return;
}

/*
===================
unInvite player from spectating
===================
*/
void cmd_specUnInvite( gentity_t *ent ) {
	int	target;
	gentity_t	*player;
	char arg[MAX_TOKEN_CHARS];
	int team=ent->client->sess.sessionTeam;

	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		if ( !teamInfo[team].spec_lock ) {
			CP( "print \"Your team isn't locked from spectators!\n\"" );
			return;
		}

		trap_Argv( 1, arg, sizeof( arg ) );
		if ( ( target = ClientNumberFromString( ent, arg ) ) == -1 ) {
			return;
		}

		player = g_entities + target;

		// Can't uninvite self
		if ( player->client == ent->client ) {
			CP( "print \"You can't specuninvite yourself!\n\"" );
			return;
		}

		// Can't uninvite an active player.
		if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
			CP( "print \"You can't specuninvite a non-spectator!\n\"" );
			return;
		}

		// Can't uninvite a already speclocked player
		if (player->client->sess.specInvited < team) {
			CP(va("print \"%s ^7already can't spectate your team!\n\"", ent->client->pers.netname));
			return;
		}

		player->client->sess.specInvited &= ~team;
		G_updateSpecLock(team, qtrue);

		// Notify sender/recipient
		CP( va( "print \"%s^7 can't any longer spectate your team.\n\"", player->client->pers.netname ) );
		CPx(player->client->ps.clientNum, va("print \"%s ^7has revoked your ability to spectate the %s team.\n\"",
			ent->client->pers.netname, aTeams[team]));

	} else {CP( "print \"Spectators can't specuninvite players!\n\"" );}
}

/*
===================
Revoke ability from all players to spectate
===================
*/
void cmd_uninviteAll( gentity_t *ent) {
	int team = ent->client->sess.sessionTeam;

	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		if ( !teamInfo[team].spec_lock ) {
			CP( "print \"Your team isn't locked from spectators!\n\"" );
			return;
		}

		// Remove all specs
		G_removeSpecInvite(team);

		// Notify that team only that specs lost privilage
		//TP(team, "chat",  va("^3TEAM NOTICE: ^7%s ^7has revoked ALL spec's invites for your team.", ent->client->pers.netname));
		// Inform specs..
		//TP(TEAM_SPECTATOR, "print", va("%s ^7revoked ALL spec invites from %s team", ent->client->pers.netname, aTeams[team]));

	} else {CP( "print \"Spectators can't specuninviteall!\n\"" );}

}

/*
===================
Spec lock/unlock team
===================
*/
void cmd_speclock( gentity_t *ent, qboolean lock ) {
	int team = ent->client->sess.sessionTeam;


	if (!team_commands.integer ) {
		CP("print \"Team commands are disabled!\n\"");
	return;
	}

	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		if ( (lock && teamInfo[team].spec_lock) || (!lock && !teamInfo[team].spec_lock) ) {
			CP( va("print \"Your team is already %s spectators!\n\"",
				(!lock ? "unlocked for" : "locked from" ) ));
			return;
		}

		G_updateSpecLock( team, lock );
		AP(va("cp \"%s is now SPEC%s\"2", aTeams[team], (lock ? "LOCKED" : "UNLOCKED" ) ));

	} else {CP( va("print \"Spectators can't use spec%s command!\n\"", (lock ? "lock" : "unlock" )) );}

}

/*
===================
READY / NOTREADY

Sets a player's "ready" status.

Tardo - rewrote this because the parameter handling to the function is different in rtcw.
===================
*/
void G_readyHandle( gentity_t* ent, qboolean ready ) {
	ent->client->pers.ready = ready;
}

void G_ready_cmd( gentity_t *ent, qboolean state ) {
	char *status[2] = { "^zNOT READY^7", "^nREADY^7" };

	if (!g_tournament.integer) {
		return;
	}

	if ( g_gamestate.integer == GS_PLAYING || g_gamestate.integer == GS_INTERMISSION ) {
		CP( "@print \"Match is already in progress!\n\"" );
		return;
	}

	if ( !state && g_gamestate.integer == GS_WARMUP_COUNTDOWN ) {
		CP( "print \"Countdown started..^znotready^7 ignored!\n\"" );
		return;
	}

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		CP( va("print \"Specs cannot use %s ^7command!\n\"", status[state] ));
		return;
	}
	if (level.readyTeam[ent->client->sess.sessionTeam] == qtrue && !state) { // Doesn't cope with unreadyteam but it's out anyway..
		CP(va("print \"%s ^7ignored. Your team has issued ^3TEAM READY ^7command..\n\"", status[state]));
		return;
	}
	// Move them to correct ready state
	if ( ent->client->pers.ready == state ) {
		CP( va( "print \"You are already %s!\n\"", status[state] ) );
	} else {
		ent->client->pers.ready = state;
		if ( !level.intermissiontime ) {
			if ( state ) {
				ent->client->pers.ready = qtrue;
				ent->client->ps.powerups[PW_READY] = INT_MAX;
			} else {
				ent->client->pers.ready = qfalse;
				ent->client->ps.powerups[PW_READY] = 0;
			}

			// Doesn't rly matter..score tab will show slow ones..
			AP( va( "cp \"\n%s \n^7is %s!\n\"", ent->client->pers.netname, status[state] ) );
		}
	}
}

/*
===================
TEAM-READY / NOTREADY
===================
*/
void pCmd_teamReady(gentity_t *ent, qboolean ready) {
	char *status[2] = { "NOT READY", "READY" };
	int i, p = { 0 };
	int team = ent->client->sess.sessionTeam;
	gentity_t *cl;

	if (!g_tournament.integer) {
		return;
	}
	if (team_nocontrols.integer) {
		CP("print \"Team commands are not enabled on this server.\n\"");
		return;
	}
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		CP("print \"Specs cannot use ^3TEAM ^7commands.\n\"");
		return;
	}
	if (!ready && g_gamestate.integer == GS_WARMUP_COUNTDOWN) {
		CP("print \"Countdown started, ^3notready^7 ignored.\n\"");
		return;
	}

	for (i = 0; i < level.numConnectedClients; i++) {
		cl = g_entities + level.sortedClients[i];

		if (!cl->inuse) {
			continue;
		}

		if (cl->client->sess.sessionTeam != team) {
			continue;
		}

		if ((cl->client->pers.ready != ready) && !level.intermissiontime) {
			cl->client->pers.ready = ready;
			cl->client->ps.powerups[PW_READY] = (ready ? INT_MAX : 0);
			++p;
		}
	}

	if (!p) {
		CP(va("print \"Your team is already ^3%s^7!\n\"", status[ready]));
	}
	else {
		AP(va("popin \"%s ^7team is %s%s!\n\"", aTeams[team], (ready ? "^n" : "^z"), status[ready]));
		level.readyTeam[team] = ready;
	}
}
/*
===================
Pause/Unpause
===================
*/
void pCmd_pauseHandle(gentity_t *ent, qboolean dPause) {
    int team = ent->client->sess.sessionTeam;
    char tName[MAX_NETNAME];
	char *tag, *log, *action;
	if (team_nocontrols.integer) {
		CP("print \"Team commands are not enabled on this server.\n\"");
		return;
	}



	if ( g_gamestate.integer != GS_PLAYING ) {
		CP("print \"^jError: ^7Pause can only be issued during a match!\n\"");
		return;
	}

	if (level.numPlayingClients == 0) {
		CP("print \"^jError: ^7You cannot use pause feature with no playing clients..\n\"");
	return;
	}
	DecolorString(aTeams[team], tName);

	if (!dPause) {
/*	//	level.paused = !PAUSE_NONE;  // nihi commented
		level.paused = team + 128; // nihi added
		G_spawnPrintf(DP_PAUSEINFO, level.time + 15000, NULL); // nihi added

		trap_SetConfigstring( CS_PAUSED, va( "%i", level.paused ));
		AP(va("chat \"^zconsole: ^7%s has ^3Paused ^7a match!\n\"", tName));
		AAPS("sound/world/klaxon1.wav");
	}
   // else if (level.paused != PAUSE_UNPAUSING){
    else if (team + 128 != level.paused) {// nihi added
		if (level.paused == PAUSE_NONE) {
			CP("print \"^jError: ^7Match is not paused^j!\n\"");
		return;
		}
        CP("cpm \"^3Your team didn't call the timeout!\n\"");
        return;
    }
    else {
		level.CNstart = 0; // Resets countdown if it was aborted before
		level.paused = PAUSE_UNPAUSING;
		AP(va("chat \"^zconsole: ^7%s has ^3Unpaused ^7a match!\n\"", tName));
		G_spawnPrintf(DP_UNPAUSING, level.time + 10, NULL); // nihi added
	}
	*/
		level.paused = !PAUSE_NONE;
		trap_SetConfigstring( CS_PAUSED, va( "%i", level.paused ));
		AP(va("chat \"^zconsole: ^7%s has ^3Paused ^7a match!\n\"", tName));
		AAPS("sound/world/klaxon1.wav");
	} else if (level.paused != PAUSE_UNPAUSING){
		if (level.paused == PAUSE_NONE) {
			CP("print \"^jError: ^7Match is not paused^j!\n\"");
		return;
		}

		level.CNstart = 0; // Resets countdown if it was aborted before
		level.paused = PAUSE_UNPAUSING;
		AP(va("chat \"^zconsole: ^7%s has ^3Unpaused ^7a match!\n\"", tName));
	}

}

/*
===========
Lock or Unlock game

What a mess...
===========
*/
void pCmd_gamelocked(gentity_t *ent, qboolean unlock) {
    int team = ent->client->sess.sessionTeam;
    char tName[MAX_NETNAME];
	char *tag, *log;

	if (team_nocontrols.integer) {
		CP("print \"Team commands are not enabled on this server.\n\"");
		return;
	}
	//tag = sortTag(ent);
    DecolorString(aTeams[team], tName);

	// Deals with unlocking
	// Deals with unlocking
	if (unlock) {
		if (!g_gamelocked.integer) {
			CP(va("print \"Both teams are already unlocked^z!\n\""));
		return;
		} else {
			// Axis
			if (!strcmp(tName,"red") || !strcmp(tName,"Axis")) {
				if (g_gamelocked.integer == 1) {
					trap_Cvar_Set( "g_gamelocked", "0" );
					AP(va("chat \"^zconsole:^7 %s has unlocked ^1Axis ^7team^z!\n\"", tName));
				} else if (g_gamelocked.integer == 3) {
					trap_Cvar_Set( "g_gamelocked", "2" );
					AP(va("chat \"^zconsole:^7 %s has unlocked ^1Axis ^7team^z!\n\"", tName));
				} else {
					CP(va("print \"^1Axis ^7team is already unlocked^z!\n\""));
				return;
				}
			}
			// Allied
			else if (!strcmp(tName,"blue") || !strcmp(tName,"Allies")) {
				if (g_gamelocked.integer == 2) {
					trap_Cvar_Set( "g_gamelocked", "0" );
					AP(va("chat \"^zconsole:^7 %s has unlocked ^4Allied ^7team^z!\n\"", tName));
				} else if (g_gamelocked.integer == 3) {
					trap_Cvar_Set( "g_gamelocked", "1" );
					AP(va("chat \"^zconsole:^7 %s has unlocked ^1Allied ^7team^z!\n\"", tName));
				} else {
					CP(va("print \"^4Allied ^7team is already unlocked^z!\n\""));
				return;
				}
			// Both
			}

		}
	return;
	// Deals with locking
	} else {
		if (g_gamelocked.integer == 3) {
			CP(va("print \"Both teams are already locked^z!\n\""));
		return;
		} else {
			// Axis
			if (!strcmp(tName,"red") || !strcmp(tName,"Axis")) {
				if (!g_gamelocked.integer) {
					trap_Cvar_Set( "g_gamelocked", "1" );
					AP(va("chat \"^zconsole:^7 %s has locked ^1Axis ^7team^z!\n\"", tName));
				} else if (g_gamelocked.integer == 2) {
					trap_Cvar_Set( "g_gamelocked", "3" );
					AP(va("chat \"^zconsole:^7 %s has locked ^1Axis ^7team^z!\n\"", tName));
				} else {
					CP(va("print \"^1Axis ^7team is already locked^1!\n\""));
				return;
				}
			}
			// Allied
			else if (!strcmp(tName,"blue") || !strcmp(tName,"Allies")) {
				if (!g_gamelocked.integer) {
					trap_Cvar_Set( "g_gamelocked", "2" );
					AP(va("chat \"^zconsole:^7 %s has locked ^4Allied ^7team^z!\n\"", tName));
				} else if (g_gamelocked.integer == 1) {
					trap_Cvar_Set( "g_gamelocked", "3" );
					AP(va("chat \"^zconsole:^7 %s has locked ^1Allied ^7team^z!\n\"", tName));
				} else {
					CP(va("print \"^4Allied ^7team is already unlocked^z!\n\""));
				return;
				}
			// Both
			}
		// Log it

		}
	return;
	}
}
/*
===================
OSP's stats
===================
*/
void G_scores_cmd( gentity_t *ent ) {
	G_printMatchInfo( ent );
}
// Shows a player's stats to the requesting client.
void G_weaponStats_cmd( gentity_t *ent ) {
	G_statsPrint( ent, 0 );
}


/******************* Client commands *******************/
qboolean playerCmds (gentity_t *ent, char *cmd ) {

	if(!Q_stricmp(cmd, "pm")
		 || !Q_stricmp(cmd, "msg"))					{ cmd_pmsg(ent);	return qtrue;}
//	else if(!Q_stricmp(cmd, "smoke"))				{ cmd_pSmoke(ent);			return qtrue;}
	else if(!Q_stricmp(cmd, "readyteam"))			{ pCmd_teamReady(ent, qtrue);	return qtrue;}
	else if(!Q_stricmp(cmd, "speclock"))			{ cmd_speclock(ent, qtrue);	return qtrue;}
	else if(!Q_stricmp(cmd, "players"))			    { pCmd_players(ent, qfalse);	return qtrue;}
	else if(!Q_stricmp(cmd, "specunlock"))			{ cmd_speclock(ent, qfalse);return qtrue;}
	else if(!Q_stricmp(cmd, "specinvite"))			{ cmd_specInvite(ent);		return qtrue;}
	else if(!Q_stricmp(cmd, "specuninvite"))		{ cmd_specUnInvite(ent);	return qtrue;}
	else if(!Q_stricmp(cmd, "specuninviteall"))		{ cmd_uninviteAll(ent);		return qtrue;}
	else if(!Q_stricmp(cmd, "wstats"))				{ G_statsPrint( ent, 1 );	return qtrue;}
	else if(!Q_stricmp(cmd, "cstats"))				{ G_clientStatsPrint( ent, 1, qtrue );	return qtrue;}
	else if(!Q_stricmp(cmd, "stats"))				{ G_clientStatsPrint( ent, 1, qfalse );	return qtrue;}
	else if(!Q_stricmp(cmd, "sgstats"))				{ G_statsPrint( ent, 2 );	return qtrue;}
	else if(!Q_stricmp(cmd, "stshots"))				{ G_weaponStatsLeaders_cmd( ent, qtrue, qtrue );	return qtrue;}
	else if(!Q_stricmp(cmd, "scores"))				{ G_scores_cmd(ent);	return qtrue;}
	else if(!Q_stricmp(cmd, "statsall"))			{ G_statsall_cmd( ent, 0, qfalse );	return qtrue;}
	else if(!Q_stricmp(cmd, "bottomshots"))			{ G_weaponRankings_cmd( ent, qtrue, qfalse );	return qtrue;}
	else if(!Q_stricmp(cmd, "topshots"))			{ G_weaponRankings_cmd( ent, qtrue, qtrue );	return qtrue;}
	else if(!Q_stricmp(cmd, "weaponstats"))			{ G_weaponStats_cmd( ent );	return qtrue;}
	//Tardo Ready/Unready
	else if (!strcmp(cmd,"lock"))			        { pCmd_gamelocked(ent, qfalse); return qtrue;}
	else if (!strcmp(cmd,"unlock"))		        	{ pCmd_gamelocked(ent, qtrue);  return qtrue;}
    else if(!Q_stricmp(cmd, "pause"))				{ pCmd_pauseHandle( ent, qfalse ); return qtrue;}
    else if(!Q_stricmp(cmd, "unpause"))				{ pCmd_pauseHandle( ent, qtrue ); return qtrue;}
	else if(!Q_stricmp(cmd, "ready"))				{ G_ready_cmd( ent, qtrue ); return qtrue;}
	else if(!Q_stricmp(cmd, "unready") ||
			!Q_stricmp(cmd, "notready"))			{ G_ready_cmd( ent, qfalse ); return qtrue;}
	else
		return qfalse;
}

