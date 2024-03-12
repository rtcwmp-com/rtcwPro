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



// cg_predict.c -- this file generates cg.predictedPlayerState by either
// interpolating between snapshots from the server or locally predicting
// ahead the client's movement.
// It also handles local physics interaction, like fragments bouncing off walls

#include "cg_local.h"

static pmove_t cg_pmove;

static int cg_numSolidEntities;
static centity_t   *cg_solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
static int cg_numTriggerEntities;
static centity_t   *cg_triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];

/*
====================
CG_BuildSolidList

When a new cg.snap has been set, this function builds a sublist
of the entities that are actually solid, to make for more
efficient collision detection
====================
*/
void CG_BuildSolidList( void ) {
	int i;
	centity_t   *cent;
	snapshot_t  *snap;
	entityState_t   *ent;

	cg_numSolidEntities = 0;
	cg_numTriggerEntities = 0;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	for ( i = 0 ; i < snap->numEntities ; i++ ) {
		cent = &cg_entities[ snap->entities[ i ].number ];
		ent = &cent->currentState;

		// RF, dont clip again non-solid bmodels
		if ( cent->nextState.solid == SOLID_BMODEL && ( cent->nextState.eFlags & EF_NONSOLID_BMODEL ) ) {
			continue;
		}

		if ( ent->eType == ET_ITEM || ent->eType == ET_PUSH_TRIGGER || ent->eType == ET_TELEPORT_TRIGGER
			 || ent->eType == ET_CONCUSSIVE_TRIGGER || ent->eType == ET_OID_TRIGGER ) { // JPW NERVE
			cg_triggerEntities[cg_numTriggerEntities] = cent;
			cg_numTriggerEntities++;
			continue;
		}

		if ( cent->nextState.solid ) {
			cg_solidEntities[cg_numSolidEntities] = cent;
			cg_numSolidEntities++;
			continue;
		}
	}
}

/*
====================
CG_ClipMoveToEntities

====================
*/
static void CG_ClipMoveToEntities( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
								   int skipNumber, int mask, int capsule, trace_t *tr ) {
	int i, x, zd, zu;
	trace_t trace;
	entityState_t   *ent;
	clipHandle_t cmodel;
	vec3_t bmins, bmaxs;
	vec3_t origin, angles;
	centity_t   *cent;

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];
		ent = &cent->currentState;

		if ( ent->number == skipNumber ) {
			continue;
		}

		if ( ent->solid == SOLID_BMODEL ) {
			// special value for bmodel
			cmodel = trap_CM_InlineModel( ent->modelindex );
//			VectorCopy( cent->lerpAngles, angles );
			BG_EvaluateTrajectory( &cent->currentState.apos, cg.physicsTime, angles );
			BG_EvaluateTrajectory( &cent->currentState.pos, cg.physicsTime, origin );
		} else {
			// encoded bbox
			x = ( ent->solid & 255 );
			zd = ( ( ent->solid >> 8 ) & 255 );
			zu = ( ( ent->solid >> 16 ) & 255 ) - 32;

			bmins[0] = bmins[1] = -x;
			bmaxs[0] = bmaxs[1] = x;
			bmins[2] = -zd;
			bmaxs[2] = zu;

			// MrE: use bbox or capsule
			if ( ent->eFlags & EF_CAPSULE ) {
				cmodel = trap_CM_TempCapsuleModel( bmins, bmaxs );
			} else {
				cmodel = trap_CM_TempBoxModel( bmins, bmaxs );
			}
			VectorCopy( vec3_origin, angles );
			VectorCopy( cent->lerpOrigin, origin );
		}
		// MrE: use bbox of capsule
		if ( capsule ) {
			trap_CM_TransformedCapsuleTrace( &trace, start, end,
											 mins, maxs, cmodel,  mask, origin, angles );
		} else {
			trap_CM_TransformedBoxTrace( &trace, start, end,
										 mins, maxs, cmodel,  mask, origin, angles );
		}

		if ( trace.allsolid || trace.fraction < tr->fraction ) {
			trace.entityNum = ent->number;
			*tr = trace;
		} else if ( trace.startsolid ) {
			tr->startsolid = qtrue;
		}
		if ( tr->allsolid ) {
			return;
		}
	}
}

/*
================
CG_Trace
================
*/
void    CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
				  int skipNumber, int mask ) {
	trace_t t;

	trap_CM_BoxTrace( &t, start, end, mins, maxs, 0, mask );
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	CG_ClipMoveToEntities( start, mins, maxs, end, skipNumber, mask, qfalse, &t );

	*result = t;
}

/*
================
CG_TraceCapsule
================
*/
void    CG_TraceCapsule( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
						 int skipNumber, int mask ) {
	trace_t t;

	trap_CM_CapsuleTrace( &t, start, end, mins, maxs, 0, mask );
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	CG_ClipMoveToEntities( start, mins, maxs, end, skipNumber, mask, qtrue, &t );

	*result = t;
}

/*
================
CG_PointContents
================
*/
int     CG_PointContents( const vec3_t point, int passEntityNum ) {
	int i;
	entityState_t   *ent;
	centity_t   *cent;
	clipHandle_t cmodel;
	int contents;

	contents = trap_CM_PointContents( point, 0 );

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];

		ent = &cent->currentState;

		if ( ent->number == passEntityNum ) {
			continue;
		}

		if ( ent->solid != SOLID_BMODEL ) { // special value for bmodel
			continue;
		}

		cmodel = trap_CM_InlineModel( ent->modelindex );
		if ( !cmodel ) {
			continue;
		}

		contents |= trap_CM_TransformedPointContents( point, cmodel, cent->lerpOrigin, cent->lerpAngles );
		//		contents |= trap_CM_TransformedPointContents( point, cmodel, ent->origin, ent->angles );
	}

	return contents;
}


/*
========================
CG_InterpolatePlayerState

Generates cg.predictedPlayerState by interpolating between
cg.snap->player_state and cg.nextFrame->player_state
========================
*/
static void CG_InterpolatePlayerState( qboolean grabAngles ) {
	float f;
	int i;
	playerState_t   *out;
	snapshot_t      *prev, *next;

	out = &cg.predictedPlayerState;
	prev = cg.snap;
	next = cg.nextSnap;

	*out = cg.snap->ps;

	// if we are still allowing local input, short circuit the view angles
	if ( grabAngles ) {
		usercmd_t cmd;
		int cmdNum;

		cmdNum = trap_GetCurrentCmdNumber();
		trap_GetUserCmd( cmdNum, &cmd );

		PM_UpdateViewAngles( out, &cmd, CG_Trace );
	}

	// if the next frame is a teleport, we can't lerp to it
	if ( cg.nextFrameTeleport ) {
		return;
	}

	if ( !next || next->serverTime <= prev->serverTime ) {
		return;
	}

	f = (float)( cg.time - prev->serverTime ) / ( next->serverTime - prev->serverTime );

	i = next->ps.bobCycle;
	if ( i < prev->ps.bobCycle ) {
		i += 256;       // handle wraparound
	}
	out->bobCycle = prev->ps.bobCycle + f * ( i - prev->ps.bobCycle );

	for ( i = 0 ; i < 3 ; i++ ) {
		out->origin[i] = prev->ps.origin[i] + f * ( next->ps.origin[i] - prev->ps.origin[i] );
		if ( !grabAngles ) {
			out->viewangles[i] = LerpAngle(
				prev->ps.viewangles[i], next->ps.viewangles[i], f );
		}
		out->velocity[i] = prev->ps.velocity[i] +
						   f * ( next->ps.velocity[i] - prev->ps.velocity[i] );
	}

}

/*
===================
CG_TouchItem
===================
*/
static void CG_TouchItem( centity_t *cent ) {
	gitem_t     *item;

	if ( !cg_predictItems.integer ) {
		return;
	}

//----(SA) wolf -- not allowing this for single player games
//	if( cgs.gametype == GT_SINGLE_PLAYER) {
//		return;
//	}

//----(SA) autoactivate
	if ( !cg_autoactivate.integer ) {
		return;
	}
//----(SA) end


	if ( !BG_PlayerTouchesItem( &cg.predictedPlayerState, &cent->currentState, cg.time ) ) {
		return;
	}

	// never pick an item up twice in a prediction
	if ( cent->miscTime == cg.time ) {
		return;
	}

	if ( !BG_CanItemBeGrabbed( &cent->currentState, &cg.predictedPlayerState ) ) {
		return;     // can't hold it
	}

	item = &bg_itemlist[ cent->currentState.modelindex ];

	// (SA) no prediction of books/clipboards
	if ( item->giType == IT_HOLDABLE ) {
		if ( item->giTag >= HI_BOOK1 && item->giTag <= HI_BOOK3 ) {
			return;
		}
	}
	if ( item->giType == IT_CLIPBOARD ) {
		return;
	}

	// (SA) treasure needs to be activeated, no touch
	if ( item->giType == IT_TREASURE ) {
		return;
	}

	// Special case for flags.
	// We don't predict touching our own flag
	if ( cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_RED &&
		 item->giTag == PW_REDFLAG ) {
		return;
	}
	if ( cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_BLUE &&
		 item->giTag == PW_BLUEFLAG ) {
		return;
	}


	// grab it
	BG_AddPredictableEventToPlayerstate( EV_ITEM_PICKUP, cent->currentState.modelindex, &cg.predictedPlayerState );

	// remove it from the frame so it won't be drawn
	cent->currentState.eFlags |= EF_NODRAW;

	// don't touch it again this prediction
	cent->miscTime = cg.time;

	// if its a weapon, give them some predicted ammo so the autoswitch will work
	if ( item->giType == IT_WEAPON ) {
		COM_BitSet( cg.predictedPlayerState.weapons, item->giTag );

//----(SA)	added
		if ( item->giTag == WP_SNOOPERSCOPE ) {
			COM_BitSet( cg.predictedPlayerState.weapons, WP_GARAND );
		} else if ( item->giTag == WP_GARAND ) {
			COM_BitSet( cg.predictedPlayerState.weapons, WP_SNOOPERSCOPE );
		}
//----(SA)	end

		if ( !cg.predictedPlayerState.ammo[ BG_FindAmmoForWeapon( item->giTag )] ) {
			cg.predictedPlayerState.ammo[ BG_FindAmmoForWeapon( item->giTag )] = 1;
		}
	}

//----(SA)
	if ( item->giType == IT_HOLDABLE ) {
		cg.predictedPlayerState.stats[ STAT_HOLDABLE_ITEM ] |= 1 << item->giTag;
	}
//----(SA)	end
}

void CG_AddDirtBulletParticles( vec3_t origin, vec3_t dir, int speed, int duration, int count, float randScale,
								float width, float height, float alpha, char *shadername );


/*
=========================
CG_TouchTriggerPrediction

Predict push triggers and items
=========================
*/
static void CG_TouchTriggerPrediction( void ) {
	int i;
	trace_t trace;
	entityState_t   *ent;
	clipHandle_t cmodel;
	centity_t       *cent;
	qboolean spectator;
//	vec3_t			mins, maxs; // TTimo: unused
	const char      *cs;

	// dead clients don't activate triggers
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	spectator = ( ( cg.predictedPlayerState.pm_type == PM_SPECTATOR ) || ( cg.predictedPlayerState.pm_flags & PMF_LIMBO ) ); // JPW NERVE

	if ( cg.predictedPlayerState.pm_type != PM_NORMAL && !spectator ) {
		return;
	}

	for ( i = 0 ; i < cg_numTriggerEntities ; i++ ) {
		cent = cg_triggerEntities[ i ];
		ent = &cent->currentState;

		if ( ent->eType == ET_ITEM && !spectator ) {
			CG_TouchItem( cent );
			continue;
		}

		if ( ent->solid != SOLID_BMODEL ) {
			continue;
		}

		cmodel = trap_CM_InlineModel( ent->modelindex );
		if ( !cmodel ) {
			continue;
		}

		trap_CM_BoxTrace( &trace, cg.predictedPlayerState.origin, cg.predictedPlayerState.origin,
						  cg_pmove.mins, cg_pmove.maxs, cmodel, -1 );

		if ( !trace.startsolid ) {
			continue;
		}
/*
// JPW NERVE
		if (ent->eType == ET_CONCUSSIVE_TRIGGER) {
			vec3_t dir,center;
			dir[0] = 0;
			dir[1] = 0;
			dir[2] = -1;
			CG_Printf("in concussive trigger\n");
			trap_R_ModelBounds(cmodel,mins,maxs);
			VectorAdd(mins,maxs,center);
			VectorScale(center,0.5,center);
			CG_Printf("cmodel=%d mins=%f,%f,%f maxs=%f,%f,%f\n",cmodel,
				mins[0],mins[1],mins[2],maxs[0],maxs[1],maxs[2]);

			CG_AddDirtBulletParticles( mins, dir,
									190,	// speed
									900,	// duration
									1,	// count
									0.25, 32,32, 0.5, "dirt_splash" );	// rand scale
			CG_AddDirtBulletParticles( maxs, dir,
									190,	// speed
									900,	// duration
									1,	// count
									0.25, 32,32, 0.5, "dirt_splash" );	// rand scale
		}
		else
// jpw

*/
		if ( ent->eType == ET_OID_TRIGGER ) {
			cs = CG_ConfigString( CS_OID_TRIGGERS + ent->teamNum );

			CG_ObjectivePrint( va( "You are near %s\n", cs ), SMALLCHAR_WIDTH );
		} else if ( ent->eType == ET_TELEPORT_TRIGGER )   {
			cg.hyperspace = qtrue;
		} else if ( ent->eType == ET_PUSH_TRIGGER )   {
			float s;
			vec3_t dir;

			// we hit this push trigger
			if ( spectator ) {
				continue;
			}

			// flying characters don't hit bounce pads
			if ( cg.predictedPlayerState.powerups[PW_FLIGHT] ) {
				continue;
			}

			// if we are already flying along the bounce direction, don't play sound again
			VectorNormalize2( ent->origin2, dir );
			s = DotProduct( cg.predictedPlayerState.velocity, dir );
			if ( s < 500 ) {
				// don't play the event sound again if we are in a fat trigger
				BG_AddPredictableEventToPlayerstate( EV_JUMP_PAD, 0, &cg.predictedPlayerState );
			}
			VectorCopy( ent->origin2, cg.predictedPlayerState.velocity );
		}
	}
}


//unlagged - optimized prediction
#define ABS(x) ((x) < 0 ? (-(x)) : (x))

static int IsUnacceptableError( playerState_t *ps, playerState_t *pps ) {
	vec3_t delta;
	int i;

	if ( pps->pm_type != ps->pm_type ||
			pps->pm_flags != ps->pm_flags ||
			pps->pm_time != ps->pm_time ) {
		return 1;
	}

	VectorSubtract( pps->origin, ps->origin, delta );
	if ( VectorLengthSquared( delta ) > 0.1f * 0.1f ) {
		if ( cg_showmiss.integer ) {
			CG_Printf("delta: %.2f  ", VectorLength(delta) );
		}
		return 2;
	}

	VectorSubtract( pps->velocity, ps->velocity, delta );
	if ( VectorLengthSquared( delta ) > 0.1f * 0.1f ) {
		if ( cg_showmiss.integer ) {
			CG_Printf("delta: %.2f  ", VectorLength(delta) );
		}
		return 3;
	}

	if ( pps->weaponTime != ps->weaponTime ||
			pps->gravity != ps->gravity ||
			pps->speed != ps->speed ||
			pps->delta_angles[0] != ps->delta_angles[0] ||
			pps->delta_angles[1] != ps->delta_angles[1] ||
			pps->delta_angles[2] != ps->delta_angles[2] || 
			pps->groundEntityNum != ps->groundEntityNum ) {
		return 4;
	}

	if ( pps->legsTimer != ps->legsTimer ||
			pps->legsAnim != ps->legsAnim ||
			pps->torsoTimer != ps->torsoTimer ||
			pps->torsoAnim != ps->torsoAnim ||
			pps->movementDir != ps->movementDir ) {
		return 5;
	}

	if ( pps->eFlags != ps->eFlags ) {
		return 7;
	}

	if ( pps->eventSequence != ps->eventSequence ) {
		return 8;
	}

	for ( i = 0; i < 4; i++ ) {
		if ( pps->events[i] != ps->events[i] ||
				pps->eventParms[i] != ps->eventParms[i] ) {
			return 9;
		}
	}

	if ( pps->externalEvent != ps->externalEvent ||
			pps->externalEventParm != ps->externalEventParm ||
			pps->externalEventTime != ps->externalEventTime ) {
		return 10;
	}

	if ( pps->clientNum != ps->clientNum ||
			pps->weapon != ps->weapon ||
			pps->weaponstate != ps->weaponstate ) {
		return 11;
	}

	if ( ABS(pps->viewangles[0] - ps->viewangles[0]) > 1.0f ||
			ABS(pps->viewangles[1] - ps->viewangles[1]) > 1.0f ||
			ABS(pps->viewangles[2] - ps->viewangles[2]) > 1.0f ) {
		return 12;
	}

	if ( pps->viewheight != ps->viewheight ) {
		return 13;
	}

	if ( pps->damageEvent != ps->damageEvent ||
			pps->damageYaw != ps->damageYaw ||
			pps->damagePitch != ps->damagePitch ||
			pps->damageCount != ps->damageCount ) {
		return 14;
	}

	for ( i = 0; i < MAX_STATS; i++ ) {
		if ( pps->stats[i] != ps->stats[i] ) {
			return 15;
		}
	}

	for ( i = 0; i < MAX_PERSISTANT; i++ ) {
		if ( pps->persistant[i] != ps->persistant[i] ) {
			return 16;
		}
	}

	for ( i = 0; i < MAX_POWERUPS; i++ ) {
		if ( pps->powerups[i] != ps->powerups[i] ) {
			return 17;
		}
	}

	for ( i = 0; i < MAX_WEAPONS; i++ ) {
		if ( pps->ammo[i] != ps->ammo[i] ) {
			return 18;
		}
	}

	return 0;
}
//unlagged - optimized prediction

/*
=================
CG_PredictPlayerState

Generates cg.predictedPlayerState for the current cg.time
cg.predictedPlayerState is guaranteed to be valid after exiting.

For demo playback, this will be an interpolation between two valid
playerState_t.

For normal gameplay, it will be the result of predicted usercmd_t on
top of the most recent playerState_t received from the server.

Each new snapshot will usually have one or more new usercmd over the last,
but we simulate all unacknowledged commands each time, not just the new ones.
This means that on an internet connection, quite a few pmoves may be issued
each frame.

OPTIMIZE: don't re-simulate unless the newly arrived snapshot playerState_t
differs from the predicted one.  Would require saving all intermediate
playerState_t during prediction. (this is "dead reckoning" and would definately
be nice to have in there (SA))

We detect prediction errors and allow them to be decayed off over several frames
to ease the jerk.
=================
*/
void CG_PredictPlayerState( void ) {
	int cmdNum, current;
	playerState_t oldPlayerState;
	qboolean moved;
	usercmd_t oldestCmd;
	usercmd_t latestCmd;
	vec3_t deltaAngles = { 0 };
	//unlagged - optimized prediction
	int stateIndex, predictCmd;
	int numPredicted = 0, numPlayedBack = 0; // debug code
	//unlagged - optimized prediction

	cg.hyperspace = qfalse; // will be set if touching a trigger_teleport

	// if this is the first frame we must guarantee
	// predictedPlayerState is valid even if there is some
	// other error condition
	if ( !cg.validPPS ) {
		cg.validPPS = qtrue;
		cg.predictedPlayerState = cg.snap->ps;
	}

	// demo playback just copies the moves
	if ( cg.demoPlayback || ( cg.snap->ps.pm_flags & PMF_FOLLOW ) ) {
		CG_InterpolatePlayerState( qfalse );
		return;
	}

	// non-predicting local movement will grab the latest angles
	if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
		CG_InterpolatePlayerState( qtrue );
		return;
	}

	// prepare for pmove
	cg_pmove.ps = &cg.predictedPlayerState;
	cg_pmove.pmext = &cg.pmext;

	// Arnout: are we using an mg42?
	if ( cg_pmove.ps->eFlags & EF_MG42_ACTIVE ) {
		cg_pmove.pmext->harc = cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.origin2[0];
		cg_pmove.pmext->varc = cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.origin2[1];
		VectorCopy( cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.angles2, cg_pmove.pmext->centerangles );
		cg_pmove.pmext->centerangles[PITCH] = AngleNormalize180( cg_pmove.pmext->centerangles[PITCH] );
		cg_pmove.pmext->centerangles[YAW] = AngleNormalize180( cg_pmove.pmext->centerangles[YAW] );
		cg_pmove.pmext->centerangles[ROLL] = AngleNormalize180( cg_pmove.pmext->centerangles[ROLL] );
	}

	//DHM - Nerve :: We've gone back to using normal bbox traces
	//cg_pmove.trace = CG_TraceCapsule;
	cg_pmove.trace = CG_Trace;
	cg_pmove.pointcontents = CG_PointContents;
	if ( cg_pmove.ps->pm_type == PM_DEAD ) {
		cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
		// DHM-Nerve added:: EF_DEAD is checked for in Pmove functions, but wasn't being set
		//              until after Pmove
		cg_pmove.ps->eFlags |= EF_DEAD;
		// dhm-Nerve end
	} else {
		cg_pmove.tracemask = MASK_PLAYERSOLID;
	}
	if ( ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) || ( cg.snap->ps.pm_flags & PMF_LIMBO ) ) { // JPW NERVE limbo
		cg_pmove.tracemask &= ~CONTENTS_BODY;   // spectators can fly through bodies
	}
	cg_pmove.noFootsteps = ( cgs.dmflags & DF_NO_FOOTSTEPS ) > 0;

	//----(SA)	added
	cg_pmove.noWeapClips = ( cgs.dmflags & DF_NO_WEAPRELOAD ) > 0;
	if ( cg.predictedPlayerState.aiChar ) {
		cg_pmove.noWeapClips = qtrue;   // ensure AI characters don't use clips
	}
//----(SA)	end


	// save the state before the pmove so we can detect transitions
	oldPlayerState = cg.predictedPlayerState;

	current = trap_GetCurrentCmdNumber();

	// if we don't have the commands right after the snapshot, we
	// can't accurately predict a current position, so just freeze at
	// the last good position we had
	cmdNum = current - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &oldestCmd );
	if ( oldestCmd.serverTime > cg.snap->ps.commandTime
		 && oldestCmd.serverTime < cg.time ) {  // special check for map_restart
		if ( cg_showmiss.integer ) {
			CG_Printf( "exceeded PACKET_BACKUP on commands\n" );
		}
//		return;
	}

	// get the latest command so we can know which commands are from previous map_restarts
	trap_GetUserCmd( current, &latestCmd );

	// get the most recent information we have, even if
	// the server time is beyond our current cg.time,
	// because predicted player positions are going to
	// be ahead of everything else anyway
	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		cg.predictedPlayerState = cg.nextSnap->ps;
		cg.physicsTime = cg.nextSnap->serverTime;
	} else {
		cg.predictedPlayerState = cg.snap->ps;
		cg.physicsTime = cg.snap->serverTime;
	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set( "pmove_msec", "8" );
		trap_Cvar_Update( &pmove_msec );
	} else if ( pmove_msec.integer > 33 ) {
		trap_Cvar_Set( "pmove_msec", "33" );
		trap_Cvar_Update( &pmove_msec );
	}

	cg_pmove.pmove_fixed = pmove_fixed.integer; // | cg_pmove_fixed.integer;
	cg_pmove.pmove_msec = pmove_msec.integer;

	// RTCWPro - fixed physics
	if (cg_fixedphysicsfps.integer)
	{
		if (cg_fixedphysicsfps.integer > 333)
		{
			trap_Cvar_Set("g_fixedphysicsfps", "333");
		}

		cg_pmove.fixedphysicsfps = cg_fixedphysicsfps.integer;
	}
	// RTCWPro end

//----(SA)	added
	// restore persistant client-side playerstate variables before doing the pmove
	// this could be done as suggested in q_shared.h ~line 1155, but right now I copy each variable individually
	// DHM - Nerve :: weapAnim is transmitted now
	//cg.predictedPlayerState.weapAnim				= oldPlayerState.weapAnim;
	cg.predictedPlayerState.weapAnimTimer           = oldPlayerState.weapAnimTimer;
	cg.predictedPlayerState.venomTime               = oldPlayerState.venomTime;
//----(SA)	end

	// RF, anim system
	if ( cg_animState.integer ) {
		cg.predictedPlayerState.aiState = cg_animState.integer - 1;
	}

	//unlagged - optimized prediction
	// Like the comments described above, a player's state is entirely
	// re-predicted from the last valid snapshot every client frame, which
	// can be really, really, really slow.  Every old command has to be
	// run again.  For every client frame that is *not* directly after a
	// snapshot, this is unnecessary, since we have no new information.
	// For those, we'll play back the predictions from the last frame and
	// predict only the newest commands.  Essentially, we'll be doing
	// an incremental predict instead of a full predict.
	//
	// If we have a new snapshot, we can compare its player state's command
	// time to the command times in the queue to find a match.  If we find
	// a matching state, and the predicted version has not deviated, we can
	// use the predicted state as a base - and also do an incremental predict.
	//
	// With this method, we get incremental predicts on every client frame
	// except a frame following a new snapshot in which there was a prediction
	// error.  This yeilds anywhere from a 15% to 40% performance increase,
	// depending on how much of a bottleneck the CPU is.

	// we check for cg_latentCmds because it'll mess up the optimization
	// FIXME: make cg_latentCmds work with cg_optimizePrediction?
	if ( cg_optimizePrediction.integer && !cg_latentCmds.integer ) {
		if ( cg.nextFrameTeleport || cg.thisFrameTeleport ) {
			// do a full predict
			cg.lastPredictedCommand = 0;
			cg.stateTail = cg.stateHead;
			predictCmd = current - CMD_BACKUP + 1;
		}
		// cg.physicsTime is the current snapshot's serverTime
		// if it's the same as the last one
		else if ( cg.physicsTime == cg.lastServerTime ) {
			// we have no new information, so do an incremental predict
			predictCmd = cg.lastPredictedCommand + 1;
		}
		else {
			// we have a new snapshot

			int i;
			qboolean error = qtrue;

			// loop through the saved states queue
			for ( i = cg.stateHead; i != cg.stateTail; i = (i + 1) % NUM_SAVED_STATES ) {
				// if we find a predicted state whose commandTime matches the snapshot player state's commandTime
				if ( cg.savedPmoveStates[i].commandTime == cg.predictedPlayerState.commandTime ) {
					// make sure the state differences are acceptable
					int errorcode = IsUnacceptableError( &cg.predictedPlayerState, &cg.savedPmoveStates[i] );

					// too much change?
					if ( errorcode ) {
						if ( cg_showmiss.integer ) {
							CG_Printf("errorcode %d at %d\n", errorcode, cg.time);
						}
						// yeah, so do a full predict
						break;
					}

					// this one is almost exact, so we'll copy it in as the starting point
					*cg_pmove.ps = cg.savedPmoveStates[i];
					// advance the head
					cg.stateHead = (i + 1) % NUM_SAVED_STATES;

					// set the next command to predict
					predictCmd = cg.lastPredictedCommand + 1;

					// a saved state matched, so flag it
					error = qfalse;
					break;
				}
			}

			// if no saved states matched
			if ( error ) {
				// do a full predict
				cg.lastPredictedCommand = 0;
				cg.stateTail = cg.stateHead;
				predictCmd = current - CMD_BACKUP + 1;
			}
		}

		// keep track of the server time of the last snapshot so we
		// know when we're starting from a new one in future calls
		cg.lastServerTime = cg.physicsTime;
		stateIndex = cg.stateHead;
	}
	//unlagged - optimized prediction

	// run cmds
	moved = qfalse;
	for ( cmdNum = current - CMD_BACKUP + 1 ; cmdNum <= current ; cmdNum++ ) {
		// get the command
		trap_GetUserCmd( cmdNum, &cg_pmove.cmd );
		// get the previous command
		trap_GetUserCmd( cmdNum - 1, &cg_pmove.oldcmd );

		if ( cg_pmove.pmove_fixed ) {
			PM_UpdateViewAngles( cg_pmove.ps, &cg_pmove.cmd, CG_Trace );
		}

		// don't do anything if the time is before the snapshot player time
		if ( cg_pmove.cmd.serverTime <= cg.predictedPlayerState.commandTime ) {
			continue;
		}

		// don't do anything if the command was from a previous map_restart
		if ( cg_pmove.cmd.serverTime > latestCmd.serverTime ) {
			continue;
		}

		// check for a prediction error from last frame
		// on a lan, this will often be the exact value
		// from the snapshot, but on a wan we will have
		// to predict several commands to get to the point
		// we want to compare
		if ( cg.predictedPlayerState.commandTime == oldPlayerState.commandTime ) {
			vec3_t delta;
			float len;

			if ( cg_pmove.ps->eFlags & EF_MG42_ACTIVE ) {
				// no prediction errors here, we're locked in place
				VectorClear( cg.predictedError );
			} else if ( cg.thisFrameTeleport )   {
				// a teleport will not cause an error decay
				VectorClear( cg.predictedError );
				if ( cg_showmiss.integer ) {
					CG_Printf( "PredictionTeleport\n" );
				}
				cg.thisFrameTeleport = qfalse;
			} else {
				vec3_t adjusted; //vec3_t adjusted;
				CG_AdjustPositionForMover( cg.predictedPlayerState.origin, cg.predictedPlayerState.groundEntityNum, cg.physicsTime, cg.oldTime, adjusted, deltaAngles );

				// RF, add the deltaAngles (fixes jittery view while riding trains)
				cg.predictedPlayerState.delta_angles[YAW] += ANGLE2SHORT( deltaAngles[YAW] );

				if ( cg_showmiss.integer ) {
					if ( !VectorCompare( oldPlayerState.origin, adjusted ) ) {
						CG_Printf( "prediction error\n" );
					}
				}
				VectorSubtract( oldPlayerState.origin, adjusted, delta );
				len = VectorLength( delta );
				if ( len > 0.1 ) {
					if ( cg_showmiss.integer ) {
						CG_Printf( "Prediction miss: %f\n", len );
					}
					if ( cg_errorDecay.integer ) {
						int t;
						float f;

						t = cg.time - cg.predictedErrorTime;
						f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;
						if ( f < 0 ) {
							f = 0;
						}
						if ( f > 0 && cg_showmiss.integer ) {
							CG_Printf( "Double prediction decay: %f\n", f );
						}
						VectorScale( cg.predictedError, f, cg.predictedError );
					} else {
						VectorClear( cg.predictedError );
					}
					VectorAdd( delta, cg.predictedError, cg.predictedError );
					cg.predictedErrorTime = cg.oldTime;
				}
			}
		}

		// don't predict gauntlet firing, which is only supposed to happen
		// when it actually inflicts damage
		cg_pmove.gauntletHit = qfalse;

		if ( cg_pmove.pmove_fixed ) {
			cg_pmove.cmd.serverTime = ( ( cg_pmove.cmd.serverTime + pmove_msec.integer - 1 ) / pmove_msec.integer ) * pmove_msec.integer;
		}

		// ydnar: if server respawning, freeze the player
		if (cg.serverRespawning) {
			cg_pmove.ps->pm_type = PM_FREEZE;
		}


		// RF, if waiting for mission stats to go, ignore all input
		if ( strlen( cg_missionStats.string ) > 1 || cg_norender.integer ) {
			cg_pmove.cmd.buttons = 0;
			cg_pmove.cmd.forwardmove = 0;
			cg_pmove.cmd.rightmove = 0;
			cg_pmove.cmd.upmove = 0;
			cg_pmove.cmd.wbuttons = 0;
			cg_pmove.cmd.wolfkick = 0;
			cg_pmove.cmd.angles[0] = cg_pmove.oldcmd.angles[0];
			cg_pmove.cmd.angles[1] = cg_pmove.oldcmd.angles[1];
			cg_pmove.cmd.angles[2] = cg_pmove.oldcmd.angles[2];
		}

		// NERVE - SMF
		cg_pmove.gametype = cgs.gametype;
		cg_pmove.ltChargeTime = cg_LTChargeTime.integer;
		cg_pmove.soldierChargeTime = cg_soldierChargeTime.integer;
		cg_pmove.engineerChargeTime = cg_engineerChargeTime.integer;
		cg_pmove.medicChargeTime = cg_medicChargeTime.integer;
		// -NERVE - SMF

		//unlagged - optimized prediction
		// we check for cg_latentCmds because it'll mess up the optimization
		if ( cg_optimizePrediction.integer && !cg_latentCmds.integer ) {
			// if we need to predict this command, or we've run out of space in the saved states queue
			if ( cmdNum >= predictCmd || (stateIndex + 1) % NUM_SAVED_STATES == cg.stateHead ) {
				// run the Pmove
				Pmove (&cg_pmove);

				numPredicted++; // debug code

				// record the last predicted command
				cg.lastPredictedCommand = cmdNum;

				// if we haven't run out of space in the saved states queue
				if ( (stateIndex + 1) % NUM_SAVED_STATES != cg.stateHead ) {
					// save the state for the false case (of cmdNum >= predictCmd)
					// in later calls to this function
					cg.savedPmoveStates[stateIndex] = *cg_pmove.ps;
					stateIndex = (stateIndex + 1) % NUM_SAVED_STATES;
					cg.stateTail = stateIndex;
				}
			}
			else {
				numPlayedBack++; // debug code

				if ( cg_showmiss.integer && 
						cg.savedPmoveStates[stateIndex].commandTime != cg_pmove.cmd.serverTime) {
					// this should ONLY happen just after changing the value of pmove_fixed
					CG_Printf( "saved state miss\n" );
				}

				// play back the command from the saved states
				*cg_pmove.ps = cg.savedPmoveStates[stateIndex];

				// go to the next element in the saved states array
				stateIndex = (stateIndex + 1) % NUM_SAVED_STATES;
			}
		}
		else {
			// run the Pmove
			Pmove (&cg_pmove);

			numPredicted++; // debug code
		}
		//unlagged - optimized prediction

		moved = qtrue;

		// add push trigger movement effects
		CG_TouchTriggerPrediction();
	}

	if ( cg_showmiss.integer > 1 ) {
		CG_Printf( "[%i : %i] ", cg_pmove.cmd.serverTime, cg.time );
	}

	if ( !moved ) {
		if ( cg_showmiss.integer ) {
			CG_Printf( "not moved\n" );
		}
		return;
	}

	// adjust for the movement of the groundentity
	CG_AdjustPositionForMover( cg.predictedPlayerState.origin, cg.predictedPlayerState.groundEntityNum, cg.physicsTime, cg.time, cg.predictedPlayerState.origin, deltaAngles );


	// fire events and other transition triggered things
	CG_TransitionPlayerState( &cg.predictedPlayerState, &oldPlayerState );
}

