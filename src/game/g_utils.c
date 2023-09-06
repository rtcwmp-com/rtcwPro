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

/*
 * name:	g_utils.c
 *
 * desc:	misc utility functions for game module
 *
*/

#include "g_local.h"

typedef struct {
	char oldShader[MAX_QPATH];
	char newShader[MAX_QPATH];
	float timeOffset;
} shaderRemap_t;

#define MAX_SHADER_REMAPS 128
#define MAX_MAP_MATCHES 128 // RTCWPro

int remapCount = 0;
shaderRemap_t remappedShaders[MAX_SHADER_REMAPS];

#define FLT_MAX          3.402823466e+38F // RTCWPro

void AddRemap( const char *oldShader, const char *newShader, float timeOffset ) {
	int i;

	for ( i = 0; i < remapCount; i++ ) {
		if ( Q_stricmp( oldShader, remappedShaders[i].oldShader ) == 0 ) {
			// found it, just update this one
			strcpy( remappedShaders[i].newShader,newShader );
			remappedShaders[i].timeOffset = timeOffset;
			return;
		}
	}
	if ( remapCount < MAX_SHADER_REMAPS ) {
		strcpy( remappedShaders[remapCount].newShader,newShader );
		strcpy( remappedShaders[remapCount].oldShader,oldShader );
		remappedShaders[remapCount].timeOffset = timeOffset;
		remapCount++;
	}
}

const char *BuildShaderStateConfig() {
	static char buff[MAX_STRING_CHARS * 4];
	char out[( MAX_QPATH * 2 ) + 5];
	int i;

	memset( buff, 0, MAX_STRING_CHARS );
	for ( i = 0; i < remapCount; i++ ) {
		Com_sprintf( out, ( MAX_QPATH * 2 ) + 5, "%s=%s:%5.2f@", remappedShaders[i].oldShader, remappedShaders[i].newShader, remappedShaders[i].timeOffset );
		Q_strcat( buff, sizeof( buff ), out );
	}
	return buff;
}

/*
=========================================================================

model / sound configstring indexes

=========================================================================
*/

/*
================
G_FindConfigstringIndex

================
*/
int G_FindConfigstringIndex( const char *name, int start, int max, qboolean create ) {
	int i;
	char s[MAX_STRING_CHARS];

	if ( !name || !name[0] ) {
		return 0;
	}

	for ( i = 1 ; i < max ; i++ ) {
		trap_GetConfigstring( start + i, s, sizeof( s ) );
		if ( !s[0] ) {
			break;
		}
		if ( !strcmp( s, name ) ) {
			return i;
		}
	}

	if ( !create ) {
		return 0;
	}

	if ( i == max ) {
		G_Error( "G_FindConfigstringIndex: overflow" );
	}

	trap_SetConfigstring( start + i, name );

	return i;
}


int G_ModelIndex( char *name ) {
	return G_FindConfigstringIndex( name, CS_MODELS, MAX_MODELS, qtrue );
}

int G_SoundIndex( const char *name ) {
	return G_FindConfigstringIndex( name, CS_SOUNDS, MAX_SOUNDS, qtrue );
}

//=====================================================================


/*
================
G_TeamCommand

Broadcasts a command to only a specific team
================
*/
void G_TeamCommand( team_t team, char *cmd ) {
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			if ( level.clients[i].sess.sessionTeam == team ) {
				trap_SendServerCommand( i, va( "%s", cmd ) );
			}
		}
	}
}


/*
=============
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the entity after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
gentity_t *G_Find( gentity_t *from, int fieldofs, const char *match ) {
	char    *s;

	if ( !from ) {
		from = g_entities;
	} else {
		from++;
	}

	for ( ; from < &g_entities[level.num_entities] ; from++ )
	{
		if ( !from->inuse ) {
			continue;
		}
		s = *( char ** )( (byte *)from + fieldofs );
		if ( !s ) {
			continue;
		}
		if ( !Q_stricmp( s, match ) ) {
			return from;
		}
	}

	return NULL;
}


/*
=============
G_PickTarget

Selects a random entity from among the targets
=============
*/
#define MAXCHOICES  32

gentity_t *G_PickTarget( char *targetname ) {
	gentity_t   *ent = NULL;
	int num_choices = 0;
	gentity_t   *choice[MAXCHOICES];

	if ( !targetname ) {
		//G_Printf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while ( 1 )
	{
		ent = G_Find( ent, FOFS( targetname ), targetname );
		if ( !ent ) {
			break;
		}
		choice[num_choices++] = ent;
		if ( num_choices == MAXCHOICES ) {
			break;
		}
	}

	if ( !num_choices ) {
		G_Printf( "G_PickTarget: target %s not found\n", targetname );
		return NULL;
	}

	return choice[rand() % num_choices];
}


/*
==============================
G_UseTargets

"activator" should be set to the entity that initiated the firing.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

==============================
*/
void G_UseTargets( gentity_t *ent, gentity_t *activator ) {
	gentity_t       *t;

	if ( !ent ) {
		return;
	}

	if ( ent->targetShaderName && ent->targetShaderNewName ) {
		float f = level.time * 0.001;
		AddRemap( ent->targetShaderName, ent->targetShaderNewName, f );
		trap_SetConfigstring( CS_SHADERSTATE, BuildShaderStateConfig() );
	}

	if ( !ent->target ) {
		return;
	}

	t = NULL;
	while ( ( t = G_Find( t, FOFS( targetname ), ent->target ) ) != NULL ) {
		if ( t == ent ) {
			G_Printf( "WARNING: Entity used itself.\n" );
		} else {
			if ( t->use ) {
				//G_Printf ("ent->classname %s ent->targetname %s t->targetname %s t->s.number %d\n", ent->classname, ent->targetname, t->targetname, t->s.number);

				t->flags |= ( ent->flags & FL_KICKACTIVATE ); // (SA) If 'ent' was kicked to activate, pass this along to it's targets.
															  //		It may become handy to put a "KICKABLE" flag in ents so that it knows whether to pass this along or not
															  //		Right now, the only situation where it would be weird would be an invisible_user that is a 'button' near
															  //		a rotating door that it triggers.  Kick the switch and the door next to it flies open.

				t->flags |= ( ent->flags & FL_SOFTACTIVATE ); // (SA) likewise for soft activation

				if (    activator &&
						(   ( Q_stricmp( t->classname, "func_door" ) == 0 ) ||
							( Q_stricmp( t->classname, "func_door_rotating" ) == 0 )
						)
						) {
					// check door usage rules before allowing any entity to trigger a door open
					G_TryDoor( t, ent, activator );       // (door,other,activator)
				} else {
					t->use( t, ent, activator );
				}
			}
		}
		if ( !ent->inuse ) {
			G_Printf( "entity was removed while using targets\n" );
			return;
		}
	}
}


/*
=============
TempVector

This is just a convenience function
for making temporary vectors for function calls
=============
*/
/*
float	*tv( float x, float y, float z ) {
	static	int		index;
	static	vec3_t	vecs[8];
	float	*v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}
*/

/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char    *vtos( const vec3_t v ) {
	static int index;
	static char str[8][32];
	char    *s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = ( index + 1 ) & 7;

	Com_sprintf( s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2] );

	return s;
}
char    *vtosf( const vec3_t v ) {
	static int index;
	static char str[8][64];
	char    *s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = ( index + 1 ) & 7;

	Com_sprintf( s, 64, "(%f %f %f)", v[0], v[1], v[2] );

	return s;
}


/*
===============
G_SetMovedir

The editor only specifies a single value for angles (yaw),
but we have special constants to generate an up or down direction.
Angles will be cleared, because it is being used to represent a direction
instead of an orientation.
===============
*/
void G_SetMovedir( vec3_t angles, vec3_t movedir ) {
	static vec3_t VEC_UP        = {0, -1, 0};
	static vec3_t MOVEDIR_UP    = {0, 0, 1};
	static vec3_t VEC_DOWN      = {0, -2, 0};
	static vec3_t MOVEDIR_DOWN  = {0, 0, -1};

	if ( VectorCompare( angles, VEC_UP ) ) {
		VectorCopy( MOVEDIR_UP, movedir );
	} else if ( VectorCompare( angles, VEC_DOWN ) ) {
		VectorCopy( MOVEDIR_DOWN, movedir );
	} else {
		AngleVectors( angles, movedir, NULL, NULL );
	}
	VectorClear( angles );
}



void G_InitGentity( gentity_t *e ) {
	e->inuse = qtrue;
	e->classname = "noclass";
	e->s.number = e - g_entities;
	e->r.ownerNum = ENTITYNUM_NONE;
	e->headshotDamageScale = 1.0;   // RF, default value

	// RF, init scripting
	e->scriptStatus.scriptEventIndex = -1;
}

/*
=================
G_Spawn

Either finds a free entity, or allocates a new one.

  The slots from 0 to MAX_CLIENTS-1 are always reserved for clients, and will
never be used by anything else.

Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
gentity_t *G_Spawn( void ) {
	int i, force;
	gentity_t   *e;

	e = NULL;   // shut up warning
	i = 0;      // shut up warning
	for ( force = 0 ; force < 2 ; force++ ) {
		// if we go through all entities and can't find one to free,
		// override the normal minimum times before use
		e = &g_entities[MAX_CLIENTS];
		for ( i = MAX_CLIENTS ; i < level.num_entities ; i++, e++ ) {
			if ( e->inuse ) {
				continue;
			}

			// the first couple seconds of server time can involve a lot of
			// freeing and allocating, so relax the replacement policy
			if ( !force && e->freetime > level.startTime + 2000 && level.time - e->freetime < 1000 ) {
				continue;
			}

			// reuse this slot
			G_InitGentity( e );
			return e;
		}
		if ( i != ENTITYNUM_MAX_NORMAL ) {
			break;
		}
	}
	if ( i == ENTITYNUM_MAX_NORMAL ) {
		for ( i = 0; i < MAX_GENTITIES; i++ ) {
			G_Printf( "%4i: %s\n", i, g_entities[i].classname );
		}
		G_Error( "G_Spawn: no free entities" );
	}

	// open up a new slot
	level.num_entities++;

	// let the server system know that there are more entities
	trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ),
						 &level.clients[0].ps, sizeof( level.clients[0] ) );

	G_InitGentity( e );
	return e;
}

/*
=================
G_EntitiesFree
=================
*/
qboolean G_EntitiesFree( void ) {
	int i;
	gentity_t   *e;

	e = &g_entities[MAX_CLIENTS];
	for ( i = MAX_CLIENTS; i < level.num_entities; i++, e++ ) {
		if ( e->inuse ) {
			continue;
		}
		// slot available
		return qtrue;
	}
	return qfalse;
}


/*
=================
G_FreeEntity

Marks the entity as free
=================
*/
void G_FreeEntity( gentity_t *ed ) {
	trap_UnlinkEntity( ed );     // unlink from world

	if ( ed->neverFree ) {
		return;
	}

	memset( ed, 0, sizeof( *ed ) );
	ed->classname = "freed";
	ed->freetime = level.time;
	ed->inuse = qfalse;
}

/*
=================
G_TempEntity

Spawns an event entity that will be auto-removed
The origin will be snapped to save net bandwidth, so care
must be taken if the origin is right on a surface (snap towards start vector first)
=================
*/
gentity_t *G_TempEntity( vec3_t origin, int event ) {
	gentity_t       *e;
	vec3_t snapped;

	e = G_Spawn();
	e->s.eType = ET_EVENTS + event;

	e->classname = "tempEntity";
	e->eventTime = level.time;
	e->r.eventTime = level.time;
	e->freeAfterEvent = qtrue;

	VectorCopy( origin, snapped );
	SnapVector( snapped );      // save network bandwidth
	G_SetOrigin( e, snapped );

	// find cluster for PVS
	trap_LinkEntity( e );

	return e;
}



/*
==============================================================================

Kill box

==============================================================================
*/

/*
=================
G_KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
void G_KillBox( gentity_t *ent ) {
	int i, num;
	int touch[MAX_GENTITIES];
	gentity_t   *hit;
	vec3_t mins, maxs;

	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for ( i = 0 ; i < num ; i++ ) {
		hit = &g_entities[touch[i]];
		if ( !hit->client ) {
			continue;
		}
		if ( !hit->r.linked ) { // RF, inactive AI shouldn't be gibbed
			continue;
		}

		// nail it
		G_Damage( hit, ent, ent, NULL, NULL,
				  100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG );
	}

}

//==============================================================================

/*
===============
G_AddPredictableEvent

Use for non-pmove events that would also be predicted on the
client side: jumppads and item pickups
Adds an event+parm and twiddles the event counter
===============
*/
void G_AddPredictableEvent( gentity_t *ent, int event, int eventParm ) {
	if ( !ent->client ) {
		return;
	}
	BG_AddPredictableEventToPlayerstate( event, eventParm, &ent->client->ps );
}


/*
===============
G_AddEvent

Adds an event+parm and twiddles the event counter
===============
*/
void G_AddEvent( gentity_t *ent, int event, int eventParm ) {
//	int		bits;

	if ( !event ) {
		G_Printf( "G_AddEvent: zero event added for entity %i\n", ent->s.number );
		return;
	}

	// Ridah, use the sequential event list
	if ( ent->client ) {
		// NERVE - SMF - commented in - externalEvents not being handled properly in Wolf right now
		ent->client->ps.events[ent->client->ps.eventSequence & ( MAX_EVENTS - 1 )] = event;
		ent->client->ps.eventParms[ent->client->ps.eventSequence & ( MAX_EVENTS - 1 )] = eventParm;
		ent->client->ps.eventSequence++;
		// -NERVE - SMF

		// NERVE - SMF - commented out
//		bits = ent->client->ps.externalEvent & EV_EVENT_BITS;
//		bits = ( bits + EV_EVENT_BIT1 ) & EV_EVENT_BITS;
//		ent->client->ps.externalEvent = event | bits;
//		ent->client->ps.externalEventParm = eventParm;
//		ent->client->ps.externalEventTime = level.time;
		// -NERVE - SMF
	} else {
		// NERVE - SMF - commented in - externalEvents not being handled properly in Wolf right now
		ent->s.events[ent->s.eventSequence & ( MAX_EVENTS - 1 )] = event;
		ent->s.eventParms[ent->s.eventSequence & ( MAX_EVENTS - 1 )] = eventParm;
		ent->s.eventSequence++;
		// -NERVE - SMF

		// NERVE - SMF - commented out
//		bits = ent->s.event & EV_EVENT_BITS;
//		bits = ( bits + EV_EVENT_BIT1 ) & EV_EVENT_BITS;
//		ent->s.event = event | bits;
//		ent->s.eventParm = eventParm;
		// -NERVE - SMF
	}
	ent->eventTime = level.time;
	ent->r.eventTime = level.time;
}


/*
=============
G_Sound

  Ridah, removed channel parm, since it wasn't used, and could cause confusion
=============
*/
void G_Sound( gentity_t *ent, int soundIndex ) {
	gentity_t   *te;

	te = G_TempEntity( ent->r.currentOrigin, EV_GENERAL_SOUND );
	te->s.eventParm = soundIndex;
}

/*
=============
G_AnimScriptSound
=============
*/
void G_AnimScriptSound( int soundIndex, vec3_t org, int client ) {
	gentity_t *e;
	e = &g_entities[client];
	G_AddEvent( e, EV_GENERAL_SOUND, soundIndex );
	AICast_RecordScriptSound( client );
}

//==============================================================================


/*
================
G_SetOrigin

Sets the pos trajectory for a fixed position
================
*/
void G_SetOrigin( gentity_t *ent, vec3_t origin ) {
	VectorCopy( origin, ent->s.pos.trBase );
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	VectorClear( ent->s.pos.trDelta );

	VectorCopy( origin, ent->r.currentOrigin );
}


/*
==============
G_SetOrigin
==============
*/
void G_SetAngle( gentity_t *ent, vec3_t angle ) {

	VectorCopy( angle, ent->s.apos.trBase );
	ent->s.apos.trType = TR_STATIONARY;
	ent->s.apos.trTime = 0;
	ent->s.apos.trDuration = 0;
	VectorClear( ent->s.apos.trDelta );

	VectorCopy( angle, ent->r.currentAngles );

//	VectorCopy (ent->s.angles, ent->s.apos.trDelta );

}

/*
====================
infront
====================
*/

qboolean infront( gentity_t *self, gentity_t *other ) {
	vec3_t vec;
	float dot;
	vec3_t forward;

	AngleVectors( self->s.angles, forward, NULL, NULL );
	VectorSubtract( other->r.currentOrigin, self->r.currentOrigin, vec );
	VectorNormalize( vec );
	dot = DotProduct( vec, forward );
	// G_Printf( "other %5.2f\n",	dot);
	if ( dot > 0.0 ) {
		return qtrue;
	}
	return qfalse;
}

//RF, tag connections
/*
==================
G_ProcessTagConnect
==================
*/
void G_ProcessTagConnect( gentity_t *ent ) {
	if ( !ent->tagName ) {
		G_Error( "G_ProcessTagConnect: NULL ent->tagName\n" );
	}
	if ( !ent->tagParent ) {
		G_Error( "G_ProcessTagConnect: NULL ent->tagParent\n" );
	}
	G_FindConfigstringIndex( va( "%i %i %s", ent->s.number, ent->tagParent->s.number, ent->tagName ), CS_TAGCONNECTS, MAX_TAGCONNECTS, qtrue );
	ent->s.eFlags |= EF_TAGCONNECT;

	// clear out the angles so it always starts out facing the tag direction
	VectorClear( ent->s.angles );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	ent->s.apos.trTime = level.time;
	ent->s.apos.trDuration = 0;
	ent->s.apos.trType = TR_STATIONARY;
	VectorClear( ent->s.apos.trDelta );
	VectorClear( ent->r.currentAngles );
}

/*
================
DebugLine

  debug polygons only work when running a local game
  with r_debugSurface set to 2
================
*/
int DebugLine( vec3_t start, vec3_t end, int color ) {
	vec3_t points[4], dir, cross, up = {0, 0, 1};
	float dot;

	VectorCopy( start, points[0] );
	VectorCopy( start, points[1] );
	//points[1][2] -= 2;
	VectorCopy( end, points[2] );
	//points[2][2] -= 2;
	VectorCopy( end, points[3] );


	VectorSubtract( end, start, dir );
	VectorNormalize( dir );
	dot = DotProduct( dir, up );
	if ( dot > 0.99 || dot < -0.99 ) {
		VectorSet( cross, 1, 0, 0 );
	} else { CrossProduct( dir, up, cross );}

	VectorNormalize( cross );

	VectorMA( points[0], 2, cross, points[0] );
	VectorMA( points[1], -2, cross, points[1] );
	VectorMA( points[2], -2, cross, points[2] );
	VectorMA( points[3], 2, cross, points[3] );

	return trap_DebugPolygonCreate( color, 4, points );
}

/*
================
RTCWPro - allowteams - ET port
G_AllowTeamsAllowed
================
*/
qboolean G_AllowTeamsAllowed(gentity_t* ent, gentity_t* activator)
{
	if (ent->allowteams && activator && activator->client)
	{
		if (activator->client->sess.sessionTeam != TEAM_SPECTATOR)
		{
			int checkTeam = activator->client->sess.sessionTeam;

			if (!(ent->allowteams & checkTeam))
			{
				return qfalse;
			}
		}
	}

	return qtrue;
}

/*
==============
RTCWPro - determine if
class can drop weapon
Originally from PubJ
==============
*/
qboolean AllowDropForClass(gentity_t* ent, int pclass)
{
	qboolean varval;
	gclient_t* client = ent->client;
	pclass = client->ps.stats[STAT_PLAYER_CLASS];

	switch (pclass)
	{
	case PC_SOLDIER:
		varval = g_dropWeapons.integer & WEP_DROP_SOLDIER;
		break;
	case PC_ENGINEER:
		varval = g_dropWeapons.integer & WEP_DROP_ENG;
		break;
	case PC_MEDIC:
		varval = g_dropWeapons.integer & WEP_DROP_MEDIC;
		break;
	case PC_LT:
		varval = g_dropWeapons.integer & WEP_DROP_LT;
		break;
	default:
		varval = qfalse;
		break;
	}
	return (varval);
}

/*
===========
Global sound
===========
*/
void APSound(char* sound) {
	gentity_t* ent;
	gentity_t* te;

	ent = g_entities;

	te = G_TempEntity(ent->s.pos.trBase, EV_GLOBAL_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
	te->r.svFlags |= SVF_BROADCAST;
}

/*
===========
Client sound
===========
*/
void CPSound(gentity_t* ent, char* sound) {
	gentity_t* te;

	te = G_TempEntity(ent->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
	te->s.teamNum = ent->s.clientNum;
}

/*
===========
Global sound with limited range
===========
*/
void APRSound(gentity_t* ent, char* sound) {
	gentity_t* te;

	te = G_TempEntity(ent->r.currentOrigin, EV_GENERAL_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
}

/*
===========
GetClientEntity
===========
*/
gentity_t* GetClientEntity(gentity_t* ent, char* cNum, gentity_t** found)
{
	int clientNum, i;
	qboolean allZeroes = qtrue;
	gentity_t* match;
	*found = NULL;

	for (i = 0; i < strlen(cNum); ++i)
	{
		if (cNum[i] != '0')
		{
			allZeroes = qfalse;
			break;
		}
	}

	if (allZeroes)
	{
		clientNum = 0;
	}
	else
	{
		clientNum = atoi(cNum);
		if (clientNum <= 0 || clientNum >= level.maxclients)
		{
			CP(va("print \"Invalid client number provided: ^3%s\n\"", cNum));
			return *found;
		}
	}

	match = g_entities + clientNum;
	if (!match->inuse || match->client->pers.connected != CON_CONNECTED)
	{
		CP(va("print \"No connected client with client number: ^3%i\n\"", clientNum));
		return *found;
	}

	*found = match;
	return *found;
}

/*
==================
Print colored name
==================
*/
char* TablePrintableColorName(const char* name, int maxlength)
{
	char dirty[MAX_NETNAME];
	char clean[MAX_NETNAME];
	char spaces[MAX_NETNAME] = "";
	int cleanlen;

	Q_strncpyz(dirty, name, sizeof(dirty));

	DecolorString(dirty, clean);

	cleanlen = strlen(clean);

	if (cleanlen > maxlength) {
		int remove = cleanlen - maxlength;
		char* end = dirty + strlen(dirty) - 1;

		while (*end && *(end - 1) && remove) {
			if (*(end - 1) == Q_COLOR_ESCAPE)
				end--;
			else
				remove--;

			end--;
		}

		*++end = 0;
	}
	else if (cleanlen < maxlength) {
		for (; cleanlen < maxlength; cleanlen++)
			strcat(spaces, " ");
	}

	return va("%s%s", dirty, spaces);
}

/*
==================
RTCWPro
GetFileExtension
==================
*/
void GetFileExtension(const char* filename, char* out)
{
	qboolean at_extension;

	at_extension = qfalse;

	while (*filename)
	{
		if (*filename == '.')
		{
			at_extension = qtrue;
		}

		if (at_extension)
		{
			*out++ = *filename;
		}

		filename++;
	}

	*out = 0;
}

/*
==================
RTCWPro
FileExists
==================
*/
qboolean FileExists(char* filename, char* directory, char* expected_extension, qboolean can_have_extension)
{
	char files[MAX_MAPCONFIGSTRINGS];
	char file_exists[MAX_QPATH];
	char* fs_filename, * fs_filepath;
	char file_extension[10];
	int i, filecount;

	GetFileExtension(filename, file_extension);
	if (file_extension[0])
	{
		if (Q_stricmp(file_extension, expected_extension))
		{
			return qfalse;
		}

		Q_strncpyz(file_exists, filename, sizeof(file_exists));

		if (!can_have_extension)
		{
			*strstr(filename, file_extension) = 0;
		}
	}
	else
	{
		Q_strncpyz(file_exists, filename, sizeof(file_exists));
		Q_strcat(file_exists, sizeof(file_exists), expected_extension);
	}

	filecount = trap_FS_GetFileList(directory, expected_extension, files, sizeof(files));
	fs_filepath = files;

	for (i = 0; i < filecount; ++i)
	{
		fs_filename = COM_SkipPath(fs_filepath);

		if (!Q_stricmp(file_exists, fs_filename))
		{
			return qtrue;
		}

		fs_filepath += strlen(fs_filepath) + 1;
	}

	return qfalse;
}

/*
===============
RTCWPro
Credits to S4NDMOD (with some modifications)

G_SpawnEnts
===============
*/
qboolean G_SpawnEnts(gentity_t* ent) {
	char mapName[64];

	trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));

	// Check only SP
	if ((!Q_stricmp(mapName, "assault")) || 
		(!Q_stricmp(mapName, "baseout")) || 
		(!Q_stricmp(mapName, "boss1")) || 
		(!Q_stricmp(mapName, "boss2")) || 
		(!Q_stricmp(mapName, "castle")) || 
		(!Q_stricmp(mapName, "chateau")) || 
		(!Q_stricmp(mapName, "church")) || 
		(!Q_stricmp(mapName, "crypt1")) || 
		(!Q_stricmp(mapName, "crypt2")) || 
		(!Q_stricmp(mapName, "cutscene1")) ||
		(!Q_stricmp(mapName, "cutscene6")) ||
		(!Q_stricmp(mapName, "cutscene9")) ||
		(!Q_stricmp(mapName, "cutscene11")) ||
		(!Q_stricmp(mapName, "cutscene14")) ||
		(!Q_stricmp(mapName, "cutscene19")) ||
		(!Q_stricmp(mapName, "dam")) || 
		(!Q_stricmp(mapName, "dark")) ||
		(!Q_stricmp(mapName, "dig")) || 
		(!Q_stricmp(mapName, "end")) || 
		(!Q_stricmp(mapName, "escape1")) || 
		(!Q_stricmp(mapName, "escape2")) || 
		(!Q_stricmp(mapName, "factory")) || 
		(!Q_stricmp(mapName, "forest")) || 
		(!Q_stricmp(mapName, "norway")) || 
		(!Q_stricmp(mapName, "rocket")) || 
		(!Q_stricmp(mapName, "sfm")) || 
		(!Q_stricmp(mapName, "swf")) || 
		(!Q_stricmp(mapName, "trainyard")) || 
		(!Q_stricmp(mapName, "tram")) || 
		(!Q_stricmp(mapName, "village1")) || 
		(!Q_stricmp(mapName, "village2")) || 
		(!Q_stricmp(mapName, "xlabs")))
	{

		//	Turn MED on map into regular MEDpacks for players to use
		if (!Q_strncmp(ent->classname, "item_armor_", 11) || !Q_strncmp(ent->classname, "item_health_", 12)) {
			ent->classname = "item_health";
			return qtrue;
		}

		//	Turn ammo on map into AMMOpacks for players to use
		if (!Q_strncmp(ent->classname, "ammo_", 5)) {
			ent->classname = "weapon_magicammo";
			return qtrue;
		}

		if (!Q_strncmp(ent->classname, "holdable_", 9)) {
			ent->classname = "item_health";
			return qtrue;
		}

		if ((!strcmp("func_door_rotating", ent->classname)) || (!strcmp("func_door", ent->classname))) {
			ent->wait = FLT_MAX;
		}

		if (!strcmp("trigger_aidoor", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("alarm_box", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("ai_trigger", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("ai_marker", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("ai_soldier", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("trigger_once", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("item_clipboard", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_knife2", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_grenadesmoke", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_smoketrail", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_medic_heal", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_dynamite", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_dynamite2", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_sniperScope", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_mortar", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_class_special", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_arty", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("weapon_medic_syringe", ent->classname)) {
			return qfalse;
		}

		if (!strcmp("shooter_rocket", ent->classname)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "assault")) {
		if ((!strcmp("func_door_rotating", ent->classname)) && (ent->r.currentOrigin[0] == -4510) && 
			(ent->r.currentOrigin[1] == 4616) && (ent->r.currentOrigin[2] == 664)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "baseout")) {
		if (!strcmp("func_door_rotating", ent->classname)) {

			if ((ent->r.currentOrigin[0] == -2064) && (ent->r.currentOrigin[1] == 2286) && (ent->r.currentOrigin[2] == 280)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 282) && (ent->r.currentOrigin[1] == 672) && (ent->r.currentOrigin[2] == 53)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 6) && (ent->r.currentOrigin[1] == 672) && (ent->r.currentOrigin[2] == 53)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "boss1")) {
		if (!strcmp("func_door", ent->classname)) {
			return qfalse;
		}
		if ((!strcmp("func_explosive", ent->classname)) && (ent->spawnflags != 4)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "boss2")) {
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == 574) && (ent->r.currentOrigin[1] == 968) && (ent->r.currentOrigin[2] == 88)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 514) && (ent->r.currentOrigin[1] == 1464) && (ent->r.currentOrigin[2] == 88)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "castle")) {
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == 1232) && (ent->r.currentOrigin[1] == 1950) && (ent->r.currentOrigin[2] == 336)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 1158) && (ent->r.currentOrigin[1] == 1950) && (ent->r.currentOrigin[2] == 336)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 1158) && (ent->r.currentOrigin[1] == 2117) && (ent->r.currentOrigin[2] == 100)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 1018) && (ent->r.currentOrigin[1] == 2117) && (ent->r.currentOrigin[2] == 100)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "chateau")) {
		if (!strcmp("func_explosive", ent->classname)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "church")) {
		if ((!strcmp("func_explosive", ent->classname)) && (ent->spawnflags == 20)) {
			return qfalse;
		}
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == -480) && (ent->r.currentOrigin[1] == -158) && (ent->r.currentOrigin[2] == 704)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -384) && (ent->r.currentOrigin[1] == -158) && (ent->r.currentOrigin[2] == 1152)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -384) && (ent->r.currentOrigin[1] == 734) && (ent->r.currentOrigin[2] == 1152)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "crypt1")) {
		if (!strcmp("func_explosive", ent->classname)) {
			return qfalse;
		}
		if (!strcmp("trigger_hurt", ent->classname)) {
			return qfalse;
		}
		if (!strcmp("func_door", ent->classname)) {
			return qfalse;
		}
		if (!strcmp("func_invisible_user", ent->classname)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "crypt2")) {
		if (!strcmp("func_explosive", ent->classname)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "dam")) {
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == -1340) && (ent->r.currentOrigin[1] == 5528) && (ent->r.currentOrigin[2] == 2404)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -1576) && (ent->r.currentOrigin[1] == 5528) && (ent->r.currentOrigin[2] == 2404)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -1604) && (ent->r.currentOrigin[1] == 5186) && (ent->r.currentOrigin[2] == 2392)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "dark")) {
		if (!strcmp("func_door_rotating", ent->classname)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "escape1")) {
		if (!strcmp("func_invisible_user", ent->classname)) {
			return qfalse;
		}
		if ((!strcmp("func_door", ent->classname)) && (ent->spawnflags == 7)) {
			return qfalse;
		}
		if ((!strcmp("func_explosive", ent->classname)) && (ent->spawnflags == 4)) {
			return qfalse;
		}
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == 264) && (ent->r.currentOrigin[1] == 129) && (ent->r.currentOrigin[2] == -504)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 432) && (ent->r.currentOrigin[1] == 513) && (ent->r.currentOrigin[2] == -504)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 600) && (ent->r.currentOrigin[1] == 575) && (ent->r.currentOrigin[2] == -504)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -190) && (ent->r.currentOrigin[1] == 416) && (ent->r.currentOrigin[2] == 536)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -190) && (ent->r.currentOrigin[1] == 416) && (ent->r.currentOrigin[2] == 752)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "factory")) {
		if ((!strcmp("func_door_rotating", ent->classname)) && (ent->r.currentOrigin[0] == 1400) && 
			(ent->r.currentOrigin[1] == 162) && (ent->r.currentOrigin[2] == 56)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "forest")) {
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == -3492) && (ent->r.currentOrigin[1] == -434) && (ent->r.currentOrigin[2] == 264)) {
				return qtrue;
			}
			else {
				return qfalse;
			}
		}
		if (!strcmp("func_invisible_user", ent->classname)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "norway")) {
		if (!strcmp("func_invisible_user", ent->classname)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "rocket")) {
		if (!strcmp("func_invisible_user", ent->classname)) {
			return qfalse;
		}
		if (!strcmp("func_door_rotating", ent->classname)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "sfm")) {
		if (!strcmp("func_invisible_user", ent->classname)) {
			return qfalse;
		}
		if ((!strcmp("func_door_rotating", ent->classname)) && (ent->r.currentOrigin[0] == 478) && 
			(ent->r.currentOrigin[1] == -576) && (ent->r.currentOrigin[2] == -64)) {
			return qfalse;
		}
	}

	if (!Q_stricmp(mapName, "swf")) {
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == 1986) && (ent->r.currentOrigin[1] == -192) && (ent->r.currentOrigin[2] == 544)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 2368) && (ent->r.currentOrigin[1] == -514) && (ent->r.currentOrigin[2] == 548)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 1440) && (ent->r.currentOrigin[1] == -1090) && (ent->r.currentOrigin[2] == 544)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 1440) && (ent->r.currentOrigin[1] == -1214) && (ent->r.currentOrigin[2] == 544)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 354) && (ent->r.currentOrigin[1] == -1288) && (ent->r.currentOrigin[2] == 544)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -98) && (ent->r.currentOrigin[1] == 16) && (ent->r.currentOrigin[2] == 544)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -222) && (ent->r.currentOrigin[1] == 16) && (ent->r.currentOrigin[2] == 544)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 384) && (ent->r.currentOrigin[1] == -732) && (ent->r.currentOrigin[2] == 484)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 384) && (ent->r.currentOrigin[1] == -548) && (ent->r.currentOrigin[2] == 484)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 786) && (ent->r.currentOrigin[1] == 168) && (ent->r.currentOrigin[2] == 376)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "trainyard")) {
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == -1480) && (ent->r.currentOrigin[1] == 606) && (ent->r.currentOrigin[2] == 120)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 1904) && (ent->r.currentOrigin[1] == 382) && (ent->r.currentOrigin[2] == -192)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 1406) && (ent->r.currentOrigin[1] == 80) && (ent->r.currentOrigin[2] == -192)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 3522) && (ent->r.currentOrigin[1] == 200) && (ent->r.currentOrigin[2] == 72)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 2120) && (ent->r.currentOrigin[1] == -610) && (ent->r.currentOrigin[2] == 72)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "tram")) {
		if (!strcmp("trigger_hurt", ent->classname)) {
			ent->classname = "trigger_push";
		}
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == -252) && (ent->r.currentOrigin[1] == 318) && (ent->r.currentOrigin[2] == 504)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -2718) && (ent->r.currentOrigin[1] == 36) && (ent->r.currentOrigin[2] == -264)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -3784) && (ent->r.currentOrigin[1] == -1598) && (ent->r.currentOrigin[2] == -576)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -3784) && (ent->r.currentOrigin[1] == -1474) && (ent->r.currentOrigin[2] == -576)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "village1")) {
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == -1040) && (ent->r.currentOrigin[1] == 130) && (ent->r.currentOrigin[2] == -128)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -880) && (ent->r.currentOrigin[1] == 130) && (ent->r.currentOrigin[2] == -128)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 610) && (ent->r.currentOrigin[1] == 1920) && (ent->r.currentOrigin[2] == 68)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 236) && (ent->r.currentOrigin[1] == 2338) && (ent->r.currentOrigin[2] == -128)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 64) && (ent->r.currentOrigin[1] == -670) && (ent->r.currentOrigin[2] == -128)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 318) && (ent->r.currentOrigin[1] == -324) && (ent->r.currentOrigin[2] == -128)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "village2")) {
		if (!strcmp("func_door_rotating", ent->classname)) {
			if ((ent->r.currentOrigin[0] == 2808) && (ent->r.currentOrigin[1] == -66) && (ent->r.currentOrigin[2] == 8)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == 1024) && (ent->r.currentOrigin[1] == -799) && (ent->r.currentOrigin[2] == -64)) {
				return qfalse;
			}
			if ((ent->r.currentOrigin[0] == -968) && (ent->r.currentOrigin[1] == 705) && (ent->r.currentOrigin[2] == 2)) {
				return qfalse;
			}
		}
	}

	if (!Q_stricmp(mapName, "xlabs")) {
		if (!strcmp("trigger_hurt", ent->classname)) {
			return qfalse;
		}
		if (!strcmp("func_invisible_user", ent->classname)) {
			return qfalse;
		}
		if ((!strcmp("func_explosive", ent->classname)) && (ent->spawnflags == 4)) {
			return qfalse;
		}
		if ((!strcmp("func_door_rotating", ent->classname)) && (ent->r.currentOrigin[0] == 1500) && 
			(ent->r.currentOrigin[1] == 80) && (ent->r.currentOrigin[2] == 48)) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
==================
Nihi - check for matching maps

RTCWPro
G_FindMatchingMaps
==================
*/
int G_FindMatchingMaps(gentity_t* ent, char* mapName) {
	int numMatches = 0;
	int mapIndex = -1;
	int i;

	for (i = 0; i <= level.mapcount + 1; i++)
	{
		if (strstr(level.maplist[i], mapName) != NULL)
		{
			if (numMatches > MAX_MAP_MATCHES)
			{
				CP(va("print \"^3Too many matches found. Narrow your search!\n"));
				break;
			}

			if (numMatches == 0)
			{
				mapIndex = i;
			}
			else if (numMatches == 1)
			{
				CP(va("print \"^3Multiple matches found:\n"));
				CP(va("print \"^7  %s\n\"", level.maplist[mapIndex]));
				CP(va("print \"^7  %s\n\"", level.maplist[i]));
			}
			else if (numMatches > 1)
			{
				CP(va("print \"^7  %s\n\"", level.maplist[i]));
			}

			numMatches += 1;
		}

		if (Q_stricmp(level.maplist[i], mapName) == 0)
		{
			mapIndex = i;
			numMatches = 1; // found exact match...will clean this all up later
			break;
		}
	}

	if (numMatches == 1)
	{
		return mapIndex;
	}
	else if (numMatches > 1)
	{
		return -1;
	}
	else
	{
		CP(va("print \"^3%s ^7is not on the server.\n\"", mapName));
		return -1;
	}
}

