#include "cg_local.h"


qbool CG_NDP_AnalyzeObituary(entityState_t* ent, snapshot_t* snapshot);






int m_currServerTime;
int m_firstServerTime;
int m_lastServerTime;
static char m_demoPath[4096];
static char ndp_configStrings[MAX_GAMESTATE_CHARS];
int ndp_configStringOffsets[MAX_CONFIGSTRINGS];
int ndp_numConfigStringBytes;

int ndp_myKills[1024] = { 0 };
int ndp_myKillsSize = 0;
int ndp_alliesWins[1024] = { 0 };
int ndp_axisWins[1024] = { 0 };
int ndp_alliesWinsSize = 0;
int ndp_axisWinsSize = 0;
int ndp_lastgamestate = 0;
int ndp_currentRound = 0;
int ndp_nextTimeLimit = 0;

int ndp_round1End[32] = { 0 };
int ndp_round2End[32] = { 0 };
int ndp_round1EndSize = 0;
int ndp_round2EndSize = 0;

int ndp_docPickupTime[1024] = { 0 };
int ndp_docDropTime[1024] = { 0 };
int ndp_docPickupSize = 0;
int ndp_docDropSize = 0;
qbool ndp_killStreak[1024] = { qfalse };
qbool isRtcwPro = qfalse;
qbool isRtcwProV128 = qfalse;
qbool isRtcwProV129 = qfalse;
qbool isRtcwProV130 = qfalse;
qbool isRtcw10 = qfalse;

/*
=================
CG_NDP_FindGameVersion

Figure out what game version the demo is from the config string. 

If we know what it is we don't need to look for it again
=================
*/

static qbool gameVersionFound = qfalse;
qbool CG_NDP_FindGameVersion(void) {
	const char* info = CG_ConfigString(CS_SERVERINFO);
	char* current_gamename = Info_ValueForKey(info, "gamename");
	if (strlen(current_gamename) == 0) {
		return qfalse;
	}
	isRtcw10 = (Q_strncmp(current_gamename, "^1RTCW  1.0", strlen("^1RTCW  1.0")) == 0);
	isRtcwPro = (Q_strncmp(current_gamename, "RtcwPro", strlen("RtcwPro")) == 0);
	isRtcwProV128 = (Q_strncmp(current_gamename, "RtcwPro 1.2.8", strlen("RtcwPro 1.2.8")) == 0);
	isRtcwProV129 = (Q_strncmp(current_gamename, "RtcwPro 1.2.9", strlen("RtcwPro 1.2.9")) == 0);
	isRtcwProV130 = (Q_strncmp(current_gamename, "RtcwPro 1.3", strlen("RtcwPro 1.3")) == 0);
	//earlier versions all use 1.2.8 tinfo
	if (isRtcwPro && !isRtcwProV128 && !isRtcwProV129 && !isRtcwProV130) {
		isRtcwProV128 = qtrue;
	}
	gameVersionFound = qtrue;
	return qtrue;
}

/*
=================
CG_NDP_AnalyzeSnapshot

Syscall invoked by the client when a new snapshot has been loaded into cl_demo. 

Several successive snapshots may have the same entity event that has already been handled.
=================
*/
static int prevEvents[MAX_GENTITIES] = { 0 };
static qboolean prevPresent[MAX_GENTITIES] = { 0 };
static qboolean currPresent[MAX_GENTITIES] = { 0 };
qbool CG_NDP_AnalyzeSnapshot(int progress)
{
	int snapshotNumber;
	int serverTime;
	trap_GetCurrentSnapshotNumber(&snapshotNumber, &serverTime);

	snapshot_t snapshot;
	trap_GetSnapshot(snapshotNumber, &snapshot);

	if (!gameVersionFound) {
		if (CG_NDP_FindGameVersion() == qfalse) {
			return qfalse;
		}
	}

	int i;
	entityState_t* es;
	for (i = 0; i < snapshot.numEntities; i++) {
		es = &snapshot.entities[i];

		if (!prevPresent[es->number]) {
			prevEvents[es->number] = 0;
		}
		currPresent[es->number] = qtrue;

		//check for event only entities
		if (es->eType > ET_EVENTS) {
			//skip events already handled
			if (prevPresent[es->number]) {
				continue;
			}

			es->event = es->eType - ET_EVENTS;
			prevEvents[es->number] = es->event;
		}
		else
		{
			//check for events riding with another entity
			int j;

			for (j = 0; j < MAX_EVENTS; j++) {
				if (es->events[j] > ET_EVENTS) {
					if (prevEvents[es->number]) {
						continue;
					}
					prevEvents[es->number] = es->events[j];
					es->event = es->events[j];
				}
				else {
					continue;
				}
				if ((es->events[j] & (~EV_EVENT_BITS)) == 0) {
					continue;
				}
			}
		}
		if (isRtcwPro) {
			switch (es->event) {
			case 86: //EV_OBITUARY+1
				CG_NDP_AnalyzeObituary(es, &snapshot);
				break;
			default:
				break;
			}
		}
		else if (!isRtcw10) {
			switch (es->event) {
			case 85: //EV_OBITUARY
				CG_NDP_AnalyzeObituary(es, &snapshot);
				break;
			default:
				break;
			}
		}
	}

	for (i = 0; i < MAX_GENTITIES; i++) {
		prevPresent[i] = currPresent[i];
		currPresent[i] = qfalse;
	}
	return qtrue;

}

/*
=================
CG_NDP_AnalyzeCommand

Receives all server commands for analysis. Pick interesting stuff to save for the timeline
=================
*/
static qbool ndp_someoneHasDocs = qfalse;
static int ndp_defender = 0;
void CG_NDP_AnalyzeCommand(int serverTime)
{
	//int argc = trap_Argc();
	const char* cmdName = CG_Argv(0);
	//Com_Printf("%s\n", cmdName);
	if (Q_stricmp(cmdName, "cs") == 0) {
		if (atoi(CG_Argv(1)) == CS_MULTI_MAPWINNER) {
			int winner = atoi(Info_ValueForKey(CG_Argv(2), "winner"));

			if (ndp_lastgamestate == GS_PLAYING) {
				if (!winner) {
					ndp_axisWins[ndp_axisWinsSize++] = serverTime;
				}
				else {
					ndp_alliesWins[ndp_alliesWinsSize++] = serverTime;
				}
				if (ndp_currentRound == 0) {
					ndp_round1End[ndp_round1EndSize++] = serverTime;
				}
				else {
					ndp_round2End[ndp_round2EndSize++] = serverTime;
				}
			}

		}
		if (atoi(CG_Argv(1)) == CS_WOLFINFO) {
			const char*	info = CG_Argv(2);
			ndp_lastgamestate = atoi(Info_ValueForKey(info, "gamestate"));
			ndp_currentRound = atoi(Info_ValueForKey(info, "g_currentRound"));
			ndp_nextTimeLimit = atof(Info_ValueForKey(info, "g_nextTimeLimit"));
		}
		if (atoi(CG_Argv(1)) == CS_MULTI_INFO) {
			const char* info = CG_Argv(2);
			ndp_defender = atoi(Info_ValueForKey(info, "defender"));
		}

	}
	if (Q_stricmp(cmdName, "print") == 0) {
		if (Q_stricmp(CG_Argv(1), "Timelimit hit.\n") == 0) {
			if (ndp_defender) {
				ndp_alliesWins[ndp_alliesWinsSize++] = serverTime;
			}
			else {
				ndp_axisWins[ndp_axisWinsSize++] = serverTime;
			}
			if (ndp_currentRound == 0) {
				ndp_round1End[ndp_round1EndSize++] = serverTime;
			}
			else {
				ndp_round2End[ndp_round2EndSize++] = serverTime;
			}
		}
	}
	if (Q_stricmp(cmdName, "tinfo") == 0) {
		int i, powerups;
		qbool someoneHasDocsNow = qfalse;

		if (!gameVersionFound) {
			if (CG_NDP_FindGameVersion() == qfalse) {
				return;
			}
		}

		if (!isRtcwPro) {
			numSortedTeamPlayers = atoi(CG_Argv(3));

			for (i = 0; i < numSortedTeamPlayers; i++) {
				powerups = atoi(CG_Argv(i * 5 + 7));
				if (powerups & ((1 << PW_REDFLAG) | (1 << PW_BLUEFLAG) | (1 << PW_CAPPEDOBJ))) {
					someoneHasDocsNow = qtrue;
				}
			}

		}
		else {
			if (isRtcwProV128) {
				numSortedTeamPlayers = atoi(CG_Argv(3));
				for (i = 0; i < numSortedTeamPlayers; i++) {
					powerups = atoi(CG_Argv(i * 11 + 7));
					if (powerups & ((1 << PW_REDFLAG) | (1 << PW_BLUEFLAG) | (1 << PW_CAPPEDOBJ))) {
						someoneHasDocsNow = qtrue;
					}
				}
			}
			else if (isRtcwProV129) {
				numSortedTeamPlayers = atoi(CG_Argv(3));
				for (i = 0; i < numSortedTeamPlayers; i++) {
					powerups = atoi(CG_Argv(i * 12 + 7));
					if (powerups & ((1 << PW_REDFLAG) | (1 << PW_BLUEFLAG) | (1 << PW_CAPPEDOBJ))) {
						someoneHasDocsNow = qtrue;
					}
				}
			}
			else if (isRtcwProV130) {
				int teamInfoPlayers = Q_atoi(CG_Argv(1));
				for (i = 0; i < teamInfoPlayers; i++) {
					powerups = Q_atoi(CG_Argv(i * 12 + 5));
					if (powerups & ((1 << PW_REDFLAG) | (1 << PW_BLUEFLAG) | (1 << PW_CAPPEDOBJ))) {
						someoneHasDocsNow = qtrue;
					}
				}
			}


		}
		if (!ndp_someoneHasDocs && someoneHasDocsNow) {
			//doc pick up
			ndp_someoneHasDocs = someoneHasDocsNow;
			ndp_docPickupTime[ndp_docPickupSize++] = serverTime;
		}
		else if (ndp_someoneHasDocs && !someoneHasDocsNow) {
			//doc drop
			ndp_someoneHasDocs = someoneHasDocsNow;
			ndp_docDropTime[ndp_docDropSize++] = serverTime;
		}

	}
}

/*
=================
CG_NDP_AnalyzeObituary

Uses the obitary event from the snapshot to save your kills for the timeline
=================
*/

qbool CG_NDP_AnalyzeObituary(entityState_t* ent, snapshot_t* snapshot) {
	int mod;
	int target, attacker;
	//int killtype = 0;               // DHM - Nerve :: 0==Axis; 1==Allied; 2==your kill
	//char* message;
	//char* message2;
	const char* targetInfo;
	const char* attackerInfo;
	char targetName[32];
	//char attackerName[32];
	int attackerTeam;
	int targetTeam;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;
	if (attacker < 0 || attacker >= MAX_CLIENTS) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
		return qtrue;
	}
	else {
		attackerInfo = CG_ConfigString(CS_PLAYERS + attacker);
		attackerTeam = atoi(Info_ValueForKey(attackerInfo, "t"));
	}

	targetInfo = CG_ConfigString(CS_PLAYERS + target);
	if (!targetInfo) {
		return qtrue;
	}
	Q_strncpyz(targetName, Info_ValueForKey(targetInfo, "n"), sizeof(targetName) - 2);
	targetTeam = atoi(Info_ValueForKey(targetInfo, "t"));

	// check for kill messages from the current clientNum, ignore self kills and team kills
	if (attacker == snapshot->ps.clientNum && attacker != target && attackerTeam != targetTeam) {
		ndp_myKills[ndp_myKillsSize++] = snapshot->serverTime;
	}
	return qtrue;

}


static void RestoreSession(void)
{
	char session[256];
	int ms;
	float speed;

	trap_Cvar_VariableStringBuffer("demo_SessionData", session, sizeof(session));
	if (sscanf(session, "%d %f", &ms, &speed) == 2) {
		if (ms >= m_firstServerTime && ms <= m_lastServerTime) {
			m_currServerTime = ms;
			cg_timescale.value = speed;
		}
	}
}


/*
=================
CG_NDP_EndAnalysis

Syscall invokes this when demo parsing has completed. Demo parsing is performed after a video restart as well. 

For video restart, restore the time the demo was at. Session save is performed in CG_Shutdown. 
=================
*/

void CG_NDP_EndAnalysis(const char* filePath, int firstServerTime, int lastServerTime, qboolean videoRestart)
{

	int i, firstKillTime = 0, streakCounter = 0;
	int distanceFromFirst = 0;
	for (i = 0; i < ndp_myKillsSize; i++) {
		if (!firstKillTime) {
			//initialize
			firstKillTime = ndp_myKills[i];
			streakCounter++;
			continue;
		}
		distanceFromFirst = ndp_myKills[i] - firstKillTime;
		if (distanceFromFirst < 10000) {
			streakCounter++;
		}
		else {
			//streak ended
			if (streakCounter >= 2) {
				int j;
				for (j = i - 1; j > (i - 1) - streakCounter; j--) {
					ndp_killStreak[j] = qtrue;
				}
			}
			//reset
			streakCounter = 1;
			firstKillTime = ndp_myKills[i];
		}
	}
	

	m_firstServerTime = firstServerTime;
	m_lastServerTime = lastServerTime;

	if (videoRestart) {
		RestoreSession();
	}
	else {
		m_currServerTime = firstServerTime;
	}

	strcpy(m_demoPath, filePath);
	trap_CNQ3_NDP_Seek(m_currServerTime);
	cgs.serverCommandSequence = 0;

}

void CG_NDP_SeekAbsolute(int serverTime)
{
	if (serverTime != m_currServerTime) {
		int time = trap_CNQ3_NDP_Seek(serverTime);
		m_currServerTime = time;
		//cg.nextSnap = NULL;
		//cg.snap = NULL;
		//CG_SeekDemo(serverTime, SEEKDEMO_UPDATETIME_BIT | SEEKDEMO_RATELIMIT_BIT);
	}
}

void CG_NDP_SeekRelative(int seconds)
{
	CG_NDP_SeekAbsolute(cg.time + seconds * 1000);
}


/*
=================
CG_NDP_ResetStateWhenBackInTime

When the demo goes backwards in time, animations using "Time" to determine when to stop need to be reset
=================
*/
void CG_NDP_ResetStateWhenBackInTime(void)
{
	CG_InitLocalEntities();
	CG_InitMarkPolys();
	CG_ClearParticles();
	CG_ClearTrails();
	trap_S_ClearLoopingSounds(qtrue);
	CG_SoundInit();
	trap_SendConsoleCommand("s_stop\n");

	cg.damageTime = 0.0f;
	cg.v_dmg_time = 0.0f;
	cg.centerPrintAnnouncerTime = 0;
	cg.attackerTime = 0;
	cg.cameraShakeTime = 0;
	cg.centerPrintTime = 0;
	cg.crosshairClientTime = 0;
	cg.crosshairPowerupTime = 0;
	cg.cursorHintTime = 0;
	cg.duckTime = 0;
	cg.fadeTime = 0;
	cg.grenLastTime = 0;
	cg.headEndTime = 0;
	cg.headStartTime = 0;
	cg.holdableSelectTime = 0;
	cg.identifyNextTime = 0;
	cg.itemFadeTime = 0;
	cg.itemPickupBlendTime = 0;
	cg.itemPickupTime = 0;
	cg.landTime = 0;
	cg.lastGetTriggerDistancesTime = 0;
	cg.lastKillTime = 0;
	cg.nextOrbitTime = 0;
	cg.oidPrintTime = 0;
	cg.popinPrintTime = 0;
	cg.powerupTime = 0;
	cg.rewardTime = 0;
	cg.scoreFadeTime = 0;
	cg.scoresRequestTime = 0;
	cg.statsRequestTime = 0;
	cg.stepTime = 0;
	cg.topshotsRequestTime = 0;
	cg.v_fireTime = 0;
	cg.v_noFireTime = 0;
	cg.voiceChatTime = 0;
	cg.voiceTime = 0;
	cg.weaponAnimationTime = 0;
	cg.weaponSelectTime = 0;
	cg.zoomTime = 0;
	cg.zoomedTime = 0;
	cgs.notifyPos = 0;
	cgs.notifyLastPos = 0;
	cgs.teamChatPos = 0;
	cgs.teamLastChatPos = 0;
}


/*
=================
CG_NDP_SetGameTime

Advance the demo playback by incrementing the current time every frame
=================
*/
void CG_NDP_SetGameTime() {
	static int prevRealTime = 0;
	const int currRealTime = trap_Milliseconds();
	const int frameDuration = currRealTime - prevRealTime;

	m_currServerTime += (int)((float)frameDuration * cg_timescale.value);

	if (m_currServerTime >= m_lastServerTime)
	{
		m_currServerTime = m_lastServerTime;
	}
	else
	{
		trap_CNQ3_NDP_ReadUntil(m_currServerTime);
	}

	cg.time = m_currServerTime;
	prevRealTime = currRealTime;

}

void CG_NDP_GoToNextFrag(qbool forward) {
	int i; 
	if (forward) {
		for (i = 0; i < ndp_myKillsSize; i++) {
			if (ndp_myKills[i] > m_currServerTime) {
				if (ndp_myKills[i] - 3000 < m_currServerTime) {
					continue;
				}
				CG_NDP_SeekAbsolute(ndp_myKills[i] - 3000);
				return;
			}
		}
	}
	else {
		for (i = ndp_myKillsSize - 1; i >= 0; i--) {
			if (ndp_myKills[i] < m_currServerTime) {
				CG_NDP_SeekAbsolute(ndp_myKills[i] - 3000);
				return;
			}
		}
	}
	
}