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
	if (!g_antiWarp.integer || g_gamestate.integer == GS_INTERMISSION)
	{
		return qfalse;
	}


	if (ent && ent->client)
	{
		// don't antiwarp spectators and players that are in limbo
		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR || (ent->client->ps.pm_flags & PMF_LIMBO))
		{
			return qfalse;
		}

		// don't antiwarp bots - ET Legacy port
		if (ent->r.svFlags & SVF_BOT)
		{
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
static float G_CmdScale(gentity_t *ent, usercmd_t *cmd)
{


	float scale = abs(cmd->forwardmove);
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


	return scale;
}

void ClientThink_cmd(gentity_t* ent, usercmd_t* cmd);

void DoClientThinks(gentity_t *ent)
{
	usercmd_t *cmd;
	float     speed, delta, scale;
	int       lastCmd, lastTime, latestTime, serverTime, totalDelta, timeDelta, savedTime;
	int drop_threshold = LAG_MAX_DROP_THRESHOLD;
	int startPackets = ent->client->cmdcount;
	qboolean  deltahax;

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
		cmd = &ent->client->cmds[ent->client->cmdhead];

		deltahax = qfalse;

		serverTime = cmd->serverTime;
		totalDelta = latestTime - cmd->serverTime;

		if (pmove_fixed.integer || ent->client->pers.pmoveFixed)
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

		speed = G_CmdScale(ent, cmd);

		// if the warping player stopped but still has some speed keep antiwarping
		if (speed == 0 && VectorLength(ent->client->ps.velocity) > LAG_SPEED_THRESHOLD)
		{
			speed = 1.0f;
		}

		if (timeDelta > 50)
		{
			timeDelta = 50;
			delta = (speed * (float)timeDelta);
			delta *= scale;
			deltahax = qtrue;
		}
		else
		{
			delta = (speed * (float)timeDelta);
			delta *= scale;
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
			timeDelta = (int)(ceil((double)(delta / speed))); // prefer speedup
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

#ifdef ANTIWARP_DEBUG
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
#endif

	//ent->client->ps.stats[STAT_ANTIWARP_DELAY] = latestTime - ent->client->ps.commandTime;
	//if (ent->client->ps.stats[STAT_ANTIWARP_DELAY] < 0)
	//	ent->client->ps.stats[STAT_ANTIWARP_DELAY] = 0;
}
