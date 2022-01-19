/*
===========================================================================

OpenWolf GPL Source Code
Copyright (C) 2009 SlackerLinux85 <SlackerLinux85@gmail.com>
Copyright (C) 2011 Dusan Jocic <dusanjocic@msn.com>

OpenWolf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

OpenWolf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/
#ifdef MYSQLDEP
#include "database.h"

// Dushan
#if defined(WIN32) || defined(WIN64)
#    include <winsock.h>
#    include <mysql.h>
#else
#    include <mysql/mysql.h>
#endif

extern cvar_t* db_enable;

#define MAX_QUERYS_RESULTS 100

MYSQL* connection, * connection2;

typedef struct {
	MYSQL_RES* results;
	MYSQL_ROW row;
} db_MySQL_querylist_t;

db_MySQL_querylist_t querylist[MAX_QUERYS_RESULTS];

int OW_MySQL_GetFreeQueryID(void) {
	int i;

	for (i = 0; i < MAX_QUERYS_RESULTS; i++) {
		if (!querylist[i].results) {
			return i;
		}
	}
	return -1;
}

qboolean OW_MySQL_Init(dbinterface_t* dbi) {
	dbi->DBConnectMaster = OW_MySQL_ConnectMaster;
	dbi->DBConnectSlave = OW_MySQL_ConnectSlave;
	dbi->DBStatus = OW_MySQL_DBStatus;
	dbi->DBDisconnect = OW_MySQL_Disconnect;

	dbi->DBCreateTable = OW_MySQL_CreateTable;

	dbi->RunQuery = OW_MySQL_RunQuery;
	dbi->FinishQuery = OW_MySQL_FinishQuery;

	dbi->NextRow = OW_MySQL_NextRow;
	dbi->RowCount = OW_MySQL_RowCount;

	dbi->GetFieldByID = OW_MySQL_GetFieldByID;
	dbi->GetFieldByName = OW_MySQL_GetFieldByName;
	dbi->GetFieldByID_int = OW_MySQL_GetFieldByID_int;
	dbi->GetFieldByName_int = OW_MySQL_GetFieldByName_int;
	dbi->FieldCount = OW_MySQL_FieldCount;

	dbi->CleanString = OW_MySQL_CleanString;

	return qtrue;
}

//
// MYSQL Connecting related functions
//
void OW_MySQL_ConnectMaster(void) {
	//init mysql
	connection = mysql_init(connection);

	//attempt to connect to mysql
	if (!mysql_real_connect(
		connection,
		db_addressMaster->string,
		db_usernameMaster->string,
		db_passwordMaster->string,
		db_databaseMaster->string,
		0,
		NULL,
		0)) {
		Com_Printf("WARNING: MySQL loading failed: %s.\n", mysql_error(connection));
		return;
	}

	// Dushan - moved creating databases after connecting to MySQL
	OW_MySQL_CreateTable();

	Com_Printf("MySQL Master Server Loaded.\n");
}

void OW_MySQL_ConnectSlave(void) {

	//init mysql
	connection2 = mysql_init(connection2);

	//attempt to connect to mysql
	if (!mysql_real_connect(
		connection2,
		db_addressSlave->string,
		db_usernameSlave->string,
		db_passwordSlave->string,
		db_databaseSlave->string,
		0,
		NULL,
		0)) {
		Com_Printf("WARNING: MySQL loading failed: %s.\n", mysql_error(connection2));
		return;

	}

	// Dushan - moved creating databases after connecting to MySQL
	OW_MySQL_CreateTable();

	Com_Printf("MySQL Slave Server Loaded.\n");
}

void OW_MySQL_DBStatus(void) {
}

void OW_MySQL_Disconnect(void) {
	int i;

	//clear all results
	for (i = 0; i < MAX_QUERYS_RESULTS; i++) {
		if (querylist[i].results) {
			mysql_free_result(querylist[i].results);
			querylist[i].results = NULL;
			Com_DPrintf("DEV: MySQL Freeing query ID %i.\n", i);
		}
	}

	if (connection) {
		mysql_close(connection);
		connection = NULL;
		Com_Printf("MySQL Master Server Unloaded.\n");
	}

	if (connection2) {
		mysql_close(connection2);
		connection2 = NULL;
		Com_Printf("MySQL Slave Server Unloaded.\n");
	}
}

//
// MYSQL Query related functions
//
int OW_MySQL_RunQuery(const char* query) {
	int queryid;

	queryid = OW_MySQL_GetFreeQueryID();

	if (queryid >= 0) {
		// Dushan - RunQuery should work on both MySQL connections
		if (connection && connection2) {
			if (mysql_query(connection, query)) {
				// Dushan - print info if query fail on MySQL connection (master server)
				Com_Printf("WARNING: MySQL Query failed: %s\n", mysql_error(connection));
				return -1;
			}
			if (mysql_query(connection2, query)) {
				// Dushan - print info if query fail on MySQL connection2 (slave server)
				Com_Printf("WARNING: MySQL Query failed: %s\n", mysql_error(connection2));
				return -1;
			}

			// Dushan - query both MySQL connections
			querylist[queryid].results = mysql_store_result(connection);
			querylist[queryid].results = mysql_store_result(connection2);
			Com_DPrintf("DEV: MySQL using query ID %i.\n", queryid);
			return queryid;
		}
	}
	else {
		Com_DPrintf("DEV: MySQL Failed to obtain a query ID.\n");
		return -1;
	}
	return -1;
}

void OW_MySQL_FinishQuery(int queryid) {
	if (querylist[queryid].results) {
		mysql_free_result(querylist[queryid].results);
		querylist[queryid].results = NULL;
		Com_DPrintf("DEV: MySQL Freeing query ID %i.\n", queryid);
	}
}

//
// MYSQL ROW related functions
//
qboolean OW_MySQL_NextRow(int queryid) {
	if (querylist[queryid].results) {
		//get next row
		querylist[queryid].row = mysql_fetch_row(querylist[queryid].results);
		//if its not valid return false
		if (!querylist[queryid].row) {
			return qfalse;
		}
		return qtrue;
	}
	return qfalse;
}

int OW_MySQL_RowCount(int queryid) {
	if (querylist[queryid].results)
		return mysql_num_rows(querylist[queryid].results);
	return 0;
}

//
// MYSQL Field related functions
//
void OW_MySQL_GetFieldByID(int queryid, int fieldid, char* buffer, int len) {
	if (querylist[queryid].row[fieldid]) {
		Q_strncpyz(buffer, querylist[queryid].row[fieldid], len);
	}
	else {
		Com_Printf("WARNING: MySQL field %i doesnt exist.\n", fieldid);
	}
}

void OW_MySQL_GetFieldByName(int queryid, const char* name, char* buffer, int len) {
	MYSQL_FIELD* fields;
	int num_fields;
	int i;

	if (querylist[queryid].results) {
		num_fields = mysql_num_fields(querylist[queryid].results);
		fields = mysql_fetch_fields(querylist[queryid].results);

		//loop through till we find the field
		for (i = 0; i < num_fields; i++) {
			if (!strcmp(fields[i].name, name)) {
				//found check for valid data
				if (querylist[queryid].row[i]) {
					Q_strncpyz(buffer, querylist[queryid].row[i], len);
					return;
				}
			}
		}
		Com_Printf("WARNING: MySQL field %s doesnt exist.\n", name);
	}
}

int OW_MySQL_GetFieldByID_int(int queryid, int fieldid) {
	if (querylist[queryid].row[fieldid]) {
		return atoi(querylist[queryid].row[fieldid]);
	}
	else {
		Com_Printf("WARNING: MySQL field %i doesnt exist.\n", fieldid);
		return 0;
	}
	return 0;
}

int OW_MySQL_GetFieldByName_int(int queryid, const char* name) {
	MYSQL_FIELD* fields;
	int num_fields;
	int i;

	if (querylist[queryid].results) {
		num_fields = mysql_num_fields(querylist[queryid].results);
		fields = mysql_fetch_fields(querylist[queryid].results);

		//loop through till we find the field
		for (i = 0; i < num_fields; i++) {
			if (!strcmp(fields[i].name, name)) {
				//found check for valid data
				if (querylist[queryid].row[i]) {
					return atoi(querylist[queryid].row[i]);
				}
			}
		}
		Com_Printf("WARNING: MySQL field %s doesnt exist.\n", name);
	}
	return 0;
}

int OW_MySQL_FieldCount(int queryid) {
	if (querylist[queryid].results) {
		return mysql_num_fields(querylist[queryid].results);
	}
	return 0;
}

//
// MYSQL Misc functions
//
void OW_MySQL_CleanString(const char* in, char* out, int len) {
	if (connection && len > 0 && in[0]) {
		mysql_real_escape_string(connection, out, in, len);
	}
	if (connection2 && len > 0 && in[0]) {
		mysql_real_escape_string(connection2, out, in, len);
	}
}

//
// MYSQL Create database
//
void OW_MySQL_CreateTable(void) {

	// Dushan : Write and created only info if database is connected
	if (db_enable->integer == 1) {

		// If it is connected to database
		if (connection) {

			//
			// Userinfo table structure
			// Player relevate structure
			//

			// create table user_info and fill table userinfo with some values
			if (mysql_query(connection, "CREATE TABLE IF NOT EXISTS user_info (\
									user_id mediumint(8) unsigned NOT NULL auto_increment, \
									clan_id mediumint(8) unsigned NOT NULL default '3', \
									user_permissions mediumtext collate utf8_bin NOT NULL, \
									user_perm_from mediumint(8) unsigned NOT NULL default '0', \
									user_ip varchar(40) collate utf8_bin NOT NULL default '', \
									user_guid varchar(33) collate utf8_bin NOT NULL default '', \
									user_regdate int(11) unsigned NOT NULL default '0', \
									username varchar(255) collate utf8_bin NOT NULL default '', \
									username_clean varchar(255) collate utf8_bin NOT NULL default '', \
									user_password varchar(40) collate utf8_bin NOT NULL default '', \
									user_passchg int(11) unsigned NOT NULL default '0', \
									user_pass_convert tinyint(1) unsigned NOT NULL default '0', \
									user_newpasswd varchar(40) collate utf8_bin NOT NULL default '', \
									user_email varchar(100) collate utf8_bin NOT NULL default '', \
									user_email_hash bigint(20) NOT NULL default '0', \
									user_birthday varchar(10) collate utf8_bin NOT NULL default '', \
									user_lastvisit int(11) unsigned NOT NULL default '0', \
									user_warnings tinyint(4) NOT NULL default '0', \
									user_last_warning int(11) unsigned NOT NULL default '0', \
									user_login_attempts tinyint(4) NOT NULL default '0', \
									user_inactive_reason tinyint(2) NOT NULL default '0', \
									user_inactive_time int(11) unsigned NOT NULL default '0', \
									user_timezone decimal(5,2) NOT NULL default '0.00', \
									user_dst tinyint(1) unsigned NOT NULL default '0', \
									user_dateformat varchar(30) collate utf8_bin NOT NULL default 'd M Y H:i', \
									user_rank mediumint(8) unsigned NOT NULL default '0', \
									user_new_privmsg int(4) NOT NULL default '0', \
									user_unread_privmsg int(4) NOT NULL default '0', \
									user_last_privmsg int(11) unsigned NOT NULL default '0', \
									user_message_rules tinyint(1) unsigned NOT NULL default '0', \
									user_full_folder int(11) NOT NULL default '-3', \
									user_emailtime int(11) unsigned NOT NULL default '0', \
									user_avatar varchar(255) collate utf8_bin NOT NULL default '', \
									user_avatar_type tinyint(2) NOT NULL default '0', \
									user_avatar_width smallint(4) unsigned NOT NULL default '0', \
									user_avatar_height smallint(4) unsigned NOT NULL default '0', \
									user_from varchar(100) collate utf8_bin NOT NULL default '', \
									user_yim varchar(255) collate utf8_bin NOT NULL default '', \
									user_msnm varchar(255) collate utf8_bin NOT NULL default '', \
									user_website varchar(200) collate utf8_bin NOT NULL default '', \
									user_interests text collate utf8_bin NOT NULL, \
									PRIMARY KEY  (user_id), \
									UNIQUE KEY username_clean (username_clean), \
									KEY user_birthday (user_birthday), \
									KEY user_email_hash (user_email_hash))")) {
				Com_Printf("WARNING: MySQL Query failed: %s\n", mysql_error(connection));
				return;
			}

			//
			// Ban table structure
			//

			if (mysql_query(connection, "CREATE TABLE IF NOT EXISTS user_banlist (\
									ban_id mediumint(8) unsigned NOT NULL auto_increment, \
									ban_userid mediumint(8) unsigned NOT NULL default '0', \
									ban_ip varchar(40) collate utf8_bin NOT NULL default '', \
									ban_guid varchar(33) collate utf8_bin NOT NULL default '', \
									ban_email varchar(100) collate utf8_bin NOT NULL default '', \
									ban_start int(11) unsigned NOT NULL default '0', \
									ban_end int(11) unsigned NOT NULL default '0', \
									ban_exclude tinyint(1) unsigned NOT NULL default '0', \
									ban_reason varchar(255) collate utf8_bin NOT NULL default '', \
									ban_give_reason varchar(255) collate utf8_bin NOT NULL default '', \
									PRIMARY KEY  (ban_id), \
									KEY ban_end (ban_end), \
									KEY ban_user (ban_userid,ban_exclude), \
									KEY ban_email (ban_email,ban_exclude), \
									KEY ban_ip (ban_ip,ban_exclude))")) {
				Com_Printf("WARNING: MySQL Query failed: %s\n", mysql_error(connection));
				return;
			}

			//
			// Mute table structure
			// This is copy from ban table
			//

			if (mysql_query(connection, "CREATE TABLE IF NOT EXISTS user_mutelist (\
									mute_id mediumint(8) unsigned NOT NULL auto_increment, \
									mute_userid mediumint(8) unsigned NOT NULL default '0', \
									mute_ip varchar(40) collate utf8_bin NOT NULL default '', \
									mute_guid varchar(33) collate utf8_bin NOT NULL default '', \
									mute_start int(11) unsigned NOT NULL default '0', \
									mute_end int(11) unsigned NOT NULL default '0', \
									mute_exclude tinyint(1) unsigned NOT NULL default '0', \
									mute_reason varchar(255) collate utf8_bin NOT NULL default '', \
									PRIMARY KEY  (mute_id), \
									KEY mute_end (mute_end), \
									KEY mute_user (mute_userid,mute_exclude), \
									KEY mute_ip (mute_ip,mute_exclude))")) {
				Com_Printf("WARNING: MySQL Query failed: %s\n", mysql_error(connection));
				return;
			}


			// ========================================================================== //

			//
			// Create server structure
			// Server/Mod relevate structure
			//

			// create table players
			if (mysql_query(connection, "CREATE TABLE `user_players` ( \
									`user_guid` char(36) COLLATE utf8_bin NOT NULL, \
									`user_permissions` mediumtext COLLATE utf8_bin, \
									`user_ip4` varchar(15) COLLATE utf8_bin DEFAULT NULL, \
									`user_ip6` varchar(39) COLLATE utf8_bin DEFAULT NULL, \
									`user_xp` int(11) DEFAULT '0', \
									`user_skill` int(11) DEFAULT '0', \
									`username` varchar(50) COLLATE utf8_bin DEFAULT NULL, \
									`clanname` varchar(50) COLLATE utf8_bin DEFAULT NULL, \
									`user_password` varchar(50) COLLATE utf8_bin DEFAULT NULL, \
									`user_email` varchar(255) COLLATE utf8_bin DEFAULT NULL, \
									PRIMARY KEY (`user_guid`))")) {
				Com_Printf("WARNING: MySQL Query failed: %s\n", mysql_error(connection));
				return;
			}


			// ========================================================================== //

			//
			// Player Stats structure
			//

			// NOTE : used information from VSP Stats

			// create table player_stats
			if (mysql_query(connection, "CREATE TABLE IF NOT EXISTS player_stats (\
									playerID varchar(100) BINARY NOT NULL default '', \
									playerName varchar(255) NOT NULL default '', \
									skill int(10) unsigned default '0', \
									kills int(11) default '0', \
									deaths int(11) default '0', \
									kill_streak int(11) default '0', \
									death_streak int(11) default '0', \
									games int(10) unsigned default '0', \
									PRIMARY KEY  (`playerID`))")) {
				Com_Printf("WARNING: MySQL Query failed: %s\n", mysql_error(connection));
				return;
			}

		}

		Com_Printf("-----------------------------------\n");

	}
}
#endif
