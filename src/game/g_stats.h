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
L0 - g_stats.h

Stats stuff

Created: 9. Apr / 2013
Last Updated: 
===========================================================================
*/

/**** Killing sprees (consistent over round) ****/
typedef struct {
	char *msg;
	char *snd;	
} killing_sprees_t;

static const killing_sprees_t killingSprees[] = {
	{"MULTI KILL!", "multikill.wav"},
	{"MEGA KILL!", "megakill.wav"},
	{"RAMPAGE!", "rampage.wav"},
	{"GUNSLINGER!", "gunslinger.wav"},
	{"ULTRA KILL!", "ultrakill.wav"},
	{"MANIAC!", "maniac.wav"},
	{"SLAUGHTER!", "slaughter.wav"},
	{"MASSACRE!", "massacre.wav"},
	{"IMPRESSIVE!", "impressive.wav"},
	{"DOMINATING!", "dominating.wav"},
	{"BLOODBATH!", "bloodbath.wav"},
	{"KILLING MACHINE!", "killingmachine.wav"},
	{"MONSTER KILL!", "monsterkill.wav"},
	{"LUDICROUS KILL!", "ludicrouskill.wav"},
	{"UNSTOPPABLE!", "unstoppable.wav"},
	{"UNREAL!", "unreal.wav"},
	{"OUTSTANDING!", "outstanding.wav"},
	{"WICKED SICK!", "wickedsick.wav"},
	{"HOLY SHIT!", "holyshit.wav"},
	{"BLAZE OF GLORY!!", "blazeofglory.wav"},
	{NULL, NULL}
};

/**** Killer Sprees (resets when player dies) ****/
typedef struct {
	char *msg;
	char *snd;	
} killer_sprees_t;

static const killer_sprees_t killerSprees[] = {
	{"MULTI KILL!" , "multikill.wav"},
	{"KILLING SPREE!" , "killingspree.wav"},
	{"RAMPAGE!" , "rampage.wav"},
	{"ULTRA KILL!" , "ultraKill.wav"},
	{"MONSTER KILL!" , "monsterkill.wav"},
	{"LUDICROUS KILL!" , "ludicrouskill.wav"},
	{"DOMINATING!" , "dominating.wav"},
	{"GODLIKE!" , "godlike.wav"},
	{"UNSTOPPABLE!" , "unstoppable.wav"},
	{"WICKED SICK!" , "wickedsick.wav"},
	{"HOLY SHIT!!" , "holyshit.wav"},
	{NULL, NULL}
};
