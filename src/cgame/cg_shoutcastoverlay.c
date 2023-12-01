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
Copied from * ET: Legacy
 * Copyright (C) 2012-2023 ET:Legacy team <mail@etlegacy.com>
 *
 * This file is part of ET: Legacy - http://www.etlegacy.com
 *
 * ET: Legacy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ET: Legacy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "cg_local.h"
#include "../ui/ui_shared.h"

displayContextDef_t cgDC;

#define MAX_PLAYERS     6

#define FONT_HEADER         &cgDC.Assets.textFont
#define FONT_TEXT           &cgDC.Assets.textFont

#define PLAYER_LIST_STATUS_WIDTH 28
#define PLAYER_LIST_STATUS_HEIGHT 28

#define PLAYER_LIST_WIDTH 142
#define PLAYER_LIST_HEIGHT 28
#define PLAYER_LIST_X 15
#define PLAYER_LIST_Y (SCREEN_HEIGHT - 180)

#define PLAYER_STATUS_NAMEBOX_WIDTH 150
#define PLAYER_STATUS_NAMEBOX_HEIGHT 16
#define PLAYER_STATUS_NAMEBOX_X (SCREEN_WIDTH - (SCREEN_WIDTH / 2) - (PLAYER_STATUS_NAMEBOX_WIDTH / 2))
#define PLAYER_STATUS_NAMEBOX_Y (SCREEN_HEIGHT - 75)

#define PLAYER_STATUS_STATSBOX_WIDTH 260
#define PLAYER_STATUS_STATSBOX_HEIGHT 30
#define PLAYER_STATUS_STATSBOX_X (SCREEN_WIDTH - (SCREEN_WIDTH / 2) - (PLAYER_STATUS_STATSBOX_WIDTH / 2))
#define PLAYER_STATUS_STATSBOX_Y (PLAYER_STATUS_NAMEBOX_Y + PLAYER_STATUS_NAMEBOX_HEIGHT)

#define GAMETIME_WIDTH 60
#define GAMETIME_HEIGHT 30
#define GAMETIME_X (SCREEN_WIDTH / 2) - (GAMETIME_WIDTH / 2)
#define GAMETIME_Y 12

#define TEAMNAMES_WIDTH 190
#define TEAMNAMES_HEIGHT (GAMETIME_HEIGHT)

#define POWERUPS_WIDTH 36
#define POWERUPS_HEIGHT 36

static vec4_t bg = { 0.0f, 0.0f, 0.0f, 0.7f };

static vec4_t colorAllies = { 0.121f, 0.447f, 0.811f, 0.45f };
static vec4_t colorAxis   = { 0.749f, 0.129f, 0.129f, 0.45f };

int players[12];

/*
=============================
RTCWPro
CG_SCSortDistance
=============================
*/
int QDECL CG_SCSortDistance(const void* a, const void* b) {
	scItem_t* A = (scItem_t*)a;
	scItem_t* B = (scItem_t*)b;

	if (A->dist < B->dist)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

/*
=============================
RTCWPro
CG_ShoutcasterItems -- called form CG_DrawActive
=============================
*/
void CG_ShoutcasterItems() {
	int			i;
	centity_t* cent;

	memset(cg.scItems, 0, MAX_SCITEMS * sizeof(cg.scItems[0]));
	cg.numSCItems = 0;

	for (i = 0; i < MAX_ENTITIES; i++)
	{
		cent = &cg_entities[i];

		if (!cent->currentValid)
			continue;

		switch (cent->currentState.eType)
		{
		case ET_MISSILE:
			CG_ShoutcasterDynamite(i);
			break;
		default:
			break;
		}
	}

	// Sort Dynamite numbers
	qsort(cg.scItems, cg.numSCItems, sizeof(cg.scItems[0]), CG_SCSortDistance);

	// Draw Dynamite numbers
	for (i = 0; i < cg.numSCItems; i++)
	{
		CG_Text_Paint(cg.scItems[i].position[0], cg.scItems[i].position[1], cg.scItems[i].scale, cg.scItems[i].color, cg.scItems[i].text, 0, 0, ITEM_TEXTSTYLE_NORMAL);
	}

	// Players List
	if (cg_shoutcastDrawPlayers.integer && cgs.gamestate != GS_INTERMISSION)
	{
		CG_DrawShoutcastPlayerList();
	}

	// Players Status
	if ((cg.snap->ps.pm_flags & PMF_FOLLOW) && cgs.gamestate != GS_INTERMISSION)
	{
		CG_DrawShoutcastPlayerStatus();
	}
}
/*
=============================
RTCWPro
CG_Shoutcaster_Dynamite
=============================
*/
void CG_ShoutcasterDynamite(int num) {
	centity_t* cent;
	vec3_t		origin;
	vec_t		position[2];

	cent = &cg_entities[num];
	if (cent->currentState.eType != ET_MISSILE || cent->currentState.weapon != WP_DYNAMITE || cent->currentState.teamNum >= 4)
		return;

	if (!cent->currentValid)
		return;

	// Ent visible?
	if (PointVisible(cent->lerpOrigin))
		cent->lastSeenTime = cg.time;

	// Break if no action
	if (!cent->lastSeenTime || cg.time - cent->lastSeenTime >= 1000)
		return;

	// Ent position
	VectorCopy(cent->lerpOrigin, origin);

	// Add height, plus a little
	origin[2] += 20;

	if (!VisibleToScreen(origin, position))
	{
		return;
	}

	cg.scItems[cg.numSCItems].position[0] = position[0] / cgs.screenXScale;
	cg.scItems[cg.numSCItems].position[1] = position[1] / cgs.screenYScale;

	// Distance to player
	cg.scItems[cg.numSCItems].dist = VectorDistance(cg.predictedPlayerState.origin, origin);
	cg.scItems[cg.numSCItems].scale = 600 / cg.scItems[cg.numSCItems].dist * 0.2f;

	// Figure out color
	CG_ColorForPercent(100 * (30000 - cg.time + cent->currentState.effect1Time + 1000) / 30000, cg.scItems[cg.numSCItems].color);
	cg.scItems[cg.numSCItems].color[3] = 1 - ((float)(cg.time - cent->lastSeenTime) / 1000.f);

	// Center text
	cg.scItems[cg.numSCItems].text = va("%i", ((30000 - cg.time + cent->currentState.effect1Time) / 1000) + 1);
	cg.scItems[cg.numSCItems].position[0] -= CG_Text_Width_Ext(cg.scItems[cg.numSCItems].text, cg.scItems[cg.numSCItems].scale, 0, &cgDC.Assets.textFont) / 2;

	// Paint the text.
	//CG_Text_Paint_Ext( position[0], position[1], scale, scale, color, str, 0, 0, ITEM_TEXTSTYLE_NORMAL, &cgs.media.limboFont1 );

	// Increment number of items
	cg.numSCItems++;
}

/**
 * @brief Get the current ammo and/or clip count of the holded weapon (if using ammo).
 * @param[out] ammo - the number of ammo left (in the current clip if using clip)
 * @param[out] clips - the total ammount of ammo in all clips (if using clip)
 * @param[out] akimboammo - the number of ammo left in the second pistol of akimbo (if using akimbo)
 * @param[out] colorAmmo
 */
void CG_PlayerAmmoValue(int* ammo, int* clips, int* akimboammo, vec4_t** colorAmmo /*, vec4_t **colorClip*/)
{
	centity_t* cent;
	playerState_t* ps;
	weapon_t      weap;

	*ammo = *clips = *akimboammo = -1;

	if (cg.snap->ps.clientNum == cg.clientNum)
	{
		cent = &cg.predictedPlayerEntity;
	}
	else
	{
		cent = &cg_entities[cg.snap->ps.clientNum];
	}
	ps = &cg.snap->ps;

	weap = (weapon_t)cent->currentState.weapon;

	if (!IS_VALID_WEAPON(weap))
	{
		return;
	}

	// some weapons don't draw ammo count
	//if (!GetAmmoTableData(weap)->useAmmo)
	//{
	//	return;
	//}

	if (cg.snap->ps.eFlags & EF_MG42_ACTIVE)
	{
		return;
	}

	// total ammo in clips, grenade launcher is not a clip weapon but show the clip anyway
	if (1) //GetAmmoTableData(weap)->useClip || (weap == WP_M7 || weap == WP_GPG40))
	{
		// current reserve
		*clips = cg.snap->ps.ammo[BG_FindAmmoForWeapon(weap)];

		// current clip
		*ammo = ps->ammoclip[BG_FindClipForWeapon(weap)];

		if (colorAmmo)
		{
			float maxClip = GetAmmoTableData(weap)->maxclip * (*akimboammo != -1 ? 2 : 1);
			float totalAmmo = *ammo; // +(*akimboammo != -1 ? *akimboammo : 0);
			float ammoLeft = maxClip ? totalAmmo * 100 / maxClip : 0;
			float alpha = (**colorAmmo)[3];

			if (ammoLeft <= 30.f)
			{
				*colorAmmo = &colorRed;
			}
			else if (ammoLeft <= 40.f)
			{
				*colorAmmo = &colorOrange;
			}
			else if (ammoLeft <= 50.f)
			{
				*colorAmmo = &colorYellow;
			}

			(**colorAmmo)[3] = alpha;
		}
	}
	/*else
	{
		float maxAmmo = 0;

		// some weapons don't draw ammo clip count text
		*ammo = ps->ammoclip[BG_FindClipForWeapon(weap)] + cg.snap->ps.ammo[BG_FindAmmoForWeapon(weap)];

		maxAmmo = BG_MaxAmmoForWeapon(weap);

		if (colorAmmo)
		{
			float totalAmmo = *ammo + (*akimboammo != -1 ? *akimboammo : 0);
			float ammoLeft = maxAmmo ? totalAmmo * 100 / maxAmmo : 0;
			float alpha = (**colorAmmo)[3];

			if (ammoLeft <= 30.f)
			{
				*colorAmmo = &colorRed;
			}
			else if (ammoLeft <= 40.f)
			{
				*colorAmmo = &colorOrange;
			}
			else if (ammoLeft <= 50.f)
			{
				*colorAmmo = &colorYellow;
			}

			(**colorAmmo)[3] = alpha;
		}
	}
	*/
}

/**
* @brief CG_GetPlayerCurrentWeapon
* @param[in] player
*/
static int CG_GetPlayerCurrentWeapon(clientInfo_t *player)
{
	int curWeap;

	curWeap = cg_entities[player->clientNum].currentState.weapon;

	return curWeap;
}

/**
* @brief CG_DrawShoutcastPlayerOverlayAxis
* @param[in] player
* @param[in] x
* @param[in] y
* @param[in] index
*/
static void CG_DrawShoutcastPlayerOverlayAxis(clientInfo_t *player, float x, float y, int index)
{
	int    curWeap, weapScale, textWidth, textHeight;
	float  fraction;
	float  topRowX    = x;
	float  bottomRowX = x;
	char   *text;
	char   name[MAX_NAME_LENGTH + 2] = { 0 };
	vec4_t hcolor, borderColor;

	if (player->health > 0)
	{
		Vector4Copy(colorLtGrey, borderColor);
	}
	else
	{
		Vector4Copy(bg, borderColor);
	}

	// draw box
	CG_FillRect(x, y, PLAYER_LIST_WIDTH, PLAYER_LIST_HEIGHT, bg);
	CG_FillRect(x, y, PLAYER_LIST_STATUS_WIDTH, PLAYER_LIST_STATUS_HEIGHT, colorAxis);
	CG_DrawRect_FixedBorder(x, y, PLAYER_LIST_STATUS_WIDTH, PLAYER_LIST_STATUS_HEIGHT, 2, borderColor);
	CG_DrawRect_FixedBorder(x + PLAYER_LIST_STATUS_WIDTH - 0.75f, y, PLAYER_LIST_WIDTH - PLAYER_LIST_STATUS_WIDTH + 0.5f, PLAYER_LIST_HEIGHT / 2, 2, borderColor);
	CG_DrawRect_FixedBorder(x, y, PLAYER_LIST_WIDTH, PLAYER_LIST_HEIGHT, 2, cg.snap->ps.clientNum == player->clientNum ? colorYellow : borderColor);

	// draw HP bar
	fraction = (float)CG_GetPlayerMaxHealthFrac(player->clientNum, (float)player->health, cg_entities[player->clientNum].currentState.teamNum, player->team);
	CG_FilledBar2(topRowX + PLAYER_LIST_STATUS_WIDTH, y + 1, PLAYER_LIST_WIDTH - PLAYER_LIST_STATUS_WIDTH - 1, PLAYER_LIST_HEIGHT / 2 - 1.75f, colorAxis, colorAxis,
                 bg, bg, fraction, BAR_BGSPACING_X0Y0, -1);

	// draw health
	if (player->health > 0)
	{
		CG_GetColorForHealth(player->health, hcolor);

		text      = va("%i", player->health);
		textWidth = CG_Text_Width_Ext(text, 0.27f, 0, FONT_TEXT);
		CG_Text_Paint_Ext(topRowX + (PLAYER_LIST_STATUS_WIDTH / 2) - (textWidth / 2) - 0.5f - 6, y + (PLAYER_LIST_HEIGHT / 2) + 4, 0.27f, 0.27f, hcolor, text, 0, 0, ITEM_TEXTSTYLE_NORMAL, FONT_TEXT);
	}
	else if (player->health == 0)
	{
		CG_DrawPic(topRowX + (PLAYER_LIST_STATUS_WIDTH / 2) - 10, y + (PLAYER_LIST_HEIGHT / 2) - 10, 20, 20, cgs.media.medicIcon);
	}
	else if (player->health < 0)
	{
		CG_DrawPic(topRowX + (PLAYER_LIST_STATUS_WIDTH / 2) - 10, y + (PLAYER_LIST_HEIGHT / 2) - 10, 20, 20, cgs.media.scoreEliminatedShader);
	}

	// draw name limit 20 chars
	Q_ColorizeString(player->health < 0 ? '9' : '7', player->cleanname, name, MAX_NAME_LENGTH + 2);
	textHeight = CG_Text_Height_Ext(name, 0.16f, 0, FONT_TEXT);
	CG_Text_Paint(x + PLAYER_LIST_STATUS_WIDTH + 7, y + (PLAYER_LIST_HEIGHT / 4) + (textHeight / 2), 0.16f, colorWhite, name, 0, 20, ITEM_TEXTSTYLE_NORMAL);

	// draw follow bind
	if (player->health < 0)
	{
		Vector4Copy(colorMdGrey, hcolor);
	}
	else
	{
		Vector4Copy(colorWhite, hcolor);
	}

	//text      = va("(F%i)", index + 1);
	//textWidth = CG_Text_Width_Ext(text, 0.12f, 0, FONT_TEXT);
	//CG_Text_Paint(x + PLAYER_LIST_WIDTH - textWidth - 5, y + (PLAYER_LIST_HEIGHT / 4) + 2.0f, 0.12f, hcolor, text, 0, 0, ITEM_TEXTSTYLE_NORMAL);
	
	// draw class
	CG_DrawPic(bottomRowX + PLAYER_LIST_STATUS_WIDTH + 4, y + (PLAYER_LIST_HEIGHT * 0.75f) - 6, 12, 12, cgs.media.classPics[cg_entities[player->clientNum].currentState.teamNum]);
	bottomRowX += PLAYER_LIST_STATUS_WIDTH + 16;

	if (cg_entities[player->clientNum].currentState.teamNum != player->latchedClass)
	{
		// arrow latched class
		CG_DrawPic(bottomRowX, y + (PLAYER_LIST_HEIGHT * 0.75f) - 6, 12, 12, trap_R_RegisterShaderNoMip("icons/icon_arrow.tga"));
		// latched class
		CG_DrawPic(bottomRowX + 9, y + (PLAYER_LIST_HEIGHT * 0.75f) - 6, 12, 12, cgs.media.classPics[player->latchedClass]);
	}

	// draw powerups
	bottomRowX = x + PLAYER_LIST_WIDTH;
	if (player->powerups & ((1 << PW_REDFLAG) | (1 << PW_BLUEFLAG)))
	{
		CG_DrawPic(bottomRowX - 14, y + (PLAYER_LIST_HEIGHT * 0.75f) - 11, 20, 20, cgs.media.treasureIcon);
		//CG_DrawPic(topRowX - (PLAYER_LIST_STATUS_WIDTH / 2) + 10, y + (PLAYER_LIST_HEIGHT * 0.75f) - 20, 32, 32, cgs.media.treasureIcon);
		bottomRowX -= 14;
	}

	if (player->powerups & (1 << PW_INVULNERABLE))
	{
		CG_DrawPic(bottomRowX - 14, y + (PLAYER_LIST_HEIGHT * 0.75f) -7, 12, 12, cgs.media.spawnInvincibleShader);
	}

	// draw weapon icon
	curWeap    = CG_GetPlayerCurrentWeapon(player);
	weapScale  = cg_weapons[curWeap].weaponIconScale * 15;
	bottomRowX = x + PLAYER_LIST_WIDTH - 63;

	if (IS_VALID_WEAPON(curWeap) && cg_weapons[curWeap].weaponIconScale == 1)
	{
		bottomRowX += weapScale;
	}

	// note: WP_NONE is excluded
	if (IS_VALID_WEAPON(curWeap) && cg_weapons[curWeap].weaponIcon[2])
	{
		CG_DrawPic(bottomRowX, y + (PLAYER_LIST_HEIGHT * 0.75f) - 8, weapScale, weapScale, cg_weapons[curWeap].weaponIcon[2]);
	}
}

/**
* @brief CG_DrawShoutcastPlayerOverlayAllies
* @param[in] player
* @param[in] x
* @param[in] y
* @param[in] index
*/
static void CG_DrawShoutcastPlayerOverlayAllies(clientInfo_t *player, float x, float y, int index)
{
	int    curWeap, weapScale, textWidth, textHeight;
	float  fraction;
	float  topRowX    = x;
	float  bottomRowX = x + PLAYER_LIST_WIDTH - PLAYER_LIST_STATUS_WIDTH;
	char   *text;
	char   name[MAX_NAME_LENGTH + 2] = { 0 };
	vec4_t hcolor, borderColor;

	if (player->health > 0)
	{
		Vector4Copy(colorLtGrey, borderColor);
	}
	else
	{
		Vector4Copy(bg, borderColor);
	}

	// draw box
	CG_FillRect(x + 0.75f, y, PLAYER_LIST_WIDTH - 1, PLAYER_LIST_HEIGHT, bg);
	CG_FillRect(x + PLAYER_LIST_WIDTH - PLAYER_LIST_STATUS_WIDTH, y, PLAYER_LIST_STATUS_WIDTH, PLAYER_LIST_STATUS_HEIGHT, colorAllies);
	CG_DrawRect_FixedBorder(x + PLAYER_LIST_WIDTH - PLAYER_LIST_STATUS_WIDTH, y, PLAYER_LIST_STATUS_WIDTH, PLAYER_LIST_STATUS_HEIGHT, 2, borderColor);
	CG_DrawRect_FixedBorder(x, y, PLAYER_LIST_WIDTH - PLAYER_LIST_STATUS_WIDTH + 0.75f, PLAYER_LIST_HEIGHT / 2, 2, borderColor);
	CG_DrawRect_FixedBorder(x, y, PLAYER_LIST_WIDTH, PLAYER_LIST_HEIGHT, 2, cg.snap->ps.clientNum == player->clientNum ? colorYellow : borderColor);

	// draw HP bar
	fraction = (float)CG_GetPlayerMaxHealthFrac(player->clientNum, (float)player->health, cg_entities[player->clientNum].currentState.teamNum, player->team);
	CG_FilledBar2(topRowX + 1, y + 1, PLAYER_LIST_WIDTH - PLAYER_LIST_STATUS_WIDTH - 1, PLAYER_LIST_HEIGHT / 2 - 1.5f, colorAllies, colorAllies,
                 bg, bg, fraction, BAR_BGSPACING_X0Y0 | BAR_LEFT, -1);

	topRowX += PLAYER_LIST_WIDTH;

	// draw health
	if (player->health > 0)
	{
		CG_GetColorForHealth(player->health, hcolor);

		text      = va("%i", player->health);
		textWidth = CG_Text_Width_Ext(text, 0.27f, 0, FONT_TEXT);
		CG_Text_Paint_Ext(topRowX - (PLAYER_LIST_STATUS_WIDTH / 2) - (textWidth / 2) - 0.5f - 6, y + (PLAYER_LIST_HEIGHT / 2) + 4, 0.27f, 0.27f, hcolor, text, 0, 0, ITEM_TEXTSTYLE_NORMAL, FONT_TEXT);
	}
	else if (player->health == 0)
	{
		CG_DrawPic(topRowX - (PLAYER_LIST_STATUS_WIDTH / 2) - 10, y + (PLAYER_LIST_HEIGHT / 2) - 10, 20, 20, cgs.media.medicIcon);
	}
	else if (player->health < 0)
	{
		CG_DrawPic(topRowX - (PLAYER_LIST_STATUS_WIDTH / 2) - 10, y + (PLAYER_LIST_HEIGHT / 2) - 10, 20, 20, cgs.media.scoreEliminatedShader);
	}

	// draw name limit 20 chars
	Q_ColorizeString(player->health < 0 ? '9' : '7', player->cleanname, name, MAX_NAME_LENGTH + 2);
	textWidth  = CG_Text_Width_Ext(name, 0.16f, 0, FONT_TEXT);
	textHeight = CG_Text_Height_Ext(name, 0.16f, 0, FONT_TEXT);
	if (textWidth > 116)
	{
		textWidth = 116;
	}
	CG_Text_Paint(x + PLAYER_LIST_WIDTH - textWidth - 36, y + (PLAYER_LIST_HEIGHT / 4) + (textHeight / 2), 0.16f, colorWhite, name, 0, 20, ITEM_TEXTSTYLE_NORMAL);

	// draw follow bind
	if (player->health < 0)
	{
		Vector4Copy(colorMdGrey, hcolor);
	}
	else
	{
		Vector4Copy(colorWhite, hcolor);
	}

	//text = va("(F%i)", index + 7);
	//CG_Text_Paint(x + 5, y + (PLAYER_LIST_HEIGHT / 4) + 2.0f, 0.12f, hcolor, text, 0, 0, ITEM_TEXTSTYLE_NORMAL);

	// draw class
	CG_DrawPic(bottomRowX - 16, y + (PLAYER_LIST_HEIGHT * 0.75f) - 6, 12, 12, cgs.media.classPics[cg_entities[player->clientNum].currentState.teamNum]);
	bottomRowX -= 16;

	if (cg_entities[player->clientNum].currentState.teamNum != player->latchedClass)
	{
		// arrow latched class
		CG_DrawPic(bottomRowX - 10, y + (PLAYER_LIST_HEIGHT * 0.75f) - 6, 12, 12, trap_R_RegisterShaderNoMip("icons/icon_arrow_left.tga"));
		// latched class
		CG_DrawPic(bottomRowX - 19, y + (PLAYER_LIST_HEIGHT * 0.75f) - 6, 12, 12, cgs.media.classPics[player->latchedClass]);
	}

	// draw powerups
	bottomRowX = x;
	if (player->powerups & ((1 << PW_REDFLAG) | (1 << PW_BLUEFLAG)))
	{
		CG_DrawPic(bottomRowX + 2, y + (PLAYER_LIST_HEIGHT * 0.75f) - 11, 20, 20, cgs.media.treasureIcon);
		//CG_DrawPic(topRowX - (PLAYER_LIST_STATUS_WIDTH / 2) - 15, y + (PLAYER_LIST_HEIGHT * 0.75f) - 20, 32, 32, cgs.media.treasureIcon);
		bottomRowX += 14;
	}

	if (player->powerups & (1 << PW_INVULNERABLE))
	{
		CG_DrawPic(bottomRowX + 2, y + (PLAYER_LIST_HEIGHT * 0.75f) - 6.5f, 12, 12, cgs.media.spawnInvincibleShader);
	}

	// draw weapon icon
	curWeap   = CG_GetPlayerCurrentWeapon(player);
	weapScale = cg_weapons[curWeap].weaponIconScale * 15;

	// note: WP_NONE is excluded
	if (IS_VALID_WEAPON(curWeap) && cg_weapons[curWeap].weaponIcon[2])
	{
		CG_DrawPic(x + 43, y + (PLAYER_LIST_HEIGHT * 0.75f) - 8, weapScale, weapScale, cg_weapons[curWeap].weaponIcon[2]);
	}
}

/**
* @brief CG_DrawShoutcastOverlay
*/
void CG_DrawShoutcastPlayerList(void)
{
	clientInfo_t *ci;
	int          axis   = 0;
	int          allies = 0;
	int          i;

	if (cgs.topshots.show == SHOW_ON || cg.showCLgameStats)
	{
		return;
	}

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		ci = &cgs.clientinfo[i];

		if (!ci->infoValid)
		{
			continue;
		}

		if (ci->team == TEAM_SPECTATOR)
		{
			continue;
		}

		if (ci->team == TEAM_RED && axis < MAX_PLAYERS)
		{
			CG_DrawShoutcastPlayerOverlayAxis(ci, PLAYER_LIST_X, PLAYER_LIST_Y + (PLAYER_LIST_HEIGHT * axis) + (1 * axis), axis);
			players[axis] = ci->clientNum;
			axis++;
		}

		if (ci->team == TEAM_BLUE && allies < MAX_PLAYERS)
		{
			CG_DrawShoutcastPlayerOverlayAllies(ci, SCREEN_WIDTH - PLAYER_LIST_WIDTH - PLAYER_LIST_X, PLAYER_LIST_Y + (PLAYER_LIST_HEIGHT * allies) + (1 * allies), allies);
			players[allies + 6] = ci->clientNum;
			allies++;
		}

		if (axis >= MAX_PLAYERS && allies >= MAX_PLAYERS)
		{
			break;
		}
	}
}

/**
* @brief CG_DrawShoutcastPlayerChargebar
* @param[in] x
* @param[in] y
* @param[in] width
* @param[in] height
* @param[in] flags
*/
static void CG_DrawShoutcastPlayerChargebar(float x, float y, int width, int height, int flags)
{
	float    barFrac, chargeTime;
	qboolean charge = qtrue;
	vec4_t   color;

	chargeTime = 20.0f;
	switch (cg.snap->ps.stats[STAT_PLAYER_CLASS])
	{
	case PC_ENGINEER:
		chargeTime = cg_engineerChargeTime.integer;
		break;
	case PC_MEDIC:
		chargeTime = cg_medicChargeTime.integer;
		break;
	case PC_LT:
		chargeTime = cg_LTChargeTime.integer;
		break;
	default:
		chargeTime = cg_soldierChargeTime.integer;
		break;
	}

	// display colored charge bar if charge bar isn't full enough

	if (chargeTime < 0)
	{
		return;
	}

	barFrac = (cg.time - cg.snap->ps.classWeaponTime) / chargeTime;

	if (barFrac > 1.0f)
	{
		barFrac = 1.0f;
	}

	if (!charge)
	{
		color[0] = 1.0f;
		color[1] = color[2] = 0.1f;
		color[3] = 0.5f;
	}
	else
	{
		color[0] = color[1] = 1.0f;
		color[2] = barFrac;
		color[3] = 0.25f + barFrac * 0.5f;
	}

	CG_FilledBar2(x, y, width, height, color, NULL, bg, bg, barFrac, flags, -1);
}

/**
* @brief CG_DrawShoutcasterStaminaBar
* @param[in] x
* @param[in] y
* @param[in] width
* @param[in] height
* @param[in] flags
*/
static void CG_DrawShoutcastPlayerStaminaBar(float x, float y, int width, int height, int flags)
{
	vec4_t colour = { 0.1f, 1.0f, 0.1f, 0.5f };
	vec_t  *color = colour;
	float  frac   = cg.snap->ps.sprintTime / (float)SPRINTTIME;

	color[0] = 1.0f - frac;
	color[1] = frac;

	CG_FilledBar2(x, y, width, height, color, NULL, bg, bg, frac, flags, -1);
}

/**
* @brief CG_RequestPlayerStats
* @param[in] clientNum
*/
void CG_RequestPlayerStats(int clientNum)
{
	if (cgs.gamestats.requestTime < cg.time)
	{
		cgs.gamestats.requestTime = cg.time + 1000;
		trap_SendClientCommand(va("gamestats %d", clientNum));
	}
}

/**
* @brief CG_DrawShoutcastPlayerStatus
*/
void CG_DrawShoutcastPlayerStatus(void)
{
	gameStats_t *gs     = &cgs.gamestats;
	clientInfo_t  *player = &cgs.clientinfo[cg.snap->ps.clientNum];
	playerState_t *ps     = &cg.snap->ps;
	vec4_t        hcolor;
	float         nameBoxWidth = PLAYER_STATUS_NAMEBOX_WIDTH;
	float         nameBoxHeight = PLAYER_STATUS_NAMEBOX_HEIGHT;
	float         nameBoxX = PLAYER_STATUS_NAMEBOX_X;
	float         nameBoxY = PLAYER_STATUS_NAMEBOX_Y;
	float         statsBoxWidth = PLAYER_STATUS_STATSBOX_WIDTH;
	float         statsBoxHeight = PLAYER_STATUS_STATSBOX_HEIGHT;
	float         statsBoxX = PLAYER_STATUS_STATSBOX_X;
	float         statsBoxY = PLAYER_STATUS_STATSBOX_Y;
	float         textWidth, textWidth2, textHeight;
	char		  *kills, *deaths, *suicides, *gibs, *dmgGiven, *dmgRcvd, *text, *revives, *health_given, *ammo_given;
	int           ammo, clip, akimbo, curWeap, weapScale, tmpX, weaponX = 0;
	char          name[MAX_NAME_LENGTH + 2] = { 0 };

	if (cgs.topshots.show == SHOW_ON || cg.showCLgameStats)
	{
		return;
	}

	// draw name box
	CG_FillRect(nameBoxX, nameBoxY, nameBoxWidth, nameBoxHeight, bg);
	CG_DrawRect_FixedBorder(nameBoxX, nameBoxY, nameBoxWidth, nameBoxHeight, 2, colorLtGrey);

	// draw team flag
	if (player->team == TEAM_BLUE)
	{
		CG_DrawPic(nameBoxX + 2, nameBoxY + (nameBoxHeight / 2) - 6, 18, 10, trap_R_RegisterShaderNoMip("ui_mp/assets/usa_flag.tga"));
	}
	else if (player->team == TEAM_RED)
	{
		CG_DrawPic(nameBoxX + 4, nameBoxY + (nameBoxHeight / 2) - 6, 18, 10, trap_R_RegisterShaderNoMip("ui_mp/assets/ger_flag.tga"));
	}

	// draw name limit 20 chars, width 110
	Q_ColorizeString('7', player->cleanname, name, MAX_NAME_LENGTH + 2);
	textWidth  = CG_Text_Width_Ext(name, 0.19f, 0, FONT_TEXT);
	textHeight = CG_Text_Height_Ext(name, 0.19f, 0, FONT_TEXT);
	if (textWidth > 110)
	{
		textWidth = 110;
	}
	CG_Text_Paint(nameBoxX + (nameBoxWidth / 2) - (textWidth / 2), nameBoxY + (nameBoxHeight / 2) + (textHeight / 2), 0.19f, colorWhite, name, 0, 20, ITEM_TEXTSTYLE_NORMAL);

	// draw country flag
	WM_SE_DrawFlags(nameBoxX + nameBoxWidth - 18, nameBoxY + (nameBoxHeight / 2) + 3, 1, player->clientNum);

	// draw stats box
	CG_FillRect(statsBoxX, statsBoxY, statsBoxWidth, statsBoxHeight, bg);
	CG_DrawRect_FixedBorder(statsBoxX, statsBoxY, statsBoxWidth, statsBoxHeight, 2, colorLtGrey);

	// draw powerups
	tmpX = statsBoxX + statsBoxWidth;
	if (ps->powerups[PW_REDFLAG] || ps->powerups[PW_BLUEFLAG])
	{
		CG_DrawPic(tmpX - 3, statsBoxY, POWERUPS_WIDTH, POWERUPS_HEIGHT, cgs.media.treasureIcon);
		tmpX += 23;
	}

	if (ps->powerups[PW_INVULNERABLE])
	{
		CG_DrawPic(tmpX - 3, statsBoxY, POWERUPS_WIDTH, POWERUPS_HEIGHT, cgs.media.spawnInvincibleShader);
	}

	CG_DrawShoutcastPlayerChargebar(statsBoxX, statsBoxY + statsBoxHeight, statsBoxWidth / 2, 6, BAR_BG | BAR_BGSPACING_X0Y0 | BAR_LEFT);
	CG_DrawShoutcastPlayerStaminaBar(statsBoxX + (statsBoxWidth / 2), statsBoxY + statsBoxHeight, statsBoxWidth / 2, 6, BAR_BG | BAR_BGSPACING_X0Y0);

	// draw ammo count
	CG_PlayerAmmoValue(&ammo, &clip, &akimbo, NULL);

	if (ammo > 0 || clip > 0)
	{
		if (clip == -1)
		{
			text = va("%i", ammo);
		}
		else
		{
			text = va("%i/%i", ammo, clip);
		}

		textWidth = CG_Text_Width_Ext(text, 0.19f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + statsBoxWidth - (textWidth / 2) - 25, statsBoxY + (statsBoxHeight / 2) + 2, 0.19f, colorWhite, text, 0, 0, ITEM_TEXTSTYLE_NORMAL);
	}

	// draw weapon icon
	curWeap   = CG_GetPlayerCurrentWeapon(player);
	weapScale = cg_weapons[curWeap].weaponIconScale * 16;
	tmpX      = statsBoxX + statsBoxWidth - 60;

	//if (IS_VALID_WEAPON(curWeap) && cg_weapons[curWeap].weaponIconScale == 1)
	//{
	//	tmpX += weapScale;
	//}

	// note: WP_NONE is excluded
	if (IS_VALID_WEAPON(curWeap) && cg_weapons[curWeap].weaponIcon[2])
	{
		CG_DrawPic(tmpX, statsBoxY + (statsBoxHeight / 2) - 8, weapScale, weapScale, cg_weapons[curWeap].weaponIcon[2]);
		weaponX = tmpX;
	}

	// draw hp
	if (cg.snap->ps.stats[STAT_HEALTH] > 0)
	{
		CG_GetColorForHealth(cg.snap->ps.stats[STAT_HEALTH], hcolor);
		text       = va("%i", cg.snap->ps.stats[STAT_HEALTH]);
		textWidth  = CG_Text_Width_Ext(text, 0.19f, 0, FONT_TEXT);
		textHeight = CG_Text_Height_Ext(text, 0.19f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX - (textWidth / 2) + 12, statsBoxY + (statsBoxHeight / 2) + (textHeight / 2), 0.19f, hcolor, text, 0, 0, ITEM_TEXTSTYLE_NORMAL);
	}
	else if (cg.snap->ps.stats[STAT_HEALTH] <= 0 && (cg.snap->ps.pm_flags & PMF_LIMBO))
	{
		CG_DrawPic(statsBoxX + 5, statsBoxY + (statsBoxHeight / 2) - 6, 12, 12, cgs.media.scoreEliminatedShader);
	}
	else
	{
		CG_DrawPic(statsBoxX + 5, statsBoxY + (statsBoxHeight / 2) - 6, 12, 12, cgs.media.medicIcon);
	}

	statsBoxX += 18;

	// draw class
	CG_DrawPic(statsBoxX + 8, statsBoxY + (statsBoxHeight / 2) - 6, 12, 12, cgs.media.classPics[cg_entities[player->clientNum].currentState.teamNum]);
	statsBoxX += 13;

	if (cg_entities[player->clientNum].currentState.teamNum != player->latchedClass)
	{
		// arrow latched class
		CG_DrawPic(statsBoxX + 5, statsBoxY + (statsBoxHeight / 2) - 6, 12, 12, trap_R_RegisterShaderNoMip("icons/icon_arrow.tga"));
		// latched class
		CG_DrawPic(statsBoxX + 14, statsBoxY + (statsBoxHeight / 2) - 6, 12, 12, cgs.media.classPics[player->latchedClass]);
	}

	CG_RequestPlayerStats(cg.snap->ps.clientNum);

	if (gs->nClientID == player->clientNum && gs->fHasStats)
	{
		statsBoxX = PLAYER_STATUS_STATSBOX_X + 55;

		dmgGiven		= va("%d", gs->damage_giv);
		dmgRcvd			= va("%d", gs->damage_rec);
		kills			= va("%d", gs->kills);
		deaths			= va("%d", gs->deaths);
		suicides		= va("%d", gs->suicides);
		gibs			= va("%d", gs->gibs);
		revives			= va("%d", gs->revives);
		health_given	= va("%d", gs->health_given);
		ammo_given		= va("%d", gs->ammo_given);

		// kills
		textWidth  = CG_Text_Width_Ext("K", 0.16f, 0, FONT_TEXT);
		textHeight = CG_Text_Height_Ext("K", 0.16f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 6, statsBoxY + (statsBoxHeight / 2) - (textHeight / 2), 0.16f, colorMdGrey, "K", 0, 0, ITEM_TEXTSTYLE_NORMAL);

		textHeight = CG_Text_Height_Ext(kills, 0.19f, 0, FONT_TEXT);
		textWidth2 = CG_Text_Width_Ext(kills, 0.19f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 6 + (textWidth / 2) - (textWidth2 / 2), statsBoxY + (statsBoxHeight / 2) + (textHeight / 2) + 4, 0.19f, colorWhite, kills, 0, 0, ITEM_TEXTSTYLE_NORMAL);
		statsBoxX += 6 + textWidth2;

		// deaths
		textWidth  = CG_Text_Width_Ext("D", 0.16f, 0, FONT_TEXT);
		textHeight = CG_Text_Height_Ext("D", 0.16f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 6, statsBoxY + (statsBoxHeight / 2) - (textHeight / 2), 0.16f, colorMdGrey, "D", 0, 0, ITEM_TEXTSTYLE_NORMAL);

		textHeight = CG_Text_Height_Ext(deaths, 0.19f, 0, FONT_TEXT);
		textWidth2 = CG_Text_Width_Ext(deaths, 0.19f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 6 + (textWidth / 2) - (textWidth2 / 2), statsBoxY + (statsBoxHeight / 2) + (textHeight / 2) + 4, 0.19f, colorWhite, deaths, 0, 0, ITEM_TEXTSTYLE_NORMAL);
		statsBoxX += 6 + textWidth2;

		// suicides
		textWidth  = CG_Text_Width_Ext("SK", 0.16f, 0, FONT_TEXT);
		textHeight = CG_Text_Height_Ext("SK", 0.16f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 6, statsBoxY + (statsBoxHeight / 2) - (textHeight / 2), 0.16f, colorMdGrey, "SK", 0, 0, ITEM_TEXTSTYLE_NORMAL);

		textHeight = CG_Text_Height_Ext(suicides, 0.19f, 0, FONT_TEXT);
		textWidth2 = CG_Text_Width_Ext(suicides, 0.19f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 6 + (textWidth / 2) - (textWidth2 / 2), statsBoxY + (statsBoxHeight / 2) + (textHeight / 2) + 4, 0.19f, colorWhite, suicides, 0, 0, ITEM_TEXTSTYLE_NORMAL);
		statsBoxX += 6 + textWidth2;

		// gibs
		textWidth = CG_Text_Width_Ext("G", 0.16f, 0, FONT_TEXT);
		textHeight = CG_Text_Height_Ext("G", 0.16f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 10, statsBoxY + (statsBoxHeight / 2) - (textHeight / 2), 0.16f, colorMdGrey, "G", 0, 0, ITEM_TEXTSTYLE_NORMAL);

		textHeight = CG_Text_Height_Ext(gibs, 0.19f, 0, FONT_TEXT);
		textWidth2 = CG_Text_Width_Ext(gibs, 0.19f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 10 + (textWidth / 2) - (textWidth2 / 2), statsBoxY + (statsBoxHeight / 2) + (textHeight / 2) + 4, 0.19f, colorWhite, gibs, 0, 0, ITEM_TEXTSTYLE_NORMAL);
		statsBoxX += 10 + textWidth2;

		// dmgGiven
		textWidth  = CG_Text_Width_Ext("DG", 0.16f, 0, FONT_TEXT);
		textHeight = CG_Text_Height_Ext("DG", 0.16f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 10, statsBoxY + (statsBoxHeight / 2) - (textHeight / 2), 0.16f, colorMdGrey, "DG", 0, 0, ITEM_TEXTSTYLE_NORMAL);

		textHeight = CG_Text_Height_Ext(dmgGiven, 0.19f, 0, FONT_TEXT);
		textWidth2 = CG_Text_Width_Ext(dmgGiven, 0.19f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 10 + (textWidth / 2) - (textWidth2 / 2), statsBoxY + (statsBoxHeight / 2) + (textHeight / 2) + 4, 0.19f, colorMdGreen, dmgGiven, 0, 0, ITEM_TEXTSTYLE_NORMAL);
		statsBoxX += 10 + textWidth2;

		// dmgRcvd
		textWidth  = CG_Text_Width_Ext("DR", 0.16f, 0, FONT_TEXT);
		textHeight = CG_Text_Height_Ext("DR", 0.16f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 10, statsBoxY + (statsBoxHeight / 2) - (textHeight / 2), 0.16f, colorMdGrey, "DR", 0, 0, ITEM_TEXTSTYLE_NORMAL);

		textHeight = CG_Text_Height_Ext(dmgRcvd, 0.19f, 0, FONT_TEXT);
		textWidth2 = CG_Text_Width_Ext(dmgRcvd, 0.19f, 0, FONT_TEXT);
		CG_Text_Paint(statsBoxX + 10 + (textWidth / 2) - (textWidth2 / 2), statsBoxY + (statsBoxHeight / 2) + (textHeight / 2) + 4, 0.19f, colorRed, dmgRcvd, 0, 0, ITEM_TEXTSTYLE_NORMAL);
		statsBoxX += 10 + textWidth2;

		if (cg_entities[player->clientNum].currentState.teamNum == 1) // Medic
		{
			if (weaponX > 0)
				statsBoxX = weaponX - 30;

			// revives
			textWidth = CG_Text_Width_Ext("R", 0.16f, 0, FONT_TEXT);
			textHeight = CG_Text_Height_Ext("R", 0.16f, 0, FONT_TEXT);
			CG_Text_Paint(statsBoxX, statsBoxY + (statsBoxHeight / 2) - (textHeight / 2), 0.16f, colorMdGrey, "R", 0, 0, ITEM_TEXTSTYLE_NORMAL);

			textHeight = CG_Text_Height_Ext(revives, 0.19f, 0, FONT_TEXT);
			textWidth2 = CG_Text_Width_Ext(revives, 0.19f, 0, FONT_TEXT);
			CG_Text_Paint(statsBoxX + (textWidth / 2) - (textWidth2 / 2), statsBoxY + (statsBoxHeight / 2) + (textHeight / 2) + 4, 0.19f, colorWhite, revives, 0, 0, ITEM_TEXTSTYLE_NORMAL);
			statsBoxX += 10 + textWidth2;

			// health_given
			textWidth = CG_Text_Width_Ext("H", 0.16f, 0, FONT_TEXT);
			textHeight = CG_Text_Height_Ext("H", 0.16f, 0, FONT_TEXT);
			CG_Text_Paint(statsBoxX + 4, statsBoxY + (statsBoxHeight / 2) - (textHeight / 2), 0.16f, colorMdGrey, "H", 0, 0, ITEM_TEXTSTYLE_NORMAL);

			textHeight = CG_Text_Height_Ext(health_given, 0.19f, 0, FONT_TEXT);
			textWidth2 = CG_Text_Width_Ext(health_given, 0.19f, 0, FONT_TEXT);
			CG_Text_Paint(statsBoxX + 4 + (textWidth / 2) - (textWidth2 / 2), statsBoxY + (statsBoxHeight / 2) + (textHeight / 2) + 4, 0.19f, colorWhite, health_given, 0, 0, ITEM_TEXTSTYLE_NORMAL);
			statsBoxX += textWidth2 + 4;
		}

		if (cg_entities[player->clientNum].currentState.teamNum == 3) // LT
		{
			if (weaponX > 0)
				statsBoxX = weaponX - 30;

			// ammo_given
			textWidth = CG_Text_Width_Ext("A", 0.16f, 0, FONT_TEXT);
			textHeight = CG_Text_Height_Ext("A", 0.16f, 0, FONT_TEXT);
			CG_Text_Paint(statsBoxX, statsBoxY + (statsBoxHeight / 2) - (textHeight / 2), 0.16f, colorMdGrey, "A", 0, 0, ITEM_TEXTSTYLE_NORMAL);

			textHeight = CG_Text_Height_Ext(ammo_given, 0.19f, 0, FONT_TEXT);
			textWidth2 = CG_Text_Width_Ext(ammo_given, 0.19f, 0, FONT_TEXT);
			CG_Text_Paint(statsBoxX + (textWidth / 2) - (textWidth2 / 2), statsBoxY + (statsBoxHeight / 2) + (textHeight / 2) + 4, 0.19f, colorWhite, ammo_given, 0, 0, ITEM_TEXTSTYLE_NORMAL);
			statsBoxX += 10 + textWidth2;
		}
	}
}

/**
* @brief CG_DrawShoutcastTeamNames
*/
static void CG_DrawShoutcastTeamNames()
{
	rectDef_t rect;
	int       textWidth;
	int       textHeight;
	char      *text;

	if (cg_shoutcastDrawTeamNames.integer)
	{
		// draw axis label
		if (Q_PrintStrlen(cg_shoutcastTeamNameRed.string) > 0)
		{
			text = va("%s", cg_shoutcastTeamNameRed.string);
		}
		else
		{
			text = va("%s", "Axis");
		}

		rect.x = GAMETIME_X - TEAMNAMES_WIDTH;
		rect.y = GAMETIME_Y;
		rect.w = TEAMNAMES_WIDTH;
		rect.h = TEAMNAMES_HEIGHT;
		GradientBar_Paint(&rect, colorAxis);

		// max width 174, limit 20 chars
		textWidth  = CG_Text_Width_Ext(text, 0.3f, 0, FONT_TEXT);
		textHeight = CG_Text_Height_Ext(text, 0.3f, 0, FONT_TEXT);

		if (textWidth > 174)
		{
			textWidth = 174;
		}

		CG_Text_Paint(rect.x + (rect.w / 2) - (textWidth / 2) + 1.35f, rect.y + (rect.h / 2) + (textHeight / 2) + 1.35f, 0.3f, colorBlack, text, 0, 20, ITEM_TEXTSTYLE_NORMAL);
		CG_Text_Paint(rect.x + (rect.w / 2) - (textWidth / 2), rect.y + (rect.h / 2) + (textHeight / 2), 0.3f, colorWhite, text, 0, 20, ITEM_TEXTSTYLE_NORMAL);

		// draw allies label
		if (Q_PrintStrlen(cg_shoutcastTeamNameBlue.string) > 0)
		{
			text = va("%s", cg_shoutcastTeamNameBlue.string);
		}
		else
		{
			text = va("%s", "Allies");
		}

		rect.x = GAMETIME_X + GAMETIME_WIDTH;
		rect.y = GAMETIME_Y;
		rect.w = TEAMNAMES_WIDTH;
		rect.h = TEAMNAMES_HEIGHT;
		GradientBar_Paint(&rect, colorAllies);

		// max width 174, limit 20 chars
		textWidth  = CG_Text_Width_Ext(text, 0.3f, 0, FONT_TEXT);
		textHeight = CG_Text_Height_Ext(text, 0.3f, 0, FONT_TEXT);

		if (textWidth > 174)
		{
			textWidth = 174;
		}

		CG_Text_Paint(rect.x + (rect.w / 2) - (textWidth / 2) + 1.35f, rect.y + (rect.h / 2) + (textHeight / 2) + 1.35f, 0.3f, colorBlack, text, 0, 20, ITEM_TEXTSTYLE_NORMAL);
		CG_Text_Paint(rect.x + (rect.w / 2) - (textWidth / 2), rect.y + (rect.h / 2) + (textHeight / 2), 0.3f, colorWhite, text, 0, 20, ITEM_TEXTSTYLE_NORMAL);
	}
}

int CG_CalculateShoutcasterReinfTime(team_t team)
{
	int dwDeployTime;

	dwDeployTime = (team == TEAM_RED) ? cg_redlimbotime.integer : cg_bluelimbotime.integer;
	return (int)(1 + (dwDeployTime - ((cgs.aReinfOffset[team] + cg.time - cgs.levelStartTime) % dwDeployTime)) * 0.001f);
}

/*
========================
RTCWPro
CG_DrawShoutcastTimer
========================
*/
void CG_DrawShoutcastTimer(void)
{
	//if (cgs.gamestats.show == SHOW_ON)
	//{
	//	return;
	//}
	
	vec4_t color = { .6f, .6f, .6f, 1.f };
	char* text, * rtAllies = "", * rtAxis = ""; // , * round;
	int    tens;
	int    msec    = (cgs.timelimit * 60000.f) - (cg.time - cgs.levelStartTime); // 60.f * 1000.f
	int    seconds = msec / 1000;
	int    mins    = seconds / 60;
	int    w       = GAMETIME_WIDTH;
	int    h       = GAMETIME_HEIGHT;
	int    x       = GAMETIME_X;
	int    y       = GAMETIME_Y;
	int    redScoreX = GAMETIME_X - 20;
	int    redScoreY = GAMETIME_Y + 20;
	int    blueScoreX = GAMETIME_X + GAMETIME_WIDTH + 5;
	int	   blueScoreY = GAMETIME_Y + 20;
	int    textWidth;

	seconds -= mins * 60;
	tens     = seconds / 10;
	seconds -= tens * 10;

	if (cgs.gamestate != GS_PLAYING)
	{
		text     = va("^7%s", CG_TranslateString("WARMUP")); // don't draw reinforcement time in warmup mode // ^*
		color[3] = fabs(sin(cg.time * 0.002));
	}
	else if (msec < 0 && cgs.timelimit > 0.0f)
	{
		text     = "^70:00";
		color[3] = fabs(sin(cg.time * 0.002));
	}
	else
	{
		int reinfTimeAx = CG_CalculateShoutcasterReinfTime(TEAM_RED);
		int reinfTimeAl = CG_CalculateShoutcasterReinfTime(TEAM_BLUE);

		rtAllies = va("^$%i", reinfTimeAl);
		rtAxis = va("^1%i", reinfTimeAx);

		if (cgs.timelimit <= 0.0f)
		{
			text = "";
		}
		else
		{
			text = va("^7%2i:%i%i", mins, tens, seconds);
		}

		color[3] = 1.f;
	}

	// draw scores
	CG_Text_Paint(redScoreX, redScoreY, 0.5f, colorWhite, cg_shoutcastRedScore.string, 0, 0, 0);
	CG_Text_Paint(blueScoreX, blueScoreY, 0.5f, colorWhite, cg_shoutcastBlueScore.string, 0, 0, 0);

	textWidth = CG_Text_Width_Ext(text, 0.23f, 0, FONT_HEADER);

	// draw box
	CG_FillRect(x, y, w, h, bg);
	CG_DrawRect_FixedBorder(x, y, w, h, 2, colorLtGrey);

	// game time
	CG_Text_Paint(x + w / 2 - textWidth / 2 - 2, y + 13, 0.23f, color, text, 0, 0, 0);

	// axis reinf time
	CG_Text_Paint(x + 3, y + h - 5, 0.20f, color, rtAxis, 0, 0, 0);

	// allies reinf time
	textWidth = CG_Text_Width_Ext(rtAllies, 0.20f, 0, FONT_HEADER);
	CG_Text_Paint(x + w - textWidth - 3, y + h - 5, 0.20f, color, rtAllies, 0, 0, 0);

	// round number
	if (cgs.gametype == GT_WOLF_STOPWATCH)
	{
		/*round     = va("%i/2", cgs.currentRound + 1);
		textWidth = CG_Text_Width_Ext(round, 0.15f, 0, FONT_HEADER);
		CG_Text_Paint(x + w / 2 - textWidth / 2, y + h - 5.5f, 0.15f, colorWhite, round, 0, 0, 0);*/

		if (cgs.currentRound == 0) {
			CG_DrawPic(x + w / 2 - 6, y + h - 16, 14, 14, trap_R_RegisterShader("sprites/stopwatch1.tga"));
		}
		else {
			CG_DrawPic(x + w / 2 - 6, y + h - 16, 14, 14, trap_R_RegisterShader("sprites/stopwatch2.tga"));
		}
	}
	CG_DrawShoutcastTeamNames();

	//return y += TINYCHAR_HEIGHT;
}

/**
* @brief CG_DrawShoutcastPowerups
*/
//void CG_DrawShoutcastPowerups(void)
//{
//	if (cg.flagIndicator & (1 << PW_REDFLAG))
//	{
//		if (cg.redFlagCounter > 0)
//		{
//			CG_DrawPic(POWERUPS_X, POWERUPS_Y, POWERUPS_WIDTH, POWERUPS_HEIGHT, cgs.media.objectiveTeamShader);
//		}
//		else
//		{
//			CG_DrawPic(POWERUPS_X, POWERUPS_Y, POWERUPS_WIDTH, POWERUPS_HEIGHT, cgs.media.objectiveDroppedShader);
//		}
//	}
//	else if (cg.flagIndicator & (1 << PW_BLUEFLAG))
//	{
//		if (cg.blueFlagCounter > 0)
//		{
//			CG_DrawPic(POWERUPS_X, POWERUPS_Y, POWERUPS_WIDTH, POWERUPS_HEIGHT, cgs.media.objectiveTeamShader);
//		}
//		else
//		{
//			CG_DrawPic(POWERUPS_X, POWERUPS_Y, POWERUPS_WIDTH, POWERUPS_HEIGHT, cgs.media.objectiveDroppedShader);
//		}
//	}
//}

/**
* @brief CG_ToggleShoutcasterMode
*        set event handling to CGAME_EVENT_SHOUTCAST so we can listen to keypresses
* @param[in] shoutcaster
*/
//void CG_ToggleShoutcasterMode(int shoutcaster)
//{
//	if (shoutcaster)
//	{
//		CG_EventHandling(CGAME_EVENT_SHOUTCAST, qfalse);
//	}
//	else
//	{
//		CG_EventHandling(CGAME_EVENT_NONE, qfalse);
//	}
//}

/**
* @brief CG_ShoutcastCheckKeyCatcher
*
* @details track the moment when key catcher is changed away from KEYCATCH_UI
*          so we can set back event handling to CGAME_EVENT_SHOUTCAST
*          and key catcher to KEYCATCH_CGAME for shoutcaster follow keybinds
*
* @param[in] keycatcher
*/
//void CG_ShoutcastCheckKeyCatcher(int keycatcher)
//{
//	// going out of ui menu
//	if (cgs.clientinfo[cg.clientNum].shoutStatus && cgs.eventHandling == CGAME_EVENT_NONE &&
//		cg.snap->ps.pm_type != PM_INTERMISSION && !(keycatcher & KEYCATCH_UI) && (cg.lastKeyCatcher & KEYCATCH_UI))
//	{
//		CG_ToggleShoutcasterMode(1);
//	}
//
//	// going out of limbo menu
//	if (cgs.clientinfo[cg.clientNum].shoutStatus && cgs.eventHandling == CGAME_EVENT_NONE && !(keycatcher & KEYCATCH_UI))
//	{
//		CG_ToggleShoutcasterMode(1);
//	}
//
//	// resolution changes don't automatically close the ui menus but show confirmation window after vid_restart
//	// so need to turn off shoutcast event handling otherwise mouse cursor will not work
//	if (cgs.clientinfo[cg.clientNum].shoutStatus && cgs.eventHandling == CGAME_EVENT_SHOUTCAST && (keycatcher & KEYCATCH_UI))
//	{
//		CG_ToggleShoutcasterMode(0);
//	}
//}

/**
* @brief CG_Shoutcast_KeyHandling
* @param[in] _key
* @param[in] down
*/
qboolean CG_Shoutcast_KeyHandling(int key, qboolean down)
{
	if (down)
	{
		return CG_ShoutcastCheckExecKey(key, qtrue);
	}

	return qfalse;
}

/**
* @brief CG_ShoutcastCheckExecKey
* @param[in] key
* @param[in] doaction
* @return
*/
qboolean CG_ShoutcastCheckExecKey(int key, qboolean doaction)
{
	if (key == K_ESCAPE)
	{
		return qtrue;
	}

	if ((key & K_CHAR_FLAG))
	{
		return qfalse;
	}

	key &= ~K_CHAR_FLAG;

	if (key >= K_F1 && key <= K_F12)
	{
		if (doaction)
		{
			trap_SendClientCommand(va("follow %d", players[key - K_F1]));
		}

		return qtrue;
	}

	return qfalse;
}
