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

#include "g_local.h"

level_locals_t level;

typedef struct {
	vmCvar_t    *vmCvar;
	char        *cvarName;
	char        *defaultString;
	int cvarFlags;
	int modificationCount;          // for tracking changes
	qboolean trackChange;           // track this variable, and announce if changed
	qboolean fConfigReset;          // OSP: set this var to the default on a config reset
	qboolean teamShader;      // track and if changed, update shader state
} cvarTable_t;

gentity_t g_entities[MAX_GENTITIES];
gclient_t g_clients[MAX_CLIENTS];

gentity_t       *g_camEnt = NULL;   //----(SA)	script camera

// Rafael gameskill
extern int bg_pmove_gameskill_integer;
// done

vmCvar_t g_gametype;

// Rafael gameskill
vmCvar_t g_gameskill;
// done

vmCvar_t g_dmflags;
vmCvar_t g_fraglimit;
vmCvar_t g_timelimit;
vmCvar_t g_capturelimit;
vmCvar_t g_friendlyFire;
vmCvar_t g_password;
vmCvar_t g_maxclients;
vmCvar_t g_maxGameClients;
vmCvar_t g_minGameClients;          // NERVE - SMF
vmCvar_t g_dedicated;
vmCvar_t g_speed;
vmCvar_t g_gravity;
vmCvar_t g_cheats;
vmCvar_t g_knockback;
vmCvar_t g_quadfactor;
vmCvar_t g_forcerespawn;
vmCvar_t g_inactivity;
vmCvar_t g_debugMove;
vmCvar_t g_debugDamage;
vmCvar_t g_debugAlloc;
vmCvar_t g_debugBullets;    //----(SA)	added
vmCvar_t g_debugMode;
vmCvar_t g_weaponRespawn;
vmCvar_t g_motd;
vmCvar_t g_synchronousClients;
vmCvar_t g_warmup;
// NERVE - SMF
vmCvar_t g_warmupLatch;
vmCvar_t g_nextTimeLimit;
vmCvar_t g_preciseTimeSet;
vmCvar_t g_usePreciseConsoleTime;
vmCvar_t g_showHeadshotRatio;
vmCvar_t g_userTimeLimit;
vmCvar_t g_userAlliedRespawnTime;
vmCvar_t g_userAxisRespawnTime;
vmCvar_t g_currentRound;
vmCvar_t g_noTeamSwitching;
vmCvar_t g_altStopwatchMode;
vmCvar_t g_gamestate;
vmCvar_t g_swapteams;
// -NERVE - SMF

vmCvar_t g_restarted;
vmCvar_t g_log;

vmCvar_t g_logSync;
vmCvar_t g_podiumDist;
vmCvar_t g_podiumDrop;
vmCvar_t g_voteFlags;
vmCvar_t g_complaintlimit;          // DHM - Nerve
vmCvar_t g_maxlives;                // DHM - Nerve
vmCvar_t g_voiceChatsAllowed;       // DHM - Nerve
vmCvar_t g_alliedmaxlives;          // Xian
vmCvar_t g_axismaxlives;            // Xian
vmCvar_t g_fastres;                 // Xian
vmCvar_t g_fastResMsec;
vmCvar_t g_knifeonly;               // Xian
vmCvar_t g_enforcemaxlives;         // Xian

vmCvar_t g_needpass;
vmCvar_t g_weaponTeamRespawn;
vmCvar_t g_doWarmup;
vmCvar_t g_teamAutoJoin;
vmCvar_t g_teamForceBalance;
vmCvar_t g_listEntity;
vmCvar_t g_banIPs;
vmCvar_t g_filterBan;
vmCvar_t g_rankings;
vmCvar_t g_enableBreath;
vmCvar_t g_smoothClients;
vmCvar_t pmove_fixed;
vmCvar_t pmove_msec;

// Rafael
vmCvar_t g_autoactivate;
vmCvar_t g_testPain;

vmCvar_t g_missionStats;
vmCvar_t ai_scriptName;         // name of AI script file to run (instead of default for that map)
vmCvar_t g_scriptName;          // name of script file to run (instead of default for that map)

vmCvar_t g_developer;

vmCvar_t g_userAim;

vmCvar_t g_forceModel;
vmCvar_t g_mg42arc;

vmCvar_t g_footstepAudibleRange;
// JPW NERVE multiplayer reinforcement times
vmCvar_t g_redlimbotime;
vmCvar_t g_bluelimbotime;

//vmCvar_t g_asoffset; // temporary for adjusting a/s delay
// charge times for character class special weapons
vmCvar_t g_medicChargeTime;
vmCvar_t g_engineerChargeTime;
vmCvar_t g_LTChargeTime;
vmCvar_t g_soldierChargeTime;

// jpw

// Gordon
vmCvar_t g_antilag;

vmCvar_t mod_url;
vmCvar_t url;

vmCvar_t g_dbgRevive;

// rtcwpro begin
//S4NDM4NN - fix errors when sv_fps is adjusted
vmCvar_t g_screenShake;
vmCvar_t g_preciseHeadHitBox;
vmCvar_t sv_fps;
//vmCvar_t g_gamelocked;	// Controls if referee locked the game so players can't join
vmCvar_t sv_hostname;	// So it's more accesible
vmCvar_t svx_serverStreaming; // So it's more accessible

// stats
vmCvar_t g_stats_curl_submit;
vmCvar_t g_stats_curl_submit_URL;
vmCvar_t g_stats_curl_submit_headers;
vmCvar_t g_gameStatslog; // temp cvar for event logging
vmCvar_t g_statsDebug; // write in logfile to debug crashes
vmCvar_t g_statsRetryCount;
vmCvar_t g_statsRetryDelay;
vmCvar_t g_apiquery_curl_URL;

// Match
vmCvar_t team_maxplayers;
vmCvar_t team_nocontrols;

vmCvar_t match_warmupDamage;
vmCvar_t match_mutespecs;
vmCvar_t match_latejoin;
vmCvar_t match_minplayers;
vmCvar_t match_readypercent;
vmCvar_t match_timeoutlength;
vmCvar_t match_timeoutcount;

vmCvar_t vote_allow_comp;
vmCvar_t vote_allow_gametype;
vmCvar_t vote_allow_kick;
vmCvar_t vote_allow_map;
vmCvar_t vote_allow_matchreset;
vmCvar_t vote_allow_mutespecs;
vmCvar_t vote_allow_nextmap;
//vmCvar_t vote_allow_pub;
vmCvar_t vote_allow_referee;
vmCvar_t vote_allow_shuffleteamsxp;
vmCvar_t vote_allow_swapteams;
vmCvar_t vote_allow_friendlyfire;
vmCvar_t vote_allow_timelimit;
vmCvar_t vote_allow_warmupdamage;
vmCvar_t vote_allow_antilag;
vmCvar_t vote_allow_balancedteams;
vmCvar_t vote_allow_muting;
vmCvar_t vote_allow_cointoss;
vmCvar_t vote_allow_knifeonly;
vmCvar_t vote_limit;
vmCvar_t vote_percent;
vmCvar_t refereePassword;
vmCvar_t shoutcastPassword;

vmCvar_t g_spectatorInactivity;
vmCvar_t g_allowSoftKill;
vmCvar_t g_bannedMSG;	// Message that's printed to banned users
vmCvar_t g_drawHitboxes;	// So I can adjust them a little..
vmCvar_t g_hitsounds;			// Hitsounds - Requires soundpack
vmCvar_t g_privateServer; // If disabled it doesn't check for password but instead re-uses it for ban bypass based on NO-SQL solution.
vmCvar_t TXThandle;		// Optional so text (file) based checks (ip/guid tempban-ban) can be disabled..
vmCvar_t g_disableInv;			// Disables (re)spawn/revive protection crap when one starts to shot..
vmCvar_t g_unlockWeapons;		// Gives ability to drop weapon to all classes..
vmCvar_t g_crouchRate;			// If enabled it recharges stamina faster when player is crouching..
vmCvar_t g_disableSMGPickup;		// If enabled, client can't pickup SMG if they already have one.
vmCvar_t g_fastStabSound;		// 0 = off, 1 = (OSP's) goat sound, 2 = humiliation sound, 3 = random between 1 or 2
vmCvar_t g_axisSpawnProtectionTime;		// How long Axis player is invulrable when (s)he spawns.
vmCvar_t g_alliedSpawnProtectionTime;	// How long Allied player is invulrable when (s)he spawns.

// MOTD's
vmCvar_t g_serverMessage;	// Shows a center print each time when player switches teams.

// General
vmCvar_t g_showFlags;	// mcwf GeoIP
vmCvar_t g_inactivityToSpecs;	// Puts inactive players in spectators instead of dropping them.
vmCvar_t g_ignoreSpecs;	// Ignores spectators - referees can still bypass the ignore..
vmCvar_t g_allowVote;	// Replaced voteFlags as i hate it and see it to complicated for new users..
vmCvar_t g_disallowedVotes; // Disalloved votes separeted by space..
vmCvar_t g_headshotsOnly;	// Enables headshots only mode
vmCvar_t g_allowPMs;		// Allow private messages
vmCvar_t g_hitsounds;		// Hitsounds - client can disable them localy if they're on.
vmCvar_t g_mapConfigs;		// Essentials for custom map configs...
vmCvar_t g_lifeStats;		// If enabled it prints killer health
vmCvar_t g_pauseLimit;	// How many pauses per team
vmCvar_t g_duelAutoPause; // If enabled, it auto pauses when in duel mode with uneven teams.
vmCvar_t g_damageRadiusKnockback; // explosions knockback

// Match specific
vmCvar_t g_tournament;	// Ready-unready system
vmCvar_t g_ltNades;			// Number of nades a lt starts with
vmCvar_t g_medicNades;		// Number of nades a med starts with
vmCvar_t g_soldNades;		// Number of nades sold starts with
vmCvar_t g_engNades;		// Number of nades eng starts with
vmCvar_t g_medicClips;		// Number of clips in weapon med starts with
vmCvar_t g_engineerClips;	// Number of clips in weapon eng starts with
vmCvar_t g_soldierClips;	// Number of clips in weapon sold starts with
vmCvar_t g_leutClips;		// Number of clips in weapon leut starts with
vmCvar_t g_pistolClips;		// How many clips pistol gets
vmCvar_t g_maxTeamPF;		// Max Pf's per team
vmCvar_t g_maxTeamSniper;	// Max snipers per team
vmCvar_t g_maxTeamVenom;	// Max venoms per team
vmCvar_t g_maxTeamFlamer;	// Max flamers per 
vmCvar_t g_allowReadyTeam;  // Allow readyteam command

vmCvar_t g_antiWarp;
vmCvar_t g_dropWeapons;			// allow drop weapon for each class, bitflag value: 1 - soldier, 2 - eng, 4 - medic, 8 - lt, default 9

vmCvar_t stats_matchid;

vmCvar_t P; // ET Port Players server info
vmCvar_t g_hsDamage;
vmCvar_t g_spawnOffset;
vmCvar_t g_bodiesGrabFlags;
vmCvar_t g_mapScriptDirectory;
vmCvar_t g_thinkStateLevelTime;
vmCvar_t g_endStateLevelTime;
vmCvar_t g_thinkSnapOrigin;
vmCvar_t g_fixedphysicsfps;
vmCvar_t g_alternatePing;
vmCvar_t g_allowForceTapout;
vmCvar_t g_allowEnemySpawnTimer;
vmCvar_t g_reviveSameDirection;

vmCvar_t g_clientLogFile;  // log filename for client input
vmCvar_t g_logClientInput; // log unknown commands from client i.e. hacks

vmCvar_t g_allowSS; // allow /reqss
vmCvar_t g_ssAddress; // e.g. hostname or ip:port
vmCvar_t g_ssWebhookId; // id contained in the discord webhook link (numbers only) e.g. webhooks/id/
vmCvar_t g_ssWebhookToken; // token contained in the discord webhook link (chars) e.g. webhooks/id/token
vmCvar_t g_ssWaitTime; // wait time between reqss cmds to prevent spam
vmCvar_t g_broadcastClients; // fix clients appearing from thin air on some maps
vmCvar_t g_logConfigStringChanges; // log config string changes (debugging)
vmCvar_t g_playPauseMusic; // play music during pause

 // unlagged
vmCvar_t g_floatPlayerPosition;
vmCvar_t g_delagHitscan;
vmCvar_t g_maxExtrapolatedFrames;
vmCvar_t g_maxLagCompensation;
vmCvar_t g_delagMissiles;

cvarTable_t gameCvarTable[] = {
	// don't override the cheat state set by the system
	{ &g_cheats, "sv_cheats", "", 0, qfalse },

	// noset vars
	{ NULL, "gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
	{ NULL, "gamedate", __DATE__, CVAR_ROM, 0, qfalse  },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse  },
	{ NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
	{ NULL, "Compile Date", __DATE__, CVAR_ROM | CVAR_SERVERINFO, 0, qfalse  }, // RTCWPro

	// latched vars
	// DHM - Nerve :: default to GT_WOLF
	{ &g_gametype, "g_gametype", "5", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse  },

	// Rafael gameskill
	{ &g_gameskill, "g_gameskill", "3", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse  },
	// done

// JPW NERVE multiplayer stuffs
	{ &g_redlimbotime, "g_redlimbotime", "30000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	{ &g_bluelimbotime, "g_bluelimbotime", "30000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	{ &g_medicChargeTime, "g_medicChargeTime", "45000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	{ &g_engineerChargeTime, "g_engineerChargeTime", "30000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	{ &g_LTChargeTime, "g_LTChargeTime", "40000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
	{ &g_soldierChargeTime, "g_soldierChargeTime", "20000", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse },
// jpw

	{ &g_maxclients, "sv_maxclients", "20", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },            // NERVE - SMF - made 20 from 8
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_minGameClients, "g_minGameClients", "8", CVAR_SERVERINFO, 0, qfalse  },                              // NERVE - SMF

	// change anytime vars
	{ &g_dmflags, "dmflags", "0", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE, 0, qtrue  },
	{ &g_fraglimit, "fraglimit", "0", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_capturelimit, "capturelimit", "8", /*CVAR_SERVERINFO |*/ CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },

	{ &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse  },

	{ &g_friendlyFire, "g_friendlyFire", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue  },

	{ &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE  },                            // NERVE - SMF - merge from team arena

	{ &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_doWarmup, "g_doWarmup", "0", CVAR_ARCHIVE, 0, qtrue  },

	//S4NDM4NN - need to get sv_fps
	{ &sv_fps, "sv_fps", "20", 0, 0,qfalse},
	// NERVE - SMF
	{ &g_warmupLatch, "g_warmupLatch", "1", 0, 0, qfalse },

	{ &g_nextTimeLimit, "g_nextTimeLimit", "0", CVAR_WOLFINFO, 0, qfalse  },
	{ &g_preciseTimeSet, "g_preciseTimeSet", "0", CVAR_WOLFINFO, 0, qfalse  },
	{ &g_usePreciseConsoleTime, "g_usePreciseConsoleTime", "1", CVAR_WOLFINFO, 0, qfalse  },
	{ &g_currentRound, "g_currentRound", "0", CVAR_WOLFINFO, 0, qfalse  },
	{ &g_altStopwatchMode, "g_altStopwatchMode", "0", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_gamestate, "gamestate", "-1", CVAR_WOLFINFO | CVAR_ROM, 0, qfalse  },

	{ &g_noTeamSwitching, "g_noTeamSwitching", "0", CVAR_ARCHIVE, 0, qtrue  },

	{ &g_showHeadshotRatio, "g_showHeadshotRatio", "0", 0, 0, qfalse  },

	{ &g_userTimeLimit, "g_userTimeLimit", "0", 0, 0, qfalse  },
	{ &g_userAlliedRespawnTime, "g_userAlliedRespawnTime", "0", 0, 0, qfalse  },
	{ &g_userAxisRespawnTime, "g_userAxisRespawnTime", "0", 0, 0, qfalse  },
//	{ &g_asoffset, "g_asoffset", "1000", 0, 0, qfalse  },  // temporary for adjusting a/s delay
	{ &g_swapteams, "g_swapteams", "0", CVAR_ROM, 0, qfalse },
	// -NERVE - SMF

	{ &g_log, "g_log", "", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_gameStatslog, "g_gameStatslog", "16", CVAR_ARCHIVE, 0, qfalse  }, // default to 16 so the server saves JSON stats
	{ &g_statsDebug, "g_statsDebug", "0", CVAR_ARCHIVE, 0, qfalse  },
	// there will be future modifications and additions to the stats cvars....work in progress
//    { &g_stats_mongodb, "g_stats_mongodb", "0", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_stats_curl_submit, "g_stats_curl_submit", "0", CVAR_ARCHIVE, 0, qfalse  },
    { &g_stats_curl_submit_URL, "g_stats_curl_submit_URL", "https://rtcwproapi.donkanator.com/submit", CVAR_ARCHIVE, 0, qfalse  },
    { &g_stats_curl_submit_headers, "g_stats_curl_submit_headers", "0", CVAR_ARCHIVE, 0, qfalse  }, // not used at the moment, headers are currently hardcoded
	{ &g_statsRetryCount, "g_statsRetryCount", "3", CVAR_ARCHIVE, 0, qfalse  }, // number of attempts to send stats if first attempt fails
	{ &g_statsRetryDelay, "g_statsRetryDelay", "2", CVAR_ARCHIVE, 0, qfalse  }, // delay in seconds to retry sending stats if first attempt fails
	{ &g_apiquery_curl_URL, "g_apiquery_curl_URL", "https://rtcwproapi.donkanator.com/serverquery", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_password, "g_password", "", CVAR_USERINFO, 0, qfalse  },
	{ &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse  },
	// show_bug.cgi?id=500
	{ &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_dedicated, "dedicated", "0", 0, 0, qfalse  },
    { &g_unlockWeapons, "g_unlockWeapons", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
    { &g_disableSMGPickup, "g_disableSMGPickup", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_speed, "g_speed", "320", 0, 0, qtrue  },
	{ &g_gravity, "g_gravity", "800", 0, 0, qtrue  },
	{ &g_knockback, "g_knockback", "1000", 0, 0, qtrue  },
	{ &g_quadfactor, "g_quadfactor", "3", 0, 0, qtrue  },
	{ &g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, qtrue  },
	{ &g_weaponTeamRespawn, "g_weaponTeamRespawn", "30", 0, 0, qtrue },
	{ &g_forcerespawn, "g_forcerespawn", "0", 0, 0, qtrue },
	{ &g_inactivity, "g_inactivity", "0", 0, 0, qtrue },
	{ &g_debugMove, "g_debugMove", "0", 0, 0, qfalse },
	{ &g_debugDamage, "g_debugDamage", "0", CVAR_CHEAT, 0, qfalse },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse },
	{ &g_debugBullets, "g_debugBullets", "0", CVAR_CHEAT, 0, qfalse}, //----(SA)	added
	{ &g_debugMode, "g_debugMode", "0", CVAR_CHEAT, 0, qfalse },
	{ &g_preciseHeadHitBox, "g_preciseHeadHitBox", "1", 0, 0, qfalse }, // default to 1
	{ &g_motd, "g_motd", "", CVAR_ARCHIVE, 0, qfalse },

	{ &g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse },
	{ &g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse },

	{ &g_voteFlags, "g_voteFlags", "255", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qfalse },
	{ &g_listEntity, "g_listEntity", "0", 0, 0, qfalse },

	{ &g_complaintlimit, "g_complaintlimit", "3", CVAR_ARCHIVE, 0, qtrue},                        // DHM - Nerve
	{ &g_maxlives, "g_maxlives", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_SERVERINFO, 0, qtrue},     // DHM - Nerve
	{ &g_voiceChatsAllowed, "g_voiceChatsAllowed", "4", CVAR_ARCHIVE, 0, qfalse},             // DHM - Nerve

	{ &g_alliedmaxlives, "g_alliedmaxlives", "0", CVAR_LATCH | CVAR_SERVERINFO, 0, qtrue},      // Xian
	{ &g_axismaxlives, "g_axismaxlives", "0", CVAR_LATCH | CVAR_SERVERINFO, 0, qtrue},      // Xian
	{ &g_fastres, "g_fastres", "0", CVAR_ARCHIVE, 0, qtrue},                                  // Xian - Fast Medic Resing
	{ &g_fastResMsec, "g_fastResMsec", "1000", CVAR_ARCHIVE, 0, qtrue},                                   // Xian - Fast Medic Resing
	{ &g_knifeonly, "g_knifeonly", "0", 0, 0, qtrue},                                         // Xian - Fast Medic Resing
	{ &g_enforcemaxlives, "g_enforcemaxlives", "0", CVAR_ARCHIVE, 0, qtrue},                              // Xian - Gestapo enforce maxlives stuff by temp banning

	{ &g_enableBreath, "g_enableBreath", "1", CVAR_SERVERINFO, 0, qtrue},
	{ &g_testPain, "g_testPain", "0", CVAR_CHEAT, 0, qfalse },
	{ &g_missionStats, "g_missionStats", "0", CVAR_ROM, 0, qfalse },
	{ &g_developer, "developer", "0", CVAR_CHEAT, 0, qfalse },
	{ &g_rankings, "g_rankings", "0", 0, 0, qfalse},
	{ &g_userAim, "g_userAim", "1", CVAR_CHEAT, 0, qfalse },

	{ &g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse},
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, qfalse},
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qfalse},

	{&g_mg42arc, "g_mg42arc", "0", CVAR_TEMP, 0, qfalse},

	{&g_footstepAudibleRange, "g_footstepAudibleRange", "256", CVAR_CHEAT, 0, qfalse},

	{&g_scriptName, "g_scriptName", "", CVAR_ROM, 0, qfalse},
	{&ai_scriptName, "ai_scriptName", "", CVAR_ROM, 0, qfalse},

	// points to the URL for mod information, should not be modified by server admin
	{&mod_url, "mod_url", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse},
	// configured by the server admin, points to the web pages for the server
	{&url, "URL", "", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse},

	{ &g_antilag, "g_antilag", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse },
	//{ &g_maxVotes, "g_maxVotes", "2", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },

// match
	{ &team_maxplayers, "team_maxplayers", "0", 0, 0, qfalse, qfalse },
	{ &team_nocontrols, "team_nocontrols", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &match_warmupDamage, "match_warmupDamage", "1", 0, 0, qfalse },
	{ &match_mutespecs, "match_mutespecs", "0", 0, 0, qfalse, qtrue },
	{ &match_latejoin, "match_latejoin", "1", 0, 0, qfalse, qfalse },
	{ &match_minplayers, "match_minplayers", "2", 0, 0, qfalse, qfalse },
	{ &match_readypercent, "match_readypercent", "100", 0, 0, qfalse, qtrue },
	{ &match_timeoutlength, "match_timeoutlength", "180", 0, 0, qfalse, qtrue },
	{ &match_timeoutcount, "match_timeoutcount", "3", 0, 0, qfalse, qtrue },
	{ &g_showFlags, "g_showFlags", "1", 0 },
	//{ &g_noTeamSwitching, "g_noTeamSwitching", "1", 0, 0, qfalse, qfalse },
	//{ &g_gamelocked, "g_gamelocked", "0", CVAR_ROM, 0, qfalse },
	{ &g_hitsounds, "g_hitsounds", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &sv_hostname, "sv_hostname", "", CVAR_SERVERINFO, 0, qfalse },
	{ &g_drawHitboxes, "g_drawHitboxes", "0", 0, 0, qfalse },
	{ &svx_serverStreaming, "svx_serverStreaming", "", CVAR_SERVERINFO, 0, qfalse },
	{ &g_bannedMSG, "g_bannedMSG", "You are ^3Banned ^7from this server!", CVAR_ARCHIVE, 0, qfalse },
	{ &g_privateServer, "g_privateServer", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &TXThandle, "TXThandle", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_axisSpawnProtectionTime, "g_axisSpawnProtectionTime", "3000", CVAR_ARCHIVE, 0, qfalse },
	{ &g_alliedSpawnProtectionTime, "g_alliedSpawnProtectionTime", "3000", CVAR_ARCHIVE, 0, qfalse },
	{ &g_serverMessage, "g_serverMessage", "^1Server running RtcwPro", CVAR_ARCHIVE, 0, qfalse },
	{ &g_disableInv, "g_disableInv", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_fastStabSound, "g_fastStabSound", "0", CVAR_ARCHIVE, 0, qfalse },

	// voting
	{ &vote_limit, "vote_limit", "5", CVAR_ARCHIVE, qfalse, qfalse },
	{ &vote_percent, "vote_percent", "51", CVAR_ARCHIVE, qfalse, qfalse },
	{ &vote_allow_comp, "vote_allow_comp", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_gametype,     "vote_allow_gametype", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_kick,         "vote_allow_kick", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_map,          "vote_allow_map", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_matchreset,   "vote_allow_matchreset", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_mutespecs,    "vote_allow_mutespecs", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_nextmap,      "vote_allow_nextmap", "1", 0, 0, qfalse, qfalse },
	//{ &vote_allow_pub,          "vote_allow_pub", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_referee,      "vote_allow_referee", "0", 0, 0, qfalse, qfalse },
	{ &vote_allow_swapteams,    "vote_allow_swapteams", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_friendlyfire, "vote_allow_friendlyfire", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_timelimit,    "vote_allow_timelimit", "0", 0, 0, qfalse, qfalse },
	{ &vote_allow_warmupdamage, "vote_allow_warmupdamage", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_antilag,      "vote_allow_antilag", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_balancedteams,"vote_allow_balancedteams", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_muting,       "vote_allow_muting", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_cointoss,		"vote_allow_cointoss", "1", 0, 0, qfalse, qfalse },
	{ &vote_allow_knifeonly,	"vote_allow_knifeonly", "1", 0, 0, qfalse, qfalse },

	// RTCWPro
	{ &g_screenShake, "g_screenShake", "100", CVAR_ARCHIVE, 0, qtrue },
	{ &g_antiWarp, "g_antiWarp", "0", CVAR_LATCH, qtrue },
	{ &refereePassword, "refereePassword", "none", CVAR_ARCHIVE, 0, qfalse },
	{ &shoutcastPassword, "shoutcastPassword", "none", CVAR_ARCHIVE, 0, qfalse },
	{ &g_allowPMs, "g_allowPMs", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_crouchRate, "g_crouchRate", "5", CVAR_ARCHIVE, 0, qfalse },
	{ &g_mapConfigs, "g_mapConfigs", "0", CVAR_LATCH, 0, qfalse },
	{ &g_lifeStats, "g_lifeStats", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_ltNades, "g_ltNades", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_medicNades, "g_medicNades", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_engNades, "g_engNades", "8", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_soldNades, "g_soldNades", "4", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_medicClips, "g_medicClips", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_engineerClips, "g_engineerClips", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_soldierClips, "g_soldierClips", "2", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_leutClips, "g_leutClips", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_pistolClips, "g_pistolClips", "3", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_maxTeamPF, "g_maxTeamPF", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_maxTeamSniper, "g_maxTeamSniper", "-1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_maxTeamVenom, "g_maxTeamVenom", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_maxTeamFlamer, "g_maxTeamFlamer", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qtrue },
	{ &g_tournament, "g_tournament", "0", CVAR_ARCHIVE | CVAR_LATCH | CVAR_SERVERINFO, 0, qtrue },
	{ &g_dbgRevive, "g_dbgRevive", "0", 0, 0, qfalse },
	{ &g_dropWeapons, "g_dropWeapons", "9", CVAR_ARCHIVE, 0, qtrue, qtrue },
	{ &g_hsDamage, "g_hsDamage", "50", CVAR_ARCHIVE, 0, qfalse, qtrue },
	{ &g_pauseLimit, "g_pauseLimit", "3", CVAR_ARCHIVE, 0, qfalse, qfalse },
	{ &g_spawnOffset, "g_spawnOffset", "9", CVAR_ARCHIVE, 0, qfalse, qfalse },
	{ &g_bodiesGrabFlags, "g_bodiesGrabFlags", "1", CVAR_ARCHIVE, 0, qtrue },
	{ &g_mapScriptDirectory, "g_mapScriptDirectory", "", 0, qfalse },
	{ &g_damageRadiusKnockback, "g_damageRadiusKnockback", "1000", 0, 0, qtrue },
	{ &g_fixedphysicsfps, "g_fixedphysicsfps", "125", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_SYSTEMINFO, 0, qtrue },
	{ &g_alternatePing, "g_alternatePing", "1", CVAR_LATCH, qfalse },
	{ &g_allowForceTapout, "g_allowForceTapout", "1", CVAR_ARCHIVE, qtrue },
	{ &g_allowEnemySpawnTimer, "g_allowEnemySpawnTimer", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, qtrue },
	{ &g_clientLogFile, "g_clientLogFile", "logs/clientInput.log", CVAR_ARCHIVE, 0, qfalse },
	{ &g_logClientInput, "g_logClientInput", "0", CVAR_ARCHIVE, qtrue },
	{ &g_reviveSameDirection, "g_reviveSameDirection", "0", CVAR_ARCHIVE, 0, qfalse },
	//{ &g_thinkStateLevelTime, "g_thinkStateLevelTime", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	//{ &g_thinkSnapOrigin, "g_thinkSnapOrigin", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	//{ &g_endStateLevelTime, "g_endStateLevelTime", "1", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{&stats_matchid, "stats_matchid", "None", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse},
	{ &g_allowSS, "g_allowSS", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_ssAddress, "g_ssAddress", "none", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_ssWebhookId, "g_ssWebhookId", "none", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_ssWebhookToken, "g_ssWebhookToken", "none", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_ssWaitTime, "g_ssWaitTime", "30", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_broadcastClients, "g_broadcastClients", "0", CVAR_ARCHIVE | CVAR_LATCH, 0, qfalse },
	{ &g_logConfigStringChanges, "g_logConfigStringChanges", "0", CVAR_ARCHIVE, 0, qfalse },
	{ &g_playPauseMusic, "g_playPauseMusic", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &P, "P", "", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qfalse }, // ET Port Players server info
	{ &g_allowReadyTeam, "g_allowReadyTeam", "0", CVAR_ARCHIVE, 0, qfalse },// Allow readyteam command


	// unlagged
	{ &g_delagHitscan, "g_delagHitscan", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qtrue },
	{ &g_maxExtrapolatedFrames, "g_maxExtrapolatedFrames", "2", 0 , 0, qfalse },
	{ &g_maxLagCompensation, "g_maxLagCompensation", "125", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qtrue },
	{ &g_delagMissiles, "g_delagMissiles", "0", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qtrue }
};

// bk001129 - made static to avoid aliasing
static int gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[0] );

void G_InitGame( int levelTime, int randomSeed, int restart );
void G_RunFrame( int levelTime );
void G_ShutdownGame( int restart );
void CheckExitRules( void );

// Ridah, Cast AI
qboolean AICast_VisibleFromPos( vec3_t srcpos, int srcnum,
								vec3_t destpos, int destnum, qboolean updateVisPos );
qboolean AICast_CheckAttackAtPos( int entnum, int enemy, vec3_t pos, qboolean ducking, qboolean allowHitWorld );
void AICast_Init( void );
// done.

void G_RetrieveMoveSpeedsFromClient( int entnum, char *text );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
#if defined( __MACOS__ )
#pragma export on
#endif
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6 ) {
#if defined( __MACOS__ )
#pragma export off
#endif
	switch ( command ) {
	case GAME_INIT:
		G_InitGame( arg0, arg1, arg2 );
		return 0;
	case GAME_SHUTDOWN:
		G_ShutdownGame( arg0 );
		return 0;
	case GAME_CLIENT_CONNECT:
		return (int)ClientConnect( arg0, arg1, arg2 );
	case GAME_CLIENT_THINK:
		ClientThink( arg0 );
		return 0;
	case GAME_CLIENT_USERINFO_CHANGED:
		ClientUserinfoChanged( arg0 );
		return 0;
	case GAME_CLIENT_DISCONNECT:
		ClientDisconnect( arg0 );
		return 0;
	case GAME_CLIENT_BEGIN:
		ClientBegin( arg0 );
		return 0;
	case GAME_CLIENT_COMMAND:
		ClientCommand( arg0 );
		return 0;
	case GAME_RUN_FRAME:
		G_RunFrame( arg0 );
		return 0;
	case GAME_CONSOLE_COMMAND:
		return ConsoleCommand();
	case BOTAI_START_FRAME:
		return BotAIStartFrame( arg0 );
		// Ridah, Cast AI
	case AICAST_VISIBLEFROMPOS:
		return AICast_VisibleFromPos( (float *)arg0, arg1, (float *)arg2, arg3, arg4 );
	case AICAST_CHECKATTACKATPOS:
		return AICast_CheckAttackAtPos( arg0, arg1, (float *)arg2, arg3, arg4 );
		// done.

	case GAME_RETRIEVE_MOVESPEEDS_FROM_CLIENT:
		G_RetrieveMoveSpeedsFromClient( arg0, (char *)arg1 );
		return 0;

	case G_RETURN_API_QUERY_RESPONSE:
		trap_HandleApiResponse(arg0, (char*)arg1, arg2);
		return 0;
	}

	return -1;
}

void QDECL G_Printf( const char *fmt, ... ) {
	va_list argptr;
	char text[1024];

	va_start( argptr, fmt );
	Q_vsnprintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	trap_Printf( text );
}

void QDECL G_DPrintf( const char *fmt, ... ) {
	va_list argptr;
	char text[1024];

	if ( !g_developer.integer ) {
		return;
	}

	va_start( argptr, fmt );
	Q_vsnprintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	trap_Printf( text );
}

void QDECL G_Error( const char *fmt, ... ) {
	va_list argptr;
	char text[1024];

	va_start( argptr, fmt );
	Q_vsnprintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	trap_Error( text );
}


#define CH_KNIFE_DIST       48  // from g_weapon.c
#define CH_LADDER_DIST      100
#define CH_WATER_DIST       100
#define CH_BREAKABLE_DIST   64
#define CH_DOOR_DIST        96
#define CH_ACTIVATE_DIST    96
#define CH_EXIT_DIST        256

#define CH_MAX_DIST         256     // use the largest value from above
#define CH_MAX_DIST_ZOOM    8192    // max dist for zooming hints
/*
==============
G_CheckForCursorHints
	non-AI's check for cursor hint contacts

	server-side because there's info we want to show that the client
	just doesn't know about.  (health or other info of an explosive,invisible_users,items,etc.)

	traceEnt is the ent hit by the trace, checkEnt is the ent that is being
	checked against (in case the traceent was an invisible_user or something)

==============
*/
void G_CheckForCursorHints( gentity_t *ent ) {
	vec3_t forward, right, up, offset, end;
	trace_t     *tr;
	float dist;
	gentity_t   *checkEnt, *traceEnt = 0;
	//gentity_t *traceEnt2 = 0; // JPW NERVE // TTimo unused
	playerState_t *ps;
	int hintType, hintDist, hintVal;
	qboolean zooming, indirectHit;      // indirectHit means the checkent was not the ent hit by the trace (checkEnt!=traceEnt)
	int trace_contents;                 // DHM - Nerve

	// FIXME:	no need at all to do this trace/string comparison every frame.
	//			stagger it at least a little bit

//	if(!servercursorhints)
//		return;

	if ( !ent->client ) {
		return;
	}

	ps = &ent->client->ps;

	if ( ps->aiChar != AICHAR_NONE ) {
		return;
	}

	indirectHit = qfalse;

	zooming = (qboolean)( ps->eFlags & EF_ZOOMING );

	AngleVectors( ps->viewangles, forward, right, up );

	VectorCopy( ps->origin, offset );
	offset[2] += ps->viewheight;

	// lean
	if ( ps->leanf ) {
		VectorMA( offset, ps->leanf, right, offset );
	}

//	SnapVector( offset );

	if ( zooming ) {
		VectorMA( offset, CH_MAX_DIST_ZOOM, forward, end );
	} else {
		VectorMA( offset, CH_MAX_DIST, forward, end );
	}

	tr = &ps->serverCursorHintTrace;

	// DHM - Nerve
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		trace_contents = ( CONTENTS_TRIGGER | CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY );
	} else {
		trace_contents = ( CONTENTS_TRIGGER | CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY | CONTENTS_CORPSE );
	}

	trap_Trace( tr, offset, NULL, NULL, end, ps->clientNum, trace_contents );
	// dhm - end

	// reset all
	hintType    = ps->serverCursorHint      = HINT_NONE;
	hintVal     = ps->serverCursorHintVal   = 0;

	if ( zooming ) {
		dist        = tr->fraction * CH_MAX_DIST_ZOOM;
		hintDist    = CH_MAX_DIST_ZOOM;
	} else {
		dist        = tr->fraction * CH_MAX_DIST;
		hintDist    = CH_MAX_DIST;
	}

	if ( tr->fraction == 1 ) {
		return;
	}

	traceEnt = &g_entities[tr->entityNum];

	// DHM - Nerve :: Ignore trigger_objective_info
	if ( ( ps->stats[ STAT_PLAYER_CLASS ] != PC_MEDIC && traceEnt->client )
		 || !( strcmp( traceEnt->classname, "trigger_objective_info" ) )
		 || !( strcmp( traceEnt->classname, "trigger_multiple" ) ) ) {

		trap_Trace( tr, offset, NULL, NULL, end, traceEnt->s.number, trace_contents );
		if ( zooming ) {
			dist        = tr->fraction * CH_MAX_DIST_ZOOM;
			hintDist    = CH_MAX_DIST_ZOOM;
		} else {
			dist        = tr->fraction * CH_MAX_DIST;
			hintDist    = CH_MAX_DIST;
		}
		if ( tr->fraction == 1 ) {
			return;
		}
		traceEnt = &g_entities[tr->entityNum];
	}

	//
	// WORLD
	//
	if ( tr->entityNum == ENTITYNUM_WORLD ) {
		if ( ( tr->contents & CONTENTS_WATER ) && !( ps->powerups[PW_BREATHER] ) ) {
			hintDist = CH_WATER_DIST;
			hintType = HINT_WATER;
		}
		// ladder
		else if ( ( tr->surfaceFlags & SURF_LADDER ) && !( ps->pm_flags & PMF_LADDER ) ) {
			hintDist = CH_LADDER_DIST;
			hintType = HINT_LADDER;
		}
	}
	//
	// PEOPLE
	//
	else if ( tr->entityNum < MAX_CLIENTS ) {

		// DHM - Nerve
		if ( g_gametype.integer >= GT_WOLF ) {
			// Show medics a syringe if they can revive someone
			if ( traceEnt->client && traceEnt->client->sess.sessionTeam == ent->client->sess.sessionTeam
				 && ps->stats[ STAT_PLAYER_CLASS ] == PC_MEDIC
				 && traceEnt->client->ps.pm_type == PM_DEAD
				 && !( traceEnt->client->ps.pm_flags & PMF_LIMBO ) ) {
				hintDist    = 48;     // JPW NERVE matches weapon_syringe in g_weapon.c
				hintType    = HINT_REVIVE;
			}
		}
		// dhm - Nerve

	}
	//
	// OTHER ENTITIES
	//
	else {
		checkEnt = traceEnt;

		// check invisible_users first since you don't want to draw a hint based
		// on that ent, but rather on what they are targeting.
		// so find the target and set checkEnt to that to show the proper hint.
		if ( traceEnt->s.eType == ET_GENERAL ) {

			// ignore trigger_aidoor.  can't just not trace for triggers, since I need invisible_users...
			// damn, I would like to ignore some of these triggers though.

			if ( !Q_stricmp( traceEnt->classname, "trigger_aidoor" ) ) {
				return;
			}

			if ( !Q_stricmp( traceEnt->classname, "func_invisible_user" ) ) {
				indirectHit = qtrue;

				// DHM - Nerve :: Put this back in only in multiplayer
				if ( g_gametype.integer >= GT_WOLF && traceEnt->s.dmgFlags ) { // hint icon specified in entity
					hintType = traceEnt->s.dmgFlags;
					hintDist = CH_ACTIVATE_DIST;
					checkEnt = 0;
				} else {                      // use target for hint icon
					checkEnt = G_Find( NULL, FOFS( targetname ), traceEnt->target );
					if ( !checkEnt ) {     // no target found
						hintType = HINT_BAD_USER;
						hintDist = CH_MAX_DIST_ZOOM;    // show this one from super far for debugging
					}
				}
			}
		}


		if ( checkEnt ) {
			if ( checkEnt->s.eType == ET_GENERAL || checkEnt->s.eType == ET_MG42_BARREL ) {

				// this is effectively an 'exit' brush.  they should be created with:
				//
				// classname = 'ai_trigger'
				// ainame = 'player'
				// target = 'endmap'
				//
				if ( !Q_stricmp( traceEnt->classname, "ai_trigger" ) ) {
					if ( ( !Q_stricmp( traceEnt->aiName, "player" ) ) &&
						 ( !Q_stricmp( traceEnt->target, "endmap" ) ) ) {
						hintDist = CH_EXIT_DIST;
						hintType = HINT_EXIT;

						// show distance in the cursorhint bar
						hintVal = (int)dist;        // range for this hint is 256, so it happens to translate nicely
					}
				}

				if ( ( // general mg42 hint conditions
						 !Q_stricmp( traceEnt->classname, "misc_mg42" ) ) &&
					 ( ps->weapon != WP_SNIPERRIFLE ) && // JPW NERVE no hint if you're scoped in sniperwise
					 // ATVI #470
					 // mount hint conditions only, if busted MG42, no check
					 (
						 ( traceEnt->health < 255 ) ||
						 (
							 !( ent->client->ps.pm_flags & PMF_DUCKED ) &&
							 ( traceEnt->r.currentOrigin[2] - ent->r.currentOrigin[2] < 40 ) &&
							 ( traceEnt->r.currentOrigin[2] - ent->r.currentOrigin[2] > 0 ) &&
							 !infront( traceEnt, ent )
						 )
					 )
					 ) {

					hintDist = CH_ACTIVATE_DIST;
					hintType = HINT_MG42;

// JPW NERVE -- busted MG42 shouldn't show a use hint by default
					if ( traceEnt->health <= 0 ) {
						hintDist = 0;
						hintType = HINT_FORCENONE;
						ps->serverCursorHint = HINT_FORCENONE;
					}
// jpw

					// DHM - Nerve :: Engineers can repair turrets
					if ( g_gametype.integer >= GT_WOLF ) {
						if ( ps->stats[ STAT_PLAYER_CLASS ] == PC_ENGINEER ) {
							if ( !traceEnt->takedamage ) {
								hintType = HINT_BUILD;
								hintDist = CH_BREAKABLE_DIST;
								hintVal = traceEnt->health;
								if ( hintVal > 255 ) {
									hintVal = 255;
								}
							}
						}
					}
					// dhm - end
				}
			} else if ( checkEnt->s.eType == ET_EXPLOSIVE )      {
//				if(traceEnt->s.dmgFlags) {	 // override flag		// hint icon specified in entity, this overrides type
//					hintType = traceEnt->s.dmgFlags;
//				} else {
				if ( ( checkEnt->health > 0 ) || ( checkEnt->spawnflags & 64 ) ) {  // JPW NERVE they are now if it's dynamite			// 0 health explosives are not breakable
					hintDist    = CH_BREAKABLE_DIST;
					hintType    = HINT_BREAKABLE;

					// DHM - Nerve :: Show different hint if it can only be destroyed with dynamite
					if ( g_gametype.integer >= GT_WOLF && ( checkEnt->spawnflags & 64 ) ) {
// JPW NERVE only show hint for players who can blow it up
						vec3_t mins,maxs,range = { 40, 40, 52 };
						int i,num;
						//int defendingTeam=0; // TTimo unused
						int touch[MAX_GENTITIES];
						gentity_t *hit = NULL;

						VectorSubtract( ent->client->ps.origin, range, mins );
						VectorAdd( ent->client->ps.origin, range, maxs );
						num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );
						for ( i = 0 ; i < num ; i++ ) {
							hit = &g_entities[touch[i]];
							if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
								continue;
							}
							if ( !strcmp( hit->classname,"trigger_objective_info" ) ) {
								if ( !( hit->spawnflags & ( AXIS_OBJECTIVE | ALLIED_OBJECTIVE ) ) ) {
									continue;
								}
// we're in a trigger_objective_info field with at least one owner, so use this one and bail
								break;
							}
						}
						if ( ( hit ) &&
							 ( ( ( ent->client->sess.sessionTeam == TEAM_RED ) && ( hit->spawnflags & ALLIED_OBJECTIVE ) ) ||
							   ( ( ent->client->sess.sessionTeam == TEAM_BLUE ) && ( hit->spawnflags & AXIS_OBJECTIVE ) ) )
							 ) {
							hintDist = CH_BREAKABLE_DIST * 2;
							hintType = HINT_BREAKABLE_DYNAMITE;
						} else {
							hintDist = 0;
							hintType = ps->serverCursorHint     = HINT_FORCENONE;
							hintVal     = ps->serverCursorHintVal   = 0;
							return;
						}
// jpw
					}
					// dhm - end

					hintVal     = checkEnt->health;         // also send health to client for visualization
				}
//				}
			} else if ( checkEnt->s.eType == ET_ALARMBOX )      {
				if ( checkEnt->health > 0 ) {
//					hintDist	= CH_BREAKABLE_DIST;
					hintType    = HINT_ACTIVATE;
				}
			} else if ( checkEnt->s.eType == ET_ITEM )      {
				gitem_t *it;
				it = &bg_itemlist[checkEnt->item - bg_itemlist];

				hintDist = CH_ACTIVATE_DIST;

				switch ( it->giType ) {
				case IT_HEALTH:
					hintType = HINT_HEALTH;
					break;
				case IT_TREASURE:
					hintType = HINT_TREASURE;
					break;
				case IT_CLIPBOARD:
					hintType = HINT_CLIPBOARD;
					break;
				case IT_WEAPON:
// JPW NERVE changed this to be more specific
					if ( it->giTag != WP_LUGER && it->giTag != WP_COLT ) {
						if ( ps->stats[STAT_PLAYER_CLASS] == PC_SOLDIER ) {   // soldier can pick up all weapons
							hintType = HINT_WEAPON;
							break;
						}
						if ( ps->stats[STAT_PLAYER_CLASS] != PC_LT ) { // medics & engrs can't pick up squat
							break;
						}
						if ( it->giTag == WP_THOMPSON || it->giTag == WP_MP40 || it->giTag == WP_STEN ) {
							hintType = HINT_WEAPON;
						}
					}
// jpw
					break;
				case IT_AMMO:
					hintType = HINT_AMMO;
					break;
				case IT_ARMOR:
					hintType = HINT_ARMOR;
					break;
				case IT_POWERUP:
					hintType = HINT_POWERUP;
					break;
				case IT_HOLDABLE:
					hintType = HINT_HOLDABLE;
					break;
				case IT_KEY:
					hintType = HINT_INVENTORY;
					break;
				case IT_TEAM:
					if ( !Q_stricmp( traceEnt->classname, "team_CTF_redflag" ) && ent->client->sess.sessionTeam == TEAM_BLUE ) {
						hintType = HINT_POWERUP;
					} else if ( !Q_stricmp( traceEnt->classname, "team_CTF_blueflag" ) && ent->client->sess.sessionTeam == TEAM_RED ) {
						hintType = HINT_POWERUP;
					}
					break;
				case IT_BAD:
				default:
					break;
				}
			} else if ( checkEnt->s.eType == ET_MOVER )     {
				if ( !Q_stricmp( checkEnt->classname, "func_door_rotating" ) ) {
					if ( checkEnt->moverState == MOVER_POS1ROTATE ) {  // stationary/closed
						hintDist = CH_DOOR_DIST;
						hintType = HINT_DOOR_ROTATING;

						if ( checkEnt->key == -1 ) {   // locked
							//						hintType = HINT_DOOR_ROTATING_LOCKED;
						}
					}
				} else if ( !Q_stricmp( checkEnt->classname, "func_door" ) )         {
					if ( checkEnt->moverState == MOVER_POS1 ) {    // stationary/closed
						hintDist = CH_DOOR_DIST;
						hintType = HINT_DOOR;

						if ( checkEnt->key == -1 ) {   // locked
							//						hintType = HINT_DOOR_LOCKED;
						}
					}
				} else if ( !Q_stricmp( checkEnt->classname, "func_button" ) )         {
					hintDist = CH_ACTIVATE_DIST;
					hintType = HINT_BUTTON;
				}/*
				else if(checkEnt->s.dmgFlags == HINT_CHAIR) {
					hintDist = CH_ACTIVATE_DIST;
					hintType = HINT_CHAIR;
				}*/
			}

			// DHM - Nerve :: Handle wolf multiplayer hints
			if ( g_gametype.integer >= GT_WOLF ) {

				if ( checkEnt->s.eType == ET_MISSILE ) {
					if ( ps->stats[ STAT_PLAYER_CLASS ] == PC_ENGINEER ) {
						hintDist    = CH_BREAKABLE_DIST;
						hintType    = HINT_DISARM;
						hintVal     = checkEnt->health;     // also send health to client for visualization
						if ( hintVal > 255 ) {
							hintVal = 255;
						}
					}
				}

			}
			// dhm - end

			// hint icon specified in entity (and proper contact was made, so hintType was set)
			// first try the checkent...
			if ( checkEnt->s.dmgFlags && hintType ) {
				hintType = checkEnt->s.dmgFlags;
			}
		}

		// then the traceent
		if ( traceEnt->s.dmgFlags && hintType ) {
			hintType = traceEnt->s.dmgFlags;
		}

	}

	if ( zooming ) {

		hintDist = CH_MAX_DIST_ZOOM;

		// zooming can eat a lot of potential hints
		switch ( hintType ) {

			// allow while zooming
		case HINT_PLAYER:
		case HINT_TREASURE:
		case HINT_LADDER:
		case HINT_EXIT:
		case HINT_PLYR_FRIEND:
		case HINT_PLYR_NEUTRAL:
		case HINT_PLYR_ENEMY:
		case HINT_PLYR_UNKNOWN:
			break;

		default:
			return;
		}
	}

	if ( dist <= hintDist ) {
		ps->serverCursorHint = hintType;
		ps->serverCursorHintVal = hintVal;
	}


//	Com_Printf("hint: %d\n", ps->serverCursorHint);
}


/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void ) {
	gentity_t   *e, *e2;
	int i, j;
	int c, c2;

	c = 0;
	c2 = 0;
	for ( i = 1, e = g_entities + i ; i < level.num_entities ; i++,e++ ) {
		if ( !e->inuse ) {
			continue;
		}

		if ( !e->team ) {
			continue;
		}

		if ( e->flags & FL_TEAMSLAVE ) {
			continue;
		}

		if ( !Q_stricmp( e->classname, "func_tramcar" ) ) {
			if ( e->spawnflags & 8 ) { // leader
				e->teammaster = e;
			} else
			{
				continue;
			}
		}

		c++;
		c2++;
		for ( j = i + 1, e2 = e + 1 ; j < level.num_entities ; j++,e2++ )
		{
			if ( !e2->inuse ) {
				continue;
			}
			if ( !e2->team ) {
				continue;
			}
			if ( e2->flags & FL_TEAMSLAVE ) {
				continue;
			}
			if ( !strcmp( e->team, e2->team ) ) {
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
//				e2->key = e->key;	// (SA) I can't set the key here since the master door hasn't finished spawning yet and therefore has a key of -1
				e2->flags |= FL_TEAMSLAVE;

				if ( !Q_stricmp( e2->classname, "func_tramcar" ) ) {
					trap_UnlinkEntity( e2 );
				}

				// make sure that targets only point at the master
				if ( e2->targetname ) {
					e->targetname = e2->targetname;

					// Rafael
					// note to self: added this because of problems
					// pertaining to keys and double doors
					if ( Q_stricmp( e2->classname, "func_door_rotating" ) ) {
						e2->targetname = NULL;
					}
				}
			}
		}
	}

	if ( trap_Cvar_VariableIntegerValue( "g_gametype" ) != GT_SINGLE_PLAYER ) {
		G_Printf( "%i teams with %i entities\n", c, c2 );
	}
}


/*
==============
G_RemapTeamShaders
==============
*/
void G_RemapTeamShaders() {
#ifdef MISSIONPACK
	char string[1024];
	float f = level.time * 0.001;
	Com_sprintf( string, sizeof( string ), "team_icon/%s_red", g_redteam.string );
	AddRemap( "textures/ctf2/redteam01", string, f );
	AddRemap( "textures/ctf2/redteam02", string, f );
	Com_sprintf( string, sizeof( string ), "team_icon/%s_blue", g_blueteam.string );
	AddRemap( "textures/ctf2/blueteam01", string, f );
	AddRemap( "textures/ctf2/blueteam02", string, f );
	trap_SetConfigstring( CS_SHADERSTATE, BuildShaderStateConfig() );
#endif
}

/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars( void ) {
	int i;
	cvarTable_t *cv;
	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
							cv->defaultString, cv->cvarFlags );
		if ( cv->vmCvar ) {
			cv->modificationCount = cv->vmCvar->modificationCount;
            // update vote info for clients, if necessary
			G_checkServerToggle(cv->vmCvar);
		}

		if ( cv->teamShader ) {
			remapped = qtrue;
		}
	}

	if ( remapped ) {
		G_RemapTeamShaders();
	}

	// check some things
	// DHM - Gametype is currently restricted to supported types only
	if ( g_gametype.integer < GT_WOLF || g_gametype.integer > GT_WOLF_CPH ) { // JPW NERVE WOLF_CPH now highest type
		G_Printf( "g_gametype %i is out of range, defaulting to GT_WOLF(5)\n", g_gametype.integer );
		trap_Cvar_Set( "g_gametype", "5" );
		trap_Cvar_Update( &g_gametype );
	}

	// Rafael gameskill
	if ( g_gameskill.integer < GSKILL_EASY || g_gameskill.integer > GSKILL_VERYHARD ) {
		G_Printf( "g_gameskill %i is out of range, default to medium\n", g_gameskill.integer );
		trap_Cvar_Set( "g_gameskill", "3" ); // default to medium
	}

	bg_pmove_gameskill_integer = g_gameskill.integer;
	// done

	level.warmupModificationCount = g_warmup.modificationCount;

	trap_SetConfigstring(CS_SERVERTOGGLES, va("%d", level.server_settings));

	// OSPx
	// Ready percents
	if (match_readypercent.integer < 1) {
		trap_Cvar_Set("match_readypercent", "1");
	}
	else if (match_readypercent.integer > 100) {
		trap_Cvar_Set("match_readypercent", "1");
	}

	// Sanity check (clamps)
	if (pmove_msec.integer < 8) {
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) {
		trap_Cvar_Set("pmove_msec", "33");
	}
	if (match_timeoutcount.integer > 999) {
		trap_Cvar_Set("match_timeoutcount", "999");
	}

	if (g_ssWaitTime.integer < 30)
	{
		G_Printf("g_ssWaitTime %i is out of range, defaulting to 30\n", g_ssWaitTime.integer);
		trap_Cvar_Set("g_ssWaitTime", "30");
	}
	// -OSPx
}




/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars( void ) {
	int i;
	cvarTable_t *cv;
	qboolean remapped = qfalse;
	qboolean    fToggles           = qfalse;
	qboolean    fVoteFlags         = qfalse;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) {
		if ( cv->vmCvar ) {
			trap_Cvar_Update( cv->vmCvar );

			if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) {
					trap_SendServerCommand( -1, va( "print \"Server:[lof] %s [lon]changed to[lof] %s\n\"",
													cv->cvarName, cv->vmCvar->string ) );
				}

				if ( cv->teamShader ) {
					remapped = qtrue;
				}
				// OSPx
				if (cv->vmCvar == &match_readypercent) {
					if (match_readypercent.integer < 1) {
						trap_Cvar_Set(cv->cvarName, "1");
					}
					else if (match_readypercent.integer > 100) {
						trap_Cvar_Set(cv->cvarName, "100");
					}
				} else if (cv->vmCvar == &pmove_msec) {
					if (pmove_msec.integer < 8) {
						trap_Cvar_Set(cv->cvarName, "8");
					}
					else if (pmove_msec.integer > 33) {
						trap_Cvar_Set(cv->cvarName, "33");
					}
				} // -OSPx

				// cvar toggle
// Update vote info for clients, if necessary
				if (cv->vmCvar == &vote_allow_kick            || cv->vmCvar == &vote_allow_map            ||
				    cv->vmCvar == &vote_allow_matchreset      || cv->vmCvar == &vote_allow_gametype       ||
				    cv->vmCvar == &vote_allow_mutespecs       || cv->vmCvar == &vote_allow_nextmap        ||
				    cv->vmCvar == &vote_allow_referee        ||
				    cv->vmCvar == &vote_allow_swapteams       || cv->vmCvar == &vote_allow_friendlyfire   ||
				    cv->vmCvar == &vote_allow_timelimit       || cv->vmCvar == &vote_allow_warmupdamage   ||
				    cv->vmCvar == &vote_allow_antilag         || cv->vmCvar == &vote_allow_balancedteams  ||
				    cv->vmCvar == &vote_allow_muting             )
				{
					fVoteFlags = qtrue;
				}
				else
				{
					fToggles = (G_checkServerToggle(cv->vmCvar) || fToggles);
				}

				if (cv->vmCvar == &g_antiWarp)
				{
					if (g_antiWarp.integer)
					{
						trap_Cvar_Set("g_synchronousClients", "0");
					}
				}

				if (cv->vmCvar == &g_spawnOffset) 
				{
					if (g_spawnOffset.integer < 1)
					{
						G_Printf("g_spawnOffset %i is out of range, defaulting to 9\n", g_spawnOffset.integer);
						trap_Cvar_Set("g_spawnOffset", "9");
					}
				}

				if (cv->vmCvar == &g_ssWaitTime)
				{
					if (g_ssWaitTime.integer < 30)
					{
						G_Printf("g_ssWaitTime %i is out of range, defaulting to 30\n", g_ssWaitTime.integer);
						trap_Cvar_Set("g_ssWaitTime", "30");
					}
				}
			}
		}
	}

	if (fVoteFlags)
	{
		G_voteFlags();
	}

	if (fToggles)
	{
		trap_SetConfigstring(CS_SERVERTOGGLES, va("%d", level.server_settings));
	}
// end cvar toggle
	if ( remapped ) {
		G_RemapTeamShaders();
	}
}

/*
==============
RTCWPro - load the list of maps on
the server into an array
Source: PubJ (nihi)

LoadMapList
==============
*/
void LoadMapList(void)
{
	char maps[MAX_MAPCONFIGSTRINGS] = {'\0'};
	char noext[MAX_QPATH] = {'\0'};
	int i;

	level.mapcount = trap_FS_GetFileList("maps", ".bsp", maps, sizeof(maps));
	char* map = maps;

    for (i = 0; i <= level.mapcount+1; i++) {
		COM_StripExtension(map, noext);
		strcpy(level.maplist[i],noext);
		map += strlen(map) + 1;
	}
}

/*
==============
G_SpawnScriptCamera
	create the game entity that's used for camera<->script communication and portal location for camera view
==============
*/
void G_SpawnScriptCamera( void ) {
	if ( g_camEnt ) {
		G_FreeEntity( g_camEnt );
	}

	g_camEnt = G_Spawn();

	g_camEnt->scriptName = "scriptcamera";

	g_camEnt->s.eType = ET_CAMERA;
	g_camEnt->s.apos.trType = TR_STATIONARY;
	g_camEnt->s.apos.trTime = 0;
	g_camEnt->s.apos.trDuration = 0;
	VectorCopy( g_camEnt->s.angles, g_camEnt->s.apos.trBase );
	VectorClear( g_camEnt->s.apos.trDelta );

	g_camEnt->s.frame = 0;

	g_camEnt->r.svFlags |= SVF_NOCLIENT;        // only broadcast when in use

	if ( g_camEnt->s.number >= MAX_CLIENTS && g_camEnt->scriptName ) {
		G_Script_ScriptParse( g_camEnt );
		G_Script_ScriptEvent( g_camEnt, "spawn", "" );
	}

}

/*
============
G_InitGame

============
*/
void G_InitGame( int levelTime, int randomSeed, int restart ) {
	int i;
	char cs[MAX_INFO_STRING];
	char mapName[MAX_QPATH];

	if ( trap_Cvar_VariableIntegerValue( "g_gametype" ) != GT_SINGLE_PLAYER ) {
		G_Printf( "------- Game Initialization -------\n" );
		G_Printf( "gamename: %s\n", GAMEVERSION );
		G_Printf( "gamedate: %s\n", __DATE__ );
	}

	srand( randomSeed );

	//load a file inside the pk3s to become referenced paks for pure check
	trap_FS_FOpenFile("rtcwpro_models.dat", &i, FS_READ);
	trap_FS_FCloseFile(i);

	trap_FS_FOpenFile("rtcwpro_assets.dat", &i, FS_READ);
	trap_FS_FCloseFile(i);

	trap_FS_FOpenFile("rtcwpro_xhair.dat", &i, FS_READ);
	trap_FS_FCloseFile(i);

	G_RegisterCvars();

	// Xian enforcemaxlives stuff
	/*
	we need to clear the list even if enforce maxlives is not active
	in cas ethe g_maxlives was changed, and a map_restart happened
	*/

/*

	ClearMaxLivesGUID();

	// just for verbosity
	if ( g_enforcemaxlives.integer && ( g_maxlives.integer > 0 || g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0 ) ) {
		G_Printf( "EnforceMaxLives-Cleared GUID List\n" );
	}

//	G_ProcessIPBans();
*/
	G_InitMemory();

	// NERVE - SMF - intialize gamestate
	if ( g_gamestate.integer == GS_INITIALIZE ) {
		if ( g_noTeamSwitching.integer ) {
			if (g_tournament.integer) {// L0 - Ready
				trap_Cvar_Set( "gamestate", va( "%i", GS_WARMUP ) );
				trap_SetConfigstring( CS_READY, va( "%i", READY_PENDING ) );
				level.ref_allready = qfalse;
			} else {
				trap_Cvar_Set( "gamestate", va( "%i", GS_WAITING_FOR_PLAYERS ) );
			}
		} else {
			trap_Cvar_Set( "gamestate", va( "%i", GS_WARMUP ) );
		}
	}

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;

	level.snd_fry = G_SoundIndex( "sound/player/fry.wav" );    // FIXME standing in lava / slime
	level.bulletRicochetSound = G_SoundIndex( "bulletRicochet" );
	level.snipersound = G_SoundIndex( "sound/weapons/mauser/mauserf1.wav" );
	level.knifeSound[0] = G_SoundIndex( "sound/weapons/knife/knife_hitwall1.wav" );

	// RF, init the anim scripting
	level.animScriptData.soundIndex = G_SoundIndex;
	level.animScriptData.playSound = G_AnimScriptSound;

	if ( g_gametype.integer != GT_SINGLE_PLAYER && g_log.string[0] ) {
		if ( g_logSync.integer ) {
			trap_FS_FOpenFile( g_log.string, &level.logFile, FS_APPEND_SYNC );
		} else {
			trap_FS_FOpenFile( g_log.string, &level.logFile, FS_APPEND );
		}
		if ( !level.logFile ) {
			G_Printf( "WARNING: Couldn't open logfile: %s\n", g_log.string );
		} else {
			char serverinfo[MAX_INFO_STRING];

			trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

			G_LogPrintf( "------------------------------------------------------------\n" );
			G_LogPrintf( "InitGame: %s\n", serverinfo );
		}
        if (g_gameStatslog.integer && (g_gamestate.integer == GS_PLAYING)) { // definitely needs improving but here for testing purposes
                char newGamestatFile[256];
                char mapName[64];
                char buf2[64];
                int retval;
                qtime_t ct;
                trap_RealTime(&ct);
                trap_Cvar_VariableStringBuffer( "mapname", mapName, sizeof(mapName) );
                char *buf;
                time_t unixTime = time(NULL);  // come back and make globally available
                //char cs[MAX_STRING_CHARS];

				// reset disconnect stats for each half round
				memset(level.disconnectStats, 0, sizeof(level.disconnectStats));
				level.disconnectCount = 0;

                // we want to save some information for the match and round
                if (g_currentRound.integer == 1) {

					retval = G_read_round_jstats(); // load stats from round 1

                    Q_strncpyz(level.jsonStatInfo.round_id,"2",sizeof(level.jsonStatInfo.round_id) );
                    if (retval == 0) {
                        buf=va("%ld", unixTime);
                        trap_Cvar_Set( "stats_matchid", buf);

                    }
                    else {
                        trap_Cvar_VariableStringBuffer("stats_matchid",buf2,sizeof(buf2));
                        buf = va("%s",buf2);
                    }
					//buf = va("%s",level.match_id);

					Q_strncpyz(level.jsonStatInfo.match_id,buf,sizeof(level.jsonStatInfo.match_id) );
                }
                else {
                    	for ( i = 0; i < level.numConnectedClients; i++ ) {
                            G_deleteStats( level.sortedClients[i] );
                        }



                     buf=va("%ld", unixTime);
                     trap_Cvar_Set( "stats_matchid", buf);
                     //level.match_id = va("%s",buf);
                     Q_strncpyz(level.jsonStatInfo.round_id,"1",sizeof(level.jsonStatInfo.round_id) );
                     Q_strncpyz(level.jsonStatInfo.match_id,buf,sizeof(level.jsonStatInfo.match_id) );
                }

                G_write_match_info();


                Com_sprintf( newGamestatFile, sizeof( newGamestatFile ), "stats/%d_%d_%d/gameStats_match_%s_round_%d_%s.json", ct.tm_mday, ct.tm_mon+1, 1900+ct.tm_year, buf,g_currentRound.integer+1,mapName);
                trap_FS_FOpenFile( va("stats/%d_%d_%d/gameStats_match_%s_round_%d_%s.json", ct.tm_mday, ct.tm_mon+1, 1900+ct.tm_year, buf,g_currentRound.integer+1,mapName), &level.jsonStatInfo.gameStatslogFile, FS_WRITE );
                if ( !level.jsonStatInfo.gameStatslogFile ) {
                    G_Printf( "WARNING: Couldn't open gameStatlogfile: %s\n", newGamestatFile );
                } else {

                    char hpath[256];
                    char game[60];
                    trap_Cvar_VariableStringBuffer( "fs_homepath", hpath, sizeof( hpath ) );
                    trap_Cvar_VariableStringBuffer( "fs_game", game, sizeof( game ) );

                    Com_sprintf( level.jsonStatInfo.gameStatslogFileName, sizeof( level.jsonStatInfo.gameStatslogFileName ), "%s/%s/stats/%d_%d_%d/gameStats_match_%s_round_%d_%s.json", hpath, game,ct.tm_mday, ct.tm_mon+1, 1900+ct.tm_year, buf,g_currentRound.integer+1,mapName);

                    G_writeServerInfo();


                }

            }

	} else {
		if ( trap_Cvar_VariableIntegerValue( "g_gametype" ) != GT_SINGLE_PLAYER ) {
			G_Printf( "Not logging to disk.\n" );
		}
	}


	G_InitWorldSession();

	// DHM - Nerve :: Clear out spawn target config strings
	if ( g_gametype.integer >= GT_WOLF ) {

		trap_GetConfigstring( CS_MULTI_INFO, cs, sizeof( cs ) );

		if (g_logConfigStringChanges.integer)
			LogEntry("logs/configStrings.log", va("Round: [ %d ] Location: [ G_InitGame ] CS Before: [ %s ] variable: [ numspawntargets ]\n", g_currentRound.integer + 1, cs));

		Info_SetValueForKey( cs, "numspawntargets", "0" );
		trap_SetConfigstring( CS_MULTI_INFO, cs );

		if (g_logConfigStringChanges.integer)
			LogEntry("logs/configStrings.log", va("Round: [ %d ] Location: [ G_InitGame ] CS After: [ %s ] variable: [ numspawntargets ]\n", g_currentRound.integer + 1, cs));

		for ( i = CS_MULTI_SPAWNTARGETS; i < CS_MULTI_SPAWNTARGETS + MAX_MULTI_SPAWNTARGETS; i++ ) {
			trap_SetConfigstring( i, "" );
		}
	}

	// initialize all entities for this game
	memset( g_entities, 0, MAX_GENTITIES * sizeof( g_entities[0] ) );
	level.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset( g_clients, 0, MAX_CLIENTS * sizeof( g_clients[0] ) );
	level.clients = g_clients;

	// set client fields on player ents
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		g_entities[i].client = level.clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	// let the server system know where the entites are
	trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ),
						 &level.clients[0].ps, sizeof( level.clients[0] ) );

	// load level script
	G_Script_ScriptLoad();

	// reserve some spots for dead player bodies
	InitBodyQue();

	ClearRegisteredItems();

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString();

	// create the camera entity that will communicate with the scripts
	G_SpawnScriptCamera();

	// general initialization
	G_FindTeams();

	// make sure we have flags for CTF, etc
	if ( g_gametype.integer >= GT_TEAM ) {
		G_CheckTeamItems();
	}

	SaveRegisteredItems();

	if ( trap_Cvar_VariableIntegerValue( "g_gametype" ) != GT_SINGLE_PLAYER ) {
		G_Printf( "-----------------------------------\n" );
	}

    G_loadMatchGame();

	GeoIP_open();

	// L0 - auto cfg for each map
	if (!restart && g_mapConfigs.integer){
		//char mapName[64];

		trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));

		for (int x = 0; mapName[x]; x++) {
			mapName[x] = tolower(mapName[x]);
		}

		trap_SendConsoleCommand(EXEC_APPEND, va("exec mapConfigs/%s.cfg \n", mapName));
	} // L0 - end
	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAISetup( restart );
		BotAILoadMap( restart );
		G_InitBots( restart );
	}

	G_RemapTeamShaders();

	// L0 - Pause
	// Start with off! As if map_restart occur while paused screen fade is stuck..
	// Disconnect while paused is handled in client side.
	trap_SetConfigstring( CS_PAUSED,  va( "%i", PAUSE_NONE ));

	LoadMapList();

	// L0 - Clamp stuff if needed
	// TODO: Move this if into it's own function if more is introduced (i.e. Duel mode..)
	{
		char* info = NULL;

		if (g_medicClips.integer > 18) {
			trap_Cvar_Set("g_medicClips", "18");
			info = "g_medicClips - Clamping to 18\n";
		}

		if (g_engineerClips.integer > 18) {
			trap_Cvar_Set("g_engineerClips", "18");
			info = va("%sg_engineerClips - Clamping to 18\n", info);
		}

		if (g_soldierClips.integer > 18) {
			trap_Cvar_Set("g_soldierClips", "18");
			info = va("%sg_soldierClips - Clamping to 18\n", info);
		}

		if (g_leutClips.integer > 18) {
			trap_Cvar_Set("g_leutClips", "18");
			info = va("%sg_leutClips - Clamping to 18\n", info);
		}

		if (info) {
			G_Printf(
				"Following Cvars are set too high:\n%s-----------------------------------\n", info
			);
		}

		// These sometimes goes off so make sure..
		if (g_tournament.integer && g_gamestate.integer == GS_PLAYING)
		{
			teamInfo[TEAM_RED].timeouts = match_timeoutcount.integer;
			teamInfo[TEAM_RED].team_lock = qtrue;

			teamInfo[TEAM_BLUE].timeouts = match_timeoutcount.integer;
			teamInfo[TEAM_BLUE].team_lock = qtrue;
		}
	}
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) {
	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		G_Printf( "==== ShutdownGame ====\n" );
	}

	if ( level.logFile ) {
		G_LogPrintf( "ShutdownGame:\n" );
		G_LogPrintf( "------------------------------------------------------------\n" );
		trap_FS_FCloseFile( level.logFile );
	}
	if (level.jsonStatInfo.gameStatslogFile) {
        // we may want to put some closing information into the gamestat file...
        trap_FS_FCloseFile( level.jsonStatInfo.gameStatslogFile);
	}

	// Ridah, shutdown the Botlib, so weapons and things get reset upon doing a "map xxx" command
	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		int i;

		// Ridah, kill AI cast's
		for ( i = 0 ; i < g_maxclients.integer ; i++ ) {
			if ( g_entities[i].r.svFlags & SVF_CASTAI ) {
				trap_DropClient( i, "Drop Cast AI" );
			}
		}
		// done.
	}
	// done.
	// OSPx - Country Flags
	GeoIP_close();

	// write all the client session data so we can get it back
	G_WriteSessionData();


	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAIShutdown( restart );
	}
}



//===================================================================

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list argptr;
	char text[1024];

	va_start( argptr, error );
	Q_vsnprintf( text, sizeof( text ), error, argptr );
	va_end( argptr );

	G_Error( "%s", text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list argptr;
	char text[1024];

	va_start( argptr, msg );
	Q_vsnprintf( text, sizeof( text ), msg, argptr );
	va_end( argptr );

	G_Printf( "%s", text );
}

#endif

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
AddTournamentPlayer

If there are less than two tournament players, put a
spectator in the game and restart
=============
*/
void AddTournamentPlayer( void ) {
	int i;
	gclient_t   *client;
	gclient_t   *nextInLine;

	if ( level.numPlayingClients >= 2 ) {
		return;
	}

	// never change during intermission
	if ( level.intermissiontime ) {
		return;
	}

	nextInLine = NULL;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		// never select the dedicated follow or scoreboard clients
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ||
			 client->sess.spectatorClient < 0  ) {
			continue;
		}

		if ( !nextInLine || client->sess.spectatorTime < nextInLine->sess.spectatorTime ) {
			nextInLine = client;
		}
	}

	if ( !nextInLine ) {
		return;
	}

	level.warmupTime = -1;

	// set them to free-for-all team
	SetTeam( &g_entities[ nextInLine - level.clients ], "f" ,qfalse);
}

/*
=======================
RemoveTournamentLoser

Make the loser a spectator at the back of the line
=======================
*/
void RemoveTournamentLoser( void ) {
	int clientNum;

	if ( level.numPlayingClients != 2 ) {
		return;
	}

	clientNum = level.sortedClients[1];

	if ( level.clients[ clientNum ].pers.connected != CON_CONNECTED ) {
		return;
	}

	// make them a spectator
	SetTeam( &g_entities[ clientNum ], "s",qfalse );
}


/*
=======================
AdjustTournamentScores

=======================
*/
void AdjustTournamentScores( void ) {
	int clientNum;

	clientNum = level.sortedClients[0];
	if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
		level.clients[ clientNum ].sess.wins++;
		ClientUserinfoChanged( clientNum );
	}

	clientNum = level.sortedClients[1];
	if ( level.clients[ clientNum ].pers.connected == CON_CONNECTED ) {
		level.clients[ clientNum ].sess.losses++;
		ClientUserinfoChanged( clientNum );
	}

}

/*
=============
SortRanks

=============
*/
int QDECL SortRanks( const void *a, const void *b ) {
	gclient_t   *ca, *cb;

	ca = &level.clients[*(int *)a];
	cb = &level.clients[*(int *)b];

	// sort special clients last
	if ( ca->sess.spectatorState == SPECTATOR_SCOREBOARD || ca->sess.spectatorClient < 0 ) {
		return 1;
	}
	if ( cb->sess.spectatorState == SPECTATOR_SCOREBOARD || cb->sess.spectatorClient < 0  ) {
		return -1;
	}

	// then connecting clients
	if ( ca->pers.connected == CON_CONNECTING ) {
		return 1;
	}
	if ( cb->pers.connected == CON_CONNECTING ) {
		return -1;
	}


	// then spectators
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR && cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( ca->sess.spectatorTime < cb->sess.spectatorTime ) {
			return -1;
		}
		if ( ca->sess.spectatorTime > cb->sess.spectatorTime ) {
			return 1;
		}
		return 0;
	}
	if ( ca->sess.sessionTeam == TEAM_SPECTATOR ) {
		return 1;
	}
	if ( cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		return -1;
	}

	// then sort by score
	if ( ca->ps.persistant[PERS_SCORE]
		 > cb->ps.persistant[PERS_SCORE] ) {
		return -1;
	}
	if ( ca->ps.persistant[PERS_SCORE]
		 < cb->ps.persistant[PERS_SCORE] ) {
		return 1;
	}
	return 0;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks( void ) {
	int i;
	int rank;
	int score;
	int newScore;
	gclient_t   *cl;

	level.follow1 = -1;
	level.follow2 = -1;
	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.voteInfo.numVotingClients = 0;     // don't count bots

	level.numFinalDead[0] = 0;      // NERVE - SMF
	level.numFinalDead[1] = 0;      // NERVE - SMF

	level.voteInfo.numVotingTeamClients[ 0 ] = 0;
	level.voteInfo.numVotingTeamClients[ 1 ] = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected != CON_DISCONNECTED ) {
			level.sortedClients[level.numConnectedClients] = i;
			level.numConnectedClients++;

			if ( level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ) {
				level.numNonSpectatorClients++;

				// decide if this should be auto-followed
				if ( level.clients[i].pers.connected == CON_CONNECTED ) {

					level.numPlayingClients++;

					if (!(g_entities[i].r.svFlags & SVF_BOT)) {
						level.voteInfo.numVotingClients++; // if not a bot they are a voting client
					}

					if ( level.clients[i].sess.sessionTeam == TEAM_RED ) {
						// NERVE - SMF
						if ( level.clients[i].ps.persistant[PERS_RESPAWNS_LEFT] == 0
								&& g_entities[i].health <= 0 ) {
							level.numFinalDead[0]++;
						}

						if (!(g_entities[i].r.svFlags & SVF_BOT)) { // if not a bot they are a team voting client
							level.voteInfo.numVotingTeamClients[0]++;
						}

					} else if ( level.clients[i].sess.sessionTeam == TEAM_BLUE ) {
						// NERVE - SMF
						if ( level.clients[i].ps.persistant[PERS_RESPAWNS_LEFT] == 0
								&& g_entities[i].health <= 0 ) {
							level.numFinalDead[1]++;
						}

						if (!(g_entities[i].r.svFlags & SVF_BOT)) { // if not a bot they are a team voting client
							level.voteInfo.numVotingTeamClients[1]++;
						}
					}

					if ( level.follow1 == -1 ) {
						level.follow1 = i;
					} else if ( level.follow2 == -1 ) {
						level.follow2 = i;
					}
				}
			}
		}
	}

	qsort( level.sortedClients, level.numConnectedClients,
		   sizeof( level.sortedClients[0] ), SortRanks );

	// set the rank value for all clients that are connected and not spectators
	if ( g_gametype.integer >= GT_TEAM ) {
		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for ( i = 0;  i < level.numConnectedClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			if ( level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 2;
			} else if ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] ) {
				cl->ps.persistant[PERS_RANK] = 0;
			} else {
				cl->ps.persistant[PERS_RANK] = 1;
			}
		}
	} else {
		rank = -1;
		score = 0;
		for ( i = 0;  i < level.numPlayingClients; i++ ) {
			cl = &level.clients[ level.sortedClients[i] ];
			newScore = cl->ps.persistant[PERS_SCORE];
			if ( i == 0 || newScore != score ) {
				rank = i;
				// assume we aren't tied until the next client is checked
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank;
			} else {
				// we are tied with the previous client
				level.clients[ level.sortedClients[i - 1] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			score = newScore;
			if ( g_gametype.integer == GT_SINGLE_PLAYER && level.numPlayingClients == 1 ) {
				level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
		}
	}

	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if ( g_gametype.integer >= GT_TEAM ) {
		trap_SetConfigstring( CS_SCORES1, va( "%i", level.teamScores[TEAM_RED] ) );
		trap_SetConfigstring( CS_SCORES2, va( "%i", level.teamScores[TEAM_BLUE] ) );
	} else {
		if ( level.numConnectedClients == 0 ) {
			trap_SetConfigstring( CS_SCORES1, va( "%i", SCORE_NOT_PRESENT ) );
			trap_SetConfigstring( CS_SCORES2, va( "%i", SCORE_NOT_PRESENT ) );
		} else if ( level.numConnectedClients == 1 ) {
			trap_SetConfigstring( CS_SCORES1, va( "%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va( "%i", SCORE_NOT_PRESENT ) );
		} else {
			trap_SetConfigstring( CS_SCORES1, va( "%i", level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] ) );
			trap_SetConfigstring( CS_SCORES2, va( "%i", level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE] ) );
		}
	}

	//RtcwPro player info
	ServerPlayerInfo();

	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission, send the new info to everyone
	if ( level.intermissiontime ) {
		SendScoreboardMessageToAllClients();
	}
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients( void ) {
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[ i ].pers.connected == CON_CONNECTED ) {
			DeathmatchScoreboardMessage( g_entities + i );
		}
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( gentity_t *ent ) {
	// take out of follow mode if needed
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		StopFollowing( ent );
	}

	// move to the spot
	VectorCopy( level.intermission_origin, ent->s.origin );
	VectorCopy( level.intermission_origin, ent->client->ps.origin );
	VectorCopy( level.intermission_angle, ent->client->ps.viewangles );
	ent->client->ps.pm_type = PM_INTERMISSION;

	// clean up powerup info
	// memset( ent->client->ps.powerups, 0, sizeof(ent->client->ps.powerups) );

	ent->client->ps.eFlags = 0;
	ent->s.eFlags = 0;
	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.event = 0;
	ent->s.events[0] = ent->s.events[1] = ent->s.events[2] = ent->s.events[3] = 0;      // DHM - Nerve
	ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint( void ) {
	gentity_t   *ent, *target;
	vec3_t dir;
	char cs[MAX_STRING_CHARS];              // DHM - Nerve
	char        *buf;                       // DHM - Nerve
	int winner;                             // DHM - Nerve

	if ( g_gametype.integer >= GT_WOLF ) {
		ent = NULL;

		// NERVE - SMF - if the match hasn't ended yet, and we're just a spectator
		if ( !level.intermissiontime ) {

			// try to find the intermission spawnpoint with no team flags set
			ent = G_Find( NULL, FOFS( classname ), "info_player_intermission" );

			for ( ; ent; ent = G_Find( ent, FOFS( classname ), "info_player_intermission" ) ) {
				if ( !ent->spawnflags ) {
					break;
				}
			}
		}

		trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof( cs ) );
		buf = Info_ValueForKey( cs, "winner" );
		winner = atoi( buf );

		// Change from scripting value for winner (0==AXIS, 1==ALLIES) to spawnflag value
		if ( winner == 0 ) {
			winner = 1;
		} else {
			winner = 2;
		}

		if ( !ent ) {
			ent = G_Find( NULL, FOFS( classname ), "info_player_intermission" );

			if ( ent && !( ent->spawnflags & winner ) ) {
				ent = G_Find( ent, FOFS( classname ), "info_player_intermission" );
			}
		}
	} else {
		// find the intermission spot
		ent = G_Find( NULL, FOFS( classname ), "info_player_intermission" );
	}

	if ( !ent ) {   // the map creator forgot to put in an intermission point...
		SelectSpawnPoint( vec3_origin, level.intermission_origin, level.intermission_angle );
	} else {
		VectorCopy( ent->s.origin, level.intermission_origin );
		VectorCopy( ent->s.angles, level.intermission_angle );
		// if it has a target, look towards it
		if ( ent->target ) {
			target = G_PickTarget( ent->target );
			if ( target ) {
				VectorSubtract( target->s.origin, level.intermission_origin, dir );
				vectoangles( dir, level.intermission_angle );
			}
		}
	}

}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission( void ) {
	int i;
	gentity_t   *client;

	if ( level.intermissiontime ) {
		return;     // already active
	}

	// if in tournement mode, change the wins / losses
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		AdjustTournamentScores();
	}

	level.intermissiontime = level.time;
	FindIntermissionPoint();

	// move all clients to the intermission point
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		client = g_entities + i;
		if ( !client->inuse ) {
			continue;
		}
		// respawn if dead
		if ( g_gametype.integer < GT_WOLF && client->health <= 0 ) {
			respawn( client );
		}
		MoveClientToIntermission( client );
	}

	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();

	// reset client spawnpoints
	//SendSpawnpointResetToClients();

	G_matchInfoDump(EOM_MATCHINFO);
}


/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar

=============
*/
void ExitLevel( void ) {
	int i;
	gclient_t *cl;

	// if we are running a tournement map, kick the loser to spectator status,
	// which will automatically grab the next spectator and restart
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		if ( !level.restarted ) {
			RemoveTournamentLoser();
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
			level.restarted = qtrue;
			level.changemap = NULL;
			level.intermissiontime = 0;
		}
		return;
	}

	trap_SendConsoleCommand( EXEC_APPEND, "vstr nextmap\n" );
	level.changemap = NULL;
	level.intermissiontime = 0;

	// reset all the scores so we don't enter the intermission again
	level.teamScores[TEAM_RED] = 0;
	level.teamScores[TEAM_BLUE] = 0;
	for ( i = 0 ; i < g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.persistant[PERS_SCORE] = 0;
		//cl->ps.persistant[PERS_RESTRICTEDWEAPON] = WP_NONE; // reset weapon restrictions on exit
	}

	// reset all the weapon restrictions so next time the players spawn they get set correctly
	level.alliedFlamer = level.axisFlamer = 0;
	level.alliedSniper = level.axisSniper = 0;
	level.alliedPF = level.axisPF = 0;
	level.alliedVenom = level.axisVenom = 0;

	// we need to do this here before chaning to CON_CONNECTING
	G_WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for ( i = 0 ; i < g_maxclients.integer ; i++ ) {

		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}

	 if (g_altStopwatchMode.integer == 1 && g_currentRound.integer == 1) {
		G_swapTeams();
	 }
	G_LogPrintf( "ExitLevel: executed\n" );
}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... ) {
	va_list argptr;
	char string[1024];
	int min, tens, sec;

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof( string ), "%3i:%i%i ", min, tens, sec );

	va_start( argptr, fmt );
	Q_vsnprintf( string + 7, sizeof( string ) - 7, fmt, argptr );
	va_end( argptr );

	if ( g_dedicated.integer ) {
		G_Printf( "%s", string + 7 );
	}

	if ( !level.logFile ) {
		return;
	}

	trap_FS_Write( string, strlen( string ), level.logFile );
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string ) {
	int i, numSorted;
	gclient_t       *cl;

	// NERVE - SMF - do not allow LogExit to be called in non-playing gamestate
	if ( g_gamestate.integer != GS_PLAYING ) {
		return;
	}

	G_LogPrintf( "Exit: %s\n", string );

	level.intermissionQueued = level.time;

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	trap_SetConfigstring( CS_INTERMISSION, "1" );

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numConnectedClients;
	if ( numSorted > 32 ) {
		numSorted = 32;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		G_LogPrintf( "red:%i  blue:%i\n",
					 level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
	}

	// NERVE - SMF - send gameCompleteStatus message to master servers
	trap_SendConsoleCommand( EXEC_APPEND, "gameCompleteStatus\n" );

	for ( i = 0 ; i < numSorted ; i++ ) {
		int ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		if ( cl->pers.connected == CON_CONNECTING ) {
			continue;
		}

		// RTCWPro
		//ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		if (g_alternatePing.integer)
		{
			ping = cl->pers.alternatePing < 999 ? cl->pers.alternatePing : 999;
		}
		else
		{
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}
		// RTCWPro end

		G_LogPrintf( "score: %i  ping: %i  client: %i %s\n",
					 cl->ps.persistant[PERS_SCORE], ping, level.sortedClients[i],
					 cl->pers.netname );
	}

	// NERVE - SMF
	if ( g_gametype.integer == GT_WOLF_STOPWATCH ) {
		char cs[MAX_STRING_CHARS];
		int winner, defender;

		trap_GetConfigstring( CS_MULTI_INFO, cs, sizeof( cs ) );
		defender = atoi( Info_ValueForKey( cs, "defender" ) );

		trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof( cs ) );
		winner = atoi( Info_ValueForKey( cs, "winner" ) );

		// NERVE - SMF
		if ( !g_currentRound.integer ) {
			if ( winner == defender ) {
				// if the defenders won, use default timelimit
				trap_Cvar_Set( "g_nextTimeLimit", va( "%f", g_timelimit.value ) );
			} else {
				// use remaining time as next timer
				trap_Cvar_Set( "g_nextTimeLimit", va( "%f", ( level.timeCurrent - level.startTime ) / 60000.f ) );
			}
		} else {
			// reset timer
			trap_Cvar_Set( "g_nextTimeLimit", "0" );
		}

		trap_Cvar_Set( "g_currentRound", va( "%i", !g_currentRound.integer ) );
	}
	// -NERVE - SMF
}

/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit( void ) {
	//static int fActions = 0;	// L0 - OSP port
	int ready, notReady, playerCount;
	int i;
	gclient_t   *cl;
	int readyMask;

	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		return;
	}

	// DHM - Nerve :: Flat 10 second timer until exit
	if ( g_gametype.integer >= GT_WOLF ) {
		if ( level.time < level.intermissiontime + 10000 ) {
			return;
		}

		ExitLevel();
		return;
	}
	// dhm - end

	// see which players are ready
	ready = 0;
	notReady = 0;
	readyMask = 0;
	playerCount = 0;
	for ( i = 0 ; i < g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) {
			continue;
		}

		playerCount++;
		if ( cl->readyToExit ) {
			ready++;
			if ( i < 16 ) {
				readyMask |= 1 << i;
			}
		} else {
			notReady++;
		}
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for ( i = 0 ; i < g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
	}

	// never exit in less than five seconds
	if ( level.time < level.intermissiontime + 5000 ) {
		return;
	}

	// only test ready status when there are real players present
	if ( playerCount > 0 ) {
		// if nobody wants to go, clear timer
		if ( !ready ) {
			level.readyToExit = qfalse;
			return;
		}

	// if everyone wants to go, go now
		if ( !notReady ) {
			ExitLevel();
			return;
		}
	}

	// the first person to ready starts the ten second timeout
	if ( !level.readyToExit ) {
		level.readyToExit = qtrue;
		level.exitTime = level.time;
	}

	// if we have waited ten seconds since at least one player
	// wanted to exit, go ahead
	if ( level.time < level.exitTime + 10000 ) {
		return;
	}

	ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied( void ) {
	int a, b;
	char cs[MAX_STRING_CHARS];
	char    *buf;

	// DHM - Nerve :: GT_WOLF checks the current value of
	if ( g_gametype.integer >= GT_WOLF ) {
		trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof( cs ) );

		buf = Info_ValueForKey( cs, "winner" );
		a = atoi( buf );

		return a == -1;
	}

	if ( level.numPlayingClients < 2 ) {
		return qfalse;
	}

	if ( g_gametype.integer >= GT_TEAM ) {
		return level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE];
	}

	a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
	b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];

	return a == b;
}

qboolean G_ScriptAction_SetWinner( gentity_t *ent, char *params );

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules( void ) {
	int i;
	gclient_t   *cl;
	gentity_t   *gm;
	char cs[MAX_STRING_CHARS];
	char txt[5];
	int num;

	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if ( level.intermissiontime ) {
		CheckIntermissionExit();
		return;
	}

	if ( level.intermissionQueued ) {
		if ( level.time - level.intermissionQueued >= INTERMISSION_DELAY_TIME || g_gametype.integer >= GT_WOLF ) {
			level.intermissionQueued = 0;
			BeginIntermission();
		}
		return;
	}

	if ( g_timelimit.value && !level.warmupTime && level.paused == PAUSE_NONE ) {
		if ( level.timeCurrent - level.startTime >= g_timelimit.value * 60000 ) {

			// check for sudden death
			if ( g_gametype.integer != GT_CTF && ScoreIsTied() ) {
				// score is tied, so don't end the game
				return;
			}

			if ( g_gametype.integer >= GT_WOLF ) {
				gm = G_Find( NULL, FOFS( scriptName ), "game_manager" );
// JPW NERVE -- in CPH, check final capture/hold times and adjust winner
				// 0 == axis, 1 == allied
				if ( g_gametype.integer == GT_WOLF_CPH ) {
					num = -1;
					if ( level.capturetimes[TEAM_RED] > level.capturetimes[TEAM_BLUE] ) {
						num = 0;
					}
					if ( level.capturetimes[TEAM_RED] < level.capturetimes[TEAM_BLUE] ) {
						num = 1;
					}
					if ( num != -1 ) {
						sprintf( txt,"%d",num );
						G_ScriptAction_SetWinner( NULL, txt );
					}
				}
// jpw
				if ( gm ) {
					G_Script_ScriptEvent( gm, "trigger", "timelimit_hit" );
				}
			}

			// NERVE - SMF - do not allow LogExit to be called in non-playing gamestate
			// - This already happens in LogExit, but we need it for the print command
			if ( g_gamestate.integer != GS_PLAYING ) {
				return;
			}

			trap_SendServerCommand( -1, "print \"Timelimit hit.\n\"" );
			LogExit( "Timelimit hit." );

			return;
		}
	}

	if ( level.numPlayingClients < 2 ) {
		return;
	}

	if ( g_gametype.integer >= GT_WOLF && ( g_maxlives.integer > 0 || g_axismaxlives.integer > 0 || g_alliedmaxlives.integer > 0 ) )
	{
		if ( level.numFinalDead[0] >= level.axisPlayers && level.axisPlayers > 0 )
		{
			trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof( cs ) );
			Info_SetValueForKey( cs, "winner", "1" );
			trap_SetConfigstring( CS_MULTI_MAPWINNER, cs );
			// RTCWPro - moved from WM_DrawObjectives in cg
			//AAPS("sound/match/winallies_pro.wav");
			LogExit( "Axis team eliminated." );
		}
		else if ( level.numFinalDead[1] >= level.alliedPlayers && level.alliedPlayers > 0 )
		{
			trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof( cs ) );
			Info_SetValueForKey( cs, "winner", "0" );
			trap_SetConfigstring( CS_MULTI_MAPWINNER, cs );
			//AAPS("sound/match/winaxis_pro.wav");
			LogExit( "Allied team eliminated." );
		}
	}

	if ( ( g_gametype.integer != GT_CTF && g_gametype.integer < GT_WOLF ) && g_fraglimit.integer )
	{
		if ( level.teamScores[TEAM_RED] >= g_fraglimit.integer )
		{
			trap_SendServerCommand( -1, "print \"Red hit the fraglimit.\n\"" );
			//AAPS("sound/match/winaxis_pro.wav");
			LogExit( "Fraglimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_fraglimit.integer )
		{
			trap_SendServerCommand( -1, "print \"Blue hit the fraglimit.\n\"" );
			//AAPS("sound/match/winallies_pro.wav");
			LogExit( "Fraglimit hit." );
			return;
		}

		for ( i = 0 ; i < g_maxclients.integer ; i++ )
		{
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED )
			{
				continue;
			}
			if ( cl->sess.sessionTeam != TEAM_FREE )
			{
				continue;
			}

			if ( cl->ps.persistant[PERS_SCORE] >= g_fraglimit.integer )
			{
				LogExit( "Fraglimit hit." );
				trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " hit the fraglimit.\n\"",
												cl->pers.netname ) );
				return;
			}
		}
	}

	if ( g_gametype.integer == GT_CTF && g_capturelimit.integer )
	{

		if ( level.teamScores[TEAM_RED] >= g_capturelimit.integer )
		{
			trap_SendServerCommand( -1, "print \"Red hit the capturelimit.\n\"" );
			//AAPS("sound/match/winaxis_pro.wav");
			LogExit( "Capturelimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_capturelimit.integer )
		{
			trap_SendServerCommand( -1, "print \"Blue hit the capturelimit.\n\"" );
			//AAPS("sound/match/winallies_pro.wav");
			LogExit( "Capturelimit hit." );
			return;
		}
	}
}



/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/


/*
=============
CheckTournement

Once a frame, check for changes in tournement player state
=============
*/
void CheckTournement( void ) {
	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if ( g_gametype.integer != GT_TOURNAMENT ) {
		return;
	}
	if ( level.numPlayingClients == 0 ) {
		return;
	}

	// pull in a spectator if needed
	if ( level.numPlayingClients < 2 ) {
		AddTournamentPlayer();
	}

	// if we don't have two players, go back to "waiting for players"
	if ( level.numPlayingClients != 2 ) {
		if ( level.warmupTime != -1 ) {
			level.warmupTime = -1;
			trap_SetConfigstring( CS_WARMUP, va( "%i", level.warmupTime ) );
			G_LogPrintf( "Warmup:\n" );
		}
		return;
	}

	if ( level.warmupTime == 0 ) {
		return;
	}

	// if the warmup is changed at the console, restart it
	if ( g_warmup.modificationCount != level.warmupModificationCount ) {
		level.warmupModificationCount = g_warmup.modificationCount;
		level.warmupTime = -1;
	}

	// if all players have arrived, start the countdown
	if ( level.warmupTime < 0 ) {
		if ( level.numPlayingClients == 2 ) {
			// fudge by -1 to account for extra delays
			level.warmupTime = level.time + (g_warmup.integer - 1) * 1000;
			trap_SetConfigstring( CS_WARMUP, va( "%i", level.warmupTime ) );
		}
		return;
	}

	// if the warmup time has counted down, restart
	if ( level.time > level.warmupTime ) {
		level.warmupTime += 10000;
		trap_Cvar_Set( "g_restarted", "1" );
		trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );

		level.restarted = qtrue;
		return;
	}
}

/*
=============
CheckWolfMP

NERVE - SMF
=============
*/
void CheckGameState( void ) {
	gamestate_t current_gs;

	current_gs = trap_Cvar_VariableIntegerValue( "gamestate" );

	if ( level.intermissiontime && current_gs != GS_INTERMISSION ) {
		trap_Cvar_Set( "gamestate", va( "%i", GS_INTERMISSION ) );
		return;
	}

	if ( g_tournament.integer && !trap_Cvar_VariableIntegerValue( "sv_serverRestarting" ) ) {
		if ( current_gs != GS_WAITING_FOR_PLAYERS && g_minGameClients.integer > 1 && level.numPlayingClients <= 1 && level.lastRestartTime + 1000 < level.time ) {
			level.lastRestartTime = level.time;

			if (g_tournament.integer) {
				trap_SetConfigstring( CS_READY, va( "%i", READY_PENDING ) );
				trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", GS_WARMUP ) );
			} else {
				trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", GS_WAITING_FOR_PLAYERS ) );
			}
		}
	}

	if ( current_gs == GS_WAITING_FOR_PLAYERS && g_minGameClients.integer > 1 &&
		 level.numPlayingClients >= g_minGameClients.integer && level.lastRestartTime + 1000 < level.time
		 && !g_tournament.integer ) {

		level.lastRestartTime = level.time;
		trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", GS_WARMUP ) );
	}

	// if the warmup is changed at the console, restart it
	if ( current_gs == GS_WARMUP_COUNTDOWN && g_warmup.modificationCount != level.warmupModificationCount ) {
		level.warmupModificationCount = g_warmup.modificationCount;
		current_gs = GS_WARMUP;
	}

	// check warmup latch
	if ( current_gs == GS_WARMUP ) {

		// L0 - Tourny..
		if (g_tournament.integer) {

			if (G_playersReady() || level.readyAll) {
				level.warmupSwap = qfalse;
				level.warmupTime = level.time + 11000;
				trap_SetConfigstring( CS_READY, va( "%i", READY_NONE ));
				trap_SetConfigstring( CS_WARMUP, va( "%i", level.warmupTime ) );
				trap_Cvar_Set( "gamestate", va( "%i", GS_WARMUP_COUNTDOWN ) );
				// Prevents joining once countdown starts..
				//if (g_tournament.integer == 2) // xmod has a value of 2 but we do not
				G_readyTeamLock();

				if (!level.readyPrint) {
					// show all players ready for both right now - need to fix pause for noteamswitching = 1
					AP(va("cp \"%s\n\"2", (g_noTeamSwitching.integer ? "^3All players are now ready!" : "^3All players are now ready!"))); //"Ready threshold reached!") ));
					level.readyPrint = qtrue;
				}
			} else {

				level.warmupSwap = qtrue;
				trap_SetConfigstring( CS_READY, va( "%i", (g_noTeamSwitching.integer ? READY_PENDING : READY_AWAITING) ));
			}

		} else { // Tourny is off..act casual
			int delay = g_warmup.integer + 1;
			trap_SetConfigstring( CS_READY, va( "%i", READY_NONE ));

			if ( delay < 6 ) {
				trap_Cvar_Set( "g_warmup", "5" );
				delay = 11;
			}

			level.warmupTime = level.time + ( delay * 1000 );
			trap_SetConfigstring( CS_WARMUP, va( "%i", level.warmupTime ) );
			trap_Cvar_Set( "gamestate", va( "%i", GS_WARMUP_COUNTDOWN ) );
		}
	}

	// L0 - Reset countdown if ready goes off (eg. player enters, leaves..)
	if ( current_gs == GS_WARMUP_COUNTDOWN ) {
		if (g_tournament.integer) {
			if (!G_playersReady()) // && !level.readyAll )
				G_readyReset(qfalse);
		}
	}
}


/*
=============
CheckWolfMP

NERVE - SMF - Once a frame, check for changes in wolf MP player state
=============
*/
void CheckWolfMP() {
	// TTimo unused
//	static qboolean latch = qfalse;

	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if ( g_gametype.integer < GT_WOLF ) {
		return;
	}

	// NERVE - SMF - check game state
	CheckGameState();

	if ( level.warmupTime == 0 ) {
		return;
	}

	// L0 - Countdown
	if (level.time > level.warmupTime - 7100 && !level.cnStarted) {
		level.cnStarted = qtrue;
		CountDown();
		return;
	}
}
// -NERVE - SMF

/*
==================
CheckVote
==================
*/
void CheckVote( void )
{
	if ( !level.voteInfo.voteTime || level.voteInfo.vote_fn == NULL || level.time - level.voteInfo.voteTime < 1000 )
	{
		return;
	}

	if ( level.time - level.voteInfo.voteTime >= VOTE_TIME ) {
		AP( va( "cpm \"^2Vote FAILED! ^3(%s)\n\"", level.voteInfo.voteString ) );
		G_LogPrintf( "Vote Failed: %s\n", level.voteInfo.voteString );
	}
	else
	{
		int total = level.voteInfo.numVotingClients;

		// issue #391 when kicking a player only do the same team's players for the total
		if (level.voteInfo.vote_fn == G_Kick_v)
		{
			gentity_t* other = &g_entities[atoi(level.voteInfo.vote_value)];
			if (!other->client || other->client->sess.sessionTeam == TEAM_SPECTATOR)
			{
				total = level.voteInfo.numVotingClients;
			}
			else
			{
				total = level.voteInfo.numVotingTeamClients[other->client->sess.sessionTeam == TEAM_RED ? 0 : 1];
			}
		}
		else
		{
			total = level.voteInfo.numVotingClients;
		}

		// OSPx - Vote percent..
		int pcnt = (!Q_stricmp(level.voteInfo.voteString, "Start Match") ? 75 : vote_percent.integer);

		if (pcnt > 99)
			pcnt = 99;
		else if (pcnt < 1)
			pcnt = 1;

		// Vote will always pass with single client..rest is perc depended..
		//if ((total == 1) || (100 * level.voteInfo.voteYes / total >= pcnt))
		if ((total == 1) || (level.voteInfo.voteYes > pcnt * total / 100))
		{
			AP("cpm \"^5Vote passed!\n\"");
			G_LogPrintf("Vote Passed: %s\n", level.voteInfo.voteString);

			// execute the command, then remove the vote
			level.voteInfo.vote_fn(NULL, 0, NULL, NULL, qfalse); // Perform the passed vote
			level.voteExecuteTime = level.time + 3000;
			level.prevVoteExecuteTime = level.time + 4000;

		}
		else if (level.voteInfo.voteNo && level.voteInfo.voteNo >= (100 - pcnt) * total / 100)
		{
			// same behavior as a timeout
			AP( va( "cpm \"^2Vote FAILED! ^3(%s)\n\"", level.voteInfo.voteString ) );
			G_LogPrintf( "Vote Failed: %s\n", level.voteInfo.voteString );
		}
		else
		{
			// still waiting for a majority
			return;
		}

	}

	level.voteInfo.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );

}

/*
=============
CheckReloadStatus
=============
*/
qboolean reloading = qfalse;

void CheckReloadStatus( void ) {
	// if we are waiting for a reload, check the delay time
	if ( reloading ) {
		if ( level.reloadDelayTime ) {
			if ( level.reloadDelayTime < level.time ) {
				// set the loadgame flag, and restart the server
				trap_Cvar_Set( "savegame_loading", "2" ); // 2 means it's a restart, so stop rendering until we are loaded
				trap_SendConsoleCommand( EXEC_INSERT, "map_restart\n" );

				level.reloadDelayTime = 0;
			}
		} else if ( level.reloadPauseTime ) {
			if ( level.reloadPauseTime < level.time ) {
				reloading = qfalse;
				level.reloadPauseTime = 0;
			}
		}
	}
}

/*
==================
CheckCvars
==================
*/
void CheckCvars( void ) {
	static int lastMod = -1;

	if ( g_password.modificationCount != lastMod ) {
		lastMod = g_password.modificationCount;
		if ( *g_password.string && Q_stricmp( g_password.string, "none" ) ) {
			trap_Cvar_Set( "g_needpass", "1" );
		} else {
			trap_Cvar_Set( "g_needpass", "0" );
		}
	}
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink( gentity_t *ent ) {
	float thinktime;

	// L0 - Pause dump
	if ( level.paused != PAUSE_NONE && ( ent - g_entities ) >= g_maxclients.integer &&
		 ent->nextthink > level.time && strstr( ent->classname, "DPRINTF_" ) == NULL ) {
		ent->nextthink += level.time - level.previousTime;
	}

	// RF, run scripting
	if ( ent->s.number >= MAX_CLIENTS ) {
//----(SA)	this causes trouble in various maps
		// escape1 - first radio room nazi is not there
		// basein - truck you start in is rotated 90 deg off
		// will explain more if necessary when awake :)

//		if (!(saveGamePending || (g_missionStats.string[0] || g_missionStats.string[1]))) {
		G_Script_ScriptRun( ent );
//		}
//----(SA)	end
	}

	thinktime = ent->nextthink;
	if ( thinktime <= 0 ) {
		return;
	}
	if ( thinktime > level.time ) {
		return;
	}

	ent->nextthink = 0;
	if ( !ent->think ) {
		G_Error( "NULL ent->think" );
	}
	ent->think( ent );
}

/*
================
OSPx - check for team stuff..
================
*/
void HandleEmptyTeams(void) {

	// if there is a live Tournament Stopwatch round going do not reset the match if the other team rage quits
	if (g_gamestate.integer == GS_PLAYING && g_tournament.integer == 1 && g_gametype.integer == GT_WOLF_STOPWATCH)
		return;

	if (g_gamestate.integer != GS_INTERMISSION) {
		if (!level.axisPlayers && match_minplayers.integer > 1) {
			G_teamReset(TEAM_RED, qtrue, qfalse);

			// Reset match if not paused with an empty team
			if (level.paused == PAUSE_NONE && g_gamestate.integer == GS_PLAYING)
				Svcmd_ResetMatch_f(qtrue, qtrue);
		}
		else if (!level.alliedPlayers && match_minplayers.integer > 1) {
			G_teamReset(TEAM_BLUE, qtrue, qfalse);

			// Reset match if not paused with an empty team
			if (level.paused == PAUSE_NONE && g_gamestate.integer == GS_PLAYING)
				Svcmd_ResetMatch_f(qtrue, qtrue);
		}
	}
}

/*
================
L0 - SortedActivePlayers

Sort players per teams so I can re-use this call where need it..
================
*/
void SortedActivePlayers(void) {
	int i;
	level.axisPlayers = 0;
	level.alliedPlayers = 0;

	for(i = 0; i < level.maxclients; i++){
		if (level.clients[i].pers.connected != CON_CONNECTED)
			continue;
		if (level.clients[i].sess.sessionTeam == TEAM_RED)
			level.axisPlayers++;
		if (level.clients[i].sess.sessionTeam == TEAM_BLUE)
			level.alliedPlayers++;
	}
}

/*
===========
RtcwPro - G_teamJoinCheck (et port)

Checks to see if a specified team is allowing players to join.
===========
*/
qboolean G_teamJoinCheck(int team_num, gentity_t* ent) {
	int cnt = TeamCount(-1, team_num);

	// Sanity check
	if (cnt == 0 && g_gamestate.integer != GS_PLAYING && g_gamestate.integer != GS_WARMUP_COUNTDOWN) { // only reset if we aren't playing/countdown
		G_teamReset(team_num, qtrue, qfalse);
		teamInfo[team_num].team_lock = qfalse;
	}

	// Check for locked teams
	if ((team_num == TEAM_RED || team_num == TEAM_BLUE)) {
		if (ent->client->sess.sessionTeam == team_num) {
			return(qtrue);
		}

		// Check for full teams
		if (team_maxplayers.integer > 0 && team_maxplayers.integer <= cnt) {
			CP(va("cp \"The %s team is full!\n\"2", aTeams[team_num]));
			//AP(va("print \"*** ^3INFO: ^7The %s team is full.\n\"", aTeams[team_num]));
			return(qfalse);
		} // Check for locked teams
		else if (teamInfo[team_num].team_lock /*&& (!(ent->client->pers.invite & team_num))*/) {
			CP(va("cp \"The %s team is LOCKED!\n\"2", aTeams[team_num]));
			//AP(va("print \"*** ^3INFO: ^7The %s team is locked.\n\"", aTeams[team_num]));
			return(qfalse);
		}
	}
	return(qtrue);
}

/*
KK commented this out this is a "referee" lock we don't need
================
L0 - TeamLockStatus

Sometimes people lock the teams and leave with callvotes off..as result game becomes unplayable..
So this deals with issue..
================

void TeamLockStatus(void) {

	if ((g_gamestate.integer == GS_WAITING_FOR_PLAYERS || g_gamestate.integer == GS_WARMUP) && g_gamelocked.integer != 0)
	{
		trap_Cvar_Set("g_gamelocked", "0"); // unlock teams during warmup
		trap_Cvar_Update(&g_gamelocked);
		G_teamReset(0, qfalse, qtrue); // both teams
	}

	// RtcwPro added this to avoid erroneous text at the end of the round
	if (g_gamestate.integer != GS_INTERMISSION && g_gamelocked.integer > 0)
	{
		// Check now
		if (level.numPlayingClients == 0 && g_gamelocked.integer > 0) {
			trap_Cvar_Set("g_gamelocked", "0");
			trap_Cvar_Update(&g_gamelocked);
			G_teamReset(0, qfalse, qtrue); // both teams
			AP("chat \"^zconsole: ^7Teams have no players! Server is releasing the team lock^z!\n\"");
		}
		else if (!level.axisPlayers && g_gamelocked.integer == 3) {
			trap_Cvar_Set("g_gamelocked", "2");
			trap_Cvar_Update(&g_gamelocked);
			G_teamReset(TEAM_RED, qfalse, qfalse);
			AP("chat \"^zconsole: ^1Axis ^7team has no players! Server unlocked Axis team^z!\n\"");
		}
		else if (!level.axisPlayers && g_gamelocked.integer == 1) {
			trap_Cvar_Set("g_gamelocked", "0");
			trap_Cvar_Update(&g_gamelocked);
			G_teamReset(0, qfalse, qtrue); // both teams
			AP("chat \"^zconsole: ^1Axis ^7team has no players! Server unlocked Axis team^z!\n\"");
		}
		else if (!level.alliedPlayers && g_gamelocked.integer == 2) {
			trap_Cvar_Set("g_gamelocked", "0");
			trap_Cvar_Update(&g_gamelocked);
			G_teamReset(0, qfalse, qtrue); // both teams
			AP("chat \"^zconsole: ^4Allied ^7team has no players! Server unlocked Allied team^z!\n\"");
		}
		else if (!level.alliedPlayers && g_gamelocked.integer == 3) {
			trap_Cvar_Set("g_gamelocked", "1");
			trap_Cvar_Update(&g_gamelocked);
			G_teamReset(TEAM_BLUE, qfalse, qfalse);
			AP("chat \"^zconsole: ^4Allied ^7team has no players! Server unlocked Allied team^z!\n\"");
		}
	}
}
*/

/*
Player Info (port from ET)
sane replacement for OSP's Players_Axis/Players_Allies
*/
void ServerPlayerInfo(void) {
	//128 bits
	char playerinfo[MAX_CLIENTS + 1];
	gentity_t* e;
	team_t playerteam;
	int i;
	int lastclient;

	memset(playerinfo, 0, sizeof(playerinfo));

	lastclient = -1;
	e = &g_entities[0];
	for (i = 0; i < MAX_CLIENTS; i++, e++) {
		if (e->client == NULL || e->client->pers.connected == CON_DISCONNECTED) {
			playerinfo[i] = '-';
			continue;
		}

		//keep track of highest connected/connecting client
		lastclient = i;

		if (e->inuse == qfalse) {
			playerteam = 0;
		}
		else {
			playerteam = e->client->sess.sessionTeam;
		}
		playerinfo[i] = (char)'0' + playerteam;
	}
	//terminate the string, if we have any non-0 clients
	if (lastclient != -1) {
		playerinfo[lastclient + 1] = (char)0;
	}
	else {
		playerinfo[0] = (char)0;
	}

	trap_Cvar_Set("P", playerinfo);
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime ) {
	int i;
	gentity_t   *ent;
	int msec;
	int worldspawnflags, gt;
	int start, end;

	// if we are waiting for the level to restart, do nothing
	if ( level.restarted ) {
		return;
	}

	// L0 - Pause
	// OSP Handling of pause offsets
	if (level.paused == PAUSE_NONE) {
		level.timeCurrent = levelTime - level.timeDelta;
	}
	else {
		level.timeDelta = levelTime - level.timeCurrent;
		if ((level.time % 500) == 0) {
			// Respawn and time issuses
			trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime + level.timeDelta));
		}
	} // End

//	level.frameTime = trap_Milliseconds();
	if (g_antilag.integer < 2) // not unlagged
		level.frameStartTime = trap_Milliseconds();

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;
	msec = level.time - level.previousTime;

	// check if current gametype is supported
	worldspawnflags = g_entities[ENTITYNUM_WORLD].spawnflags;
	if  ( !level.latchGametype && g_gamestate.integer == GS_PLAYING &&
		  ( ( g_gametype.integer == GT_WOLF && ( worldspawnflags & NO_GT_WOLF ) ) ||
			( g_gametype.integer == GT_WOLF_STOPWATCH && ( worldspawnflags & NO_STOPWATCH ) ) ||
			( ( g_gametype.integer == GT_WOLF_CP || g_gametype.integer == GT_WOLF_CPH ) && ( worldspawnflags & NO_CHECKPOINT ) ) ) // JPW NERVE added CPH
		  ) {

		if ( !( worldspawnflags & NO_GT_WOLF ) ) {
			gt = 5;
		} else {
			gt = 6; // stopwatch
		}

		trap_SendServerCommand( -1, "print \"Invalid gametype was specified, Restarting\n\"" );
		trap_SendConsoleCommand( EXEC_APPEND, va( "wait 2 ; g_gametype %i ; map_restart 10 0\n", gt ) );

		level.latchGametype = qtrue;
	}

	// get any cvar changes
	G_UpdateCvars();

	//
	// go through all allocated objects
	//
	//start = trap_Milliseconds();
	ent = &g_entities[0];
	for ( i = 0 ; i < level.num_entities ; i++, ent++ ) {
		if ( !ent->inuse ) {
			continue;
		}

		// check EF_NODRAW status for non-clients
		if ( i > level.maxclients ) {
			if ( ent->flags & FL_NODRAW ) {
				ent->s.eFlags |= EF_NODRAW;
			} else {
				ent->s.eFlags &= ~EF_NODRAW;
			}
		}

		// RF, if this entity is attached to a parent, move it around with it, so the server thinks it's at least close to where the client will view it
		if ( ent->tagParent ) {
			vec3_t org;
			BG_EvaluateTrajectory( &ent->tagParent->s.pos, level.time, org );
			G_SetOrigin( ent, org );
			VectorCopy( org, ent->s.origin );
			if ( ent->r.linked ) {    // update position
				trap_LinkEntity( ent );
			}
		}

		// clear events that are too old
		if ( level.time - ent->eventTime > EVENT_VALID_MSEC ) {
			if ( ent->s.event ) {
				ent->s.event = 0;   // &= EV_EVENT_BITS;
				//if ( ent->client ) {
				//ent->client->ps.externalEvent = 0;	// (SA) MISSIONPACK.  Wolf does not have ps.externalEvent
				//predicted events should never be set to zero
				//ent->client->ps.events[0] = 0;
				//ent->client->ps.events[1] = 0;
				//}
			}
			if ( ent->freeAfterEvent ) {
				// tempEntities or dropped items completely go away after their event
				G_FreeEntity( ent );
				continue;
			} else if ( ent->unlinkAfterEvent ) {
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				trap_UnlinkEntity( ent );
			}
		}

		// MrE: let the server know about bbox or capsule collision
		if ( ent->s.eFlags & EF_CAPSULE ) {
			ent->r.svFlags |= SVF_CAPSULE;
		} else {
			ent->r.svFlags &= ~SVF_CAPSULE;
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent ) {
			continue;
		}

		if ( !ent->r.linked && ent->neverFree ) {
			continue;
		}

		if ( ent->s.eType == ET_MISSILE
			 || ent->s.eType == ET_FLAMEBARREL
			 || ent->s.eType == ET_FP_PARTS
			 || ent->s.eType == ET_FIRE_COLUMN
			 || ent->s.eType == ET_FIRE_COLUMN_SMOKE
			 || ent->s.eType == ET_EXPLO_PART
			 || ent->s.eType == ET_RAMJET
		) {
			// L0 - Pause dump
			if ( level.paused == PAUSE_NONE ) {
				if (g_antilag.integer != 2 || g_delagMissiles.integer == 0) {
					G_RunMissile(ent);
				}
			} else {
				// During a pause, gotta keep track of stuff in the air
				ent->s.pos.trTime += level.time - level.previousTime;
				// Keep pulsing right for dynmamite
				if ( ent->methodOfDeath == MOD_DYNAMITE ) {
					ent->s.effect1Time += level.time - level.previousTime;
				}
				G_RunThink( ent );
			}
			continue;
		}

		// DHM - Nerve :: Server-side collision for flamethrower
		if ( ent->s.eType == ET_FLAMETHROWER_CHUNK ) {
			G_RunFlamechunk( ent );
			continue;
		}

		if ( ent->s.eType == ET_ITEM || ent->physicsObject ) {
			G_RunItem( ent );
			continue;
		}

		if ( ent->s.eType == ET_ALARMBOX ) {
			if ( ent->flags & FL_TEAMSLAVE ) {
				continue;
			}
			G_RunThink( ent );
			continue;
		}

		if ( ent->s.eType == ET_MOVER || ent->s.eType == ET_PROP ) {
			G_RunMover( ent );
			continue;
		}

		if ( i < MAX_CLIENTS ) {
			G_RunClient( ent );
			continue;
		}

		G_RunThink( ent );
	}
//end = trap_Milliseconds();

	// Ridah, move the AI
	//AICast_StartServerFrame ( level.time );

	if (g_antilag.integer == 2 && g_delagMissiles.integer == 1) // unlagged
	{
		//unlagged - backward reconciliation #2
		// NOW run the missiles, with all players backward-reconciled
		// to the positions they were in exactly 50ms ago, at the end
		// of the last server frame
		G_TimeShiftAllClients(level.previousTime, NULL);

		ent = &g_entities[0];
		for (i = 0; i < level.num_entities; i++, ent++) {
			if (!ent->inuse) {
				continue;
			}

			// temporary entities don't think
			if (ent->freeAfterEvent) {
				continue;
			}

			if (level.paused == PAUSE_NONE && ent->s.eType == ET_MISSILE) {
				G_RunMissile(ent);
			}
		}

		G_UnTimeShiftAllClients(NULL);
		//unlagged - backward reconciliation #2
	}

	end = trap_Milliseconds();
  
	start = trap_Milliseconds();
	// perform final fixups on the players
	ent = &g_entities[0];
	for (i=0 ; i < level.maxclients ; i++, ent++ ) {
		if ( ent->inuse ) {
			ClientEndFrame( ent );
		}
	}
	end = trap_Milliseconds();

	// see if it is time to do a tournement restart
//	CheckTournament();

	// NERVE - SMF
	CheckWolfMP();

	// see if it is time to end the level
	CheckExitRules();

	// update to team status?
	CheckTeamStatus();

	// cancel vote if timed out
	CheckVote();

	// L0 sync team
//	if (g_needBalance.integer){
//		if (level.time >= level.balanceTimer)
//			balanceTeams();
//	}
	// end
	// check team votes
//	CheckTeamVote( TEAM_RED );
//	CheckTeamVote( TEAM_BLUE );

	// for tracking changes
	CheckCvars();

	// L0 - countdown
	if ((level.time > level.cnPush) && (g_gamestate.integer == GS_WARMUP_COUNTDOWN)) {
		CountDown();
	} // end

	if ( g_listEntity.integer ) {
		for ( i = 0; i < MAX_GENTITIES; i++ ) {
			G_Printf( "%4i: %s\n", i, g_entities[i].classname );
		}
		trap_Cvar_Set( "g_listEntity", "0" );
	}

	// NERVE - SMF
	if ( g_showHeadshotRatio.integer && level.missedHeadshots > 0 ) {
		G_Printf( "Headshot Ratio = %2.2f percent, made = %i, missed = %i\n", ( float )level.totalHeadshots / level.missedHeadshots * 100.f, level.totalHeadshots, level.missedHeadshots );
	}

	// Ridah, check if we are reloading, and times have expired
	CheckReloadStatus();

	qboolean isServerRestarting = trap_Cvar_VariableIntegerValue("sv_serverRestarting");
	if (!isServerRestarting) {
		// L0 - Count active players..
		SortedActivePlayers();
		// L0 - Check Team Lock status..
		HandleEmptyTeams();
	}

	if (g_antilag.integer == 2) // unlagged
	{
		//unlagged - backward reconciliation #4
		// record the time at the end of this frame - it should be about
		// the time the next frame begins - when the server starts
		// accepting commands from connected clients
		level.frameStartTime = trap_Milliseconds();
		//unlagged - backward reconciliation #4
	}

}
