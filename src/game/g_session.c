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
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

void G_WriteWeaponStatsData( gclient_t* client ) {
	const char* var;
	const char* s = G_writeStats(client);
	if (s != NULL) {
		var = va("wstats%i", client - level.clients);
		trap_Cvar_Set(var, s);
	}
}

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char  *s;
	const char  *var;

	// L0 - OSP -- stats reset check
	if ( level.fResetStats ) {
		G_deleteStats( client - level.clients );
	} else {
		// write wstats
		G_WriteWeaponStatsData(client);
	}/// End

    s = va( "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %s %s %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",       // updated for new stat data
		client->sess.sessionTeam,
		client->sess.spectatorTime,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.wins,
		client->sess.losses,
		client->sess.playerType,        // DHM - Nerve
		client->sess.playerWeapon,      // DHM - Nerve
		client->sess.playerItem,        // DHM - Nerve
		client->sess.playerSkin,        // DHM - Nerve
		client->sess.spawnObjectiveIndex, // DHM - Nerve
		client->sess.latchPlayerType,   // DHM - Nerve
		client->sess.latchPlayerWeapon, // DHM - Nerve
		client->sess.latchPlayerItem,   // DHM - Nerve
		client->sess.latchPlayerSkin,    // DHM - Nerve
		// L0 - New stuff
		client->sess.referee,			// User is ref
		client->sess.shoutcaster,			// User is shoutcaster
		client->sess.muted,		// User is ignored
		client->sess.uci,			// mcwf's GeoIP
		client->sess.ip,			// L0 - IP
		client->sess.guid,			// Guid
		client->sess.rounds,		// rounds played in stopwatch
		client->sess.selectedWeapon,// Selected weapon
		client->sess.specInvited,	// Can watch..
		client->sess.specLocked,	// Spec lock
		client->sess.deaths,
		client->sess.kills,
		client->sess.damage_given,
		client->sess.damage_received,
		client->sess.team_damage,
		client->sess.team_kills,
		client->sess.gibs,
		client->sess.acc_shots,
		client->sess.acc_hits,
		client->sess.headshots,
		client->sess.suicides,
		client->sess.med_given,
		client->sess.ammo_given,
		client->sess.revives,
		client->sess.knifeKills,
		//new below
		client->sess.dyn_planted,
		client->sess.dyn_defused,
		client->sess.obj_captured,
		client->sess.obj_destroyed,
		client->sess.obj_returned,
		client->sess.obj_taken
	);

	var = va( "session%i", client - level.clients );
	trap_Cvar_Set( var, s );
}
/*
================
G_ClientSwap
Client swap handling
================
*/
void G_ClientSwap( gclient_t *client ) {
	int flags = 0;

	if ( client->sess.sessionTeam == TEAM_RED ) {
		client->sess.sessionTeam = TEAM_BLUE;
	} else if ( client->sess.sessionTeam == TEAM_BLUE )   {
		client->sess.sessionTeam = TEAM_RED;
	}
	// Swap spec invites as well
	if ( client->sess.specInvited & TEAM_RED ) {
		flags |= TEAM_BLUE;

	}
	if ( client->sess.specInvited & TEAM_BLUE ) {
		flags |= TEAM_RED;

	}

	client->sess.specInvited = flags;

	flags = 0;
	// Swap spec follows as well
	if ( client->sess.specLocked & TEAM_RED ) {
		flags |= TEAM_BLUE;
	}
	if ( client->sess.specLocked & TEAM_BLUE ) {
		flags |= TEAM_RED;
	}

	client->sess.specLocked = flags;

}
/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char s[MAX_STRING_CHARS];
	const char  *var;
	qboolean test;

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof( s ) );

    sscanf( s, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %s %s %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",       //  updated for new stats
			(int *)&client->sess.sessionTeam,
			&client->sess.spectatorTime,
			(int *)&client->sess.spectatorState,
			&client->sess.spectatorClient,
			&client->sess.wins,
			&client->sess.losses,
			&client->sess.playerType,       // DHM - Nerve
			&client->sess.playerWeapon,     // DHM - Nerve
			&client->sess.playerItem,       // DHM - Nerve
			&client->sess.playerSkin,       // DHM - Nerve
			&client->sess.spawnObjectiveIndex, // DHM - Nerve
			&client->sess.latchPlayerType,  // DHM - Nerve
			&client->sess.latchPlayerWeapon, // DHM - Nerve
			&client->sess.latchPlayerItem,  // DHM - Nerve
			&client->sess.latchPlayerSkin,   // DHM - Nerve
			// L0 - New stuff
			(int *)&client->sess.referee,
			(int*)&client->sess.shoutcaster,
			(int *)&client->sess.muted,
			&client->sess.uci,
			(char *)&client->sess.ip,
			(char *)&client->sess.guid,
			&client->sess.rounds,
			&client->sess.selectedWeapon,
			&client->sess.specInvited,
			&client->sess.specLocked,
			&client->sess.deaths,
			&client->sess.kills,
			&client->sess.damage_given,
			&client->sess.damage_received,
			&client->sess.team_damage,
			&client->sess.team_kills,
			&client->sess.gibs,
			&client->sess.acc_shots,
			&client->sess.acc_hits,
			&client->sess.headshots,
			&client->sess.suicides,
			&client->sess.med_given,
			&client->sess.ammo_given,
			&client->sess.revives,
			&client->sess.knifeKills,
			//new below
			&client->sess.dyn_planted,
            &client->sess.dyn_defused,
            &client->sess.obj_captured,
            &client->sess.obj_destroyed,
            &client->sess.obj_returned,
            &client->sess.obj_taken
			);

	// L0 - OSP stats -- pull and parse weapon stats
	*s = 0;
	trap_Cvar_VariableStringBuffer( va( "wstats%i", (int)(client - level.clients) ), s, sizeof( s ) );
	if ( *s ) {
		G_parseStats( s );
		if ( g_gamestate.integer == GS_PLAYING ) {
			client->sess.rounds = g_currentRound.integer + 1; // sess.rounds++;
		}
	}
	// NERVE - SMF
	if ( g_altStopwatchMode.integer ) {
		test = qtrue;
	} else {
		test = g_currentRound.integer == 1;
	}

	if (g_gametype.integer == GT_WOLF_STOPWATCH && test) {
		if (g_tournament.integer && level.warmupSwap ||
			!g_tournament.integer && level.warmupTime > 0
		) {
		    G_ClientSwap( client );
/*
			if (client->sess.sessionTeam == TEAM_RED) {
				client->sess.sessionTeam = TEAM_BLUE;
			}
			else if (client->sess.sessionTeam == TEAM_BLUE) {
				client->sess.sessionTeam = TEAM_RED;
			}
*/
		}
	}

	if (g_swapteams.integer) {
		trap_Cvar_Set("g_swapteams", "0");
		G_ClientSwap( client );
/*
		if (client->sess.sessionTeam == TEAM_RED) {
			client->sess.sessionTeam = TEAM_BLUE;
		}
		else if (client->sess.sessionTeam == TEAM_BLUE) {
			client->sess.sessionTeam = TEAM_RED;
		}
*/
	}
}

/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo ) {
	clientSession_t *sess;
	const char      *value;

	sess = &client->sess;

	// initial team determination
	if ( g_gametype.integer >= GT_TEAM ) {
		// always spawn as spectator in team games
		sess->sessionTeam = TEAM_SPECTATOR;
	} else {
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		} else {
			switch ( g_gametype.integer ) {
			default:
			case GT_FFA:
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 &&
					 level.numNonSpectatorClients >= g_maxGameClients.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_TOURNAMENT:
				// if the game is full, go into a waiting mode
				if ( level.numNonSpectatorClients >= 2 ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	// DHM - Nerve
	sess->latchPlayerType = sess->playerType = 0;
	sess->latchPlayerWeapon = sess->playerWeapon = 0;
	sess->latchPlayerItem = sess->playerItem = 0;
	sess->latchPlayerSkin = sess->playerSkin = 0;

	sess->spawnObjectiveIndex = 0;
	// dhm - end
	// L0 - New stuff
	sess->muted = 0;
	sess->uci = 0;
	Q_strncpyz(sess->ip, "", sizeof(sess->ip));
	sess->selectedWeapon = 0;
	G_deleteStats( client - level.clients ); // OSP - Stats
	sess->specInvited = 0;
	sess->specLocked = 0;

	G_WriteClientSessionData( client );
}

/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char s[MAX_STRING_CHARS];
	int gt;

	trap_Cvar_VariableStringBuffer( "session", s, sizeof( s ) );
	gt = atoi( s );

	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt ) {
		level.newSession = qtrue;
		level.fResetStats = qtrue; // L0 - OSP Stats
		G_Printf( "Gametype changed, clearing session data.\n" );
	// L0 - OSP speclock port
	} else {
		char* tmp = s;
		qboolean test = (g_altStopwatchMode.integer != 0 || g_currentRound.integer == 1);

#define GETVAL( x ) if ( ( tmp = strchr( tmp, ' ' ) ) == NULL ) {return; \
						   } x = atoi( ++tmp );

		// Get team lock stuff
		GETVAL(gt);
		teamInfo[TEAM_RED].spec_lock = (gt & TEAM_RED) ? qtrue : qfalse;
		teamInfo[TEAM_BLUE].spec_lock = (gt & TEAM_BLUE) ? qtrue : qfalse;

		if ((tmp = strchr(va("%s", tmp), ' ')) != NULL) {
			tmp++;
			trap_GetServerinfo(s, sizeof(s));
			if (Q_stricmp(tmp, Info_ValueForKey(s, "mapname"))) {
				level.fResetStats = qtrue;
				G_Printf("Map changed, clearing player stats.\n");
			}
		}

		// OSP - have to make sure spec locks follow the right teams
		if (g_gametype.integer == GT_WOLF_STOPWATCH && g_gamestate.integer != GS_PLAYING && test) {
			G_swapTeamLocks();
		}

		if (g_swapteams.integer) {
			G_swapTeamLocks();
		}
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int i;

	// L0 - Speclock
	trap_Cvar_Set( "session",
		va( "%i %i", g_gametype.integer,
			( teamInfo[TEAM_RED].spec_lock * TEAM_RED | teamInfo[TEAM_BLUE].spec_lock * TEAM_BLUE )
		));

	// L0 - OSP Stats
	// Keep stats for all players in sync
	if (!level.fResetStats && level.numConnectedClients > 0) {
		if ((g_gamestate.integer == GS_WARMUP_COUNTDOWN &&
			((g_gametype.integer == GT_WOLF_STOPWATCH && g_currentRound.integer == 0) ||
				(g_gametype.integer != GT_WOLF_STOPWATCH && level.clients[level.sortedClients[0]].sess.rounds >= 1)))) {
			level.fResetStats = qtrue;
		}
	}

	for ( i = 0; i < level.numConnectedClients; i++ ) {
		if ( level.clients[level.sortedClients[i]].pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( &level.clients[level.sortedClients[i]]);
			// For slow connecters and a short warmup
		}
		
		if ( level.fResetStats ) {
			G_deleteStats( level.sortedClients[i] );
			if (g_currentRound.integer == 1 && g_gameStatslog.integer) G_read_round_jstats();
		}
	}

	/*for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected == CON_CONNECTED) {
			G_WriteClientSessionData(&level.clients[i]);
		}
	}*/
}
