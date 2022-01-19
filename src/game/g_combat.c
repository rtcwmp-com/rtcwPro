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
 * name:		g_combat.c
 *
 * desc:
 *
*/

#include "g_local.h"


/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, int score ) {
	if ( !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup
	if (g_gamestate.integer != GS_PLAYING) {
		return;
	}

	// Ridah, no scoring during single player
	// DHM - Nerve :: fix typo
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		return;
	}
	// done.


	ent->client->ps.persistant[PERS_SCORE] += score;
	if ( g_gametype.integer >= GT_TEAM ) { // JPW NERVE changed to >=
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	}
	CalculateRanks();
}

/*
=================
Droppable weapons and FindIndex for TossClientItems
=================
*/
int droppableWeapons[] = {
	WP_MP40, WP_THOMPSON, WP_STEN, WP_MAUSER, WP_PANZERFAUST, WP_FLAMETHROWER, WP_COLT, WP_LUGER
};
int FindIndex(int a[], int num_elements, int value)
{
	int i;
	for (i = 0; i < num_elements; i++)
	{
		if (a[i] == value)
		{
			return(value);
		}
	}
	return(-1);
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	gitem_t     *item;
	int weapon;
	gentity_t   *drop = 0;
	int pclass = self->client->ps.stats[STAT_PLAYER_CLASS];

	// drop the weapon if not a gauntlet or machinegun
	if (pclass != PC_MEDIC) { // RTCWPro - not for medics
		weapon = self->s.weapon;
	}
	else {
		weapon = WP_NONE;
	}

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.

	// (SA) always drop what you were switching to
	if ( 1 ) {
		if ( self->client->ps.weaponstate == WEAPON_DROPPING ) {
			weapon = self->client->pers.cmd.weapon;
		}
		if ( !( COM_BitCheck( self->client->ps.weapons, weapon ) ) ) {
			weapon = WP_NONE;
		}
	}

	// JPW NERVE don't drop these weapon types
	if ( ( weapon == WP_FLAMETHROWER ) || ( weapon == WP_GARAND ) || ( weapon == WP_MAUSER ) || ( weapon == WP_VENOM ) ) {
		weapon = WP_NONE;
	}
	// jpw

	if ( weapon > WP_NONE && FindIndex(droppableWeapons, ARRAY_LEN(droppableWeapons), weapon) >= 0 && self->client->ps.ammo[ BG_FindAmmoForWeapon( weapon )] ) { // < WP_MONSTER_ATTACK1 replaced
		// find the item type for this weapon
		item = BG_FindItemForWeapon( weapon );
		// spawn the item

		// Rafael
		if ( !( self->client->ps.persistant[PERS_HWEAPON_USE] ) ) {
			drop = Drop_Item( self, item, 0, qfalse );
			// JPW NERVE -- fix ammo counts
			drop->count = self->client->ps.ammoclip[BG_FindClipForWeapon( weapon )];
			drop->item->quantity = self->client->ps.ammoclip[BG_FindClipForWeapon( weapon )];
			// jpw
		}
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t dir;
	vec3_t angles;

	if ( attacker && attacker != self ) {
		VectorSubtract( attacker->s.pos.trBase, self->s.pos.trBase, dir );
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract( inflictor->s.pos.trBase, self->s.pos.trBase, dir );
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw( dir );

	angles[YAW] = vectoyaw( dir );
	angles[PITCH] = 0;
	angles[ROLL] = 0;
}


/*
==============
GibHead
==============
*/
void GibHead( gentity_t *self, int killer ) {
	G_AddEvent( self, EV_GIB_HEAD, killer );
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer ) {
	gentity_t *other = &g_entities[killer];
	vec3_t dir;

	VectorClear( dir );
	if ( other->inuse ) {
		if ( other->client ) {
			VectorSubtract( self->r.currentOrigin, other->r.currentOrigin, dir );
			VectorNormalize( dir );
		} else if ( !VectorCompare( other->s.pos.trDelta, vec3_origin ) ) {
			VectorNormalize2( other->s.pos.trDelta, dir );
		}
	}

	G_AddEvent( self, EV_GIB_PLAYER, DirToByte( dir ) );
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	if ( self->health > GIB_HEALTH ) {
		return;
	}

	GibEntity( self, 0 );
}


// these are just for logging, the client prints its own messages
char    *modNames[] = {
	"MOD_UNKNOWN",
	"MOD_SHOTGUN",
	"MOD_GAUNTLET",
	"MOD_MACHINEGUN",
	"MOD_GRENADE",
	"MOD_GRENADE_SPLASH",
	"MOD_ROCKET",
	"MOD_ROCKET_SPLASH",
	"MOD_RAILGUN",
	"MOD_LIGHTNING",
	"MOD_BFG",
	"MOD_BFG_SPLASH",
	"MOD_KNIFE",
	"MOD_KNIFE2",
	"MOD_KNIFE_STEALTH",
	"MOD_LUGER",
	"MOD_COLT",
	"MOD_MP40",
	"MOD_THOMPSON",
	"MOD_STEN",
	"MOD_MAUSER",
	"MOD_SNIPERRIFLE",
	"MOD_GARAND",
	"MOD_SNOOPERSCOPE",
	"MOD_SILENCER", //----(SA)
	"MOD_AKIMBO",    //----(SA)
	"MOD_BAR",   //----(SA)
	"MOD_FG42",
	"MOD_FG42SCOPE",
	"MOD_PANZERFAUST",
	"MOD_ROCKET_LAUNCHER",
	"MOD_GRENADE_LAUNCHER",
	"MOD_VENOM",
	"MOD_VENOM_FULL",
	"MOD_FLAMETHROWER",
	"MOD_TESLA",
	"MOD_SPEARGUN",
	"MOD_SPEARGUN_CO2",
	"MOD_GRENADE_PINEAPPLE",
	"MOD_CROSS",
	"MOD_MORTAR",
	"MOD_MORTAR_SPLASH",
	"MOD_KICKED",
	"MOD_GRABBER",
	"MOD_DYNAMITE",
	"MOD_DYNAMITE_SPLASH",
	"MOD_AIRSTRIKE",
	"MOD_SYRINGE",
	"MOD_AMMO",
	"MOD_ARTILLERY",
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
	"MOD_GRAPPLE",
	"MOD_EXPLOSIVE",
	"MOD_POISONGAS",
	"MOD_ZOMBIESPIT",
	"MOD_ZOMBIESPIT_SPLASH",
	"MOD_ZOMBIESPIRIT",
	"MOD_ZOMBIESPIRIT_SPLASH",
	"MOD_LOPER_LEAP",
	"MOD_LOPER_GROUND",
	"MOD_LOPER_HIT",
// JPW NERVE
	"MOD_LT_ARTILLERY",
	"MOD_LT_AIRSTRIKE",
	"MOD_ENGINEER",  // not sure if we'll use
	"MOD_MEDIC",     // these like this or not
	"MOD_BAT",
// jpw
// OSPx
	"MOD_ADMKILL",
	"MOD_SELFKILL",
	"MOD_SWITCHTEAM",
	"MOD_NUM_MODS"
// -OSPx
};

/*
==================
player_die
==================
*/
void limbo( gentity_t *ent, qboolean makeCorpse ); // JPW NERVE

void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	gentity_t   *ent;
	// TTimo might be used uninitialized
	int contents = 0;
	int killer;
	int i;
	char        *killerName, *obit;
	qboolean nogib = qtrue;
	gitem_t     *item = NULL; // JPW NERVE for flag drop
	vec3_t launchvel,launchspot;      // JPW NERVE
	gentity_t   *flag; // JPW NERVE

	if ( self->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if ( level.intermissiontime ) {
		return;
	}


	// L0 - OSP - death stats handled out-of-band of G_Damage for external calls
	G_addStats( self, attacker, damage, meansOfDeath );
	// OSP
	self->client->ps.pm_type = PM_DEAD;

	G_AddEvent( self, EV_STOPSTREAMINGSOUND, 0 );

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( meansOfDeath == MOD_SELFKILL && g_gamestate.integer == GS_PLAYING) {
		int r = rand() %2; // randomize messages

		if (r == 0)
			AP(va("print \"%s ^7slit his throat.\n\"", self->client->pers.netname));
		else if (r == 1)
			AP(va("print \"%s ^7commited suicide.\n\"", self->client->pers.netname));
	}
	/* new stats if (meansOfDeath == MOD_KNIFE2 && g_gamestate.integer == GS_PLAYING) {
		attacker->client->pers.stats.knife++;
	}*/

	// If person gets stabbed use custom sound from soundpack
	// it's broadcasted to victim and heard only if standing near victim...
	if ( (meansOfDeath == MOD_KNIFE_STEALTH || meansOfDeath == MOD_KNIFE) && !OnSameTeam(self, attacker) && g_fastStabSound.integer > 0) {
		int r = rand() %2;
		char *snd = "goat.wav"; // default

		if (r == 0 || g_fastStabSound.integer == 1)
			snd = "goat.wav";
		else if (r == 1 || g_fastStabSound.integer == 2)
			snd = "humiliation.wav";

		APS(va("sound/match/%s", snd));

		//APRS(self, va("sound/match/%s", ((g_fastStabSound.integer == 1) ? "goat.wav" :
		//	((g_fastStabSound.integer == 2) ? "humiliation.wav" : snd)	)));

		attacker->client->sess.knifeKills++;
		//write_RoundStats(attacker->client->pers.netname, attacker->client->pers.stats.knifeStealth, ROUND_FASTSTABS);
	}

	// RtcwPro commented this out - we want artillery distinguished from airstrike
	//if (meansOfDeath == MOD_ARTILLERY && g_gamestate.integer == GS_PLAYING)  {
	//	meansOfDeath = MOD_AIRSTRIKE; // Just Remaps it back..
	//}

	if ( meansOfDeath < 0 || meansOfDeath >= sizeof( modNames ) / sizeof( modNames[0] ) ) {
		obit = "<bad obituary>";
	} else {
		obit = modNames[ meansOfDeath ];
	}

	// L0 - Don't bother in warmup etc..
	if (g_gamestate.integer == GS_PLAYING) // this is only on server side not needed during warmup
	{
		G_LogPrintf( "Kill: %i %i %i: %s killed %s by %s\n",
				 killer, self->s.number, meansOfDeath, killerName,
				 self->client->pers.netname, obit );
        // GameStats logging .... probably want to control this via cvar (as well as do a better job in general)
        if (g_gameStatslog.integer) {
            if (killer == self->s.number) {
                 G_writeGeneralEvent(self,self,obit,eventSuicide);
            }
            else if (OnSameTeam(attacker, self)) {
                if ( attacker->client ) {
                    G_writeGeneralEvent(attacker,self,obit,eventTeamkill);
                }
            }
            else {
                if ( attacker->client ) {
                    int weapID;
                    weapID = G_weapStatIndex_MOD( meansOfDeath );
                    //G_writeGeneralEvent(attacker,self,obit,eventKill);
					G_writeGeneralEvent(attacker, self, va("%s", aWeaponInfo[weapID].pszName), eventKill);
                }
            }

        }
	}
	// L0 - Stats
	if (attacker && attacker->client && g_gamestate.integer == GS_PLAYING) {
		// Life kills & death spress
		if (!OnSameTeam(attacker, self))
		{
			// attacker->client->pers.spreeDeaths = 0; // Reset deaths for death spress  // nihi commented out
			attacker->client->pers.life_kills++;		// life kills
		} // End
	}

	//if (g_gamestate.integer == GS_PLAYING) { // euro guys want this during warmup like OSP

	// broadcast the death event to everyone
	ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
	ent->s.eventParm = meansOfDeath;
	ent->s.otherEntityNum = self->s.number;
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST; // send to everyone

	//}

	self->enemy = attacker;

	self->client->ps.persistant[PERS_KILLED]++; // TODO find out what PERS_KILLED is used for????

// JPW NERVE -- if player is holding ticking grenade, drop it
	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		if ( ( self->client->ps.grenadeTimeLeft ) && ( self->s.weapon != WP_DYNAMITE ) ) {
			launchvel[0] = crandom();
			launchvel[1] = crandom();
			launchvel[2] = random();
			VectorScale( launchvel, 160, launchvel );
			VectorCopy( self->r.currentOrigin, launchspot );
			launchspot[2] += 40;
			fire_grenade( self, launchspot, launchvel, self->s.weapon )->damage = 0;
			self->client->ps.ammoclip[BG_FindClipForWeapon(self->s.weapon)] -= ammoTable[self->s.weapon].uses;
			
			// RtcwPro Issue #345 Clear out empty weapon, change to next best weapon
			//PM_SwitchIfEmpty();
			if (self->client->ps.ammoclip[BG_FindClipForWeapon(self->s.weapon)] == 0)
				G_AddEvent(self, EV_NOAMMO, 0);
		}
	}
// jpw

	if ( attacker && attacker->client ) {
		if ( attacker == self || OnSameTeam( self, attacker ) ) {

			// DHM - Nerve :: Complaint lodging
			if ( attacker != self && level.warmupTime <= 0 ) {
				if ( attacker->client->pers.localClient ) {
					trap_SendServerCommand( self - g_entities, "complaint -4" );
				} else {
					trap_SendServerCommand( self - g_entities, va( "complaint %i", attacker->s.number ) );
					self->client->pers.complaintClient = attacker->s.clientNum;
					self->client->pers.complaintEndTime = level.time + 20500;
				}
			}
			// dhm

			// JPW NERVE
			if ( g_gametype.integer >= GT_WOLF ) { // high penalty to offset medic heal
				AddScore( attacker, WOLF_FRIENDLY_PENALTY );
			} else {
				// jpw
				AddScore( attacker, -1 );
			}
			// L0 - Life stats
			if (g_lifeStats.integer) {
				float acc = 0.00f;

				acc = (self->client->pers.life_acc_shots == 0) ?
					0.00 : ((float)self->client->pers.life_acc_hits / (float)self->client->pers.life_acc_shots ) * 100.00f ;

					CPx(self-g_entities, va("chat \"^zLast life: ^7Kills:^z%d ^7Headshots:^z%d ^7Acc:^z%2.2f\n\"",
						self->client->pers.life_kills, self->client->pers.life_headshots, acc));
			} // End
		} else {
			// JPW NERVE -- mostly added as conveneience so we can tweak from the #defines all in one place
			if ( g_gametype.integer >= GT_WOLF ) {
				AddScore( attacker, WOLF_FRAG_BONUS );
			} else {
				// jpw
				AddScore( attacker, 1 );
			}

			attacker->client->lastKillTime = level.time;
			// L0 - Life stats
			if (g_lifeStats.integer) {
				float acc = 0.00f;

				acc = (self->client->pers.life_acc_shots == 0) ?
					0.00 : ((float)self->client->pers.life_acc_hits / (float)self->client->pers.life_acc_shots ) * 100.00f ;

				// Class based..
				/*if (self->client->ps.stats[PC_MEDIC])
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7Rev:^3%d ^7Acc:^3%2.2f ^7Killer: %s^3(%ihp)\n\"",
						self->client->pers.life_kills, self->client->pers.life_headshots, self->client->pers.life_revives, acc, attacker->client->pers.netname, attacker->health));
				else if (self->client->ps.stats[PC_LT])
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7AmmoGiv:^3%d ^7Acc:^3%2.2f ^7Killer: %s^3(%ihp)\n\"",
					self->client->pers.life_kills, self->client->pers.life_headshots, self->client->pers.life_ammo, acc, attacker->client->pers.netname, attacker->health));
				else if (self->client->ps.stats[PC_ENGINEER])
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7Gibs: ^3%d ^7Acc:^3%2.2f ^7Killer: %s^3(%ihp)\n\"",
						self->client->pers.life_kills, self->client->pers.life_headshots, self->client->pers.life_gibs, acc, attacker->client->pers.netname, attacker->health));
				else*/
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7Gibs: ^3%d ^7Acc:^3%2.2f ^7Killer: %s^3(%ihp)\n\"",
					self->client->pers.life_kills, self->client->pers.life_headshots, self->client->pers.life_gibs, acc, attacker->client->pers.netname, attacker->health));
			} // End
		}
	} else {
		AddScore( self, -1 );
	}

	// Add team bonuses
	Team_FragBonuses( self, inflictor, attacker );

	// if client is in a nodrop area, don't drop anything
// JPW NERVE new drop behavior
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {   // only drop here in single player; in multiplayer, drop @ limbo
		contents = trap_PointContents( self->r.currentOrigin, -1 );
		if ( !( contents & CONTENTS_NODROP ) ) {
			TossClientItems( self );
		}
	}

	// drop flag regardless
	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		if ( self->client->ps.powerups[PW_REDFLAG] ) {
			item = BG_FindItem( "Red Flag" );
			if ( !item ) {
				item = BG_FindItem( "Objective" );
			}
			//G_matchPrintInfo(va("^5Allies have lost %s!", self->message), qfalse);
			trap_SendServerCommand(-1, va("cp \"^5Allies have lost %s!\n\" 2", self->message));
			self->client->ps.powerups[PW_REDFLAG] = 0;
		}
		if ( self->client->ps.powerups[PW_BLUEFLAG] ) {
			item = BG_FindItem( "Blue Flag" );
			if ( !item ) {
				item = BG_FindItem( "Objective" );
			}
			//G_matchPrintInfo(va("^5Axis have lost %s!", self->message), qfalse);
			trap_SendServerCommand(-1, va("cp \"^5Axis have lost %s!\n\" 2", self->message));

			self->client->ps.powerups[PW_BLUEFLAG] = 0;
		}

		if ( item ) {
			G_writeObjectiveEvent(self, objDropped  );
			launchvel[0] = 0;
			launchvel[1] = 0;
			launchvel[2] = 40;

			flag = LaunchItem( item,self->r.currentOrigin,launchvel,self->s.number );
			flag->s.modelindex2 = self->s.otherEntityNum2; // JPW NERVE FIXME set player->otherentitynum2 with old modelindex2 from flag and restore here
			flag->message = self->message;  // DHM - Nerve :: also restore item name
			// Clear out player's temp copies
			self->s.otherEntityNum2 = 0;
			self->message = NULL;
		}

		// send a fancy "MEDIC!" scream.  Sissies, ain' they?
		if ( self->client != NULL ) {
			if ( self->health > GIB_HEALTH && meansOfDeath != MOD_SUICIDE ) {

				if ( self->client->sess.sessionTeam == TEAM_RED ) {
					if ( random() > 0.5 ) {
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/axis/g-medic2.wav" ) );
					} else {
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/axis/g-medic3.wav" ) );
					}
				} else {
					if ( random() > 0.5 ) {
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/allies/a-medic3.wav" ) );
					} else {
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/allies/a-medic2.wav" ) );
					}
				}
			}
		}
	}
// jpw

	Cmd_Score_f( self );        // show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		gclient_t   *client;

		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		if ( client->sess.spectatorClient == self->s.number ) {
			Cmd_Score_f( g_entities + i );
		}
	}

	self->takedamage = qtrue;   // can still be gibbed
	self->r.contents = CONTENTS_CORPSE;

	self->s.powerups = 0;

// JPW NERVE -- only corpse in SP; in MP, need CONTENTS_BODY so medic can operate
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		self->s.weapon = WP_NONE;
		self->s.angles[0] = 0;
	} else {
		self->client->limboDropWeapon = self->s.weapon; // store this so it can be dropped in limbo
	}
// jpw
	self->s.angles[2] = 0;
	LookAtKiller( self, inflictor, attacker );

	VectorCopy( self->s.angles, self->client->ps.viewangles );
	self->s.loopSound = 0;

	trap_UnlinkEntity( self );
	self->r.maxs[2] = 0;
	self->client->ps.maxs[2] = 0;
	trap_LinkEntity( self );

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.time + 800;

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof( self->client->ps.powerups ) );

	// RTCWPro - set this up
	self->client->ps.powerups[PW_READY] = (player_ready_status[self->client->ps.clientNum].isReady == 1) ? INT_MAX : 0;

	// never gib in a nodrop
	if ( self->health <= GIB_HEALTH && !( contents & CONTENTS_NODROP ) ) {
		GibEntity( self, killer );
		nogib = qfalse;
	}

	if ( nogib ) {
		// normal death
		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if ( self->health <= GIB_HEALTH ) {
			self->health = GIB_HEALTH + 1;
		}

// JPW NERVE for medic
		self->client->medicHealAmt = 0;
// jpw

		// DHM - Play death animation, and set pm_time to delay 'fallen' animation
		self->client->ps.pm_time = BG_AnimScriptEvent( &self->client->ps, ANIM_ET_DEATH, qfalse, qtrue );

		G_AddEvent( self, EV_DEATH1 + 1, killer );

		// the body can still be gibbed
		self->die = body_die;
	}

	trap_LinkEntity( self );

	if ( g_gametype.integer >= GT_WOLF && meansOfDeath == MOD_SUICIDE ) {
		limbo( self, qtrue );
	}
}


/*
================
CheckArmor
================
*/
int CheckArmor( gentity_t *ent, int damage, int dflags ) {
	gclient_t   *client;
	int save;
	int count;

	if ( !damage ) {
		return 0;
	}

	client = ent->client;

	if ( !client ) {
		return 0;
	}

	if ( dflags & DAMAGE_NO_ARMOR ) {
		return 0;
	}

	// armor
	count = client->ps.stats[STAT_ARMOR];
	save = ceil( damage * ARMOR_PROTECTION );
	if ( save >= count ) {
		save = count;
	}

	if ( !save ) {
		return 0;
	}

	client->ps.stats[STAT_ARMOR] -= save;

	return save;
}

/*
==============
G_ArmorDamage
	brokeparts is how many should be broken off now
	curbroke is how many are broken
	the difference is how many to pop off this time
==============
*/
void G_ArmorDamage( gentity_t *targ ) {
	int brokeparts, curbroke;
	int numParts;
	int dmgbits = 16;   // 32/2;
	int i;

	if ( !targ->client ) {
		return;
	}

	if ( targ->s.aiChar == AICHAR_PROTOSOLDIER ) {
		numParts = 9;
	} else if ( targ->s.aiChar == AICHAR_SUPERSOLDIER ) {
		numParts = 14;
	} else if ( targ->s.aiChar == AICHAR_HEINRICH ) {
		numParts = 20;
	} else {
		return;
	}

	if ( numParts > dmgbits ) {
		numParts = dmgbits; // lock this down so it doesn't overwrite any bits that it shouldn't.  TODO: fix this


	}
	// determined here (on server) by location of hit and existing armor, you're updating here so
	// the client knows which pieces are still in place, and by difference with previous state, which
	// pieces to play an effect where the part is blown off.
	// Need to do it here so we have info on where the hit registered (head, torso, legs or if we go with more detail; arm, leg, chest, codpiece, etc)

	// ... Ick, just discovered that the refined hit detection ("hit nearest to which tag") is clientside...

	// For now, I'll randomly pick a part that hasn't been cleared.  This might end up looking okay, and we won't need the refined hits.
	//	however, we still have control on the server-side of which parts come off, regardless of what scheme is used.

	brokeparts = (int)( ( 1 - ( (float)( targ->health ) / (float)( targ->client->ps.stats[STAT_MAX_HEALTH] ) ) ) * numParts );

	if ( brokeparts && ( ( targ->s.dmgFlags & ( ( 1 << numParts ) - 1 ) ) != ( 1 << numParts ) - 1 ) ) {   // there are still parts left to clear

		// how many are removed already?
		curbroke = 0;
		for ( i = 0; i < numParts; i++ ) {
			if ( targ->s.dmgFlags & ( 1 << i ) ) {
				curbroke++;
			}
		}

		// need to remove more
		if ( brokeparts - curbroke >= 1 && curbroke < numParts ) {
			for ( i = 0; i < ( brokeparts - curbroke ); i++ ) {
				int remove = rand() % ( numParts );

				if ( !( ( targ->s.dmgFlags & ( ( 1 << numParts ) - 1 ) ) != ( 1 << numParts ) - 1 ) ) { // no parts are available any more
					break;
				}

				// FIXME: lose the 'while' loop.  Still should be safe though, since the check above verifies that it will eventually find a valid part
				while ( targ->s.dmgFlags & ( 1 << remove ) ) {
					remove = rand() % ( numParts );
				}

				targ->s.dmgFlags |= ( 1 << remove );    // turn off 'undamaged' part
				if ( (int)( random() + 0.5 ) ) {                       // choose one of two possible replacements
					targ->s.dmgFlags |= ( 1 << ( numParts + remove ) );
				}
			}
		}
	}
}

/*
==============
RTCWPro
G_GetHitsoundStyle
==============
*/
char* G_GetHitsoundStyle(int headStyle, int bodyStyle, qboolean headshot) {

	if (headshot) 
	{
		switch (headStyle)
		{
		case 0:
			return "sound/hitsounds/hithead1.wav";
			break;
		case 1:
			return "sound/hitsounds/hithead1.wav";
			break;
		case 2:
			return "sound/hitsounds/hithead2.wav";
			break;
		case 3:
			return "sound/hitsounds/hithead3.wav";
			break;
		case 4:
			return "sound/hitsounds/hithead4.wav";
			break;
		case 5:
			return "sound/hitsounds/hithead5.wav";
			break;
		case 6:
			return "sound/hitsounds/hithead6.wav";
			break;
		case 7:
			return "sound/hitsounds/hithead7.wav";
			break;
		case 8:
			return "sound/hitsounds/hithead8.wav";
			break;
		default:
			return "sound/hitsounds/hithead1.wav";
			break;
		}
	}
	else
	{
		switch (bodyStyle)
		{
		case 0:
			return "sound/hitsounds/hitbody1.wav";
			break;
		case 1:
			return "sound/hitsounds/hitbody1.wav";
			break;
		case 2:
			return "sound/hitsounds/hitbody2.wav";
			break;
		case 3:
			return "sound/hitsounds/hitbody3.wav";
			break;
		case 4:
			return "sound/hitsounds/hitbody4.wav";
			break;
		case 5:
			return "sound/hitsounds/hitbody5.wav";
			break;
		default:
			return "sound/hitsounds/hitbody1.wav";
			break;
		}
	}
}

/*
==============
RTCWPro
G_Hitsounds
==============
*/
void G_Hitsounds( gentity_t *target, gentity_t *attacker, int mod, qboolean headshot ) {
	gentity_t* te;

	if (!target || !attacker || !target->client || !attacker->client) 
	{
		return;
	}

	qboolean onSameTeam = OnSameTeam(target, attacker);

	// if player is hurting him self don't give any sounds
	if (target->client == attacker->client) 
	{
		return;  // this happens at flaming your self... just return silence...			
	}

	if (mod == MOD_ARTILLERY ||
		mod == MOD_GRENADE_SPLASH ||
		mod == MOD_DYNAMITE_SPLASH ||
		mod == MOD_DYNAMITE ||
		mod == MOD_ROCKET ||
		mod == MOD_ROCKET_SPLASH ||
		mod == MOD_KNIFE ||
		mod == MOD_KNIFE2 ||
		mod == MOD_GRENADE ||
		mod == MOD_AIRSTRIKE ||
		mod == MOD_ARTY ||
		mod == MOD_EXPLOSIVE ||
		mod == MOD_MORTAR ||
		mod == MOD_MORTAR_SPLASH ||
		mod == MOD_SYRINGE || 
		mod == MOD_UNKNOWN)
	{
		return;
	}

	if (!attacker->client->pers.hitSoundType) 
	{
		return;
	}

	// if team mate
	if (target->client && attacker->client && onSameTeam) 
	{
		//attacker->client->ps.persistant[PERS_HITBODY]--;
		//hitEventType = HIT_TEAMSHOT;

		if (attacker->client->pers.hitSoundType & HITSOUND_TEAM) 
		{
			te = G_TempEntity(attacker->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND);
			te->s.eventParm = G_SoundIndex("sound/hitsounds/hitteam1.wav");
			te->s.teamNum = attacker->s.clientNum;
		}
	}
	// If enemy
	else if (target &&
		target->client &&
		attacker &&
		attacker->client &&
		attacker->s.number != ENTITYNUM_NONE &&
		attacker->s.number != ENTITYNUM_WORLD &&
		attacker != target &&
		!onSameTeam)
	{
		te = G_TempEntity(attacker->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND);

		if (headshot) 
		{
			if (attacker->client->pers.hitSoundType & HITSOUND_HEAD) 
			{
				//attacker->client->ps.persistant[PERS_HITHEAD]++;
				//hitEventType = HIT_HEADSHOT;

				int headStyle = attacker->client->pers.hitSoundHeadStyle;
				te->s.eventParm = G_SoundIndex(G_GetHitsoundStyle(headStyle, 0, qtrue));
			}
		}
		else 
		{
			if (attacker->client->pers.hitSoundType & HITSOUND_BODY) 
			{
				//attacker->client->ps.persistant[PERS_HITBODY]++;
				//hitEventType = HIT_BODYSHOT;

				int bodyStyle = attacker->client->pers.hitSoundBodyStyle;
				te->s.eventParm = G_SoundIndex(G_GetHitsoundStyle(0, bodyStyle, qfalse));
			}
		}

		te->s.teamNum = attacker->s.clientNum;
	}
}

/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/
void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
	gclient_t   *client;
	int take;
	int save;
	int asave;
	int knockback;
	qboolean isHeadShot = qfalse;

	if ( !targ->takedamage ) {
		return;
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued || ( g_gamestate.integer != GS_PLAYING && match_warmupDamage.integer == 0 ) ) {
		return;
	}

	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

// JPW NERVE
	if ( ( targ->waterlevel >= 3 ) && ( mod == MOD_FLAMETHROWER ) ) {
		return;
	}
// jpw

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER && !( targ->aiName ) && !( targ->isProp ) && !targ->scriptName ) {
		if ( targ->use && targ->moverState == MOVER_POS1 ) {
			targ->use( targ, inflictor, attacker );
		}
		return;
	}

	if ( targ->s.eType == ET_MOVER && targ->aiName && !( targ->isProp ) && !targ->scriptName ) {
		switch ( mod ) {
		case MOD_GRENADE:
		case MOD_GRENADE_SPLASH:
		case MOD_ROCKET:
		case MOD_ROCKET_SPLASH:
			break;
		default:
			return; // no damage from other weapons
		}
	} else if ( targ->s.eType == ET_EXPLOSIVE )   {
		// 32 Explosive
		// 64 Dynamite only
		if ( ( targ->spawnflags & 32 ) || ( targ->spawnflags & 64 ) ) {
			switch ( mod ) {
			case MOD_GRENADE:
			case MOD_GRENADE_SPLASH:
			case MOD_ROCKET:
			case MOD_ROCKET_SPLASH:
			case MOD_AIRSTRIKE:
			case MOD_ARTILLERY:
			case MOD_GRENADE_PINEAPPLE:
			case MOD_MORTAR:
			case MOD_MORTAR_SPLASH:
			case MOD_EXPLOSIVE:
				if ( targ->spawnflags & 64 ) {
					return;
				}

				break;

			case MOD_DYNAMITE:
			case MOD_DYNAMITE_SPLASH:
				break;

			default:
				return;
			}
		}
	}

	client = targ->client;

	if ( client ) {
		if ( client->noclip) { // KK commenting this out during our alpha test || client->ps.powerups[PW_INVULNERABLE]  ) {
			return;
		}
	}

	if ( !dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize( dir );
	}

	knockback = damage;
	if ( knockback > 200 ) {
		knockback = 200;
	}
	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	}
	if ( dflags & DAMAGE_NO_KNOCKBACK ) {
		knockback = 0;
	}
	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client && ( g_friendlyFire.integer || !OnSameTeam( targ, attacker ) ) ) {
		vec3_t kvel;
		float mass;

		mass = 200;

		if ( mod == MOD_LIGHTNING && !( ( level.time + targ->s.number * 50 ) % 400 ) ) {
			knockback = 60;
			dir[2] = 0.3;
		}

    	if (dflags & DAMAGE_RADIUS) {
		    VectorScale( dir, g_damageRadiusKnockback.value * (float)knockback / mass, kvel );	
		} else {
        	VectorScale( dir, g_knockback.value * (float)knockback / mass, kvel );
		}
		
		VectorAdd( targ->client->ps.velocity, kvel, targ->client->ps.velocity );

		if ( targ == attacker && !(  mod != MOD_ROCKET &&
									 mod != MOD_ROCKET_SPLASH &&
									 mod != MOD_GRENADE &&
									 mod != MOD_GRENADE_SPLASH &&
									 mod != MOD_DYNAMITE ) ) {
			targ->client->ps.velocity[2] *= 0.25;
		}

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !( dflags & DAMAGE_NO_PROTECTION ) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
		if ( targ != attacker && OnSameTeam( targ, attacker )  ) {
			if ( ( g_gamestate.integer != GS_PLAYING && match_warmupDamage.integer == 0 ) ) {
				return;
			}
		}

		// check for godmode
		if ( targ->flags & FL_GODMODE ) {
			return;
		}

		// RF, warzombie defense position is basically godmode for the time being
		if ( targ->flags & FL_DEFENSE_GUARD ) {
			return;
		}

		// check for invulnerability // (SA) moved from below so DAMAGE_NO_PROTECTION will still work
		if ( client && client->ps.powerups[PW_INVULNERABLE] ) { //----(SA)	added
			return;
		}

	}

	// battlesuit protects from all radius damage (but takes knockback)
	// and protects 50% against all damage
	if ( client && client->ps.powerups[PW_BATTLESUIT] ) {
		G_AddEvent( targ, EV_POWERUP_BATTLESUIT, 0 );
		if ( dflags & DAMAGE_RADIUS ) {
			return;
		}
		damage *= 0.5;
	}

	// add to the attacker's hit counter
	if ( attacker->client && targ != attacker && targ->health > 0 ) {
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->ps.persistant[PERS_HITS] -= damage;
		} else {
			attacker->client->ps.persistant[PERS_HITS] += damage;
		}
	}

	if ( damage < 1 ) {
		damage = 1;
	}
	take = damage;
	save = 0;

	// save some from armor
	asave = CheckArmor( targ, take, dflags );
	take -= asave;

	// RTCWPro - head stuff
	//if ( IsHeadShot( targ, qfalse, dir, point, mod ) ) {
	if (targ->headshot && targ->client) {

		if ( take * 2 < g_hsDamage.integer )
		{
			take = g_hsDamage.integer; // head shots, all weapons, do minimum 50 points damage
		}
		else
		{
			take *= 2; // sniper rifles can do full-kill (and knock into limbo)

		}

		if ( !( targ->client->ps.eFlags & EF_HEADSHOT ) ) {  // only toss hat on first headshot
			G_AddEvent( targ, EV_LOSE_HAT, DirToByte( dir ) );
		}

		// RTCWPro - some debug info
		if (g_debugBullets.integer)
		{
			AP(va("print \"%s ^7headshot for %i dmg\n\"", targ->client->pers.netname, take));
		}

		targ->client->ps.eFlags |= EF_HEADSHOT;

		isHeadShot = qtrue;

		// L0 - OSP stats - Record the headshot
		if ( client && attacker && attacker->client
			 && attacker->client->sess.sessionTeam != targ->client->sess.sessionTeam ) {
			G_addStatsHeadShot( attacker, mod );
		} // End
	}

	if ( g_debugDamage.integer ) {
		G_Printf( "client: %i health: %i damage: %i mod: %i\n", targ->s.number, targ->health, take, mod); //, asave );
		AP(va("print \"client:%i health:%i damage:%i mod: %i\n\"", targ->s.number, targ->health, take, mod));
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) {
		if ( attacker ) {
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		//client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;

		if ( dir ) {
			VectorCopy( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}
	}

	// See if it's the player hurting the emeny flag carrier
	Team_CheckHurtCarrier( targ, attacker );

	if ( targ->client ) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
	}

	// RTCWPro - hitsounds
	if (g_hitsounds.integer) {
		if ((attacker->client) && (targ->client)) {
			G_Hitsounds(targ, attacker, mod, isHeadShot);
		}
	}

	// do the damage
	if ( take ) {
		targ->health = targ->health - take;

		// Ridah, can't gib with bullet weapons (except VENOM)
		if ( mod != MOD_VENOM && attacker == inflictor && targ->health <= GIB_HEALTH ) {
			if ( targ->aiCharacter != AICHAR_ZOMBIE ) { // zombie needs to be able to gib so we can kill him (although he doesn't actually GIB, he just dies)
				targ->health = GIB_HEALTH + 1;
			}
		}


// JPW NERVE overcome previous chunk of code for making grenades work again
		if ( ( g_gametype.integer != GT_SINGLE_PLAYER ) && ( take > 190 ) ) { // 190 is greater than 2x mauser headshot, so headshots don't gib
			targ->health = GIB_HEALTH - 1;

			// gibbed by a nade or other explosion
			/*if (attacker->client && attacker != targ && !OnSameTeam(attacker, targ) && (g_gamestate.integer != GS_WARMUP)) //do not add gibs to stats during warmup.
			{
                    attacker->client->sess.gibs++;	//gibbed an enemy
                    attacker->client->pers.life_gibs++;
            }*/
		}
// jpw
		//G_Printf("health at: %d\n", targ->health);
		if ( targ->health <= 0 ) {
			if (client) {
				targ->flags |= FL_NO_KNOCKBACK;
				if (g_gametype.integer >= GT_WOLF) {

					// do gib counting before sending them to limbo
					if ((targ->health <= FORCE_LIMBO_HEALTH) && (!(targ->client->ps.pm_flags & PMF_LIMBO)
						&& attacker->client && attacker != targ && !OnSameTeam(attacker, targ) && (g_gamestate.integer != GS_WARMUP))) //do not add gibs to stats during warmup.
					{
						attacker->client->sess.gibs++;
						attacker->client->pers.life_gibs++;
					}

					// JPW NERVE -- repeated shooting sends to limbo
					if ((targ->health < FORCE_LIMBO_HEALTH) && (targ->health > GIB_HEALTH) && (!(targ->client->ps.pm_flags & PMF_LIMBO))) {
						limbo(targ, qtrue);
					}
					// jpw
				}
			}

			if ( targ->health < -999 ) {
				targ->health = -999;
			}

			targ->enemy = attacker;
			if ( targ->die ) { // Ridah, mg42 doesn't have die func (FIXME)
				targ->die( targ, inflictor, attacker, take, mod );
			}

			// if we freed ourselves in death function
			if ( !targ->inuse ) {
				return;
			}

			// RF, entity scripting
			if ( targ->s.number >= MAX_CLIENTS && targ->health <= 0 ) { // might have revived itself in death function
				G_Script_ScriptEvent( targ, "death", "" );
			}

		} else if ( targ->pain ) {
			if ( dir ) {  // Ridah, had to add this to fix NULL dir crash
				VectorCopy( dir, targ->rotate );
				VectorCopy( point, targ->pos3 ); // this will pass loc of hit
			} else {
				VectorClear( targ->rotate );
				VectorClear( targ->pos3 );
			}

			targ->pain( targ, attacker, take, point );

			// RF, entity scripting
			if ( targ->s.number >= MAX_CLIENTS ) {
				G_Script_ScriptEvent( targ, "pain", va( "%d %d", targ->health, targ->health + take ) );
			}
		// L0 - OSP - update weapon/dmg stats
		} else {
			G_addStats( targ, attacker, take, mod );
		} // End

		//G_ArmorDamage(targ);	//----(SA)	moved out to separate routine

		// Ridah, this needs to be done last, incase the health is altered in one of the event calls
		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}
	}

}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage( gentity_t *targ, vec3_t origin ) {
	vec3_t dest;
	trace_t tr;
	vec3_t midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd( targ->r.absmin, targ->r.absmax, midpoint );
	VectorScale( midpoint, 0.5, midpoint );

	VectorCopy( midpoint, dest );
	trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if ( tr.fraction == 1.0 ) {
		return qtrue;
	}



	if ( &g_entities[tr.entityNum] == targ ) {
		return qtrue;
	}

	// this should probably check in the plane of projection,
	// rather than in world coordinate, and also include Z
	VectorCopy( midpoint, dest );
	dest[0] += 15.0;
	dest[1] += 15.0;
	trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if ( tr.fraction == 1.0 ) {
		return qtrue;
	}

	VectorCopy( midpoint, dest );
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if ( tr.fraction == 1.0 ) {
		return qtrue;
	}

	VectorCopy( midpoint, dest );
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if ( tr.fraction == 1.0 ) {
		return qtrue;
	}

	VectorCopy( midpoint, dest );
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if ( tr.fraction == 1.0 ) {
		return qtrue;
	}


	return qfalse;
}


/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage( vec3_t origin, gentity_t *attacker, float damage, float radius,
						 gentity_t *ignore, int mod ) {
	float points, dist;
	gentity_t   *ent;
	int entityList[MAX_GENTITIES];
	int numListedEntities;
	vec3_t mins, maxs;
	vec3_t v;
	vec3_t dir;
	int i, e;
	qboolean hitClient = qfalse;
// JPW NERVE
	float boxradius;
	vec3_t dest;
	trace_t tr;
	vec3_t midpoint;
// jpw


	if ( radius < 1 ) {
		radius = 1;
	}

	boxradius = 1.41421356 * radius; // radius * sqrt(2) for bounding box enlargement --
	// bounding box was checking against radius / sqrt(2) if collision is along box plane
	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - boxradius; // JPW NERVE
		maxs[i] = origin[i] + boxradius; // JPW NERVE
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if ( ent == ignore ) {
			continue;
		}
		if ( !ent->takedamage ) {
			continue;
		}

/* JPW NERVE -- we can put this back if we need to, but it kinna sucks for human-sized bboxes
		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}
*/
// JPW NERVE
		if ( !ent->r.bmodel ) {
			VectorSubtract( ent->r.currentOrigin,origin,v ); // JPW NERVE simpler centroid check that doesn't have box alignment weirdness
		} else {
			for ( i = 0 ; i < 3 ; i++ ) {
				if ( origin[i] < ent->r.absmin[i] ) {
					v[i] = ent->r.absmin[i] - origin[i];
				} else if ( origin[i] > ent->r.absmax[i] ) {
					v[i] = origin[i] - ent->r.absmax[i];
				} else {
					v[i] = 0;
				}
			}
		}
// jpw
		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

// JPW NERVE -- different radiusdmg behavior for MP -- big explosions should do less damage (over less distance) through failed traces
		if ( CanDamage( ent, origin ) ) {
			if ( LogAccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract( ent->r.currentOrigin, origin, dir );
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage( ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod );
		}
// JPW NERVE --  MP weapons should do 1/8 damage through walls over 1/8th distance
		else {
			if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
				VectorAdd( ent->r.absmin, ent->r.absmax, midpoint );
				VectorScale( midpoint, 0.5, midpoint );
				VectorCopy( midpoint, dest );

				trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
				if ( tr.fraction < 1.0 ) {
					VectorSubtract( dest,origin,dest );
					dist = VectorLength( dest );
					if ( dist < radius * 0.2f ) { // closer than 1/4 dist
						if ( LogAccuracyHit( ent, attacker ) ) {
							hitClient = qtrue;
						}
						VectorSubtract( ent->r.currentOrigin, origin, dir );
						dir[2] += 24;
						G_Damage( ent, NULL, attacker, dir, origin, (int)( points * 0.1f ), DAMAGE_RADIUS, mod );
					}
				}
			}
		}
// jpw
	}
	return hitClient;
}
