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

L0 - Port from ET for stats code by OSP

================================
*/

#include "q_shared.h"
#include "bg_public.h"

//	--> WP_* to WS_* conversion
static const weap_ws_convert_t aWeapID[WP_NUM_WEAPONS] = {

	{ WP_NONE,              WS_MAX },           // 0

	// German weapons
	{ WP_KNIFE,             WS_KNIFE },
	{ WP_LUGER,             WS_LUGER },
	{ WP_MP40,              WS_MP40 },
	{ WP_GRENADE_LAUNCHER,  WS_GRENADE },       // 5
	{ WP_PANZERFAUST,       WS_PANZERFAUST },
	{ WP_FLAMETHROWER,      WS_FLAMETHROWER },
	{ WP_VENOM,				WS_VENOM },

	// American equivalents
	{ WP_COLT,              WS_COLT },          // 10
	{ WP_THOMPSON,          WS_THOMPSON },
	{ WP_GRENADE_PINEAPPLE, WS_GRENADE },
	{ WP_STEN,              WS_STEN },
	{ WP_MEDIC_SYRINGE,     WS_SYRINGE },
	{ WP_AMMO,              WS_MAX },
	{ WP_ARTY,              WS_ARTILLERY },

	{ WP_SILENCER,          WS_LUGER },         // 20
	{ WP_DYNAMITE,          WS_DYNAMITE },
	{ WP_SMOKETRAIL,        WS_ARTILLERY },
	{ VERYBIGEXPLOSION,     WS_MAX },
	{ WP_MEDKIT,            WS_MAX },
	{ WP_BINOCULARS,        WS_MAX },

	{ WP_PLIERS,            WS_MAX },
	{ WP_MAUSER,			WS_RIFLE },
	{ WP_SNIPERRIFLE,       WS_RIFLE },
	
	{ WP_FG42,              WS_FG42 },
	{ WP_FG42SCOPE,         WS_FG42 },
	
	{ WP_MORTAR,            WS_MORTAR }

	// L0 - Did I cover all?
};


// Get right stats index based on weapon id
extWeaponStats_t BG_WeapStatForWeapon( weapon_t iWeaponID ) {
	weapon_t i;

	for ( i = WP_NONE; i < WP_NUM_WEAPONS; i++ ) {
		if ( iWeaponID == aWeapID[i].iWeapon ) {
			return aWeapID[i].iWS;
		}
	}

	return WS_MAX;
}
