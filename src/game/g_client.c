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
#include <time.h>
// g_client.c -- client functions that don't happen every frame

// Ridah, new bounding box
//static vec3_t	playerMins = {-15, -15, -24};
//static vec3_t	playerMaxs = {15, 15, 32};
//vec3_t playerMins = {-18, -18, -24};
//vec3_t playerMaxs = {18, 18, 48};
vec3_t	playerMins = {-18, -18, -24}; //-24
vec3_t	playerMaxs = {18, 18, 48}; //51};  //elver fix bounding box, fuck yea!
// done.

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
If the start position is targeting an entity, the players camera will start out facing that ent (like an info_notnull)
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int i;
	vec3_t dir;

	G_SpawnInt( "nobots", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}

	ent->enemy = G_PickTarget( ent->target );
	if ( ent->enemy ) {
		VectorSubtract( ent->enemy->s.origin, ent->s.origin, dir );
		vectoangles( dir, ent->s.angles );
	}

}

//----(SA) added
/*QUAKED info_player_checkpoint (1 0 0) (-16 -16 -24) (16 16 32) a b c d
these are start points /after/ the level start
the letter (a b c d) designates the checkpoint that needs to be complete in order to use this start position
*/
void SP_info_player_checkpoint( gentity_t *ent ) {
	ent->classname = "info_player_checkpoint";
	SP_info_player_deathmatch( ent );
}

//----(SA) end


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start( gentity_t *ent ) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32) AXIS ALLIED
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
	int i, num;
	int touch[MAX_GENTITIES];
	gentity_t   *hit;
	vec3_t mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for ( i = 0 ; i < num ; i++ ) {
		hit = &g_entities[touch[i]];
		if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
			return qtrue;
		}

	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define MAX_SPAWN_POINTS    128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
	gentity_t   *spot;
	vec3_t delta;
	float dist, nearestDist;
	gentity_t   *nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ( ( spot = G_Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL ) {

		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define MAX_SPAWN_POINTS    128
gentity_t *SelectRandomDeathmatchSpawnPoint( void ) {
	gentity_t   *spot;
	int count;
	int selection;
	gentity_t   *spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while ( ( spot = G_Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL ) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}
		spots[ count ] = spot;
		count++;
	}

	if ( !count ) { // no spots that won't telefrag
		return G_Find( NULL, FOFS( classname ), "info_player_deathmatch" );
	}

	selection = rand() % count;
	return spots[ selection ];
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles ) {
	gentity_t   *spot;
	gentity_t   *nearestSpot;

	nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

	spot = SelectRandomDeathmatchSpawnPoint();
	if ( spot == nearestSpot ) {
		// roll again if it would be real close to point of death
		spot = SelectRandomDeathmatchSpawnPoint();
		if ( spot == nearestSpot ) {
			// last try
			spot = SelectRandomDeathmatchSpawnPoint();
		}
	}

	// find a single player start spot
	if ( !spot ) {
		G_Error( "Couldn't find a spawn point" );
	}

	VectorCopy( spot->s.origin, origin );
	origin[2] += 9;
	VectorCopy( spot->s.angles, angles );

	return spot;
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles ) {
	gentity_t   *spot;

	spot = NULL;
	while ( ( spot = G_Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL ) {
		if ( spot->spawnflags & 1 ) {
			break;
		}
	}

	if ( !spot || SpotWouldTelefrag( spot ) ) {
		return SelectSpawnPoint( vec3_origin, origin, angles );
	}

	VectorCopy( spot->s.origin, origin );
	origin[2] += 9;
	VectorCopy( spot->s.angles, angles );

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue( void ) {
	int i;
	gentity_t   *ent;

	level.bodyQueIndex = 0;
	for ( i = 0; i < BODY_QUEUE_SIZE ; i++ ) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
	if ( level.time - ent->timestamp > 6500 ) {
		// the body ques are never actually freed, they are just unlinked
		trap_UnlinkEntity( ent );
		ent->physicsObject = qfalse;
		return;
	}
	ent->nextthink = level.time + 100;
	ent->s.pos.trBase[2] -= 1;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent ) {
	gentity_t       *body;
	int contents, i;

	trap_UnlinkEntity( ent );

	// if client is in a nodrop area, don't leave the body
	contents = trap_PointContents( ent->s.origin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		return;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = ( level.bodyQueIndex + 1 ) % BODY_QUEUE_SIZE;

	trap_UnlinkEntity( body );

	body->s = ent->s;
	body->s.eFlags = EF_DEAD;       // clear EF_TALK, etc

	if ( ent->client->ps.eFlags & EF_HEADSHOT ) {
		body->s.eFlags |= EF_HEADSHOT;          // make sure the dead body draws no head (if killed that way)

	}
	body->s.eType = ET_CORPSE;
	body->classname = "corpse";
	body->s.powerups = 0;   // clear powerups
	body->s.loopSound = 0;  // clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;        // don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

	// DHM - Clear out event system
	for ( i = 0; i < MAX_EVENTS; i++ )
		body->s.events[i] = 0;
	body->s.eventSequence = 0;

	// DHM - Nerve
	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		// change the animation to the last-frame only, so the sequence
		// doesn't repeat anew for the body
		switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
		case BOTH_DEATH1:
		case BOTH_DEAD1:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
			break;
		case BOTH_DEATH2:
		case BOTH_DEAD2:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
			break;
		case BOTH_DEATH3:
		case BOTH_DEAD3:
		default:
			body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
			break;
		}
	}
	// dhm

	body->r.svFlags = ent->r.svFlags;
	VectorCopy( ent->r.mins, body->r.mins );
	VectorCopy( ent->r.maxs, body->r.maxs );
	VectorCopy( ent->r.absmin, body->r.absmin );
	VectorCopy( ent->r.absmax, body->r.absmax );

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	// DHM - Nerve :: allow bullets to pass through bbox
	body->r.contents = 0;
	body->r.ownerNum = ent->r.ownerNum;

	body->nextthink = level.time + 5000;
	body->think = BodySink;

	body->die = body_die;

	// don't take more damage if already gibbed
	if ( ent->health <= GIB_HEALTH ) {
		body->takedamage = qfalse;
	} else {
		body->takedamage = qtrue;
	}


	VectorCopy( body->s.pos.trBase, body->r.currentOrigin );
	trap_LinkEntity( body );
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int i;

	// set the delta angle
	for ( i = 0 ; i < 3 ; i++ ) {
		int cmdAngle;

		cmdAngle = ANGLE2SHORT( angle[i] );
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy( ent->s.angles, ent->client->ps.viewangles );
}

/* JPW NERVE
================
limbo
================
*/
void limbo( gentity_t *ent, qboolean makeCorpse ) {
	int i,contents;
	//int startclient = ent->client->sess.spectatorClient;
	int startclient = ent->client->ps.clientNum;

	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		G_Printf( "FIXME: limbo called from single player game.  Shouldn't see this\n" );
		return;
	}
	if ( !( ent->client->ps.pm_flags & PMF_LIMBO ) ) {

		// DHM - Nerve :: First save off persistant info we'll need for respawn
		for ( i = 0; i < MAX_PERSISTANT; i++ )
			ent->client->saved_persistant[i] = ent->client->ps.persistant[i];
		// dhm

		ent->client->ps.pm_flags |= PMF_LIMBO;
		ent->client->ps.pm_flags |= PMF_FOLLOW;

		if ( makeCorpse ) {
			CopyToBodyQue( ent ); // make a nice looking corpse
		} else {
			trap_UnlinkEntity( ent );
		}

		// DHM - Nerve :: reset these values
		ent->client->ps.viewlocked = 0;
		ent->client->ps.viewlocked_entNum = 0;

		ent->r.maxs[2] = 0;
		ent->r.currentOrigin[2] += 8;
		contents = trap_PointContents( ent->r.currentOrigin, -1 ); // drop stuff
		ent->s.weapon = ent->client->limboDropWeapon; // stored in player_die()
		if ( makeCorpse && !( contents & CONTENTS_NODROP ) ) {
			TossClientItems( ent );
		}

		ent->client->sess.spectatorClient = startclient;
		Cmd_FollowCycle_f( ent,1 ); // get fresh spectatorClient

		if ( ent->client->sess.spectatorClient == startclient ) {
			// No one to follow, so just stay put
			ent->client->sess.spectatorState = SPECTATOR_FREE;
		} else {
			ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		}

//		ClientUserinfoChanged( ent->client - level.clients );		// NERVE - SMF - don't do this
		if ( ent->client->sess.sessionTeam == TEAM_RED ) {
			ent->client->deployQueueNumber = level.redNumWaiting;
			level.redNumWaiting++;
		} else if ( ent->client->sess.sessionTeam == TEAM_BLUE )     {
			ent->client->deployQueueNumber = level.blueNumWaiting;
			level.blueNumWaiting++;
		}
		// TODO Check this against OSPx
		for ( i = 0 ; i < level.maxclients ; i++ ) {
			if ( level.clients[i].ps.pm_flags & PMF_LIMBO
				 && level.clients[i].sess.spectatorClient == ent->s.number
				 &&  level.clients[i].sess.sessionTeam == ent->client->sess.sessionTeam) {
			     Cmd_FollowCycle_f( &g_entities[i], 1 );
			}
		}
	}
}

/* JPW NERVE
================
reinforce
================
// -- called when time expires for a team deployment cycle and there is at least one guy ready to go
*/
void reinforce( gentity_t *ent ) {
	int p, team; // numDeployable=0, finished=0; // TTimo unused
	char *classname;
	gclient_t *rclient;
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		G_Printf( "FIXME: reinforce called from single player game.  Shouldn't see this\n" );
		return;
	}
	if ( !( ent->client->ps.pm_flags & PMF_LIMBO ) ) {
		G_Printf( "player already deployed, skipping\n" );
		return;
	}
	// get team to deploy from passed entity

	team = ent->client->sess.sessionTeam;

	// find number active team spawnpoints
	if ( team == TEAM_RED ) {
		classname = "team_CTF_redspawn";
	} else if ( team == TEAM_BLUE ) {
		classname = "team_CTF_bluespawn";
	} else {
		assert( 0 );
	}

	// DHM - Nerve :: restore persistant data now that we're out of Limbo
	rclient = ent->client;

	for ( p = 0; p < MAX_PERSISTANT; p++ )
		rclient->ps.persistant[p] = rclient->saved_persistant[p];
	// dhm
    respawn( ent );
}
// jpw


/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {
	//gentity_t	*tent;

	// Ridah, if single player, reload the last saved game for this player
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {

		if ( reloading || saveGamePending ) {
			return;
		}

		if ( !( ent->r.svFlags & SVF_CASTAI ) ) {
			// Fast method, just do a map_restart, and then load in the savegame
			// once everything is settled.
			trap_SetConfigstring( CS_SCREENFADE, va( "1 %i 500", level.time + 250 ) );
			reloading = qtrue;
			level.reloadDelayTime = level.time + 1500;

			return;
		}
	}

	// done.

	ent->client->ps.pm_flags &= ~PMF_LIMBO; // JPW NERVE turns off limbo

	// DHM - Nerve :: Already handled in 'limbo()'
	if ( g_gametype.integer < GT_WOLF ) {
		CopyToBodyQue( ent );
	}

	if ((g_maxlives.integer > 0 || g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0) && g_gamestate.integer == GS_PLAYING)
	{
		if (ent->client->ps.persistant[PERS_RESPAWNS_LEFT] > 0)
		{
			ClientSpawn(ent, qfalse);
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT]--;
			G_DPrintf("Respawning %s, %i lives left\n", ent->client->pers.netname, ent->client->ps.persistant[PERS_RESPAWNS_LEFT]);
		}
		else
		{
			limbo(ent, qtrue);
		}
	}
	else
	{
		ClientSpawn(ent, qfalse);
	}


	// DHM - Nerve :: Add back if we decide to have a spawn effect
	// add a teleportation effect
	//tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
	//tent->s.clientNum = ent->s.clientNum;
}

// NERVE - SMF - merge from team arena
/*
================
TeamCount

Returns number of players on a team
================
*/
team_t TeamCount( int ignoreClientNum, int team ) {
	int i;
	int count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return count;
}
// -NERVE - SMF

/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
	int i;
	int counts[TEAM_NUM_TEAMS];

	memset( counts, 0, sizeof( counts ) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == TEAM_BLUE ) {
			counts[TEAM_BLUE]++;
		} else if ( level.clients[i].sess.sessionTeam == TEAM_RED )   {
			counts[TEAM_RED]++;
		}
	}

	if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
		return TEAM_RED;
	}
	if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
		return TEAM_BLUE;
	}
	// equal team count, so join the team with the lowest score
	if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ) {
		return TEAM_RED;
	}
	return TEAM_BLUE;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ( ( p = strchr( model, '/' ) ) != NULL ) {
		*p = 0;
	}

	Q_strcat( model, MAX_QPATH, "/" );
	Q_strcat( model, MAX_QPATH, skin );
}

// NERVE - SMF
/*
===========
SetWolfUserVars
===========
*/
void SetWolfUserVars( gentity_t *ent, char *wolfinfo ) {
	gclient_t *client;
	int mask, team;

	client = ent->client;
	if ( !client ) {
		return;
	}

	// check if we have a valid snapshot
	mask = MP_TEAM_MASK;
	team = ( client->pers.cmd.mpSetup & mask ) >> MP_TEAM_OFFSET;

	if ( !team ) {
		return;
	}

	// set player class
	mask = MP_CLASS_MASK;
	client->sess.latchPlayerType = ( client->pers.cmd.mpSetup & mask ) >> MP_CLASS_OFFSET;

	// set weapon
	mask = MP_WEAPON_MASK;
	client->sess.latchPlayerWeapon = ( client->pers.cmd.mpSetup & mask ) >> MP_WEAPON_OFFSET;
}

// -NERVE - SMF

// DHM - Nerve
/*
===========
SetWolfSkin

Forces a client's skin (for Wolfenstein teamplay)
===========
*/

#define MULTIPLAYER_ALLIEDMODEL "multi"
#define MULTIPLAYER_AXISMODEL   "multi_axis"

void SetWolfSkin( gclient_t *client, char *model ) {

	switch ( client->sess.sessionTeam ) {
	case TEAM_RED:
		Q_strcat( model, MAX_QPATH, "red" );
		break;
	case TEAM_BLUE:
		Q_strcat( model, MAX_QPATH, "blue" );
		break;
	default:
		Q_strcat( model, MAX_QPATH, "red" );
		break;
	}

	switch ( client->sess.playerType ) {
	case PC_SOLDIER:
		Q_strcat( model, MAX_QPATH, "soldier" );
		break;
	case PC_MEDIC:
		Q_strcat( model, MAX_QPATH, "medic" );
		break;
	case PC_ENGINEER:
		Q_strcat( model, MAX_QPATH, "engineer" );
		break;
	case PC_LT:
		Q_strcat( model, MAX_QPATH, "lieutenant" );
		break;
	default:
		Q_strcat( model, MAX_QPATH, "soldier" );
		break;
	}

	// DHM - A skinnum will be in the session data soon...
	switch ( client->sess.playerSkin ) {
	case 1:
		Q_strcat( model, MAX_QPATH, "1" );
		break;
	case 2:
		Q_strcat( model, MAX_QPATH, "2" );
		break;
	case 3:
		Q_strcat( model, MAX_QPATH, "3" );
		break;
	default:
		Q_strcat( model, MAX_QPATH, "1" );
		break;
	}
}

/*
===========
RTCWPro
Checks and potentially sets STAT_MAX_HEALTH for both teams
Source: ET
===========
*/
int G_CountTeamMedics(team_t team, qboolean alivecheck)
{
	int numMedics = 0;
	int i, j;

	for (i = 0; i < level.numConnectedClients; i++)
	{
		j = level.sortedClients[i];

		if (level.clients[j].sess.sessionTeam != team)
		{
			continue;
		}

		if (level.clients[j].sess.playerType != PC_MEDIC)
		{
			continue;
		}

		if (alivecheck)
		{
			if (g_entities[j].health <= 0)
			{
				continue;
			}

			if (level.clients[j].ps.pm_type == PM_DEAD || (level.clients[j].ps.pm_flags & PMF_LIMBO))
			{
				continue;
			}
		}

		numMedics++;
	}

	return numMedics;
}

/*
===========
RTCWPro
Sets health based on number of medics
Source: ET Legacy
===========
*/
void AddMedicTeamBonus(gclient_t* client)
{
	//if (!(client->sess.sessionTeam == TEAM_RED || client->sess.sessionTeam == TEAM_BLUE))
	//	return;

	//gclient_t* cl;
	//int i, startHealth;

	int numMedics = G_CountTeamMedics(client->sess.sessionTeam, qfalse);

	// compute health mod
	client->pers.maxHealth = 100 + 10 * numMedics;

	if (client->pers.maxHealth > 125)
	{
		client->pers.maxHealth = 125;
	}

	if (client->sess.playerType == PC_MEDIC)
	{
		client->pers.maxHealth *= 1.12;

		if (client->pers.maxHealth > 140)
			client->pers.maxHealth = 140;
	}

	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
}

void SetWolfSpawnWeapons( gentity_t *ent ) {

	gclient_t* client = ent->client;

	int pc = client->sess.playerType;
	int starthealth = 100, numMedics = 0;   // JPW NERVE
	// L0 - ammoClips and NadeValues
	//
	// Patched this whole function but not commented it much so be aware..
	//
	// ammo
	int		soldClips = g_soldierClips.integer;
	int		ltClips = g_leutClips.integer;
	int		engClips = g_engineerClips.integer;
	int		medClips = g_medicClips.integer;
	int		gunClips = g_pistolClips.integer;
	// nades
	int		engNades = g_engNades.integer;
	int		soldNades = g_soldNades.integer;
	int		medNades = g_medicNades.integer;
	int		ltNades = g_ltNades.integer;

	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

	// Reset special weapon time
	client->ps.classWeaponTime = -999999;

// Xian -- Commented out and moved to ClientSpawn for clarity
//	client->ps.powerups[PW_INVULNERABLE] = level.time + 3000; // JPW NERVE some time to find cover

	// RTCWPro - update ready status
	if (g_gamestate.integer == GS_WARMUP || g_gamestate.integer == GS_WAITING_FOR_PLAYERS)
		client->ps.powerups[PW_READY] = (player_ready_status[client->ps.clientNum].isReady == 1) ? INT_MAX : 0;

	// Communicate it to cgame
	client->ps.stats[STAT_PLAYER_CLASS] = pc;

	// Abuse teamNum to store player class as well (can't see stats for all clients in cgame)
	client->ps.teamNum = pc;

	// JPW NERVE -- zero out all ammo counts
	memset( client->ps.ammo,MAX_WEAPONS,sizeof( int ) );

	// All players start with a knife (not OR-ing so that it clears previous weapons)
	client->ps.weapons[0] = 0;
	client->ps.weapons[1] = 0;
	COM_BitSet( client->ps.weapons, WP_KNIFE );

	client->ps.ammo[BG_FindAmmoForWeapon( WP_KNIFE )] = 1;
	client->ps.weapon = WP_KNIFE;
	client->ps.weaponstate = WEAPON_READY;

	// Engineer gets dynamite
	if ( pc == PC_ENGINEER ) {
		COM_BitSet( client->ps.weapons, WP_DYNAMITE );
		client->ps.ammo[BG_FindAmmoForWeapon( WP_DYNAMITE )] = 0;
		client->ps.ammoclip[BG_FindClipForWeapon( WP_DYNAMITE )] = 1;

		// NERVE - SMF
		COM_BitSet( client->ps.weapons, WP_PLIERS );
		client->ps.ammoclip[BG_FindClipForWeapon( WP_PLIERS )] = 1;
		client->ps.ammo[WP_PLIERS] = 1;
	}

	if ( g_knifeonly.integer != 1 ) {

		// Lieutenant gets binoculars, ammo pack, artillery, and a grenade
		if ( pc == PC_LT ) {
			client->ps.stats[STAT_KEYS] |= ( 1 << INV_BINOCS );
			COM_BitSet( client->ps.weapons, WP_AMMO );
			client->ps.ammo[BG_FindAmmoForWeapon( WP_AMMO )] = 0;
			client->ps.ammoclip[BG_FindClipForWeapon( WP_AMMO )] = 1;
			COM_BitSet( client->ps.weapons, WP_ARTY );
			client->ps.ammo[BG_FindAmmoForWeapon( WP_ARTY )] = 0;
			client->ps.ammoclip[BG_FindClipForWeapon( WP_ARTY )] = 1;

			// NERVE - SMF
			COM_BitSet( client->ps.weapons, WP_SMOKE_GRENADE );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_SMOKE_GRENADE )] = 1;
			client->ps.ammo[WP_SMOKE_GRENADE] = 1;

			switch ( client->sess.sessionTeam ) {
			case TEAM_BLUE:
				COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = ltNades;
				break;
			case TEAM_RED:
				COM_BitSet( client->ps.weapons, WP_GRENADE_LAUNCHER );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_LAUNCHER )] = ltNades;
				break;
			default:
				COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = ltNades;
				break;
			}
		}

		// Everyone gets a pistol
		switch ( client->sess.sessionTeam ) { // JPW NERVE was playerPistol

			case TEAM_RED: // JPW NERVE
				COM_BitSet( client->ps.weapons, WP_LUGER );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_LUGER )] += 8;
				client->ps.ammo[BG_FindAmmoForWeapon( WP_LUGER )] += gunClips * 8;
				client->ps.weapon = WP_LUGER;
				break;
			default: // '0' // TEAM_BLUE
				COM_BitSet( client->ps.weapons, WP_COLT );
				client->ps.ammoclip[BG_FindClipForWeapon( WP_COLT )] += 8;
				client->ps.ammo[BG_FindAmmoForWeapon( WP_COLT )] += gunClips * 8;
				client->ps.weapon = WP_COLT;
				break;
		}

		switch ( client->sess.sessionTeam ) { // was playerItem
			int nades;

			case TEAM_BLUE:
				COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
				client->ps.ammo[BG_FindAmmoForWeapon( WP_GRENADE_PINEAPPLE )] = 0;
				if ( pc == PC_LT ) nades = ltNades;
				else if ( pc == PC_ENGINEER ) nades = engNades;
				else if ( pc == PC_MEDIC ) nades = medNades;
				else if ( pc == PC_SOLDIER ) nades = soldNades;
				else nades = 1;
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = nades;
				break;
			case TEAM_RED:
				COM_BitSet( client->ps.weapons, WP_GRENADE_LAUNCHER );
				client->ps.ammo[BG_FindAmmoForWeapon( WP_GRENADE_LAUNCHER )] = 0;
				if ( pc == PC_LT ) nades = ltNades;
				else if ( pc == PC_ENGINEER ) nades = engNades;
				else if ( pc == PC_MEDIC ) nades = medNades;
				else if ( pc == PC_SOLDIER ) nades = soldNades;
				else nades = 1;
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_LAUNCHER )] = nades;
				break;
			default:
				COM_BitSet( client->ps.weapons, WP_GRENADE_PINEAPPLE );
				client->ps.ammo[BG_FindAmmoForWeapon( WP_GRENADE_PINEAPPLE )] = 0;
				if ( pc == PC_LT ) nades = ltNades;
				else if ( pc == PC_ENGINEER ) nades = engNades;
				else if ( pc == PC_MEDIC ) nades = medNades;
				else if ( pc == PC_SOLDIER ) nades = soldNades;
				else nades = 1;
				client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )] = nades;
				break;
		}


		// JPW NERVE
		if ( pc == PC_MEDIC ) {
			COM_BitSet( client->ps.weapons, WP_MEDIC_SYRINGE );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MEDIC_SYRINGE )] = 10;

			// NERVE - SMF
			COM_BitSet( client->ps.weapons, WP_MEDKIT );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MEDKIT )] = 1;
			client->ps.ammo[WP_MEDKIT] = 1;
		}
		// jpw

		// Soldiers and Lieutenants get a 2-handed weapon
		if ( pc == PC_SOLDIER || pc == PC_LT ) {

			// JPW NERVE -- if LT is selected but illegal weapon, set to team-specific SMG
			if ( ( pc == PC_LT ) && ( client->sess.playerWeapon > 5 ) ) {
				if ( client->sess.sessionTeam == TEAM_RED ) {
					client->sess.playerWeapon = 3;
				} else {
					client->sess.playerWeapon = 4;
				}
			}
			// jpw
			switch ( client->sess.playerWeapon ) {

				case 3:     // WP_MP40
					COM_BitSet( client->ps.weapons, WP_MP40 );
					client->ps.ammoclip[BG_FindClipForWeapon( WP_MP40 )] += 32;
					if ( pc == PC_SOLDIER ) {
						client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += (32 * soldClips);
					} else {
						client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += (32 * ltClips);
					}
					client->ps.weapon = WP_MP40;
					break;

				case 4:     // WP_THOMPSON
					COM_BitSet( client->ps.weapons, WP_THOMPSON );
					client->ps.ammoclip[BG_FindClipForWeapon( WP_THOMPSON )] += 30;
					if ( pc == PC_SOLDIER ) {
						client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += (30 * soldClips);
					} else {
						client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += (30 * ltClips);
					}
					client->ps.weapon = WP_THOMPSON;
					break;

				case 5:     // WP_STEN
					COM_BitSet( client->ps.weapons, WP_STEN );
					client->ps.ammoclip[BG_FindClipForWeapon( WP_STEN )] += 32;
					if ( pc == PC_SOLDIER ) {
						client->ps.ammo[BG_FindAmmoForWeapon( WP_STEN )] += (32 * soldClips);
					} else {
						client->ps.ammo[BG_FindAmmoForWeapon( WP_STEN )] += (32 * ltClips);
					}
					client->ps.weapon = WP_STEN;
					break;


				// NOTES - when porting ET pub IsWeaponDisabled they were doing in ClientSpawn and checking both sess.latchPlayerWeapon and sess.playerWeapon values
				// we are already past that point and sess.playerWeapon has been set to sess.latchPlayerWeapon
				// so we're going to call the method with sess.playerWeapon

				case 6:     // WP_MAUSER, WP_SNIPERRIFLE
					if ( pc != PC_SOLDIER ) {
						return;
					}

					if (g_maxTeamSniper.integer != -1 ) {
						if (IsWeaponDisabled(ent, client->sess.playerWeapon, WP_MAUSER, client->sess.sessionTeam, qtrue)) {
							trap_SendServerCommand( client->ps.clientNum, va("cp \"^3*** Sniper limit(^1%d^3) has been reached. Select a different weapon.\n\"2", g_maxTeamSniper.integer));
							SetDefaultWeapon(client, qtrue);
							break;
						}
					}

					COM_BitSet( client->ps.weapons, WP_SNIPERRIFLE );
					client->ps.ammoclip[BG_FindClipForWeapon( WP_SNIPERRIFLE )] = 10;
					client->ps.ammo[BG_FindAmmoForWeapon( WP_SNIPERRIFLE )] = 10;
					client->ps.weapon = WP_SNIPERRIFLE;

					COM_BitSet( client->ps.weapons, WP_MAUSER );
					client->ps.ammoclip[BG_FindClipForWeapon( WP_MAUSER )] = 10;
					client->ps.ammo[BG_FindAmmoForWeapon( WP_MAUSER )] = 10;
					client->ps.weapon = WP_MAUSER;
					break;

				case 8:     // WP_PANZERFAUST
					if ( pc != PC_SOLDIER ) {
						return;
					}

					if ( g_maxTeamPF.integer != -1 ) {
						if (IsWeaponDisabled(ent, client->sess.playerWeapon, WP_PANZERFAUST, client->sess.sessionTeam, qtrue)) {
							trap_SendServerCommand( client->ps.clientNum, va("cp \"^3*** Panzer limit(^1%d^3) has been reached. Select a different weapon.\n\"2", g_maxTeamPF.integer));
							SetDefaultWeapon(client, qtrue);
							break;
						}
					}

					COM_BitSet( client->ps.weapons, WP_PANZERFAUST );
					client->ps.ammo[BG_FindAmmoForWeapon( WP_PANZERFAUST )] = 4;
					client->ps.weapon = WP_PANZERFAUST;
					break;

				case 9:     // WP_VENOM
					if ( pc != PC_SOLDIER ) {
						return;
					}

					if ( g_maxTeamVenom.integer != -1 ) {
						if (IsWeaponDisabled(ent, client->sess.playerWeapon, WP_VENOM, client->sess.sessionTeam, qtrue)) {
							trap_SendServerCommand( client->ps.clientNum, va("cp \"^3*** Venom limit(^1%d^3) has been reached. Select a different weapon.\n\"2", g_maxTeamVenom.integer));
							SetDefaultWeapon(client, qtrue);
							break;
						}
					}

					COM_BitSet( client->ps.weapons, WP_VENOM );
					client->ps.ammoclip[BG_FindAmmoForWeapon( WP_VENOM )] = 500;
					client->ps.weapon = WP_VENOM;
					break;

				case 10:    // WP_FLAMETHROWER
					if ( pc != PC_SOLDIER ) {
						return;
					}

					if ( g_maxTeamFlamer.integer != -1 ) {
						if (IsWeaponDisabled(ent, client->sess.playerWeapon, WP_FLAMETHROWER, client->sess.sessionTeam, qtrue)) {
							trap_SendServerCommand( client->ps.clientNum, va("cp \"^3*** Flamer limit(^1%d^3) has been reached. Select a different weapon.\n\"2", g_maxTeamFlamer.integer));
							SetDefaultWeapon(client, qtrue);
							break;
						}

						if (client->pers.restrictedWeapon != WP_FLAMETHROWER) {
							(client->sess.sessionTeam == TEAM_RED) ? level.axisFlamer++ : level.alliedFlamer++;
							client->pers.restrictedWeapon = WP_FLAMETHROWER;
						}
					}

					COM_BitSet( client->ps.weapons, WP_FLAMETHROWER );
					client->ps.ammoclip[BG_FindAmmoForWeapon( WP_FLAMETHROWER )] = 200;
					client->ps.weapon = WP_FLAMETHROWER;
					break;

				default:    // give MP40 if given invalid weapon number
					if ( client->sess.sessionTeam == TEAM_RED ) { // JPW NERVE
						COM_BitSet( client->ps.weapons, WP_MP40 );
						client->ps.ammoclip[BG_FindClipForWeapon( WP_MP40 )] += 32;
						if ( pc == PC_SOLDIER ) {
							client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += (32 * soldClips);
						} else {
							client->ps.ammo[BG_FindAmmoForWeapon( WP_MP40 )] += (32 * ltClips);
						}
						client->ps.weapon = WP_MP40;
					} else { // TEAM_BLUE
						COM_BitSet( client->ps.weapons, WP_THOMPSON );
						client->ps.ammoclip[BG_FindClipForWeapon( WP_THOMPSON )] += 30;
						if ( pc == PC_SOLDIER ) {
							client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += (30 * soldClips);
						} else {
							client->ps.ammo[BG_FindAmmoForWeapon( WP_THOMPSON )] += (30 * ltClips);
						}
						client->ps.weapon = WP_THOMPSON;
					}
					break;
			}
		} else { // medic or engineer gets assigned MP40 or Thompson with one magazine ammo
			// L0 - Removed and handled in g_players.c now...due custom MG spawning..
			SetDefaultWeapon(client, qfalse);
			// End
		}
	} else // Knifeonly block
	{
		if ( pc == PC_MEDIC ) {
			COM_BitSet( client->ps.weapons, WP_MEDIC_SYRINGE );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MEDIC_SYRINGE )] = 20;

			// NERVE - SMF
			COM_BitSet( client->ps.weapons, WP_MEDKIT );
			client->ps.ammoclip[BG_FindClipForWeapon( WP_MEDKIT )] = 1;
			client->ps.ammo[WP_MEDKIT] = 1;
		}
	} // End Knifeonly stuff -- Ensure that medics get their basic stuff
}
// dhm - end


/*
===========
ClientCheckName
============
*/
static void ClientCleanName( const char *in, char *out, int outSize ) {
	int len, colorlessLen;
	char ch;
	char    *p;
	int spaces;

	//save room for trailing null byte
	outSize--;

	len = 0;
	colorlessLen = 0;
	p = out;
	*p = 0;
	spaces = 0;

	while ( 1 ) {
		ch = *in++;
		if ( !ch ) {
			break;
		}

		// don't allow leading spaces
		if ( !*p && ch == ' ' ) {
			continue;
		}

		// check colors
		if ( ch == Q_COLOR_ESCAPE ) {
			// solo trailing carat is not a color prefix
			if ( !*in ) {
				break;
			}

		/*	// don't allow black in a name, period
			if ( ColorIndex( *in ) == 0 ) {
				in++;
				continue;
			}
			*/

			// make sure room in dest for both chars
			if ( len > outSize - 2 ) {
				break;
			}

			*out++ = ch;
			*out++ = *in++;
			len += 2;
			continue;
		}

		// don't allow too many consecutive spaces
		if ( ch == ' ' ) {
			spaces++;
			if ( spaces > 3 ) {
				continue;
			}
		} else {
			spaces = 0;
		}

		if ( len > outSize - 1 ) {
			break;
		}

		*out++ = ch;
		colorlessLen++;
		len++;
	}
	*out = 0;

	// don't allow empty names
	if ( *p == 0 || colorlessLen == 0 ) {
		Q_strncpyz( p, "UnnamedPlayer", outSize );
	}
}

/*
==================
G_CheckForExistingModelInfo

  If this player model has already been parsed, then use the existing information.
  Otherwise, set the modelInfo pointer to the first free slot.

  returns qtrue if existing model found, qfalse otherwise
==================
*/
qboolean G_CheckForExistingModelInfo( gclient_t *cl, char *modelName, animModelInfo_t **modelInfo ) {
	int i;
	animModelInfo_t *trav, *firstFree = NULL;
	gclient_t *cl_trav;
	char modelsUsed[MAX_ANIMSCRIPT_MODELS];

	for ( i = 0, trav = level.animScriptData.modelInfo; i < MAX_ANIMSCRIPT_MODELS; i++, trav++ ) {
		if ( trav->modelname[0] ) {
			if ( !Q_stricmp( trav->modelname, modelName ) ) {
				// found a match, use this modelinfo
				*modelInfo = trav;
				level.animScriptData.clientModels[cl->ps.clientNum] = i + 1;
				return qtrue;
			}
		} else if ( !firstFree ) {
			firstFree = trav;
			level.animScriptData.clientModels[cl->ps.clientNum] = i + 1;
		}
	}

	// set the modelInfo to the first free slot
	if ( !firstFree ) {
		// attempt to free a model that is no longer being used
		memset( modelsUsed, 0, sizeof( modelsUsed ) );
		for ( i = 0, cl_trav = level.clients; i < MAX_CLIENTS; i++, cl_trav++ ) {
			if ( cl_trav != cl && g_entities[cl_trav->ps.clientNum].inuse && cl_trav->modelInfo ) {
				modelsUsed[ (int)( cl_trav->modelInfo - level.animScriptData.modelInfo ) ] = 1;
			}
		}
		// now use the first slot that isn't being utilized
		for ( i = 0, trav = level.animScriptData.modelInfo; i < MAX_ANIMSCRIPT_MODELS; i++, trav++ ) {
			if ( !modelsUsed[i] ) {
				firstFree = trav;
				level.animScriptData.clientModels[cl->ps.clientNum] = i + 1;
				break;
			}
		}
	}

	if ( !firstFree ) {
		G_Error( "unable to find a free modelinfo slot, cannot continue\n" );
	} else {
		*modelInfo = firstFree;
		// clear the structure out ready for use
		memset( *modelInfo, 0, sizeof( *modelInfo ) );
	}
	// qfalse signifies that we need to parse the information from the script files
	return qfalse;
}

/*
=============
G_ParseAnimationFiles
=============
*/
static char text[100000];                   // <- was causing callstacks >64k

qboolean G_ParseAnimationFiles( char *modelname, gclient_t *cl ) {
	char filename[MAX_QPATH];
	fileHandle_t f;
	int len;

	// set the name of the model in the modelinfo structure
	Q_strncpyz( cl->modelInfo->modelname, modelname, sizeof( cl->modelInfo->modelname ) );

	// load the cfg file
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/wolfanim.cfg", modelname );
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		G_Printf( "G_ParseAnimationFiles(): file '%s' not found\n", filename );       //----(SA)	added
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) {
		G_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	BG_AnimParseAnimConfig( cl->modelInfo, filename, text );

	// load the script file
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/wolfanim.script", modelname );
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		if ( cl->modelInfo->version > 1 ) {
			return qfalse;
		}
		// try loading the default script for old legacy models
		Com_sprintf( filename, sizeof( filename ), "models/players/default.script", modelname );
		len = trap_FS_FOpenFile( filename, &f, FS_READ );
		if ( len <= 0 ) {
			return qfalse;
		}
	}
	if ( len >= sizeof( text ) - 1 ) {
		G_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	BG_AnimParseAnimScript( cl->modelInfo, &level.animScriptData, cl->ps.clientNum, filename, text );

	// ask the client to send us the movespeeds if available
	if ( g_gametype.integer == GT_SINGLE_PLAYER && g_entities[0].client && g_entities[0].client->pers.connected == CON_CONNECTED ) {
		trap_SendServerCommand( 0, va( "mvspd %s", modelname ) );
	}

	return qtrue;
}

/*
===========
OSPx - Store Client's IP
============
*/
void SaveIP_f(gclient_t * client, char * sip) {
	Q_strncpyz(client->sess.ip, sip, sizeof(client->sess.ip));
	return;
}

/*
===========
OSPx - To save some time..
============
*/
char *SanitizeClientIP(char *ip, qboolean printFull) {

	if (!printFull) {
		char* token;

		if (strlen(ip) > 15) {
			token = strtok(ip, "::");
			return va("%s.*.*.*", ip);
		}
		token = strtok(ip, ".");
		return va("%s.*.*.*", token);
	}
	return va("%s", ip);
}

/*
===========
L0 - Check spoofing..

Used ETpub for reference
============
*/
char *spoofcheck( gclient_t *client, char *guid, char *ip ){
	char *cIP;

	if(Q_stricmp(client->sess.guid, guid)) {
		if( !client->sess.guid ||
			!Q_stricmp( client->sess.guid, "" ) ||
			!Q_stricmp( client->sess.guid, "NOGUID" ) ) {

			if( Q_stricmp( guid, "unknown" ) && Q_stricmp( guid, "NO_GUID" ) ) {
				Q_strncpyz( client->sess.guid, guid, sizeof( client->sess.guid ) );
			}
		} else {
			G_LogPrintf( "GUID SPOOF: client %i Original guid %s"
				"Secondary guid %s\n",
				client->ps.clientNum,
				client->sess.guid,
				guid);

			// We use more permanent (no options to disable it) version
			return "You are kicked for GUID spoofing";
		}
	}

	cIP = va("%s", client->sess.ip );
	if(Q_stricmp(cIP, ip) != 0) {
		G_LogPrintf(
			"IP SPOOF: client %i Original ip %s \n"
			"Secondary ip %s\n",
			client->ps.clientNum,
			cIP,
			ip
		);

		return "You are kicked for IP spoofing";
	}

	return 0;
}

/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged(int clientNum) {
	gentity_t* ent;
	char* s;
	char model[MAX_QPATH], modelname[MAX_QPATH];

	//----(SA) added this for head separation
	char head[MAX_QPATH];

	char oldname[MAX_STRING_CHARS];
	gclient_t* client;
	char* c1;
	char userinfo[MAX_INFO_STRING];

	ent = g_entities + clientNum;
	client = ent->client;

	client->ps.clientNum = clientNum;

	trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo)) {
		strcpy(userinfo, "\\name\\badinfo");
	}

	// check for local client
	s = Info_ValueForKey(userinfo, "ip");
	if (s && !strcmp(s, "localhost")) {
		client->pers.localClient = qtrue;
		//	client->sess.referee = RL_REFEREE;
	}
	// L0
		// Save IP for getstatus..
	s = Info_ValueForKey(userinfo, "ip");
	if (s[0] != 0) {
		SaveIP_f(client, s);
	} // OSPx - Country Flags
	else if (!(ent->r.svFlags & SVF_BOT) && !strlen(s)) {
		// To solve the IP bug..
		s = va("%s", client->sess.ip);
	}

	s = Info_ValueForKey(userinfo, "cg_uinfo");
	//sscanf(s, "%i %i %i", &client->pers.clientFlags, &client->pers.clientTimeNudge, &client->pers.clientMaxPackets);
	//sscanf(s, "%i %i %i %s", &client->pers.clientFlags, &client->pers.clientTimeNudge, &client->pers.clientMaxPackets, client->sess.guid);
	sscanf(s, "%i %i %i %i %i %i %s %i", &client->pers.clientFlags, &client->pers.clientTimeNudge, &client->pers.clientMaxPackets,
		&client->pers.hitSoundType, &client->pers.hitSoundBodyStyle, &client->pers.hitSoundHeadStyle, client->sess.guid, &client->pers.antilag);

	// Check for "" GUID..
	if (!Q_stricmp(client->sess.guid, "D41D8CD98F00B204E9800998ECF8427E") ||
		!Q_stricmp(client->sess.guid, "d41d8cd98f00b204e9800998ecf8427e")) {
		trap_DropClient(clientNum, "(Known bug) Corrupted GUID^3! ^7Restart your game..");
	}

	//// Check for Shared GUIDs and drop client - this is messing up stats
	if (!Q_stricmp(client->sess.guid, "8E6A51BAF1C7E338A118D9E32472954E") ||
		!Q_stricmp(client->sess.guid, "8e6a51baf1c7e338a118d9e32472954e") ||
		!Q_stricmp(client->sess.guid, "58E419DE5A8B2655F6D48EAB68275DB5") ||
		!Q_stricmp(client->sess.guid, "58e419de5a8b2655f6d48eab68275db5") ||
		!Q_stricmp(client->sess.guid, "FBE2ED832F8415EFBAAA5DF10074484A") ||
		!Q_stricmp(client->sess.guid, "fbe2ed832f8415efbaaa5df10074484a")) {
		trap_DropClient(clientNum, "^3Shared GUID Violation. ^7Delete your RTCWKEY in Main and restart your game.");
	}

	if (!Q_stricmp(client->sess.guid,NO_GUID)) {
        trap_DropClient(clientNum, "Empty or invalid rtcwkey");
	}

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !atoi( s ) ) {
		client->pers.predictItemPickup = qfalse;
	} else {
		client->pers.predictItemPickup = qtrue;
	}

//unlagged - client options
	// see if the player has opted out
	s = Info_ValueForKey( userinfo, "cg_delag" );
	if ( !atoi( s ) ) {
		client->pers.delag = 0;
	} else {
		client->pers.delag = atoi( s );
	}

	// see if the player is nudging his shots
	s = Info_ValueForKey( userinfo, "cg_cmdTimeNudge" );
	client->pers.cmdTimeNudge = atoi( s );

	// see if the player wants to debug the backward reconciliation
	s = Info_ValueForKey( userinfo, "cg_debugDelag" );
	if ( !atoi( s ) ) {
		client->pers.debugDelag = qfalse;
	}
	else {
		client->pers.debugDelag = qtrue;
	}

	// see if the player is simulating incoming latency
	s = Info_ValueForKey( userinfo, "cg_latentSnaps" );
	client->pers.latentSnaps = atoi( s );

	// see if the player is simulating outgoing latency
	s = Info_ValueForKey( userinfo, "cg_latentCmds" );
	client->pers.latentCmds = atoi( s );

	// see if the player is simulating outgoing packet loss
	s = Info_ValueForKey( userinfo, "cg_plOut" );
	client->pers.plOut = atoi( s );
//unlagged - client options

	// check the auto activation
	s = Info_ValueForKey( userinfo, "cg_autoactivate" );
	if ( !atoi( s ) ) {
		client->pers.autoActivate = PICKUP_ACTIVATE;
	} else {
		client->pers.autoActivate = PICKUP_TOUCH;
	}

	// check the auto reload setting
	s = Info_ValueForKey( userinfo, "cg_autoReload" );
	if ( atoi( s ) ) {
		client->pers.bAutoReloadAux = qtrue;
		client->pmext.bAutoReload = qtrue;
	} else {
		client->pers.bAutoReloadAux = qfalse;
		client->pmext.bAutoReload = qfalse;
	}

	s = Info_ValueForKey(userinfo, "cg_findMedic");
	if (!atoi(s)) {
		client->pers.findMedic = qfalse;
	}
	else {
		client->pers.findMedic = qtrue;
	}

	// set name
	Q_strncpyz( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey( userinfo, "name" );
	ClientCleanName( s, client->pers.netname, sizeof( client->pers.netname ) );

	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			Q_strncpyz( client->pers.netname, "scoreboard", sizeof( client->pers.netname ) );
		}
	}

	if ( client->pers.connected == CON_CONNECTED ) {
		if ( strcmp( oldname, client->pers.netname ) ) {
			// L0
			// Do not allow renaming in intermissions.
			// Name animations for one;
			//	Generally suck,
			// & two;
			//	Push score table up which is annoying.
			// Name change could simply be ignored but then in certain scenarios,
			// it may be difficult for Admins to pinpoint a problematic player.
			if (level.intermissiontime) {
				Q_strncpyz(client->pers.netname, oldname, sizeof(client->pers.netname));
				Info_SetValueForKey(userinfo, "name", oldname);
				trap_SetUserinfo(clientNum, userinfo);
				// It will only push score table up for them so they get taste of their own medicine..
				CPx(client->ps.clientNum, "print \"^1Denied! ^7You cannot rename during intermission^1!\n\"");
				return;
			}
			else {
				AP(va("print \"[lof]%s" S_COLOR_WHITE " [lon]renamed to[lof] %s\n\"", oldname, client->pers.netname));
			}

            if (g_gameStatslog.integer && (g_gamestate.integer == GS_PLAYING)) {
                G_writeGeneralEvent (ent,ent, " ", eventNameChange);
            }

		}
	}

	// RTCWPro
	// don't use handicap here
	//client->pers.maxHealth = 100; atoi(Info_ValueForKey(userinfo, "handicap"));
	/*if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;*/

	//if (g_debugMode.integer)
	//{
	//	AP(va("print \"ClientUserinfoChanged:%i class: %i\n\"", client->ps.clientNum, client->ps.teamNum));
	//}

	AddMedicTeamBonus(client);
	// RTCWPro

	// set model
	if ( g_forceModel.integer ) {
		Q_strncpyz( model, DEFAULT_MODEL, sizeof( model ) );
		Q_strcat( model, sizeof( model ), "/default" );
	} else {
		Q_strncpyz( model, Info_ValueForKey( userinfo, "model" ), sizeof( model ) );
	}

	// RTCWPro: revive anim bug fix, credits: Nobo
	// RF, reset anims so client's dont freak out
	//client->ps.legsAnim = 0;
	//client->ps.torsoAnim = 0;

	// DHM - Nerve :: Forcibly set both model and skin for multiplayer.
	if ( g_gametype.integer >= GT_WOLF ) {

		// To communicate it to cgame
		client->ps.stats[ STAT_PLAYER_CLASS ] = client->sess.playerType;

		if ( client->sess.sessionTeam == TEAM_BLUE ) {
			Q_strncpyz( model, MULTIPLAYER_ALLIEDMODEL, MAX_QPATH );
		} else {
			Q_strncpyz( model, MULTIPLAYER_AXISMODEL, MAX_QPATH );
		}

		Q_strcat( model, MAX_QPATH, "/" );

		SetWolfSkin( client, model );

		Q_strncpyz( head, "", MAX_QPATH );
		SetWolfSkin( client, head );
	}

	// strip the skin name
	Q_strncpyz( modelname, model, sizeof( modelname ) );
	if ( strstr( modelname, "/" ) ) {
		modelname[ strstr( modelname, "/" ) - modelname ] = 0;
	} else if ( strstr( modelname, "\\" ) ) {
		modelname[ strstr( modelname, "\\" ) - modelname ] = 0;
	}

	if ( !G_CheckForExistingModelInfo( client, modelname, &client->modelInfo ) ) {
		if ( !G_ParseAnimationFiles( modelname, client ) ) {
			G_Error( "Failed to load animation scripts for model %s\n", modelname );
		}
	}

	// team`
	// DHM - Nerve :: Already took care of models and skins above
	if ( g_gametype.integer < GT_WOLF ) {

		//----(SA) added this for head separation
		// set head
		if ( g_forceModel.integer ) {
			Q_strncpyz( head, DEFAULT_HEAD, sizeof( head ) );
		} else {
			Q_strncpyz( head, Info_ValueForKey( userinfo, "head" ), sizeof( head ) );
		}

		//----(SA) end

		switch ( client->sess.sessionTeam ) {
		case TEAM_RED:
			ForceClientSkin( client, model, "red" );
			break;
		case TEAM_BLUE:
			ForceClientSkin( client, model, "blue" );
			break;
		default: // TEAM_FREE, TEAM_SPECTATOR, TEAM_NUM_TEAMS not handled in switch
			break;
		}
		if ( g_gametype.integer >= GT_TEAM && client->sess.sessionTeam == TEAM_SPECTATOR ) {
			// don't ever use a default skin in teamplay, it would just waste memory
			ForceClientSkin( client, model, "red" );
		}

	}
	//dhm - end

	// L0 - Set guid
//	if (strcmp( ent->client->sess.guid, "0" ) == 0 || strcmp(ent->client->sess.guid, "") == 0)
//		setGuid(Info_ValueForKey( userinfo, "cl_guid" ), ent->client->sess.guid);



	// colors
	c1 = Info_ValueForKey( userinfo, "color" );

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds

//----(SA) modified these for head separation

	if ( ent->r.svFlags & SVF_BOT ) {

		s = va("n\\%s\\t\\%i\\model\\%s\\head\\%s\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\cc\\255\\mu\\%i",
	//	s = va( "n\\%s\\t\\%i\\model\\%s\\head\\%s\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s",
				client->pers.netname, client->sess.sessionTeam, model, head, c1,
				client->pers.maxHealth, client->sess.wins, client->sess.losses,
				Info_ValueForKey( userinfo, "skill" ),
				client->sess.uci, (client->sess.muted ? 1 : 0));
	} else {
	//	s = va( "n\\%s\\t\\%i\\model\\%s\\head\\%s\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i",
			s = va("n\\%s\\t\\%i\\model\\%s\\head\\%s\\c1\\%s\\w\\%i\\l\\%i\\cc\\%i\\mu\\%i\\ref\\%i\\scs\\%i",
				client->pers.netname, client->sess.sessionTeam, model, head, c1, client->sess.wins, client->sess.losses,
				client->sess.uci, (client->sess.muted ? 1 : 0), client->sess.referee, client->sess.shoutcaster);
	}

//----(SA) end

	trap_SetConfigstring( CS_PLAYERS + clientNum, s );

	// OSPx - We need to send client private info (ip..) only to log and not a configstring,
	// as \configstrings reveals all user data in it which is something we don't want..
	if (!(ent->r.svFlags & SVF_BOT)) {
		char *team;

		team = (client->sess.sessionTeam == TEAM_RED) ? "Axis" :
			((client->sess.sessionTeam == TEAM_BLUE) ? "Allied" : "Spectator");

		// Print essentials and skip the garbage
		s = va("name\\%s\\team\\%s\\IP\\%s\\cc\\%i\\muted\\%s\\status\\%i\\scs\\%i\\timenudge\\%i\\maxpackets\\%i\\guid\\%s",
			client->pers.netname, team, client->sess.ip, client->sess.uci, (client->sess.muted ? "yes" : "no"), client->sess.referee,
			client->sess.shoutcaster, client->pers.clientTimeNudge, client->pers.clientMaxPackets, client->sess.guid);
	}
	// Account for bots..
	else {
		char *team;

		team = (client->sess.sessionTeam == TEAM_RED) ? "Axis" :
			((client->sess.sessionTeam == TEAM_BLUE) ? "Allied" : "Spectator");

		s = va("Bot: name\\%s\\team\\%s", client->pers.netname, team);
	}
	G_LogPrintf( "ClientUserinfoChanged: %i %s\n", clientNum, s );
}

/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char        *value;
	gclient_t   *client;
	char userinfo[MAX_INFO_STRING];
	gentity_t   *ent;
	int			i;
// L0 - MySQL example
#ifdef USE_MYSQL
	char query[1000];
#endif

	ent = &g_entities[clientNum];

	trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));

	// L0 - ASCII name bug crap..
	value = Info_ValueForKey(userinfo, "name");
	for (i = 0; i < strlen(value); i++) {
		if (value[i] < 0) {
			// extended ASCII chars have values between -128 and 0 (signed char)
			return "Change your name, extended ASCII chars are ^1NOT allowed!";
		}
	}

	// IP filtering
	// show_bug.cgi?id=500
	// recommanding PB based IP / GUID banning, the builtin system is pretty limited
	// check to see if they are on the banned IP list
	value = Info_ValueForKey( userinfo, "ip" );
	if ( G_FilterIPBanPacket( value ) ) {
		return "You are banned from this server.";
	}

	// Auth client
	/*if (trap_Cvar_VariableIntegerValue("sv_AuthEnabled")) {
		if (!Info_ValueForKey(userinfo, "cl_guid") || !Q_stricmp(Info_ValueForKey(userinfo, "cl_guid"), NO_GUID)) {
			return "Valid GUID is required to enter this server.";
		}
	}*/

	// Xian - check for max lives enforcement ban
	if ( g_enforcemaxlives.integer && ( g_maxlives.integer > 0 || g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0 ) ) {
		if ( trap_Cvar_VariableIntegerValue( "sv_punkbuster" ) ) {
			value = Info_ValueForKey( userinfo, "cl_guid" );
			if ( G_FilterMaxLivesPacket( value ) ) {
				return "Max Lives Enforcement Temp Ban. You will be able to reconnect when the next round starts. This ban is enforced to ensure you don't reconnect to get additional lives.";
			}
		} else {
			value = Info_ValueForKey( userinfo, "ip" ); // this isn't really needed, oh well.
			if ( G_FilterMaxLivesIPPacket( value ) ) {
				return "Max Lives Enforcement Temp Ban. You will be able to reconnect when the next round starts. This ban is enforced to ensure you don't reconnect to get additional lives.";
			}
		}
	}
	// End Xian

	// we don't check password for bots and local client
	// NOTE: local client <-> "ip" "localhost"
	//   this means this client is not running in our current process
	if ( !( ent->r.svFlags & SVF_BOT ) && ( strcmp( Info_ValueForKey( userinfo, "ip" ), "localhost" ) != 0 ) ) {
		// check for a password
		value = Info_ValueForKey( userinfo, "password" );
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
			 strcmp( g_password.string, value ) != 0 ) {
			return "Invalid password";
		}
	}

	// they can connect
	ent->client = level.clients + clientNum;
	client = ent->client;

	memset( client, 0, sizeof( *client ) );

	client->pers.connected = CON_CONNECTING;
	client->pers.connectTime = level.time;          // DHM - Nerve

	if ( firstTime ) {
		client->pers.initialSpawn = qtrue;              // DHM - Nerve

	}
	client->pers.complaints = 0;                    // DHM - Nerve

	// read or initialize the session data
	if ( firstTime || ( g_gametype.integer < GT_WOLF && level.newSession ) ) {
		G_InitSessionData( client, userinfo );
	}
	G_ReadSessionData( client );

	if ( isBot ) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		if ( !G_BotConnect( clientNum, !firstTime ) ) {
			return "BotConnectfailed";
		}
	}

	// OSPx - Country Flags
	if (gidb != NULL) {
		value = Info_ValueForKey(userinfo, "ip");

		if (!strcmp(value, "localhost")) {
			client->sess.uci = 0;
		}
		else {
			unsigned long ip = GeoIP_addr_to_num(value);

			if (((ip & 0xFF000000) == 0x0A000000) ||
				((ip & 0xFFF00000) == 0xAC100000) ||
				((ip & 0xFFFF0000) == 0xC0A80000)) {

				client->sess.uci = 0;
			}
			else {
				unsigned int ret = GeoIP_seek_record(gidb, ip);

				if (ret > 0) {
					client->sess.uci = ret;
				}
				else {
					client->sess.uci = 246;
					G_LogPrintf("GeoIP: This IP: %s cannot be located\n", value);
				}
			}
		}
	}
	else {
		client->sess.uci = 255;
	} // -OSPx

// L0 - MySQL example
#ifdef USE_MYSQL
	value = Info_ValueForKey(userinfo, "ip");

	if (sprintf(query, "INSERT INTO test(ip, username) VALUES('%s', '%s') ", value, client->pers.netname)) {
		trap_SQL_RunQuery(query);
		G_Printf("INSERT statement succeeded\n");
	}
	else {
		G_Printf("INSERT statement failed\n");
	}
#endif

	// get and distribute relevent paramters
	G_LogPrintf( "ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime && !isBot ) {

		AP(va("print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname));

		// RTCWPro - move here from SetTeam
		CPx(clientNum, va("print \"This server is running ^3%s\n\"", GAMEVERSION));
		CPx(clientNum, "print \"^7Type ^3/commands ^7to see the list of all available options.\n\"");
		if (strlen(g_serverMessage.string) > 0) CPx(clientNum, va( "cp \"%s\n\"2", g_serverMessage.string));
	}

	// count current clients and rank for scoreboard
	CalculateRanks();

	// Trigger rest lookup
	trap_SendServerCommand(clientNum, "revalidate");
	
	//unlagged - backward reconciliation #5
	// announce it
	if ( g_delagHitscan.integer ) {
		trap_SendServerCommand( clientNum, "print \"Server is Unlagged: full lag compensation is ON!\n\"" );
	}
	else {
		trap_SendServerCommand( clientNum, "print \"Server is Unlagged: full lag compensation is OFF!\n\"" );
	}
	//unlagged - backward reconciliation #5

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	gentity_t   *ent;
	gclient_t   *client;
	//gentity_t	*tent;
	int flags;
	int spawn_count;                // DHM - Nerve

	ent = g_entities + clientNum;

	if ( ent->botDelayBegin ) {
		G_QueueBotBegin( clientNum );
		ent->botDelayBegin = qfalse;
		return;
	}

	client = level.clients + clientNum;

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	client->pers.connected = CON_CONNECTED;
	// ATVI Wolfenstein Misc #414
	// don't reset the enterTime during a map_restart, we only want this when user explicitely changes team (and upon entering map)
	if ( !trap_Cvar_VariableIntegerValue( "sv_serverRestarting" ) ) {
		client->pers.enterTime = level.time;
	}
	client->pers.teamState.state = TEAM_BEGIN;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	// DHM - Nerve :: Also save PERS_SPAWN_COUNT, so that CG_Respawn happens
	spawn_count = client->ps.persistant[PERS_SPAWN_COUNT];
	flags = client->ps.eFlags;
	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;
	client->ps.persistant[PERS_SPAWN_COUNT] = spawn_count;

	// MrE: use capsule for collision
	//client->ps.eFlags |= EF_CAPSULE;
	//ent->r.svFlags |= SVF_CAPSULE;

	client->pers.complaintClient = -1;
	client->pers.complaintEndTime = -1;

	// locate ent at a spawn point
	ClientSpawn( ent, qfalse );


	// Xian -- Changed below for team independant maxlives

	if ( g_maxlives.integer > 0 ) {
		ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = ( g_maxlives.integer - 1 );
	} else {
		ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = -1;
	}

	if ( g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0 ) {
		if ( client->sess.sessionTeam == TEAM_RED ) {
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = ( g_axismaxlives.integer - 1 );
		} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = ( g_alliedmaxlives.integer - 1 );
		} else {
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = -1;
		}
	}

	// DHM - Nerve :: Start players in limbo mode if they change teams during the match
	if ( g_gametype.integer >= GT_WOLF && client->sess.sessionTeam != TEAM_SPECTATOR
		 && (((g_tournament.integer) && ( level.time - client->pers.connectTime ) > 1000)
        || ( level.time - client->pers.connectTime ) > 6000)) {
		ent->client->ps.pm_type = PM_DEAD;
		ent->r.contents = CONTENTS_CORPSE;
		ent->health = 0;
		ent->client->ps.stats[STAT_HEALTH] = 0;

		if ( g_maxlives.integer > 0 ) {
			ent->client->ps.persistant[PERS_RESPAWNS_LEFT]++;
		}

		limbo( ent, qfalse );
	}

	// Ridah, trigger a spawn event
	// DHM - Nerve :: Only in single player
	if ( g_gametype.integer == GT_SINGLE_PLAYER && !( ent->r.svFlags & SVF_CASTAI ) ) {
		AICast_ScriptEvent( AICast_GetCastState( clientNum ), "spawn", "" );
	}

	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
		// send event
		// DHM - Nerve :: Add back if we decide to have a spawn effect
		//tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		//tent->s.clientNum = ent->s.clientNum;

		if ( g_gametype.integer != GT_TOURNAMENT ) {
			// Ridah
			if ( !ent->r.svFlags & SVF_CASTAI ) {
				// done.
				trap_SendServerCommand( -1, va( "print \"[lof]%s" S_COLOR_WHITE " [lon]entered the game\n\"", client->pers.netname ) );
			}
		}
	}
	G_LogPrintf( "ClientBegin: %i\n", clientNum );

	// Xian - Check for maxlives enforcement
	if ( g_enforcemaxlives.integer == 1 && ( g_maxlives.integer > 0 || g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0 ) ) {
		char *value;
		char userinfo[MAX_INFO_STRING];
		trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
		value = Info_ValueForKey( userinfo, "cl_guid" );
		G_LogPrintf( "EnforceMaxLives-GUID: %s\n", value );
		AddMaxLivesGUID( value );
	}
	// End Xian

	// count current clients and rank for scoreboard
	CalculateRanks();

    time_t unixTime = time(NULL);
    client->sess.start_time = unixTime;//level.time; // start time of client (come back and change to unix time perhaps?)
}

// ------------------------------------------------------
// Team Weapon Count - ET Pub Port
// ------------------------------------------------------
int TeamWeaponCount(gentity_t* ent, team_t team, int weap) {
	int i, j, cnt;

	if (weap == -1) { // we aint checking for a weapon, so always include ourselves
		cnt = 1;
	}
	else { // we ARE checking for a weapon, so ignore ourselves
		cnt = 0;
	}

	// RtcwPro - without this loop we still have problems switching mid-round
	// loops through players and set all the latched weapons
	for (i = 0; i < level.maxclients; i++) {

		j = level.sortedClients[i];

		if (j == ent - g_entities) {
			continue;
		}

		if (level.clients[j].sess.sessionTeam != team) {
			continue;
		}

		SetWolfUserVars(&g_entities[j], NULL);
	}

	for (i = 0; i < level.numConnectedClients; i++) {

		j = level.sortedClients[i];

		if (j == ent - g_entities) {
			continue;
		}

		if (level.clients[j].sess.sessionTeam != team) {
			continue;
		}

		if (weap != -1) {


			gentity_t *player;
			player = g_entities + level.sortedClients[j];

			// if player is not in limbo and has the weapon
			if (!(player->client->ps.pm_flags & PMF_LIMBO) && level.clients[j].sess.playerWeapon == weap && level.clients[j].sess.latchPlayerWeapon == weap) {
				cnt++;
			}
		}
	}

	return cnt;
}

// ------------------------------------------------------
// Weapon Limiting - ET Pub Port
// ------------------------------------------------------
qboolean IsWeaponDisabled(
	gentity_t* ent,
	int sessionWeapon,
	weapon_t weapon,
	team_t team,
	qboolean quiet)
{
	int playerCount, weaponCount, maxCount;

	// tjw: specs can have any weapon they want
	if (team == TEAM_SPECTATOR) {
		return qfalse;
	}

	// forty - Flames heavy weapons restriction fix
	playerCount = TeamWeaponCount(ent, team, -1);
	weaponCount = TeamWeaponCount(ent, team, sessionWeapon);

	switch (weapon) {
		case WP_PANZERFAUST:
			maxCount = g_maxTeamPF.integer;
			if (maxCount == -1) {
				return qfalse;
			}
			/*if (strstr(team_maxPanzers.string, "%-")) { // these 2 if blocks allows a percentage
				maxCount = floor(maxCount * playerCount * 0.01f);
			}
			else if (strstr(team_maxPanzers.string, "%")) {
				maxCount = ceil(maxCount * playerCount * 0.01f);
			}*/
			if (weaponCount >= maxCount) {
				/*if (!quiet && !(ent->client->ps.pm_flags & PMF_LIMBO)) {
					CP("cp \"^1*^3 PANZERFAUST not available!^1 *\" 1");
				}*/
				return qtrue;
			}
			break;
		case WP_VENOM:
			maxCount = g_maxTeamVenom.integer;
			if (maxCount == -1) {
				return qfalse;
			}
			/*if (strstr(team_maxMG42s.string, "%-")) { // these 2 if blocks allows a percentage
				maxCount = floor(maxCount * playerCount * 0.01f);
			}
			else if (strstr(team_maxMG42s.string, "%")) {
				maxCount = ceil(maxCount * playerCount * 0.01f);
			}*/
			if (weaponCount >= maxCount) {
				/*if (!quiet && !(ent->client->ps.pm_flags & PMF_LIMBO)) {
					CP("cp \"^1*^3 VENOM not available!^1 *\" 1");
				}*/
				return qtrue;
			}
			break;
		case WP_FLAMETHROWER:
			maxCount = g_maxTeamFlamer.integer;
			if (maxCount == -1) {
				return qfalse;
			}
			/*if (strstr(team_maxFlamers.string, "%-")) { // these 2 if blocks allows a percentage
				maxCount = floor(maxCount * playerCount * 0.01f);
			}
			else if (strstr(team_maxFlamers.string, "%")) {
				maxCount = ceil(maxCount * playerCount * 0.01f);
			}*/
			if (weaponCount >= maxCount) {
				/*if (!quiet && !(ent->client->ps.pm_flags & PMF_LIMBO)) {
					CP("cp \"^1*^3 FLAMETHROWER not available!^1 *\" 1");
				}*/
				return qtrue;
			}
			break;
		case WP_MAUSER:
			maxCount = g_maxTeamSniper.integer;
			if (maxCount == -1) {
				return qfalse;
			}
			/*if (strstr(team_maxMortars.string, "%-")) { // these 2 if blocks allows a percentage
				maxCount = floor(maxCount * playerCount * 0.01f);
			}
			else if (strstr(team_maxMortars.string, "%")) {
				maxCount = ceil(maxCount * playerCount * 0.01f);
			}*/
			if (weaponCount >= maxCount) {
				/*if (!quiet && !(ent->client->ps.pm_flags & PMF_LIMBO)) {
					CP("cp \"^1*^3 SNIPER not available!^1 *\" 1");
				}*/
				return qtrue;
			}
			break;

	}

	return qfalse;
}


/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(gentity_t *ent, qboolean revived) {
	int index;
	vec3_t spawn_origin, spawn_angles;
	gclient_t *client;
	int i;
	clientPersistant_t saved;
	clientSession_t savedSess;
	int persistant[MAX_PERSISTANT];
	gentity_t *spawnPoint;
	int flags;
	int savedPing;
	int savedTeam;
	qboolean savedVoted = qfalse;         // NERVE - SMF

	index = ent - g_entities;
	client = ent->client;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client

	// regardless of revive or respawn clear the powerups
	if (client->ps.powerups)
	{
		ent->s.powerups = 0;
		memset(client->ps.powerups, 0, sizeof(client->ps.powerups));
	}

	if (revived) {
		spawnPoint = ent;
		VectorCopy(ent->r.currentOrigin, spawn_origin); // fix document/revive bug by using r.currentOrigin  //VectorCopy( ent->s.origin, spawn_origin );
		spawn_origin[2] += 9;   // spawns seem to be sunk into ground?
		VectorCopy(ent->s.angles, spawn_angles);
	}
	else
	{
		ent->aiName = "player";  // needed for script AI
		//ent->aiTeam = 1;		// member of allies
		//ent->client->ps.teamNum = ent->aiTeam;
		//AICast_ScriptParse( AICast_GetCastState(ent->s.number) );
		// done.

		if (client->sess.sessionTeam == TEAM_SPECTATOR) {
			spawnPoint = SelectSpectatorSpawnPoint(
				spawn_origin, spawn_angles);
		}
		else if (g_gametype.integer >= GT_TEAM) {
			spawnPoint = SelectCTFSpawnPoint(
				client->sess.sessionTeam,
				client->pers.teamState.state,
				spawn_origin, spawn_angles, client->sess.spawnObjectiveIndex);
		}
		else {
			do {
				// the first spawn should be at a good looking spot
				if (!client->pers.initialSpawn && client->pers.localClient) {
					client->pers.initialSpawn = qtrue;
					spawnPoint = SelectInitialSpawnPoint(spawn_origin, spawn_angles);
				}
				else {
					// don't spawn near existing origin if possible
					spawnPoint = SelectSpawnPoint(
						client->ps.origin,
						spawn_origin, spawn_angles);
				}

				if ((spawnPoint->flags & FL_NO_BOTS) && (ent->r.svFlags & SVF_BOT)) {
					continue;   // try again
				}
				// just to be symetric, we have a nohumans option...
				if ((spawnPoint->flags & FL_NO_HUMANS) && !(ent->r.svFlags & SVF_BOT)) {
					continue;   // try again
				}

				break;

			} while (1);
		}
	}

	client->pers.teamState.state = TEAM_ACTIVE;

	// toggle the teleport bit so the client knows to not lerp
	flags = ent->client->ps.eFlags & EF_TELEPORT_BIT;
	flags ^= EF_TELEPORT_BIT;
	flags |= (client->ps.eFlags & EF_VOTED); // L0 - Fixes vote abuse by suicide and vote override..

	if (g_antilag.integer) // Unlagged
	{
		//unlagged - backward reconciliation #3
		// we don't want players being backward-reconciled to the place they died
		G_ResetHistory(ent);
		// and this is as good a time as any to clear the saved state
		ent->client->saved.leveltime = 0;
		//unlagged - backward reconciliation #3
	}

	// clear everything but the persistant data

	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
	savedTeam = client->ps.teamNum;

	// NERVE - SMF
	if ( client->ps.eFlags & EF_VOTED ) {
		savedVoted = qtrue;
	}

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}

	memset( client, 0, sizeof( *client ) );

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
	client->ps.teamNum = savedTeam;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}

	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + 12000;

	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
	client->ps.eFlags = flags;
	// MrE: use capsules for AI and player
	//client->ps.eFlags |= EF_CAPSULE;

	// TTimo
	if ( savedVoted ) {
		client->ps.eFlags |= EF_VOTED;
	}

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	if ( !( ent->r.svFlags & SVF_CASTAI ) ) {
		ent->classname = "player";
	}
	ent->r.contents = CONTENTS_BODY;

	// RF, AI should be clipped by monsterclip brushes
	if ( ent->r.svFlags & SVF_CASTAI ) {
		ent->clipmask = MASK_PLAYERSOLID | CONTENTS_MONSTERCLIP;
	} else {
		ent->clipmask = MASK_PLAYERSOLID;
	}

	ent->client->animationInfo.bodyModelHandle = ent->client->sess.sessionTeam == TEAM_RED ?
		AXIS_MODEL_HANDLE : ALLIED_MODEL_HANDLE;

	// DHM - Nerve :: Init to -1 on first spawn;
	if ( !revived ) {
		ent->props_frame_state = -1;
	}

	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;
	// L0
	// Life stats
	ent->client->pers.life_kills = 0;
	ent->client->pers.life_acc_hits = 0;
	ent->client->pers.life_acc_shots = 0;
	ent->client->pers.life_headshots = 0;
	// End life stats

	VectorCopy( playerMins, ent->r.mins );
	VectorCopy( playerMaxs, ent->r.maxs );

	// Ridah, setup the bounding boxes and viewheights for prediction
	VectorCopy( ent->r.mins, client->ps.mins );
	VectorCopy( ent->r.maxs, client->ps.maxs );

	client->ps.crouchViewHeight = CROUCH_VIEWHEIGHT;
	client->ps.standViewHeight = DEFAULT_VIEWHEIGHT;
	client->ps.deadViewHeight = DEAD_VIEWHEIGHT;

	client->ps.crouchMaxZ = client->ps.maxs[2] - ( client->ps.standViewHeight - client->ps.crouchViewHeight );

	client->ps.runSpeedScale = 0.8;
	client->ps.sprintSpeedScale = 1.1;
	client->ps.crouchSpeedScale = 0.25;

	// Rafael
	client->ps.sprintTime = SPRINTTIME;
	client->ps.sprintExertTime = 0;

	client->ps.friction = 1.0;
	// done.

	// TTimo
	// retrieve from the persistant storage (we use this in pmoveExt_t beause we need it in bg_*)
	client->pmext.bAutoReload = client->pers.bAutoReloadAux;
	// done

	client->ps.clientNum = index;

	trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );  // NERVE - SMF - moved this up here

	SetWolfUserVars( ent, NULL );           // NERVE - SMF

	// DHM - Nerve :: Add appropriate weapons
	if ( g_gametype.integer >= GT_WOLF ) {

		if ( !revived ) {
			qboolean update = qfalse;

			if ( client->sess.playerType != client->sess.latchPlayerType ) {
				update = qtrue;
			}


			client->sess.playerType = client->sess.latchPlayerType;
			client->sess.playerWeapon = client->sess.latchPlayerWeapon;
			client->sess.playerItem = client->sess.latchPlayerItem;
			client->sess.playerSkin = client->sess.latchPlayerSkin;

			if ( update ) {
				ClientUserinfoChanged( index );

                if (g_gameStatslog.integer && (g_gamestate.integer == GS_PLAYING) ) {
                    G_writeGeneralEvent (ent,ent, " ", eventClassChange);
                }
			}
		}

		// TTimo keep it isolated from spectator to be safe still
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			// Xian - Moved the invul. stuff out of SetWolfSpawnWeapons and put it here for clarity
			if ( g_fastres.integer == 1 && revived ) {
				client->ps.powerups[PW_INVULNERABLE] = level.time + g_fastResMsec.integer;
			} else {
				// L0 - Spawn protection
				if (client->sess.sessionTeam == TEAM_RED)
					client->ps.powerups[PW_INVULNERABLE] = level.time + g_axisSpawnProtectionTime.integer;
				else if (client->sess.sessionTeam == TEAM_BLUE)
					client->ps.powerups[PW_INVULNERABLE] = level.time + g_alliedSpawnProtectionTime.integer;
				// We don't know what team player is...default it
				else
					client->ps.powerups[PW_INVULNERABLE] = level.time + 3000;
				// End
			}
		}

		// End Xian

		SetWolfSpawnWeapons(ent); // JPW NERVE -- increases stats[STAT_MAX_HEALTH] based on # of medics in game
	}
	// dhm - end

	AddMedicTeamBonus(client);

	// JPW NERVE ***NOTE*** the following line is order-dependent and must *FOLLOW* SetWolfSpawnWeapons() in multiplayer
	// SetWolfSpawnWeapons() now adds medic team bonus and stores in ps.stats[STAT_MAX_HEALTH].
	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];

	//if (g_debugMode.integer)
	//{
	//	G_Printf("Player spawned with StartHealth: %i MaxHealth: %i STAT_MAX_HEALTH: %i\n", ent->health, client->pers.maxHealth, client->ps.stats[STAT_MAX_HEALTH]);
	//	AP(va("print \"Player spawned with StartHealth:%i MaxHealth: %i STAT_MAX_HEALTH: %i\n\"", ent->health, client->pers.maxHealth, client->ps.stats[STAT_MAX_HEALTH]));
	//}

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	// if spawning at spawn point do default view
	if (!revived)
	{
		SetClientViewAngle( ent, spawn_angles );
	}
	// else if g_reviveSameDirection is enabled spawn them in the direction they were killed
	else if (g_reviveSameDirection.integer)
	{
		vec3_t newangle;

		// RtcwPro - restore the value for the client's view before death
		newangle[YAW] = client->pers.deathYaw; //ps.persistant[PERS_DEATH_YAW];
		newangle[PITCH] = 0;
		newangle[ROLL] = 0;

		SetClientViewAngle(ent, newangle);
	}
	// else do default view
	else
	{
		SetClientViewAngle(ent, spawn_angles);
	}

	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		//G_KillBox( ent );
		trap_LinkEntity( ent );
	}

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;
	client->latched_wbuttons = 0;   //----(SA)	added

	if ( level.intermissiontime ) {
		MoveClientToIntermission( ent );
	} else {
		// fire the targets of the spawn point
		if ( !revived ) {
			G_UseTargets( spawnPoint, ent );
		}
	}

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent - g_entities );

	// positively link the client, even if the command times are weird
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
		//BG_PlayerStateToEntityStatePro(&client->ps, &ent->s, level.time, qtrue); // RTCWPro
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
		trap_LinkEntity( ent );
	}

	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
	//BG_PlayerStateToEntityStatePro(&client->ps, &ent->s, level.time, qtrue); // RTCWPro

	// show_bug.cgi?id=569
	//G_ResetMarkers( ent );

	// RTCWPro - head stuff
	// add the head entity if it already hasn't been
	AddHeadEntity(ent);
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t   *ent;
	gentity_t   *flag = NULL;
	gitem_t     *item = NULL;
	vec3_t launchvel;
	int i;

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;
	}

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			 && level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			 && level.clients[i].sess.spectatorClient == clientNum ) {
			StopFollowing( &g_entities[i] );
		}
		if ( g_gametype.integer >= GT_WOLF
			 && level.clients[i].ps.pm_flags & PMF_LIMBO
			 && level.clients[i].sess.spectatorClient == clientNum ) {
			Cmd_FollowCycle_f( &g_entities[i], 1 );
		}
	}

	// NERVE - SMF - remove complaint client
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.complaintClient == clientNum ) {
			level.clients[i].pers.complaintClient = -1;
			level.clients[i].pers.complaintEndTime = 0;

			trap_SendServerCommand( i, "complaint -2" );
			break;
		}
	}

	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED
		 && ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems( ent );

		// New code for tossing flags
		if (g_gametype.integer >= GT_WOLF) {
			if (ent->client->ps.powerups[PW_REDFLAG]) {
				item = BG_FindItem("Red Flag");
				if (!item) {
					item = BG_FindItem("Objective");
				}

				ent->client->ps.powerups[PW_REDFLAG] = 0;
			}
			if (ent->client->ps.powerups[PW_BLUEFLAG]) {
				item = BG_FindItem("Blue Flag");
				if (!item) {
					item = BG_FindItem("Objective");
				}

				ent->client->ps.powerups[PW_BLUEFLAG] = 0;
			}

			if (item) {
				// OSPx - Fix documents passing exploit
				launchvel[0] = 0;
				launchvel[1] = 0;
				launchvel[2] = 40;

				flag = LaunchItem(item, ent->r.currentOrigin, launchvel, ent->s.number);
				flag->s.modelindex2 = ent->s.otherEntityNum2; // JPW NERVE FIXME set player->otherentitynum2 with old modelindex2 from flag and restore here
				flag->message = ent->message;   // DHM - Nerve :: also restore item name
				// Clear out player's temp copies
				ent->s.otherEntityNum2 = 0;
				ent->message = NULL;
			}

			// Record the players stats if they /quit so we can reload or save them
			if (g_gameStatslog.integer && (ent->client->sess.sessionTeam == TEAM_BLUE || ent->client->sess.sessionTeam == TEAM_RED))
			{
				// record any player that disconnects
				G_jstatsByPlayers(qtrue, qtrue, ent->client);
			}
		}
	}

	G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

	// if we are playing in tourney mode and losing, give a win to the other player
	if ( g_gametype.integer == GT_TOURNAMENT && !level.intermissiontime
		 && !level.warmupTime && level.sortedClients[1] == clientNum ) {
		level.clients[ level.sortedClients[0] ].sess.wins++;
		ClientUserinfoChanged( level.sortedClients[0] );
	}

	// if a player disconnects during warmup make sure the team's ready status doesn't start the match
	if (g_tournament.integer
		&& (g_gamestate.integer == GS_WARMUP || g_gamestate.integer == GS_WARMUP_COUNTDOWN)
		&& (ent->client->sess.sessionTeam == TEAM_BLUE || ent->client->sess.sessionTeam == TEAM_RED))
	{
		G_readyResetOnPlayerLeave(ent->client->sess.sessionTeam);
	}

    if (g_gameStatslog.integer && g_gamestate.integer == GS_PLAYING) {
        G_writeDisconnectEvent(ent);
    }

	trap_UnlinkEntity( ent );
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->client->sess.sessionTeam = TEAM_FREE;

	ent->client->sess.end_time = level.time; // end time of client (come back and change to unix time perhaps?)


// JPW NERVE -- mg42 additions
	ent->active = 0;
// jpw

	// RTCWPro - head stuff
	FreeHeadEntity(ent);

	trap_SetConfigstring( CS_PLAYERS + clientNum, "" );

	CalculateRanks();

	HandleEmptyTeams();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownClient( clientNum );
	}
}

/*
==================
G_RetrieveMoveSpeedsFromClient
==================
*/
void G_RetrieveMoveSpeedsFromClient( int entnum, char *text ) {
	char *text_p, *token;
	animation_t *anim;
	animModelInfo_t *modelInfo;

	text_p = text;

	// get the model name
	token = COM_Parse( &text_p );
	if ( !token || !token[0] ) {
		G_Error( "G_RetrieveMoveSpeedsFromClient: internal error" );
	}

	modelInfo = BG_ModelInfoForModelname( token );

	if ( !modelInfo ) {
		// ignore it
		return;
	}

	while ( 1 ) {
		token = COM_Parse( &text_p );
		if ( !token || !token[0] ) {
			break;
		}

		// this is a name
		anim = BG_AnimationForString( token, modelInfo );
		if ( anim->moveSpeed == 0 ) {
			G_Error( "G_RetrieveMoveSpeedsFromClient: trying to set movespeed for non-moving animation" );
		}

		// get the movespeed
		token = COM_Parse( &text_p );
		if ( !token || !token[0] ) {
			G_Error( "G_RetrieveMoveSpeedsFromClient: missing movespeed" );
		}
		anim->moveSpeed = atoi( token );
	}
}
