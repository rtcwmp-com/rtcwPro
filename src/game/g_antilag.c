/*
===========================================================================

- L0

Removed default antilag and ported a new one from 1.0 based on unlagged source and few tweaks from s4ndmod.

Last change: 12 Jan / 2013
=======================================
*/

#include "g_local.h"

// OSP
#define IS_ACTIVE( x ) ( \
		x->r.linked == qtrue &&	\
		x->client->ps.stats[STAT_HEALTH] > 0 &&	\
		x->client->sess.sessionTeam != TEAM_SPECTATOR && \
		( x->client->ps.pm_flags & PMF_LIMBO ) == 0	\
		)

/*
============
G_ResetTrail

Clear out the given client's origin trails (should be called from ClientBegin and when
the teleport bit is toggled)
============
*/
void G_ResetTrail( gentity_t *ent ) {
	int		i, time;

	// fill up the origin trails with data (assume the current position
	// for the last 1/2 second or so)
	ent->client->trailHead = NUM_CLIENT_TRAILS - 1;
	for ( i = ent->client->trailHead, time = level.time; i >= 0; i--, time -= 50 ) {
		VectorCopy( ent->r.mins, ent->client->trail[i].mins );
		VectorCopy( ent->r.maxs, ent->client->trail[i].maxs );
		VectorCopy( ent->r.currentOrigin, ent->client->trail[i].currentOrigin );
		ent->client->trail[i].leveltime = time;
		ent->client->trail[i].time = time;
	}
}


/*
============
G_StoreTrail

Keep track of where the client's been (usually called every ClientThink)
============
*/
void G_StoreTrail( gentity_t *ent ) {
	int		head, newtime;

	// L0 - OSP port
	if ( !IS_ACTIVE( ent ) )
		return;

	head = ent->client->trailHead;

	// if we're on a new frame
	if ( ent->client->trail[head].leveltime < level.time ) {
		// snap the last head up to the end of frame time
		ent->client->trail[head].time = level.previousTime;

		// increment the head
		ent->client->trailHead++;
		if ( ent->client->trailHead >= NUM_CLIENT_TRAILS ) {
			ent->client->trailHead = 0;
		}
		head = ent->client->trailHead;
	}

	if ( ent->r.svFlags & SVF_BOT ) {
		// bots move only once per frame
		newtime = level.time;
	} else {
		// calculate the actual server time
		// (we set level.frameStartTime every G_RunFrame)
		newtime = level.previousTime + trap_Milliseconds() - level.frameStartTime;
		if ( newtime > level.time ) {
			newtime = level.time;
		} else if ( newtime <= level.previousTime ) {
			newtime = level.previousTime + 1;
		}
	}

	// store all the collision-detection info and the time
	VectorCopy( ent->r.mins, ent->client->trail[head].mins );
	VectorCopy( ent->r.maxs, ent->client->trail[head].maxs );
	VectorCopy( ent->r.currentOrigin, ent->client->trail[head].currentOrigin );
	ent->client->trail[head].leveltime = level.time;
	ent->client->trail[head].time = newtime;
}


/*
=============
TimeShiftLerp

Used below to interpolate between two previous vectors
Returns a vector "frac" times the distance between "start" and "end"
=============
*/
static void TimeShiftLerp( float frac, vec3_t start, vec3_t end, vec3_t result ) {
	float	comp = 1.0f - frac;

	result[0] = frac * start[0] + comp * end[0];
	result[1] = frac * start[1] + comp * end[1];
	result[2] = frac * start[2] + comp * end[2];
}


/*
=================
G_TimeShiftClient

Move a client back to where he was at the specified "time"
=================
*/
void G_TimeShiftClient( gentity_t *ent, int time ) {
	int		j, k;

	if ( time > level.time ) {
		time = level.time;
	}

	// find two entries in the origin trail whose times sandwich "time"
	// assumes no two adjacent trail records have the same timestamp
	j = k = ent->client->trailHead;
	do {
		if ( ent->client->trail[j].time <= time )
			break;

		k = j;
		j--;
		if ( j < 0 ) {
			j = NUM_CLIENT_TRAILS - 1;
		}
	}
	while ( j != ent->client->trailHead );

	// if we got past the first iteration above, we've sandwiched (or wrapped)
	if ( j != k ) {
		// make sure it doesn't get re-saved
		if ( ent->client->saved.leveltime != level.time ) {
			// save the current origin and bounding box
			VectorCopy( ent->r.mins, ent->client->saved.mins );
			VectorCopy( ent->r.maxs, ent->client->saved.maxs );
			VectorCopy( ent->r.currentOrigin, ent->client->saved.currentOrigin );
			ent->client->saved.leveltime = level.time;
		}

		// if we haven't wrapped back to the head, we've sandwiched, so
		// we shift the client's position back to where he was at "time"
		if ( j != ent->client->trailHead ) {
			float	frac = (float)(ent->client->trail[k].time - time) /
				(float)(ent->client->trail[k].time - ent->client->trail[j].time);

			// interpolate between the two origins to give position at time index "time"
			TimeShiftLerp( frac,
				ent->client->trail[k].currentOrigin, ent->client->trail[j].currentOrigin,
				ent->r.currentOrigin );

			// lerp these too, just for fun (and ducking)
			TimeShiftLerp( frac,
				ent->client->trail[k].mins, ent->client->trail[j].mins,
				ent->r.mins );

			TimeShiftLerp( frac,
				ent->client->trail[k].maxs, ent->client->trail[j].maxs,
				ent->r.maxs );

			// this will recalculate absmin and absmax
			trap_LinkEntity( ent );
		} else {
			// we wrapped, so grab the earliest
			VectorCopy( ent->client->trail[k].currentOrigin, ent->r.currentOrigin );
			VectorCopy( ent->client->trail[k].mins, ent->r.mins );
			VectorCopy( ent->client->trail[k].maxs, ent->r.maxs );

			// this will recalculate absmin and absmax
			trap_LinkEntity( ent );
		}
	}
}


/*
=====================
G_TimeShiftAllClients

Move ALL clients back to where they were at the specified "time",
except for "skip"
=====================
*/
void G_TimeShiftAllClients( int time, gentity_t *skip ) {
	int			i;
	gentity_t	*ent;

	if ( time > level.time ) {
		time = level.time;
	}

	// for every client
	ent = &g_entities[0];
	for ( i = 0; i < MAX_CLIENTS; i++, ent++ ) {
		if ( ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip ) {
			if(!(ent->client->ps.pm_flags & PMF_LIMBO)){
				G_TimeShiftClient( ent, time );
			}
		}
	}
}


/*
===================
G_UnTimeShiftClient

Move a client back to where he was before the time shift
===================
*/
void G_UnTimeShiftClient( gentity_t *ent ) {
	// if it was saved
	if ( ent->client->saved.leveltime == level.time ) {
		// move it back
		VectorCopy( ent->client->saved.mins, ent->r.mins );
		VectorCopy( ent->client->saved.maxs, ent->r.maxs );
		VectorCopy( ent->client->saved.currentOrigin, ent->r.currentOrigin );
		ent->client->saved.leveltime = 0;

		// this will recalculate absmin and absmax
		trap_LinkEntity( ent );
	}
}

/*
=======================
G_UnTimeShiftAllClients

Move ALL the clients back to where they were before the time shift,
except for "skip"
=======================
*/
void G_UnTimeShiftAllClients( gentity_t *skip ) {
	int			i;
	gentity_t	*ent;

	ent = &g_entities[0];
	for ( i = 0; i < MAX_CLIENTS; i++, ent++) {
		if ( ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip ) {
			if(!(ent->client->ps.pm_flags & PMF_LIMBO)){
				G_UnTimeShiftClient( ent );
			}
		}
	}
}




