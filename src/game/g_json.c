#include "g_local.h"
#include "g_stats.h"
#include <jansson.h>
//#include "../qcommon/jansson/jansson.h"
#include <time.h>
// send/receive to json server: only need to include a few additional functions
//   although tested and works.....not necessary until we
//#define URL_FORMAT   "https://192.168.1.2:3000/gameStats"
//#define URL_SIZE     256

/*
===========
stats2JSON

Output the end of round stats in Json format ...
 ... should rename this function to something more appropriate
===========
*/

void G_stats2JSON(int winner ) {

    int i, j, eff,rc;
	float tot_acc = 0.00f;
	char* s;
	gclient_t *cl;
	char mapName[64];
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
    json_t *root = json_object();
    json_t* weapArray;

    trap_Cvar_VariableStringBuffer( "mapname", mapName, sizeof(mapName) );

	qtime_t ct;
	trap_RealTime(&ct);
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
            json_object_set_new(jdata, "end_time", json_integer(cl->sess.end_time));   // need to fix
            json_object_set_new(jdata, "rounds", json_integer(cl->sess.rounds));
            json_object_set_new(jdata, "kills", json_integer(cl->sess.kills));
            json_object_set_new(jdata, "deaths", json_integer(cl->sess.deaths));
            json_object_set_new(jdata, "gibs", json_integer(cl->sess.gibs));
            json_object_set_new(jdata, "suicides", json_integer(cl->sess.suicides));
            json_object_set_new(jdata, "teamkills", json_integer(cl->sess.team_kills));
            json_object_set_new(jdata, "headshots", json_integer(cl->sess.headshots));
            json_object_set_new(jdata, "damagegiven", json_integer(cl->sess.damage_given));
            json_object_set_new(jdata, "damagereceived", json_integer(cl->sess.damage_received));
            json_object_set_new(jdata, "damageteam", json_integer(cl->sess.team_damage));
            json_object_set_new(jdata, "hits", json_integer(cl->sess.acc_hits));
            json_object_set_new(jdata, "shots", json_integer(cl->sess.acc_shots));
            json_object_set_new(jdata, "accuracy", json_real(((cl->sess.acc_shots == 0) ? 0.00 : ((float)cl->sess.acc_hits / (float)cl->sess.acc_shots) * 100.00f)));
            json_object_set_new(jdata, "revives", json_integer(cl->sess.revives));
            json_object_set_new(jdata, "ammogiven", json_integer(cl->sess.ammo_given));
            json_object_set_new(jdata, "healthgiven", json_integer(cl->sess.med_given));
            json_object_set_new(jdata, "poisoned", json_integer(cl->sess.poisoned));
            json_object_set_new(jdata, "knifekills", json_integer(cl->sess.knifeKills));
            json_object_set_new(jdata, "killpeak", json_integer(cl->sess.killPeak));
            json_object_set_new(jdata, "efficiency", json_real(eff));
            // The following objects are not stored over multiple rounds....need to add to g_session if we want these to reflect multiple rounds
            json_object_set_new(jdata, "score", json_integer(cl->ps.persistant[PERS_SCORE]));
            json_object_set_new(jdata, "dyn_planted", json_integer(cl->sess.dyn_planted));
            json_object_set_new(jdata, "dyn_defused", json_integer(cl->sess.dyn_defused));
            json_object_set_new(jdata, "obj_captured", json_integer(cl->sess.obj_captured));
            json_object_set_new(jdata, "obj_destroyed", json_integer(cl->sess.obj_destroyed));
            json_object_set_new(jdata, "obj_returned", json_integer(cl->sess.obj_returned));
            json_object_set_new(jdata, "obj_taken", json_integer(cl->sess.obj_taken));

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

            json_object_set(jdata, "wstats", weapArray);
            json_object_set(jplayer, pGUID, jdata);
            json_decref(weapArray);
            json_decref(jdata);

        }

        json_object_set(jteam, teamname, jplayer);

    }

        s = json_dumps( jteam, 1 ); // for a pretty print form

        if (level.gameStatslogFile && g_gameStatslog.integer) {
            trap_FS_Write( "\"players\": ", strlen( "\"players\": " ), level.gameStatslogFile );
            trap_FS_Write( s, strlen( s ), level.gameStatslogFile );

            trap_FS_Write( "\n", strlen( "\n" ), level.gameStatslogFile );
        }
        else {   // forget the comments above and write it to original test json file :)
            rc = json_dump_file(root, "./test.json", 0);
            if (rc) {
                fprintf(stderr, "cannot save json to file\n");
            }
        }

        free( s );
        json_decref( root );
}
/*
===========
writeServerInfo

Output server related information
===========
*/

void G_writeServerInfo (void){
	//int s;
	char* s;
	char mapName[64];
    char newGamestatFile[MAX_QPATH];
    FILE		*gsfile;
    time_t unixTime = time(NULL);  // come back and make globally available
    trap_Cvar_VariableStringBuffer( "mapname", mapName, sizeof(mapName) );
    char *buf;
    char cs[MAX_STRING_CHARS];

	qtime_t ct;
	trap_RealTime(&ct);
	// we want to save some information for the match and round
    trap_GetConfigstring( CS_ROUNDINFO, cs, sizeof( cs ) );

    Info_SetValueForKey( cs, "roundStart", va("%ld", unixTime) );
    Info_SetValueForKey( cs, "round", va("%i",g_currentRound.integer));

    if (g_currentRound.integer == 0) {
        Info_SetValueForKey( cs, "matchid", va("%ld", unixTime) );
    }
    trap_SetConfigstring( CS_ROUNDINFO, cs );

    json_t *jdata = json_object();
   // json_t *jinfo = json_object();
        json_object_set_new(jdata, "serverName",    json_string(sv_hostname.string));
        json_object_set_new(jdata, "serverIP",    json_string(""));
        json_object_set_new(jdata, "gameVersion",    json_string(GAMEVERSION));
        json_object_set_new(jdata, "jsonGameStatVersion",    json_string(JSONGAMESTATVERSION));
        //json_object_set_new(jdata, "g_customConfig",    json_string(va("%s",g_customConfig)));  // use level.config hash
        json_object_set_new(jdata, "g_gametype",    json_string(va("%i",g_gametype.integer)));
        json_object_set_new(jdata, "date",    json_string(va("%02d:%02d:%02d (%02d /%d /%d)", ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday, ct.tm_mon, 1900+ct.tm_year )));
        json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
        json_object_set_new(jdata, "map",    json_string(mapName));
        json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));
        json_object_set_new(jdata, "round",    json_string(va("%i",g_currentRound.integer)));

        s = json_dumps( jdata, 1 );

        trap_FS_Write( "{\n \"serverinfo\": \n", strlen( "{\n \"serverinfo\": \n" ), level.gameStatslogFile );
        trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
        trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );
        json_decref(jdata);

        free(s);


        G_writeGameLogStart();  // write necessary json info for gamelog array....will provide better solution later


}


/*
===========
writeGameInfo

Output end of info (i.e. round, winner, etc)
===========
*/

void G_writeGameInfo (int winner ){
	//int s;
	char* s;
	char mapName[64];
    char newGamestatFile[MAX_QPATH];
    char *buf;
    char *buf2;
    char *buf3;
    char cs[MAX_STRING_CHARS];

    FILE		*gsfile;
    time_t unixTime = time(NULL);  // come back and make globally available
    trap_Cvar_VariableStringBuffer( "mapname", mapName, sizeof(mapName) );

	qtime_t ct;
	trap_RealTime(&ct);

    trap_GetConfigstring(CS_ROUNDINFO, cs, sizeof(cs));  // retrieve round/match info saved

    json_t *jdata = json_object();
        buf = Info_ValueForKey(cs, "matchid");
        json_object_set_new(jdata, "match_id",    json_string(va("%s",buf)));
        buf3 = Info_ValueForKey(cs, "round");
        json_object_set_new(jdata, "round",    json_string(buf3));
        buf2 = Info_ValueForKey(cs, "roundStart");
        json_object_set_new(jdata, "round_start",    json_string(buf2));
        json_object_set_new(jdata, "round_end",    json_string(va("%ld", unixTime)));
       // json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));
        json_object_set_new(jdata, "winner",    json_string(va("%s", (winner == 0) ? "Axis" : "Allied")));
        s = json_dumps( jdata, 1 );
        trap_FS_Write( "\"gameinfo\": \n", strlen( "\"gameinfo\": \n" ), level.gameStatslogFile );
        trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
        trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );
        json_decref(jdata);
        free(s);


}

// Probably should have just made an array of all events to loop over.....bah will do that when more events are added

void G_writeObjectiveEvent (gentity_t* agent,int objType){
    char* s;
    json_t *jdata = json_object();
    time_t unixTime = time(NULL);  // come back and make globally available
    if (!g_gameStatslog.integer) {
        return;
    }


    json_t *eventStats =  json_array();
    json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
    if (objType == objReturned) {
        json_object_set_new(jdata, "event",    json_string("ObjReturned"));
    }
    else if (objType == objTaken) {
        json_object_set_new(jdata, "event",    json_string("ObjTaken"));
    }
    else if (objType == objCapture) {
        json_object_set_new(jdata, "event",    json_string("ObjCapture"));
    }
    else if (objType == objDynDefuse) {
        json_object_set_new(jdata, "event",    json_string("ObjDynDefused"));
    }
    else if (objType == objDynPlant) {
        json_object_set_new(jdata, "event",    json_string("ObjDynPlanted"));
    }
    else if (objType == objSpawnFlag) {
        json_object_set_new(jdata, "event",    json_string("ObjSpawnFlagCaptured"));
    }

    // json_object_set_new(jdata, "team",    json_string(team));
    json_object_set_new(jdata, "agent",    json_string(agent->client->sess.guid));

    if (level.gameStatslogFile) {
         s = json_dumps( jdata, 0 );
         trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
         trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );

    }
         json_decref(jdata);
         free(s);

    level.eventNum++;
}



void G_writeGeneralEvent (gentity_t* agent,gentity_t* other, char* weapon, int eventType){
    char* s;
    json_t *jdata = json_object();
    json_t *event = json_object();
     time_t unixTime = time(NULL);  // come back and make globally available
       // json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
       json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
       if (eventType == eventSuicide) {
            json_object_set_new(jdata, "event",    json_string("suicide"));
            json_object_set_new(jdata, "player",    json_string(va("%s",agent->client->sess.guid)));
       }
       else if (eventType == eventKill) {
            json_object_set_new(jdata, "event",    json_string("kill"));
            json_object_set_new(jdata, "killer",    json_string(va("%s",agent->client->sess.guid)));
            json_object_set_new(jdata, "victim",    json_string(va("%s",other->client->sess.guid)));
            json_object_set_new(jdata, "weapon",    json_string(weapon));
            json_object_set_new(jdata, "killer_health",    json_integer(agent->health));
        }
       else if (eventType == eventTeamkill) {
            json_object_set_new(jdata, "event",    json_string("teamkill"));
            json_object_set_new(jdata, "killer",    json_string(va("%s",agent->client->sess.guid)));
            json_object_set_new(jdata, "victim",    json_string(va("%s",other->client->sess.guid)));
            json_object_set_new(jdata, "weapon",    json_string(weapon));
            json_object_set_new(jdata, "killer_health",    json_integer(agent->health));
        }
        else if (eventType == eventRevive) {
            json_object_set_new(jdata, "event",   json_string("revive"));
            json_object_set_new(jdata, "revived",  json_string(va("%s",agent->client->sess.guid)));
            json_object_set_new(jdata, "reviver",  json_string(va("%s",other->client->sess.guid)));
        }
       // json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));

        if (level.gameStatslogFile) {
                s = json_dumps( jdata, 0 );
                trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
                trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );
                json_decref(jdata);
                free(s);
        }
    level.eventNum++;
}




void G_writeDisconnectEvent (char* player){
    char* s;
    json_t *jdata = json_object();
    json_t *event = json_object();
     time_t unixTime = time(NULL);  // come back and make globally available
       // json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
        json_object_set_new(jdata, "event",    json_string("disconnect"));
        json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
        json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));
        json_object_set_new(jdata, "player",    json_string(player));
        if (level.gameStatslogFile) {
                s = json_dumps( jdata, 0 );
                trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
                trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );
                json_decref(jdata);
                free(s);
        }
    level.eventNum++;
}


// the following 3 functions are silly but for the time being necessary to make the output a true 'json'

void G_writeClosingJson(void)
{
    trap_FS_Write( "}\n", strlen( "}\n"), level.gameStatslogFile );
}


void G_writeGameLogStart(void)
{
    char* s;
    json_t *jdata = json_object();
    time_t unixTime = time(NULL);  // come back and make globally available
    if (level.gameStatslogFile) {
        trap_FS_Write( "\"gamelog\": [\n", strlen( "\"gamelog\": [\n"), level.gameStatslogFile );


        //json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
        json_object_set_new(jdata, "event",    json_string("round_start"));
        json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));
        json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));

                s = json_dumps( jdata, 0 );
                trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
                trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );
                json_decref(jdata);

                free(s);
    }


}

void G_writeGameLogEnd(char* endofroundinfo)
{
    char* s;
    json_t *jdata = json_object();
    json_t *event = json_object();
     time_t unixTime = time(NULL);  // come back and make globally available
        json_t *eventStats =  json_array();
       // json_object_set_new(jdata, "timestamp",    json_string(" "));
        //json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
        json_object_set_new(jdata, "event",    json_string("round_end"));
        json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));
        json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
        json_object_set_new(jdata, "result",    json_string(endofroundinfo));
        if (level.gameStatslogFile) {
                s = json_dumps( jdata, 0 );
                trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
                trap_FS_Write( "\n", strlen( "\n" ), level.gameStatslogFile );
                json_decref(jdata);
                free(s);
        }

        trap_FS_Write( "],\n", strlen( "],\n" ), level.gameStatslogFile );
}
