#include "g_local.h"
#include "g_stats.h"
/*#include <jansson.h>
#ifdef _WIN32
#include "../qcommon/jansson_win/jansson.h"
#else
#include "../qcommon/jansson/jansson.h"
#endif // _WIN32
*/
#include <time.h>

#ifdef _WIN32 
inline int access(const char* pathname, int mode) {
    return _access(pathname, mode);
}
#else
#include <unistd.h>
#endif

#define MATCHID level.jsonStatInfo.match_id
#define ROUNDID level.jsonStatInfo.round_id

typedef enum {
    ModeExists = 0,
    ModeExecute = 1,
    ModeWrite = 2,
    ModeRead = 4
} FileAccessMode;

static char* GetHomePath()
{
    char hpath[256];
    char game[60];
    trap_Cvar_VariableStringBuffer("fs_homepath", hpath, sizeof(hpath));
    trap_Cvar_VariableStringBuffer("fs_game", game, sizeof(game));
    return va("%s/%s", hpath, game);
    
}
// possible values for filename
// stats/matchinfo.json OR level.jsonStatInfo.gameStatslogFileName
qboolean CanAccessFile(char* str, char* filename)
{
    if (g_gameStatslog.integer)
    {
        if (!strcmp(filename, "")) // if filename hasn't been initialized just return
            return qfalse;

        if (g_statsDebug.integer)
            G_LogPrintf("%s\n", str);

        if (!strcmp(filename, "stats/matchinfo.json"))
        {
            char* homePath = GetHomePath();
            filename = va("%s/stats/matchinfo.json", homePath);
        }

        int result = access(filename, ModeWrite);

        if (result == 0)
            return qtrue;
        else
        {
            G_LogPrintf(va("Stats: COULD NOT ACCESS FILE %s\n", filename));
            return qfalse;
        }

        return qfalse;
    }

    return qfalse;
}

/*
  Build player stats objects
  This will allow us to access stats for players who quit before the round is finished
*/
void BuildPlayerStats(gclient_t *client, qboolean clientDisconnected)
{
    gclient_t* cl;
    int eff;
    char n1[MAX_NETNAME];
    char n2[MAX_NETNAME];
    unsigned int m;

    if (clientDisconnected && client != NULL)
    {
        int dc = level.disconnectCount;

        if (level.disconnectStats[dc].guid == NULL)
        {
            cl = client; // used passed in client

            DecolorString(cl->pers.netname, n1);
            SanitizeString(n1, n2);
            Q_CleanStr(n2);
            n2[15] = 0;

            Q_strncpyz(level.disconnectStats[dc].aliasColored, cl->pers.netname, sizeof(level.disconnectStats[dc].aliasColored));
            Q_strncpyz(level.disconnectStats[dc].alias, n2, sizeof(level.disconnectStats[dc].alias));

            eff = (cl->sess.deaths + cl->sess.kills == 0) ? 0 : 100 * cl->sess.kills / (cl->sess.deaths + cl->sess.kills);
            if (eff < 0)
            {
                eff = 0;
            }

            level.disconnectStats[dc].guid = cl->sess.guid;
            level.disconnectStats[dc].sessionTeam = cl->sess.sessionTeam;
            level.disconnectStats[dc].start_time = cl->sess.start_time;
            level.disconnectStats[dc].rounds = cl->sess.rounds;
            level.disconnectStats[dc].kills = cl->sess.kills;
            level.disconnectStats[dc].deaths = cl->sess.deaths;
            level.disconnectStats[dc].gibs = cl->sess.gibs;
            level.disconnectStats[dc].suicides = cl->sess.suicides;
            level.disconnectStats[dc].team_kills = cl->sess.team_kills;
            level.disconnectStats[dc].headshots = cl->sess.headshots;
            level.disconnectStats[dc].damage_given = cl->sess.damage_given;
            level.disconnectStats[dc].damage_received = cl->sess.damage_received;
            level.disconnectStats[dc].team_damage = cl->sess.team_damage;
            level.disconnectStats[dc].acc_hits = cl->sess.acc_hits;
            level.disconnectStats[dc].acc_shots = cl->sess.acc_shots;
            level.disconnectStats[dc].efficiency = eff;
            level.disconnectStats[dc].revives = cl->sess.revives;
            level.disconnectStats[dc].ammo_given = cl->sess.ammo_given;
            level.disconnectStats[dc].med_given = cl->sess.med_given;
            level.disconnectStats[dc].knifeKills = cl->sess.knifeKills;
            level.disconnectStats[dc].killPeak = cl->sess.killPeak;
            level.disconnectStats[dc].score = cl->ps.persistant[PERS_SCORE];
            level.disconnectStats[dc].dyn_planted = cl->sess.dyn_planted;
            level.disconnectStats[dc].dyn_defused = cl->sess.dyn_defused;
            level.disconnectStats[dc].obj_captured = cl->sess.obj_captured;
            level.disconnectStats[dc].obj_destroyed = cl->sess.obj_destroyed;
            level.disconnectStats[dc].obj_returned = cl->sess.obj_returned;
            level.disconnectStats[dc].obj_taken = cl->sess.obj_taken;
            level.disconnectStats[dc].obj_checkpoint = cl->sess.obj_checkpoint;
            level.disconnectStats[dc].obj_killcarrier = cl->sess.obj_killcarrier;
            level.disconnectStats[dc].obj_protectflag = cl->sess.obj_protectflag;

            int j = cl->sess.sessionTeam;
            for (m = WS_KNIFE; m < WS_MAX; m++)
            {
                if (cl->sess.aWeaponStats[m].atts || cl->sess.aWeaponStats[m].hits || cl->sess.aWeaponStats[m].deaths)
                {
                    level.disconnectStats[dc].aWeaponStats[m].kills = cl->sess.aWeaponStats[m].kills;
                    level.disconnectStats[dc].aWeaponStats[m].deaths = cl->sess.aWeaponStats[m].deaths;
                    level.disconnectStats[dc].aWeaponStats[m].headshots = cl->sess.aWeaponStats[m].headshots;
                    level.disconnectStats[dc].aWeaponStats[m].hits = cl->sess.aWeaponStats[m].hits;
                    level.disconnectStats[dc].aWeaponStats[m].atts = cl->sess.aWeaponStats[m].atts;
                }
            }

            level.disconnectCount++;
        }
    }
    else
    {
        // loop for amount of players playing and how many have already quit
        for (int i = 0; i < level.numPlayingClients; i++)
        {
            cl = level.clients + level.sortedClients[i];

            if (cl->pers.connected != CON_CONNECTED)
                continue;

            DecolorString(cl->pers.netname, n1);
            SanitizeString(n1, n2);
            Q_CleanStr(n2);
            n2[15] = 0;

            Q_strncpyz(level.playerStats[i].aliasColored, cl->pers.netname, sizeof(level.playerStats[i].aliasColored));
            Q_strncpyz(level.playerStats[i].alias, n2, sizeof(level.playerStats[i].alias));

            eff = (cl->sess.deaths + cl->sess.kills == 0) ? 0 : 100 * cl->sess.kills / (cl->sess.deaths + cl->sess.kills);
            if (eff < 0)
            {
                eff = 0;
            }

            level.playerStats[i].guid = cl->sess.guid;
            level.playerStats[i].sessionTeam = cl->sess.sessionTeam;
            level.playerStats[i].start_time = cl->sess.start_time;
            level.playerStats[i].rounds = cl->sess.rounds;
            level.playerStats[i].kills = cl->sess.kills;
            level.playerStats[i].deaths = cl->sess.deaths;
            level.playerStats[i].gibs = cl->sess.gibs;
            level.playerStats[i].suicides = cl->sess.suicides;
            level.playerStats[i].team_kills = cl->sess.team_kills;
            level.playerStats[i].headshots = cl->sess.headshots;
            level.playerStats[i].damage_given = cl->sess.damage_given;
            level.playerStats[i].damage_received = cl->sess.damage_received;
            level.playerStats[i].team_damage = cl->sess.team_damage;
            level.playerStats[i].acc_hits = cl->sess.acc_hits;
            level.playerStats[i].acc_shots = cl->sess.acc_shots;
            level.playerStats[i].efficiency = eff;
            level.playerStats[i].revives = cl->sess.revives;
            level.playerStats[i].ammo_given = cl->sess.ammo_given;
            level.playerStats[i].med_given = cl->sess.med_given;
            level.playerStats[i].knifeKills = cl->sess.knifeKills;
            level.playerStats[i].killPeak = cl->sess.killPeak;
            level.playerStats[i].score = cl->ps.persistant[PERS_SCORE];
            level.playerStats[i].dyn_planted = cl->sess.dyn_planted;
            level.playerStats[i].dyn_defused = cl->sess.dyn_defused;
            level.playerStats[i].obj_captured = cl->sess.obj_captured;
            level.playerStats[i].obj_destroyed = cl->sess.obj_destroyed;
            level.playerStats[i].obj_returned = cl->sess.obj_returned;
            level.playerStats[i].obj_taken = cl->sess.obj_taken;
            level.playerStats[i].obj_checkpoint = cl->sess.obj_checkpoint;
            level.playerStats[i].obj_killcarrier = cl->sess.obj_killcarrier;
            level.playerStats[i].obj_protectflag = cl->sess.obj_protectflag;

            int j = cl->sess.sessionTeam;
            for (m = WS_KNIFE; m < WS_MAX; m++)
            {
                if (cl->sess.aWeaponStats[m].atts || cl->sess.aWeaponStats[m].hits || cl->sess.aWeaponStats[m].deaths)
                {
                    level.playerStats[i].aWeaponStats[m].kills = cl->sess.aWeaponStats[m].kills;
                    level.playerStats[i].aWeaponStats[m].deaths = cl->sess.aWeaponStats[m].deaths;
                    level.playerStats[i].aWeaponStats[m].headshots = cl->sess.aWeaponStats[m].headshots;
                    level.playerStats[i].aWeaponStats[m].hits = cl->sess.aWeaponStats[m].hits;
                    level.playerStats[i].aWeaponStats[m].atts = cl->sess.aWeaponStats[m].atts;
                }
            }
        }
    }
}

/*
  Retrieve player stats from json data and use it to set player session data
*/

int getPstats(json_t *jsonData, char *id, gclient_t *client) {

    //if (!CanAccessFile("Stats: get pstats", level.jsonStatInfo.gameStatslogFileName))
    //    return 0;

    json_t *pcat, *pitem, *pstats;
    int i = 0;
    pstats = json_object();
    pcat = json_object();
    pstats = json_object_get(jsonData, id);
    pcat = json_object_get(pstats, "categories");

    pitem = json_object_get(pcat, "kills");
    if(!(json_is_object(pstats)))
    {
        G_Printf("error reading player data\n");
        json_decref(jsonData);
        return 0;
    }
    /*
        - Use switch or iterate over categories...new struct with data type need tho
        - Put an else clause that resets stats to zero if error
    */

    if(json_is_integer(pitem)) {
        client->sess.kills=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "deaths");
    if(json_is_integer(pitem)) {
        client->sess.deaths=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "gibs");
    if(json_is_integer(pitem)) {
        client->sess.gibs=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "suicides");
    if(json_is_integer(pitem)) {
        client->sess.suicides=json_integer_value(pitem);

    }

    pitem = json_object_get(pcat, "teamkills");
    if(json_is_integer(pitem)) {
        client->sess.team_kills=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "damagegiven");
    if(json_is_integer(pitem)) {
        client->sess.damage_given=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "damagereceived");
    if(json_is_integer(pitem)) {
        client->sess.damage_received=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "damageteam");
    if(json_is_integer(pitem)) {
        client->sess.team_damage=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "hits");
    if(json_is_integer(pitem)) {
        client->sess.acc_hits=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "shots");
    if(json_is_integer(pitem)) {
        client->sess.acc_shots=json_integer_value(pitem);


    }
    /*
    pitem = json_object_get(pcat, "accuracy");
    if(json_is_number(pitem)) {
        // calculate this elsewhere...


    }
    */
    pitem = json_object_get(pcat, "revives");
    if(json_is_integer(pitem)) {
            client->sess.revives=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "ammogiven");
    if(json_is_integer(pitem)) {
        client->sess.ammo_given=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "healthgiven");
    if(json_is_integer(pitem)) {
        client->sess.med_given=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "knifekills");
    if(json_is_integer(pitem)) {
        client->sess.knifeKills=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "killpeak");
    if(json_is_integer(pitem)) {
        client->sess.killPeak=json_integer_value(pitem);


    }
    /*
    pitem = json_object_get(pcat, "efficiency");
    if(json_is_number(pitem)) {  // might want to simply calculate this outside of here


    }
    */
    /*
    pitem = json_object_get(pcat, "score");
    if(json_is_integer(pitem)) {
        client->sess.score=json_integer_value(pitem);
        G_Printf("score: %llu\n",json_integer_value(pitem));

    }
    */
    pitem = json_object_get(pcat, "dyn_planted");
    if(json_is_integer(pitem)) {
        client->sess.dyn_planted=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "dyn_defused");
    if(json_is_integer(pitem)) {
        client->sess.dyn_defused=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "obj_captured");
    if(json_is_integer(pitem)) {
        client->sess.obj_captured=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "obj_destroyed");
    if(json_is_integer(pitem)) {
        client->sess.obj_destroyed=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "obj_returned");
    if(json_is_integer(pitem)) {
        client->sess.obj_returned=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "obj_taken");
    if(json_is_integer(pitem)) {
        client->sess.obj_taken=json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "obj_checkpoint");
    if (json_is_integer(pitem)) {
        client->sess.obj_checkpoint = json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "obj_killcarrier");
    if (json_is_integer(pitem)) {
        client->sess.obj_killcarrier = json_integer_value(pitem);


    }
    pitem = json_object_get(pcat, "obj_protectflag");
    if (json_is_integer(pitem)) {
        client->sess.obj_protectflag = json_integer_value(pitem);


    }
    return 1;
}

int G_write_match_info( void )
{
    json_t *data = NULL;
    //json_t* json, *object, *jstattype, *jstats;
    //json_error_t error;
    //char* buf;
    char* s;
    //char mapName[MAX_QPATH];
    //char gameConfig[MAX_QPATH];
    time_t unixTime = time(NULL);
    fileHandle_t matchfileinfo;
	qtime_t ct;
	trap_RealTime(&ct);

    Q_strncpyz(level.jsonStatInfo.round_start,va("%ld", unixTime),sizeof(level.jsonStatInfo.round_start));
    Q_strncpyz(level.jsonStatInfo.round_timelimit,va("%s",GetLevelTime()),sizeof(level.jsonStatInfo.round_timelimit));

    json_t *jdata = json_object();
    json_object_set_new(jdata, "matchID",    json_string(MATCHID));
    json_object_set_new(jdata, "roundID",    json_string(level.jsonStatInfo.round_id));
    json_object_set_new(jdata, "timelimit",    json_string(level.jsonStatInfo.round_timelimit));
    json_object_set_new(jdata, "roundstart",    json_string(level.jsonStatInfo.round_start));

    trap_FS_FOpenFile("stats/matchinfo.json", &matchfileinfo, FS_WRITE );
    if (matchfileinfo)
    {
        if (!CanAccessFile("Stats: writing match info", "stats/matchinfo.json"))
            return 0;

        s = json_dumps( jdata, 1 );
        trap_FS_Write( "{\n \"matchinfo\": \n", strlen( "{\n \"matchinfo\": \n" ), matchfileinfo);
        trap_FS_Write( s, strlen( s ), matchfileinfo );
        trap_FS_Write( "}\n", strlen( "}\n" ), matchfileinfo);
        json_decref(jdata);
        free(s);

        trap_FS_FCloseFile( matchfileinfo);
    }

    return 0;
}

int G_read_match_info( void )
{
    if (!CanAccessFile("Stats: reading match info", "stats/matchinfo.json"))
        return 0;

    json_t *data = NULL;
    json_t* json, *object, *jstattype; // , * jstats;
    json_error_t error;

    json = json_load_file("stats/matchinfo.json", 0, &error);
    if (error.line != -1) {
        G_Printf("error: unable to read json round stat file\n");
        return 0;
    }

    object = json_object();
    if (json)
    {
        object = json_object_get(json, "matchinfo");
        jstattype = json_object_get(object, "matchID");


        if (json_string_value(jstattype)) {
            Q_strncpyz(MATCHID,json_string_value(jstattype),sizeof(MATCHID));
        }
        else {
            json_decref(object);
            return 0;
        }

        jstattype = json_object_get(object, "roundID");
        if (json_string_value(jstattype)) {
            Q_strncpyz(ROUNDID,json_string_value(jstattype),sizeof(ROUNDID));
        }
        else{
            json_decref(object);
            return 0;
        }

        jstattype = json_object_get(object, "timelimit");
        if (json_string_value(jstattype)) {
            Q_strncpyz(level.jsonStatInfo.round_timelimit,json_string_value(jstattype),sizeof(level.jsonStatInfo.round_timelimit));
        }
        else{
            json_decref(object);
            return 0;
        }
        json_decref(object);
        return 1;

    }
    json_decref(object);
    return 0;
}
/*
 Check if stats meet requirement for submission

*/
int G_check_before_submit( char* jsonfile)
{
    if (!CanAccessFile("Stats: check before submit", jsonfile))
        return 0;

    json_t *data = NULL;
    json_t *json,*jstats;
    json_error_t error;

    int minPlayers = 3; // require 3 players for stats submission

    json = json_load_file(jsonfile, 0, &error);
    if (error.line != -1) {
        G_Printf("error: unable to read json round stat file\n");
        return 0;
    }

    if (json)
    {
      //  Can add more conditions but for now based only on number of players
        jstats = json_object_get(json, "stats");
        if (json_array_size(jstats) >= minPlayers) {
            return 1;
        }
        else {
            G_Printf("Stats API: Skipping file submission. Number of players under minPlayers.\n");
        }
        json_decref(json);

    }
    return 0;
}


/*
Read stats back into client session if player disconnected and came back
*/
int G_read_round_jstats_reconnect(gclient_t* client)
{
    char pGUID[64];

    for (int i = 0; i < ArrayLength(level.disconnectStats); i++)
    {
        sprintf(pGUID, "%s", client->sess.guid);

        if (!Q_stricmp(level.disconnectStats[i].guid, pGUID))
        {
            client->sess.deaths = level.disconnectStats[i].deaths;
            client->sess.kills = level.disconnectStats[i].kills;
            client->sess.damage_given = level.disconnectStats[i].damage_given;
            client->sess.damage_received = level.disconnectStats[i].damage_received;
            client->sess.team_damage = level.disconnectStats[i].team_damage;
            client->sess.team_kills = level.disconnectStats[i].team_kills;
            client->sess.gibs = level.disconnectStats[i].gibs;
            client->sess.acc_shots = level.disconnectStats[i].acc_shots;
            client->sess.acc_hits = level.disconnectStats[i].acc_hits;
            client->sess.headshots = level.disconnectStats[i].headshots;
            client->sess.suicides = level.disconnectStats[i].suicides;
            client->sess.med_given = level.disconnectStats[i].med_given;
            client->sess.ammo_given = level.disconnectStats[i].ammo_given;
            client->sess.revives = level.disconnectStats[i].revives;
            client->sess.knifeKills = level.disconnectStats[i].knifeKills;
            client->sess.dyn_planted = level.disconnectStats[i].dyn_planted;
            client->sess.dyn_defused = level.disconnectStats[i].dyn_defused;
            client->sess.obj_captured = level.disconnectStats[i].obj_captured;
            client->sess.obj_destroyed = level.disconnectStats[i].obj_destroyed;
            client->sess.obj_returned = level.disconnectStats[i].obj_returned;
            client->sess.obj_taken = level.disconnectStats[i].obj_taken;
            client->sess.obj_checkpoint = level.disconnectStats[i].obj_checkpoint;
            client->sess.obj_killcarrier = level.disconnectStats[i].obj_killcarrier;
            client->sess.obj_protectflag = level.disconnectStats[i].obj_protectflag;

            for (int m = WS_KNIFE; m < WS_MAX; m++)
            {
                if (level.disconnectStats[i].aWeaponStats[m].atts || level.disconnectStats[i].aWeaponStats[m].hits || level.disconnectStats[i].aWeaponStats[m].deaths)
                {
                    client->sess.aWeaponStats[m].kills = level.disconnectStats[i].aWeaponStats[m].kills;
                    client->sess.aWeaponStats[m].deaths = level.disconnectStats[i].aWeaponStats[m].deaths;
                    client->sess.aWeaponStats[m].headshots = level.disconnectStats[i].aWeaponStats[m].headshots;
                    client->sess.aWeaponStats[m].hits = level.disconnectStats[i].aWeaponStats[m].hits;
                    client->sess.aWeaponStats[m].atts = level.disconnectStats[i].aWeaponStats[m].atts;
                }
            }

            memset(&level.disconnectStats[i], 0, sizeof(level.disconnectStats[i]));
            level.disconnectCount--;

            return 1;
        }
    }

    return 0;
}

/*
 Read in stats from json to client session

 Currently reads in stats from round 1 but the idea is to have it reload
 stats from previous round upon a map_restart and g_currentround > 1...

*/
int G_read_round_jstats( void )
{
    json_t *data = NULL;
    json_t *json,*object,*jstattype, *jstats;
    json_error_t error;
    gclient_t *cl;
    int j, i;
    char pGUID[64];
    char hpath[256];
    char game[60];
    char mapName[64];
   // char *matchid;
    qtime_t ct;
    trap_RealTime(&ct);
    trap_Cvar_VariableStringBuffer( "mapname", mapName, sizeof(mapName) );
    char buf[64];
 //   char cs[MAX_STRING_CHARS];

    // we want to save some information for the match and round
    // TODO: Change to currentRound >= 1 and set g_currentRound -= 1
    if (g_currentRound.integer == 1) {
        trap_Cvar_VariableStringBuffer("stats_matchid",buf,sizeof(buf));
        if (!buf) {
            G_Printf("MatchID issue, not going to touch stats\n");
            return 0;
        }
        level.match_id = va("%s",buf);
    }
    else {
        G_Printf("Incorrect round, not going to touch stats\n");
        return 0;
    }


    trap_Cvar_VariableStringBuffer( "fs_homepath", hpath, sizeof( hpath ) );
    trap_Cvar_VariableStringBuffer( "fs_game", game, sizeof( game ) );
// for filename we use g_currentRound+1 and so g_currentRound for filename is the previous round

    char* jfile = va("%s/%s/stats/%d_%d_%d/gameStats_match_%s_round_%d_%s.json", hpath, game,ct.tm_mday, ct.tm_mon+1, 1900+ct.tm_year, buf,g_currentRound.integer,mapName);
    json = json_load_file(jfile, 0, &error);

    if (!CanAccessFile("Stats: reading round stats", jfile))
        return 0;

    if (error.line != -1) {
        G_Printf("error: unable to read json round stat file\n");
        return 0;
    }

    if (json)
    {
        object = json_object_get(json, "serverinfo");
        jstattype = json_object_get(object, "g_gameStatslog");
        /* TODO:
            Parse stats depending on output type...
            currently only for g_gameStatslog 16
        */
        G_Printf("gamestatstype: %s\n",json_string_value(jstattype));


        object = json_object_get(json, "gameinfo");
        jstattype = json_object_get(object, "round");
        // double check that this is round 1 stats
        if (!json_string_value(jstattype)) {

            return 0;
        }

        Q_strncpyz(level.jsonStatInfo.round_id,json_string_value(jstattype),sizeof(level.jsonStatInfo.round_id));
        jstattype = json_object_get(object, "match_id");
        if (json_string_value(jstattype)) {
            Q_strncpyz(MATCHID,json_string_value(jstattype),sizeof(MATCHID));
        }

        jstats = json_object_get(json, "stats");

// dont forget to put in checks to make sure objects and arrays are true array/objs
        for(i = 0; i < json_array_size(jstats); i++)
        {
            json_t *data;

            data = json_array_get(jstats, i);
            if(!json_is_object(data))
            {
                fprintf(stderr, "error with reading round 1 stats..not an object\n");
                json_decref(data);
                return 0;
            }
            // loop over clients and see if we find a match for guid
        // TODO: Adjust for new GUID/player key
            for ( j = 0; j < level.numPlayingClients; j++ ) {
                cl = level.clients + level.sortedClients[j];
                sprintf(pGUID,"%s",cl->sess.guid);

                if (json_is_object(json_object_get(data, pGUID)))
                {
                    getPstats(data,pGUID,cl);

                }

            } // j
            

        } //  i
        json_decref(json);
        return 1;

    }

    return 0;
}

/*
===========
G_jstatsByPlayers

Output the end of round stats in Json format with player array...
===========
*/

void G_jstatsByPlayers(qboolean wstats, qboolean clientDisconnected, gclient_t *client) {

	float tot_acc = 0.00f;
	char* statString = "";
	//gclient_t *cl;
	char pGUID[64];
    unsigned int m, dwWeaponMask = 0;
	char strWeapInfo[MAX_STRING_CHARS] = { 0 };
	time_t unixTime = time(NULL);
    json_t* jdata;
    json_t* jstats;
    json_t* jplayer;
    json_t* jcat;
    json_t* weapOb;
    json_t* weapArray;

    // build player stats for players still on the server
    if (!clientDisconnected)
    {
        //for (int j = 0; j < level.numPlayingClients; j++)
        //{
        //    cl = level.clients + level.sortedClients[j];
        //    BuildPlayerStats(cl->ps.clientNum, qfalse);
        //}

        BuildPlayerStats(NULL, qfalse); // call this one time for all clients

        jstats = json_array();

        int t = 0;

        // copy the disconnectedStats data into the playerStats array
        for (int i = 0; i < level.disconnectCount; i++)
        {
            level.playerStats[level.numConnectedClients + i] = level.disconnectStats[i];
        }


        for (int j = 0; j < ArrayLength(level.playerStats); j++)
        {
            if (level.playerStats[j].guid == NULL) // if array index is empty continue
                continue;

            jplayer = json_object();
            jdata = json_object();

            sprintf(pGUID, "%s", level.playerStats[j].guid);

            json_object_set_new(jdata, "alias_colored", json_string(level.playerStats[j].aliasColored));
            json_object_set_new(jdata, "alias", json_string(level.playerStats[j].alias));
            json_object_set_new(jdata, "team", json_string((level.playerStats[j].sessionTeam == TEAM_RED) ? "Axis" : "Allied"));
            json_object_set_new(jdata, "start_time", json_integer(level.playerStats[j].start_time));

            json_object_set_new(jdata, "num_rounds", json_integer(level.playerStats[j].rounds));

            jcat = json_object();

            json_object_set_new(jcat, "kills", json_integer(level.playerStats[j].kills));
            json_object_set_new(jcat, "deaths", json_integer(level.playerStats[j].deaths));
            json_object_set_new(jcat, "gibs", json_integer(level.playerStats[j].gibs));
            json_object_set_new(jcat, "suicides", json_integer(level.playerStats[j].suicides));
            json_object_set_new(jcat, "teamkills", json_integer(level.playerStats[j].team_kills));
            json_object_set_new(jcat, "headshots", json_integer(level.playerStats[j].headshots));
            json_object_set_new(jcat, "damagegiven", json_integer(level.playerStats[j].damage_given));
            json_object_set_new(jcat, "damagereceived", json_integer(level.playerStats[j].damage_received));
            json_object_set_new(jcat, "damageteam", json_integer(level.playerStats[j].team_damage));
            json_object_set_new(jcat, "hits", json_integer(level.playerStats[j].acc_hits));
            json_object_set_new(jcat, "shots", json_integer(level.playerStats[j].acc_shots));
            json_object_set_new(jcat, "accuracy", json_real(((level.playerStats[j].acc_shots == 0) ? 0.00 : ((float)level.playerStats[j].acc_hits / (float)level.playerStats[j].acc_shots) * 100.00f)));
            json_object_set_new(jcat, "revives", json_integer(level.playerStats[j].revives));
            json_object_set_new(jcat, "ammogiven", json_integer(level.playerStats[j].ammo_given));
            json_object_set_new(jcat, "healthgiven", json_integer(level.playerStats[j].med_given));
            json_object_set_new(jcat, "knifekills", json_integer(level.playerStats[j].knifeKills));
            json_object_set_new(jcat, "killpeak", json_integer(level.playerStats[j].killPeak));
            json_object_set_new(jcat, "efficiency", json_real(level.playerStats[j].efficiency));
            json_object_set_new(jcat, "score", json_integer(level.playerStats[j].score));
            json_object_set_new(jcat, "dyn_planted", json_integer(level.playerStats[j].dyn_planted));
            json_object_set_new(jcat, "dyn_defused", json_integer(level.playerStats[j].dyn_defused));
            json_object_set_new(jcat, "obj_captured", json_integer(level.playerStats[j].obj_captured));
            json_object_set_new(jcat, "obj_destroyed", json_integer(level.playerStats[j].obj_destroyed));
            json_object_set_new(jcat, "obj_returned", json_integer(level.playerStats[j].obj_returned));
            json_object_set_new(jcat, "obj_taken", json_integer(level.playerStats[j].obj_taken));
            json_object_set_new(jcat, "obj_checkpoint", json_integer(level.playerStats[j].obj_checkpoint));
            json_object_set_new(jcat, "obj_killcarrier", json_integer(level.playerStats[j].obj_killcarrier));
            json_object_set_new(jcat, "obj_protectflag", json_integer(level.playerStats[j].obj_protectflag));


            weapArray = json_array();
            int i = level.playerStats[j].sessionTeam;

            for (m = WS_KNIFE; m < WS_MAX; m++) {
                if (level.playerStats[j].aWeaponStats[m].atts || level.playerStats[j].aWeaponStats[m].hits ||
                    level.playerStats[j].aWeaponStats[m].deaths) {
                    dwWeaponMask |= (1 << i);
                    weapOb = json_object();
                    json_object_set_new(weapOb, "weapon", json_string((m >= WS_KNIFE && m < WS_MAX) ? aWeaponInfo[m].pszName : "UNKNOWN"));
                    json_object_set_new(weapOb, "kills", json_integer(level.playerStats[j].aWeaponStats[m].kills));
                    json_object_set_new(weapOb, "deaths", json_integer(level.playerStats[j].aWeaponStats[m].deaths));
                    json_object_set_new(weapOb, "headshots", json_integer(level.playerStats[j].aWeaponStats[m].headshots));
                    json_object_set_new(weapOb, "hits", json_integer(level.playerStats[j].aWeaponStats[m].hits));
                    json_object_set_new(weapOb, "shots", json_integer(level.playerStats[j].aWeaponStats[m].atts));
                    json_array_append_new(weapArray, weapOb);
                }
            }

            if (g_gameStatslog.integer & JSON_CATEGORIES) {
                json_object_update(jdata, jcat);
            }
            else {
                json_object_set_new(jdata, "categories", jcat);
            }

            if (wstats) {
                json_object_set_new(jdata, "wstats", weapArray);
                json_object_set_new(jplayer, pGUID, jdata);
            }
            else {
                json_object_set_new(jplayer, pGUID, jdata);
                json_decref(weapArray);
            }

            json_array_append_new(jstats, jplayer);
        }

        

        statString = json_dumps(jstats, 1);
        json_decref(jstats);
    }
    

    if (level.jsonStatInfo.gameStatslogFile && g_gameStatslog.integer && !clientDisconnected)
    {
        if (!CanAccessFile("Stats: writing stats by players", level.jsonStatInfo.gameStatslogFileName))
            return;

        trap_FS_Write("\"stats\": ", strlen("\"stats\": "), level.jsonStatInfo.gameStatslogFile);
        trap_FS_Write(statString, strlen(statString), level.jsonStatInfo.gameStatslogFile);
        trap_FS_Write(",\n", strlen(",\n"), level.jsonStatInfo.gameStatslogFile);  // for writing weapon stats after
        //trap_FS_Write( "\n", strlen( "\n" ), level.jsonStatInfo.gameStatslogFile );
        free(statString);
    }
    else if (clientDisconnected)
    {
        BuildPlayerStats(client, clientDisconnected);
    }

    if (!wstats) {// write weapon stats separately
        G_jWeaponStats();
    }
}

//void AddQuitPlayerStats(char *guid)
//{
//    int i = 0;
//
//    // find the first available index in the array
//    while (i < 12 && level.disconnectStats[i][0] != '\0')
//    {        
//        i++;
//    }
//
//    if (i < 12)
//    {
//        Q_strncpyz(level.disconnectStats[i], guid, sizeof(level.disconnectStats[i]));
//        level.disconnectCount++;
//    }
//    else
//    {
//        G_Printf(va("Disconnect stats array is full. Cannot add player guid: %s", guid));
//    }
//}

/*
===========
G_statsByTeam

Output the end of round stats in Json format with team array ...
===========
*/

void G_jstatsByTeam(qboolean wstats) {

    int i, j, eff,rc;
	float tot_acc = 0.00f;
	char* s;
	gclient_t *cl;
	char n1[MAX_NETNAME];
	char n2[MAX_NETNAME];
	char teamname[10];
	char pGUID[64];
    unsigned int m, dwWeaponMask = 0;
	char strWeapInfo[MAX_STRING_CHARS] = { 0 };
	time_t unixTime = time(NULL);
    json_t* jdata;
    json_t* jteam;
    json_t* jplayer;
    json_t* weapOb;
    json_t* weapArray;
    json_t* jcat;

    jteam =  json_object();

	for ( i = TEAM_RED; i <= TEAM_BLUE; i++ ) {
		if ( !TeamCount( -1, i ) ) {
			continue;
		}
        sprintf(teamname,"%s",(i == TEAM_RED) ? "Axis" : "Allied"  );

        jplayer = json_object();

        for ( j = 0; j < level.numPlayingClients; j++ ) {
			cl = level.clients + level.sortedClients[j];

			if ( cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam != i ) {
				continue;
			}
			DecolorString(cl->pers.netname, n1);
			SanitizeString(n1, n2);
			Q_CleanStr(n2);
			n2[15] = 0;

			eff = ( cl->sess.deaths + cl->sess.kills == 0 ) ? 0 : 100 * cl->sess.kills / ( cl->sess.deaths + cl->sess.kills );
			if ( eff < 0 ) {
				eff = 0;
			}
			sprintf(pGUID,"%s",cl->sess.guid);

            jdata = json_object();
           // json_object_set_new(jdata, "GUID", json_string(cl->sess.guid));
            json_object_set_new(jdata, "alias", json_string(n2));
          //  json_object_set_new(jdata, "team", json_string((i == TEAM_RED) ? "Axis" : "Allied"  ));
            json_object_set_new(jdata, "start_time", json_integer(cl->sess.start_time));   // need to fix
            json_object_set_new(jdata, "num_rounds", json_integer(cl->sess.rounds));

            jcat = json_object();

            json_object_set_new(jcat, "kills", json_integer(cl->sess.kills));
            json_object_set_new(jcat, "deaths", json_integer(cl->sess.deaths));
            json_object_set_new(jcat, "gibs", json_integer(cl->sess.gibs));
            json_object_set_new(jcat, "suicides", json_integer(cl->sess.suicides));
            json_object_set_new(jcat, "teamkills", json_integer(cl->sess.team_kills));
            json_object_set_new(jcat, "headshots", json_integer(cl->sess.headshots));
            json_object_set_new(jcat, "damagegiven", json_integer(cl->sess.damage_given));
            json_object_set_new(jcat, "damagereceived", json_integer(cl->sess.damage_received));
            json_object_set_new(jcat, "damageteam", json_integer(cl->sess.team_damage));
            json_object_set_new(jcat, "hits", json_integer(cl->sess.acc_hits));
            json_object_set_new(jcat, "shots", json_integer(cl->sess.acc_shots));
            json_object_set_new(jcat, "accuracy", json_real(((cl->sess.acc_shots == 0) ? 0.00 : ((float)cl->sess.acc_hits / (float)cl->sess.acc_shots) * 100.00f)));
            json_object_set_new(jcat, "revives", json_integer(cl->sess.revives));
            json_object_set_new(jcat, "ammogiven", json_integer(cl->sess.ammo_given));
            json_object_set_new(jcat, "healthgiven", json_integer(cl->sess.med_given));
            json_object_set_new(jcat, "knifekills", json_integer(cl->sess.knifeKills));
            json_object_set_new(jcat, "killpeak", json_integer(cl->sess.killPeak));
            json_object_set_new(jcat, "efficiency", json_real(eff));
            // The following objects are not stored over multiple rounds....need to add to g_session if we want these to reflect multiple rounds
            json_object_set_new(jcat, "score", json_integer(cl->ps.persistant[PERS_SCORE]));
            json_object_set_new(jcat, "dyn_planted", json_integer(cl->sess.dyn_planted));
            json_object_set_new(jcat, "dyn_defused", json_integer(cl->sess.dyn_defused));
            json_object_set_new(jcat, "obj_captured", json_integer(cl->sess.obj_captured));
            json_object_set_new(jcat, "obj_destroyed", json_integer(cl->sess.obj_destroyed));
            json_object_set_new(jcat, "obj_returned", json_integer(cl->sess.obj_returned));
            json_object_set_new(jcat, "obj_taken", json_integer(cl->sess.obj_taken));
            json_object_set_new(jcat, "obj_checkpoint", json_integer(cl->sess.obj_checkpoint));
            json_object_set_new(jcat, "obj_killcarrier", json_integer(cl->sess.obj_killcarrier));
            json_object_set_new(jcat, "obj_protectflag", json_integer(cl->sess.obj_protectflag));

            weapArray = json_array();

            for (m = WS_KNIFE; m < WS_MAX; m++) {
                if (cl->sess.aWeaponStats[m].atts || cl->sess.aWeaponStats[m].hits ||
                    cl->sess.aWeaponStats[m].deaths) {
                        dwWeaponMask |= (1 << i);
                        weapOb = json_object();
                        json_object_set_new(weapOb, "weapon", json_string((m >= WS_KNIFE && m < WS_MAX ) ? aWeaponInfo[m].pszName : "UNKNOWN" ));
                        json_object_set_new(weapOb, "kills", json_integer(cl->sess.aWeaponStats[m].kills));
                        json_object_set_new(weapOb, "deaths", json_integer(cl->sess.aWeaponStats[m].deaths));
                        json_object_set_new(weapOb, "headshots", json_integer(cl->sess.aWeaponStats[m].headshots));
                        json_object_set_new(weapOb, "hits", json_integer(cl->sess.aWeaponStats[m].hits));
                        json_object_set_new(weapOb, "shots", json_integer(cl->sess.aWeaponStats[m].atts));
                        json_array_append(weapArray, weapOb);
                        json_decref(weapOb);
                }
            }
/*
            if (wstats) {  // include wstats with player stats
                json_object_set(jdata, "wstats", weapArray);
                json_object_set(jplayer, pGUID, jdata);
                json_decref(weapArray);
            }
            else {
                json_object_set(jplayer, pGUID, jdata);
            }
*/
            if (g_gameStatslog.integer & JSON_CATEGORIES) {
                json_object_update(jdata, jcat);
            }
            else {
                json_object_set(jdata, "categories", jcat);
            }

            if (wstats) {
                json_object_set(jdata, "wstats", weapArray);
                json_object_set(jplayer, pGUID, jdata);
                json_decref(weapArray);
            }
            else {
                json_object_set(jplayer, pGUID, jdata);
            }
            json_decref(jdata);

        }


        json_object_set(jteam, teamname, jplayer);

    }
        s = json_dumps( jteam, 1 ); // for a pretty print form
        //s = json_dumps( jteam, 0 );

        if (level.jsonStatInfo.gameStatslogFile && g_gameStatslog.integer)
        {
            if (!CanAccessFile("Stats: writing stats by team", level.jsonStatInfo.gameStatslogFileName))
                return;

            trap_FS_Write( "\"stats\": ", strlen( "\"stats\": " ), level.jsonStatInfo.gameStatslogFile );
            trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );
            trap_FS_Write( ",\n", strlen( ",\n" ), level.jsonStatInfo.gameStatslogFile );  // for writing weapon stats after
            //trap_FS_Write( "\n", strlen( "\n" ), level.jsonStatInfo.gameStatslogFile ); // for keeping weapon stats in playerstats
            free( s );
        }



        if (!wstats) {// write weapon stats separately
            G_jWeaponStats();
        }
}

/*
===========
G_jWeaponStats

Output the weapon stats for each player
===========
*/

void G_jWeaponStats(void) {

    int i, j, rc;
	char* s;
    char pGUID[64];
    unsigned int m, dwWeaponMask = 0;
	char strWeapInfo[MAX_STRING_CHARS] = { 0 };
	time_t unixTime = time(NULL);
    json_t* jwstat;
    json_t* jplayer;
    json_t* weapOb;
    json_t* weapArray;


    jwstat =  json_array();

    for (j = 0; j < ArrayLength(level.playerStats); j++)
    {
        if (level.playerStats[j].guid == NULL) // if array index is empty continue
            continue;

        jplayer = json_object();

		sprintf(pGUID,"%s", level.playerStats[j].guid);
        weapArray = json_array();

        i = level.playerStats[j].sessionTeam;

        for (m = WS_KNIFE; m < WS_MAX; m++) {
            if (level.playerStats[j].aWeaponStats[m].atts || level.playerStats[j].aWeaponStats[m].hits ||
                level.playerStats[j].aWeaponStats[m].deaths) {
                    dwWeaponMask |= (1 << i);
                    weapOb = json_object();
                    json_object_set_new(weapOb, "weapon", json_string((m >= WS_KNIFE && m < WS_MAX ) ? aWeaponInfo[m].pszName : "UNKNOWN" ));
                    json_object_set_new(weapOb, "kills", json_integer(level.playerStats[j].aWeaponStats[m].kills));
                    json_object_set_new(weapOb, "deaths", json_integer(level.playerStats[j].aWeaponStats[m].deaths));
                    json_object_set_new(weapOb, "headshots", json_integer(level.playerStats[j].aWeaponStats[m].headshots));
                    json_object_set_new(weapOb, "hits", json_integer(level.playerStats[j].aWeaponStats[m].hits));
                    json_object_set_new(weapOb, "shots", json_integer(level.playerStats[j].aWeaponStats[m].atts));
                    json_array_append(weapArray, weapOb);
                    json_decref(weapOb);
            }
        }
        json_object_set(jplayer, pGUID, weapArray);
        json_array_append(jwstat, jplayer);
        json_decref(weapArray);
        json_decref(jplayer);

    }




    //s = json_dumps( jwstat, 0 ); // for a pretty print form
    s = json_dumps( jwstat, 1 ); // for a pretty print form

    if (level.jsonStatInfo.gameStatslogFile && g_gameStatslog.integer) {

        if (!CanAccessFile("Stats: writing weapon stats", level.jsonStatInfo.gameStatslogFileName)){
            json_decref(jwstat);
            free(s);
            return;
        }
            

        trap_FS_Write( "\"wstats\": ", strlen( "\"wstats\": " ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );

        trap_FS_Write( "\n", strlen( "\n" ), level.jsonStatInfo.gameStatslogFile );
        free( s );
    }

    json_decref(jwstat);
}




/*
===========
writeServerInfo

Output server related information
===========
*/
void G_writeServerInfo(void) {

    //char* buf;
    char* s;
    char mapName[MAX_QPATH];
    char server_ip[16];
    char server_country[3];
    char gameConfig[MAX_QPATH];
    time_t unixTime = time(NULL);
    //char cs[MAX_STRING_CHARS];
	qtime_t ct;
	trap_RealTime(&ct);

	// we want to save some information for the match and round
    //trap_GetConfigstring( CS_ROUNDINFO, cs, sizeof( cs ) );

    G_read_match_info();

    trap_Cvar_VariableStringBuffer( "mapname", mapName, sizeof(mapName) );
    trap_Cvar_VariableStringBuffer( "sv_GameConfig", gameConfig, sizeof(gameConfig) );
	trap_Cvar_VariableStringBuffer("sv_serverIP", server_ip, sizeof(server_ip));
    trap_Cvar_VariableStringBuffer("sv_serverCountry", server_country, sizeof(server_country));

    json_t *jdata = json_object();
    json_object_set_new(jdata, "serverName",    json_string(sv_hostname.string));
    json_object_set_new(jdata, "serverIP",    json_string(va("%s",server_ip)));
    json_object_set_new(jdata, "serverCountry",    json_string(va("%s",server_country)));
    json_object_set_new(jdata, "gameVersion",    json_string(GAMEVERSION));
    json_object_set_new(jdata, "jsonGameStatVersion",    json_string(JSONGAMESTATVERSION));
    json_object_set_new(jdata, "g_gameStatslog",    json_string(va("%i", g_gameStatslog.integer)));
    json_object_set_new(jdata, "sv_GameConfig", json_string(va("%s", gameConfig)));
    json_object_set_new(jdata, "g_gametype",    json_string(va("%i",g_gametype.integer)));
    json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));

    if (level.jsonStatInfo.gameStatslogFile) {

        if (!CanAccessFile("Stats: writing server info", level.jsonStatInfo.gameStatslogFileName))
            return;

        s = json_dumps( jdata, 1 );
        //s = json_dumps( jdata, 0 );

        trap_FS_Write( "{\n \"serverinfo\": \n", strlen( "{\n \"serverinfo\": \n" ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( ",\n", strlen( ",\n" ), level.jsonStatInfo.gameStatslogFile );
        json_decref(jdata);

        free(s);
    }

    G_writeGameLogStart();  // write necessary json info for gamelog array....will provide better solution later

}


/*
===========
writeGameInfo

Output end of info (i.e. round, winner, etc)
===========
*/

void G_writeGameInfo (int winner ) {

	char* s;
	char mapName[64];
    /*char buf[64];
    char *buf2;
    char *buf3;
    char *buf4;
    char cs[MAX_STRING_CHARS];*/

    //time_t unixTime = time(NULL);
    trap_Cvar_VariableStringBuffer( "mapname", mapName, sizeof(mapName) );


 //   trap_GetConfigstring(CS_ROUNDINFO, cs, sizeof(cs));  // retrieve round/match info saved

    json_t *jdata = json_object();

//    trap_Cvar_VariableStringBuffer("stats_matchid",buf,sizeof(buf));
    json_object_set_new(jdata, "match_id",    json_string(va("%s",MATCHID)));


    json_object_set_new(jdata, "round",    json_string(va("%s",ROUNDID)));

    int roundStart = atoi(level.jsonStatInfo.round_start);
    int roundTotalSeconds = ((level.timeCurrent - level.startTime) / 60000.0f) * 60;
    int roundEnd = roundStart + roundTotalSeconds;

    json_object_set_new(jdata, "round_start",    json_string(va("%s",level.jsonStatInfo.round_start)));
    json_object_set_new(jdata, "round_end",    json_string(va("%ld", roundEnd))); // KK unixTime doesn't account for pause time
    json_object_set_new(jdata, "map",    json_string(mapName));

    json_object_set_new(jdata, "time_limit",    json_string(va("%s",level.jsonStatInfo.round_timelimit)));

    json_object_set_new(jdata, "allies_cycle",    json_string(va("%i",g_bluelimbotime.integer / 1000)));
    json_object_set_new(jdata, "axis_cycle",    json_string(va("%i",g_redlimbotime.integer / 1000)));

    json_object_set_new(jdata, "winner", json_string(va("%s", (winner == 0) ? "Axis" : "Allied")));

    // note we want to write the winner on the second round but since this is called at
    // the end of each round we only write out when g_currentRound = 0
    /*
    if (g_currentRound.integer == 0) {
        json_object_set_new(jdata, "winner",    json_string(va("%s", (winner == 0) ? "Axis" : "Allied")));
    }
    else {
        json_object_set_new(jdata, "winner",    json_string(" "));
    }
    */

    if (level.jsonStatInfo.gameStatslogFile) {

        if (!CanAccessFile("Stats: writing game info", level.jsonStatInfo.gameStatslogFileName))
            return;

        //s = json_dumps( jdata, 0 );
        s = json_dumps( jdata, 1 );
        trap_FS_Write( "\"gameinfo\": \n", strlen( "\"gameinfo\": \n" ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( ",\n", strlen( ",\n" ), level.jsonStatInfo.gameStatslogFile );
        json_decref(jdata);
        free(s);

    }


}

// Probably should have just made an array of all events to loop over.....bah will do that when more events are added

void G_writeObjectiveEvent (gentity_t* agent,int objType) {

    char* s;
    //char buf[64];
    json_t *jdata = json_object();
    time_t unixTime = time(NULL);
        // additional safety check
    if (!g_gameStatslog.integer || g_gamestate.integer != GS_PLAYING) {
        return;
    }
   // trap_Cvar_VariableStringBuffer("stats_matchid",buf,sizeof(buf));

    json_object_set_new(jdata, "match_id",    json_string(va("%s",MATCHID)));
    json_object_set_new(jdata, "round_id",    json_string(va("%s",ROUNDID)));
    json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
    json_object_set_new(jdata, "group",    json_string("player"));
    switch ( objType ) {
        case objDropped:
            json_object_set_new(jdata, "label",    json_string("ObjDropped"));
            break;
        case objReturned:
            json_object_set_new(jdata, "label",    json_string("ObjReturned"));
            break;
        case objTaken:
            json_object_set_new(jdata, "label",    json_string("ObjTaken"));
            break;
        case objCapture:
            json_object_set_new(jdata, "label",    json_string("ObjCapture"));
            break;
        case objDynDefuse:
            json_object_set_new(jdata, "label",    json_string("ObjDynDefused"));
            break;
        case objDynPlant:
            json_object_set_new(jdata, "label",    json_string("ObjDynPlanted"));
            break;
        case objSpawnFlag:
            json_object_set_new(jdata, "label",    json_string("ObjSpawnFlagCaptured"));
            break;
        case objDestroyed:
            json_object_set_new(jdata, "label",    json_string("ObjDestroyed"));
            break;
        case objKilledCarrier:
            json_object_set_new(jdata, "label",    json_string("ObjKilledCarrier"));
            break;
        case objProtectFlag:
            json_object_set_new(jdata, "label",    json_string("ObjProtectFlag"));
            break;
        default:
            json_object_set_new(jdata, "label",    json_string("unknown_event"));
            break;
    }
    // json_object_set_new(jdata, "team",    json_string(team));
    json_object_set_new(jdata, "agent",    json_string(agent->client->sess.guid));

    if (level.jsonStatInfo.gameStatslogFile) {

        if (!CanAccessFile("Stats: writing Objective event", level.jsonStatInfo.gameStatslogFileName))
            return;

        s = json_dumps( jdata, 0 );
        trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( ",\n", strlen( ",\n" ), level.jsonStatInfo.gameStatslogFile );
        free(s);

    }
    json_decref(jdata);


    level.eventNum++;
}


void G_writeChatEvent(gentity_t* agent, const char* chatText)
{
    // if client wrote same thing as their last chat return to prevent spamming gamelog
    if (!Q_stricmp(agent->client->sess.lastChatText, chatText))
        return;

    agent->client->sess.lastChatText = (char *)chatText;

    char* s;
    json_t* jdata = json_object();
    json_t* event = json_object();
    time_t unixTime = time(NULL);

    json_object_set_new(jdata, "match_id", json_string(va("%s", MATCHID)));
    json_object_set_new(jdata, "round_id", json_string(va("%s", ROUNDID)));
    json_object_set_new(jdata, "unixtime", json_string(va("%ld", unixTime)));
    json_object_set_new(jdata, "group", json_string("server"));
    json_object_set_new(jdata, "label", json_string("global_chat"));
    json_object_set_new(jdata, "agent", json_string(va("%s", agent->client->sess.guid)));
    json_object_set_new(jdata, "text", json_string(va("%s", chatText)));

    if (level.jsonStatInfo.gameStatslogFile) {

        if (!CanAccessFile("Stats: writing global chat event", level.jsonStatInfo.gameStatslogFileName))
            return;

        s = json_dumps(jdata, 0);
        trap_FS_Write(s, strlen(s), level.jsonStatInfo.gameStatslogFile);
        trap_FS_Write(",\n", strlen(",\n"), level.jsonStatInfo.gameStatslogFile);

        free(s);
    }
    json_decref(jdata);
    level.eventNum++;
}


void G_writeGeneralEvent (gentity_t* agent,gentity_t* other, char* weapon, int eventType) {
    // additional safety check
    if ( g_gamestate.integer != GS_PLAYING ) {
        return;
    }

    char* s;
    //char buf[64];
    char* pclass;
    json_t *jdata = json_object();
    time_t unixTime = time(NULL);
    

    json_object_set_new(jdata, "match_id",    json_string(va("%s",MATCHID)));

    json_object_set_new(jdata, "round_id",    json_string(va("%s",ROUNDID)));
    json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));

    switch ( eventType ) {
	    case eventSuicide:
            json_object_set_new(jdata, "group",    json_string("player"));
            json_object_set_new(jdata, "label",    json_string("suicide"));
            json_object_set_new(jdata, "agent",    json_string(va("%s",agent->client->sess.guid)));
            break;
	    case eventKill:
            json_object_set_new(jdata, "group",    json_string("player"));
            json_object_set_new(jdata, "label",    json_string("kill"));
            json_object_set_new(jdata, "agent",    json_string(va("%s",agent->client->sess.guid)));
            json_object_set_new(jdata, "other",    json_string(va("%s",other->client->sess.guid)));
            json_object_set_new(jdata, "weapon",    json_string(weapon));
            json_object_set_new(jdata, "agent_health",    json_integer(agent->health));
            if (g_gameStatslog.integer & JSON_KILLDATA) {
                json_object_set_new(jdata, "agent_pos",    json_string(va("%f,%f,%f",agent->client->ps.origin[0],agent->client->ps.origin[1],agent->client->ps.origin[2])));
                json_object_set_new(jdata, "agent_angle",    json_string(va("%f",agent->client->ps.viewangles[1])));
                json_object_set_new(jdata, "other_pos",    json_string(va("%f,%f,%f",other->client->ps.origin[0],other->client->ps.origin[1],other->client->ps.origin[2])));
                json_object_set_new(jdata, "other_angle",    json_string(va("%f",other->client->ps.viewangles[1])));
                // straight up stupid way to do this...
                int axisAlive, alliedAlive;
                axisAlive = G_teamAlive(TEAM_RED);
                alliedAlive = G_teamAlive(TEAM_BLUE);
                json_object_set_new(jdata, "allies_alive",    json_string(va("%i",alliedAlive)));
                json_object_set_new(jdata, "axis_alive",    json_string(va("%i",axisAlive)));

            }
            break;
	    case eventTeamkill:
            json_object_set_new(jdata, "group",    json_string("player"));
            json_object_set_new(jdata, "label",    json_string("teamkill"));
            json_object_set_new(jdata, "agent",    json_string(va("%s",agent->client->sess.guid)));
            json_object_set_new(jdata, "other",    json_string(va("%s",other->client->sess.guid)));
            json_object_set_new(jdata, "weapon",    json_string(weapon));
            json_object_set_new(jdata, "other_health",    json_integer(agent->health));
            break;
	    case eventRevive:
            json_object_set_new(jdata, "group",    json_string("player"));
            json_object_set_new(jdata, "label",   json_string("revive"));
            json_object_set_new(jdata, "agent",  json_string(va("%s",agent->client->sess.guid)));
            json_object_set_new(jdata, "other",  json_string(va("%s",other->client->sess.guid)));
            break;
	    case eventPause:
            json_object_set_new(jdata, "group",    json_string("server"));
            json_object_set_new(jdata, "label",   json_string("pause"));
            json_object_set_new(jdata, "other",  json_string(va("%s",agent->client->sess.guid)));
            break;
	    case eventUnpause:
            json_object_set_new(jdata, "group",    json_string("server"));
            json_object_set_new(jdata, "label",   json_string("unpause"));
            json_object_set_new(jdata, "other",  json_string(va("%s",agent->client->sess.guid)));
            break;
	    case teamFirstSpawn:
            //json_object_set_new(jdata, "event",   json_string("respawnTimer"));
            json_object_set_new(jdata, "group",    json_string("server"));
            json_object_set_new(jdata, "label",   json_string("firstRespawn"));
            int redRespawnTime = (g_redlimbotime.integer - level.dwRedReinfOffset) / 1000;
            int blueRespawnTime = (g_bluelimbotime.integer - level.dwBlueReinfOffset) / 1000;
            json_object_set_new(jdata, "Axis",  json_string(va("%ld",unixTime +redRespawnTime)));
            json_object_set_new(jdata, "Allied",  json_string(va("%ld",unixTime +blueRespawnTime)));
            break;
	    case eventClassChange:
            if (agent->client->sess.playerType == PC_MEDIC) {pclass = "M";}
            else if (agent->client->sess.playerType == PC_SOLDIER) {pclass = "S";}
            else if (agent->client->sess.playerType == PC_LT) {pclass = "L";}
            else if (agent->client->sess.playerType == PC_ENGINEER) {pclass = "E";}
            else {pclass="X";}
            json_object_set_new(jdata, "group",    json_string("player"));
            json_object_set_new(jdata, "label",   json_string("class_change"));
            json_object_set_new(jdata, "agent",  json_string(va("%s",agent->client->sess.guid)));
            //json_object_set_new(jdata, "other",  json_string(va("%i",agent->client->sess.playerType)));
            json_object_set_new(jdata, "other",  json_string(va("%s",pclass)));
            break;
	    case eventNameChange:
		    json_object_set_new(jdata, "group",    json_string("player"));
            json_object_set_new(jdata, "label",   json_string("name_change"));
            json_object_set_new(jdata, "agent",  json_string(va("%s",agent->client->sess.guid)));
            json_object_set_new(jdata, "other",  json_string(va("%s",agent->client->pers.netname)));
            break;
        default:
            json_object_set_new(jdata, "group",    json_string("server"));
            json_object_set_new(jdata, "label",   json_string("unknown event"));
            break;
	}

    if (level.jsonStatInfo.gameStatslogFile) {

        if (!CanAccessFile(va("Stats: writing %s event", LookupEventType(eventType)), level.jsonStatInfo.gameStatslogFileName)){
            json_decref(jdata);
            return;
        }

        s = json_dumps( jdata, 0 );
        trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( ",\n", strlen( ",\n" ), level.jsonStatInfo.gameStatslogFile );

        free(s);
    }
    json_decref(jdata);
    level.eventNum++;
}

/*
    Unused at the moment.....plan to remove once set on formating and such
     dir is direction from which the attack occured
*/
void G_writeCombatEvent (gentity_t* agent,gentity_t* other, vec3_t dir) {
    char* s;
    json_t *jdata = json_object();
    time_t unixTime = time(NULL);
        // additional safety check
    if ( g_gamestate.integer != GS_PLAYING ) {
        return;
    }
    json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
    json_object_set_new(jdata, "group",    json_string("combat"));
    json_object_set_new(jdata, "label",    json_string("kill"));
    json_object_set_new(jdata, "agent",    json_string(va("%s",agent->client->sess.guid)));
    json_object_set_new(jdata, "agent_loc",    json_string(va("%f,%f,%f",agent->client->ps.origin[0],agent->client->ps.origin[1],agent->client->ps.origin[2])));
    json_object_set_new(jdata, "agent_angles",    json_string(va("%f,%f,%f",agent->client->ps.viewangles[0],agent->client->ps.viewangles[1],agent->client->ps.viewangles[2])));
    json_object_set_new(jdata, "other",    json_string(va("%s",other->client->sess.guid)));
    json_object_set_new(jdata, "other_loc",    json_string(va("%f,%f,%f",other->client->ps.origin[0],other->client->ps.origin[1],other->client->ps.origin[2])));
    json_object_set_new(jdata, "other_angles",    json_string(va("%f,%f,%f",other->client->ps.viewangles[0],other->client->ps.viewangles[1],other->client->ps.viewangles[2])));
    json_object_set_new(jdata, "attack_dir",    json_string(va("%f,%f,%f",dir[1],dir[2],dir[3])));
    if (level.jsonStatInfo.gameStatslogFile) {
        s = json_dumps( jdata, 0 );
        trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( ",\n", strlen( ",\n" ), level.jsonStatInfo.gameStatslogFile );
        free(s);
    }
    json_decref(jdata);
}


void G_writeDisconnectEvent (gentity_t* agent) {

    char* s;
    //char buf[64];
    json_t *jdata = json_object();
    time_t unixTime = time(NULL);
    // additional safety check
    if ( g_gamestate.integer != GS_PLAYING || agent->client->sess.sessionTeam == TEAM_SPECTATOR) {
        return;
    }

    json_object_set_new(jdata, "match_id",    json_string(va("%s",MATCHID)));
    json_object_set_new(jdata, "round_id",    json_string(va("%s",ROUNDID)));
    json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
    json_object_set_new(jdata, "group",    json_string("player"));
    json_object_set_new(jdata, "label",    json_string("disconnect"));
    json_object_set_new(jdata, "agent",    json_string(va("%s",agent->client->sess.guid)));

    if (level.jsonStatInfo.gameStatslogFile) {

        if (!CanAccessFile("Stats: writing disconnect event", level.jsonStatInfo.gameStatslogFileName))
            return;

        s = json_dumps( jdata, 0 );
        trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( ",\n", strlen( ",\n" ), level.jsonStatInfo.gameStatslogFile );
        free(s);
    }
    json_decref(jdata);

}
// the following 3 functions are silly but for the time being necessary to make the output a true 'json'

void G_writeClosingJson(void)
{
    char buf[64];
    int ret = 0;
    if (level.jsonStatInfo.gameStatslogFile) {
        trap_FS_Write( "}\n", strlen( "}\n"), level.jsonStatInfo.gameStatslogFile );
        trap_FS_FCloseFile(level.jsonStatInfo.gameStatslogFile );

        // check stats file to make sure it satisfies conditions for submission....
        ret = G_check_before_submit(level.jsonStatInfo.gameStatslogFileName);

        if (g_stats_curl_submit.integer && ret > 0) {
            trap_Cvar_VariableStringBuffer("stats_matchid",buf,sizeof(buf));
            G_Printf("Stats API: Starting stats upload process.\n");

            if ( level.jsonStatInfo.gameStatslogFile && buf ) {

                if (!CanAccessFile("Stats: writing closing json", level.jsonStatInfo.gameStatslogFileName))
                    return;

                // prevent from writing to this file because stats are submitting
                char fileNametoSubmit[256];
                sprintf(fileNametoSubmit, "%s", level.jsonStatInfo.gameStatslogFileName);
                Com_sprintf(level.jsonStatInfo.gameStatslogFileName, sizeof(level.jsonStatInfo.gameStatslogFileName), "");

                trap_submit_curlPost(fileNametoSubmit, va("%s",buf));
            }
            else {
                G_Printf("Stats API: No file to upload. Skipping.\n");
            }

        }
        Com_sprintf(level.jsonStatInfo.gameStatslogFileName, sizeof(level.jsonStatInfo.gameStatslogFileName), ""); // prevent from writing to this file because we just closed it
    }



}


void G_writeGameLogStart(void)
{
    char* s;
    json_t *jdata = json_object();
    time_t unixTime = time(NULL);
    //char *buf3;
    char buf[64];
    //char cs[MAX_STRING_CHARS];
    trap_Cvar_VariableStringBuffer("stats_matchid",buf,sizeof(buf));

    if (level.jsonStatInfo.gameStatslogFile) {

        if (!CanAccessFile("Stats: writing game log start", level.jsonStatInfo.gameStatslogFileName))
            return;

        trap_FS_Write( "\"gamelog\": [\n", strlen( "\"gamelog\": [\n"), level.jsonStatInfo.gameStatslogFile );
        json_object_set_new(jdata, "match_id",    json_string(va("%s",buf)));
        json_object_set_new(jdata, "round_id",    json_string(va("%s",ROUNDID)));
        json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));

        json_object_set_new(jdata, "group",    json_string("server"));
        json_object_set_new(jdata, "label",    json_string("round_start"));

        s = json_dumps( jdata, 0 );
        trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( ",\n", strlen( ",\n" ), level.jsonStatInfo.gameStatslogFile );
        json_decref(jdata);

        free(s);

    }


}




void G_writeGameLogEnd(void)
{
    char* s;
    //char buf[64];
    json_t *jdata = json_object();
    time_t unixTime = time(NULL);
    json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
    json_object_set_new(jdata, "match_id",    json_string(va("%s",MATCHID)));
    json_object_set_new(jdata, "round_id",    json_string(va("%s",ROUNDID)));
    json_object_set_new(jdata, "group",    json_string("server"));
    json_object_set_new(jdata, "label",    json_string("round_end"));

    if (level.jsonStatInfo.gameStatslogFile) {

        if (!CanAccessFile("Stats: writing game log end", level.jsonStatInfo.gameStatslogFileName)){
            json_decref(jdata);
            return;
        }

        s = json_dumps( jdata, 0 );
        trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( "\n", strlen( "\n" ), level.jsonStatInfo.gameStatslogFile );
        json_decref(jdata);
        free(s);
        trap_FS_Write( "],\n", strlen( "],\n" ), level.jsonStatInfo.gameStatslogFile );
    }else{
        json_decref(jdata);
    }

}


// Close up the file properly on map restarts

void G_writeGameEarlyExit(void)
{
    char* s;
    char buf[64];
    json_t *jdata = json_object();
    json_t *event = json_object();
    time_t unixTime = time(NULL);
    json_t *eventStats =  json_array();

    trap_Cvar_VariableStringBuffer("stats_matchid",buf,sizeof(buf));

    json_object_set_new(jdata, "match_id",    json_string(va("%s",MATCHID)));
    json_object_set_new(jdata, "round_id",    json_string(va("%s",ROUNDID)));
    json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
    json_object_set_new(jdata, "group",    json_string("server"));
    json_object_set_new(jdata, "label",    json_string("map_restart"));

    if (level.jsonStatInfo.gameStatslogFile) {

        if (!CanAccessFile("Stats: writing game early exit", level.jsonStatInfo.gameStatslogFileName))
            return;

        s = json_dumps( jdata, 0 );
        trap_FS_Write( s, strlen( s ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_Write( "\n", strlen( "\n" ), level.jsonStatInfo.gameStatslogFile );
        json_decref(jdata);
        free(s);
        trap_FS_Write( "]\n}\n", strlen( "]\n}\n" ), level.jsonStatInfo.gameStatslogFile );
        trap_FS_FCloseFile(level.jsonStatInfo.gameStatslogFile );
       /*
       // do not submit rounds that exit early
        if (g_stats_curl_submit.integer) {
            trap_submit_curlPost(level.jsonStatInfo.gameStatslogFileName, va("%s",buf));
        }
      */
    }


}


/*
 number of enemies alive .... will move elsewhere (if we decide to keep it)
  note: for efficiency purposes we should introduce new level vars
  level.axisalive and level.alliedalive then increment/decrement where needed
*/
int G_teamAlive(int team ) {

    //if (!CanAccessFile("Stats: checking for alive players", level.jsonStatInfo.gameStatslogFileName))
    //    return 0;

    int  j;
	gclient_t *cl;
    int numAlive = 0;

    for ( j = 0; j < level.numPlayingClients; j++ ) {
			cl = level.clients + level.sortedClients[j];

			if ( cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam != team ) {
				continue;
			}

            if (!(cl->ps.pm_flags & PMF_LIMBO) && !(cl->ps.pm_type == PM_DEAD)) {
                numAlive++;
            }
        }
    return numAlive;

}

char* LookupEventType(int eventType)
{
    switch (eventType) {
    case eventSuicide:
        return "Suicide";
        break;
    case eventKill:
        return "Kill";
        break;
    case eventTeamkill:
        return "TeamKill";
        break;
    case eventRevive:
        return "Revive";
        break;
    case eventPause:
        return "Pause";
        break;
    case eventUnpause:
        return "Unpause";
        break;
    case teamFirstSpawn:
        return "FirstSpawn";
        break;
    case eventClassChange:
        return "Class";
        break;
    case eventNameChange:
        return "NameChange";
        break;
    default:
        return "Unknown event";
        break;
    }
}