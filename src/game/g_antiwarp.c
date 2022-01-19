/*
RTCWPro - antiwarp, original credit to zinx
Port: etPro > etPub > S4NDMOD > rtcwPubJ > OSPx
Warping occurs when the server receives several new commands from a player in a very short period of time.
This happens because packets from the player were lost or never sent.
The player will appear to cross a great distance in a very short period of time, making them hard to hit.
The antiwarp system delays commands when they are recieved too quickly.
Thus, when a player sends 700ms worth of commands in 50ms, the commands will be spread out over 700ms,
causing the player to move smoothly (unwarp them) to other players.
The net effect is that players with unreliable or congested upstream will not benefit from their situation;
rather they (in a sense) are penalized for it, while all the other players on the server are not.
*/

#include "g_local.h"

qboolean G_DoAntiwarp(gentity_t* ent) {

	// only antiwarp if requested
	if (!g_antiWarp.integer)
	{
		return qfalse;
	}

	//dont use for intermission to fix high ping on results screen
	if (level.intermissiontime)
	{
		return qfalse;
	}

	if (ent && ent->client)
	{
		// don't antiwarp spectators
		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
			ent->client->ps.pm_flags & PMF_LIMBO) {
			return qfalse;
		}

		// don't antiwarp during map load
		if (ent->client->pers.connected == CON_CONNECTING) {
			return qfalse;
		}

		// don't antiwarp if they haven't been connected for 5 seconds
		// note: this check is generally only triggered during mid-map
		// connects, because clients connect before loading the map.
		if ((level.time - ent->client->pers.connectTime) < 5000) {
			return qfalse;
		}
	}

	return qtrue;
}

void AW_AddUserCmd(int clientNum, usercmd_t* cmd) {

	gentity_t* ent = g_entities + clientNum;

	int idx = (ent->client->cmdhead + ent->client->cmdcount) % LAG_MAX_COMMANDS;
	ent->client->cmds[idx] = *cmd;

	if (ent->client->cmdcount < LAG_MAX_COMMANDS)
	{
		ent->client->cmdcount++;
	}
	else
	{
		ent->client->cmdhead = (ent->client->cmdhead + 1) % LAG_MAX_COMMANDS;
	}
}

// zinx - G_CmdScale is a hack :x
extern float pm_proneSpeedScale;
static float AW_CmdScale(gentity_t* ent, usercmd_t* cmd) {

	float scale;

#ifdef CGAMEDLL
	int gametype = cg_gameType.integer;
	int movespeed = cg_movespeed.integer;
#elif GAMEDLL
	int gametype = g_gametype.integer;
	int movespeed = g_speed.integer;
#endif

	scale = abs(cmd->forwardmove);
	if (abs(cmd->rightmove) > scale)
	{
		scale = abs(cmd->rightmove);
	}
	// zinx - don't count crouch/jump; just count moving in water
	if (ent->waterlevel && abs(cmd->upmove) > scale)
	{
		scale = abs(cmd->upmove);
	}

	scale /= 127.f;

	// JPW NERVE -- half move speed if heavy weapon is carried
	// this is the counterstrike way of doing it -- ie you can switch to a non-heavy weapon and move at
	// full speed.  not completely realistic (well, sure, you can run faster with the weapon strapped to your
	// back than in carry position) but more fun to play.  If it doesn't play well this way we'll bog down the
	// player if the own the weapon at all.
	//
#if 0   // zinx - not letting them go at sprint speed for now.
	if ((ent->client->ps.weapon == WP_PANZERFAUST) ||
		(ent->client->ps.weapon == WP_MOBILE_MG42) ||
		(ent->client->ps.weapon == WP_MOBILE_MG42_SET) ||
		(ent->client->ps.weapon == WP_MORTAR)) {
		if (ent->client->sess.skill[SK_HEAVY_WEAPONS] >= 3) {
			scale *= 0.75;
		}
		else {
			scale *= 0.5;
		}
	}

	if (ent->client->ps.weapon == WP_FLAMETHROWER) { // trying some different balance for the FT
		if (!(ent->client->sess.skill[SK_HEAVY_WEAPONS] >= 3) || cmd->buttons & BUTTON_ATTACK) {
			scale *= 0.7;
		}
	}
#endif

	/*if (gametype == GT_SINGLE_PLAYER)
	{
		// Adjust the movespeed
		scale *= (((float)movespeed) / (float)127);

	} // if (gametype == GT_SINGLE_PLAYER)...*/

#if 0   // zinx - not letting them go at sprint speed for now.
	if (ent->client->ps.eFlags & EF_PRONE) {
		scale *= pm_proneSpeedScale;
	}
	else if (ent->client->ps.eFlags & PMF_DUCKED) {
		scale *= ent->client->ps.crouchSpeedScale;
	}
#endif

	return scale;
}

void ClientThink_cmd(gentity_t* ent, usercmd_t* cmd);

void DoClientThinks(gentity_t* ent) {

	int lastCmd, lastTime;
	int latestTime;
	int drop_threshold = LAG_MAX_DROP_THRESHOLD;
	int startPackets = ent->client->cmdcount;
	//	usercmd_t newcmd;

	if (ent->client->cmdcount <= 0)
	{
		return;
	}

	// allow some more movement if time has passed
	latestTime = trap_Milliseconds();
	if (ent->client->lastCmdRealTime > latestTime)
	{
		// zinx - stoopid server went backwards in time, reset the delta
		// instead of giving them even -less- movement ability
		ent->client->cmddelta = 0;
	}
	else
	{
		ent->client->cmddelta -= (latestTime - ent->client->lastCmdRealTime);
	}
	if (ent->client->cmdcount <= 1 && ent->client->cmddelta < 0)
	{
		ent->client->cmddelta = 0;
	}
	ent->client->lastCmdRealTime = latestTime;

	lastCmd = (ent->client->cmdhead + ent->client->cmdcount - 1) % LAG_MAX_COMMANDS;

	lastTime = ent->client->ps.commandTime;
	latestTime = ent->client->cmds[lastCmd].serverTime;

	while (ent->client->cmdcount > 0)
	{
		usercmd_t* cmd = &ent->client->cmds[ent->client->cmdhead];
		float speed, delta, scale;
		int savedTime;
		qboolean deltahax = qfalse;

		int serverTime = cmd->serverTime;
		int totalDelta = latestTime - cmd->serverTime;
		int timeDelta;

		if (ent->client->pers.pmoveFixed)
		{
			serverTime = ((serverTime + pmove_msec.integer - 1) / pmove_msec.integer) * pmove_msec.integer;
		}

		timeDelta = serverTime - lastTime;

		if (totalDelta >= drop_threshold)
		{
			// zinx - whoops. too lagged.
			drop_threshold = LAG_MIN_DROP_THRESHOLD;
			lastTime = ent->client->ps.commandTime = cmd->serverTime;
			goto drop_packet;
		}

		if (totalDelta < 0)
		{
			// zinx - oro? packet from the future
			goto drop_packet;
		}

		if (timeDelta <= 0)
		{
			// zinx - packet from the past
			goto drop_packet;
		}

		scale = 1.f / LAG_DECAY;

		speed = AW_CmdScale(ent, cmd);
		delta = (speed * (float)timeDelta);
		delta *= scale;

		if (timeDelta > 50)
		{
			timeDelta = 50;
			delta = (speed * (float)timeDelta);
			delta *= scale;
			deltahax = qtrue;
		}

		if ((ent->client->cmddelta + delta) >= LAG_MAX_DELTA)
		{
			// too many commands this server frame

			// if it'll fit in the next frame, just wait until then.
			if (delta < LAG_MAX_DELTA
				&& (totalDelta + delta) < LAG_MIN_DROP_THRESHOLD)
			{
				break;
			}

			// try to split it up in to smaller commands

			delta = ((float)LAG_MAX_DELTA - ent->client->cmddelta);
			timeDelta = ceil(delta / speed); // prefer speedup
			delta = (float)timeDelta * speed;

			if (timeDelta < 1)
			{
				break;
			}

			delta *= scale;
			deltahax = qtrue;
		}

		ent->client->cmddelta += delta;

		if (deltahax)
		{
			savedTime = cmd->serverTime;
			cmd->serverTime = lastTime + timeDelta;
		}
		else
		{
			savedTime = 0;  // zinx - shut up compiler
		}

		// zinx - erh.  hack, really. make it run for the proper amount of time.
		ent->client->ps.commandTime = lastTime;
		ClientThink_cmd(ent, cmd);
		lastTime = ent->client->ps.commandTime;

		if (deltahax)
		{
			cmd->serverTime = savedTime;

			if (delta <= 0.1f)
			{
				break;
			}

			continue;
		}

	drop_packet:

		if (ent->client->cmdcount <= 0)
		{
			// ent->client was cleared...
			break;
		}

		ent->client->cmdhead = (ent->client->cmdhead + 1) % LAG_MAX_COMMANDS;
		ent->client->cmdcount--;
		continue;
	}

	// zinx - added ping, packets processed this frame
	// warning: eats bandwidth like popcorn
	if (g_antiWarp.integer & 32)
	{
		trap_SendServerCommand(
			ent - g_entities,
			va("cp \"%d %d\n\"", latestTime - lastTime, startPackets - ent->client->cmdcount)
		);
	}

	// zinx - debug; size is added lag (amount above player's network lag)
	// rotation is time
	if ((g_antiWarp.integer & 16) && ent->client->cmdcount)
	{
		vec3_t org, parms;

		VectorCopy(ent->client->ps.origin, org);
		SnapVector(org);

		parms[0] = 3;
		parms[1] = (float)(latestTime - ent->client->ps.commandTime) / 10.f;
		if (parms[1] < 1.f)
		{
			parms[1] = 1.f;
		}
		parms[2] = (ent->client->ps.commandTime * 180.f) / 1000.f;

		//etpro_AddDebugLine( org, parms, ((ent - g_entities) % 32), LINEMODE_SPOKES, LINESHADER_RAILCORE, 0, qfalse );
	}

	//ent->client->ps.stats[STAT_ANTIWARP_DELAY] = latestTime - ent->client->ps.commandTime;
	//if (ent->client->ps.stats[STAT_ANTIWARP_DELAY] < 0)
	//	ent->client->ps.stats[STAT_ANTIWARP_DELAY] = 0;
}
