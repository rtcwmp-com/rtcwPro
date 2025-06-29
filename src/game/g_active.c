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

#include "ai_cast_fight.h"   // need these for avoidance


extern void G_CheckForCursorHints( gentity_t *ent );



/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
	gclient_t   *client;
	float count;
	vec3_t angles;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) {
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) {
		return;     // didn't take any damage
	}

	if ( count > 127 ) {
		count = 127;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} else {
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH] / 360.0 * 256;
		client->ps.damageYaw = angles[YAW] / 360.0 * 256;
	}

	if (g_debugDamage.integer)
	{
		AP(va("print \"damage feedback: [ %i ] pitch [ %i ] yaw [ %i ]\n\"", client->damage_knockback, client->ps.damagePitch, client->ps.damageYaw));
	}

	// play an apropriate pain sound
	if ( ( level.time > player->pain_debounce_time ) && !( player->flags & FL_GODMODE ) && !( player->r.svFlags & SVF_CASTAI ) && !( player->s.powerups & PW_INVULNERABLE ) ) { //----(SA)
		player->pain_debounce_time = level.time + 700;
		G_AddEvent( player, EV_PAIN, player->health );
	}

	client->ps.damageEvent++;   // Ridah, always increment this since we do multiple view damage anims

	client->ps.damageCount = count;

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}


#define MIN_BURN_INTERVAL 399 // JPW NERVE set burn timeinterval so we can do more precise damage (was 199 old model)

/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
	qboolean envirosuit;
	int waterlevel;
	// TTimo: unused
//	static int	lastflameburntime = 0; // JPW NERVE for slowing flamethrower burn intervals and doing more damage per interval

	if ( ent->client->noclip ) {
		ent->client->airOutTime = level.time + 12000;   // don't need air
		return;
	}

	waterlevel = ent->waterlevel;

	envirosuit = ent->client->ps.powerups[PW_BATTLESUIT] > level.time;

//	G_Printf("breathe: %d   invuln: %d   nofatigue: %d\n", ent->client->ps.powerups[PW_BREATHER], level.time - ent->client->ps.powerups[PW_INVULNERABLE], ent->client->ps.powerups[PW_NOFATIGUE]);

	//
	// check for drowning
	//
	if ( waterlevel == 3 ) {
		// envirosuit give air
		if ( envirosuit ) {
			ent->client->airOutTime = level.time + 10000;
		}

		//----(SA)	both these will end up being by virtue of having the 'breather' powerup
		if ( ent->client->ps.aiChar == AICHAR_FROGMAN ) {  // let frogmen breathe forever
			ent->client->airOutTime = level.time + 10000;
		}

		if ( ent->client->ps.aiChar == AICHAR_SEALOPER ) { // ditto
			ent->client->airOutTime = level.time + 10000;
		}

		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time ) {

			if ( ent->client->ps.powerups[PW_BREATHER] ) { // take air from the breather now that we need it
				ent->client->ps.powerups[PW_BREATHER] -= ( level.time - ent->client->airOutTime );
				ent->client->airOutTime = level.time + ( level.time - ent->client->airOutTime );
			} else {


				// drown!
				ent->client->airOutTime += 1000;
				if ( ent->health > 0 ) {
					// take more damage the longer underwater
					ent->damage += 2;
					if ( ent->damage > 15 ) {
						ent->damage = 15;
					}

					// play a gurp sound instead of a normal pain sound
					if ( ent->health <= ent->damage ) {
						G_Sound( ent, G_SoundIndex( "*drown.wav" ) );
					} else if ( rand() & 1 ) {
						G_Sound( ent, G_SoundIndex( "sound/player/gurp1.wav" ) );
					} else {
						G_Sound( ent, G_SoundIndex( "sound/player/gurp2.wav" ) );
					}

					// don't play a normal pain sound
					ent->pain_debounce_time = level.time + 200;

					G_Damage( ent, NULL, NULL, NULL, NULL,
							  ent->damage, DAMAGE_NO_ARMOR, MOD_WATER );
				}
			}
		}
	} else {
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if ( waterlevel && ( ent->watertype & CONTENTS_LAVA ) ) {
		if ( ent->health > 0 && ent->pain_debounce_time <= level.time ) {

			if ( envirosuit ) {
				G_AddEvent( ent, EV_POWERUP_BATTLESUIT, 0 );
			} else {
				if ( ent->watertype & CONTENTS_LAVA ) {
					G_Damage( ent, NULL, NULL, NULL, NULL,
							  30 * waterlevel, 0, MOD_LAVA );
				}
			}

		}
	}

	//
	// check for burning from flamethrower
	//
	// JPW NERVE MP way
	if ( ent->s.onFireEnd && ent->client ) {
		if ( level.time - ent->client->lastBurnTime >= MIN_BURN_INTERVAL ) {

			// JPW NERVE server-side incremental damage routine / player damage/health is int (not float)
			// so I can't allocate 1.5 points per server tick, and 1 is too weak and 2 is too strong.
			// solution: allocate damage far less often (MIN_BURN_INTERVAL often) and do more damage.
			// That way minimum resolution (1 point) damage changes become less critical.

			ent->client->lastBurnTime = level.time;
			if ( ( ent->s.onFireEnd > level.time ) && ( ent->health > 0 ) ) {
				gentity_t *attacker;
				attacker = g_entities + ent->flameBurnEnt;
				G_Damage( ent, attacker->parent, attacker->parent, NULL, NULL, 5, DAMAGE_NO_KNOCKBACK, MOD_FLAMETHROWER ); // JPW NERVE was 7
			}
		}
	}
	// jpw
}



/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) {
	if ( ent->aiCharacter ) {
		return;
	}

	if ( ent->waterlevel && ( ent->watertype & CONTENTS_LAVA ) ) { //----(SA)	modified since slime is no longer deadly
		ent->s.loopSound = level.snd_fry;
	} else {
		ent->s.loopSound = 0;
	}
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int i, j;
	trace_t trace;
	gentity_t   *other;

	memset( &trace, 0, sizeof( trace ) );
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		for ( j = 0 ; j < i ; j++ ) {
			if ( pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if ( j != i ) {
			continue;   // duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, other, &trace );
		}

		if ( !other->touch ) {
			continue;
		}

		other->touch( other, ent, &trace );
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void    G_TouchTriggers( gentity_t *ent ) {
	int i, num;
	int touch[MAX_GENTITIES];
	gentity_t   *hit;
	trace_t trace;
	vec3_t mins, maxs;
	static vec3_t range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	for ( i = 0 ; i < num ; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}
		if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
			continue;
		}

		// ignore most entities if a spectator
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			if ( hit->s.eType != ET_TELEPORT_TRIGGER ) {
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) {
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
				continue;
			}
		} else {
			// MrE: always use capsule for player
			//if ( !trap_EntityContactCapsule( mins, maxs, hit ) ) {
			if ( !trap_EntityContact( mins, maxs, hit ) ) {
				continue;
			}
		}

		memset( &trace, 0, sizeof( trace ) );

		if ( hit->touch ) {
			hit->touch( hit, ent, &trace );
		}

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, hit, &trace );
		}
	}
}

/*
=================
RTCWPro - follow clients in freecam
by aiming/shooting at them
Note: using generic tracing

Credits: ETLegacy and rtcwPub
G_SpectatorAttackFollow
=================
*/
qboolean G_SpectatorAttackFollow(gentity_t* ent)
{
	trace_t       tr;
	vec3_t        forward, right, up;
	vec3_t        start, end;
	vec3_t        mins, maxs;
	static vec3_t enlargeMins = { -5, -5, -5 };
	static vec3_t enlargeMaxs = { 5, 5, 5 };

	if (!ent->client)
	{
		return qfalse;
	}

	AngleVectors(ent->client->ps.viewangles, forward, right, up);
	VectorCopy(ent->client->ps.origin, start);
	VectorMA(start, 8192, forward, end);

	// enlarge the hitboxes, so spectators can easily click on them..
	VectorCopy(ent->r.mins, mins);
	VectorCopy(ent->r.maxs, maxs);
	VectorAdd(mins, enlargeMins, mins);
	VectorAdd(maxs, enlargeMaxs, maxs);

	// also put the start-point a bit forward, so we don't start the trace in solid..
	VectorMA(start, 75.0f, forward, start);

	trap_Trace(&tr, start, mins, maxs, end, ent->client->ps.clientNum, CONTENTS_BODY | CONTENTS_CORPSE);
	//G_HistoricalTrace(ent, &tr, start, mins, maxs, end, ent->s.number, CONTENTS_BODY | CONTENTS_CORPSE);

	if ((&g_entities[tr.entityNum])->client)
	{
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		ent->client->sess.spectatorClient = tr.entityNum;
		return qtrue;
	}

	return qfalse;
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
	pmove_t pm;
	gclient_t   *client;

	client = ent->client;

	if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		client->ps.pm_type = PM_SPECTATOR;

		if (client->sess.specSpeed <= 0)
		{
			client->sess.specSpeed = 400; // faster than normal
		}

		client->ps.speed = client->sess.specSpeed;

		if ( client->ps.sprintExertTime ) {
			client->ps.speed *= 3;  // (SA) allow sprint in free-cam mode
		}

		// L0 - Pause
		if ( level.paused != PAUSE_NONE && client->sess.referee == RL_NONE && client->sess.shoutcaster == 0) {
			client->ps.pm_type = PM_FREEZE;
			ucmd->buttons = 0;
			ucmd->forwardmove = 0;
			ucmd->rightmove = 0;
			ucmd->upmove = 0;
			ucmd->wbuttons = 0;
		}
		else if (client->sess.shoutcaster && client->noclip)
		{
			client->ps.pm_type = PM_NOCLIP;
		}

		// set up for pmove
		memset( &pm, 0, sizeof( pm ) );
		pm.ps = &client->ps;
		pm.pmext = &client->pmext;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;   // spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		Pmove( &pm ); // JPW NERVE

		// Rafael - Activate
		// Ridah, made it a latched event (occurs on keydown only)
		if ( client->latched_buttons & BUTTON_ACTIVATE ) {
			Cmd_Activate_f( ent );
		}

		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );

		G_TouchTriggers( ent );
		trap_UnlinkEntity( ent );
	}

	if ( ent->flags & FL_NOFATIGUE ) {
		ent->client->ps.sprintTime = 20000;
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = ucmd->wbuttons;

	// attack button cycles through spectators
	if ( ( client->buttons & BUTTON_ATTACK ) && !( client->oldbuttons & BUTTON_ATTACK ) )
	{
		// RTCWPro - make it usable by aiming/shooting
		if (client->sess.spectatorState == SPECTATOR_FREE && client->sess.sessionTeam == TEAM_SPECTATOR)
		{
			if (G_SpectatorAttackFollow(ent))
			{
				return;
			}
		}

		Cmd_FollowCycle_f(ent, 1);
	}
	// RTCWPro - make it usable by m1/2 for both directions
	else if ((client->buttons & BUTTON_ATTACK) && !(client->oldbuttons & BUTTON_ATTACK) &&
		!(client->buttons & BUTTON_ACTIVATE))
	{
		Cmd_FollowCycle_f(ent, 1);
	}
	else if ((client->wbuttons & WBUTTON_ATTACK2) && !(client->oldwbuttons & WBUTTON_ATTACK2) &&
		!(client->buttons & BUTTON_ACTIVATE))
	{
		Cmd_FollowCycle_f(ent, -1);
	}
	else if (( client->sess.sessionTeam == TEAM_SPECTATOR ) && // don't let dead team players do free fly
		( client->sess.spectatorState == SPECTATOR_FOLLOW ) &&
		( client->buttons & BUTTON_ACTIVATE ) &&
		!( client->oldbuttons & BUTTON_ACTIVATE ) &&
		G_allowFollow(ent, TEAM_RED) && G_allowFollow(ent, TEAM_BLUE) )
	{ // OSPx - Speclock
		// code moved to StopFollowing
		StopFollowing( ent );
	}
}



/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {
	if ( !g_inactivity.integer ) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
	} else if ( client->pers.cmd.forwardmove ||
				client->pers.cmd.rightmove ||
				client->pers.cmd.upmove ||
				( client->pers.cmd.wbuttons & WBUTTON_ATTACK2 ) ||
				( client->pers.cmd.buttons & BUTTON_ATTACK ) ) {
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;
	} else if ( !client->pers.localClient ) {
		if ( level.time > client->inactivityTime ) {
			trap_DropClient( client - level.clients, "Dropped due to inactivity" );
			return qfalse;
		}
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
			client->inactivityWarning = qtrue;
			trap_SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second

Modifed Source: ET Legacy
==================
*/
void ClientTimerActions(gentity_t *ent, int msec)
{
	gclient_t *client = ent->client;

	client->timeResidual += msec;

	while (client->timeResidual >= 1000)
	{
		client->timeResidual -= 1000;

		// regenerate
		if (ent->health < client->ps.stats[STAT_MAX_HEALTH])
		{
			// medic only
			if (client->sess.playerType == PC_MEDIC)
			{
				if (ent->health > client->ps.stats[STAT_MAX_HEALTH] / 1.11)
				{
					ent->health += 2;

					if (ent->health > client->ps.stats[STAT_MAX_HEALTH])
					{
						ent->health = client->ps.stats[STAT_MAX_HEALTH];
					}
				}
				else
				{
					ent->health += 3;
					if (ent->health > client->ps.stats[STAT_MAX_HEALTH] / 1.1)
					{
						ent->health = client->ps.stats[STAT_MAX_HEALTH] / 1.1;
					}
				}
			}

		}
		else if (ent->health > client->ps.stats[STAT_MAX_HEALTH])               // count down health when over max
		{
			ent->health--;
		}
	}
}


/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) {
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;

//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = client->pers.cmd.wbuttons;

	if ( ( client->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) & ( client->oldbuttons ^ client->buttons ) ) ||
		 ( client->wbuttons & WBUTTON_ATTACK2 & ( client->oldwbuttons ^ client->wbuttons ) ) ) {
		client->readyToExit ^= 1;
	}
}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) {
	int i;
	int event;
	gclient_t   *client;
	int damage;
	vec3_t dir;

	client = ent->client;

	if ( oldEventSequence < client->ps.eventSequence - MAX_EVENTS ) {
		oldEventSequence = client->ps.eventSequence - MAX_EVENTS;
	}
	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) {
		event = client->ps.events[ i & ( MAX_EVENTS - 1 ) ];

		switch ( event ) {
		case EV_FALL_NDIE:
			//case EV_FALL_SHORT:
		case EV_FALL_DMG_10:
		case EV_FALL_DMG_15:
		case EV_FALL_DMG_25:
			//case EV_FALL_DMG_30:
		case EV_FALL_DMG_50:
			//case EV_FALL_DMG_75:

			if ( ent->s.eType != ET_PLAYER ) {
				break;      // not in the player model
			}
			if ( g_dmflags.integer & DF_NO_FALLING ) {
				break;
			}
			if ( event == EV_FALL_NDIE ) {
				damage = 9999;
			} else if ( event == EV_FALL_DMG_50 )     {
				damage = 50;
				ent->client->ps.pm_time = 1000;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear( ent->client->ps.velocity );
			} else if ( event == EV_FALL_DMG_25 )     {
				damage = 25;
				ent->client->ps.pm_time = 250;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear( ent->client->ps.velocity );
			} else if ( event == EV_FALL_DMG_15 )     {
				damage = 15;
				ent->client->ps.pm_time = 1000;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear( ent->client->ps.velocity );
			} else if ( event == EV_FALL_DMG_10 )     {
				damage = 10;
				ent->client->ps.pm_time = 1000;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear( ent->client->ps.velocity );
			} else {
				damage = 5; // never used
			}
			VectorSet( dir, 0, 0, 1 );
			ent->pain_debounce_time = level.time + 200; // no normal pain sound
			G_Damage( ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING );
			break;
// JPW NERVE
		case EV_TESTID1:
		case EV_TESTID2:
		case EV_ENDTEST:
			break;
// jpw
		case EV_FIRE_WEAPON_MG42:
			// L0 - disable invincible time when player spawns and starts shooting
			if (g_disableInv.integer)
				ent->client->ps.powerups[PW_INVULNERABLE] = 0;
			// end
			mg42_fire( ent );
			break;

		case EV_FIRE_WEAPON:
		case EV_FIRE_WEAPONB:
		case EV_FIRE_WEAPON_LASTSHOT:
			FireWeapon( ent );
			break;

//----(SA)	modified
		case EV_USE_ITEM1:      // ( HI_MEDKIT )	medkit
		case EV_USE_ITEM2:      // ( HI_WINE )		wine
		case EV_USE_ITEM3:      // ( HI_SKULL )		skull of invulnerable
		case EV_USE_ITEM4:      // ( HI_WATER )		protection from drowning
		case EV_USE_ITEM5:      // ( HI_ELECTRIC )	protection from electric attacks
		case EV_USE_ITEM6:      // ( HI_FIRE )		protection from fire attacks
		case EV_USE_ITEM7:      // ( HI_STAMINA )	restores fatigue bar and sets "nofatigue" for a time period
		case EV_USE_ITEM8:      // ( HI_BOOK1 )
		case EV_USE_ITEM9:      // ( HI_BOOK2 )
		case EV_USE_ITEM10:     // ( HI_BOOK3 )
			UseHoldableItem( ent, event - EV_USE_ITEM0 );
			break;
//----(SA)	end

		default:
			break;
		}
	}

}

// DHM - Nerve
void WolfFindMedic( gentity_t *self ) {
	int i, medic = -1;
	gclient_t   *cl;
	vec3_t start, end, temp;
	trace_t tr;
	float bestdist = 1024, dist;

	self->client->ps.viewlocked_entNum = 0;
	self->client->ps.viewlocked = 0;
	self->client->ps.stats[STAT_DEAD_YAW] = 999;

	// RTCWPro - medcam lock toggle
	if (!self->client->pers.findMedic) {
		return;
	}

	VectorCopy( self->s.pos.trBase, start );
	start[2] += self->client->ps.viewheight;

	for ( i = 0; i < level.numPlayingClients; i++ ) {
		cl = &level.clients[ level.sortedClients[i] ];

		if ( cl->ps.clientNum == self->client->ps.clientNum ) {
			continue;
		}
		if ( cl->sess.sessionTeam != self->client->sess.sessionTeam ) {
			continue;
		}
		if ( cl->ps.stats[ STAT_HEALTH ] <= 0 ) {
			continue;
		}
		if ( cl->ps.stats[ STAT_PLAYER_CLASS ] != PC_MEDIC ) {
			continue;
		}

		VectorCopy( g_entities[level.sortedClients[i]].s.pos.trBase, end );
		end[2] += cl->ps.viewheight;

		trap_Trace( &tr, start, NULL, NULL, end, self->s.number, CONTENTS_SOLID );
		if ( tr.fraction < 0.95 ) {
			continue;
		}

		VectorSubtract( end, start, end );
		dist = VectorNormalize( end );

		if ( dist < bestdist ) {
			medic = cl->ps.clientNum;
			vectoangles( end, temp );
			self->client->ps.stats[STAT_DEAD_YAW] = temp[YAW];
			bestdist = dist;
		}
	}

	if ( medic >= 0 ) {
		self->client->ps.viewlocked_entNum = medic;
		self->client->ps.viewlocked = 7;
	}
}


/*
==============
OSPx - LTinfoMsg

Shows ammo stocks of clients..
==============
*/
char *weaponStr(int weapon)
{
	switch (weapon) {
	case WP_MP40:				return "MP40";
	case WP_THOMPSON:			return "Thompson";
	case WP_STEN:				return "Sten";
	case WP_MAUSER:				return "Mauser";
	case WP_SNIPERRIFLE:		return "Sniper Rifle";
	case WP_FLAMETHROWER:		return "Flamethrower";
	case WP_PANZERFAUST:		return "Panzerfaust";
	case WP_VENOM:				return "Venom";
	case WP_GRENADE_LAUNCHER:	return "Grenade";
	case WP_GRENADE_PINEAPPLE:	return "Grenade";
	case WP_KNIFE:				return "Knife";
	case WP_KNIFE2:				return "Knife";
	case WP_LUGER:				return "Luger";
	case WP_COLT:				return "Colt";
	case WP_MEDIC_SYRINGE:		return "Syringe";
	default:
		return "";
	}
}
// Draw str
void LTinfoMSG(gentity_t *ent) {
	unsigned int current = 0;
	unsigned int stock = 0;
	unsigned int nades = 0;
	weapon_t weapon;

	gentity_t *target;
	trace_t tr;
	vec3_t start, end, forward;

	if (ent->client->ps.stats[STAT_HEALTH] <= 0)
		return;

	if (g_gamestate.integer != GS_PLAYING)
		return;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);

	VectorCopy(ent->s.pos.trBase, start);	//set 'start' to the player's position (plus the viewheight)
	start[2] += ent->client->ps.viewheight;
	VectorMA(start, 512, forward, end);	//put 'end' 512 units forward of 'start'

	//see if we hit anything between 'start' and 'end'
	trap_Trace(&tr, start, NULL, NULL, end, ent->s.number, (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_TRIGGER));

	if (tr.surfaceFlags & SURF_NOIMPACT)	return;
	if (tr.entityNum == ENTITYNUM_WORLD)	return;
	if (tr.entityNum >= MAX_CLIENTS)		return;

	target = &g_entities[tr.entityNum];
	if ((!target->inuse) || (!target->client))		return;
	if (target->client->ps.stats[STAT_HEALTH] <= 0)	return;
	if (!OnSameTeam(target, ent))					return;

	ent->client->infoTime = level.time;
	weapon = target->client->ps.weapon;
	current += target->client->ps.ammoclip[BG_FindClipForWeapon(weapon)];
	stock += target->client->ps.ammo[BG_FindAmmoForWeapon(weapon)];
	nades += target->client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_PINEAPPLE)];
	nades += target->client->ps.ammoclip[BG_FindClipForWeapon(WP_GRENADE_LAUNCHER)];

	if (Q_stricmp(weaponStr(weapon), ""))
	{
		if (weapon == WP_GRENADE_PINEAPPLE || weapon == WP_GRENADE_LAUNCHER)
			CP(va("cp \"%s: %i\n\"1", weaponStr(weapon), current));
		else if (weapon == WP_KNIFE || weapon == WP_KNIFE2)
			CP(va("cp \"%s - Grenades: %i\n\"1", weaponStr(weapon), current, nades));
		else
			CP(va("cp \"%s: %i/%i - Grenades: %i\n\"1", weaponStr(weapon), current, stock, nades));
	}
}

/*
==================
CG_SwingAngles
==================
*/
void G_SwingAngles(float destination, float swingTolerance, float clampTolerance, float speed, float* angle, qboolean* swinging, int msec) {
	float	swing;
	float	move;
	float	scale;

#define	SWING_RIGHT	1
#define SWING_LEFT	2

	if (!*swinging) {
		// see if a swing should be started
		swing = AngleSubtract(*angle, destination);
		if (swing > swingTolerance || swing < -swingTolerance) {
			*swinging = qtrue;
		}
	}

	if (!*swinging) {
		return;
	}

	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract(destination, *angle);
	scale = Q_fabs(swing);
	scale *= 0.05;
	if (scale < 0.5)
		scale = 0.5;

	// swing towards the destination angle
	if (swing >= 0) {
		move = msec * scale * speed;
		if (move >= swing) {
			move = swing;
			*swinging = qfalse;
		}
		else {
			*swinging = SWING_LEFT;		// left
		}
		*angle = AngleMod(*angle + move);
	}
	else if (swing < 0) {
		move = msec * scale * -speed;
		if (move <= swing) {
			move = swing;
			*swinging = qfalse;
		}
		else {
			*swinging = SWING_RIGHT;	// right
		}
		*angle = AngleMod(*angle + move);
	}

	// clamp to no more than tolerance
	swing = AngleSubtract(destination, *angle);
	if (swing > clampTolerance) {
		*angle = AngleMod(destination - (clampTolerance - 1));
	}
	else if (swing < -clampTolerance) {
		*angle = AngleMod(destination + (clampTolerance - 1));
	}
}

/*
==============
SendPendingPredictableEvents
==============

void SendPendingPredictableEvents( playerState_t *ps ) {
	gentity_t *t;
	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
}
*/

/*
===============
G_PlayerAngles

Handles seperate torso motion

legs pivot based on direction of movement

head always looks exactly at cent->lerpAngles

if motion < 20 degrees, show in head only
if < 45 degrees, also show in torso
===============
*/
void G_PlayerAngles(gentity_t* ent, int msec) {
	vec3_t		legsAngles, torsoAngles, headAngles;
	float		dest;
	vec3_t		velocity;
	float		speed;
	float		clampTolerance;
	int			legsSet, torsoSet;

#define SWING_SPEED 0.1f

	legsSet = ent->s.legsAnim & ~ANIM_TOGGLEBIT;
	torsoSet = ent->s.torsoAnim & ~ANIM_TOGGLEBIT;

	VectorCopy(ent->s.apos.trBase, headAngles);
	headAngles[YAW] = AngleMod(headAngles[YAW]);
	VectorClear(legsAngles);
	VectorClear(torsoAngles);

	// --------- yaw -------------

	// Is the concept of "yawing" and "pitching" needed?
	// allow yaw to drift a bit, unless these conditions don't allow them
	if (!(BG_GetConditionValue(ent->s.number, ANIM_COND_MOVETYPE, qfalse) & ((1 << ANIM_MT_IDLE) | (1 << ANIM_MT_IDLECR)))) {
		ent->client->torsoYawing = qtrue;	// always center
		ent->client->torsoPitching = qtrue;	// always center
		ent->client->legsYawing = qtrue;	// always center
	}
	else if (BG_GetConditionValue(ent->s.number, ANIM_COND_FIRING, qtrue)) {
		ent->client->torsoYawing = qtrue;	// always center
		ent->client->torsoPitching = qtrue;	// always center
	}

	// adjust legs for movement dir
	if (ent->s.eFlags & EF_DEAD) {
		// don't let dead bodies twitch
		legsAngles[YAW] = headAngles[YAW];
		torsoAngles[YAW] = headAngles[YAW];
	}
	else {
		legsAngles[YAW] = headAngles[YAW] + ent->s.angles2[YAW];

		if (ent->s.eFlags & EF_NOSWINGANGLES) {
			legsAngles[YAW] = torsoAngles[YAW] = headAngles[YAW];	// always face firing direction
			clampTolerance = 60;
		}
		else if (!(ent->s.eFlags & EF_FIRING)) {
			torsoAngles[YAW] = headAngles[YAW] + 0.35 * ent->s.angles2[YAW];
			clampTolerance = 90;
		}
		else {	// must be firing
			torsoAngles[YAW] = headAngles[YAW];	// always face firing direction
												//if (Q_fabs(cent->currentState.angles2[YAW]) > 30)
												//	legsAngles[YAW] = headAngles[YAW];
			clampTolerance = 60;
		}

		// torso
		G_SwingAngles(torsoAngles[YAW], 25, clampTolerance, SWING_SPEED, &ent->client->torsoYawAngle, &ent->client->torsoYawing, msec);

		// if the legs are yawing (facing heading direction), allow them to rotate a bit, so we don't keep calling
		// the legs_turn animation while an AI is firing, and therefore his angles will be randomizing according to their accuracy

		clampTolerance = 150;

		if (BG_GetConditionValue(ent->s.number, ANIM_COND_MOVETYPE, qfalse) & (1 << ANIM_MT_IDLE))
		{
			ent->client->legsYawing = qfalse; // set it if they really need to swing
			G_SwingAngles(legsAngles[YAW], 20, clampTolerance, 0.5 * SWING_SPEED, &ent->client->legsYawAngle, &ent->client->legsYawing, msec);
		}
		else
			//if	( BG_GetConditionValue( ci->clientNum, ANIM_COND_MOVETYPE, qfalse ) & ((1<<ANIM_MT_STRAFERIGHT)|(1<<ANIM_MT_STRAFELEFT)) )
			if (strstr(BG_GetAnimString(ent->s.number, legsSet), "strafe"))
			{
				ent->client->legsYawing = qfalse; // set it if they really need to swing
				legsAngles[YAW] = headAngles[YAW];
				G_SwingAngles(legsAngles[YAW], 0, clampTolerance, SWING_SPEED, &ent->client->legsYawAngle, &ent->client->legsYawing, msec);
			}
			else
				if (ent->client->legsYawing)
				{
					G_SwingAngles(legsAngles[YAW], 0, clampTolerance, SWING_SPEED, &ent->client->legsYawAngle, &ent->client->legsYawing, msec);
				}
				else
				{
					G_SwingAngles(legsAngles[YAW], 40, clampTolerance, SWING_SPEED, &ent->client->legsYawAngle, &ent->client->legsYawing, msec);
				}

		torsoAngles[YAW] = ent->client->torsoYawAngle;
		legsAngles[YAW] = ent->client->legsYawAngle;
	}

	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if (headAngles[PITCH] > 180) {
		dest = (-360 + headAngles[PITCH]) * 0.75;
	}
	else {
		dest = headAngles[PITCH] * 0.75;
	}
	G_SwingAngles(dest, 15, 30, 0.1, &ent->client->torsoPitchAngle, &ent->client->torsoPitching, msec);
	torsoAngles[PITCH] = ent->client->torsoPitchAngle;

	// --------- roll -------------

	// lean towards the direction of travel
	VectorCopy(ent->s.pos.trDelta, velocity);
	speed = VectorNormalize(velocity);
	if (speed) {
		vec3_t	axis[3];
		float	side;

		speed *= 0.05;

		AnglesToAxis(legsAngles, axis);
		side = speed * DotProduct(velocity, axis[1]);
		legsAngles[ROLL] -= side;

		side = speed * DotProduct(velocity, axis[0]);
		legsAngles[PITCH] += side;
	}

	// We don't care about the minute changs pain twitch inflict.
	/*
	// pain twitch
	CG_AddPainTwitch(cent, torsoAngles);
	*/

	// pull the angles back out of the hierarchial chain
	AnglesSubtract(headAngles, torsoAngles, headAngles);
	AnglesSubtract(torsoAngles, legsAngles, torsoAngles);
	AnglesToAxis(legsAngles, ent->client->animationInfo.legsAxis);
	AnglesToAxis(torsoAngles, ent->client->animationInfo.torsoAxis);
	AnglesToAxis(headAngles, ent->client->animationInfo.headAxis);
}

void G_PlayerAnimation(gentity_t* ent) {
	int legsSet, torsoSet;
	animModelInfo_t* modelInfo = NULL;

	modelInfo = BG_ModelInfoForClient(ent->s.number);
	if (!modelInfo) {
		return;
	}

	legsSet = ent->s.legsAnim & ~ANIM_TOGGLEBIT;
	torsoSet = ent->s.torsoAnim & ~ANIM_TOGGLEBIT;

	ent->client->animationInfo.legsFrame = modelInfo->animations[legsSet].firstFrame + modelInfo->animations[legsSet].numFrames - 1;
	ent->client->animationInfo.torsoFrame = modelInfo->animations[torsoSet].firstFrame + modelInfo->animations[torsoSet].numFrames - 1;
}


//void limbo( gentity_t *ent, qboolean makeCorpse ); // JPW NERVE
void reinforce( gentity_t *ent ); // JPW NERVE

void ClientDamage( gentity_t *clent, int entnum, int enemynum, int id );        // NERVE - SMF

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
int teamRespawnTime(int team, qboolean warmup); // OSPx
void ClientThink_real( gentity_t *ent ) {
	gclient_t   *client;
	pmove_t pm;
//	vec3_t		oldOrigin;
	int oldEventSequence;
	int msec;
	usercmd_t   *ucmd;
	int monsterslick = 0;
// JPW NERVE
	int i;
	vec3_t muzzlebounce;
	gitem_t *item;
	gentity_t *ent2;
	vec3_t velocity, org, offset;
	vec3_t angles,mins,maxs;
	int weapon;
	trace_t tr;
// jpw
	int pclass;

	// Rafael wolfkick
	//int			validkick;
	//static int	wolfkicktimer = 0;

	client = ent->client;
	pclass = client->ps.stats[STAT_PLAYER_CLASS];

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if ( client->pers.connected != CON_CONNECTED ) {
		return;
	}

	// RTCWPro
	if (g_broadcastClients.integer)
	{
		ent->r.svFlags |= SVF_BROADCAST;
	}
	// RTCWPro - end

	if ( client->cameraPortal ) {
		G_SetOrigin( client->cameraPortal, client->ps.origin );
		trap_LinkEntity( client->cameraPortal );
		VectorCopy( client->cameraOrigin, client->cameraPortal->s.origin2 );
	}


	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	ent->client->ps.identifyClient = ucmd->identClient;     // NERVE - SMF

	if (g_antilag.integer == 2) // Unlagged
	{
		//Here comes the unlagged bit!
		//unlagged - backward reconciliation #4
		// frameOffset should be about the number of milliseconds into a frame 
		// this command packet was received, depending on how fast the server
		// does a G_RunFrame()
		client->frameOffset = trap_Milliseconds() - level.frameStartTime;


		//unlagged - lag simulation #3
			// if the client wants to simulate outgoing packet loss
		/*if ( client->pers.plOut ) {
			// see if a random value is below the threshhold
			float thresh = (float)client->pers.plOut / 100.0f;
			if ( random() < thresh ) {
				// do nothing at all if it is - this is a lost command
				return;
			}
		}*/
		//unlagged - lag simulation #3

		//unlagged - lag simulation #2
		// keep a queue of past commands
		/*client->pers.cmdqueue[client->pers.cmdhead] = client->pers.cmd;
		client->pers.cmdhead++;
		if ( client->pers.cmdhead >= MAX_LATENT_CMDS ) {
			client->pers.cmdhead -= MAX_LATENT_CMDS;
		}

		// if the client wants latency in commands (client-to-server latency)
		if ( client->pers.latentCmds ) {
			// save the actual command time
			int time = ucmd->serverTime;

			// find out which index in the queue we want
			int cmdindex = client->pers.cmdhead - client->pers.latentCmds - 1;
			while ( cmdindex < 0 ) {
				cmdindex += MAX_LATENT_CMDS;
			}

			// read in the old command
			client->pers.cmd = client->pers.cmdqueue[cmdindex];

			// adjust the real ping to reflect the new latency
			client->pers.realPing += time - ucmd->serverTime;
		}*/


		//unlagged - backward reconciliation #4
		// save the command time *before* pmove_fixed messes with the serverTime,
		// and *after* lag simulation messes with it :)
		// attackTime will be used for backward reconciliation later (time shift)
		client->attackTime = ucmd->serverTime;


		//unlagged - smooth clients #1
		// keep track of this for later - we'll use this to decide whether or not
		// to send extrapolated positions for this client
		client->lastUpdateFrame = level.framenum;

		//unlagged - lag simulation #1
		// if the client is adding latency to received snapshots (server-to-client latency)
		/*if ( client->pers.latentSnaps ) {
			// adjust the real ping
			client->pers.realPing += client->pers.latentSnaps * (1000 / sv_fps.integer);
			// adjust the attack time so backward reconciliation will work
			client->attackTime -= client->pers.latentSnaps * (1000 / sv_fps.integer);
		}*/
	}

	// RTCWPro
	if (g_alternatePing.integer) 
	{
		// ratmod trueping port
		// we use level.previousTime to account for 50ms lag correction
		// besides, this will turn out numbers more like what players are used to
		client->pers.pingsamples[client->pers.pingsample_counter % NUM_PING_SAMPLES] = level.previousTime + client->frameOffset - ucmd->serverTime;
		client->pers.pingsample_counter++;

		int i, sum = 0;
		int num = client->pers.pingsample_counter > NUM_PING_SAMPLES ? NUM_PING_SAMPLES : client->pers.pingsample_counter;

		// get an average of the samples we saved up
		for (i = 0; i < num; i++) {
			sum += client->pers.pingsamples[i];
		}

		client->pers.alternatePing = sum / num;

		if (client->pers.alternatePing < 0) 
		{
			client->pers.alternatePing = 0;
		}
	}
	// RTCWPro end

// JPW NERVE -- update counter for capture & hold display
	if ( g_gametype.integer == GT_WOLF_CPH ) {
		client->ps.stats[STAT_CAPTUREHOLD_RED] = level.capturetimes[TEAM_RED];
		client->ps.stats[STAT_CAPTUREHOLD_BLUE] = level.capturetimes[TEAM_BLUE];
	}
// jpw

	// sanity check the command time to prevent speedup cheating
	if (ucmd->serverTime > level.time + 200 && !G_DoAntiwarp(ent)) // RTCWPro
	{
		ucmd->serverTime = level.time + 200;
		//		G_Printf("serverTime <<<<<\n" );
	}

	if (ucmd->serverTime < level.time - 1000 && !G_DoAntiwarp(ent)) // RTCWPro
	{
		ucmd->serverTime = level.time - 1000;
		//		G_Printf("serverTime >>>>>\n" );
	}

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		return;
		/*
		// Ridah, fixes savegame timing issue
		if (msec < -100) {
			client->ps.commandTime = ucmd->serverTime - 100;
			msec = 100;
		} else {
			return;
		}
		*/
		// done.
	}
	if ( msec > 200 ) {
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set( "pmove_msec", "8" );
	} else if ( pmove_msec.integer > 33 )     {
		trap_Cvar_Set( "pmove_msec", "33" );
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) {
		ucmd->serverTime = ( ( ucmd->serverTime + pmove_msec.integer - 1 ) / pmove_msec.integer ) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//	return;
	}

	//
	// check for exiting intermission
	//
	if ( level.intermissiontime ) {
		ClientIntermissionThink( client );
		return;
	}

	// spectators don't do much
	// DHM - Nerve :: In limbo use SpectatorThink
	if ( client->sess.sessionTeam == TEAM_SPECTATOR || client->ps.pm_flags & PMF_LIMBO ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			return;
		}
		SpectatorThink( ent, ucmd );
		return;
	}

	// L0 - Pause
	if ( ( client->ps.eFlags & EF_VIEWING_CAMERA ) || level.paused != PAUSE_NONE ) {
		ucmd->buttons = 0;
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
		ucmd->wbuttons = 0;

		// freeze player (RELOAD_FAILED still allowed to move/look)
		if ( level.paused != PAUSE_NONE ) {
			client->ps.pm_type = PM_FREEZE;
		} else if ( ( client->ps.eFlags & EF_VIEWING_CAMERA )) {
			VectorClear( client->ps.velocity );
			client->ps.pm_type = PM_FREEZE;
		}
	} else if ( client->noclip ) {
		client->ps.pm_type = PM_NOCLIP;
	} else if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		client->ps.pm_type = PM_DEAD;
	} else {
		client->ps.pm_type = PM_NORMAL;
	} // End
	// JPW NERVE do some time-based muzzle flip -- this never gets touched in single player (see g_weapon.c)
	// #define RIFLE_SHAKE_TIME 150 // JPW NERVE this one goes with the commented out old damped "realistic" behavior below
	#define RIFLE_SHAKE_TIME 300 // per Id request, longer recoil time
	if ( client->sniperRifleFiredTime ) {
		if ( level.time - client->sniperRifleFiredTime > RIFLE_SHAKE_TIME ) {
			client->sniperRifleFiredTime = 0;
		} else {
			VectorCopy( client->ps.viewangles,muzzlebounce );

			// JPW per Id request, longer recoil time
			muzzlebounce[PITCH] -= 2 * cos( 2.5 * ( level.time - client->sniperRifleFiredTime ) / RIFLE_SHAKE_TIME );
			muzzlebounce[YAW] += 0.5*client->sniperRifleMuzzleYaw*cos( 1.0 - ( level.time - client->sniperRifleFiredTime ) * 3 / RIFLE_SHAKE_TIME );
			muzzlebounce[PITCH] -= 0.25 * random() * ( 1.0f - ( level.time - client->sniperRifleFiredTime ) / RIFLE_SHAKE_TIME );
			muzzlebounce[YAW] += 0.5 * crandom() * ( 1.0f - ( level.time - client->sniperRifleFiredTime ) / RIFLE_SHAKE_TIME );
			SetClientViewAngle( ent,muzzlebounce );
		}
	}
	if ( client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC ) {
		if ( level.time > client->ps.powerups[PW_REGEN] + 5000 ) {
			client->ps.powerups[PW_REGEN] = level.time;
		}
	}
	// also update weapon recharge time

	// JPW drop button drops secondary weapon so new one can be picked up
	// TTimo explicit braces to avoid ambiguous 'else'

	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		if ( ucmd->wbuttons & WBUTTON_DROP ) {
			if ( !client->dropWeaponTime  && level.paused == PAUSE_NONE ) {
				client->dropWeaponTime = 1; // just latch it for now

				//if ( ( client->ps.stats[STAT_PLAYER_CLASS] == PC_SOLDIER ) || ( client->ps.stats[STAT_PLAYER_CLASS] == PC_LT ) || (client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC )) {
				// (dropweapon) ADD NEW CVAR (if desired) and include in the following condition to allow medics to drop their weapons
				//if ( ( client->ps.stats[STAT_PLAYER_CLASS] == PC_SOLDIER ) || ( client->ps.stats[STAT_PLAYER_CLASS] == PC_LT ) )
				// RTCWPro - decide what classes can drop weapon
				if (AllowDropForClass(ent, pclass))
				{
					for ( i = 0; i < MAX_WEAPS_IN_BANK_MP; i++ ) {
						weapon = weapBanksMultiPlayer[3][i];
						if ( COM_BitCheck( client->ps.weapons,weapon ) ) {

							item = BG_FindItemForWeapon( weapon );
							VectorCopy( client->ps.viewangles, angles );

							// clamp pitch
							if ( angles[PITCH] < -30 ) {
								angles[PITCH] = -30;
							} else if ( angles[PITCH] > 30 ) {
								angles[PITCH] = 30;
							}

							AngleVectors( angles, velocity, NULL, NULL );
							VectorScale( velocity, 64, offset );
							offset[2] += client->ps.viewheight / 2;
							VectorScale( velocity, 75, velocity );
							velocity[2] += 50 + random() * 35;

							VectorAdd( client->ps.origin,offset,org );

							VectorSet( mins, -ITEM_RADIUS, -ITEM_RADIUS, 0 );
							VectorSet( maxs, ITEM_RADIUS, ITEM_RADIUS, 2 * ITEM_RADIUS );

							trap_Trace( &tr, client->ps.origin, mins, maxs, org, ent->s.number, MASK_SOLID );
							VectorCopy( tr.endpos, org );

							ent2 = LaunchItem( item, org, velocity, client->ps.clientNum );
							COM_BitClear( client->ps.weapons,weapon );

							if ( weapon == WP_MAUSER ) {
								COM_BitClear( client->ps.weapons,WP_SNIPERRIFLE );
							}

							// Clear out empty weapon, change to next best weapon
							G_AddEvent(ent, EV_NOAMMO, 0);

							i = MAX_WEAPS_IN_BANK_MP;
							// show_bug.cgi?id=568
							if ( client->ps.weapon == weapon ) {
								client->ps.weapon = 0;
							}
							ent2->count = client->ps.ammoclip[BG_FindClipForWeapon( weapon )];
							ent2->item->quantity = client->ps.ammoclip[BG_FindClipForWeapon( weapon )];
							client->ps.ammoclip[BG_FindClipForWeapon( weapon )] = 0;
						}
					}
				}
			}
		} else {
			client->dropWeaponTime = 0;
		}
	}

// jpw

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if ( !ClientInactivityTimer( client ) ) {
		return;
	}

	if ( reloading || client->cameraPortal || level.paused != PAUSE_NONE) { // TODO check this against OSPx
		ucmd->buttons = 0;
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
		ucmd->wbuttons = 0;
		ucmd->wolfkick = 0;
		// L0 - Pause
		if ( level.paused != PAUSE_NONE ) {
			client->ps.pm_type = PM_FREEZE;
		// End
		} else if ( client->cameraPortal ) {
			VectorClear( client->ps.velocity );
			client->ps.pm_type = PM_FREEZE;
		}
	// L0 - Pause (TWICE! ...)
	} else if ( level.paused != PAUSE_NONE ) {
		client->ps.pm_type = PM_FREEZE;
	} else if ( client->noclip ) {
		client->ps.pm_type = PM_NOCLIP;
	} else if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		client->ps.pm_type = PM_DEAD;
	} else {
		client->ps.pm_type = PM_NORMAL;
	}

	// set parachute anim condition flag
	BG_UpdateConditionValue( ent->s.number, ANIM_COND_PARACHUTE, ( ent->flags & FL_PARACHUTE ) != 0, qfalse );

	// all playing clients are assumed to be in combat mode
	if ( !client->ps.aiChar ) {
		client->ps.aiState = AISTATE_COMBAT;
	}

	client->ps.gravity = g_gravity.value;

	// set speed
	client->ps.speed = g_speed.value;

	if ( client->ps.powerups[PW_HASTE] ) {
		client->ps.speed *= 1.3;
	}

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	client->currentAimSpreadScale = (float)client->ps.aimSpreadScale / 255.0;

	memset( &pm, 0, sizeof( pm ) );

	pm.ps = &client->ps;
	pm.pmext = &client->pmext;
	pm.cmd = *ucmd;
	pm.oldcmd = client->pers.oldcmd;
	if ( pm.ps->pm_type == PM_DEAD ) {
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
		// DHM-Nerve added:: EF_DEAD is checked for in Pmove functions, but wasn't being set
		//              until after Pmove
		pm.ps->eFlags |= EF_DEAD;
		// dhm-Nerve end
	} else {
		pm.tracemask = MASK_PLAYERSOLID;
	}
	// MrE: always use capsule for AI and player
	//pm.trace = trap_TraceCapsule;//trap_Trace;
	//DHM - Nerve :: We've gone back to using normal bbox traces
	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = ( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	pm.noWeapClips = ( g_dmflags.integer & DF_NO_WEAPRELOAD ) > 0;
	if ( ent->aiCharacter && AICast_NoReload( ent->s.number ) ) {
		pm.noWeapClips = qtrue; // ensure AI characters don't use clips if they're not supposed to.

	}

	// RTCWPro
	if (g_fixedphysicsfps.integer)
	{
		if (g_fixedphysicsfps.integer > 333)
		{
			trap_Cvar_Set("g_fixedphysicsfps", "333");
		}

		pm.fixedphysicsfps = g_fixedphysicsfps.integer;
	}
	// RTCWPro end

	// Ridah
//	if (ent->r.svFlags & SVF_NOFOOTSTEPS)
//		pm.noFootsteps = qtrue;

	VectorCopy( client->ps.origin, client->oldOrigin );

	// NERVE - SMF
	pm.gametype = g_gametype.integer;
	pm.ltChargeTime = g_LTChargeTime.integer;
	pm.soldierChargeTime = g_soldierChargeTime.integer;
	pm.engineerChargeTime = g_engineerChargeTime.integer;
	pm.medicChargeTime = g_medicChargeTime.integer;
	// -NERVE - SMF

	monsterslick = Pmove( &pm );

	// RTCWPro - revive anim bug fix
	if (ent->client->revive_animation_playing)
	{
		if (ent->client->ps.pm_time == 0 || !(ent->client->ps.pm_flags & PMF_TIME_LOCKPLAYER))
		{
			int lock_time_remaining = 2100 - (level.time - ent->client->movement_lock_begin_time);

			if (lock_time_remaining <= 0)
			{
				ent->client->revive_animation_playing = qfalse;
				ent->client->ps.legsTimer = 0;
				ent->client->ps.torsoTimer = 0;
			}
			else
			{
				ent->client->ps.pm_flags |= PMF_TIME_LOCKPLAYER;
				ent->client->ps.pm_time = lock_time_remaining;
			}
		}
	}

	if ( monsterslick && !( ent->flags & FL_NO_MONSTERSLICK ) ) {
		//vec3_t	dir;
		//vec3_t	kvel;
		//vec3_t	forward;
		// TTimo gcc: might be used unitialized in this function
		float angle = 0.0f;
		qboolean bogus = qfalse;

		// NE
		if ( ( monsterslick & SURF_MONSLICK_N ) && ( monsterslick & SURF_MONSLICK_E ) ) {
			angle = 45;
		}
		// NW
		else if ( ( monsterslick & SURF_MONSLICK_N ) && ( monsterslick & SURF_MONSLICK_W ) ) {
			angle = 135;
		}
		// N
		else if ( monsterslick & SURF_MONSLICK_N ) {
			angle = 90;
		}
		// SE
		else if ( ( monsterslick & SURF_MONSLICK_S ) && ( monsterslick & SURF_MONSLICK_E ) ) {
			angle = 315;
		}
		// SW
		else if ( ( monsterslick & SURF_MONSLICK_S ) && ( monsterslick & SURF_MONSLICK_W ) ) {
			angle = 225;
		}
		// S
		else if ( monsterslick & SURF_MONSLICK_S ) {
			angle = 270;
		}
		// E
		else if ( monsterslick & SURF_MONSLICK_E ) {
			angle = 0;
		}
		// W
		else if ( monsterslick & SURF_MONSLICK_W ) {
			angle = 180;
		} else
		{
			bogus = qtrue;
		}
	}

	// server cursor hints
	if ( ent->lastHintCheckTime < level.time ) {
		G_CheckForCursorHints( ent );

		ent->lastHintCheckTime = level.time + FRAMETIME;
	}

	// DHM - Nerve :: Set animMovetype to 1 if ducking
	if ( ent->client->ps.pm_flags & PMF_DUCKED ) {
		ent->s.animMovetype = 1;
	} else {
		ent->s.animMovetype = 0;
	}

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
		ent->r.eventTime = level.time;
	}

	// RTCWPro
	// Ridah, fixes jittery zombie movement
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate(&ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue);
	}
	else {
		BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue);
	}

	/*if (g_thinkStateLevelTime.integer) 
	{
		BG_PlayerStateToEntityStatePro(&ent->client->ps, &ent->s, level.time, qtrue);
	}
	else
	{
		BG_PlayerStateToEntityStatePro(&ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue);
	}*/
	// RTCWPro end

	//SendPendingPredictableEvents( &ent->client->ps );
	
	if ( !( ent->client->ps.eFlags & EF_FIRING ) ) {
		client->fireHeld = qfalse;      // for grapple
	}


	// RTCWPro
	/*if (!g_thinkSnapOrigin.integer) 
	{
		// use the precise origin for linking
		VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);
	}
	else
	{
		// use the snapped origin for linking so it matches client predicted versions
		VectorCopy(ent->s.pos.trBase, ent->r.currentOrigin);
	}*/

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy(ent->s.pos.trBase, ent->r.currentOrigin);
	// RTCWPro

	VectorCopy( pm.mins, ent->r.mins );
	VectorCopy( pm.maxs, ent->r.maxs );

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	G_PlayerAngles(ent, msec);
	G_PlayerAnimation(ent);

	// execute client events
	// L0 - Pause dump
	if ( level.paused == PAUSE_NONE ) {
		ClientEvents( ent, oldEventSequence );
	}

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity( ent );
	if ( !ent->client->noclip ) {
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	// store the client's current position for antilag traces

	// Nobo antilag
	if (g_antilag.integer == 1)
		G_StoreTrail(ent);

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if ( ent->client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons = client->buttons & ~client->oldbuttons;
//	client->latched_buttons |= client->buttons & ~client->oldbuttons;	// FIXME:? (SA) MP method (causes problems for us.  activate 'sticks')

	//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = ucmd->wbuttons;
	client->latched_wbuttons = client->wbuttons & ~client->oldwbuttons;
//	client->latched_wbuttons |= client->wbuttons & ~client->oldwbuttons;	// FIXME:? (SA) MP method

	// Rafael - Activate
	// Ridah, made it a latched event (occurs on keydown only)
	if ( client->latched_buttons & BUTTON_ACTIVATE ) {
		Cmd_Activate_f( ent );
	}

	if ( ent->flags & FL_NOFATIGUE ) {
		ent->client->ps.sprintTime = 20000;
	}


	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {

		// DHM - Nerve
		if ( g_gametype.integer >= GT_WOLF ) {
			WolfFindMedic( ent );
		}
		// dhm - end

		// wait for the attack button to be pressed
		// TODO check this against OSPx which is wayyyyy different
		if ( level.time > client->respawnTime ) {
			// forcerespawn is to prevent users from waiting out powerups
			if ( ( g_gametype.integer != GT_SINGLE_PLAYER ) &&
				 ( g_forcerespawn.integer > 0 ) &&
				 ( ( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000 )  &&
				 ( !( ent->client->ps.pm_flags & PMF_LIMBO ) ) ) { // JPW NERVE
				// JPW NERVE
				if ( g_gametype.integer >= GT_WOLF ) {
					limbo( ent, qtrue );
				} else {
					respawn( ent );
				}
				// jpw
				return;
			}

			// DHM - Nerve :: Single player game respawns immediately as before,
			//				  but in multiplayer, require button press before respawn
			if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
				respawn( ent );
			}
			// NERVE - SMF - we want to only respawn on jump button now
			else if ( ( ucmd->upmove > 0 ) &&
					  ( !( ent->client->ps.pm_flags & PMF_LIMBO ) ) ) { // JPW NERVE
				// JPW NERVE
				if ( g_gametype.integer >= GT_WOLF ) {
					limbo( ent, qtrue );
				} else {
					respawn( ent );
				}
				// jpw
			}
			// dhm - Nerve :: end

			// NERVE - SMF - we want to immediately go to limbo mode if gibbed
			else if ( client->ps.stats[STAT_HEALTH] <= GIB_HEALTH && !( ent->client->ps.pm_flags & PMF_LIMBO ) ) {
				if ( g_gametype.integer >= GT_WOLF ) {
					limbo( ent, qfalse );
				} else {
					respawn( ent );
				}
			}
			// -NERVE - SMF
		} // -OSPx
		return;
	}

	// perform once-a-second actions
	// L0 - Pause dump
	if ( level.paused == PAUSE_NONE ) {
		ClientTimerActions( ent, msec );
	}
}

/*
==================
ClientThink_cmd
==================
*/
void ClientThink_cmd(gentity_t* ent, usercmd_t* cmd) {

	ent->client->pers.oldcmd = ent->client->pers.cmd;
	ent->client->pers.cmd = *cmd;
	ClientThink_real(ent);
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) {
	gentity_t *ent;
	usercmd_t newcmd;

	ent = g_entities + clientNum;

	// RTCWPro - this goes above
	//ent->client->pers.oldcmd = ent->client->pers.cmd;
	// new cmd
	//trap_GetUsercmd( clientNum, &ent->client->pers.cmd );
	trap_GetUsercmd(clientNum, &newcmd);

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.time;

	if (G_DoAntiwarp(ent))
	{
		AW_AddUserCmd(clientNum, &newcmd);
		DoClientThinks(ent);
	}
	else
	{
		ClientThink_cmd(ent, &newcmd);
	}

	/*if ( !g_synchronousClients.integer ) {
		ClientThink_real( ent );
	}*/
}

void G_RunClient( gentity_t *ent ) {

	if (G_DoAntiwarp(ent))
	{
		DoClientThinks(ent);
	}

	if ( !g_synchronousClients.integer )
	{
		return;
	}

	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real( ent );
}

/*
==================
SpectatorClientEndFrame
TODO check this against OSPx its wayyy different
==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) {
	int savedClass;		// NERVE - SMF
	   	int do_respawn = 0; // JPW NERVE
	// if we are doing a chase cam or a remote view, grab the latest info
	if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW || (ent->client->ps.pm_flags & PMF_LIMBO))
	{
		int       clientNum, testtime;
		gclient_t *cl;
	//	qboolean  do_respawn = qfalse;

		// Players can respawn quickly in warmup
		if (g_gamestate.integer != GS_PLAYING && ent->client->respawnTime <= level.timeCurrent &&
		    ent->client->sess.sessionTeam != TEAM_SPECTATOR)
		{
//			do_respawn = qtrue;
			do_respawn = 1;
		}
		else if (ent->client->sess.sessionTeam == TEAM_RED)
		{
			testtime                            = (level.dwRedReinfOffset + level.timeCurrent - level.startTime) % g_redlimbotime.integer;
			//do_respawn                          = (testtime < ent->client->pers.lastReinforceTime);
			if (testtime < ent->client->pers.lastReinforceTime) {
                do_respawn = 1;
			}
			ent->client->pers.lastReinforceTime = testtime;
		}
		else if (ent->client->sess.sessionTeam == TEAM_BLUE)
		{
			testtime                            = (level.dwBlueReinfOffset + level.timeCurrent - level.startTime) % g_bluelimbotime.integer;
			//do_respawn                          = (testtime < ent->client->pers.lastReinforceTime);
			if (testtime < ent->client->pers.lastReinforceTime) {
                do_respawn = 1;
			}
			ent->client->pers.lastReinforceTime = testtime;
		}





		if (do_respawn)
		{
			reinforce(ent);
			return;
		}

		clientNum = ent->client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) {
			clientNum = level.follow1;
		} else if ( clientNum == -2 ) {
			clientNum = level.follow2;
		}

		if ( clientNum >= 0 ) {
			cl = &level.clients[ clientNum ];
			if ((cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR) ||
				(cl->pers.connected == CON_CONNECTED && cl->sess.shoutcaster && ent->client->sess.shoutcaster)) { // RTCWPro
				// L0 - Ping & Score bug fix
				// This solves the /serverstatus and score table (who's specing/demoing you) bug..
				int ping = ent->client->ps.ping;
				int score = ent->client->ps.persistant[PERS_SCORE];
				// DHM - Nerve :: carry flags over
                int	flags = ( cl->ps.eFlags & ~( EF_VOTED ) ) | ( ent->client->ps.eFlags & ( EF_VOTED ) );
				// JPW NERVE -- limbo latch
				if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR && ent->client->ps.pm_flags & PMF_LIMBO ) {
					// abuse do_respawn var
			//		savedScore = ent->client->ps.persistant[PERS_SCORE];


					do_respawn = ent->client->ps.pm_time;


			//		savedRespawns = ent->client->ps.persistant[PERS_RESPAWNS_LEFT];
					savedClass = ent->client->ps.stats[STAT_PLAYER_CLASS];

					ent->client->ps = cl->ps;
					ent->client->ps.pm_flags |= PMF_FOLLOW;
					ent->client->ps.pm_flags |= PMF_LIMBO;

				//	ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = savedRespawns;
					ent->client->ps.pm_time = do_respawn;                           // put pm_time back
				//	ent->client->ps.persistant[PERS_SCORE] = savedScore;            // put score back
					ent->client->ps.stats[STAT_PLAYER_CLASS] = savedClass;          // NERVE - SMF - put player class back
				} else {
					ent->client->ps = cl->ps;
					ent->client->ps.pm_flags |= PMF_FOLLOW;
				}
				// jpw
				// DHM - Nerve :: carry flags over
				ent->client->ps.eFlags = flags;
				// L0 - Ping & Score bug fix
				ent->client->ps.ping = ping;
				ent->client->ps.persistant[PERS_SCORE] = score;
				return;
			} else {
				// drop them to free spectators unless they are dedicated camera followers
				if ( ent->client->sess.spectatorClient >= 0 ) {
					ent->client->sess.spectatorState = SPECTATOR_FREE;
					ClientBegin( ent->client - level.clients );
				}
			}
		}
	}

	if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
	// L0 - Speclock
	ent->client->ps.powerups[PW_BLACKOUT] = ( G_blockoutTeam( ent, TEAM_RED ) * TEAM_RED ) |
											( G_blockoutTeam( ent, TEAM_BLUE ) * TEAM_BLUE );
}


// DHM - Nerve :: After reviving a player, their contents stay CONTENTS_CORPSE until it is determined
//					to be safe to return them to PLAYERSOLID

qboolean StuckInClient( gentity_t *self ) {
	gentity_t *hit;
	vec3_t hitmin, hitmax;
	vec3_t selfmin, selfmax;
	int i;

	hit = &g_entities[0];
	for ( i = 0; i < level.maxclients; i++, hit++ ) {
		if ( !hit->inuse ) {
			continue;
		}
		if ( hit == self ) {
			continue;
		}
		if ( !hit->client ) {
			continue;
		}
		if ( !hit->s.solid ) {
			continue;
		}
		if ( hit->health <= 0 ) {
			continue;
		}

		VectorAdd( hit->r.currentOrigin, hit->r.mins, hitmin );
		VectorAdd( hit->r.currentOrigin, hit->r.maxs, hitmax );
		VectorAdd( self->r.currentOrigin, self->r.mins, selfmin );
		VectorAdd( self->r.currentOrigin, self->r.maxs, selfmax );

		if ( hitmin[0] > selfmax[0] ) {
			continue;
		}
		if ( hitmax[0] < selfmin[0] ) {
			continue;
		}
		if ( hitmin[1] > selfmax[1] ) {
			continue;
		}
		if ( hitmax[1] < selfmin[1] ) {
			continue;
		}
		if ( hitmin[2] > selfmax[2] ) {
			continue;
		}
		if ( hitmax[2] < selfmin[2] ) {
			continue;
		}

		return qtrue;
	}
	return qfalse;
}

extern vec3_t playerMins, playerMaxs;
#define WR_PUSHAMOUNT 25

void WolfRevivePushEnt( gentity_t *self, gentity_t *other ) {
	vec3_t dir, push;

	if (self->client && other->client) {
		// no push in pause state
		if (self->client->ps.pm_type == PM_FREEZE || other->client->ps.pm_type == PM_FREEZE) {
			return;
		}
	}

	// apply push effect only every 50ms to match sv_fps 20 behavior
	// scaling the speed to match higher framerates results in smaller push overall as friction has more effect on lower speeds
	if ((self->client && self->client->lastRevivePushTime + 50 > level.time) 
		|| (other->client && other->client->lastRevivePushTime + 50 > level.time))	{
		return;
	}

	VectorSubtract( self->r.currentOrigin, other->r.currentOrigin, dir );
	dir[2] = 0;
	VectorNormalizeFast( dir );

	VectorScale( dir, WR_PUSHAMOUNT, push );

	if ( self->client ) {
		VectorAdd( self->s.pos.trDelta, push, self->s.pos.trDelta );
		VectorAdd( self->client->ps.velocity, push, self->client->ps.velocity );
		self->client->lastRevivePushTime = level.time - (level.time % 50);
	}

	VectorScale( dir, -WR_PUSHAMOUNT, push );
	push[2] = WR_PUSHAMOUNT / 2;

	VectorAdd( other->s.pos.trDelta, push, other->s.pos.trDelta );
	//VectorAdd( other->client->ps.velocity, push, other->client->ps.velocity );
	if ( other->client ) {
		VectorAdd( other->client->ps.velocity, push, other->client->ps.velocity );
		other->client->lastRevivePushTime = level.time - (level.time % 50);
	}
}

void WolfReviveBbox( gentity_t *self ) {
	int touch[MAX_GENTITIES];
	int num,i, touchnum = 0;
	gentity_t   *hit = NULL;
	vec3_t mins, maxs;
	gentity_t   *capsulehit = NULL;

	VectorAdd( self->r.currentOrigin, playerMins, mins );
	VectorAdd( self->r.currentOrigin, playerMaxs, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// Arnout, we really should be using capsules, do a quick, more refined test using mover collision
	if ( num ) {
		capsulehit = G_TestEntityPosition( self );
	}

	for ( i = 0 ; i < num ; i++ ) {
		hit = &g_entities[touch[i]];
		if ( hit->client ) {
			// ATVI Wolfenstein Misc #467
			// don't look at yourself when counting the hits
			if ( hit->client->ps.persistant[PERS_HWEAPON_USE] && hit != self ) {
				touchnum++;
				// Move corpse directly to the person who revived them
				if ( self->props_frame_state >= 0 ) {
					trap_UnlinkEntity( self );
					VectorCopy( g_entities[self->props_frame_state].client->ps.origin, self->client->ps.origin );
					VectorCopy( self->client->ps.origin, self->r.currentOrigin );
					trap_LinkEntity( self );

					// Reset value so we don't continue to warp them
					self->props_frame_state = -1;

					// reset push velocity, otherwise player will fly away if is velocity too strong
					VectorClear(self->s.pos.trDelta);
					VectorClear(self->client->ps.velocity);
				}
			} else if ( hit->health > 0 ) {
				if ( hit->s.number != self->s.number ) {
					WolfRevivePushEnt( hit, self );
					touchnum++;
				}
			}
		} else if ( hit->r.contents & ( CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_PLAYERCLIP ) )   {
			// Arnout: if hit is a mover, use capsulehit (this will only work if we touch one mover at a time - situations where you hit two are
			// really rare anyway though. The real fix is to move everything to capsule collision detection though
			if ( hit->s.eType == ET_MOVER ) {
				if ( capsulehit && capsulehit != hit ) {
					continue;   // we collided with a mover, but we're not stuck in this one
				} else {
					continue;   // we didn't collide with any movers
				}
			}

			WolfRevivePushEnt( hit, self );
			touchnum++;
		}
	}

	if ( g_dbgRevive.integer ) {
		G_Printf( "WolfReviveBbox: touchnum: %d\n", touchnum );
	}

	if ( touchnum == 0 ) {
		if ( g_dbgRevive.integer ) {
			G_Printf( "WolfReviveBbox:  Player is solid now!\n" );
		}
		self->r.contents = CONTENTS_BODY;
	}
}

// dhm

// RTCWPro - patched for the pub head stuff
void G_DrawHitBoxes(gentity_t* ent) {
	gentity_t* bboxEnt;
	vec3_t b1, b2;

	// Draw body hitbox
	VectorCopy(ent->r.currentOrigin, b1);
	VectorCopy(ent->r.currentOrigin, b2);
	VectorAdd(b1, ent->r.mins, b1);
	VectorAdd(b2, ent->r.maxs, b2);
	bboxEnt = G_TempEntity(b1, EV_RAILTRAIL);
	VectorCopy(b2, bboxEnt->s.origin2);
	bboxEnt->s.dmgFlags = 1;
	bboxEnt->s.otherEntityNum2 = ent->s.number;

	// Draw head hitbox
	UpdateHeadEntity(ent);
	VectorCopy(ent->head->r.currentOrigin, b1);
	VectorCopy(ent->head->r.currentOrigin, b2);
	VectorAdd(b1, ent->head->r.mins, b1);
	VectorAdd(b2, ent->head->r.maxs, b2);
	bboxEnt = G_TempEntity(b1, EV_RAILTRAIL);
	VectorCopy(b2, bboxEnt->s.origin2);
	bboxEnt->s.dmgFlags = 1;
	bboxEnt->s.otherEntityNum2 = ent->s.number;
	RemoveHeadEntity(ent);
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEndFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	int i;

	//unlagged - smooth clients #1
	int frames;
	//unlagged - smooth clients #1

	// RTCWPro
	if (g_alternatePing.integer) 
	{
		if (ent->client->ps.ping >= 999) 
		{
			ent->client->pers.alternatePing = ent->client->ps.ping;
		}
	}
	// RTCWPro end

	// used for informing of speclocked teams.
	// Zero out here and set only for certain specs
	ent->client->ps.powerups[PW_BLACKOUT] = 0;
	if ( ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) || ( ent->client->ps.pm_flags & PMF_LIMBO ) ) { // JPW NERVE
		SpectatorClientEndFrame( ent );
		return;
	}

	if ( !ent->aiCharacter ) {
		// turn off any expired powerups
		for ( i = 0; i < PW_NUM_POWERUPS; i++ ) {

			if ( i == PW_ELECTRIC || // these aren't dependant on level.time
				 i == PW_BREATHER ||
				 i == PW_NOFATIGUE ||
				 i == PW_CAPPEDOBJ || // RtcwPro added for double objective map like radar
				  ent->client->ps.powerups[i] == 0  // L0 - Pause dump
				 ) {

				continue;
			}
			// L0 - Pause dump
			// OSP -- If we're paused, update powerup timers accordingly.
			// Make sure we dont let stuff like CTF flags expire.
			if ( level.paused != PAUSE_NONE &&
				 ent->client->ps.powerups[i] != INT_MAX ) {
				ent->client->ps.powerups[i] += level.time - level.previousTime;
			}

			if ( ent->client->ps.powerups[ i ] < level.time ) {
				ent->client->ps.powerups[ i ] = 0;
			}
		}
	}
	// L0 - Pause dump
	// OSP - If we're paused, make sure other timers stay in sync
	//		--> Any new things in ET we should worry about?
	if ( level.paused != PAUSE_NONE ) {
		int time_delta = level.time - level.previousTime;

		ent->client->airOutTime += time_delta;
		ent->client->inactivityTime += time_delta;
		ent->client->lastBurnTime += time_delta;
		ent->client->pers.connectTime += time_delta;
		ent->client->pers.enterTime += time_delta;
		ent->client->pers.teamState.lastreturnedflag += time_delta;
		ent->client->pers.teamState.lasthurtcarrier += time_delta;
		ent->client->pers.teamState.lastfraggedcarrier += time_delta;
		ent->client->ps.classWeaponTime += time_delta;
		ent->client->respawnTime += time_delta;
		ent->client->sniperRifleFiredTime += time_delta;
		ent->lastHintCheckTime += time_delta;
		ent->pain_debounce_time += time_delta;
		ent->s.onFireEnd += time_delta;
	}

	// save network bandwidth
#if 0
	if ( !g_synchronousClients->integer && ent->client->ps.pm_type == PM_NORMAL ) {
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear( ent->client->ps.viewangles );
	}
#endif

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) {
		return;
	}

	// burn from lava, etc
	P_WorldEffects( ent );

	// apply all the damage taken this frame
	P_DamageFeedback( ent );

	// ET Legacy port
	// increases stats[STAT_MAX_HEALTH] based on # of medics in game
	// AddMedicTeamBonus() now adds medic team bonus and stores in ps.stats[STAT_MAX_HEALTH].
	AddMedicTeamBonus(ent->client);

	// all players are init in game, we can set properly starting health
	if (level.startTime == level.time - (GAME_INIT_FRAMES * FRAMETIME))
	{
		ent->health = ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH];

		if (ent->client->sess.playerType == PC_MEDIC)
		{
			ent->health = ent->client->ps.stats[STAT_HEALTH] /= 1.12;
			if (ent->health > 140)
				ent->health = 140;
		}
	}
	else
	{
		ent->client->ps.stats[STAT_HEALTH] = ent->health;
	}
	// End ETL port

	ent->client->ps.stats[STAT_HEALTH] = ent->health;   // FIXME: get rid of ent->health...

	G_SetClientSound( ent );

	// set the latest infor

	if (g_antilag.integer < 2) // Nobo antilag or off
	{
		// add the EF_CONNECTION flag if we haven't gotten commands recently
		if (level.time - ent->client->lastCmdTime > 1000) {
			ent->s.eFlags |= EF_CONNECTION;
		}
		else {
			ent->s.eFlags &= ~EF_CONNECTION;
		}

		// RTCWPro
		// Ridah, fixes jittery zombie movement
		if (g_smoothClients.integer) {
			BG_PlayerStateToEntityStateExtraPolate(&ent->client->ps, &ent->s, ent->client->ps.commandTime, ((ent->r.svFlags & SVF_CASTAI) == 0));
		}
		else {
			BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, ((ent->r.svFlags & SVF_CASTAI) == 0));
		}
	}
	else if (g_antilag.integer == 2) // Unlagged
	{
		BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue);

		//SendPendingPredictableEvents( &ent->client->ps );
	
		//unlagged - smooth clients #1
			// mark as not missing updates initially
		ent->client->ps.eFlags &= ~EF_CONNECTION;

		// see how many frames the client has missed
		frames = level.framenum - ent->client->lastUpdateFrame - 1;

		// don't extrapolate more than two frames
		if (frames > g_maxExtrapolatedFrames.integer) {
			frames = g_maxExtrapolatedFrames.integer;

			// if they missed more than two in a row, show the phone jack
			ent->client->ps.eFlags |= EF_CONNECTION;
			ent->s.eFlags |= EF_CONNECTION;
		}

		// did the client miss any frames?
		if (frames > 0 && g_smoothClients.integer) {
			// yep, missed one or more, so extrapolate the player's movement
			G_PredictPlayerMove(ent, (float)frames / sv_fps.integer);
			// save network bandwidth
			SnapVector(ent->s.pos.trBase);
		}
		//unlagged - smooth clients #1

		//unlagged - backward reconciliation #1
		// store the client's position for backward reconciliation later
		G_StoreHistory(ent);
		//unlagged - backward reconciliation #1

	}



	/*if (g_endStateLevelTime.integer) 
	{
		BG_PlayerStateToEntityStatePro(&ent->client->ps, &ent->s, level.time, qfalse);
	}
	else
	{
		BG_PlayerStateToEntityStatePro(&ent->client->ps, &ent->s, ent->client->ps.commandTime, qfalse);
	}*/
	// RTCWPro end

	//SendPendingPredictableEvents( &ent->client->ps );

	// DHM - Nerve :: If it's been a couple frames since being revived, and props_frame_state
	//					wasn't reset, go ahead and reset it
	if ( ent->props_frame_state >= 0 && ( ( level.time - ent->s.effect3Time ) > 100 ) ) {
		ent->props_frame_state = -1;
	}

	if ( ent->health > 0 && StuckInClient( ent ) ) {
		G_DPrintf( "%s is stuck in a client.\n", ent->client->pers.netname );
		ent->r.contents = CONTENTS_CORPSE;
	}

	if ( g_gametype.integer >= GT_WOLF && ent->health > 0 && ent->r.contents == CONTENTS_CORPSE ) {
		WolfReviveBbox( ent );
	}

	// DHM - Nerve :: Reset 'count2' for flamethrower
	if ( !( ent->client->buttons & BUTTON_ATTACK ) ) {
		ent->count2 = 0;
	}
	// dhm

	if (ent->client->pers.drawHitBoxes && g_drawHitboxes.integer && ent->health > 0) {
		G_DrawHitBoxes(ent);
	}
}
