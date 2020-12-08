#include "g_local.h"
#include "g_stats.h"
#include <jansson.h>
//#include "../qcommon/jansson/jansson.h"
#include <time.h>
// send/receive to json server: only need to include a few additional functions
//   although tested and works.....not necessary until we
//#define URL_FORMAT   "https://192.168.1.2:3000/gameStats"
//#define URL_SIZE     256

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


        json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
        json_object_set_new(jdata, "event",    json_string("SOR"));
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
        json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
        json_object_set_new(jdata, "event",    json_string("EOR"));
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

void G_stats2JSON(int winner ) {

    int i, j, eff,rc;
	float tot_acc = 0.00f;
	char* s;
	gclient_t *cl;
	char mapName[64];
	char n1[MAX_NETNAME];
	char n2[MAX_NETNAME];
    unsigned int m, dwWeaponMask = 0;
	char strWeapInfo[MAX_STRING_CHARS] = { 0 };
	time_t unixTime = time(NULL);
    json_t* jdata;
    json_t* weapOb;
    json_t *root = json_object();
    json_t *playersArray =  json_array();
    json_t* weapArray;

    trap_Cvar_VariableStringBuffer( "mapname", mapName, sizeof(mapName) );

	qtime_t ct;
	trap_RealTime(&ct);

	for ( i = TEAM_RED; i <= TEAM_BLUE; i++ ) {
		if ( !TeamCount( -1, i ) ) {
			continue;
		}

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
            jdata = json_object();
            json_object_set_new(jdata, "GUID", json_string(cl->sess.guid));
            json_object_set_new(jdata, "alias", json_string(n2));
            json_object_set_new(jdata, "team", json_string((i == TEAM_RED) ? "Axis" : "Allied"  ));
            json_object_set_new(jdata, "start_time", json_integer(cl->sess.start_time));
            json_object_set_new(jdata, "end_time", json_integer(cl->sess.end_time));
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
            json_array_append(playersArray, jdata);
            json_decref(weapArray);
            json_decref(jdata);

        }


    }

         s = json_dumps( playersArray, 1 ); // for a pretty print form
/*
 previously had it save this in a separate file (not within the event file)
    there are benefits to doing such as this by itself preserves the 'json' structure and can be easily read into memory and updated then written out again
    we may want to do two separate saves: 1 within the events log and the other within a gamestats log or something
further note: we will ultimately want the 'timestamps' and 'gameID' match those of the 'root' object's .... easily can do this once we figure out the fine details
    of how we want this all implemented....
*/
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

void G_writeGameInfo (void){
	//int s;
	char* s;
	char mapName[64];
    char newGamestatFile[MAX_QPATH];
    FILE		*gsfile;
    time_t unixTime = time(NULL);  // come back and make globally available
    trap_Cvar_VariableStringBuffer( "mapname", mapName, sizeof(mapName) );

	qtime_t ct;
	trap_RealTime(&ct);

    json_t *jdata = json_object();
   // json_t *jinfo = json_object();
        json_object_set_new(jdata, "serverName",    json_string(sv_hostname.string));
        json_object_set_new(jdata, "serverIP",    json_string(""));
        json_object_set_new(jdata, "gameVersion",    json_string(GAMEVERSION));
        json_object_set_new(jdata, "jsonGameStatVersion",    json_string(JSONGAMESTATVERSION));
        //json_object_set_new(jdata, "g_customConfig",    json_string(va("%s",g_customConfig)));
        json_object_set_new(jdata, "g_gametype",    json_string(va("%i",g_gametype.integer)));
        json_object_set_new(jdata, "date",    json_string(va("%02d:%02d:%02d (%02d /%d /%d)", ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday, ct.tm_mon, 1900+ct.tm_year )));
        json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
        json_object_set_new(jdata, "map",    json_string(mapName));
        json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));
        json_object_set_new(jdata, "round",    json_string(va("%i",g_currentRound.integer)));
        s = json_dumps( jdata, 1 );
        //s = json_dumps( jinfo, 1 );
        trap_FS_Write( "{\n \"gameinfo\": \n", strlen( "{\n \"gameinfo\": \n" ), level.gameStatslogFile );
        trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
        trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );
        json_decref(jdata);
       // json_decref(jinfo);
        free(s);


        G_writeGameLogStart();  // write necessary json info for gamelog array


}
/*
Plan to combine all writing of events into a single writeEvent function....should have thought
ahead a bit more before starting...
*/

//void G_writeKillEvent (char* killer, char* victim, char* weapon){
void G_writeKillEvent (char* killer, char* victim, char* weapon, int killerhealth){
    int eventtype =0;  // plan to condense everything into one event function
    char* s;
    json_t *jdata = json_object();
    time_t unixTime = time(NULL);  // come back and make globally available
    if (eventtype == 0) {
      //  json_t *eventStats =  json_array();
       // json_object_set_new(jdata, "timestamp",    json_string(" "));
        json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
        json_object_set_new(jdata, "event",    json_string("kill"));
        json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
        json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));
        json_object_set_new(jdata, "killer",    json_string(killer));
        json_object_set_new(jdata, "victim",    json_string(victim));
        json_object_set_new(jdata, "weapon",    json_string(weapon));
        json_object_set_new(jdata, "khealth",    json_integer(killerhealth));

       //json_array_append(eventStats, jdata);
      //  json_t *event = json_object();
       // json_object_set_new(event,"event", json_string("kill"));
      //  json_object_set_new(event,"stats", eventStats);
        if (level.gameStatslogFile) {
               //  s = json_dumps( event, 0 );
                 s = json_dumps( jdata, 0 );
                trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
                trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );
                json_decref(jdata);
              //  json_decref(eventStats);
              //  json_decref(event);
                free(s);
        }

    }
    level.eventNum++;

}

void G_writeTeamKillEvent (char* killer, char* victim){
    char* s;
    json_t *jdata = json_object();
    json_t *event = json_object();
     time_t unixTime = time(NULL);  // come back and make globally available
        json_t *eventStats =  json_array();
       // json_object_set_new(jdata, "timestamp",    json_string(" "));
        json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
        json_object_set_new(jdata, "event",    json_string("teamkill"));
        json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));
        json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
        json_object_set_new(jdata, "killer",    json_string(killer));
        json_object_set_new(jdata, "victim",    json_string(victim));

        if (level.gameStatslogFile) {
              //   s = json_dumps( event, 0 );
                s = json_dumps( jdata, 0 );
                trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
                trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );
                json_decref(jdata);
                free(s);
        }


        level.eventNum++;

}

void G_writeSuicideEvent (char* player){
    char* s;
    json_t *jdata = json_object();
    json_t *event = json_object();
     time_t unixTime = time(NULL);  // come back and make globally available
        json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
        json_object_set_new(jdata, "event",    json_string("suicide"));
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


void G_writeReviveEvent (char* revived, char* medic){
    char* s;
    json_t *jdata = json_object();
    json_t *event = json_object();
     time_t unixTime = time(NULL);  // come back and make globally available
        json_t *eventStats =  json_array();
        json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
        json_object_set_new(jdata, "event",    json_string("revive"));
        json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
        json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));
        json_object_set_new(jdata, "revived",    json_string(revived));
        json_object_set_new(jdata, "reviver",    json_string(medic));
        if (level.gameStatslogFile) {
              //   s = json_dumps( event, 0 );
                s = json_dumps( jdata, 0 );
                trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
                trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );
                json_decref(jdata);
                free(s);
        }
        level.eventNum++;
}

void G_writeObjectiveEvent (char* team, char* objective, char* player){
    int eventtype =0;  // plan to condense everything into one event function
    char* s;
    json_t *jdata = json_object();
    time_t unixTime = time(NULL);  // come back and make globally available
    if (!g_gameStatslog.integer) {
        return;
    }
    if (eventtype == 0) {
        json_t *eventStats =  json_array();
      //  json_object_set_new(jdata, "timestamp",    json_string(" "));
        json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
        json_object_set_new(jdata, "event",    json_string(objective));
        json_object_set_new(jdata, "unixtime",    json_string(va("%ld", unixTime)));
        json_object_set_new(jdata, "levelTime",    json_string(GetLevelTime()));
        json_object_set_new(jdata, "team",    json_string(team));
        json_object_set_new(jdata, "player",    json_string(player));
        if (level.gameStatslogFile) {
                 //s = json_dumps( event, 0 );
                s = json_dumps( jdata, 0 );
                trap_FS_Write( s, strlen( s ), level.gameStatslogFile );
                trap_FS_Write( ",\n", strlen( ",\n" ), level.gameStatslogFile );
                json_decref(jdata);
                free(s);
        }

    }
    level.eventNum++;
}

void G_writeDisconnectEvent (char* player){
    char* s;
    json_t *jdata = json_object();
    json_t *event = json_object();
     time_t unixTime = time(NULL);  // come back and make globally available
        json_object_set_new(jdata, "event_order",    json_integer(level.eventNum));
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
