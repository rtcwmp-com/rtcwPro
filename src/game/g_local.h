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



// g_local.h -- local definitions for game module

#include "q_shared.h"
#include "bg_public.h"
#include "g_public.h"
#include "../../MAIN/ui_mp/menudef.h"
#ifdef _WIN32
#include "../qcommon/jansson_win/jansson.h"
#else
#include "../qcommon/jansson/jansson.h"
#endif // _WIN32

//==================================================================

// the "gameversion" client command will print this plus compile date
//----(SA) Wolfenstein
//#define GAMEVERSION "RtcwPro 1.0 beta"
#define JSONGAMESTATVERSION "0.1.4"

// done.

#define BODY_QUEUE_SIZE     8

#define INFINITE            1000000

#define FRAMETIME           100                 // msec
#define EVENT_VALID_MSEC    300
#define CARNAGE_REWARD_TIME 3000
#define REWARD_SPRITE_TIME  2000

#define INTERMISSION_DELAY_TIME 1000

#define MG42_MULTIPLAYER_HEALTH 350             // JPW NERVE

// gentity->flags
#define FL_GODMODE              0x00000010
#define FL_NOTARGET             0x00000020
#define FL_TEAMSLAVE            0x00000400  // not the first on the team
#define FL_NO_KNOCKBACK         0x00000800
#define FL_DROPPED_ITEM         0x00001000
#define FL_NO_BOTS              0x00002000  // spawn point not for bot use
#define FL_NO_HUMANS            0x00004000  // spawn point just for bots
#define FL_AI_GRENADE_KICK      0x00008000  // an AI has already decided to kick this grenade
// Rafael
#define FL_NOFATIGUE            0x00010000  // cheat flag no fatigue

#define FL_TOGGLE               0x00020000  //----(SA)	ent is toggling (doors use this for ex.)
#define FL_KICKACTIVATE         0x00040000  //----(SA)	ent has been activated by a kick (doors use this too for ex.)
#define FL_SOFTACTIVATE         0x00000040  //----(SA)	ent has been activated while 'walking' (doors use this too for ex.)
#define FL_DEFENSE_GUARD        0x00080000  // warzombie defense pose

#define FL_PARACHUTE            0x00100000
#define FL_WARZOMBIECHARGE      0x00200000
#define FL_NO_MONSTERSLICK      0x00400000
#define FL_NO_HEADCHECK         0x00800000

#define FL_NODRAW               0x01000000


#define MAX_NUM_MAPS 500
#define MAX_MAP_NAMELEN 50
// movers are things like doors, plats, buttons, etc
typedef enum {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_POS3,
	MOVER_1TO2,
	MOVER_2TO1,
	// JOSEPH 1-26-00
	MOVER_2TO3,
	MOVER_3TO2,
	// END JOSEPH

	// Rafael
	MOVER_POS1ROTATE,
	MOVER_POS2ROTATE,
	MOVER_1TO2ROTATE,
	MOVER_2TO1ROTATE
} moverState_t;


// door AI sound ranges
#define HEAR_RANGE_DOOR_LOCKED      128 // really close since this is a cruel check
#define HEAR_RANGE_DOOR_KICKLOCKED  512
#define HEAR_RANGE_DOOR_OPEN        256
#define HEAR_RANGE_DOOR_KICKOPEN    768

// DHM - Nerve :: Worldspawn spawnflags to indicate if a gametype is not supported
#define NO_GT_WOLF      1
#define NO_STOPWATCH    2
#define NO_CHECKPOINT   4

//============================================================================

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

//====================================================================
//
// Scripting, these structure are not saved into savegames (parsed each start)
typedef struct
{
	char    *actionString;
	qboolean ( *actionFunc )( gentity_t *ent, char *params );
} g_script_stack_action_t;
//
typedef struct
{
	//
	// set during script parsing
	g_script_stack_action_t     *action;            // points to an action to perform
	char                        *params;
} g_script_stack_item_t;
//
#define G_MAX_SCRIPT_STACK_ITEMS    64
//
typedef struct
{
	g_script_stack_item_t items[G_MAX_SCRIPT_STACK_ITEMS];
	int numItems;
} g_script_stack_t;
//
typedef struct
{
	int eventNum;                           // index in scriptEvents[]
	char                *params;            // trigger targetname, etc
	g_script_stack_t stack;
} g_script_event_t;
//
typedef struct
{
	char        *eventStr;
	qboolean ( *eventMatch )( g_script_event_t *event, char *eventParm );
} g_script_event_define_t;
//
// Script Flags
#define SCFL_GOING_TO_MARKER    0x1
#define SCFL_ANIMATING          0x2
//
// Scripting Status (NOTE: this MUST NOT contain any pointer vars)
typedef struct
{
	int scriptStackHead, scriptStackChangeTime;
	int scriptEventIndex;       // current event containing stack of actions to perform
	// scripting system variables
	int scriptId;                   // incremented each time the script changes
	int scriptFlags;
	char    *animatingParams;
} g_script_status_t;
//
#define G_MAX_SCRIPT_ACCUM_BUFFERS  8
//
void G_Script_ScriptEvent( gentity_t *ent, char *eventStr, char *params );
//====================================================================


#define CFOFS( x ) ( (int)&( ( (gclient_t *)0 )->x ) )

// RTCWPro
#define NUM_PING_SAMPLES 64

struct gentity_s {
	entityState_t s;                // communicated by server to clients
	entityShared_t r;               // shared by both the server system and game

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	struct gclient_s    *client;            // NULL if not a client

	qboolean inuse;

	char        *classname;         // set in QuakeEd
	int spawnflags;                 // set in QuakeEd

	qboolean neverFree;             // if true, FreeEntity will only unlink
									// bodyque uses this

	int flags;                      // FL_* variables

	char        *model;
	char        *model2;
	int freetime;                   // level.time when the object was freed

	int eventTime;                  // events will be cleared EVENT_VALID_MSEC after set
	qboolean freeAfterEvent;
	qboolean unlinkAfterEvent;

	qboolean physicsObject;         // if true, it can be pushed by movers and fall off edges
									// all game items are physicsObjects,
	float physicsBounce;            // 1.0 = continuous bounce, 0.0 = no bounce
	int clipmask;                   // brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance

	// movers
	moverState_t moverState;
	int soundPos1;
	int sound1to2;
	int sound2to1;
	int soundPos2;
	int soundLoop;
	// JOSEPH 1-26-00
	int sound2to3;
	int sound3to2;
	int soundPos3;
	// END JOSEPH

	int soundKicked;
	int soundKickedEnd;

	int soundSoftopen;
	int soundSoftendo;
	int soundSoftclose;
	int soundSoftendc;

	gentity_t   *parent;
	gentity_t   *nextTrain;
	gentity_t   *prevTrain;
	// JOSEPH 1-26-00
	vec3_t pos1, pos2, pos3;
	// END JOSEPH

	char        *message;

	int timestamp;              // body queue sinking, etc

	float angle;                // set in editor, -1 = up, -2 = down
	char        *target;
	char        *targetname;
	char        *team;
	char        *targetShaderName;
	char        *targetShaderNewName;
	gentity_t   *target_ent;

	float speed;
	float closespeed;           // for movers that close at a different speed than they open
	vec3_t movedir;

	int gDuration;
	int gDurationBack;
	vec3_t gDelta;
	vec3_t gDeltaBack;

	int nextthink;
	void ( *think )( gentity_t *self );
	void ( *reached )( gentity_t *self );       // movers call this when hitting endpoint
	void ( *blocked )( gentity_t *self, gentity_t *other );
	void ( *touch )( gentity_t *self, gentity_t *other, trace_t *trace );
	void ( *use )( gentity_t *self, gentity_t *other, gentity_t *activator );
	void ( *pain )( gentity_t *self, gentity_t *attacker, int damage, vec3_t point );
	void ( *die )( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod );

	// Nobo - More  (from rtcwPub)
	void		(*more)(gentity_t *ent);
	int			moreCalls;
	qboolean	moreCalled;

	int pain_debounce_time;
	int fly_sound_debounce_time;            // wind tunnel
	int last_move_time;

	int health;

	qboolean takedamage;

	int damage;
	int splashDamage;           // quad will increase this without increasing radius
	int splashRadius;
	int methodOfDeath;
	int splashMethodOfDeath;

	int count;

	gentity_t   *chain;
	gentity_t   *enemy;
	gentity_t   *activator;
	gentity_t   *teamchain;     // next entity in team
	gentity_t   *teammaster;    // master of the team

	int watertype;
	int waterlevel;

	int noise_index;

	// timing variables
	float wait;
	float random;

	// Rafael - sniper variable
	// sniper uses delay, random, radius
	int radius;
	float delay;

	// JOSEPH 10-11-99
	int TargetFlag;
	float duration;
	vec3_t rotate;
	vec3_t TargetAngles;
	// END JOSEPH

	gitem_t     *item;          // for bonus items

	// Ridah, AI fields
	char        *aiAttributes;
	char        *aiName;
	int aiTeam;
	void ( *AIScript_AlertEntity )( gentity_t *ent );
	qboolean aiInactive;
	int aiCharacter;            // the index of the type of character we are (from aicast_soldier.c)
	// done.

	char        *aiSkin;
	char        *aihSkin;

	vec3_t dl_color;
	char        *dl_stylestring;
	char        *dl_shader;
	int dl_atten;


	int key;                    // used by:  target_speaker->nopvs,

	qboolean active;
	qboolean botDelayBegin;

	// Rafael - mg42
	float harc;
	float varc;

	int props_frame_state;

	// Ridah
	int missionLevel;               // mission we are currently trying to complete
									// gets reset each new level
	// done.

	// Rafael
	qboolean is_dead;
	// done

	int start_size;
	int end_size;

	// Rafael props

	qboolean isProp;

	int mg42BaseEnt;

	gentity_t   *melee;

	char        *spawnitem;

	qboolean nopickup;

	int flameQuota, flameQuotaTime, flameBurnEnt;

	int count2;

	int grenadeExplodeTime;         // we've caught a grenade, which was due to explode at this time
	int grenadeFired;               // the grenade entity we last fired

	int mg42ClampTime;              // time to wait before an AI decides to ditch the mg42

	char        *track;

	// entity scripting system
	char                *scriptName;

	int numScriptEvents;
	g_script_event_t    *scriptEvents;  // contains a list of actions to perform for each event type
	g_script_status_t scriptStatus;     // current status of scripting
	// the accumulation buffer
	int scriptAccumBuffer[G_MAX_SCRIPT_ACCUM_BUFFERS];

	qboolean AASblocking;
	float accuracy;

	char        *tagName;       // name of the tag we are attached to
	gentity_t   *tagParent;

	float headshotDamageScale;

	int lastHintCheckTime;                  // DHM - Nerve
	// -------------------------------------------------------------------------------------------
	// if working on a post release patch, new variables should ONLY be inserted after this point
	// DHM - Nerve :: the above warning does not really apply to MP, but I'll follow it for good measure

	int voiceChatSquelch;                   // DHM - Nerve
	int voiceChatPreviousTime;              // DHM - Nerve
	int lastBurnedFrameNumber;              // JPW - Nerve   : to fix FT instant-kill exploit
	// OSPx
	qboolean	dmginloop;
	gentity_t	*dmgparent;
	int thrownKnifeTime;

	// RTCWPro - allowteams - ET port
	int allowteams;

	// player ammo
	int playerAmmo;
	int playerAmmoClip;
	int playerWeapon;
	int playerNades;

    // pause stuff from rtcwPub
	int			trType_pre_pause;
	vec3_t		trBase_pre_pause;

	// RTCWPro - head stuff
	qboolean	headshot;
	qboolean	is_head;
	gentity_t*  head;
};

// Ridah
#include "ai_cast_global.h"
// done.

typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum {
	SPECTATOR_NOT,
	SPECTATOR_FREE,
	SPECTATOR_FOLLOW,
	SPECTATOR_SCOREBOARD
} spectatorState_t;

typedef enum {
	TEAM_BEGIN,     // Beginning a team game, spawn at base
	TEAM_ACTIVE     // Now actively playing
} playerTeamStateState_t;

typedef struct {
	playerTeamStateState_t state;

	int location;

	int captures;
	int basedefense;
	int carrierdefense;
	int flagrecovery;
	int fragcarrier;
	int assists;

	float lasthurtcarrier;
	float lastreturnedflag;
	float flagsince;
	float lastfraggedcarrier;
} playerTeamState_t;

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define FOLLOW_ACTIVE1  -1
#define FOLLOW_ACTIVE2  -2

// L0 - OSP stats port - weapon stat counters
typedef struct {
	unsigned int atts;
	unsigned int deaths;
	unsigned int headshots;
	unsigned int hits;
	unsigned int kills;
} weapon_stat_t;
// End

// RTCWPro - allowteams - ET port
#define ALLOW_AXIS_TEAM         1
#define ALLOW_ALLIED_TEAM       2

// RTCWPro - drop weapon stuff
#define WEP_DROP_SOLDIER 1
#define WEP_DROP_ENG 2
#define WEP_DROP_MEDIC 4
#define WEP_DROP_LT 8

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct {
	team_t sessionTeam;
	int spectatorTime;              // for determining next-in-line to play
	spectatorState_t spectatorState;
	int spectatorClient;            // for chasecam and follow mode
	int start_time;                 // player starts/begins game
	int end_time;                   // player ends/leaves game
	int wins, losses;               // tournament stats
	int playerType;                 // DHM - Nerve :: for GT_WOLF
	int playerWeapon;               // DHM - Nerve :: for GT_WOLF
	int playerItem;                 // DHM - Nerve :: for GT_WOLF
	int playerSkin;                 // DHM - Nerve :: for GT_WOLF
	int spawnObjectiveIndex;         // JPW NERVE index of objective to spawn nearest to (returned from UI)
	int latchPlayerType;            // DHM - Nerve :: for GT_WOLF not archived
	int latchPlayerWeapon;          // DHM - Nerve :: for GT_WOLF not archived
	int latchPlayerItem;            // DHM - Nerve :: for GT_WOLF not archived
	int latchPlayerSkin;            // DHM - Nerve :: for GT_WOLF not archived

	// L0 - New sessions
	unsigned int uci;   // mcwf's GeoIP
//	unsigned char ip[4];// IPs
	char ip[47];		// IP
	//char guid[15];		// Guid
	char guid[GUID_LEN];		// Guid
	int ignoreClients[MAX_CLIENTS / ( sizeof( int ) * 8 )];
	qboolean muted;
	int selectedWeapon; // If enabled allows mp40, sten, thompson..
	// OSP port
	int damage_given;
	int damage_received;
	int deaths;
	int kills;
	int rounds;
	int suicides;
	int team_damage;
	int team_kills;

	// referee
	int referee, status, shoutcaster;
	int spec_invite, specInvited, specLocked;

	// New ones
	int headshots;
	int med_given;
	int ammo_given;
	int gibs;
	int poisoned;
	int revives;
	int acc_shots;  // Overall acc
	int acc_hits;	// -||-
	int killPeak;
	int knifeKills;
	int obj_captured;
	int obj_destroyed;
	int obj_returned;
	int obj_taken;
	int obj_checkpoint;
	int dyn_planted;
	int dyn_defused;
	weapon_stat_t aWeaponStats[WS_MAX + 1];   // Weapon stats.  +1 to avoid invalid weapon check
	//weapon_stat_t aWeaponStats[WS_MAX + 1];   // Weapon stats.  +1 to avoid invalid weapon check

	int clientFlags;		// Sort some stuff based upon user settings
	int specSpeed;
} clientSession_t;

//
#define MAX_NETNAME         36
#define MAX_VOTE_COUNT      3

#define PICKUP_ACTIVATE 0   // pickup items only when using "+activate"
#define PICKUP_TOUCH    1   // pickup items when touched
#define PICKUP_FORCE    2   // pickup the next item when touched (and reset to PICKUP_ACTIVATE when done)

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct {
	clientConnected_t connected;
	usercmd_t cmd;                  // we would lose angles if not persistant
	usercmd_t oldcmd;               // previous command processed by pmove()
	qboolean localClient;           // true if "ip" info key is "localhost"
	qboolean initialSpawn;          // the first spawn should be at a cool location
	qboolean predictItemPickup;     // based on cg_predictItems userinfo
	qboolean pmoveFixed;            //
	char netname[MAX_NETNAME];

	int autoActivate;               // based on cg_autoactivate userinfo		(uses the PICKUP_ values above)
	int emptySwitch;                // based on cg_emptyswitch userinfo (means "switch my weapon for me when ammo reaches '0' rather than -1)

	int maxHealth;                  // for handicapping
	int enterTime;                  // level.time the client entered the game
	int connectTime;                // DHM - Nerve :: level.time the client first connected to the server
	playerTeamState_t teamState;    // status in teamplay games
	int voteCount;                  // to prevent people from constantly calling votes
	int teamVoteCount;              // to prevent people from constantly calling votes

	int complaints;                     // DHM - Nerve :: number of complaints lodged against this client
	int complaintClient;                // DHM - Nerve :: able to lodge complaint against this client
	int complaintEndTime;               // DHM - Nerve :: until this time has expired

	int lastReinforceTime;              // DHM - Nerve :: last reinforcement

	qboolean teamInfo;              // send team overlay updates?

	qboolean bAutoReloadAux; // TTimo - auxiliary storage for pmoveExt_t::bAutoReload, to achieve persistance
	// L0
	unsigned int autoaction;            // End-of-match auto-requests
	unsigned int clientFlags;           // Client settings that need server involvement
	unsigned int clientMaxPackets;      // Client com_maxpacket settings
	unsigned int clientTimeNudge;       // Client cl_timenudge settings
	unsigned int hitSoundType;
	unsigned int hitSoundBodyStyle;
	unsigned int hitSoundHeadStyle;
	int cmd_debounce;                   // Dampening of command spam
	unsigned int invite;                // Invitation to a team to join
	int throwingKnives;

	// Shortcuts
	int lastkilled_client;
	int	lastrevive_client;
	int	lastkiller_client;
	int	lastammo_client;
	int	lasthealth_client;

	// Life stats
	int life_acc_shots;
	int life_acc_hits;
	int life_headshots;
	int life_kills;
	int life_gibs;
	unsigned int int_stats;
	unsigned int int_statsType;
//	unsigned int int_dragBodies;
	unsigned int int_selectedWeapon;
	// tardo
	qboolean ready;
	int restrictedWeapon;
	qboolean drawHitBoxes;
	qboolean findMedic;
	// g_alternatePing from rtcwPub
	int	alternatePing;
	int	pingsamples[NUM_PING_SAMPLES];
	int	samplehead;
} clientPersistant_t;

// L0 - antilag port
#define NUM_CLIENT_TRAILS 64
typedef struct {
    vec3_t mins, maxs;
    vec3_t currentOrigin;
    int time;
	clientAnimationInfo_t animInfo;
} clientTrail_t;

// L0 - AntiWarp
#define LAG_MAX_COMMANDS 512
#define LAG_MAX_DELTA 75
#define LAG_MAX_DROP_THRESHOLD 800
#define LAG_MIN_DROP_THRESHOLD ( LAG_MAX_DROP_THRESHOLD - 200 )
#define LAG_DECAY 1.02f
// End

typedef struct {
	vec3_t mins;
	vec3_t maxs;

	vec3_t origin;

	int time;
	int servertime;
} clientMarker_t;

#define MAX_CLIENT_MARKERS 10


#define LT_SPECIAL_PICKUP_MOD   3       // JPW NERVE # of times (minus one for modulo) LT must drop ammo before scoring a point
#define MEDIC_SPECIAL_PICKUP_MOD    4   // JPW NERVE same thing for medic
#define CMD_DEBOUNCE    5000    // 5s between cmds

// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
	// ps MUST be the first element, because the server expects it
	playerState_t ps;               // communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t pers;
	clientSession_t sess;

	qboolean readyToExit;           // wishes to leave the intermission

	qboolean noclip;

	int lastCmdTime;                // level.time of last usercmd_t, for EF_CONNECTION
									// we can't just use pers.lastCommand.time, because
									// of the g_sycronousclients case
	int buttons;
	int oldbuttons;
	int latched_buttons;

	int wbuttons;
	int oldwbuttons;
	int latched_wbuttons;
	vec3_t oldOrigin;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int damage_armor;               // damage absorbed by armor
	int damage_blood;               // damage taken out of health
	int damage_knockback;           // impact damage
	vec3_t damage_from;             // origin for vector calculation
	qboolean damage_fromWorld;      // if true, don't use the damage_from vector

	int accurateCount;              // for "impressive" reward sound

	int accuracy_shots;             // total number of shots
	int accuracy_hits;              // total number of hits

	//
	int lastkilled_client;          // last client that this client killed
	int lasthurt_client;            // last client that damaged this client
	int lasthurt_mod;               // type of damage the client did

	// timers

	int inactivityTime;             // kick players when time > this
	qboolean inactivityWarning;     // qtrue if the five seoond warning has been given
	int rewardTime;                 // clear the EF_AWARD_IMPRESSIVE, etc when time > this

	int airOutTime;

	int lastKillTime;               // for multiple kill rewards

	qboolean fireHeld;              // used for hook
	gentity_t   *hook;              // grapple hook if out

	int switchTeamTime;             // time the player switched teams

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int timeResidual;

	float currentAimSpreadScale;

	int medicHealAmt;

	// RF, may be shared by multiple clients/characters
	animModelInfo_t *modelInfo;

	// -------------------------------------------------------------------------------------------
	// if working on a post release patch, new variables should ONLY be inserted after this point

	gentity_t   *persistantPowerup;
	int portalID;
	int ammoTimes[WP_NUM_WEAPONS];
	int invulnerabilityTime;

	gentity_t   *cameraPortal;              // grapple hook if out
	vec3_t cameraOrigin;

	int dropWeaponTime;         // JPW NERVE last time a weapon was dropped
	int limboDropWeapon;         // JPW NERVE weapon to drop in limbo
	int deployQueueNumber;         // JPW NERVE player order in reinforcement FIFO queue
	int sniperRifleFiredTime;         // JPW NERVE last time a sniper rifle was fired (for muzzle flip effects)
	float sniperRifleMuzzleYaw;       // JPW NERVE for time-dependent muzzle flip in multiplayer
	int lastBurnTime;         // JPW NERVE last time index for flamethrower burn
	int PCSpecialPickedUpCount;         // JPW NERVE used to count # of times somebody's picked up this LTs ammo (or medic health) (for scoring)
	int saved_persistant[MAX_PERSISTANT];           // DHM - Nerve :: Save ps->persistant here during Limbo
/*
	// g_antilag.c
	int topMarker;
	clientMarker_t clientMarkers[MAX_CLIENT_MARKERS];
	clientMarker_t backupMarker;

	gentity_t       *tempHead;  // Gordon: storing a temporary head for bullet head shot detection

	pmoveExt_t pmext;
*/

	clientAnimationInfo_t animationInfo;
	float legsYawAngle, torsoYawAngle, torsoPitchAngle;
	qboolean torsoYawing, legsYawing, torsoPitching;


	// g_antilag.c
	// L0 - antilag port
    int trailHead;
    clientTrail_t trail[NUM_CLIENT_TRAILS];
	int last_trail_node_store_time;
	int accum_trail_node_store_time;
	clientTrail_t saved_trail_node;
	// antilag end

	gentity_t		*tempHead;	// Gordon: storing a temporary head for bullet head shot detection

	pmoveExt_t	pmext;

	// L0 - New stuff
	int			doublekill;		// (stats) Double+ Kills
	int			infoTime;		// LT/spies Info
	int respawnTime;                        ///< can respawn when time > this, force after g_forcerespwan
	// AntiWarp
	int lastCmdRealTime;
	int cmdhead;							// antiwarp command queue head
	int cmdcount;							// antiwarp command queue # valid commands
	float cmddelta;							// antiwarp command queue # valid commands
	usercmd_t cmds[LAG_MAX_COMMANDS];       // antiwarp command queue
	// End

	// revive anim bug fix
	qboolean revive_animation_playing;
	int movement_lock_begin_time;

};

//
// this structure is cleared as each map is entered
//
#define MAX_SPAWN_VARS          64
#define MAX_SPAWN_VARS_CHARS    2048
#define VOTE_MAXSTRING          256     // Same value as MAX_STRING_TOKENS

typedef struct voteInfo_s {
	char voteString[MAX_STRING_CHARS];
	int voteTime;                       // level.time vote was called
	int voteYes;
	int voteNo;
	int numVotingClients;               // set by CalculateRanks
	int numVotingTeamClients[2];
	int ( *vote_fn )( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
	char vote_value[VOTE_MAXSTRING];        // Desired vote item setting.
} voteInfo_t;

typedef struct jsonStatInfo_s {
   char  match_id[MAX_STRING_CHARS];
   char  round_id[MAX_STRING_CHARS];
   char  round_start[MAX_STRING_CHARS];
   char  round_timelimit[MAX_STRING_CHARS];
   char  gameStatslogFileName[256];
   fileHandle_t gameStatslogFile; // for outputting events in a nice format (possibly temporary) - nihi
} jsonStatInfo_t;


typedef struct {
	struct gclient_s    *clients;       // [maxclients]

	struct gentity_s    *gentities;
	int gentitySize;
	int num_entities;               // current number, <= MAX_GENTITIES

	int warmupTime;                 // restart match at this time
	qboolean warmupSwap;			// Swaps teams in SW with g_tournament enabled

	fileHandle_t logFile;


	// store latched cvars here that we want to get at often
	int maxclients;

	int framenum;
	int time;                           // in msec
	int previousTime;                   // so movers can back up when blocked
	int frameTime;                      // Gordon: time the frame started, for antilag stuff
	int frameStartTime;					// L0 - antilag port - actual time frame started
	int startTime;                      // level.time the map was started

	int teamScores[TEAM_NUM_TEAMS];
	int lastTeamLocationTime;               // last time of client team location update

	qboolean newSession;                // don't use any old session data, because
										// we changed gametype

	qboolean restarted;                 // waiting for a map_restart to fire

	int numConnectedClients;
	int numNonSpectatorClients;         // includes connecting clients
	int numPlayingClients;              // connected, non-spectators
	int sortedClients[MAX_CLIENTS];             // sorted by score
	int follow1, follow2;               // clientNums for auto-follow spectators

	int snd_fry;                        // sound index for standing in lava

	int warmupModificationCount;            // for detecting if g_warmup is changed

	// voting state
	char voteString[MAX_STRING_CHARS];
	char voteDisplayString[MAX_STRING_CHARS];
	int voteTime;                       // level.time vote was called
	int voteExecuteTime;                // time the vote is executed
	int prevVoteExecuteTime;            // JPW NERVE last vote execute time
	int voteYes;
	int voteNo;
	int numVotingClients;               // set by CalculateRanks

	// team voting state
	char teamVoteString[2][MAX_STRING_CHARS];
	int teamVoteTime[2];                // level.time vote was called
	int teamVoteYes[2];
	int teamVoteNo[2];
	int numteamVotingClients[2];        // set by CalculateRanks

	// spawn variables
	qboolean spawning;                  // the G_Spawn*() functions are valid
	int numSpawnVars;
	char        *spawnVars[MAX_SPAWN_VARS][2];  // key / value pairs
	int numSpawnVarChars;
	char spawnVarChars[MAX_SPAWN_VARS_CHARS];

	// intermission state
	int intermissionQueued;             // intermission was qualified, but
										// wait INTERMISSION_DELAY_TIME before
										// actually going there so the last
										// frag can be watched.  Disable future
										// kills during this delay
	int intermissiontime;               // time the intermission was started
	char        *changemap;
	qboolean readyToExit;               // at least one client wants to exit
	int exitTime;
	vec3_t intermission_origin;         // also used for spectator spawns
	vec3_t intermission_angle;

	qboolean locationLinked;            // target_locations get linked
	gentity_t   *locationHead;          // head of the location list
	int bodyQueIndex;                   // dead bodies
	gentity_t   *bodyQue[BODY_QUEUE_SIZE];

	int portalSequence;
	// Ridah
	char        *scriptAI;
	int reloadPauseTime;                // don't think AI/client's until this time has elapsed
	int reloadDelayTime;                // don't start loading the savegame until this has expired

	int lastGrenadeKick;

	int loperZapSound;
	int stimSoldierFlySound;
	int bulletRicochetSound;
	// done.

	int snipersound;

	//----(SA)	added
	int knifeSound[4];
	//----(SA)	end

// JPW NERVE
	int capturetimes[4];         // red, blue, none, spectator for WOLF_MP_CPH
	int redReinforceTime, blueReinforceTime;         // last time reinforcements arrived in ms
	int redNumWaiting, blueNumWaiting;         // number of reinforcements in queue
	vec3_t spawntargets[MAX_MULTI_SPAWNTARGETS];      // coordinates of spawn targets
	int numspawntargets;         // # spawntargets in this map
// jpw

	// RF, entity scripting
	char        *scriptEntity;

	// player/AI model scripting (server repository)
	animScriptData_t animScriptData;

	// NERVE - SMF - debugging/profiling info
	int totalHeadshots;
	int missedHeadshots;
	qboolean lastRestartTime;
	// -NERVE - SMF

	int numFinalDead[2];                // DHM - Nerve :: unable to respawn and in limbo (per team)
	int numOidTriggers;                 // DHM - Nerve

	qboolean latchGametype;             // DHM - Nerve

// L0 - New stuff
	int axisLeft;		// For DM
	int alliedLeft;		// For DM
	int dwBlueReinfOffset;	// Reinforcements offset
	int dwRedReinfOffset;	// Reinforcements offset
	int	axisPlayers;		// For auto lock and auto team balance
	int alliedPlayers;		// For auto lock and auto team balance
	int taken;			// Flag retaking
	int balanceTimer;	// Auto balance teams timer
	qboolean fResetStats; // OSP Stats

	// Countdown
	qboolean	cnStarted;
	int			cnPush;
	int			cnNum;

	// voting and referee
	voteInfo_t voteInfo;
	int server_settings;

	// Weapons restrictions
	int axisSniper, alliedSniper;
	int axisPF, alliedPF;
	int axisVenom, alliedVenom;
	int axisFlamer, alliedFlamer;

	// Pause
	int paused;
	int timeCurrent;	// Real game clock
	int timeDelta;
	int axisTimeouts;
	int alliedTimeouts;
	qboolean axisCalledTimeout;
	qboolean autoPaused;

	// OSP Stats
	int sortedStats[MAX_CLIENTS];	// sorted by weapon stats

	// Map Achievers
	int topAchiever;
	char *topAchieverPlayer;

	// Ready
	qboolean ref_allready;                  // Referee forced match start
	qboolean readyAll;
	qboolean readyPrint;
	qboolean readyTeam[TEAM_NUM_TEAMS];

	// Forced/Instant tapout timer to cope with flood..
	int spawnFloodTimer;
	int svCvarsCount;

    char maplist[MAX_NUM_MAPS][MAX_MAP_NAMELEN];
	int mapcount;

	int eventNum;  // event counter
	jsonStatInfo_t jsonStatInfo;  // for stats match/round info
	char* match_id; // for stats round matching...
    char* round_id; //
} level_locals_t;

// OSPx - Team extras
typedef struct {
	qboolean spec_lock;
	qboolean team_lock;
	char team_name[24];
	int timeouts;
} team_info;
// -OSP

extern qboolean reloading;                  // loading up a savegame
// JPW NERVE
extern char testid1[];
extern char testid2[];
extern char testid3[];
// jpw

//
// g_spawn.c
//
qboolean    G_SpawnString( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean    G_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean    G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean    G_SpawnVector( const char *key, const char *defaultString, float *out );
void        G_SpawnEntitiesFromString( void );
char *G_NewString( const char *string );
// Ridah
qboolean G_CallSpawn( gentity_t *ent );
// done.

//
// g_cmds.c
//
void Cmd_Score_f( gentity_t *ent );
void StopFollowing( gentity_t *ent );
//void BroadcastTeamChange( gclientgclient_t *client, int oldTeam );
void SetTeam( gentity_t *ent, char *s, qboolean forced );
void SetWolfData( gentity_t *ent, char *ptype, char *weap, char *grenade, char *skinnum );  // DHM - Nerve
void Cmd_FollowCycle_f( gentity_t *ent, int dir );

//
// g_items.c
//
void G_CheckTeamItems( void );
void G_RunItem( gentity_t *ent );
void RespawnItem( gentity_t *ent );

void UseHoldableItem( gentity_t *ent, int item );
void PrecacheItem( gitem_t *it );
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle, qboolean novelocity );
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity, int ownerNum );
void SetRespawn( gentity_t *ent, float delay );
void G_SpawnItem( gentity_t *ent, gitem_t *item );
void FinishSpawningItem( gentity_t *ent );
void Think_Weapon( gentity_t *ent );
int ArmorIndex( gentity_t *ent );
void Fill_Clip( playerState_t *ps, int weapon );
void    Add_Ammo( gentity_t *ent, int weapon, int count, qboolean fillClip );
void Touch_Item( gentity_t *ent, gentity_t *other, trace_t *trace );

// Touch_Item_Auto is bound by the rules of autoactivation (if cg_autoactivate is 0, only touch on "activate")
void Touch_Item_Auto( gentity_t *ent, gentity_t *other, trace_t *trace );

void ClearRegisteredItems( void );
void RegisterItem( gitem_t *item );
void SaveRegisteredItems( void );
void Prop_Break_Sound( gentity_t *ent );
void Spawn_Shard( gentity_t *ent, gentity_t *inflictor, int quantity, int type );

//
// g_utils.c
//
// Ridah
int G_FindConfigstringIndex( const char *name, int start, int max, qboolean create );
// done.
int G_ModelIndex( char *name );
int     G_SoundIndex( const char *name );
void    G_TeamCommand( team_t team, char *cmd );
void    G_KillBox( gentity_t *ent );
gentity_t *G_Find( gentity_t *from, int fieldofs, const char *match );
gentity_t *G_PickTarget( char *targetname );
void    G_UseTargets( gentity_t *ent, gentity_t *activator );
void    G_SetMovedir( vec3_t angles, vec3_t movedir );

void    G_InitGentity( gentity_t *e );
gentity_t   *G_Spawn( void );
gentity_t *G_TempEntity( vec3_t origin, int event );
void    G_Sound( gentity_t *ent, int soundIndex );
void    G_AnimScriptSound( int soundIndex, vec3_t org, int client );
void    G_FreeEntity( gentity_t *e );
//qboolean	G_EntitiesFree( void );

void    G_TouchTriggers( gentity_t *ent );
void    G_TouchSolids( gentity_t *ent );

float   *tv( float x, float y, float z );
char    *vtos( const vec3_t v );

void G_AddPredictableEvent( gentity_t *ent, int event, int eventParm );
void G_AddEvent( gentity_t *ent, int event, int eventParm );
void G_SetOrigin( gentity_t *ent, vec3_t origin );
void AddRemap( const char *oldShader, const char *newShader, float timeOffset );
const char *BuildShaderStateConfig();
void G_SetAngle( gentity_t *ent, vec3_t angle );

qboolean infront( gentity_t *self, gentity_t *other );

void G_ProcessTagConnect( gentity_t *ent );

qboolean G_AllowTeamsAllowed(gentity_t* ent, gentity_t* activator); // RTCWPro - allowteams ET - port
qboolean AllowDropForClass(gentity_t* ent, int pclass); // RTCWPro - drop weapon stuff
gentity_t* GetClientEntity(gentity_t* ent, char* cNum, gentity_t** found);
char* getDateTime(void);
char* getDate(void);
const char* getMonthString(int monthIndex);
int getYearFromCYear(int cYear);
int getDaysInMonth(int monthIndex);
char* TablePrintableColorName(const char* name, int maxlength);
qboolean FileExists(char* filename, char* directory, char* expected_extension, qboolean can_have_extension);
qboolean G_SpawnEnts(gentity_t* ent);
int G_FindMatchingMaps(gentity_t* ent, char* mapName);

//
// g_combat.c
//
qboolean CanDamage( gentity_t *targ, vec3_t origin );
void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod );
qboolean G_RadiusDamage( vec3_t origin, gentity_t *attacker, float damage, float radius, gentity_t *ignore, int mod );
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void TossClientItems( gentity_t *self );
//gentity_t* G_BuildHead( gentity_t *ent ); // RTCWPro - unused

// damage flags
#define DAMAGE_RADIUS           0x00000001  // damage was indirect
#define DAMAGE_NO_ARMOR         0x00000002  // armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK     0x00000008  // do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION    0x00000020  // armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_NO_TEAM_PROTECTION   0x00000010  // armor, shields, invulnerability, and godmode have no effect

//
// g_missile.c
//
void G_RunMissile( gentity_t *ent );
int G_PredictMissile( gentity_t *ent, int duration, vec3_t endPos, qboolean allowBounce );

// Rafael zombiespit
void G_RunDebris( gentity_t *ent );

//DHM - Nerve :: server side flamethrower collision
void G_RunFlamechunk( gentity_t *ent );

//----(SA) removed unused q3a weapon firing
gentity_t *fire_flamechunk( gentity_t *self, vec3_t start, vec3_t dir );

gentity_t *fire_grenade( gentity_t *self, vec3_t start, vec3_t aimdir, int grenadeWPID );
gentity_t *fire_rocket( gentity_t *self, vec3_t start, vec3_t dir );
gentity_t *fire_speargun( gentity_t *self, vec3_t start, vec3_t dir );

//----(SA)	added from MP
gentity_t *fire_nail( gentity_t *self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up );
gentity_t *fire_prox( gentity_t *self, vec3_t start, vec3_t aimdir );
//----(SA)	end

// Rafael sniper
void fire_lead( gentity_t *self,  vec3_t start, vec3_t dir, int damage );
qboolean visible( gentity_t *self, gentity_t *other );

gentity_t *fire_mortar( gentity_t *self, vec3_t start, vec3_t dir );
gentity_t *fire_flamebarrel( gentity_t *self, vec3_t start, vec3_t dir );
// done

//
// g_mover.c
//
gentity_t *G_TestEntityPosition( gentity_t *ent );
void G_RunMover( gentity_t *ent );
void Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator );
void G_Activate( gentity_t *ent, gentity_t *activator );

void G_TryDoor( gentity_t *ent, gentity_t *other, gentity_t *activator ); //----(SA)	added

void InitMoverRotate( gentity_t *ent );

void InitMover( gentity_t *ent );
void SetMoverState( gentity_t *ent, moverState_t moverState, int time );

//
// g_tramcar.c
//
void Reached_Tramcar( gentity_t *ent );


//
// g_misc.c
//
void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles );
void mg42_fire( gentity_t *other );


//
// g_weapon.c
//
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker );
void CalcMuzzlePoint( gentity_t *ent, int weapon, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint );
void SnapVectorTowards( vec3_t v, vec3_t to );
trace_t *CheckMeleeAttack( gentity_t *ent, float dist, qboolean isTest );
gentity_t *weapon_grenadelauncher_fire( gentity_t *ent, int grenadeWPID );
// Rafael

void CalcMuzzlePoints( gentity_t *ent, int weapon );

// Rafael - for activate
void CalcMuzzlePointForActivate( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint );
// done.

// sswolf - RTCWPro stuff
void AddHeadEntity(gentity_t* ent);
void FreeHeadEntity(gentity_t* ent);
void UpdateHeadEntity(gentity_t* ent);
void RemoveHeadEntity(gentity_t* ent);
qboolean ReviveEntity(gentity_t* ent, gentity_t* traceEnt);

//
// g_client.c
//
team_t TeamCount( int ignoreClientNum, int team );          // NERVE - SMF - merge from team arena
team_t PickTeam( int ignoreClientNum );
void SetClientViewAngle( gentity_t *ent, vec3_t angle );
gentity_t *SelectSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles );
void respawn( gentity_t *ent );
void BeginIntermission( void );
void InitClientPersistant( gclient_t *client );
void InitClientResp( gclient_t *client );
void InitBodyQue( void );
char* SanitizeClientIP(char* ip, qboolean printFull);
void ClientSpawn( gentity_t *ent, qboolean revived );
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod );
void AddScore( gentity_t *ent, int score );
void CalculateRanks( void );
qboolean SpotWouldTelefrag( gentity_t *spot );
void limbo(gentity_t* ent, qboolean makeCorpse);

//void RemoveWeaponRestrictions(gentity_t *ent);
//void ResetTeamWeaponRestrictions(int clientNum, team_t team, weapon_t enumWeapon, int weapon);


// RTCWPro - custom config - g_sha1.c
char* G_SHA1(const char* string);
//
// g_svcmds.c
//
qboolean    ConsoleCommand( void );
void G_ProcessIPBans( void );
qboolean G_FilterIPBanPacket( char *from );
qboolean G_FilterMaxLivesPacket( char *from );
qboolean G_FilterMaxLivesIPPacket( char *from );
void AddMaxLivesGUID( char *str );
void AddMaxLivesBan( const char *str );
void ClearMaxLivesBans();
void AddIPBan( const char *str );

void Svcmd_ShuffleTeams_f( void );
void Svcmd_StartMatch_f( void );
void Svcmd_ResetMatch_f(qboolean fDoReset, qboolean fDoRestart);
void Svcmd_SwapTeams_f( void );

//
// g_weapon.c
//
void G_BurnMeGood( gentity_t *self, gentity_t *body );
void FireWeapon( gentity_t *ent );

//
// p_hud.c
//
void MoveClientToIntermission( gentity_t *client );
void G_SetStats( gentity_t *ent );
void DeathmatchScoreboardMessage( gentity_t *client );

//
// g_cmds.c
//
void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, qboolean localize ); // JPW NERVE removed static declaration so it would link
qboolean Cmd_CallVote_f(gentity_t *ent, qboolean fRefCommand);
void SanitizeString(char* in, char* out);

//
// g_pweapon.c
//


//
// g_main.c
//
void FindIntermissionPoint( void );
void G_RunThink( gentity_t *ent );
void QDECL G_LogPrintf( const char *fmt, ... );
void SendScoreboardMessageToAllClients( void );
void QDECL G_Printf( const char *fmt, ... );
void QDECL G_DPrintf( const char *fmt, ... );
void QDECL G_Error( const char *fmt, ... );
void CheckVote(void);
void sortedActivePlayers(void);

//
// g_client.c
//
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
void ClientBegin( int clientNum );
void ClientCommand( int clientNum );

//
// g_active.c
//
void ClientThink( int clientNum );
void ClientEndFrame( gentity_t *ent );
void G_RunClient( gentity_t *ent );

//
// g_team.c
//
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 );
qboolean G_allowFollow( gentity_t *ent, int nTeam );

void G_shuffleTeams( void );

void G_swapTeams( void );
int G_blockoutTeam( gentity_t *ent, int nTeam );
qboolean G_desiredFollow( gentity_t *ent, int nTeam );
void G_swapTeamLocks( void );
void G_updateSpecLock( int nTeam, qboolean fLock );
void G_removeSpecInvite( int team );
qboolean G_playersReady( void );
void G_readyReset( qboolean aForced );
void G_readyResetOnPlayerLeave(int team);
void G_readyStart( void );
void G_readyTeamLock( void );

//
// g_mem.c
//
qboolean G_CanAlloc(unsigned int size);
void *G_Alloc(unsigned int size);
void G_Free(void *ptr);
void G_InitMemory( void );
void Svcmd_GameMem_f( void );

//
// g_session.c
//
void G_ReadSessionData( gclient_t *client );
void G_InitSessionData( gclient_t *client, char *userinfo );
void G_ClientSwap( gclient_t *client );
void G_InitWorldSession( void );
void G_WriteSessionData( void );

//
// g_bot.c
//
void G_InitBots( qboolean restart );
char *G_GetBotInfoByNumber( int num );
char *G_GetBotInfoByName( const char *name );
void G_CheckBotSpawn( void );
void G_QueueBotBegin( int clientNum );
qboolean G_BotConnect( int clientNum, qboolean restart );
void Svcmd_AddBot_f( void );

// ai_main.c
#define MAX_FILEPATH            144

//bot settings
typedef struct bot_settings_s
{
	char characterfile[MAX_FILEPATH];
	float skill;
	char team[MAX_FILEPATH];
} bot_settings_t;

int BotAISetup( int restart );
int BotAIShutdown( int restart );
int BotAILoadMap( int restart );
int BotAISetupClient( int client, struct bot_settings_s *settings );
int BotAIShutdownClient( int client );
int BotAIStartFrame( int time );
void BotTestAAS( vec3_t origin );


// g_cmd.c
void Cmd_Activate_f( gentity_t *ent );
int Cmd_WolfKick_f( gentity_t *ent );
// Ridah

// g_save.c
/*qboolean G_SaveGame(char *username);
void G_LoadGame(char *username);
qboolean G_SavePersistant(char *nextmap);
void G_LoadPersistant(void);
void G_UpdatePlayTime ( void );*/

// g_script.c
void G_Script_ScriptParse( gentity_t *ent );
qboolean G_Script_ScriptRun( gentity_t *ent );
void G_Script_ScriptEvent( gentity_t *ent, char *eventStr, char *params );
void G_Script_ScriptLoad( void );

float AngleDifference( float ang1, float ang2 );

// g_props.c
void Props_Chair_Skyboxtouch( gentity_t *ent );

#include "g_team.h" // teamplay specific stuff


extern level_locals_t level;
extern gentity_t g_entities[];          //DAJ was explicit set to MAX_ENTITIES
extern gentity_t       *g_camEnt;

#define FOFS( x ) ( (int)&( ( (gentity_t *)0 )->x ) )

extern vmCvar_t g_gametype;

// Rafael gameskill
extern vmCvar_t g_gameskill;
// done
extern vmCvar_t g_gameStatslog; // temp cvar for event logging
extern vmCvar_t g_stats_curl_submit;
extern vmCvar_t g_stats_curl_submit_URL;
extern vmCvar_t g_stats_curl_submit_headers;

extern vmCvar_t g_dedicated;
extern vmCvar_t g_cheats;
extern vmCvar_t g_maxclients;               // allow this many total, including spectators
extern vmCvar_t g_maxGameClients;           // allow this many active
extern vmCvar_t g_minGameClients;           // NERVE - SMF - we need at least this many before match actually starts
extern vmCvar_t g_restarted;

extern vmCvar_t g_dmflags;
extern vmCvar_t g_fraglimit;
extern vmCvar_t g_timelimit;
extern vmCvar_t g_capturelimit;
extern vmCvar_t g_friendlyFire;
extern vmCvar_t g_password;
extern vmCvar_t g_needpass;
extern vmCvar_t g_gravity;
extern vmCvar_t g_speed;
extern vmCvar_t g_knockback;
extern vmCvar_t g_quadfactor;
extern vmCvar_t g_forcerespawn;
extern vmCvar_t g_inactivity;
extern vmCvar_t g_debugMove;
extern vmCvar_t g_debugAlloc;
extern vmCvar_t g_debugDamage;
extern vmCvar_t g_debugBullets;     //----(SA)	added
extern vmCvar_t g_preciseHeadHitBox;
extern vmCvar_t g_weaponRespawn;
extern vmCvar_t g_synchronousClients;
extern vmCvar_t g_motd;
extern vmCvar_t g_warmup;
extern vmCvar_t g_voteFlags;

// DHM - Nerve :: The number of complaints allowed before kick/ban
extern vmCvar_t g_complaintlimit;
extern vmCvar_t g_maxlives;                 // DHM - Nerve :: number of respawns allowed (0==infinite)
extern vmCvar_t g_voiceChatsAllowed;        // DHM - Nerve :: number before spam control
extern vmCvar_t g_alliedmaxlives;           // Xian
extern vmCvar_t g_axismaxlives;             // Xian
extern vmCvar_t g_fastres;                  // Xian - Fast medic res'ing
extern vmCvar_t g_fastResMsec;
extern vmCvar_t g_knifeonly;                // Xian - Wacky Knife-Only rounds
extern vmCvar_t g_enforcemaxlives;          // Xian - Temp ban with maxlives between rounds

extern vmCvar_t g_weaponTeamRespawn;
extern vmCvar_t g_doWarmup;
extern vmCvar_t g_teamAutoJoin;
extern vmCvar_t g_teamForceBalance;
extern vmCvar_t g_banIPs;
extern vmCvar_t g_filterBan;
extern vmCvar_t g_rankings;
extern vmCvar_t g_enableBreath;
extern vmCvar_t g_smoothClients;
extern vmCvar_t pmove_fixed;
extern vmCvar_t pmove_msec;

//Rafael
extern vmCvar_t g_autoactivate;

extern vmCvar_t g_testPain;

extern vmCvar_t g_missionStats;
extern vmCvar_t ai_scriptName;          // name of AI script file to run (instead of default for that map)
extern vmCvar_t g_scriptName;           // name of script file to run (instead of default for that map)

extern vmCvar_t g_scriptDebug;

extern vmCvar_t g_userAim;

extern vmCvar_t g_forceModel;

extern vmCvar_t g_mg42arc;

extern vmCvar_t g_footstepAudibleRange;
// JPW NERVE multiplayer
extern vmCvar_t g_redlimbotime;
extern vmCvar_t g_bluelimbotime;
extern vmCvar_t g_medicChargeTime;
//extern vmCvar_t g_asoffset; // temporary for adjusting a/s delay

extern vmCvar_t g_engineerChargeTime;
extern vmCvar_t g_LTChargeTime;
extern vmCvar_t g_soldierChargeTime;
extern vmCvar_t sv_screenshake;
extern vmCvar_t g_screenShake;
// jpw

// NERVE - SMF
extern vmCvar_t g_warmupLatch;
extern vmCvar_t g_nextTimeLimit;
extern vmCvar_t g_showHeadshotRatio;
extern vmCvar_t g_userTimeLimit;
extern vmCvar_t g_userAlliedRespawnTime;
extern vmCvar_t g_userAxisRespawnTime;
extern vmCvar_t g_currentRound;
extern vmCvar_t g_noTeamSwitching;
extern vmCvar_t g_altStopwatchMode;
extern vmCvar_t g_gamestate;
extern vmCvar_t g_swapteams;
// -NERVE - SMF

//Gordon
extern vmCvar_t g_antilag;

extern vmCvar_t g_dbgRevive;

// rtcwpro begin
// Referee/Voting - New cvars
extern vmCvar_t refereePassword;
extern vmCvar_t shoutcastPassword;
extern vmCvar_t team_maxplayers;
extern vmCvar_t team_nocontrols;

extern vmCvar_t match_warmupDamage;
extern vmCvar_t match_mutespecs;
extern vmCvar_t match_latejoin;
extern vmCvar_t match_minplayers;
extern vmCvar_t match_readypercent;
extern vmCvar_t match_timeoutlength;
extern vmCvar_t	g_spectatorAllowDemo;
extern vmCvar_t match_timeoutcount;

// Server stuff
extern vmCvar_t	g_unlockWeapons;
extern vmCvar_t	g_disableSMGPickup;
extern vmCvar_t g_gamelocked;
extern vmCvar_t	sv_hostname;
extern vmCvar_t svx_serverStreaming;
extern vmCvar_t g_bannedMSG;
extern vmCvar_t g_privateServer;
extern vmCvar_t TXThandle;
extern vmCvar_t g_serverMessage;
extern vmCvar_t g_showFlags;
extern vmCvar_t g_allowPMs;
extern vmCvar_t	g_hitsounds;
extern vmCvar_t	g_crouchRate;
extern vmCvar_t g_drawHitboxes;
extern vmCvar_t	g_mapConfigs;
extern vmCvar_t	g_disableInv;
extern vmCvar_t	g_axisSpawnProtectionTime;
extern vmCvar_t	g_alliedSpawnProtectionTime;
extern vmCvar_t g_damageRadiusKnockback;
extern vmCvar_t	g_dropWeapons;

//S4NDM4NN - fix errors when sv_fps is adjusted
extern vmCvar_t sv_fps;

// Weapon/class stuff
extern vmCvar_t	g_ltNades;
extern vmCvar_t	g_medicNades;
extern vmCvar_t	g_soldNades;
extern vmCvar_t	g_engNades;
extern vmCvar_t	g_medicClips;
extern vmCvar_t	g_engineerClips;
extern vmCvar_t	g_soldierClips;
extern vmCvar_t	g_leutClips;
extern vmCvar_t	g_pistolClips;
extern vmCvar_t g_lifeStats;
extern vmCvar_t g_maxTeamPF;
extern vmCvar_t g_maxTeamSniper;
extern vmCvar_t g_maxTeamVenom;
extern vmCvar_t g_maxTeamFlamer;
// Misc (unsorted)
extern vmCvar_t g_pauseLimit;
extern vmCvar_t	g_fastStabSound;
extern vmCvar_t g_duelAutoPause;
extern vmCvar_t g_tournament;

//
// NOTE!!! If any vote flags are added, MAKE SURE to update the voteFlags struct in bg_misc.c w/appropriate info,
//         menudef.h for the mask and g_main.c for vote_allow_* flag updates
//
extern vmCvar_t vote_allow_comp;
extern vmCvar_t vote_allow_gametype;
extern vmCvar_t vote_allow_kick;
extern vmCvar_t vote_allow_map;
extern vmCvar_t vote_allow_matchreset;
extern vmCvar_t vote_allow_mutespecs;
extern vmCvar_t vote_allow_nextmap;
extern vmCvar_t vote_allow_pub;
extern vmCvar_t vote_allow_referee;
extern vmCvar_t vote_allow_shuffleteamsxp;
extern vmCvar_t vote_allow_swapteams;
extern vmCvar_t vote_allow_friendlyfire;
extern vmCvar_t vote_allow_timelimit;
extern vmCvar_t vote_allow_warmupdamage;
extern vmCvar_t vote_allow_antilag;
extern vmCvar_t vote_allow_balancedteams;
extern vmCvar_t vote_allow_muting;
extern vmCvar_t	vote_allow_cointoss;
extern vmCvar_t vote_limit;
extern vmCvar_t vote_percent;

extern vmCvar_t stats_matchid;
// Ref tag..
#define REFEREE	"^3Ref^7"

extern vmCvar_t	g_antiWarp; // antiwarp port
extern vmCvar_t P; // player teams in server info
extern vmCvar_t	g_hsDamage;
extern vmCvar_t g_spawnOffset; // random spawn offset for both teams, between 1 and cvar integer - 1
extern vmCvar_t g_bodiesGrabFlags;
extern vmCvar_t g_mapScriptDirectory;
extern vmCvar_t g_thinkStateLevelTime;
extern vmCvar_t g_endStateLevelTime;
extern vmCvar_t g_thinkSnapOrigin;
extern vmCvar_t g_fixedphysicsfps;
extern vmCvar_t g_alternatePing;

void    trap_Printf( const char *fmt );
void    trap_Error( const char *fmt );
int     trap_Milliseconds( void );
int		trap_RealTime( qtime_t *qtime );  // added from wolfX
int     trap_Argc( void );
void    trap_Argv( int n, char *buffer, int bufferLength );
void    trap_Args( char *buffer, int bufferLength );
int		trap_FS_FileExists(const char* filename);
int     trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void    trap_FS_Read( void *buffer, int len, fileHandle_t f );
int     trap_FS_Write( const void *buffer, int len, fileHandle_t f );
int     trap_FS_Rename( const char *from, const char *to );
void    trap_FS_FCloseFile( fileHandle_t f );
void	trap_FS_Delete( const char *filename );  // nihi - added from wolfX
int     trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
void    trap_SendConsoleCommand( int exec_when, const char *text );
void    trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void    trap_Cvar_Update( vmCvar_t *cvar );
void    trap_Cvar_Set( const char *var_name, const char *value );
void	trap_Cvar_Restrictions_Load(void);
int     trap_Cvar_VariableIntegerValue( const char *var_name );
float   trap_Cvar_VariableValue( const char *var_name );
void    trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void    trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *gameClients, int sizeofGameClient );
void    trap_DropClient( int clientNum, const char *reason );
void    trap_SendServerCommand( int clientNum, const char *text );
void    trap_SetConfigstring( int num, const char *string );
void    trap_GetConfigstring( int num, char *buffer, int bufferSize );
void    trap_GetUserinfo( int num, char *buffer, int bufferSize );
void    trap_SetUserinfo( int num, const char *buffer );
void    trap_GetServerinfo( char *buffer, int bufferSize );
void    trap_SetBrushModel( gentity_t *ent, const char *name );
void    trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void    trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
int     trap_PointContents( const vec3_t point, int passEntityNum );
qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 );
qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
void    trap_AdjustAreaPortalState( gentity_t *ent, qboolean open );
qboolean trap_AreasConnected( int area1, int area2 );
void    trap_LinkEntity( gentity_t *ent );
void    trap_UnlinkEntity( gentity_t *ent );
int     trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
int     trap_BotAllocateClient( void );
void    trap_BotFreeClient( int clientNum );
void    trap_GetUsercmd( int clientNum, usercmd_t *cmd );
qboolean    trap_GetEntityToken( char *buffer, int bufferSize );
qboolean trap_GetTag(gentity_t* ent, clientAnimationInfo_t* animInfo, char* tagName, orientation_t* or );

int     trap_DebugPolygonCreate( int color, int numPoints, vec3_t *points );
void    trap_DebugPolygonDelete( int id );

int     trap_submit_curlPost( char* jsonfile, char* matchid );

int     trap_BotLibSetup( void );
int     trap_BotLibShutdown( void );
int     trap_BotLibVarSet( char *var_name, char *value );
int     trap_BotLibVarGet( char *var_name, char *value, int size );
int     trap_BotLibDefine( char *string );
int     trap_BotLibStartFrame( float time );
int     trap_BotLibLoadMap( const char *mapname );
int     trap_BotLibUpdateEntity( int ent, void /* struct bot_updateentity_s */ *bue );
int     trap_BotLibTest( int parm0, char *parm1, vec3_t parm2, vec3_t parm3 );

int     trap_BotGetSnapshotEntity( int clientNum, int sequence );
int     trap_BotGetServerCommand( int clientNum, char *message, int size );
//int		trap_BotGetConsoleMessage(int clientNum, char *message, int size);
void    trap_BotUserCommand( int client, usercmd_t *ucmd );

void        trap_AAS_EntityInfo( int entnum, void /* struct aas_entityinfo_s */ *info );

int         trap_AAS_Initialized( void );
void        trap_AAS_PresenceTypeBoundingBox( int presencetype, vec3_t mins, vec3_t maxs );
float       trap_AAS_Time( void );

// Ridah
void        trap_AAS_SetCurrentWorld( int index );
// done.

int         trap_AAS_PointAreaNum( vec3_t point );
int         trap_AAS_TraceAreas( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas );

int         trap_AAS_PointContents( vec3_t point );
int         trap_AAS_NextBSPEntity( int ent );
int         trap_AAS_ValueForBSPEpairKey( int ent, char *key, char *value, int size );
int         trap_AAS_VectorForBSPEpairKey( int ent, char *key, vec3_t v );
int         trap_AAS_FloatForBSPEpairKey( int ent, char *key, float *value );
int         trap_AAS_IntForBSPEpairKey( int ent, char *key, int *value );

int         trap_AAS_AreaReachability( int areanum );

int         trap_AAS_AreaTravelTimeToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags );

int         trap_AAS_Swimming( vec3_t origin );
int         trap_AAS_PredictClientMovement( void /* aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize );

// Ridah, route-tables
void        trap_AAS_RT_ShowRoute( vec3_t srcpos, int srcnum, int destnum );
qboolean    trap_AAS_RT_GetHidePos( vec3_t srcpos, int srcnum, int srcarea, vec3_t destpos, int destnum, int destarea, vec3_t returnPos );
int         trap_AAS_FindAttackSpotWithinRange( int srcnum, int rangenum, int enemynum, float rangedist, int travelflags, float *outpos );
void        trap_AAS_SetAASBlockingEntity( vec3_t absmin, vec3_t absmax, qboolean blocking );
// done.

void    trap_EA_Say( int client, char *str );
void    trap_EA_SayTeam( int client, char *str );
void    trap_EA_UseItem( int client, char *it );
void    trap_EA_DropItem( int client, char *it );
void    trap_EA_UseInv( int client, char *inv );
void    trap_EA_DropInv( int client, char *inv );
void    trap_EA_Gesture( int client );
void    trap_EA_Command( int client, char *command );

void    trap_EA_SelectWeapon( int client, int weapon );
void    trap_EA_Talk( int client );
void    trap_EA_Attack( int client );
void    trap_EA_Reload( int client );
void    trap_EA_Use( int client );
void    trap_EA_Respawn( int client );
void    trap_EA_Jump( int client );
void    trap_EA_DelayedJump( int client );
void    trap_EA_Crouch( int client );
void    trap_EA_MoveUp( int client );
void    trap_EA_MoveDown( int client );
void    trap_EA_MoveForward( int client );
void    trap_EA_MoveBack( int client );
void    trap_EA_MoveLeft( int client );
void    trap_EA_MoveRight( int client );
void    trap_EA_Move( int client, vec3_t dir, float speed );
void    trap_EA_View( int client, vec3_t viewangles );

void    trap_EA_EndRegular( int client, float thinktime );
void    trap_EA_GetInput( int client, float thinktime, void /* struct bot_input_s */ *input );
void    trap_EA_ResetInput( int client, void *init );


int     trap_BotLoadCharacter( char *charfile, int skill );
void    trap_BotFreeCharacter( int character );
float   trap_Characteristic_Float( int character, int index );
float   trap_Characteristic_BFloat( int character, int index, float min, float max );
int     trap_Characteristic_Integer( int character, int index );
int     trap_Characteristic_BInteger( int character, int index, int min, int max );
void    trap_Characteristic_String( int character, int index, char *buf, int size );

int     trap_BotAllocChatState( void );
void    trap_BotFreeChatState( int handle );
void    trap_BotQueueConsoleMessage( int chatstate, int type, char *message );
void    trap_BotRemoveConsoleMessage( int chatstate, int handle );
int     trap_BotNextConsoleMessage( int chatstate, void /* struct bot_consolemessage_s */ *cm );
int     trap_BotNumConsoleMessages( int chatstate );
void    trap_BotInitialChat( int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int     trap_BotNumInitialChats( int chatstate, char *type );
int     trap_BotReplyChat( int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
int     trap_BotChatLength( int chatstate );
void    trap_BotEnterChat( int chatstate, int client, int sendto );
void    trap_BotGetChatMessage( int chatstate, char *buf, int size );
int     trap_StringContains( char *str1, char *str2, int casesensitive );
int     trap_BotFindMatch( char *str, void /* struct bot_match_s */ *match, unsigned long int context );
void    trap_BotMatchVariable( void /* struct bot_match_s */ *match, int variable, char *buf, int size );
void    trap_UnifyWhiteSpaces( char *string );
void    trap_BotReplaceSynonyms( char *string, unsigned long int context );
int     trap_BotLoadChatFile( int chatstate, char *chatfile, char *chatname );
void    trap_BotSetChatGender( int chatstate, int gender );
void    trap_BotSetChatName( int chatstate, char *name );
void    trap_BotResetGoalState( int goalstate );
void    trap_BotRemoveFromAvoidGoals( int goalstate, int number );
void    trap_BotResetAvoidGoals( int goalstate );
void    trap_BotPushGoal( int goalstate, void /* struct bot_goal_s */ *goal );
void    trap_BotPopGoal( int goalstate );
void    trap_BotEmptyGoalStack( int goalstate );
void    trap_BotDumpAvoidGoals( int goalstate );
void    trap_BotDumpGoalStack( int goalstate );
void    trap_BotGoalName( int number, char *name, int size );
int     trap_BotGetTopGoal( int goalstate, void /* struct bot_goal_s */ *goal );
int     trap_BotGetSecondGoal( int goalstate, void /* struct bot_goal_s */ *goal );
int     trap_BotChooseLTGItem( int goalstate, vec3_t origin, int *inventory, int travelflags );
int     trap_BotChooseNBGItem( int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime );
int     trap_BotTouchingGoal( vec3_t origin, void /* struct bot_goal_s */ *goal );
int     trap_BotItemGoalInVisButNotVisible( int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal );
int     trap_BotGetNextCampSpotGoal( int num, void /* struct bot_goal_s */ *goal );
int     trap_BotGetMapLocationGoal( char *name, void /* struct bot_goal_s */ *goal );
int     trap_BotGetLevelItemGoal( int index, char *classname, void /* struct bot_goal_s */ *goal );
float   trap_BotAvoidGoalTime( int goalstate, int number );
void    trap_BotInitLevelItems( void );
void    trap_BotUpdateEntityItems( void );
int     trap_BotLoadItemWeights( int goalstate, char *filename );
void    trap_BotFreeItemWeights( int goalstate );
void    trap_BotInterbreedGoalFuzzyLogic( int parent1, int parent2, int child );
void    trap_BotSaveGoalFuzzyLogic( int goalstate, char *filename );
void    trap_BotMutateGoalFuzzyLogic( int goalstate, float range );
int     trap_BotAllocGoalState( int state );
void    trap_BotFreeGoalState( int handle );

void    trap_BotResetMoveState( int movestate );
void    trap_BotMoveToGoal( void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags );
int     trap_BotMoveInDirection( int movestate, vec3_t dir, float speed, int type );
void    trap_BotResetAvoidReach( int movestate );
void    trap_BotResetLastAvoidReach( int movestate );
int     trap_BotReachabilityArea( vec3_t origin, int testground );
int     trap_BotMovementViewTarget( int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target );
int     trap_BotPredictVisiblePosition( vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target );
int     trap_BotAllocMoveState( void );
void    trap_BotFreeMoveState( int handle );
void    trap_BotInitMoveState( int handle, void /* struct bot_initmove_s */ *initmove );
// Ridah
void    trap_BotInitAvoidReach( int handle );
// done.

int     trap_BotChooseBestFightWeapon( int weaponstate, int *inventory );
void    trap_BotGetWeaponInfo( int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo );
int     trap_BotLoadWeaponWeights( int weaponstate, char *filename );
int     trap_BotAllocWeaponState( void );
void    trap_BotFreeWeaponState( int weaponstate );
void    trap_BotResetWeaponState( int weaponstate );

int     trap_GeneticParentsAndChildSelection( int numranks, float *ranks, int *parent1, int *parent2, int *child );

void    trap_SnapVector( float *v );

typedef enum
{
	shard_glass = 0,
	shard_wood,
	shard_metal,
	shard_ceramic,
	shard_rubble
} shards_t;

// Pause
#define PAUSE_NONE		0x00	// Match is not paused..
#define PAUSE_UNPAUSING 0x02    // Pause is about to expire
// Ready
#define READY_NONE		0x00	// Countdown, playing..
#define READY_AWAITING	0x01	// Awaiting all to ready up..
#define READY_PENDING	0x02	// Awaiting but can start once treshold (minclients) is reached..

// Stats
#define EOM_WEAPONSTATS 0x01    // Dump of player weapon stats at end of match.
#define EOM_MATCHINFO   0x02    // Dump of match stats at end of match.

#define AA_STATSALL     0x01    // Client AutoAction: Dump ALL player stats
#define AA_STATSTEAM    0x02    // Client AutoAction: Dump TEAM player stats



// RTCWPro - removed unused declarations

// g_antilag.c
//
void G_ResetTrail(gentity_t* ent);
void G_StoreTrail(gentity_t* ent);
void G_TimeShiftAllClients(int time, gentity_t* skip);
void G_UnTimeShiftAllClients(gentity_t* skip);
//void G_HistoricalTrace( gentity_t* ent, trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );

// End

void G_ResetMarkers( gentity_t* ent );

///////////////////////
// g_main.c
//

void G_UpdateCvars(void);
void G_teamReset(int, qboolean);
void ServerPlayerInfo(void);
void LoadMapList( void );
//
// g_match.c
//
void G_loadMatchGame(void);
void CountDown(void);
void G_spawnPrintf(int print_type, int print_time, gentity_t *owner);
void G_handlePause(qboolean dPause, int time);
//void G_verifyMatchState(int nTeam);
void G_matchPrintInfo(char *msg, qboolean printTime);
void G_printFull(char *str, gentity_t *ent); // from ET
qboolean G_cmdDebounce(gentity_t *ent, const char *pszCommand);
void G_globalSound(char *sound);
void G_resetRoundState(void);
void G_resetModeState(void);
int G_checkServerToggle(vmCvar_t *cv);
char* GetLevelTime(void);
///////////////////////
// g_referee.c
//
void Cmd_AuthRcon_f( gentity_t *ent );
void G_refAllReady_cmd( gentity_t *ent );
void G_ref_cmd( gentity_t *ent, /*unsigned int dwCommand,*/ qboolean fValue );
void G_scs_cmd(gentity_t* ent, qboolean fValue);
void G_scsSpectatorSpeed(gentity_t* ent);
void G_refLogout(gentity_t* ent);
void G_scsLogout(gentity_t* ent);
void G_scsFollowOBJ(gentity_t* ent);
qboolean G_refCommandCheck( gentity_t *ent, char *cmd );
qboolean G_scsCommandCheck(gentity_t* ent, char* cmd);
void G_refHelp_cmd( gentity_t *ent );
void G_refLockTeams_cmd( gentity_t *ent, qboolean fLock );
void G_refPause_cmd( gentity_t *ent, qboolean fPause );
void G_refPlayerPut_cmd( gentity_t *ent, int team_id );
void G_refRemove_cmd( gentity_t *ent );
void G_refSpeclockTeams_cmd( gentity_t *ent, qboolean fLock );
void G_refWarmup_cmd( gentity_t* ent );
void G_refWarning_cmd( gentity_t* ent );
void G_refMute_cmd( gentity_t *ent, qboolean mute );
void G_refRenameClient(gentity_t* ent);
void G_refRequestSS(gentity_t* ent);
void G_refMakeShoutcaster_cmd(gentity_t* ent);
void G_refRemoveShoutcaster_cmd(gentity_t* ent);
void G_refGetStatus(gentity_t* ent);
int  G_refClientnumForName( gentity_t *ent, const char *name );
void G_refPrintf(gentity_t* ent, const char *fmt, ...);// _attribute((format(printf, 2, 3)));
void G_PlayerBan(void);
void G_MakeReferee(void);
void G_RemoveReferee(void);
void G_MuteClient(void);
void G_UnMuteClient(void);
void AddIPBan(const char *str);
void DecolorString( char *in, char *out);

// g_shared.c
char *Q_StrReplace(char *haystack, char *needle, char *newp);
void setGuid( char *in, char *out );
//void Q_decolorString(char *in, char *out);
void AAPSound(char *sound);

///////////////////////
// g_vote.c
//
#define G_OK            0 // voting
#define G_INVALID       -1 // voting
#define G_NOTFOUND  -2 // voting
int  G_voteCmdCheck( gentity_t *ent, char *arg, char *arg2, qboolean fRefereeCmd );
void G_voteFlags(void);
void G_voteHelp( gentity_t *ent, qboolean fShowVote );
void G_playersMessage( gentity_t *ent );
void G_PrintConfigs(gentity_t* ent);
qboolean G_isValidConfig(gentity_t* ent, const char* configname);
qboolean G_ConfigSet(const char* configName);
// Actual voting commands
int G_Config_v(gentity_t* ent, unsigned int dwVoteIndex, char* arg, char* arg2, qboolean fRefereeCmd);
int G_Gametype_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_Kick_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_Mute_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_UnMute_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_Map_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_MapRestart_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_MatchReset_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_Mutespecs_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_Nextmap_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_Referee_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_ShuffleTeams_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_StartMatch_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_SwapTeams_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_FriendlyFire_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_Timelimit_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_Warmupfire_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_Unreferee_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_AntiLag_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_BalancedTeams_v( gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2, qboolean fRefereeCmd );
int G_CoinToss_v(gentity_t* ent, unsigned int dwVoteIndex, char* arg, char* arg2, qboolean fRefereeCmd);

//
// g_geoip.c
//
typedef struct GeoIPTag {
	fileHandle_t GeoIPDatabase;
	unsigned char * cache;
	unsigned int memsize;
} GeoIP;

unsigned long GeoIP_addr_to_num(const char *addr);
unsigned int GeoIP_seek_record(GeoIP *gi, unsigned long ipnum);
void GeoIP_open(void);
void GeoIP_close(void);
extern GeoIP* gidb;


//
// g_stats.c
//
void doSound(gentity_t *ent, int type, char *path, char *sound);
void doubleKill (gentity_t *ent, int meansOfDeath );
void KillingSprees ( gentity_t *ent, int score );
void deathSpree ( gentity_t *ent );
void killerSpree(gentity_t *ent, int score);
void FirstHeadshot (gentity_t *attacker, gentity_t *targ);
void FirstBlood (gentity_t *self, gentity_t *attacker);
unsigned int G_weapStatIndex_MOD( int iWeaponMOD );
void G_statsPrint( gentity_t *ent, int nType );
void G_addStats( gentity_t *targ, gentity_t *attacker, int dmg_ref, int mod );
void G_addStatsHeadShot( gentity_t *attacker, int mod );
void G_deleteStats( int nClient );
void G_parseStats( char *pszStatsInfo );
char *G_writeStats( gclient_t* client );
char *G_createClientStats( gentity_t *refEnt );
void G_clientStatsPrint( gentity_t *ent, int nType, qboolean toWindow );
void G_weaponStatsLeaders_cmd( gentity_t* ent, qboolean doTop, qboolean doWindow );
void G_weaponRankings_cmd( gentity_t *ent, unsigned int dwCommand, qboolean state );
void G_printMatchInfo( gentity_t *ent, qboolean fDump );
void G_matchInfoDump( unsigned int dwDumpType );
void G_statsall_cmd( gentity_t *ent, unsigned int dwCommand, qboolean fDump );
// json stat stuff
enum eventList {
    eventSuicide=0,
    eventKill,
    eventTeamkill,
    eventRevive,
    eventPause,
    eventUnpause,
    eventClassChange,
    eventNameChange,
    objTaken,
    objDropped,
    objReturned,
    objCapture,
    objDynPlant,
    objDynDefuse,
    objSpawnFlag,
    objDestroyed,
    redRespawn,
    blueRespawn,
    teamFirstSpawn,
};
// for different json output
#define JSON_STAT 1   // output stats
#define JSON_WSTAT 2  // output wstats in player stats
#define JSON_CATEGORIES 4  // output player stats in categories
#define JSON_TEAM 8  // output player stats by team
#define JSON_KILLDATA 16  // include additional data on "kill event"

// g_json.c
int getPstats(json_t *jsonData, char *id, gclient_t *client);
int G_write_match_info( void );
int G_read_match_info( void );
int G_read_round_jstats( void );
void G_jstatsByTeam(qboolean wstats);
void G_jstatsByPlayers(qboolean wstats);
void G_jWeaponStats(void);
int G_check_before_submit( char* jsonfile);
void G_writeGameInfo (int winner);
void G_writeServerInfo (void);
void G_writeDisconnectEvent (gentity_t* agent);
//void G_writeDisconnectEvent (char* player);
void G_writeObjectiveEvent (gentity_t* agent,int objType);
void G_writeGameLogEnd(void);
void G_writeGameEarlyExit(void);
void G_writeGameLogStart(void);
void G_writeClosingJson(void);
void G_writeGeneralEvent (gentity_t* agent,gentity_t* other, char* weapon, int eventType);
void G_writeCombatEvent (gentity_t* agent,gentity_t* other, vec3_t dir);
int G_teamAlive(int team ) ;  // temp addition for calculating number of alive...will improve later if we want to keep


void G_matchClockDump( gentity_t *ent );  // temp addition for cg_autoaction issue

// OSPx - New stuff below
//
// g_cmds.c

qboolean playerCmds (gentity_t *ent, char *cmd );
int ClientNumberFromString( gentity_t *to, char *s );
char *ConcatArgs( int start );
qboolean G_commandHelp(gentity_t *ent, const char *pszCommand, unsigned int dwCommand);
qboolean G_cmdDebounce(gentity_t *ent, const char *pszCommand);
void G_commands_cmd(gentity_t *ent);
void G_commandsHelp_cmd(gentity_t *ent);
qboolean G_commandCheck(gentity_t *ent, const char *cmd, qboolean fDoAnytime);

// now residing in g_utils.c  (previous declaration in g_admin.h)
//
void CPSound(gentity_t *ent, char *sound);
void APSound(char *sound);
void APRSound(gentity_t *ent, char *sound);


// Macros
//
#define AP( x ) trap_SendServerCommand( -1, x )                 // Print to all
#define CP( x ) trap_SendServerCommand( ent - g_entities, x )	// Print to an ent
#define CPx( x, y ) trap_SendServerCommand( x, y )				// Print to id = x
#define TP( x, y ) G_TeamCommand( x, y)							// Sends team command
#define APS(x)		APSound(x)									// Global sound
#define AAPS(x)		AAPSound(x)									// Global sound but hooked under cg_announcer..
#define APRS(x, y)	APRSound(x, y)								// Global sound with limited (radius) range
#define CPS(x, y)	CPSound(x, y)								// Client sound only

extern char *aTeams[TEAM_NUM_TEAMS];
extern team_info teamInfo[TEAM_NUM_TEAMS];
qboolean IsWeaponDisabled(gentity_t* ent, int sessionWeapon, weapon_t weapon, team_t team, qboolean quiet);
int TeamWeaponCount(gentity_t* ent, team_t team, int weap);
void SetDefaultWeapon(gclient_t *client, qboolean isSold);

//
// g_weapon.c
//
extern extWeaponStats_t BG_WeapStatForWeapon(weapon_t iWeaponID);

#define HELP_COLUMNS    4

//
// - Config
#define ZSF_COMP        0x01    // Have comp settings loaded for current gametype?

//
// g_antiwarp.c
//
qboolean G_DoAntiwarp(gentity_t* ent);
void AW_AddUserCmd(int clientNum, usercmd_t* cmd);
static float AW_CmdScale(gentity_t* ent, usercmd_t* cmd);
void DoClientThinks(gentity_t* ent);

/**
 * @enum enum_t_dp
 * @brief "Delayed Print" ent enumerations
 */
typedef enum
{
	DP_PAUSEINFO = 0,   ///< Print current pause info
	DP_UNPAUSING,       ///< Print unpause countdown + unpause
	DP_CONNECTINFO,     ///< Display info on connect
	DP_MVSPAWN          ///< Set up MV views for clients who need them
} enum_t_dp;
#ifdef MYSQLDEP
// SQL
int				trap_SQL_RunQuery(const char* query);
void			trap_SQL_FinishQuery(int queryid);
qboolean		trap_SQL_NextRow(int queryid);
int				trap_SQL_RowCount(int queryid);
void			trap_SQL_GetFieldbyID(int queryid, int fieldid, char* buffer, int len);
void			trap_SQL_GetFieldbyName(int queryid, const char* name, char* buffer, int len);
int				trap_SQL_GetFieldbyID_int(int queryid, int fieldid);
int				trap_SQL_GetFieldbyName_int(int queryid, const char* name);
int				trap_SQL_FieldCount(int queryid);
void			trap_SQL_CleanString(const char* in, char* out, int len);
#endif
