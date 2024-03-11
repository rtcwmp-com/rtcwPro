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
//void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, int otherEntNum );
void CG_VenomPattern( vec3_t origin, vec3_t origin2, int seed, int otherEntNum );
//void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum );
void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum, qboolean wolfkick, int otherEntNum2, int seed );

// and this as well
// RF, wrote this so we can dynamically switch between old and new values while testing g_userAim
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
	vec3_t		muzzlePoint, forward, right, up;
	entityState_t *ent = &cent->currentState;
	float spread;
	int i;
	qboolean predictedWeapon = qfalse;
	int predictedWeapons[] = { WP_COLT, WP_LUGER, WP_MP40, WP_STEN, WP_THOMPSON/*, WP_MAUSER, WP_GARAND, WP_SNIPERRIFLE, WP_SNOOPERSCOPE*/ };
	int length = sizeof(predictedWeapons) / sizeof(predictedWeapons[0]);

	// if the client isn't us, forget it
	if ( cent->currentState.number != cg.predictedPlayerState.clientNum ) {
		return;
	}

	// if it's not switched on server-side, forget it
	if ( !cgs.delagHitscan ) {
		return;
	}

	for (i = 0; i < length; i++) {
		if ( ent->weapon == predictedWeapons[i] ) {
			predictedWeapon = qtrue;
			spread = G_GetWeaponSpread( predictedWeapons[i] );
			break;
		}
	}

	if ( predictedWeapon ) {
		// do we have it on for the machinegun?
		if ( cg_delag.integer & 1 || cg_delag.integer & 2 ) {
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

			// get the muzzle point
			VectorCopy( cg.predictedPlayerState.origin, muzzlePoint );
			muzzlePoint[2] += cg.predictedPlayerState.viewheight;

			// get forward, right, and up
			AngleVectors( cg.predictedPlayerState.viewangles, forward, right, up );
			VectorMA( muzzlePoint, 14, forward, muzzlePoint );


			aimSpreadScale = (float)cg.snap->ps.aimSpreadScale / 255.0;
			//aimSpreadScale = (float)cg.predictedPlayerState.aimSpreadScale / 255.0;
			if ( aimSpreadScale < 0.15f ) aimSpreadScale = 0.15f; // (SA) just adding a temp /maximum/ accuracy for player (this will be re-visited in greater detail :)
			
			if ( ent->groundEntityNum == ENTITYNUM_NONE ) {
				aimSpreadScale = 2.0f;
			} else if ( aimSpreadScale > 1 /*|| bulletWeapon == WP_MAUSER*/) {
				aimSpreadScale = 1.0f;  // still cap at 1.0
				}

			spread = spread * aimSpreadScale;

			// do everything exactly like the server does

			/*
			r = Q_random(&seed) * M_PI * 2.0f;
			u = sin(r) * Q_crandom(&seed) * MACHINEGUN_SPREAD * 16;
			r = cos(r) * Q_crandom(&seed) * MACHINEGUN_SPREAD * 16;
			*/

			r = Q_crandom(&seed) * spread;
			u = Q_crandom(&seed) * spread;

			/*if ( ent->weapon == WP_SNOOPERSCOPE || ent->weapon == WP_SNIPERRIFLE ) {
				// aim dir already accounted for sway of scoped weapons in CalcMuzzlePoints()
				dist *= 2;
				randSpread = qfalse;
			}*/

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
			CG_Bullet( tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, flesh, fleshEntityNum , qfalse, ent->otherEntityNum2, 0 );
			//Com_Printf( "CG: Predicted bullet\n" );
		}
	}
}

/*
=================
CG_AddBoundingBox

Draws a bounding box around a player.  Called from CG_Player.
=================
*/
void CG_AddBoundingBox( centity_t *cent ) {
	polyVert_t verts[4];
	clientInfo_t *ci;
	int i;
	vec3_t mins = {-15, -15, -24};
	vec3_t maxs = {15, 15, 32};
	float extx, exty, extz;
	vec3_t corners[8];
	qhandle_t bboxShader, bboxShader_nocull;

	if ( !cg_drawBBox.integer ) {
		return;
	}

	// don't draw it if it's us in first-person
	if ( cent->currentState.number == cg.predictedPlayerState.clientNum &&
			!cg.renderingThirdPerson ) {
		return;
	}

	// don't draw it for dead players
	if ( cent->currentState.eFlags & EF_DEAD ) {
		return;
	}

	// get the shader handles
	bboxShader = trap_R_RegisterShader( "bbox" );
	bboxShader_nocull = trap_R_RegisterShader( "bbox_nocull" );

	// if they don't exist, forget it
	if ( !bboxShader || !bboxShader_nocull ) {
		return;
	}

	// get the player's client info
	ci = &cgs.clientinfo[cent->currentState.clientNum];

	// if it's us
	if ( cent->currentState.number == cg.predictedPlayerState.clientNum ) {
		// use the view height
		maxs[2] = cg.predictedPlayerState.viewheight + 6;
	}
	else {
		int x, zd, zu;

		// otherwise grab the encoded bounding box
		x = (cent->currentState.solid & 255);
		zd = ((cent->currentState.solid>>8) & 255);
		zu = ((cent->currentState.solid>>16) & 255) - 32;

		mins[0] = mins[1] = -x;
		maxs[0] = maxs[1] = x;
		mins[2] = -zd;
		maxs[2] = zu;
	}

	// get the extents (size)
	extx = maxs[0] - mins[0];
	exty = maxs[1] - mins[1];
	extz = maxs[2] - mins[2];

	
	// set the polygon's texture coordinates
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;

	// set the polygon's vertex colors
	if ( ci->team == TEAM_RED ) {
		for ( i = 0; i < 4; i++ ) {
			verts[i].modulate[0] = 160;
			verts[i].modulate[1] = 0;
			verts[i].modulate[2] = 0;
			verts[i].modulate[3] = 255;
		}
	}
	else if ( ci->team == TEAM_BLUE ) {
		for ( i = 0; i < 4; i++ ) {
			verts[i].modulate[0] = 0;
			verts[i].modulate[1] = 0;
			verts[i].modulate[2] = 192;
			verts[i].modulate[3] = 255;
		}
	}
	else {
		for ( i = 0; i < 4; i++ ) {
			verts[i].modulate[0] = 0;
			verts[i].modulate[1] = 128;
			verts[i].modulate[2] = 0;
			verts[i].modulate[3] = 255;
		}
	}

	VectorAdd( cent->lerpOrigin, maxs, corners[3] );

	VectorCopy( corners[3], corners[2] );
	corners[2][0] -= extx;

	VectorCopy( corners[2], corners[1] );
	corners[1][1] -= exty;

	VectorCopy( corners[1], corners[0] );
	corners[0][0] += extx;

	for ( i = 0; i < 4; i++ ) {
		VectorCopy( corners[i], corners[i + 4] );
		corners[i + 4][2] -= extz;
	}

	// top
	VectorCopy( corners[0], verts[0].xyz );
	VectorCopy( corners[1], verts[1].xyz );
	VectorCopy( corners[2], verts[2].xyz );
	VectorCopy( corners[3], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader, 4, verts );

	// bottom
	VectorCopy( corners[7], verts[0].xyz );
	VectorCopy( corners[6], verts[1].xyz );
	VectorCopy( corners[5], verts[2].xyz );
	VectorCopy( corners[4], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader, 4, verts );

	// top side
	VectorCopy( corners[3], verts[0].xyz );
	VectorCopy( corners[2], verts[1].xyz );
	VectorCopy( corners[6], verts[2].xyz );
	VectorCopy( corners[7], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// left side
	VectorCopy( corners[2], verts[0].xyz );
	VectorCopy( corners[1], verts[1].xyz );
	VectorCopy( corners[5], verts[2].xyz );
	VectorCopy( corners[6], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// right side
	VectorCopy( corners[0], verts[0].xyz );
	VectorCopy( corners[3], verts[1].xyz );
	VectorCopy( corners[7], verts[2].xyz );
	VectorCopy( corners[4], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// bottom side
	VectorCopy( corners[1], verts[0].xyz );
	VectorCopy( corners[0], verts[1].xyz );
	VectorCopy( corners[4], verts[2].xyz );
	VectorCopy( corners[5], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );
}

/*
================
CG_Cvar_ClampInt

Clamps a cvar between two integer values, returns qtrue if it had to.
================
*/
qboolean CG_Cvar_ClampInt( const char *name, vmCvar_t *vmCvar, int min, int max ) {
	if ( vmCvar->integer > max ) {
		CG_Printf( "Allowed values are %d to %d.\n", min, max );

		Com_sprintf( vmCvar->string, MAX_CVAR_VALUE_STRING, "%d", max );
		vmCvar->value = max;
		vmCvar->integer = max;

		trap_Cvar_Set( name, vmCvar->string );
		return qtrue;
	}

	if ( vmCvar->integer < min ) {
		CG_Printf( "Allowed values are %d to %d.\n", min, max );

		Com_sprintf( vmCvar->string, MAX_CVAR_VALUE_STRING, "%d", min );
		vmCvar->value = min;
		vmCvar->integer = min;

		trap_Cvar_Set( name, vmCvar->string );
		return qtrue;
	}

	return qfalse;
}
