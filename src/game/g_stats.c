/*
===========================================================================

wolfX GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of wolfX source code.

wolfX Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

wolfX Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with wolfX Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the wolfX Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the wolfX Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
L0 - g_stats.c

Mostly eye candy stuff.

Created: 23. Oct / 2012
Last Updated: 09. Apr / 2013
===========================================================================
*/
#include "g_local.h"
#include "g_stats.h"

static qboolean firstheadshot;
static qboolean firstblood;

/*
===========
Set time so it's more accessible..
===========
*/
extern int trap_RealTime ( qtime_t * qtime );
const char *dMonths[12] = {
"Jan", "Feb", "Mar", "Apr", "May", "Jun",
"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/*
===========
Determine gender

To deal with messages better.
===========
*/
int isGender(gentity_t *ent) {
	char userinfo[MAX_INFO_STRING];
	char *s;

	trap_GetUserinfo( ent-g_entities, userinfo, sizeof( userinfo ) );
	s = Info_ValueForKey( userinfo, "sex" );

	if (!Q_stricmp(s, "male"))
		return 0;
	if (!Q_stricmp(s, "female"))
		return 1;
	else // Defaults to male..
		return 0;
}

/*
===========
Sounds

This could be done on client side..but why bother when i'm lazy. :)
===========
*/
void doSound(gentity_t *ent, int type, char *path, char *sound) {
	gentity_t *te;

	te = G_TempEntity(ent->s.pos.trBase, type);
	te->s.eventParm = G_SoundIndex(va("%s%s", path, sound));
	te->r.svFlags |= SVF_BROADCAST;
}

/*
===========
Double+ kills
===========
*/
void doubleKill (gentity_t *ent, int meansOfDeath ) {
	/*
	char *message, *random;
	int n = rand() % 3;

	if (!g_doubleKills.integer) {
		return;
	}

	if(!ent || !ent->client) {
		return;
	}

	// White list approach makes more sense.
	if ( meansOfDeath == MOD_LUGER // handgun
		|| meansOfDeath == MOD_COLT  // handgun
		|| meansOfDeath == MOD_KNIFE_STEALTH // knife -- can be done :)
		|| meansOfDeath == MOD_THOMPSON  // Thompson
		|| meansOfDeath == MOD_MP40 // MP40
		|| meansOfDeath == MOD_STEN // STEN
	) {

		message = "";
		if((level.time - ent->client->lastKillTime) > 1000) {
			ent->client->doublekill = 0;
			ent->client->lastKillTime = level.time;
			return;
		} else {
			ent->client->doublekill++;
			ent->client->lastKillTime = level.time;
		}

		switch(ent->client->doublekill) {
			case 1: // 2 kills
				if (n == 0) random = "doublekill.wav";
				else if (n == 1) random = "doublekill2.wav";
				else if (n == 2) random = "doublekill3.wav";
				else random = "doublekill.wav";
				message = "Double kill!";
			//	doSound(ent, EV_STATS_SOUND, "sound/game/sprees/doubleKills/", va("%s", random)); 	 // nihi commented out
				AP(va("spp \"^3%s ^7%s\n\"", message, ent->client->pers.netname));
			break;
			case 2:	// 3 kills
				message = "Triple kill!";
				doSound(ent, EV_STATS_SOUND, "sound/game/sprees/doubleKills/", "tripplekill.wav");
				AP(va("spp \"^3%s ^7%s\n\"", message, ent->client->pers.netname));
			break;
			case 3: // 4 kills
				message = "Pure Ownage!";
				doSound(ent, EV_STATS_SOUND, "sound/game/sprees/doubleKills/", "oneandonly.wav");
				AP(va("spp \"^3%s ^7%s\n\"", message, ent->client->pers.netname));
			break;
		}
	}	*/  // nihi commented out
}

/*
===========
Killing sprees
===========
*/
void KillingSprees ( gentity_t *ent, int score ) {
	/*
	int killRatio = ent->client->sess.kills;
	int snd_idx;

	if (!g_killingSprees.integer)
		return;

	// if killer ratio is bellow 100 kills spam every 5th kill
	if (killRatio <= 100 && killRatio >= 5 && (killRatio % 5) == 0 ) 	{
		snd_idx = (killRatio / 5) - 1;

		AP(va("spp \"^d%s ^d(^7%dK %dhs^d): ^7%s\n\"",
			killingSprees[snd_idx <= 20 ? snd_idx : 19].msg, killRatio, ent->client->sess.headshots, ent->client->pers.netname));

		doSound(ent, EV_STATS_SOUND, "sound/game/sprees/Sprees/", killingSprees[snd_idx < 20 ? snd_idx : 19].snd );
	}
	// Anything above 100 gets spammed each 10th time..
	else if ( killRatio > 100 && killRatio >= 10 && (killRatio % 10) == 0 ) {
		snd_idx = (killRatio / 10) - 1;

		AP(va("spp \"^dHOLY SHIT ^d(^7%dK %dhs^d): ^7%s\n\"", killRatio, ent->client->sess.headshots, ent->client->pers.netname));
		doSound(ent, EV_STATS_SOUND, "sound/game/sprees/Sprees/", "holyshit_alt.wav");
	}

	// could be done some other way but anyway...do the count... :)
	ent->client->ps.persistant[PERS_SCORE] += score;
	if (g_gametype.integer >= GT_TEAM)
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;

CalculateRanks(); 	*/  // nihi commented out
}

/*
===========
Death spree
===========
*/
void deathSpree ( gentity_t *ent ) {
/*	int deaths = ent->client->pers.spreeDeaths;
	int n = rand() % 2;
	char *snd="", *spree="";

	if (!g_deathSprees.integer || deaths <= 0)
		return;

	if( deaths == 9 ) {
		if (n == 0) { spree=va("(^710 Dth^C)"); snd = "dSpree1.wav"; }
		else { spree=va("(^710 Dth^C)"); snd = "dSpree1_alt.wav"; }
	} else if( deaths == 14 ) {
		if (n == 0) { spree=va("(^715 Dth^C)"); snd = "dSpree2.wav"; }
		else { spree=va("(^715 Dth^C)"); snd = "dSpree2_alt.wav"; }
	} else if( deaths == 19 ) {
		if (n == 0) { spree=va("(^720 Dth^C)"); snd = "dSpree3.wav"; }
		else { spree=va("(^720 Dth^C)"); snd = "dSpree3_alt.wav";  }
	} else if( deaths == 24 ) {
		if (n == 0) { spree=va("(^725 Dth^C)"); snd = "dSpree4.wav"; }
		else { spree=va("(^725 Dth^C)"); snd = "dSpree4_alt.wav"; }
	}

	if (deaths == 9 || deaths == 14 || deaths == 19 || deaths == 24) {
		AP(va("spp \"^CDEATHSPREE! %s: ^7%s\n\"", spree, ent->client->pers.netname));
		doSound(ent, EV_STATS_SOUND, "sound/game/sprees/deathSpree/", va("%s", snd));
	}	*/  // nihi commented out
}

/*
===========
Killer spree

Almost identical to Killing sprees, just uses different colors and female sounds.
===========
*/
void killerSpree(gentity_t *ent, int score) {
/*	int killRatio=ent->client->pers.life_kills;
	int snd_idx;

	if(!g_killerSpree.integer)
		return;

	if(!ent || !ent->client)
		return;

	// if killer ratio is bellow 50 kills spam every 5th kill
	if (killRatio <= 50 && killRatio >= 5 && (killRatio % 5) == 0 ) 	{
		snd_idx = (killRatio / 5) - 1;

		AP(va("spp \"^j%s ^j(^7%dk^j): ^7%s\n\"",
			killerSprees[snd_idx <= 11 ? snd_idx : 10].msg, killRatio, ent->client->pers.netname));

		doSound(ent, EV_STATS_SOUND, "sound/game/sprees/killerSprees/", killerSprees[snd_idx < 11 ? snd_idx : 10].snd );
	}
	// Anything above 50 gets spammed each 10th time..
	else if ( killRatio > 50 && killRatio >= 10 && (killRatio % 10) == 0 ) {
		snd_idx = (killRatio / 10) - 1;

		AP(va("spp \"^j%s ^j(^7%dk^j): ^7%s\n\"",
			killerSprees[snd_idx <= 11 ? snd_idx : 10].msg, killRatio, ent->client->pers.netname));
		doSound(ent, EV_STATS_SOUND, "sound/game/sprees/killerSprees/", killerSprees[snd_idx < 11 ? snd_idx : 10].snd );
	}

	// could be done some other way but anyway...do the count... :)
	ent->client->ps.persistant[PERS_SCORE] += score;
	if (g_gametype.integer >= GT_TEAM)
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;

CalculateRanks(); 		*/  // nihi commented out
}

/*
===========
First headshots

Prints who done first headshots when round starts.
===========
*/
void FirstHeadshot (gentity_t *attacker, gentity_t *targ) {
/*	qboolean 	onSameTeam = OnSameTeam( targ, attacker);

	if (g_showFirstHeadshot.integer) {

		if ( !firstheadshot &&
			targ &&
			targ->client &&
			attacker &&
			attacker->client &&
			attacker->s.number != ENTITYNUM_NONE &&
			attacker->s.number != ENTITYNUM_WORLD &&
			attacker != targ &&
			g_gamestate.integer == GS_PLAYING &&
			!onSameTeam )
		{
			AP(va("chat \"%s ^7blew out %s^7's brains with the ^3FIRST HEAD SHOT^7!\"", attacker->client->pers.netname, targ->client->pers.netname));
			APS("sound/scenaric/headshot.wav");
			firstheadshot = qtrue;
		}
	} 	*/  // nihi commented out
}

/*
===========
First blood

Prints who draw first blood when round starts.
NOTE: Atm it's only a print..once I'm not lazy I'll set it in a way it can decide winner once timelimit hits on
	  specific maps (like depot, destuction) - so first blood decides who won.
===========
*/
void FirstBlood (gentity_t *self, gentity_t *attacker) {
	qboolean 	onSameTeam = OnSameTeam( self, attacker);

/*	if (g_showFirstBlood.integer) {

		if (! firstblood &&
			self &&
			self->client &&
			attacker &&
			attacker->client &&
			attacker->s.number != ENTITYNUM_NONE &&
			attacker->s.number != ENTITYNUM_WORLD &&
			attacker != self &&
			g_gamestate.integer == GS_PLAYING &&
			!onSameTeam)
		{
			AP(va("chat \"%s ^7drew ^jFIRST BLOOD ^7from ^7%s^j!\"", attacker->client->pers.netname, self->client->pers.netname));
			APS("sound/scenaric/firstblood.wav");
			firstblood = qtrue;
		}
	}*/  // nihi commented out
}

/***********************************************************************************/
/* ================= Stats - (Large) Code dump from OSP (ET port) =================*/
/***********************************************************************************/
int iWeap = WS_MAX;

static const weap_ws_convert_t aWeapMOD[MOD_NUM_MODS] = {
	{ MOD_UNKNOWN,              WS_MAX },
	{ MOD_MACHINEGUN,           WS_MG42 },
	{ MOD_GRENADE,              WS_GRENADE },
	{ MOD_GRENADE_SPLASH,		WS_GRENADE }, // RtcwPro added grenade splash
	{ MOD_ROCKET,               WS_PANZERFAUST },
	{ MOD_ROCKET_SPLASH,		WS_PANZERFAUST}, // RtcwPro added rocket splash
	{ MOD_KNIFE2,               WS_KNIFE },
	{ MOD_KNIFE,                WS_KNIFE },
	{ MOD_KNIFE_STEALTH,        WS_KNIFE },
//	{ MOD_THROWKNIFE,           WS_KNIFE },
	{ MOD_LUGER,                WS_LUGER },
	{ MOD_COLT,                 WS_COLT },
	{ MOD_MP40,                 WS_MP40 },
	{ MOD_THOMPSON,             WS_THOMPSON },
	{ MOD_STEN,                 WS_STEN },
//	{ MOD_GARAND,               WS_RIFLE },
	{ MOD_MAUSER,				WS_RIFLE}, // RtcwPro added mauser
	{ MOD_SNIPERRIFLE,          WS_RIFLE },
	{ MOD_FG42,                 WS_FG42 },
	{ MOD_FG42SCOPE,            WS_FG42 },
	{ MOD_PANZERFAUST,          WS_PANZERFAUST },
	{ MOD_GRENADE_LAUNCHER,     WS_GRENADE },
	{ MOD_FLAMETHROWER,         WS_FLAMETHROWER },
	{ MOD_VENOM,				WS_VENOM },
	{ MOD_VENOM_FULL,			WS_VENOM }, // RtcwPro added venom full
	{ MOD_GRENADE_PINEAPPLE,    WS_GRENADE },

	{ MOD_DYNAMITE,             WS_DYNAMITE },
	{ MOD_AIRSTRIKE,            WS_AIRSTRIKE },
	{ MOD_SYRINGE,              WS_SYRINGE },
//	{ MOD_POISONEDMED,           WS_POISON },
	{ MOD_ARTY,                 WS_ARTILLERY },
	{ MOD_ARTILLERY,                 WS_ARTILLERY }
};

// Get right stats index based on weapon mod
unsigned int G_weapStatIndex_MOD( int iWeaponMOD ) {
	unsigned int i;

	for ( i = 0; i < MOD_NUM_MODS; i++ ) if ( iWeaponMOD == aWeapMOD[i].iWeapon ) {
			return( aWeapMOD[i].iWS );
		}
	return( WS_MAX );
}

// +wstats
char *G_createStats( gentity_t *refEnt ) {
	unsigned int i, dwWeaponMask = 0, dwSkillPointMask = 0;
	char strWeapInfo[MAX_STRING_CHARS] = {0};
	char strSkillInfo[MAX_STRING_CHARS] = {0};

	if ( !refEnt ) {
		return( NULL );
	}

	// Add weapon stats as necessary
	for ( i = WS_KNIFE; i < WS_MAX; i++ ) {
		if ( refEnt->client->sess.aWeaponStats[i].atts || refEnt->client->sess.aWeaponStats[i].hits ||
			 refEnt->client->sess.aWeaponStats[i].deaths ) {
			dwWeaponMask |= ( 1 << i );
			Q_strcat( strWeapInfo, sizeof( strWeapInfo ),
						va( " %d %d %d %d %d",
							refEnt->client->sess.aWeaponStats[i].hits, refEnt->client->sess.aWeaponStats[i].atts,
							refEnt->client->sess.aWeaponStats[i].kills, refEnt->client->sess.aWeaponStats[i].deaths,
							refEnt->client->sess.aWeaponStats[i].headshots ) );
		}
	}

	// Additional info
	Q_strcat( strWeapInfo, sizeof( strWeapInfo ),
				va( " %d %d %d %d",
					refEnt->client->sess.damage_given,
					refEnt->client->sess.damage_received,
					refEnt->client->sess.team_damage,
					refEnt->client->sess.gibs) );

	return( va( "%d %d %d%s %d%s",
				(int)(refEnt - g_entities),
				refEnt->client->sess.rounds,
				dwWeaponMask,
				strWeapInfo,
				dwSkillPointMask,
				strSkillInfo) );
}

// L0 - Typical "1.0" info based stats (+stats)
char *G_createClientStats( gentity_t *refEnt ) {
	char strClientInfo[MAX_STRING_CHARS] = {0};

	if ( !refEnt ) {
		return( NULL );
	}

	// Info
	Q_strcat( strClientInfo, sizeof( strClientInfo ),
		va( "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			refEnt->client->sess.kills,
			refEnt->client->sess.headshots,
			refEnt->client->sess.deaths,
			refEnt->client->sess.team_kills,
			refEnt->client->sess.suicides,
			refEnt->client->sess.acc_shots,
			refEnt->client->sess.acc_hits,
			refEnt->client->sess.damage_given,
			refEnt->client->sess.damage_received,
			refEnt->client->sess.team_damage,
			refEnt->client->sess.gibs,
			refEnt->client->sess.med_given,
			refEnt->client->sess.ammo_given,
			refEnt->client->sess.revives,
			refEnt->client->sess.killPeak
			));

	return( va( "%d %s", (int)(refEnt - g_entities), strClientInfo) );
}

// Sends a player's stats to the requesting client.
void G_statsPrint( gentity_t *ent, int nType ) {
	int pid;
	char *cmd = ( nType == 0 ) ? "ws" : ( "wws" /* ( nType == 1 ) ? "wws" : "gstats" */ );   // Yes, not the cleanest
	char arg[MAX_TOKEN_CHARS];

	if ( !ent || ( ent->r.svFlags & SVF_BOT ) ) {
		return;
	}

	// If requesting stats for self, its easy.
	if ( trap_Argc() < 2 ) {
		if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
			CP( va( "%s %s\n", cmd, G_createStats( ent ) ) );
			// Specs default to players they are chasing
		} else if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			CP( va( "%s %s\n", cmd, G_createStats( g_entities + ent->client->sess.spectatorClient ) ) );
		} else {
			CP( "print \"^zInfo: ^7Type ^z\\wstats <player_id>^7 to see stats on an active player.\n\"" );
			return;
		}
	} else {
		// Find the player to poll stats.
		trap_Argv( 1, arg, sizeof( arg ) );
		if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
			return;
		}
		CP( va( "%s %s\n", cmd, G_createStats( g_entities + pid ) ) );
	}
}

// Sends a player's stats to the requesting client.
void G_clientStatsPrint( gentity_t *ent, int nType, qboolean toWindow ) {
	int pid;
	char *cmd = (toWindow) ? "cgs" : "cgsp";
	char arg[MAX_TOKEN_CHARS];

	if ( !ent || ( ent->r.svFlags & SVF_BOT ) ) {
		return;
	}

	// If requesting stats for self, its easy.
	if ( trap_Argc() < 2 ) {
		if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
			CP( va( "%s %s\n", cmd, G_createClientStats( ent ) ) );
			// Specs default to players they are chasing
		} else if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			CP( va( "%s %s\n", cmd, G_createClientStats( g_entities + ent->client->sess.spectatorClient ) ) );
		} else {
			CP( "print \"^zInfo: ^7Type ^z\\stats <player_id>^7 to see stats on an active player.\n\"" );
			return;
		}
	} else {
		// Find the player to poll stats.
		trap_Argv( 1, arg, sizeof( arg ) );
		if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
			return;
		}
		CP( va( "%s %s\n", cmd, G_createClientStats( g_entities + pid ) ) );
	}
}

// Records accuracy, damage, and kill/death stats.
void G_addStats( gentity_t *targ, gentity_t *attacker, int dmg_ref, int mod ) {
	int dmg, ref;

	if (g_gamestate.integer != GS_PLAYING) {
		return;
	}

	// Keep track of only active player-to-player interactions in a real game
	if ( !targ || !targ->client ||
		 g_gamestate.integer != GS_PLAYING ||
	//	 mod == MOD_ADMKILL ||
		 mod == MOD_SWITCHTEAM ||
		 ( g_gametype.integer >= GT_WOLF && ( targ->client->ps.pm_flags & PMF_LIMBO ) ) ||
		 ( g_gametype.integer < GT_WOLF && ( targ->s.eFlags == EF_DEAD || targ->client->ps.pm_type == PM_DEAD ) ) ) {
		return;
	}

	// Special hack for intentional gibbage
	if ( targ->health <= 0 && targ->client->ps.pm_type == PM_DEAD ) {
		if ( mod < MOD_CROSS && attacker && attacker->client ) {
			int x = attacker->client->sess.aWeaponStats[G_weapStatIndex_MOD( mod )].atts--;
			if ( x < 1 ) {
				attacker->client->sess.aWeaponStats[G_weapStatIndex_MOD( mod )].atts = 1;
			}
		}
		return;
	}

	// Suicides only affect the player specifically
	if ( targ == attacker || !attacker || !attacker->client || mod == MOD_SUICIDE ) {
		if ( !attacker || !attacker->client )
		return;
	}
	if ( mod == MOD_SUICIDE ) {
		dmg = 0;
	}
	// Telefrags only add 100 points.. not 100k!!
	else if ( mod == MOD_TELEFRAG ) {
		dmg = 100;
	}
	else {

		// RtcwPro do not give more damage the user's full health - in OSP panzer awarded 400 damage on a kill/gib - let's try to even out the damange efficiency
		if (dmg_ref >= abs(FORCE_LIMBO_HEALTH) && dmg_ref < abs(GIB_HEALTH)) {
			dmg = abs(FORCE_LIMBO_HEALTH);
		}
		else if (dmg_ref >= abs(GIB_HEALTH)) {
			dmg = abs(GIB_HEALTH);
		}
		else
			dmg = dmg_ref;
	}

	// Player team stats
	if ( g_gametype.integer >= GT_WOLF && targ->client->sess.sessionTeam == attacker->client->sess.sessionTeam ) {

		if (attacker != targ) { // don't give team damage for suicide (same as OSP)
			attacker->client->sess.team_damage += dmg;
		}

		// Don't count self kill as team kill..because it ain't!
		if ( targ->health <= 0 && !(mod == MOD_SELFKILL || mod == MOD_SUICIDE)) {

			// RtcwPro temporary fixes below - when you panzer yourself and a teammate the MOD for the attacker is not a MOD_SUICIDE it's a MOD_ROCKET/MOD_ROCKET_SPLASH

			if (attacker != targ) {
				attacker->client->sess.team_kills++;  // if it's NOT a self kill count it as a TK (same as OSP)
			}

			if (attacker == targ) {
				attacker->client->sess.suicides++;  // if it IS a self kill count it as a suicide (same as OSP)
			}
			else {
				targ->client->sess.deaths++;	// Record death when TK occurs
			}
		}
		return;
	}

	// General player stats
	if ( mod != MOD_SYRINGE ) {
		attacker->client->sess.damage_given += dmg;
		targ->client->sess.damage_received += dmg;
		if ( targ->health <= 0 ) {
			attacker->client->sess.kills++;
			targ->client->sess.deaths++;

			// L0 - Life(s) Kill peak
			if (attacker->client->pers.life_kills >= attacker->client->sess.killPeak)
				attacker->client->sess.killPeak++;
		}
	}

/*	if (mod == MOD_POISONEDMED && targ->health <= 0) {
		attacker->client->sess.poisoned++;
	}*/ // nihi commented out

	// Player weapon stats
	ref = G_weapStatIndex_MOD( mod );
	if ( dmg > 0 ) {
		attacker->client->sess.aWeaponStats[ref].hits++;
	}
	if ( targ->health <= 0 ) {
		attacker->client->sess.aWeaponStats[ref].kills++;
		targ->client->sess.aWeaponStats[ref].deaths++;
	}
}

// Records weapon headshots
void G_addStatsHeadShot( gentity_t *attacker, int mod ) {
	if ( g_gamestate.integer != GS_PLAYING ) {
		return;
	}

	if ( !attacker || !attacker->client ) {
		return;
	}
	attacker->client->sess.aWeaponStats[G_weapStatIndex_MOD( mod )].headshots++;
	// Store headshot in session as well for overall count
	attacker->client->sess.headshots++;
}

// Resets player's current stats
void G_deleteStats( int nClient ) {
	gclient_t *cl = &level.clients[nClient];

	cl->sess.damage_given = 0;
	cl->sess.damage_received = 0;
	cl->sess.deaths = 0;
	cl->sess.rounds = 0;
	cl->sess.kills = 0;
	cl->sess.suicides = 0;
	cl->sess.team_damage = 0;
	cl->sess.team_kills = 0;
	// Reset new ones as well
	cl->sess.headshots = 0;
	cl->sess.med_given = 0;
	cl->sess.ammo_given = 0;
	cl->sess.gibs = 0;
	cl->sess.revives = 0;
	cl->sess.acc_hits = 0;
	cl->sess.acc_shots = 0;
	cl->sess.killPeak = 0;

	memset( &cl->sess.aWeaponStats, 0, sizeof( cl->sess.aWeaponStats ) );
	trap_Cvar_Set( va( "wstats%i", nClient ), va( "%d", nClient ) );
}

// Parses weapon stat info for given ent
//	---> The given string must be space delimited and contain only integers
void G_parseStats( char *pszStatsInfo ) {
	gclient_t *cl;
	const char *tmp = pszStatsInfo;
	unsigned int i, dwWeaponMask, dwClientID = atoi( pszStatsInfo );

	if ( dwClientID < 0 || dwClientID > MAX_CLIENTS ) {
		return;
	}

	cl = &level.clients[dwClientID];

#define GETVAL( x ) if ( ( tmp = strchr( tmp, ' ' ) ) == NULL ) {return;} x = atoi( ++tmp );

	GETVAL( cl->sess.rounds );
	GETVAL( dwWeaponMask );
	for ( i = WS_KNIFE; i < WS_MAX; i++ ) {
		if ( dwWeaponMask & ( 1 << i ) ) {
			GETVAL( cl->sess.aWeaponStats[i].hits );
			GETVAL( cl->sess.aWeaponStats[i].atts );
			GETVAL( cl->sess.aWeaponStats[i].kills );
			GETVAL( cl->sess.aWeaponStats[i].deaths );
			GETVAL( cl->sess.aWeaponStats[i].headshots );
		}
	}

}

// Writes the weaponstats to a string and returns it (used for wstats%i)
char* G_writeStats( gclient_t* client ) {
	unsigned int i, dwWeaponMask = 0;
	char strWeapInfo[MAX_STRING_CHARS] = { 0 };

	if (!client) {
		return(NULL);
	}

	// Add weapon stats as necessary
	for (i = WS_KNIFE; i < WS_MAX; i++) {
		if (client->sess.aWeaponStats[i].atts || client->sess.aWeaponStats[i].hits ||
			client->sess.aWeaponStats[i].deaths) {
			dwWeaponMask |= (1 << i);
			Q_strcat(strWeapInfo, sizeof(strWeapInfo),
				va(" %d %d %d %d %d",
					client->sess.aWeaponStats[i].hits, client->sess.aWeaponStats[i].atts,
					client->sess.aWeaponStats[i].kills, client->sess.aWeaponStats[i].deaths,
					client->sess.aWeaponStats[i].headshots));
		}
	}

	return(va("%d %d %d%s",
		(int)(client - level.clients),
		client->sess.rounds,
		dwWeaponMask,
		strWeapInfo));
}

// These map to WS_* weapon indexes
// L0: In other words...min shots before it qualifies for top-bottom check..
const int cQualifyingShots[WS_MAX] = {
	20,     // Knife
	14,     // Luger
	14,     // Colt
	32,     // MP40
	30,     // Thompson
	32,     // STEN
	30,     // FG42 (rapid sniper mode)
	3,      // PF
	100,    // Flamer
	5,      // Grenade
	5,      // Mortar (Was I on drugs or am I missing something?)
	5,      // Dynamite
	3,      // Airstrike
	3,      // Artillery
	5,      // Syringe
	3,      // Smoke (Completelly useless..or maybe for "AS cannister kill" when blocking it?)
	50,     // MG42
	10,     // Rifle (sniper/mauser aka scopped-unscopped)
	100		// Venom
};

// ************** TOPSHOTS/BOTTOMSHOTS
//
// Gives back overall or specific weapon rankings
int QDECL SortStats( const void *a, const void *b ) {
	gclient_t   *ca, *cb;
	float accA, accB;

	ca = &level.clients[*(int *)a];
	cb = &level.clients[*(int *)b];

	// then connecting clients
	if ( ca->pers.connected == CON_CONNECTING ) {
		return( 1 );
	}
	if ( cb->pers.connected == CON_CONNECTING ) {
		return( -1 );
	}

	if ( ca->sess.sessionTeam == TEAM_SPECTATOR ) {
		return( 1 );
	}
	if ( cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		return( -1 );
	}

	if ( ( ca->sess.aWeaponStats[iWeap].atts ) < cQualifyingShots[iWeap] ) {
		return( 1 );
	}
	if ( ( cb->sess.aWeaponStats[iWeap].atts ) < cQualifyingShots[iWeap] ) {
		return( -1 );
	}

	accA = (float)( ca->sess.aWeaponStats[iWeap].hits * 100.0 ) / (float)( ca->sess.aWeaponStats[iWeap].atts );
	accB = (float)( cb->sess.aWeaponStats[iWeap].hits * 100.0 ) / (float)( cb->sess.aWeaponStats[iWeap].atts );

	// then sort by accuracy
	if ( accA > accB ) {
		return( -1 );
	}
	return( 1 );
}

// Shows the most accurate players for each weapon to the requesting client
void G_weaponStatsLeaders_cmd( gentity_t* ent, qboolean doTop, qboolean doWindow ) {
	int i, iWeap, shots, wBestAcc, cClients, cPlaces;
	int aClients[MAX_CLIENTS];
	float acc;
	char z[MAX_STRING_CHARS];
	const gclient_t* cl;

	z[0] = 0;
	for ( iWeap = WS_KNIFE; iWeap < WS_MAX; iWeap++ ) {
		wBestAcc = ( doTop ) ? 0 : 99999;
		cClients = 0;
		cPlaces = 0;

		// suckfest - needs two passes, in case there are ties
		for ( i = 0; i < level.numConnectedClients; i++ ) {
			cl = &level.clients[level.sortedClients[i]];

			if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
				continue;
			}

			shots = cl->sess.aWeaponStats[iWeap].atts;
			if ( shots >= cQualifyingShots[iWeap] ) {
				acc = (float)( ( cl->sess.aWeaponStats[iWeap].hits ) * 100.0 ) / (float)shots;
				aClients[cClients++] = level.sortedClients[i];
				if ( ( ( doTop ) ? acc : (float)wBestAcc ) > ( ( doTop ) ? wBestAcc : acc ) ) {
					wBestAcc = (int)acc;
					cPlaces++;
				}
			}
		}

		if ( !doTop && cPlaces < 2 ) {
			continue;
		}

		for ( i = 0; i < cClients; i++ ) {
			cl = &level.clients[ aClients[i] ];
			acc = (float)( cl->sess.aWeaponStats[iWeap].hits * 100.0 ) / (float)( cl->sess.aWeaponStats[iWeap].atts );

			if ( ( ( doTop ) ? acc : (float)wBestAcc + 0.999 ) >= ( ( doTop ) ? wBestAcc : acc ) ) {
				Q_strcat( z, sizeof( z ), va( " %d %d %d %d %d %d", iWeap + 1, aClients[i],
											  cl->sess.aWeaponStats[iWeap].hits,
											  cl->sess.aWeaponStats[iWeap].atts,
											  cl->sess.aWeaponStats[iWeap].kills,
											  cl->sess.aWeaponStats[iWeap].deaths ) );
			}
		}
	}
	CP( va( "%sbstats%s %s 0", ( ( doWindow ) ? "w" : "" ), ( ( doTop ) ? "" : "b" ), z ) );
}

// ************** STATSALL
//
// Shows all players' stats to the requesting client.
void G_statsall_cmd( gentity_t *ent, unsigned int dwCommand, qboolean fDump ) {
	int i;
	gentity_t *player;

	for ( i = 0; i < level.numConnectedClients; i++ ) {
		player = &g_entities[level.sortedClients[i]];
		if ( player->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		CP( va( "ws %s\n", G_createStats( player ) ) );
	}
}


// Shows best/worst accuracy for all weapons, or sorted
// accuracies for a single weapon.
void G_weaponRankings_cmd( gentity_t *ent, unsigned int dwCommand, qboolean state ) {
	gclient_t *cl;
	int c = 0, i, shots, wBestAcc;
	char z[MAX_STRING_CHARS];

	if ( trap_Argc() < 2 ) {
		G_weaponStatsLeaders_cmd( ent, state, qfalse );
		return;
	}

	wBestAcc = ( state ) ? 0 : 99999;

	// Find the weapon
	trap_Argv( 1, z, sizeof( z ) );
	if ( ( iWeap = atoi( z ) ) == 0 || iWeap < WS_KNIFE || iWeap >= WS_MAX ) {
		for (iWeap = WS_VENOM; iWeap >= WS_KNIFE; iWeap--) {
			if ( !Q_stricmp( z, aWeaponInfo[iWeap].pszCode ) ) {
				break;
			}
		}
	}

	if ( iWeap < WS_KNIFE ) {
		CP( va( "print \"\n^zInfo: %s\n\n\"",
			(state ?
				"^7 Shows BEST player for each weapon. Add ^3<weapon_ID>^7 to show all stats for a weapon " :
				"^7 Shows WORST player for each weapon. Add ^3<weapon_ID>^7 to show all stats for a weapon" )
		));

		Q_strncpyz( z, "^zAvailable weapon codes:^7\n", sizeof( z ) );
		for ( i = WS_KNIFE; i < WS_MAX; i++ ) {
			Q_strcat( z, sizeof( z ), va( "  %s - %s\n", aWeaponInfo[i].pszCode, aWeaponInfo[i].pszName ) );
		}
		CP( va( "print \"%s\"", z ) );
		return;
	}

	memcpy( &level.sortedStats, &level.sortedClients, sizeof( level.sortedStats ) );
	qsort( level.sortedStats, level.numConnectedClients, sizeof( level.sortedStats[0] ), SortStats );

	z[0] = 0;
	for ( i = 0; i < level.numConnectedClients; i++ ) {
		cl = &level.clients[level.sortedStats[i]];

		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		shots = cl->sess.aWeaponStats[iWeap].atts;
		if ( shots >= cQualifyingShots[iWeap] ) {
			float acc = (float)( cl->sess.aWeaponStats[iWeap].hits * 100.0 ) / (float)shots;

			c++;
			wBestAcc = ( ( ( state ) ? acc : wBestAcc ) > ( ( state ) ? wBestAcc : acc ) ) ? (int)acc : wBestAcc;
			Q_strcat( z, sizeof( z ), va( " %d %d %d %d %d", level.sortedStats[i],
										  cl->sess.aWeaponStats[iWeap].hits,
										  shots,
										  cl->sess.aWeaponStats[iWeap].kills,
										  cl->sess.aWeaponStats[iWeap].deaths ) );
		}
	}
	CP( va( "astats%s %d %d %d%s", ( ( state ) ? "" : "b" ), c, iWeap, wBestAcc, z ) );
}
void G_printMatchInfo( gentity_t *ent, qboolean fDump ) { // fDump is bad name but temporary fix for cg_autoaction issue
	int i, j, cnt, eff;
	float tot_acc = 0.00f;
	int tot_rev, tot_kills, tot_deaths, tot_gp, tot_hs, tot_sui, tot_tk, tot_dg, tot_dr, tot_td, tot_hits, tot_shots, tot_gib;
	gclient_t *cl;
	char *ref;
	char n1[MAX_NETNAME];
	char n2[MAX_NETNAME];
	qtime_t ct;
	trap_RealTime(&ct);
	CP(va("sc \"\nMod: %s \n^7Server: %s  \n^7Time: ^7%02d:%02d:%02d ^d(^7%02d %s %d^d)\n\n\"",
			GAMEVERSION, sv_hostname.string, ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday, dMonths[ct.tm_mon], 1900+ct.tm_year));

	cnt = 0;
	for ( i = TEAM_RED; i <= TEAM_BLUE; i++ ) {
		if ( !TeamCount( -1, i ) ) {
			continue;
		}

		tot_kills = 0;
		tot_deaths = 0;
		tot_hs = 0;
		tot_sui = 0;
		tot_tk = 0;
		tot_dg = 0;
		tot_dr = 0;
		tot_td = 0;
		tot_gib = 0;
		tot_gp = 0;
		tot_hits = 0;
		tot_shots = 0;
		tot_acc = 0;
		tot_rev = 0;
		CP(va("sc \"%s ^7Team\n"
			     "^7--------------------------------------------------------------------------"
				 "\nPlayer          ^eKll ^7Dth Sui TK ^cEff ^7Gib Accrcy HS   ^2DG   ^1DR   ^4TD  ^5Rev ^3Score\n"
				 "^7--------------------------------------------------------------------------\n\"", (i == TEAM_RED) ? "^1Axis" : "^4Allied"  ));

		for ( j = 0; j < level.numPlayingClients; j++ ) {
			cl = level.clients + level.sortedClients[j];

			if ( cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam != i ) {
				continue;
			}
            //(i == TEAM_RED) ? "^1Axis" : "^4Allied"  )
			// Bug fix - ^Pentagram always manages to break stats so it needs different approach. ^^
			DecolorString(cl->pers.netname, n1);
			SanitizeString(n1, n2);
			Q_CleanStr(n2);
			n2[15] = 0;
            ref = "^7";

			tot_kills += cl->sess.kills;
			tot_deaths += cl->sess.deaths;
			tot_sui += cl->sess.suicides;
			tot_tk += cl->sess.team_kills;
			tot_hs += cl->sess.headshots;
			tot_dg += cl->sess.damage_given;
			tot_gib += cl->sess.gibs;
			tot_dr += cl->sess.damage_received;
			tot_td += cl->sess.team_damage;
			tot_gp += cl->ps.persistant[PERS_SCORE];
			tot_hits += cl->sess.acc_hits;
			tot_shots += cl->sess.acc_shots;
			tot_rev += cl->sess.revives;

			eff = ( cl->sess.deaths + cl->sess.kills == 0 ) ? 0 : 100 * cl->sess.kills / ( cl->sess.deaths + cl->sess.kills );
			if ( eff < 0 ) {
				eff = 0;
			}

			if ( ent->client == cl ||
				 ( ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
				   ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
				   ent->client->sess.spectatorClient == level.sortedClients[j] ) ) {
				ref = "^3";
			}

			cnt++;
			CP(va("sc \"%s%-15s^e%4d^7%4d%4d%3d%s^c%4d^7%3d %6.2f%4d^2%5d^1%5d^4%5d^5%5d^7%5d\n\"",
				ref,
					n2,
					cl->sess.kills,
					cl->sess.deaths,
					cl->sess.suicides,
					cl->sess.team_kills,
					ref,
					eff,
					cl->sess.gibs,
					((cl->sess.acc_shots == 0) ? 0.00 : ((float)cl->sess.acc_hits / (float)cl->sess.acc_shots) * 100.00f),
					cl->sess.headshots,
					cl->sess.damage_given,
					cl->sess.damage_received,
					cl->sess.team_damage,
					cl->sess.revives,
					cl->ps.persistant[PERS_SCORE] ) );
			eff = (cl->sess.deaths + cl->sess.kills == 0) ? 0 : 100 * cl->sess.kills / (cl->sess.deaths + cl->sess.kills);
			if (eff < 0) {
				eff = 0;
			}

			if (ent->client == cl ||
				(ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
					ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
					ent->client->sess.spectatorClient == level.sortedClients[j])) {
				ref = "^7";
			}
		}

		eff = ( tot_kills + tot_deaths == 0 ) ? 0 : 100 * tot_kills / ( tot_kills + tot_deaths );
		if ( eff < 0 ) {
			eff = 0;
		}
		tot_acc = ( (tot_shots == 0) ? 0.00 : ((float)tot_hits / (float)tot_shots ) * 100.00f );

		CP( va( "sc \"^7--------------------------------------------------------------------------\n"
				"%-19s^e%4d^7%4d%4d%3d%4d%3d ^7%6.2f%4d^2%5d^1%5d^4%5d^5%5d^7%5d\n\n\n\"",
				"^eTotals^7",
				tot_kills,
				tot_deaths,
				tot_sui,
				tot_tk,
				eff,
				tot_gib,
				tot_acc,
				tot_hs,
				tot_dg,
				tot_dr,
				tot_td,
				tot_rev,
				tot_gp ) );

	}
	// temp for printing clock & end of round sounds
	if (fDump && ( g_gametype.integer == GT_WOLF_STOPWATCH ))
    {
        G_matchClockDump( ent );
    }


	CP( va( "sc \"%s\n\" 0", ( ( !cnt ) ? "^3\nNo scores to report." : "" ) ) );

}

// Dumps end-of-match info
void G_matchInfoDump( unsigned int dwDumpType ) {
	int i, ref;
	gentity_t *ent;
	gclient_t *cl;

	// sswolf - move the announcer sound from WM_DrawObjectives in cg, here,
	// to at least temporarily fix whatevr is causing the cut off
	char cs[MAX_STRING_CHARS];
	char* buf;
	char* endofroundinfo;
	int winner;
	trap_GetConfigstring(CS_MULTI_MAPWINNER, cs, sizeof(cs));
	buf = Info_ValueForKey(cs, "winner");
	winner = atoi(buf);

    endofroundinfo=va( "  .."); // plan to remove this soon.....just safety measure

	for ( i = 0; i < level.numConnectedClients; i++ )
	{
		ref = level.sortedClients[i];
		ent = &g_entities[ref];
		cl = ent->client;

		if ( cl->pers.connected != CON_CONNECTED )
		{
			continue;
		}

		if ( dwDumpType == EOM_WEAPONSTATS )
		{
			// If client wants to write stats to a file, don't auto send this stuff
			if (!(cl->pers.clientFlags & CGF_STATSDUMP)) {
				if ((cl->pers.autoaction & AA_STATSALL) /*|| cl->pers.mvCount > 0*/)
				{
					G_statsall_cmd(ent, 0, qfalse);
				}
				else if (cl->sess.sessionTeam != TEAM_SPECTATOR)
				{
					if (cl->pers.autoaction & AA_STATSTEAM)
					{
						G_statsall_cmd(ent, cl->sess.sessionTeam, qfalse);
					}
					else
					{
						CP(va("ws %s\n", G_createStats(ent)));
					}

				}
				else if (cl->sess.spectatorState != SPECTATOR_FREE)
				{
					int pid = cl->sess.spectatorClient;

					if ((cl->pers.autoaction & AA_STATSTEAM)) {
						G_statsall_cmd(ent, level.clients[pid].sess.sessionTeam, qfalse);
					}
					else
					{
						CP(va("ws %s\n", G_createStats(g_entities + pid)));
					}

				}
			}

			// Log it
			if ( cl->sess.sessionTeam != TEAM_SPECTATOR )
			{
				G_LogPrintf( "WeaponStats: %s\n", G_createStats( ent ) );
			}

		}

		else if ( dwDumpType == EOM_MATCHINFO )
		{
			// Don't dump score table for users with stats dump enabled
			if (!(cl->pers.clientFlags & CGF_STATSDUMP))
			{
				G_printMatchInfo(ent,qtrue);
			}
        // moved to G_matchClockDump due to cg_autoaction issue

			if ( g_gametype.integer == GT_WOLF_STOPWATCH )
			{
				// We've already missed the switch
				if ( g_currentRound.integer == 1 )
				{
                    endofroundinfo=va( "Clock set to: %d:%02d",
							g_nextTimeLimit.integer,
							(int)( 60.0 * (float)( g_nextTimeLimit.value - g_nextTimeLimit.integer ) ) );
					//CP( va( "sc \">>> ^3%s\n\"",endofroundinfo) ) ;


                    /*
					if (winner == 0)
					{
						AAPS("sound/match/winaxis.wav");
					}
					else if (winner == 1)
					{
						AAPS("sound/match/winallies.wav");
					}
					*/

				}
				else
				{

					float val = (float)( ( level.timeCurrent - ( level.startTime + level.time - level.intermissiontime ) ) / 60000.0 );
					if ( val < g_timelimit.value )
					{
					    endofroundinfo=va( "Objective reached at %d:%02d (original: %d:%02d)",
								(int)val,
								(int)( 60.0 * ( val - (int)val ) ),
								g_timelimit.integer,
								(int)( 60.0 * (float)( g_timelimit.value - g_timelimit.integer ) ) ) ;
						//CP( va( "sc \">>> ^3%s\n\"",endofroundinfo) ) ;


                        /*
						if (winner == 0)
						{
							AAPS("sound/match/winaxis.wav");
						}
						else if (winner == 1)
						{
							AAPS("sound/match/winallies.wav");
						}
						*/

					}
					else
					{
					    endofroundinfo=va( "Objective NOT reached in time (%d:%02d)",
								g_timelimit.integer,
								(int)( 60.0 * (float)( g_timelimit.value - g_timelimit.integer ) ) );
						//CP( va( "sc \">>> ^3%s\n\"",endofroundinfo) );


                        /*
						if (winner == 0)
						{
							AAPS("sound/match/winaxis.wav");
						}
						else if (winner == 1)
						{
							AAPS("sound/match/winallies.wav");
						}
						*/
					}
				}
			}

			// sswolf - non SW exits
			//else if (g_gametype.integer == GS_PLAYING)
			if (g_gametype.integer == GS_PLAYING)
			{


				if (g_timelimit.value && !level.warmupTime)
				{
					if (level.time - level.startTime >= g_timelimit.value * 60000)
					{
						if (winner == 0)
						{
							AAPS("sound/match/winaxis.wav");
						}
						else if (winner == 1)
						{
							AAPS("sound/match/winallies.wav");
						}
					}
					else
					{
						if (winner == 0)
						{
							AAPS("sound/match/winaxis.wav");
						}
						else if (winner == 1)
						{
							AAPS("sound/match/winallies.wav");
						}
					}
				}
			}
		}
	}
   // if (qtrue) {  // may want to use different cvar for event log vs. gamestat log
    if (g_gameStatslog.integer) {
        G_writeGameLogEnd(endofroundinfo);  // write last event and close the gamelog array...will provide better solution later
        G_writeGameInfo(winner);  // write out the game info relating to the match & round
        G_stats2JSON(winner); // write out the player stats
        G_writeClosingJson();  // need a closing bracket....will provide better solution later

    }
}
// temp fix for cg_autoaction issue
void G_matchClockDump( gentity_t *ent ) {

	char cs[MAX_STRING_CHARS];
	char* buf;
	int winner;
	trap_GetConfigstring(CS_MULTI_MAPWINNER, cs, sizeof(cs));
	buf = Info_ValueForKey(cs, "winner");
	winner = atoi(buf);
	char* endofroundinfo;

    if ( !level.intermissiontime ) {
		return;
	}

               if ( g_currentRound.integer == 1 )
				{
                    endofroundinfo=va( "Clock set to: %d:%02d",
							g_nextTimeLimit.integer,
							(int)( 60.0 * (float)( g_nextTimeLimit.value - g_nextTimeLimit.integer ) ) );
					CP( va( "sc \">>> ^3%s\n\"",endofroundinfo) ) ;

					if (winner == 0)
					{
						AAPS("sound/match/winaxis.wav");
					}
					else if (winner == 1)
					{
						AAPS("sound/match/winallies.wav");
					}

				}
				else
				{
					float val = (float)( ( level.timeCurrent - ( level.startTime + level.time - level.intermissiontime ) ) / 60000.0 );
					if ( val < g_timelimit.value )
					{
					    endofroundinfo=va( "Objective reached at %d:%02d (original: %d:%02d)",
								(int)val,
								(int)( 60.0 * ( val - (int)val ) ),
								g_timelimit.integer,
								(int)( 60.0 * (float)( g_timelimit.value - g_timelimit.integer ) ) ) ;
						CP( va( "sc \">>> ^3%s\n\"",endofroundinfo) ) ;

						if (winner == 0)
						{
							AAPS("sound/match/winaxis.wav");
						}
						else if (winner == 1)
						{
							AAPS("sound/match/winallies.wav");
						}

					}
					else
					{
					    endofroundinfo=va( "Objective NOT reached in time (%d:%02d)",
								g_timelimit.integer,
								(int)( 60.0 * (float)( g_timelimit.value - g_timelimit.integer ) ) );
						CP( va( "sc \">>> ^3%s\n\"",endofroundinfo) );

						if (winner == 0)
						{
							AAPS("sound/match/winaxis.wav");
						}
						else if (winner == 1)
						{
							AAPS("sound/match/winallies.wav");
						}

					}
				}


}
