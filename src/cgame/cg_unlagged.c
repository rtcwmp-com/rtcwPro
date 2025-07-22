/*
===========================================================================
Copyright (C) 2006 Neil Toronto.

This file is part of the Unlagged source code.

Unlagged source code is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Unlagged source code is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Unlagged source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "cg_local.h"

// we'll need these prototypes
void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum, qboolean wolfkick, int otherEntNum2, int seed );
// and this as well
// RF, wrote this so we can dynamically switch between old and new values while testing g_userAim
// must match in g_weapon.c
float G_GetWeaponSpread( int weapon ) {
	switch ( weapon ) {
		case WP_LUGER: return 600;
		case WP_SILENCER: return 900;
		case WP_COLT: return 800;
		case WP_AKIMBO: return 800;         //----(SA)added
		case WP_VENOM: return 600;
		case WP_MP40: return 400;
		case WP_FG42SCOPE:
		case WP_FG42:   return 500;
		case WP_BAR:
		case WP_BAR2:   return 500;
		case WP_THOMPSON: return 600;
		case WP_STEN: return 200;
		case WP_MAUSER: return 2000;
		case WP_GARAND: return 600;
		case WP_SNIPERRIFLE: return 700;         // was 300
		case WP_SNOOPERSCOPE: return 700;
		}

	// jpw
	return 0;   // shouldn't get here
}

/*
=======================
CG_PredictWeaponEffects

Draws predicted effects for the railgun, shotgun, and machinegun.  The
lightning gun is done in CG_LightningBolt, since it was just a matter
of setting the right origin and angles.
=======================
*/
void CG_PredictWeaponEffects( centity_t *cent ) {
	entityState_t *ent = &cent->currentState;

	// if the client isn't us, forget it
	if ( cent->currentState.number != cg.predictedPlayerState.clientNum ) {
		return;
	}

	// if it's not switched on server-side, forget it
	if ( !cgs.delagHitscan ) {
		return;
	}

	// do we have it on for the machinegun?
	if ( cg_delag.integer & 1 || cg_delag.integer & 2 ) {
		vec3_t		muzzlePoint, forward, right, up;
		// the server will use this exact time (it'll be serverTime on that end)
		int seed = cg.oldTime % 256;
		float r, u;
		trace_t tr;
		qboolean flesh;
		int fleshEntityNum;
		vec3_t endPoint;
		qboolean randSpread = qtrue;
		int dist = 8192;
		float aimSpreadScale;
		float spread;


		aimSpreadScale = (float)cg.predictedPlayerState.aimSpreadScale / 255.0;
		/*
		// This comment seems to match the server side aimSpreadScale more closely than the above line 
		aimSpreadScale = (float)( ( cg.nextSnap ) ? cg.nextSnap->ps.aimSpreadScale / 255.0 : cg.snap->ps.aimSpreadScale / 255.0 );
		aimSpreadScale += 0.15f; // (SA) just adding a temp /maximum/ accuracy for player (this will be re-visited in greater detail :)
		*/
		if ( cg.predictedPlayerState.groundEntityNum == ENTITYNUM_NONE ) {
			aimSpreadScale = 2.0f;
		} 
			else if ( aimSpreadScale > 1 /*|| cg.predictedPlayerState.weapon == WP_MAUSER*/) {
				aimSpreadScale = 1.0f;  // still cap at 1.0
			}

		spread = G_GetWeaponSpread( cg.predictedPlayerState.weapon ) * aimSpreadScale;

		r = Q_crandom(&seed) * spread;
		u = Q_crandom(&seed) * spread;

		/*if ( cg.predictedPlayerState.weapon == WP_SNOOPERSCOPE || cg.predictedPlayerState.weapon == WP_SNIPERRIFLE ) {
			// aim dir already accounted for sway of scoped weapons in CalcMuzzlePoints()
			dist *= 2;
			randSpread = qfalse;
		}*/

		// get the muzzle point
		VectorCopy( cg.predictedPlayerState.origin, muzzlePoint );
		muzzlePoint[2] += cg.predictedPlayerState.viewheight;

		// get forward, right, and up
		AngleVectors( cg.predictedPlayerState.viewangles, forward, right, up );
		VectorMA( muzzlePoint, 14, forward, muzzlePoint );

		VectorMA( muzzlePoint, dist, forward, endPoint );
		if ( randSpread ) {
			VectorMA( endPoint, r, right, endPoint );
			VectorMA( endPoint, u, up, endPoint );
		}

		CG_Trace(&tr, muzzlePoint, NULL, NULL, endPoint, cg.predictedPlayerState.clientNum, MASK_SHOT );

		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			return;
		}

		// snap the endpos to integers, but nudged towards the line
		SnapVectorTowards( tr.endpos, muzzlePoint );

		// do bullet impact
		if ( tr.entityNum < MAX_CLIENTS ) {
			flesh = qtrue;
			fleshEntityNum = tr.entityNum;
		} else {
			flesh = qfalse;
		}

		// do the bullet impact
		CG_Bullet( tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, flesh, fleshEntityNum , qfalse, cg.predictedPlayerEntity.currentState.otherEntityNum2, 0 );
		//CG_Printf( "Predicted bullet\n" );
	}
}

/*
=======================
CG_PredictedWeapon

Returns true if weapon is predicted
=======================
*/
qboolean CG_PredictedWeapon( int weapon ) {

	if (	weapon == WP_MP40 ||
			weapon == WP_THOMPSON ||
			weapon == WP_COLT ||
			weapon == WP_LUGER /*||
			weapon == WP_STEN ||
			weapon == WP_MAUSER ||
			weapon == WP_GARAND ||
			weapon == WP_SNIPERRIFLE ||
			weapon == WP_SNOOPERSCOPE*/ ) {

		return qtrue;

	}

	return qfalse;

}