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
L0 - g_admin.h

Headers and declarations for admins.

Created: 17. Oct / 2012
Last Updated: 28. Apr / 2013
===========================================================================
*/

// g_admin.c
qboolean cmds_admin(char cmd[MAX_TOKEN_CHARS], gentity_t *ent);
void ParseAdmStr(const char *strInput, char *strCmd, char *strArgs);
void cmd_incognito(gentity_t *ent);
void cmd_do_logout(gentity_t *ent);
void cmd_do_login (gentity_t *ent, qboolean silent);
void cmd_getstatus(gentity_t *ent);
int ClientNumberFromNameMatch(char *name, int *matches);
char *sortTag(gentity_t *ent);

// g_admin_bot.c
void sb_maxTeamKill(gentity_t *ent);
void sb_maxTeamBleed(gentity_t *ent);
void sb_minLowScore(gentity_t *ent);
void sb_maxPingFlux(gclient_t *client);
void sb_chatWarn(gentity_t *ent);

// g_cmds.c
int ClientNumberFromString( gentity_t *to, char *s );
void SanitizeString( char *in, char *out ); 
char *ConcatArgs( int start );

// g_main.c 
void CheckVote( void );
void sortedActivePlayers(void);

// Sounds - so it's simplified
void CPSound(gentity_t *ent, char *sound);
void APSound(char *sound);
void APRSound(gentity_t *ent, char *sound);

// Macros
#define APS(x) APSound(x)									// Global sound 
#define APRS(x, y) APRSound(x, y)							// Global sound with limited range
#define CPS(x, y) CPSound(x, y)								// Client sound only
#define AP(x) trap_SendServerCommand(-1, x)					// Print to all
#define CP(x) trap_SendServerCommand(ent-g_entities, x)		// Print to an ent
#define CPx(x, y) trap_SendServerCommand(x, y)				// Print to id = x
#define TP(x,y,z) G_SayToTeam(x, y, z)						// Prints to selected team
