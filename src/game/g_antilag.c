/*
===========================================================================
L0 / rtcwPub :: g_antilag.c

	Antilag functionality.
	Forks: Unfreeze -> s4ndmod -> wolfX -> main (1.4) and all in between.

Created: 24. Mar / 2014
===========================================================================
*/

#include "g_local.h"

/*
sswolf - antilag refactor
Source: Nobo
*/

#ifdef ANTILAG_REFACTOR

/*
============
IsActiveClient

Is the entity a client that's currently playing in the world (active)?
============
*/
qboolean IsActiveClient(gentity_t* ent) {
	return
		ent->r.linked == qtrue &&
		ent->client &&
		ent->client->ps.stats[STAT_HEALTH] > 0 &&
		ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
		(ent->client->ps.pm_flags & PMF_LIMBO) == 0;
}

/*
============
G_ResetTrail

Resets the given client's trail (should be called from ClientBegin and when the teleport bit is toggled)
Each trail node is populated using the client's current state within the server.
============
*/
void G_ResetTrail(gentity_t* ent) {
	int	i, time;

	// we want to store half a second worth of trails (500ms)
	const int trail_time_range_ms = 500;
	const int time_interval = round((double)trail_time_range_ms / (double)NUM_CLIENT_TRAILS);

	// fill up the origin trails with data (assume the current position
	// for the last 1/2 second or so)
	ent->client->trailHead = NUM_CLIENT_TRAILS - 1;

	for (i = ent->client->trailHead, time = level.time; i >= 0; i--, time -= time_interval)
	{
		VectorCopy(ent->r.mins, ent->client->trail[i].mins);
		VectorCopy(ent->r.maxs, ent->client->trail[i].maxs);
		VectorCopy(ent->r.currentOrigin, ent->client->trail[i].currentOrigin);
		ent->client->trail[i].time = time;
		ent->client->trail[i].animInfo = ent->client->animationInfo;
	}
}


/*
============
G_StoreTrail

Store the client's current positional information (usually called every ClientThink)
============
*/
void G_StoreTrail(gentity_t* ent) {
	int newtime;

	// only store trail nodes for actively playing clients.
	// also, don't store if the level time hasn't been set yet (it'll happen next SV_Frame).
	if (!IsActiveClient(ent) || !level.time || !level.previousTime)
	{
		return;
	}

	// 6ms is the minimum frame time for 125fps clients (average is 8ms).
#define MINIMUM_TIME_BETWEEN_NODES 6

	// limit how often higher fps clients store trail nodes.
	// otherwise we lose too much time-sensitive data that's required for higher ping players.
	int time_since_last_store = ent->client->pers.cmd.serverTime - ent->client->last_trail_node_store_time;
	ent->client->accum_trail_node_store_time += time_since_last_store;
	ent->client->last_trail_node_store_time = ent->client->pers.cmd.serverTime;

	if (ent->client->accum_trail_node_store_time < MINIMUM_TIME_BETWEEN_NODES)
	{
		return;
	}
	ent->client->accum_trail_node_store_time = 0;

	// increment the head
	ent->client->trailHead++;

	if (ent->client->trailHead >= NUM_CLIENT_TRAILS)
	{
		ent->client->trailHead = 0;
	}

	if (ent->r.svFlags & SVF_BOT)
	{
		// bots move only once per server frame (every 1000/sv_fps ms)
		newtime = level.time;
	}
	else
	{
		// level.frameStartTime is set to trap_Milliseconds() within G_RunFrame.
		//
		// we want to store where the server thinks the client is after receiving and processing
		// one of their usercmd packets (move command). trap_Milliseconds() is used for a more granular timestamp,
		// since level.time is only ever incremented every 50-ish milliseconds (depends on sv_fps).
		// 
		// if level.time were used then clients with high fps, high maxpackets, and high rate would have
		// many trail nodes with duplicate timestamps.
		int realTimeSinceFrameStartTime = trap_Milliseconds() - level.frameStartTime;
		newtime = level.previousTime + realTimeSinceFrameStartTime;
	}

	// store all the collision-detection info and the time
	clientTrail_t* trail_node = &ent->client->trail[ent->client->trailHead];
	VectorCopy(ent->r.mins, trail_node->mins);
	VectorCopy(ent->r.maxs, trail_node->maxs);
	VectorCopy(ent->r.currentOrigin, trail_node->currentOrigin);
	trail_node->time = newtime;
	trail_node->animInfo = ent->client->animationInfo;
}

/*
=================
Interpolate

Interpolates along two vectors (start -> end).
=================
*/
void Interpolate(float frac, vec3_t start, vec3_t end, vec3_t out) {
	float comp = 1.0f - frac;

	out[0] = start[0] * frac + end[0] * comp;
	out[1] = start[1] * frac + end[1] * comp;
	out[2] = start[2] * frac + end[2] * comp;
}

/*
=================
G_TimeShiftClient

Shifts a client back to where he was at the specified "time"
=================
*/
void G_TimeShiftClient(gentity_t* ent, int time) {
	int	j, k;
	qboolean found_trail_nodes_that_sandwich_time;

	// this prevents looping through every trail node if we know none of them are <= time.
	// the trail nodes are "sorted" by time, so if the oldest one isn't <= time, then none of them can be.
	if (ent->client->trail[(ent->client->trailHead + 1) & (NUM_CLIENT_TRAILS - 1)].time > time) {
		return;
	}

	// find two trail nodes in the trail whose times sandwich "time".
	// this will check every trail node, even if the head starts at index 0.. it'll wrap around to NUM_CLIENT_TRAIL_NODES - 1 and decrease from there.
	found_trail_nodes_that_sandwich_time = qfalse;
	j = k = ent->client->trailHead;

	do
	{
		if (ent->client->trail[j].time <= time)
		{
			found_trail_nodes_that_sandwich_time = j != k;
			break;
		}

		k = j;
		j--;

		if (j < 0)
		{
			j = NUM_CLIENT_TRAILS - 1;
		}
	} 	while (j != ent->client->trailHead);

	memset(&ent->client->saved_trail_node, 0, sizeof(clientTrail_t));

	// we've found two trail nodes with times that "sandwich" the passed in "time"
	if (found_trail_nodes_that_sandwich_time)
	{
		// save the current origin, bounding box and animation info; used to untimeshift the client once collision detection is complete
		VectorCopy(ent->r.mins, ent->client->saved_trail_node.mins);
		VectorCopy(ent->r.maxs, ent->client->saved_trail_node.maxs);
		VectorCopy(ent->r.currentOrigin, ent->client->saved_trail_node.currentOrigin);
		ent->client->saved_trail_node.animInfo = ent->client->animationInfo;

		// calculate a fraction that will be used to shift the client's position back to the trail node that's nearest to "time"
		float frac = (float)(time - ent->client->trail[j].time) / (float)(ent->client->trail[k].time - ent->client->trail[j].time);

		// find the "best" origin between the sandwiching trail nodes via interpolation
		Interpolate(frac, ent->client->trail[j].currentOrigin, ent->client->trail[k].currentOrigin, ent->r.currentOrigin);
		// find the "best" mins & maxs (crouching/standing).
		// it doesn't make sense to interpolate mins and maxs. the server either thinks the client
		// is crouching or not, and updates the mins & maxs immediately. there's no inbetween.
		int nearest_trail_node_index = frac < 0.5 ? j : k;
		VectorCopy(ent->client->trail[nearest_trail_node_index].mins, ent->r.mins);
		VectorCopy(ent->client->trail[nearest_trail_node_index].maxs, ent->r.maxs);
		// use the trail node's animation info that's nearest "time" (for head hitbox).
		// the current server animation code used for head hitboxes doesn't support interpolating
		// between two different animation frames (i.e. crouch -> standing animation), so can't interpolate here either.
		ent->client->animationInfo = ent->client->trail[nearest_trail_node_index].animInfo;

		// this will recalculate absmin and absmax
		trap_LinkEntity(ent);
	}
}


/*
=====================
G_TimeShiftAllClients

Move ALL clients back to where they were at the specified "time",
except for "skip"
=====================
*/
void G_TimeShiftAllClients(int time, gentity_t* skip) {
	int			i;
	gentity_t* ent;

	// don't shift clients if "skip" (the client that's trying to timeshift everyone) is more than 500ms behind the current server time (very laggy).
	if (level.time - time > 500)
	{
		return;
	}

	// shift every client thats:
	// not a spectator, not the "timeshifter" (skip), and not in limbo.
	ent = &g_entities[0];

	for (i = 0; i < MAX_CLIENTS; i++, ent++)
	{
		if (ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip)
		{
			if (!(ent->client->ps.pm_flags & PMF_LIMBO))
			{
				G_TimeShiftClient(ent, time);
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
void G_UnTimeShiftClient(gentity_t* ent) {

	// if ent was time shifted
	if (ent->client->saved_trail_node.mins[0])
	{
		// revert the time shift
		VectorCopy(ent->client->saved_trail_node.mins, ent->r.mins);
		VectorCopy(ent->client->saved_trail_node.maxs, ent->r.maxs);
		VectorCopy(ent->client->saved_trail_node.currentOrigin, ent->r.currentOrigin);
		ent->client->animationInfo = ent->client->saved_trail_node.animInfo;

		// this will recalculate absmin and absmax
		trap_LinkEntity(ent);
	}
}

/*
=======================
G_UnTimeShiftAllClients

Move ALL the clients back to where they were before the time shift,
except for "skip"
=======================
*/
void G_UnTimeShiftAllClients(gentity_t* skip) {
	int			i;
	gentity_t* ent;

	// unshift every client thats:
	// not a spectator, not the "timeshifter" (skip), and not in limbo.
	ent = &g_entities[0];

	for (i = 0; i < MAX_CLIENTS; i++, ent++)
	{
		if (ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip)
		{
			if (!(ent->client->ps.pm_flags & PMF_LIMBO))
			{
				G_UnTimeShiftClient(ent);
			}
		}
	}
}

#else

// OSP 
#define IS_ACTIVE( x ) ( \
	x->r.linked == qtrue &&	\
	x->client->ps.stats[STAT_HEALTH] > 0 && \
	x->client->sess.sessionTeam != TEAM_SPECTATOR && \
	(x->client->ps.pm_flags & PMF_LIMBO) == 0	\
	)

/*
============
G_ResetTrail

Clear out the given client's origin trails (should be called from ClientBegin and when
the teleport bit is toggled)
============
*/
void G_ResetTrail(gentity_t* ent) {
	int		i, time;

	// fill up the origin trails with data (assume the current position
	// for the last 1/2 second or so)
	ent->client->trailHead = NUM_CLIENT_TRAILS - 1;
	for (i = ent->client->trailHead, time = level.time; i >= 0; i--, time -= 50) {
		VectorCopy(ent->r.mins, ent->client->trail[i].mins);
		VectorCopy(ent->r.maxs, ent->client->trail[i].maxs);
		VectorCopy(ent->r.currentOrigin, ent->client->trail[i].currentOrigin);
		ent->client->trail[i].leveltime = time;
		ent->client->trail[i].time = time;
		ent->client->trail[i].animInfo = ent->client->animationInfo;
	}
}

/*
============
G_StoreTrail

Keep track of where the client's been (usually called every ClientThink)
============
*/
void G_StoreTrail(gentity_t* ent) {
	int		head, newtime;

	if (!IS_ACTIVE(ent))
		return;

	head = ent->client->trailHead;

	// if we're on a new frame
	if (ent->client->trail[head].leveltime < level.time) {
		// snap the last head up to the end of frame time
		ent->client->trail[head].time = level.previousTime;

		// increment the head
		ent->client->trailHead++;
		if (ent->client->trailHead >= NUM_CLIENT_TRAILS) {
			ent->client->trailHead = 0;
		}
		head = ent->client->trailHead;
	}

	if (ent->r.svFlags & SVF_BOT) {
		// bots move only once per frame
		newtime = level.time;
	}
	else {
		// calculate the actual server time
		// (we set level.frameStartTime every G_RunFrame)
		newtime = level.previousTime + trap_Milliseconds() - level.frameStartTime;
		if (newtime > level.time) {
			newtime = level.time;
		}
		else if (newtime <= level.previousTime) {
			newtime = level.previousTime + 1;
		}
	}

	// store all the collision-detection info and the time
	VectorCopy(ent->r.mins, ent->client->trail[head].mins);
	VectorCopy(ent->r.maxs, ent->client->trail[head].maxs);
	VectorCopy(ent->r.currentOrigin, ent->client->trail[head].currentOrigin);
	ent->client->trail[head].leveltime = level.time;
	ent->client->trail[head].time = newtime;
	ent->client->trail[head].animInfo = ent->client->animationInfo;
}

/*
=============
TimeShiftLerp

Used below to interpolate between two previous vectors
Returns a vector "frac" times the distance between "start" and "end"
=============
*/
static void TimeShiftLerp(float frac, vec3_t start, vec3_t end, vec3_t result) {
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
void G_TimeShiftClient(gentity_t* ent, int time) {
	int		j, k;

	if (time > level.time) {
		time = level.time;
	}

	// find two entries in the origin trail whose times sandwich "time"
	// assumes no two adjacent trail records have the same timestamp
	j = k = ent->client->trailHead;
	do {
		if (ent->client->trail[j].time <= time)
			break;

		k = j;
		j--;
		if (j < 0) {
			j = NUM_CLIENT_TRAILS - 1;
		}
	} while (j != ent->client->trailHead);

	// if we got past the first iteration above, we've sandwiched (or wrapped)
	if (j != k) {
		// make sure it doesn't get re-saved
		if (ent->client->saved.leveltime != level.time) {
			// save the current origin and bounding box
			VectorCopy(ent->r.mins, ent->client->saved.mins);
			VectorCopy(ent->r.maxs, ent->client->saved.maxs);
			VectorCopy(ent->r.currentOrigin, ent->client->saved.currentOrigin);
			ent->client->saved.leveltime = level.time;
			ent->client->saved.animInfo = ent->client->animationInfo;
		}

		// if we haven't wrapped back to the head, we've sandwiched, so
		// we shift the client's position back to where he was at "time"
		if (j != ent->client->trailHead) {
			float	frac = (float)(ent->client->trail[k].time - time) /
				(float)(ent->client->trail[k].time - ent->client->trail[j].time);

			// interpolate between the two origins to give position at time index "time"
			TimeShiftLerp(frac,
				ent->client->trail[k].currentOrigin, ent->client->trail[j].currentOrigin,
				ent->r.currentOrigin);

			// lerp these too, just for fun (and ducking)
			TimeShiftLerp(frac,
				ent->client->trail[k].mins, ent->client->trail[j].mins,
				ent->r.mins);

			TimeShiftLerp(frac,
				ent->client->trail[k].maxs, ent->client->trail[j].maxs,
				ent->r.maxs);

			ent->client->animationInfo = ent->client->trail[frac <= 0.5f ? k : j].animInfo;

			// this will recalculate absmin and absmax
			trap_LinkEntity(ent);
		}
		else {
			// we wrapped, so grab the earliest
			VectorCopy(ent->client->trail[k].currentOrigin, ent->r.currentOrigin);
			VectorCopy(ent->client->trail[k].mins, ent->r.mins);
			VectorCopy(ent->client->trail[k].maxs, ent->r.maxs);
			ent->client->animationInfo = ent->client->trail[k].animInfo;

			// this will recalculate absmin and absmax
			trap_LinkEntity(ent);
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
void G_TimeShiftAllClients(int time, gentity_t* skip) {
	int			i;
	gentity_t* ent;

	if (time > level.time) {
		time = level.time;
	}

	// for every client
	ent = &g_entities[0];
	for (i = 0; i < MAX_CLIENTS; i++, ent++) {
		if (ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip) {
			if (!(ent->client->ps.pm_flags & PMF_LIMBO)) {
				G_TimeShiftClient(ent, time);
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
void G_UnTimeShiftClient(gentity_t* ent) {
	// if it was saved
	if (ent->client->saved.leveltime == level.time) {
		// move it back
		VectorCopy(ent->client->saved.mins, ent->r.mins);
		VectorCopy(ent->client->saved.maxs, ent->r.maxs);
		VectorCopy(ent->client->saved.currentOrigin, ent->r.currentOrigin);
		ent->client->saved.leveltime = 0;
		ent->client->animationInfo = ent->client->saved.animInfo;

		// this will recalculate absmin and absmax
		trap_LinkEntity(ent);
	}
}

/*
=======================
G_UnTimeShiftAllClients

Move ALL the clients back to where they were before the time shift,
except for "skip"
=======================
*/
void G_UnTimeShiftAllClients(gentity_t* skip) {
	int			i;
	gentity_t* ent;

	ent = &g_entities[0];
	for (i = 0; i < MAX_CLIENTS; i++, ent++) {
		if (ent->client && ent->inuse && ent->client->sess.sessionTeam < TEAM_SPECTATOR && ent != skip) {
			if (!(ent->client->ps.pm_flags & PMF_LIMBO)) {
				G_UnTimeShiftClient(ent);
			}
		}
	}
}
#endif

// sswolf - unused for now
#if 0

void G_AttachBodyParts(gentity_t* ent) {
	int i;
	gentity_t   *list;

	for (i = 0; i < level.numConnectedClients; i++) {
		list = g_entities + level.sortedClients[i];
		list->client->tempHead = (list != ent && IS_ACTIVE(list)) ? G_BuildHead(list) : NULL;
	}
}

void G_DettachBodyParts(void) {
	int i;
	gentity_t   *list;

	for (i = 0; i < level.numConnectedClients; i++) {
		list = g_entities + level.sortedClients[i];
		if (list->client->tempHead != NULL) {
			G_FreeEntity(list->client->tempHead);
		}
	}
}

int G_SwitchBodyPartEntity(gentity_t* ent) {
	if (ent->s.eType == ET_TEMPHEAD) {
		return ent->parent - g_entities;
	}
	return ent - g_entities;
}

#define POSITION_READJUST										\
	if (res != results->entityNum) {							\
		VectorSubtract(end, start, dir);						\
		VectorNormalizeFast(dir);								\
																\
		VectorMA(results->endpos, -1, dir, results->endpos);	\
		results->entityNum = res;								\
	}

// Run a trace with players in historical positions.
void G_HistoricalTrace(gentity_t* ent, trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask) {
	int res;
	vec3_t dir;

	G_AttachBodyParts(ent);
	trap_Trace(results, start, mins, maxs, end, passEntityNum, contentmask);

	res = G_SwitchBodyPartEntity(&g_entities[results->entityNum]);
	POSITION_READJUST

		G_DettachBodyParts();
	return;
}
#endif

