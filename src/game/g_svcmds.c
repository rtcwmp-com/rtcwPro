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




// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"


/*
==============================================================================

PACKET FILTERING


You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and you can use '*' to match any value
so you can specify an entire class C network with "addip 192.246.40.*"

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.

TTimo NOTE: GUID functions are copied over from the model of IP banning,
used to enforce max lives independently from server reconnect and team changes (Xian)

TTimo NOTE: for persistence, bans are stored in g_banIPs cvar MAX_CVAR_VALUE_STRING
The size of the cvar string buffer is limiting the banning to around 20 masks
this could be improved by putting some g_banIPs2 g_banIps3 etc. maybe
still, you should rely on PB for banning instead

==============================================================================
*/

typedef struct ipFilter_s
{
	unsigned mask;
	unsigned compare;
} ipFilter_t;

typedef struct ipGUID_s
{
	char compare[33];
} ipGUID_t;

#define MAX_IPFILTERS   1024

typedef struct ipFilterList_s {
	ipFilter_t ipFilters[MAX_IPFILTERS];
	int numIPFilters;
	char cvarIPList[32];
} ipFilterList_t;

static ipFilterList_t ipFilters;
static ipFilterList_t ipMaxLivesFilters;
/*#ifdef USEXPSTORAGE
static ipXPStorageList_t ipXPStorage;
#endif
*/
static ipGUID_t guidMaxLivesFilters[MAX_IPFILTERS];
static int numMaxLivesFilters = 0;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter( const char *s, ipFilter_t *f ) {
	char num[128];
	int i, j;
	byte b[4];
	byte m[4];

	for ( i = 0 ; i < 4 ; i++ )
	{
		b[i] = 0;
		m[i] = 0;
	}

	for ( i = 0 ; i < 4 ; i++ )
	{
		if ( *s < '0' || *s > '9' ) {
			if ( *s == '*' ) { // 'match any'
				// b[i] and m[i] to 0
				s++;
				if ( !*s ) {
					break;
				}
				s++;
				continue;
			}
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}

		j = 0;
		while ( *s >= '0' && *s <= '9' )
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi( num );
		m[i] = 255;

		if ( !*s ) {
			break;
		}
		s++;
	}

	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;

	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans(ipFilterList_t *ipFilterList) {
	byte b[4];
	byte m[4];
	int i,j;
	char iplist_final[MAX_CVAR_VALUE_STRING];
	char ip[64];

	*iplist_final = 0;
	for ( i = 0 ; i < ipFilterList->numIPFilters ; i++ )
	{
		if (ipFilterList->ipFilters[i].compare == 0xffffffff ) {
			continue;
		}

		*(unsigned *)b = ipFilterList->ipFilters[i].compare;
		*(unsigned *)m = ipFilterList->ipFilters[i].mask;
		*ip = 0;
		for ( j = 0; j < 4 ; j++ ) {
			if ( m[j] != 255 ) {
				Q_strcat( ip, sizeof( ip ), "*" );
			} else {
				Q_strcat( ip, sizeof( ip ), va( "%i", b[j] ) );
			}
			Q_strcat( ip, sizeof( ip ), ( j < 3 ) ? "." : " " );
		}
		if ( strlen( iplist_final ) + strlen( ip ) < MAX_CVAR_VALUE_STRING ) {
			Q_strcat( iplist_final, sizeof( iplist_final ), ip );
		} else
		{
			Com_Printf( "%s overflowed at MAX_CVAR_VALUE_STRING\n", ipFilterList->cvarIPList );
			break;
		}
	}

	trap_Cvar_Set( ipFilterList->cvarIPList, iplist_final );
}

void PrintMaxLivesGUID() {
	int i;

	for ( i = 0 ; i < numMaxLivesFilters ; i++ )
	{
		G_LogPrintf( "%i. %s\n", i, guidMaxLivesFilters[i].compare );
	}
	G_LogPrintf( "--- End of list\n" );
}

/*
=================
G_FindIpData
=================


ipXPStorage_t* G_FindIpData( ipXPStorageList_t *ipXPStorageList, char *from ) {
	int i;
	unsigned in;
	byte m[4];
	char *p;

	i = 0;
	p = from;
	while ( *p && i < 4 ) {
		m[i] = 0;
		while ( *p >= '0' && *p <= '9' ) {
			m[i] = m[i] * 10 + ( *p - '0' );
			p++;
		}
		if ( !*p || *p == ':' ) {
			break;
		}
		i++, p++;
	}

	in = *(unsigned *)m;

	for ( i = 0; i < MAX_IPFILTERS; i++ ) {
		if ( !ipXPStorageList->ipFilters[ i ].timeadded || level.time - ipXPStorageList->ipFilters[ i ].timeadded > ( 5 * 60000 ) ) {
			continue;
		}

		if ( ( in & ipXPStorageList->ipFilters[ i ].filter.mask ) == ipXPStorageList->ipFilters[ i ].filter.compare ) {
			return &ipXPStorageList->ipFilters[ i ];
		}
	}

	return NULL;
}
*/
/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket(ipFilterList_t *ipFilterList, char *from ) {
	int i;
	unsigned in;
	byte m[4];
	char *p;

	i = 0;
	p = from;
	while ( *p && i < 4 ) {
		m[i] = 0;
		while ( *p >= '0' && *p <= '9' ) {
			m[i] = m[i] * 10 + ( *p - '0' );
			p++;
		}
		if ( !*p || *p == ':' ) {
			break;
		}
		i++, p++;
	}

	in = *(unsigned *)m;

	for (i = 0; i < ipFilterList->numIPFilters; i++)
		if ((in & ipFilterList->ipFilters[i].mask) == ipFilterList->ipFilters[i].compare) {
			return g_filterBan.integer != 0;
		}

	return g_filterBan.integer == 0;
}

qboolean G_FilterIPBanPacket( char *from ) {
	return( G_FilterPacket( &ipFilters, from ) );
}

qboolean G_FilterMaxLivesIPPacket( char *from ) {
	return( G_FilterPacket( &ipMaxLivesFilters, from ) );
}

#ifdef USEXPSTORAGE
ipXPStorage_t* G_FindXPBackup( char *from ) {
	ipXPStorage_t* storage = G_FindIpData( &ipXPStorage, from );

	if ( storage ) {
		storage->timeadded = 0;
	}

	return storage;
}
#endif // USEXPSTORAGE
/*
 Check to see if the user is trying to sneak back in with g_enforcemaxlives enabled
*/
qboolean G_FilterMaxLivesPacket( char *from ) {
	int i;

	for ( i = 0 ; i < numMaxLivesFilters ; i++ )
	{
		if ( !Q_stricmp( guidMaxLivesFilters[i].compare, from ) ) {
			return 1;
		}
	}
	return 0;
}

/*
=================
AddIP
=================
*/
void AddIP( ipFilterList_t *ipFilterList, const char *str ) {
	int i;

	for ( i = 0; i < ipFilterList->numIPFilters; i++ ) {
		if (  ipFilterList->ipFilters[i].compare == 0xffffffff ) {
			break;
		}               // free spot
	}
	if ( i == ipFilterList->numIPFilters ) {
		if ( ipFilterList->numIPFilters == MAX_IPFILTERS ) {
			G_Printf( "IP filter list is full\n" );
			return;
		}
		ipFilterList->numIPFilters++;
	}

	if ( !StringToFilter( str, &ipFilterList->ipFilters[i] ) ) {
		ipFilterList->ipFilters[i].compare = 0xffffffffu;
	}

	UpdateIPBans( ipFilterList );
}
void AddIPBan(const char *str) {
	AddIP(&ipFilters, str);
}
void AddMaxLivesBan( const char *str ) {
	AddIP( &ipMaxLivesFilters, str );
}
/*
=================
AddMaxLivesGUID
Xian - with g_enforcemaxlives enabled, this adds a client GUID to a list
that prevents them from quitting a disconnecting
=================
*/
void AddMaxLivesGUID( char *str ) {
	if ( numMaxLivesFilters == MAX_IPFILTERS ) {
		G_Printf( "MaxLives GUID filter list is full\n" );
		return;
	}
	Q_strncpyz( guidMaxLivesFilters[numMaxLivesFilters].compare, str, 33 );
	numMaxLivesFilters++;
}


/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans( void ) {
	char *s, *t;
	char str[MAX_CVAR_VALUE_STRING];
	ipFilters.numIPFilters = 0;
	Q_strncpyz( ipFilters.cvarIPList, "g_banIPs", sizeof( ipFilters.cvarIPList ) );

	Q_strncpyz( str, g_banIPs.string, sizeof( str ) );

	for ( t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr( s, ' ' );
		if ( !s ) {
			break;
		}
		while ( *s == ' ' )
			*s++ = 0;
		if ( *t ) {
			AddIP( &ipFilters, t );
		}
		t = s;
	}
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f( void ) {
	char str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf( "Usage:  addip <ip-mask>\n" );
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( &ipFilters, str );

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f( void ) {
	ipFilter_t f;
	int i;
	char str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf( "Usage:  removeip <ip-mask>\n" );
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if ( !StringToFilter( str, &f ) ) {
		return;
	}

	for ( i = 0 ; i < ipFilters.numIPFilters ; i++ ) {
		if ( ipFilters.ipFilters[i].mask == f.mask   &&
			 ipFilters.ipFilters[i].compare == f.compare ) {
			ipFilters.ipFilters[i].compare = 0xffffffffu;
			G_Printf( "Removed.\n" );

			UpdateIPBans( &ipFilters );
			return;
		}
	}

	G_Printf( "Didn't find %s.\n", str );
}

/*
 Xian - Clears out the entire list maxlives enforcement banlist
*/
void ClearMaxLivesBans() {
	int i;

	for ( i = 0 ; i < numMaxLivesFilters ; i++ ) {
		guidMaxLivesFilters[i].compare[0] = '\0';
	}
	numMaxLivesFilters = 0;
	ipMaxLivesFilters.numIPFilters = 0;
	Q_strncpyz( ipMaxLivesFilters.cvarIPList, "g_maxlivesbanIPs", sizeof( ipMaxLivesFilters.cvarIPList ) );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void    Svcmd_EntityList_f( void ) {
	int e;
	gentity_t       *check;

	check = g_entities + 1;
	for ( e = 1; e < level.num_entities ; e++, check++ ) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf( "%3i:", e );
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf( "ET_GENERAL          " );
			break;
		case ET_PLAYER:
			G_Printf( "ET_PLAYER           " );
			break;
		case ET_ITEM:
			G_Printf( "ET_ITEM             " );
			break;
		case ET_MISSILE:
			G_Printf( "ET_MISSILE          " );
			break;
		case ET_MOVER:
			G_Printf( "ET_MOVER            " );
			break;
		case ET_BEAM:
			G_Printf( "ET_BEAM             " );
			break;
		case ET_PORTAL:
			G_Printf( "ET_PORTAL           " );
			break;
		case ET_SPEAKER:
			G_Printf( "ET_SPEAKER          " );
			break;
		case ET_PUSH_TRIGGER:
			G_Printf( "ET_PUSH_TRIGGER     " );
			break;
// JPW NERVE
		case ET_CONCUSSIVE_TRIGGER:
			G_Printf( "ET_CONCUSSIVE_TRIGGR" );
			break;
// jpw
		case ET_TELEPORT_TRIGGER:
			G_Printf( "ET_TELEPORT_TRIGGER " );
			break;
		case ET_INVISIBLE:
			G_Printf( "ET_INVISIBLE        " );
			break;
		case ET_GRAPPLE:
			G_Printf( "ET_GRAPPLE          " );
			break;
		case ET_EXPLOSIVE:
			G_Printf( "ET_EXPLOSIVE        " );
			break;
		case ET_EF_TESLA:
			G_Printf( "ET_EF_TESLA         " );
			break;
		case ET_EF_SPOTLIGHT:
			G_Printf( "ET_EF_SPOTLIGHT     " );
			break;
		case ET_EFFECT3:
			G_Printf( "ET_EFFECT3          " );
			break;
		case ET_ALARMBOX:
			G_Printf( "ET_ALARMBOX          " );
			break;
		default:
			G_Printf( "%3i                 ", check->s.eType );
			break;
		}

		if ( check->classname ) {
			G_Printf( "%s", check->classname );
		}
		G_Printf( "\n" );
	}
}

gclient_t   *ClientForString( const char *s ) {
	gclient_t   *cl;
	int i;
	int idnum;
	// check for a name match
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	G_Printf( "User %s is not on the server\n", s );
	// check for a name match
	return NULL;
}
// fretn

static qboolean G_Is_SV_Running( void ) {

	char cvar[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer( "sv_running", cvar, sizeof( cvar ) );
	return (qboolean)atoi( cvar );
}

/*
==================
G_GetPlayerByNum
==================
*/
gclient_t   *G_GetPlayerByNum( int clientNum ) {
	gclient_t   *cl;


	// make sure server is running
	if ( !G_Is_SV_Running() ) {
		return NULL;
	}

	if ( trap_Argc() < 2 ) {
		G_Printf( "No player specified.\n" );
		return NULL;
	}

	if ( clientNum < 0 || clientNum >= level.maxclients ) {
		Com_Printf( "Bad client slot: %i\n", clientNum );
		return NULL;
	}

	cl = &level.clients[clientNum];
	if ( cl->pers.connected == CON_DISCONNECTED ) {
		G_Printf( "Client %i is not connected\n", clientNum );
		return NULL;
	}

	if ( cl ) {
		return cl;
	}


	G_Printf( "User %d is not on the server\n", clientNum );

	return NULL;
}

/*
==================
G_GetPlayerByName
==================
*/
gclient_t *G_GetPlayerByName( char *name ) {

	int i;
	gclient_t   *cl;
	char cleanName[64];

	// make sure server is running
	if ( !G_Is_SV_Running() ) {
		return NULL;
	}

	if ( trap_Argc() < 2 ) {
		G_Printf( "No player specified.\n" );
		return NULL;
	}

	for ( i = 0; i < level.numConnectedClients; i++ ) {

		cl = &level.clients[i];

		if ( !Q_stricmp( cl->pers.netname, name ) ) {
			return cl;
		}

		Q_strncpyz( cleanName, cl->pers.netname, sizeof( cleanName ) );
		Q_CleanStr( cleanName );
		if ( !Q_stricmp( cleanName, name ) ) {
			return cl;
		}
	}

	G_Printf( "Player %s is not on the server\n", name );

	return NULL;
}

// -fretn
/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void    Svcmd_ForceTeam_f( void ) {
	gclient_t   *cl;
	char str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str ,qfalse);
}

/*
============
Svcmd_StartMatch_f

NERVE - SMF - starts match if in tournament mode
============
*/
void Svcmd_StartMatch_f() {
/*	if ( !g_noTeamSwitching.integer ) {
		trap_SendServerCommand( -1, va( "print \"g_noTeamSwitching not activated.\n\"" ) );
		return;
	}
*/

	G_refAllReady_cmd( NULL );

/*
	if ( level.numPlayingClients <= 1 ) {
		trap_SendServerCommand( -1, va( "print \"Not enough playing clients to start match.\n\"" ) );
		return;
	}

	if ( g_gamestate.integer == GS_PLAYING ) {
		trap_SendServerCommand( -1, va( "print \"Match is already in progress.\n\"" ) );
		return;
	}

	if ( g_gamestate.integer == GS_WAITING_FOR_PLAYERS ) {
		trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", GS_WARMUP ) );
	}
	*/
}

/*
============
Svcmd_ResetMatch_f

NERVE - SMF - this has three behaviors
- if not in tournament mode, do a map_restart
- if in tournament mode, go back to waitingForPlayers mode
- if in stopwatch mode, reset back to first round
============
*/
void Svcmd_ResetMatch_f(qboolean fDoReset, qboolean fDoRestart) {
	int i;

	for (i = 0; i < level.numConnectedClients; i++) {
		g_entities[level.sortedClients[i]].client->pers.ready = qfalse;
		//g_entities[level.sortedClients[i]].client->ps.persistant[PERS_RESTRICTEDWEAPON] = WP_NONE; // reset weapon restrictions on restart
	}

	// reset all the weapon restrictions so next time the players spawn they get set correctly
	level.alliedFlamer = level.axisFlamer = 0;
	level.alliedSniper = level.axisSniper = 0;
	level.alliedPF = level.axisPF = 0;
	level.alliedVenom = level.axisVenom = 0;

	if (g_currentRound.integer == 1 && !fDoReset && fDoRestart && g_gameStatslog.integer) level.exitEarly = qtrue;  // planned something different and still might use it

    if (g_gamestate.integer == GS_PLAYING && g_gameStatslog.integer) {
        G_writeGameEarlyExit();  // properly close current stats output

    }

	if (fDoReset) {
		G_resetRoundState();
		G_resetModeState();
	}



	if (fDoRestart && !g_noTeamSwitching.integer || (g_minGameClients.integer > 1 && level.numPlayingClients >= g_minGameClients.integer)) {



		trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP));
		return;
	}
	else { // L0 - Tournament..
		if (g_tournament.integer) {
			trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP));
			trap_SetConfigstring(CS_READY, va("%i", READY_PENDING));
		}
		else {
			trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WAITING_FOR_PLAYERS));
		}
		return;
	}
}

/*
============
Svcmd_SwapTeams_f

NERVE - SMF - swaps all clients to opposite team
============
*/
void Svcmd_SwapTeams_f() {
//  if ( g_gamestate.integer != GS_PLAYING ) {
/*	if ( ( g_gamestate.integer == GS_INITIALIZE ) || // JPW NERVE -- so teams can swap between checkpoint rounds
		 ( g_gamestate.integer == GS_WAITING_FOR_PLAYERS ) ||
		 ( g_gamestate.integer == GS_RESET ) ) {
		trap_SendServerCommand( -1, va( "print \"Match must be in progress to swap teams.\n\"" ) );
		return;
	}
*/
	if ((g_gamestate.integer == GS_INITIALIZE) ||
	    (g_gamestate.integer == GS_WARMUP) ||
	    ( g_gamestate.integer == GS_WAITING_FOR_PLAYERS ) ||
	    (g_gamestate.integer == GS_RESET))
	{
		G_swapTeams();
		return;
	}
	if ( g_gametype.integer == GT_WOLF_STOPWATCH ) {
		trap_Cvar_Set( "g_currentRound", "0" );
		trap_Cvar_Set( "g_nextTimeLimit", "0" );
	}
/*
	// L0 - locked team switch fix on swap
	if (g_gamelocked.integer == 2)
		trap_Cvar_Set( "g_gamelocked", "1" );
	else if (g_gamelocked.integer == 1)
		trap_Cvar_Set( "g_gamelocked", "2" );
	// L0 - end
	*/
    if (g_gamestate.integer == GS_PLAYING && g_gameStatslog.integer) {
        G_writeGameEarlyExit();  // properly close current stats output
    }
	trap_Cvar_Set( "g_swapteams", "1" );

	trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", GS_WARMUP ) );
}



char    *ConcatArgs( int start );
/*
===========
L0 - Shuffle
===========
*/
void Svcmd_Shuffle_f( void )
{
	int count=0, tmpCount, i;
	int players[MAX_CLIENTS];

	memset(players, -1, sizeof(players));

	if (g_gamestate.integer == GS_RESET)
	return;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		//skip client numbers that aren't used
		if ((!g_entities[i].inuse) || (level.clients[i].pers.connected != CON_CONNECTED))
			continue;

		//ignore spectators
		if ((level.clients[i].sess.sessionTeam != TEAM_RED) && (level.clients[i].sess.sessionTeam != TEAM_BLUE))
			continue;

		players[count] = i;
		count++;
	}

	tmpCount = count;	//copy the number of active clients

	//loop through all the active players
	for (i = 0; i < count; i++)
	{
		int j;

		do {
			j = (rand() % count);
		} while (players[j] == -1);

		//put every other random choice on allies
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

	// Reset match if there's a shuffle!
	Svcmd_ResetMatch_f( qfalse, qtrue );

	AP("chat \"^zconsole:^7 Teams were shuffled^1!\n\"");
}
/*
=================
L0 - Antilag
=================
*/
void Svcmd_Antilag_f( void ) {

	if ( g_antilag.integer != 0 ) {
		trap_SendConsoleCommand( EXEC_APPEND, "g_antilag 0\n" );
		AP("chat \"^zconsole:^7 Antilag has been disbled^1!\n\"");
	} else {
		trap_SendConsoleCommand( EXEC_APPEND, "g_antilag 1\n" );
		AP("chat \"^zconsole:^7 Antilag has been enabled^2!\n\"");
	}
}

/*
====================
Svcmd_ShuffleTeams_f

OSP - randomly places players on teams
====================
*/
void Svcmd_ShuffleTeams_f(void) {
	G_resetRoundState();
	G_shuffleTeams();

	if ((g_gamestate.integer == GS_INITIALIZE) ||
		(g_gamestate.integer == GS_WARMUP) ||
		(g_gamestate.integer == GS_RESET)) {
		return;
	}

	G_resetModeState();
	Svcmd_ResetMatch_f(qfalse, qtrue);
}

/*
=================
L0 - Pause/Unpause
=================
*/
void Svcmd_Pause_f(qboolean pause) {
	if (pause) {
		level.paused = !PAUSE_NONE;
		trap_SetConfigstring( CS_PAUSED,  va( "%i", level.paused ));
		AP(va("cp \"Match has been ^3Paused^7!\n\"2"));
	} else {
		level.CNstart = 0; // Resets countdown if it was aborted before
		level.paused = PAUSE_UNPAUSING;
		AP(va("cp \"Resuming match..\n\"2"));
	}
}

/*
=================
CC_loadconfig
=================
*/
void CC_loadconfig(void)
{
	char scriptName[MAX_QPATH];

	if (trap_Argc() != 2)
	{
		G_Printf("usage: loadConfig <config name>\n");
		return;
	}

	trap_Argv(1, scriptName, sizeof(scriptName));

	memset(&level.config, 0, sizeof(config_t));
	G_ConfigSet(scriptName);
}

/*
=================
G_UpdateSvCvars
=================
*/
void G_UpdateSvCvars(void)
{
	char cs[MAX_INFO_STRING];
	//char cs[BIG_INFO_STRING];
	int  i;

	cs[0] = '\0';
	for (i = 0; i < level.svCvarsCount; i++)
	{
		if (level.svCvars[i].Val2[0] == 0) // don't send a space char when not set
		{
			Info_SetValueForKey(cs, va("V%i", i),
				va("%i %s %s", level.svCvars[i].mode, level.svCvars[i].cvarName, level.svCvars[i].Val1));
		}
		else
		{
			Info_SetValueForKey(cs, va("V%i", i),
				va("%i %s %s %s", level.svCvars[i].mode, level.svCvars[i].cvarName, level.svCvars[i].Val1, level.svCvars[i].Val2));
		}
	}

	Info_SetValueForKey(cs, "N", va("%i", level.svCvarsCount));

	// FIXME: print a warning when this configstring has nearly reached MAX_INFO_STRING size and don't set it if greater
	trap_SetConfigstring(CS_SVCVAR, cs);

}

/*
=================
CC_cvarempty
=================
*/
void CC_cvarempty(void)
{
	memset(level.svCvars, 0, sizeof(level.svCvars));
	level.svCvarsCount = 0;
	G_UpdateSvCvars();
}

/*
=================
CC_svcvar

brief Forces client cvar to a specific value
=================
*/
void CC_svcvar(void)
{
	char cvarName[MAX_CVAR_VALUE_STRING];
	char mode[16];
	char cvarValue1[MAX_CVAR_VALUE_STRING];
	char cvarValue2[MAX_CVAR_VALUE_STRING];
	int  i;
	int  index = level.svCvarsCount;
	char* p;

	if (trap_Argc() <= 3)
	{
		G_Printf("usage: sv_cvar <cvar name> <mode> <value1> <value2>\nexamples: sv_cvar r_rmse EQ 0\n          sv_cvar cl_maxpackets IN 60 125\n");
		return;
	}

	trap_Argv(1, cvarName, sizeof(cvarName));
	trap_Argv(2, mode, sizeof(mode));
	trap_Argv(3, cvarValue1, sizeof(cvarValue1));

	for (p = cvarName; *p != '\0'; ++p)
	{
		*p = tolower(*p);
	}

	if (trap_Argc() == 5)
	{
		trap_Argv(4, cvarValue2, sizeof(cvarValue2));

	}
	else
	{
		cvarValue2[0] = '\0';
	}

	// is this cvar already in the array?.. (maybe they have a double entry)
	for (i = 0; i < level.svCvarsCount; i++)
	{
		if (!Q_stricmp(cvarName, level.svCvars[i].cvarName))
		{
			index = i;
		}
	}

	if (index >= MAX_SVCVARS)
	{
		G_Printf("sv_cvar: MAX_SVCVARS hit\n");
		return;
	}

	if (!Q_stricmp(mode, "EQ") || !Q_stricmp(mode, "EQUAL"))
	{
		level.svCvars[index].mode = SVC_EQUAL;
	}
	else if (!Q_stricmp(mode, "G") || !Q_stricmp(mode, "GREATER"))
	{
		level.svCvars[index].mode = SVC_GREATER;
	}
	else if (!Q_stricmp(mode, "GE") || !Q_stricmp(mode, "GREATEREQUAL"))
	{
		level.svCvars[index].mode = SVC_GREATEREQUAL;
	}
	else if (!Q_stricmp(mode, "L") || !Q_stricmp(mode, "LOWER"))
	{
		level.svCvars[index].mode = SVC_LOWER;
	}
	else if (!Q_stricmp(mode, "LE") || !Q_stricmp(mode, "LOWEREQUAL"))
	{
		level.svCvars[index].mode = SVC_LOWEREQUAL;
	}
	else if (!Q_stricmp(mode, "IN") || !Q_stricmp(mode, "INSIDE"))
	{
		level.svCvars[index].mode = SVC_INSIDE;
	}
	else if (!Q_stricmp(mode, "OUT") || !Q_stricmp(mode, "OUTSIDE"))
	{
		level.svCvars[index].mode = SVC_OUTSIDE;
	}
	else if (!Q_stricmp(mode, "INC") || !Q_stricmp(mode, "INCLUDE"))
	{
		level.svCvars[index].mode = SVC_INCLUDE;
	}
	else if (!Q_stricmp(mode, "EXC") || !Q_stricmp(mode, "EXCLUDE"))
	{
		level.svCvars[index].mode = SVC_EXCLUDE;
	}
	else if (!Q_stricmp(mode, "WB") || !Q_stricmp(mode, "WITHBITS"))
	{
		level.svCvars[index].mode = SVC_WITHBITS;
	}
	else if (!Q_stricmp(mode, "WOB") || !Q_stricmp(mode, "WITHOUTBITS"))
	{
		level.svCvars[index].mode = SVC_WITHOUTBITS;
	}
	else
	{
		G_Printf("sv_cvar: invalid mode\n");
		return;
	}

	if (trap_Argc() == 5)
	{
		Q_strncpyz(level.svCvars[index].Val2, cvarValue2, sizeof(level.svCvars[0].Val2));
	}
	else
	{
		Q_strncpyz(level.svCvars[index].Val2, "", sizeof(level.svCvars[0].Val2));
	}

	Q_strncpyz(level.svCvars[index].cvarName, cvarName, sizeof(level.svCvars[0].cvarName));
	Q_strncpyz(level.svCvars[index].Val1, cvarValue1, sizeof(level.svCvars[0].Val1));

	// cvar wasn't yet in the array?
	if (index >= level.svCvarsCount)
	{
		level.svCvarsCount++;
	}

//	G_UpdateSvCvars();   // cause of lag on map_restart...moved call after all cvars are loaded
}

/*
=================
ConsoleCommand

=================
*/
qboolean    ConsoleCommand( void ) {
	char cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp( cmd, "entitylist" ) == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "forceteam" ) == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "game_memory" ) == 0 ) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "addbot" ) == 0 ) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "addip" ) == 0 ) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "removeip" ) == 0 ) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "listip" ) == 0 ) {
		trap_SendConsoleCommand( EXEC_INSERT, "g_banIPs\n" );
		return qtrue;
	}

	if ( Q_stricmp( cmd, "listmaxlivesip" ) == 0 ) {
		PrintMaxLivesGUID();
		return qtrue;
	}


	// NERVE - SMF
	if ( Q_stricmp( cmd, "startmatch" ) == 0 ) {
		Svcmd_StartMatch_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "resetmatch" ) == 0 ) {
		Svcmd_ResetMatch_f(qtrue, qtrue);
		return qtrue;
	}

	if ( Q_stricmp( cmd, "swap_teams" ) == 0 ) {
		Svcmd_SwapTeams_f();
		return qtrue;
	}
	// -NERVE - SMF

	// L0 - Callvotes and server side (console) handling
	// Shuffle
	if ( Q_stricmp( cmd, "shuffle" ) == 0 ) {
		Svcmd_Shuffle_f();
	return qtrue;
	}
	// Antilag
	if ( Q_stricmp( cmd, "antilag" ) == 0 ) {
		Svcmd_Antilag_f();
		return qtrue;
	}
	// Pause
	if ( Q_stricmp( cmd, "pause" ) == 0 ) {
		Svcmd_Pause_f(qtrue);
		return qtrue;
	}
	// UnPause
	if ( Q_stricmp( cmd, "unpause" ) == 0 ) {
		Svcmd_Pause_f(qfalse);
		return qtrue;
	}

	// RTCWPro - cvar limiting
	if (Q_stricmp(cmd, "sv_cvarempty") == 0) {
		CC_cvarempty();
		return qtrue;
	}
	if (Q_stricmp(cmd, "sv_cvarload") == 0) {
		G_UpdateSvCvars();

		return qtrue;
	}
	if (Q_stricmp(cmd, "sv_cvar") == 0) {
		CC_svcvar();
		return qtrue;
	}
	// RTCWPro

	// RTCWPro - custom config
	if (Q_stricmp(cmd, "reloadConfig") == 0) {
		G_ReloadConfig();
		return qtrue;
	}
	if (Q_stricmp(cmd, "loadConfig") == 0) {
		CC_loadconfig();
		return qtrue;
	}

	// RTCWPro
	if ( g_dedicated.integer ) {
		if ( Q_stricmp( cmd, "say" ) == 0 ) {
			trap_SendServerCommand( -1, va( "print \"server:[lof] %s\"", ConcatArgs( 1 ) ) );
			return qtrue;
		}
		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va( "print \"server:[lof] %s\"", ConcatArgs( 0 ) ) );
		return qtrue;
	}

	return qfalse;
}

