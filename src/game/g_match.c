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
g_match.c

Handle match related stuff, much like in et..

Author: Nate 'L0
Created: 22.10/12
Updated: 18.04/13
===========================================================================
*/
#include "g_local.h"

// OSP
char *aTeams[TEAM_NUM_TEAMS] = { "FFA", "^1Axis^7", "^4Allies^7", "Spectators" };
team_info teamInfo[TEAM_NUM_TEAMS];

/**
 * @brief Setting initialization
 */
void G_loadMatchGame(void)
{
	int  i, dwBlueOffset, dwRedOffset;
	int  aRandomValues[MAX_REINFSEEDS];
	char strReinfSeeds[MAX_STRING_CHARS];

//	G_Printf("Setting MOTD...\n");
//	trap_SetConfigstring(CS_CUSTMOTD + 0, server_motd0.string);
//	trap_SetConfigstring(CS_CUSTMOTD + 1, server_motd1.string);
//	trap_SetConfigstring(CS_CUSTMOTD + 2, server_motd2.string);
//	trap_SetConfigstring(CS_CUSTMOTD + 3, server_motd3.string);
//	trap_SetConfigstring(CS_CUSTMOTD + 4, server_motd4.string);
//	trap_SetConfigstring(CS_CUSTMOTD + 5, server_motd5.string);

	// Voting flags
//	G_voteFlags();

	// Set up the random reinforcement seeds for both teams and send to clients
	dwBlueOffset = rand() % MAX_REINFSEEDS;
	dwRedOffset  = rand() % MAX_REINFSEEDS;
	Q_strncpyz(strReinfSeeds, va("%d %d", (dwBlueOffset << REINF_BLUEDELT) + (rand() % (1 << REINF_BLUEDELT)),
	                             (dwRedOffset << REINF_REDDELT)  + (rand() % (1 << REINF_REDDELT))),
	           MAX_STRING_CHARS);

	for (i = 0; i < MAX_REINFSEEDS; i++)
	{
		//aRandomValues[i] = (rand() % REINF_RANGE) * aReinfSeeds[i];
		aRandomValues[i] = (rand() % g_spawnOffset.integer) * aReinfSeeds[i];
		Q_strcat(strReinfSeeds, MAX_STRING_CHARS, va(" %d", aRandomValues[i]));
	}

	level.dwBlueReinfOffset = 1000 * aRandomValues[dwBlueOffset] / aReinfSeeds[dwBlueOffset];
	level.dwRedReinfOffset  = 1000 * aRandomValues[dwRedOffset] / aReinfSeeds[dwRedOffset];

	trap_SetConfigstring(CS_REINFSEEDS, strReinfSeeds);
    // write first respawn time
    if (g_gameStatslog.integer && g_gamestate.integer == GS_PLAYING) {
        gentity_t *dummy = g_entities;

        G_writeGeneralEvent(dummy,dummy,"",teamFirstSpawn);
    }
}

/*
================
Default weapon

Accounts for "selected weapon" as well.
================
*/
///////////
// Deals only with soldier for weapon restrictions (To avoid breaking anything..).
void setDefWeap(gclient_t *client, int clips) {
	if (client->sess.sessionTeam == TEAM_RED)
	{
		COM_BitSet(client->ps.weapons, WP_MP40);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_MP40)] += 32;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_MP40)] += (32 * clips);
		client->ps.weapon = WP_MP40;
		client->sess.playerWeapon = WP_MP40; // set this so Weapon Restrictions work
	} else {
		COM_BitSet(client->ps.weapons, WP_THOMPSON);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_THOMPSON)] += 30;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_THOMPSON)] += (30 * clips);
		client->ps.weapon = WP_THOMPSON;
		client->sess.playerWeapon = WP_THOMPSON; // set this so Weapon Restrictions work
	}
}

///////////
// Deals with weapons
//
// NOTE: Selected weapons only works for eng and med..sold and lt can pick their weapons already..
//       so setting it can potentialy overlap with client spawn scripts..
void SetDefaultWeapon(gclient_t *client, qboolean isSold) {
	int ammo;

	// This deals with weapon restrictions.
	if (isSold) {
		setDefWeap(client, g_soldierClips.integer);
		return;
	}

	// Sorts ammo
	ammo = (client->sess.selectedWeapon == WP_THOMPSON) ? 30 : 32;

	// Medic
	if (client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC) {
		if (client->sess.selectedWeapon != 0) {
			COM_BitSet(client->ps.weapons, client->sess.selectedWeapon);
			client->ps.ammoclip[BG_FindClipForWeapon(client->sess.selectedWeapon)] += ammo;
			client->ps.ammo[BG_FindAmmoForWeapon(client->sess.selectedWeapon)] += (ammo * g_medicClips.integer);
			client->ps.weapon = client->sess.selectedWeapon;
			client->sess.playerWeapon = client->sess.selectedWeapon; // set this so Weapon Restrictions work
			return;
		}
		else {
			setDefWeap(client, g_medicClips.integer);
			return;
		}
	}

	// Engineer
	if (client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER) {
		if (client->sess.selectedWeapon != 0) {
			COM_BitSet(client->ps.weapons, client->sess.selectedWeapon);
			client->ps.ammoclip[BG_FindClipForWeapon(client->sess.selectedWeapon)] += ammo;
			client->ps.ammo[BG_FindAmmoForWeapon(client->sess.selectedWeapon)] += (ammo * g_engineerClips.integer);
			client->ps.weapon = client->sess.selectedWeapon;
			client->sess.playerWeapon = client->sess.selectedWeapon; // set this so Weapon Restrictions work
			return;
		}
		else {
			setDefWeap(client, g_engineerClips.integer);
			return;
		}
	}
}

/*
=================
Countdown

Causes some troubles on client side so done it here.
=================
*/
void CountDown(void) {

	if (level.cnStarted == qfalse) {
		return;
	}

	// Prepare to fight takes 2 seconds..
	if(level.cnNum == 0) {
		level.cnPush = level.time + 2000;
	// Just enough to fix the bug and skip to action..
	} else if (level.cnNum == 6) {
		level.cnPush = level.time + 200;
	// Otherwise, 1 second.
	} else {
		level.cnPush = level.time + 1000;  
	} 
	
	// We're done.. restart the game
	if (level.cnNum == 7) {
		level.warmupTime += 10000;
		trap_Cvar_Set( "g_restarted", "1" );
		trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
		level.restarted = qtrue;			
		
		return;
	}
		
	level.cnNum++; 
}

/*
=================
G_delayPrint

Deals with pause related functionality
=================
*/
void G_delayPrint(gentity_t *dpent) {
	int think_next = 0;
	qboolean fFree = qtrue;

	switch (dpent->spawnflags){
	case DP_PAUSEINFO:
		if (level.paused > PAUSE_UNPAUSING) {
			int cSeconds = match_timeoutlength.integer * 1000 - (level.time - dpent->timestamp);

			if (cSeconds > 1000) {
				think_next = level.time + 1000;
				fFree = qfalse;

				if (cSeconds > 30000) {
					AP(va("popin \"Timeouts Available: [^1Axis^7] %d - [^4Allies^7] %d\n\"y",
						teamInfo[TEAM_RED].timeouts, teamInfo[TEAM_BLUE].timeouts));
				}
			}
			else {
				level.paused = PAUSE_UNPAUSING;
				G_spawnPrintf(DP_UNPAUSING, level.time + 7.2, NULL);
			}
		}
		break;
	case DP_UNPAUSING:
		if (level.paused == PAUSE_UNPAUSING) {
			int cSeconds = 12 * 1000 - (level.time - dpent->timestamp);

			if (cSeconds > 1000) {
				think_next = level.time + 1000;
				fFree      = qfalse;
			}
			else {
				level.paused = PAUSE_NONE;
				AP("print \"^1FIGHT!\n\"");
				AAPS("sound/match/fight.wav");
				trap_SetConfigstring(CS_PAUSED, "0");
				trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime + level.timeDelta));
			}
		}
		break;
	default:
		break;
	}

	dpent->nextthink = think_next;
	if (fFree) {
		dpent->think = 0;
		G_FreeEntity(dpent);
	}
}

static char *pszDPInfo[] =
{
	"DPRINTF_PAUSEINFO",
	"DPRINTF_UNPAUSING",
	"DPRINTF_CONNECTINFO",
	"DPRINTF_MVSPAWN",
	"DPRINTF_UNK1",
	"DPRINTF_UNK2",
	"DPRINTF_UNK3",
	"DPRINTF_UNK4",
	"DPRINTF_UNK5"
};

/**
 * @brief G_spawnPrintf
 * @param[in] print_type
 * @param[in] print_time
 * @param[in] owner
 */
void G_spawnPrintf(int print_type, int print_time, gentity_t *owner) {
	gentity_t* ent = G_Spawn();

	ent->classname  = pszDPInfo[print_type];
	ent->clipmask   = 0;
	ent->parent     = owner;
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags  |= EF_NODRAW;
	ent->s.eType    = ET_ITEM;

	ent->spawnflags = print_type;       // Tunnel in DP enum
	ent->timestamp  = level.time;       // Time entity was created

	ent->nextthink = print_time;
	ent->think     = G_delayPrint;

	// Set it here so client can do it's own magic..
	if (print_type == DP_PAUSEINFO)
		trap_SetConfigstring(CS_PAUSED, va("%d", match_timeoutlength.integer));
	else if (print_type == DP_UNPAUSING)
		trap_SetConfigstring(CS_PAUSED, "10000");
}

/*
=================
G_handlePause

Central function for (un)pausing the game.
=================
*/
void G_handlePause(qboolean dPause, int time) {
	if (dPause) {
		level.paused = 100 + time;
		G_spawnPrintf(DP_PAUSEINFO, level.time + 15000, NULL);
	}
	else {
		level.paused = PAUSE_UNPAUSING;
		G_spawnPrintf(DP_UNPAUSING, level.time + 10, NULL);
	}
}

/**
 * @brief Update configstring for vote info
 * @param[in] cv
 * @return
 */
int G_checkServerToggle(vmCvar_t *cv)
{
	int nFlag;

	if (cv == &match_mutespecs)
	{
		nFlag = CV_SVS_MUTESPECS;
	}
	else if (cv == &g_friendlyFire)
	{
		nFlag = CV_SVS_TEAMDMG;
	}
	else if (cv == &g_antilag)
	{
		nFlag = CV_SVS_ANTILAG;
	}
	// special case for 2 bits
	else if (cv == &match_warmupDamage)
	{
		if (cv->integer > 0)
		{
			level.server_settings &= ~CV_SVS_WARMUPDMG;
			nFlag                  = (cv->integer > 2) ? 2 : cv->integer;
			nFlag                  = nFlag << 2;
		}
		else
		{
			nFlag = CV_SVS_WARMUPDMG;
		}
	}
	else
	{
		return qfalse;
	}

	if (cv->integer > 0)
	{
		level.server_settings |= nFlag;
	}
	else
	{
		level.server_settings &= ~nFlag;
	}

	return qtrue;
}

/*
=================
Match Info

Basically just some info prints..
=================
*/
// Gracefully taken from s4ndmod :p
char* GetLevelTime(void) {
	int Objseconds, Objmins, Objtens;

	Objseconds = (((g_timelimit.value * 60 * 1000) - ((level.time - level.startTime))) / 1000); // martin - this line was a bitch :-)
																								// nate	  - I know, that's why I took it. :p
	Objmins = Objseconds / 60;
	Objseconds -= Objmins * 60;
	Objtens = Objseconds / 10;
	Objseconds -= Objtens * 10;

	if (Objseconds < 0) { Objseconds = 0; }
	if (Objtens < 0) { Objtens = 0; }
	if (Objmins < 0) { Objmins = 0; }

	return va("%i:%i%i", Objmins, Objtens, Objseconds);
}

// Prints stuff
void G_matchPrintInfo(char *msg, qboolean printTime) {
	if (printTime)
		AP(va("print \"[%s] ^3%s \n\"", GetLevelTime(), msg));
	else
		AP(va("print \"*** ^3INFO: ^7%s \n\"", msg));
}

// Simple alias for sure-fire print :)
void G_printFull(char *str, gentity_t *ent) {
	if (ent != NULL) {
		CP(va("print \"%s\n\"", str));
		CP(va("cp \"%s\n\"", str));
	}
	else {
		AP(va("print \"%s\n\"", str));
		AP(va("cp \"%s\n\"", str));
	}
}
// Debounces cmd request as necessary.
qboolean G_cmdDebounce(gentity_t *ent, const char *pszCommandName) {
	if (ent->client->pers.cmd_debounce > level.time) {
		CP(va("print \"Wait another %.1fs to issue ^3%s\n\"", 1.0 * (float)(ent->client->pers.cmd_debounce - level.time) / 1000.0,
			pszCommandName));
		return(qfalse);
	}

	ent->client->pers.cmd_debounce = level.time + CMD_DEBOUNCE;
	return(qtrue);
}
// Plays specified sound globally.
void G_globalSound(char *sound) {
	gentity_t *te = G_TempEntity(level.intermission_origin, EV_GLOBAL_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
	te->r.svFlags |= SVF_BROADCAST;
}
void G_resetRoundState(void) {
	if (g_gametype.integer == GT_WOLF_STOPWATCH) {
		trap_Cvar_Set("g_currentRound", "0");
    }
	/*else if (g_gametype.integer == GT_WOLF_LMS) {
		trap_Cvar_Set("g_currentRound", "0");
		trap_Cvar_Set("g_lms_currentMatch", "0");
	}*/
}
void G_resetModeState(void) {
	if (g_gametype.integer == GT_WOLF_STOPWATCH) {
		trap_Cvar_Set("g_nextTimeLimit", "0");
	}
	/*else if (g_gametype.integer == GT_WOLF_LMS) {
		trap_Cvar_Set("g_axiswins", "0");
		trap_Cvar_Set("g_alliedwins", "0");
	}*/
}
