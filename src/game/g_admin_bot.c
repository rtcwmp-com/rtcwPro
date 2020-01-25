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
g_admin_bot.c

Deals with game admining

Author: Nate 'L0
Created: 2.Nov/12
Updated: 5.Jan/12
===========================================================================
*/
#include "g_local.h"

/*
===========
Kicks for teamkills
===========
*/
void sb_maxTeamKill( gentity_t *ent ) {	
	int count = ent->client->pers.sb_teamKills;

	if (level.warmupTime || !sab_system.integer || sab_maxTeamKills.integer == (-1))
		return;

	if ( sab_maxTeamKills.integer - ent->client->pers.sb_teamKills == 1 ) 		
		AP(va("chat \"^j[WARNING]: ^7%s ^7gets kicked on next ^zTeam Kill^7!\n\"", ent->client->pers.netname));

	if ((count >= sab_maxTeamKills.integer) && (ent->client->pers.sb_teamKills)) {	
		trap_DropClient( ent-g_entities, "Kicked \n^3For Team Killing." );		
		AP(va("chat \"^zSAB^7: %s ^7got kicked for ^zTeam Killing^7.\n\"", ent->client->pers.netname));
	return;
	}	

	ent->client->pers.sb_teamKills++;	
return;
}

/*
===========
Kicks for bleeding
===========
*/
void sb_maxTeamBleed( gentity_t *ent ) {	
	int count = ent->client->pers.sb_teamBleed;

	if (level.warmupTime || !sab_system.integer || sab_maxTeamBleed.integer == (-1))
		return;

	if ( sab_maxTeamBleed.integer - ent->client->pers.sb_teamBleed == 10 ) 		
		AP(va("chat \"^j[WARNING]: ^7%s ^7is getting close to being kicked for ^zBleeding^7!\n\"", ent->client->pers.netname));

	if ((count >= sab_maxTeamBleed.integer) && (ent->client->pers.sb_teamBleed)) {
		trap_DropClient( ent-g_entities, "Kicked \n^3For Team Wounding." );	
		AP(va("chat \"^zSAB^7: %s ^7got kicked for ^zTeam Wounding^7.\n\"", ent->client->pers.netname));
	}
return;
}

/*
===========
Kicks for low score
===========
*/
void sb_minLowScore( gentity_t *ent ) {	
												// Positive values are ignored.
	if (level.warmupTime || !sab_system.integer || sab_minLowScore.integer >= 0)
		return;

	if (ent->client->ps.persistant[PERS_SCORE] < sab_minLowScore.integer){ 	
		AP(va("chat \"^zSAB^7: %s ^7got kicked for ^zLow Score^7.\n\"", ent->client->pers.netname));
		trap_DropClient(ent->client->ps.clientNum, "Kicked \n^3For Low Score.");
	}
return;
}

/*
===========
Kicks laggers
===========
*/
void sb_maxPingFlux( gclient_t *client ) {
	int max=sab_MaxPingFlux.integer;
	int times=sab_maxPingHits.integer;

	// 50 is sanity check..so someone doesn't screw it with sab_maxPingFlux "1"
	if (level.warmupTime || !sab_system.integer || max < 50 || 
		client->sess.sessionTeam != TEAM_SPECTATOR || client->ps.pm_type != PM_DEAD)
		return;
		
	if (client->pers.sb_ping >= times ) {
		trap_DropClient(client - level.clients, "Kicked \nDue unstable ping or max ping limit.");			
		AP(va("chat \"^zSAB^7: %s ^7got kicked due ^zUnstable Ping^7.\n\"", client->pers.netname));
	return;
	// 1 hit per second
	} else if (client->ps.ping >= max && level.time > level.sb_maxPing) {
			client->pers.sb_ping++;
			level.sb_maxPing = level.time + 1000; 
	return;
	}
return;
}

/*
===========
Censoring penalty
===========
*/
void sb_chatWarn(gentity_t *ent) {
	int n = rand() % 4; // Randomize messages
	
	if (!sab_system.integer || (sab_system.integer && !sab_censorPenalty.integer)) 
		return;

	// Only for non logged in users..
	// Chat will still get censored they just wont get kicked or ignored for it..
	if (ent->client->sess.admin != ADM_NONE) 
		return;

	if (ent->client->pers.sb_chatWarned == 0) {
		if (n == 0) 			
			CP("chat \"^3Strike one! ^7You should really wash your mouth.\n\"");
		else if (n == 1)
			CP("chat \"^3Strike one! ^7You got warned for foul language..\n\"");		
		else if (n == 2)
			CP("chat \"^3Strike one! ^7This is not your local pub..\n\"");		
		else 			
			CP("chat \"^3Strike one! ^7Cursing is not allowed here.\n\"");
	}
	else if (ent->client->pers.sb_chatWarned == 1) {
		if (n == 0) 		
			CP("chat \"^3Strike two! ^7Don't push it..\n\"");
		else if (n == 1) 			
			CP("chat \"^3Strike two! ^7You where warned..\n\"");
		else if (n == 2) 			
			CP("chat \"^3Strike two! ^7Do you talk to your parents like this?\n\"");
		else
			CP("chat \"^3Strike two! ^7Foul language is not allowed here.\n\"");			
	}
	else if (ent->client->pers.sb_chatWarned == 2) {
		if (n == 0) 
			CP("chat \"^3Strike three! ^7Last warning!\n\"");			
		else if (n == 1)
			CP("chat \"^3Strike three! ^7There wont be strike four..\n\"");			
		else if (n == 2)
			CP("chat \"^3Strike three! ^7There's no more warnings after this one.\n\"");		
		 else 
			CP("chat \"^3Strike three! ^7Care to see how strike four looks like?\n\"");
	} else {		
		if (sab_censorPenalty.integer == 1) {
			ent->client->sess.ignored = 1;
			AP(va("chat \"^zSAB^7: %s ^7has been ignored due foul language^z!\n\"", ent->client->pers.netname));
		} else {
			AP(va("chat \"^zSAB^7: %s ^7got kicked for foul language^z!\n\"", ent->client->pers.netname));
			trap_DropClient(ent-g_entities,va("^3kicked for ^3Foul ^3Language!"));
		}		
	return;
	}

	// Count it..
	ent->client->pers.sb_chatWarned++;
	// Add a sound
	CPS(ent, "sound/game/admin/warn.wav");
return;
}
