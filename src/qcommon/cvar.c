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

// cvar.c -- dynamic variable tracking

#include "../game/q_shared.h"
#include "qcommon.h"

cvar_t      *cvar_vars;
cvar_t      *cvar_cheats;
int cvar_modifiedFlags;

cvar_rest_t* cvar_rest_vars;

cvar_t cvar_indexes[MAX_CVARS];
int cvar_numIndexes;

cvar_rest_t cvar_rest_indexes[MAX_CVARS];
int cvar_rest_numIndexes;

#define FILE_HASH_SIZE      256
static cvar_t*     hashTable[FILE_HASH_SIZE];
static cvar_rest_t* restHashTable[FILE_HASH_SIZE];

cvar_t *Cvar_Set2( const char *var_name, const char *value, qboolean force );

/*
================
return a hash value for the filename
================
*/
static long generateHashValue( const char *fname ) {
	int i;
	long hash;
	char letter;

	if ( !fname ) {
		Com_Error( ERR_DROP, "null name in generateHashValue" );       //gjd
	}
	hash = 0;
	i = 0;
	while ( fname[i] != '\0' ) {
		letter = tolower( fname[i] );
		hash += (long)( letter ) * ( i + 119 );
		i++;
	}
	hash &= ( FILE_HASH_SIZE - 1 );
	return hash;
}

/*
============
Cvar_ValidateString
============
*/
static qboolean Cvar_ValidateString( const char *s ) {
	if ( !s ) {
		return qfalse;
	}
	if ( strchr( s, '\\' ) ) {
		return qfalse;
	}
	if ( strchr( s, '\"' ) ) {
		return qfalse;
	}
	if ( strchr( s, ';' ) ) {
		return qfalse;
	}
	return qtrue;
}

/*
============
Cvar_Rest_ValidateString
============
*/
static qboolean Cvar_Rest_ValidateString(const char* s) {
	if (!s) {
		return qfalse;
	}
	if (strchr(s, '//')) {
		return qfalse;
	}
	return Cvar_ValidateString(s);
}

/*
============
Cvar_FindVar
============
*/
cvar_t *Cvar_FindVar( const char *var_name ) {
	cvar_t  *var;
	long hash;

	hash = generateHashValue( var_name );

	for ( var = hashTable[hash] ; var ; var = var->hashNext ) {
		if ( !Q_stricmp( var_name, var->name ) ) {
			return var;
		}
	}

	return NULL;
}

/*
============
CvarRest_FindVar
============
*/
cvar_rest_t* Cvar_Rest_FindVar(const char* var_name) {
	cvar_rest_t* var;
	long hash;

	hash = generateHashValue(var_name);

	for (var = restHashTable[hash]; var; var = var->hashNext) {
		if (!Q_stricmp(var_name, var->name)) {
			return var;
		}
	}

	return NULL;
}

/*
============
Cvar_VariableValue
============
*/
float Cvar_VariableValue( const char *var_name ) {
	cvar_t  *var;

	var = Cvar_FindVar( var_name );
	if ( !var ) {
		return 0;
	}
	return var->value;
}


/*
============
Cvar_VariableIntegerValue
============
*/
int Cvar_VariableIntegerValue( const char *var_name ) {
	cvar_t  *var;

	var = Cvar_FindVar( var_name );
	if ( !var ) {
		return 0;
	}
	return var->integer;
}


/*
============
Cvar_VariableString
============
*/
char *Cvar_VariableString( const char *var_name ) {
	cvar_t *var;

	var = Cvar_FindVar( var_name );
	if ( !var ) {
		return "";
	}
	return var->string;
}


/*
============
Cvar_VariableStringBuffer
============
*/
void Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	cvar_t *var;

	var = Cvar_FindVar( var_name );
	if ( !var ) {
		*buffer = 0;
	} else {
		Q_strncpyz( buffer, var->string, bufsize );
	}
}


/*
============
Cvar_CommandCompletion
============
*/
void    Cvar_CommandCompletion( void ( *callback )(const char *s) ) {
	cvar_t      *cvar;

	for ( cvar = cvar_vars ; cvar ; cvar = cvar->next ) {
		callback( cvar->name );
	}
}

/*
============
Cvar_ClearForeignCharacters
some cvar values need to be safe from foreign characters
============
*/
char *Cvar_ClearForeignCharacters( const char *value ) {
	static char clean[MAX_CVAR_VALUE_STRING];
	int i,j;

	j = 0;
	for ( i = 0; value[i] != '\0'; i++ )
	{
		if ( !( value[i] & 128 ) ) {
			clean[j] = value[i];
			j++;
		}
	}
	clean[j] = '\0';

	return clean;
}

/*
============
Cvar_Get

If the variable already exists, the value will not be set unless CVAR_ROM
The flags will be or'ed in if the variable exists.
============
*/
cvar_t *Cvar_Get( const char *var_name, const char *var_value, int flags ) {
	cvar_t  *var;
	long hash;

	if ( !var_name || !var_value ) {
		Com_Error( ERR_FATAL, "Cvar_Get: NULL parameter" );
	}

	if ( !Cvar_ValidateString( var_name ) ) {
		Com_Printf( "invalid cvar name string: %s\n", var_name );
		var_name = "BADNAME";
	}

#if 0       // FIXME: values with backslash happen
	if ( !Cvar_ValidateString( var_value ) ) {
		Com_Printf( "invalid cvar value string: %s\n", var_value );
		var_value = "BADVALUE";
	}
#endif

	var = Cvar_FindVar( var_name );
	if ( var ) {
		// if the C code is now specifying a variable that the user already
		// set a value for, take the new value as the reset value
		if ( ( var->flags & CVAR_USER_CREATED ) && !( flags & CVAR_USER_CREATED )
			 && var_value[0] ) {
			var->flags &= ~CVAR_USER_CREATED;
			Z_Free( var->resetString );
			var->resetString = CopyString( var_value );

			// ZOID--needs to be set so that cvars the game sets as
			// SERVERINFO get sent to clients
			cvar_modifiedFlags |= flags;
		}

		var->flags |= flags;
		// only allow one non-empty reset string without a warning
		if ( !var->resetString[0] ) {
			// we don't have a reset string yet
			Z_Free( var->resetString );
			var->resetString = CopyString( var_value );
		} else if ( var_value[0] && strcmp( var->resetString, var_value ) ) {
			Com_DPrintf( "Warning: cvar \"%s\" given initial values: \"%s\" and \"%s\"\n",
						 var_name, var->resetString, var_value );
		}
		// if we have a latched string, take that value now
		if ( var->latchedString ) {
			char *s;

			s = var->latchedString;
			var->latchedString = NULL;  // otherwise cvar_set2 would free it
			Cvar_Set2( var_name, s, qtrue );
			Z_Free( s );
		}

		// TTimo
		// if CVAR_USERINFO was toggled on for an existing cvar, check wether the value needs to be cleaned from foreigh characters
		// (for instance, seta name "name-with-foreign-chars" in the config file, and toggle to CVAR_USERINFO happens later in CL_Init)
		if ( flags & CVAR_USERINFO ) {
			char *cleaned = Cvar_ClearForeignCharacters( var->string ); // NOTE: it is probably harmless to call Cvar_Set2 in all cases, but I don't want to risk it
			if ( strcmp( var->string, cleaned ) ) {
				Cvar_Set2( var->name, var->string, qfalse ); // call Cvar_Set2 with the value to be cleaned up for verbosity
			}
		}

// use a CVAR_SET for rom sets, get won't override
#if 0
		// CVAR_ROM always overrides
		if ( flags & CVAR_ROM ) {
			Cvar_Set2( var_name, var_value, qtrue );
		}
#endif
		return var;
	}

	//
	// allocate a new cvar
	//
	if ( cvar_numIndexes >= MAX_CVARS ) {
		Com_Error( ERR_FATAL, "MAX_CVARS" );
	}
	var = &cvar_indexes[cvar_numIndexes];
	cvar_numIndexes++;
	var->name = CopyString( var_name );
	var->string = CopyString( var_value );
	var->modified = qtrue;
	var->modificationCount = 1;
	var->value = atof( var->string );
	var->integer = atoi( var->string );
	var->resetString = CopyString( var_value );

	// link the variable in
	var->next = cvar_vars;
	cvar_vars = var;

	var->flags = flags;

	hash = generateHashValue( var_name );
	var->hashNext = hashTable[hash];
	hashTable[hash] = var;

	return var;
}

/*
============
Cvar_Rest_Get

If the variable already exists, the value will not be set 
============
*/
cvar_rest_t* Cvar_Rest_Get(const char* var_name, int type, const char* value1, const char* value2) {
	cvar_rest_t* var;
	long hash;

	if (!var_name) {
		Com_Error(ERR_FATAL, "Cvar_Get: NULL parameter");
	}

	if (!Cvar_Rest_ValidateString(var_name)) {
		Com_Printf("invalid cvar name string: %s\n", var_name);
		var_name = "BADNAME";
		return NULL;
	}

	var = Cvar_Rest_FindVar(var_name);
	if (var) {
		return var;
	}

	//
	// allocate a new cvar
	//
	if (cvar_rest_numIndexes >= MAX_CVARS) {
		Com_Error(ERR_FATAL, "MAX_REST_CVARS");
		return var;
	}

	var = &cvar_rest_indexes[cvar_rest_numIndexes];
	cvar_rest_numIndexes++;
	var->name = CopyString(var_name);
	var->type = type;
	var->sVal1 = CopyString(value1);
	var->sVal2 = (value2 == NULL ? NULL : CopyString(value2));
	var->fVal1 = atof(var->sVal1);
	var->fVal2 = (value2 == NULL ? 0.00 : atof(var->sVal2));
	var->iVal1 = atoi(var->sVal1);
	var->iVal2 = (value2 == NULL ? 0 : atoi(var->sVal2));

	// link the variable in
	var->next = cvar_rest_vars;
	cvar_rest_vars = var;

	hash = generateHashValue(var_name);
	var->hashNext = restHashTable[hash];
	restHashTable[hash] = var;

	return var;
}

/*
============
Cvar_Set2
============
*/
#define FOREIGN_MSG "Foreign characters are not allowed in userinfo variables.\n"
#ifndef DEDICATED
const char* CL_TranslateStringBuf( const char *string );
#endif
cvar_t *Cvar_Set2( const char *var_name, const char *value, qboolean force ) {
	cvar_t  *var;
#ifndef DEDICATED
	cvar_rest_t* rv;
#endif

	Com_DPrintf( "Cvar_Set2: %s %s\n", var_name, value );

	if ( !Cvar_ValidateString( var_name ) ) {
		Com_Printf( "invalid cvar name string: %s\n", var_name );
		var_name = "BADNAME";
	} 

#ifndef DEDICATED
	// Check if it's allowed
	if (rv = Cvar_Rest_FindVar(var_name)) {
		if (!Cvar_RestValueIsValid(rv, value)) {
			Com_Printf(va("Value %s not allowed on this server. ^n[%s]\n", value, Cvar_RestAcceptedValues(var_name)));
			return NULL;
		}
	}
#endif

	var = Cvar_FindVar( var_name );
	if ( !var ) {
		if ( !value ) {
			return NULL;
		}
		// create it
		if ( !force ) {
			return Cvar_Get( var_name, value, CVAR_USER_CREATED );
		} else {
			return Cvar_Get( var_name, value, 0 );
		}
	}

	if ( !value ) {
		value = var->resetString;
	}

	if ( var->flags & CVAR_USERINFO ) {
		char *cleaned = Cvar_ClearForeignCharacters( value );
		if ( strcmp( value, cleaned ) ) {
			#ifdef DEDICATED
			Com_Printf( FOREIGN_MSG );
			#else
			Com_Printf( CL_TranslateStringBuf( FOREIGN_MSG ) );
			#endif
			Com_Printf( "Using %s instead of %s\n", cleaned, value );
			return Cvar_Set2( var_name, cleaned, force );
		}
	}

	if ( !strcmp( value,var->string ) ) {
		return var;
	}
	// note what types of cvars have been modified (userinfo, archive, serverinfo, systeminfo)
	cvar_modifiedFlags |= var->flags;

	if ( !force ) {
		if ( var->flags & CVAR_ROM ) {
			Com_Printf( "%s is read only.\n", var_name );
			return var;
		}

		if ( var->flags & CVAR_INIT ) {
			Com_Printf( "%s is write protected.\n", var_name );
			return var;
		}

		if ( ( var->flags & CVAR_CHEAT ) && !cvar_cheats->integer ) {
			Com_Printf( "%s is cheat protected.\n", var_name );
			return var;
		}

		if ( var->flags & CVAR_LATCH ) {
			if ( var->latchedString ) {
				if ( strcmp( value, var->latchedString ) == 0 ) {
					return var;
				}
				Z_Free( var->latchedString );
			} else
			{
				if ( strcmp( value, var->string ) == 0 ) {
					return var;
				}
			}

			Com_Printf( "%s will be changed upon restarting.\n", var_name );
			var->latchedString = CopyString( value );
			var->modified = qtrue;
			var->modificationCount++;
			return var;
		}

	} else
	{
		if ( var->latchedString ) {
			Z_Free( var->latchedString );
			var->latchedString = NULL;
		}
	}

	if ( !strcmp( value, var->string ) ) {
		return var;     // not changed

	}
	var->modified = qtrue;
	var->modificationCount++;

	Z_Free( var->string );   // free the old value string

	var->string = CopyString( value );
	var->value = atof( var->string );
	var->integer = atoi( var->string );

	return var;
}

/*
============
Cvar_SetRestricted

Sets restricted cvar flags.
============
*/
cvar_rest_t* Cvar_SetRestricted(const char* var_name, unsigned int type, const char* value1, const char* value2) {
	cvar_rest_t* var;

	Com_DPrintf("Cvar_SetRestricted: %s %d %s %s\n", var_name, type, value1, value2);

	if (!Cvar_Rest_ValidateString(var_name)) {
		Com_Printf("invalid cvar name string: %s\n", var_name);
		return NULL;
	}

	var = Cvar_Rest_FindVar(var_name);
	if (!var) {
		return Cvar_Rest_Get(var_name, type, value1, value2);
	}

	if (!value1) {
		return var;
	}

	Z_Free(var->sVal1);
	Z_Free(var->sVal2);

	var->type = type;
	var->sVal1 = CopyString(value1);
	var->sVal2 = (value2 == NULL ? NULL : CopyString(value2));
	var->fVal1 = atof(var->sVal1);
	var->fVal2 = (value2 == NULL ? 0.00 : atof(var->sVal2));
	var->iVal1 = atoi(var->sVal1);
	var->iVal2 = (value2 == NULL ? 0 : atoi(var->sVal2));

	return var;
}

/*
============
Cvar_Set
============
*/
void Cvar_Set( const char *var_name, const char *value ) {
	Cvar_Set2( var_name, value, qtrue );
}

/*
============
Cvar_SetLatched
============
*/
void Cvar_SetLatched( const char *var_name, const char *value ) {
	Cvar_Set2( var_name, value, qfalse );
}

/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue( const char *var_name, float value ) {
	char val[32];

	if ( value == (int)value ) {
		Com_sprintf( val, sizeof( val ), "%i",(int)value );
	} else {
		Com_sprintf( val, sizeof( val ), "%f",value );
	}
	Cvar_Set( var_name, val );
}

/*
============
Cvar_Reset
============
*/
void Cvar_Reset( const char *var_name ) {
	Cvar_Set2( var_name, NULL, qfalse );
}

/*
============
Cvar_SetCheatState

Any testing variables will be reset to the safe values
============
*/
void Cvar_SetCheatState( void ) {
	cvar_t  *var;

	// set all default vars to the safe value
	for ( var = cvar_vars ; var ; var = var->next ) {
		if ( var->flags & CVAR_CHEAT ) {
			if ( strcmp( var->resetString,var->string ) ) {
				Cvar_Set( var->name, var->resetString );
			}
		}
	}
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean Cvar_Command( void ) {
	cvar_t*         v;
#ifndef DEDICATED
	cvar_rest_t*    rv;
#endif

	// check variables
	v = Cvar_FindVar( Cmd_Argv( 0 ) );
	if ( !v ) {
		return qfalse;
	}

	// perform a variable print or set
	if ( Cmd_Argc() == 1 ) {
		Com_Printf( "\"%s\" is:\"%s" S_COLOR_WHITE "\" default:\"%s" S_COLOR_WHITE "\"\n", v->name, v->string, v->resetString );
		if ( v->latchedString ) {
			Com_Printf( "latched: \"%s\"\n", v->latchedString );
		}
		return qtrue;
	}

#ifndef DEDICATED
	// Check if it's allowed
	if (rv = Cvar_Rest_FindVar(v->name)) {
		if (!Cvar_RestValueIsValid(rv, Cmd_Argv(1))) {
			Com_Printf(va("Value %s not allowed on this server. ^n[%s]\n", Cmd_Argv(1), Cvar_RestAcceptedValues(v->name)));
			return qtrue;
		}
	}
#endif

	// set the value if forcing isn't required
	Cvar_Set2( v->name, Cmd_Argv( 1 ), qfalse );
	return qtrue;
}

/*
============
Cvar_Toggle_f

Toggles a cvar for easy single key binding
============
*/
void Cvar_Toggle_f( void ) {
	int v;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "usage: toggle <variable>\n" );
		return;
	}

	v = Cvar_VariableValue( Cmd_Argv( 1 ) );
	v = !v;

	Cvar_Set2( Cmd_Argv( 1 ), va( "%i", v ), qfalse );
}

/*
============
Cvar_Set_f

Allows setting and defining of arbitrary cvars from console, even if they
weren't declared in C code.
============
*/
void Cvar_Set_f( void ) {
	int i, c, l, len;
	char combined[MAX_STRING_TOKENS];

	c = Cmd_Argc();
	if ( c < 3 ) {
		Com_Printf( "usage: set <variable> <value>\n" );
		return;
	}

	combined[0] = 0;
	l = 0;
	for ( i = 2 ; i < c ; i++ ) {
		len = strlen( Cmd_Argv( i ) + 1 );
		if ( l + len >= MAX_STRING_TOKENS - 2 ) {
			break;
		}
		strcat( combined, Cmd_Argv( i ) );
		if ( i != c - 1 ) {
			strcat( combined, " " );
		}
		l += len;
	}
	Cvar_Set2( Cmd_Argv( 1 ), combined, qfalse );
}

/*
============
Cvar_SetU_f

As Cvar_Set, but also flags it as userinfo
============
*/
void Cvar_SetU_f( void ) {
	cvar_t  *v;

	if ( Cmd_Argc() != 3 ) {
		Com_Printf( "usage: setu <variable> <value>\n" );
		return;
	}
	Cvar_Set_f();
	v = Cvar_FindVar( Cmd_Argv( 1 ) );
	if ( !v ) {
		return;
	}
	v->flags |= CVAR_USERINFO;
}

/*
============
Cvar_SetS_f

As Cvar_Set, but also flags it as serverinfo
============
*/
void Cvar_SetS_f( void ) {
	cvar_t  *v;

	if ( Cmd_Argc() != 3 ) {
		Com_Printf( "usage: sets <variable> <value>\n" );
		return;
	}
	Cvar_Set_f();
	v = Cvar_FindVar( Cmd_Argv( 1 ) );
	if ( !v ) {
		return;
	}
	v->flags |= CVAR_SERVERINFO;
}

/*
============
Cvar_SetA_f

As Cvar_Set, but also flags it as archived
============
*/
void Cvar_SetA_f( void ) {
	cvar_t  *v;

	if ( Cmd_Argc() != 3 ) {
		Com_Printf( "usage: seta <variable> <value>\n" );
		return;
	}
	Cvar_Set_f();
	v = Cvar_FindVar( Cmd_Argv( 1 ) );
	if ( !v ) {
		return;
	}
	v->flags |= CVAR_ARCHIVE;
}

/*
============
Cvar_Reset_f
============
*/
void Cvar_Reset_f( void ) {
	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "usage: reset <variable>\n" );
		return;
	}
	Cvar_Reset( Cmd_Argv( 1 ) );
}

/*
============
Cvar_WriteVariables

Appends lines containing "set variable value" for all variables
with the archive flag set to qtrue.
============
*/
void Cvar_WriteVariables( fileHandle_t f ) {
	cvar_t  *var;
	char buffer[1024];

	for ( var = cvar_vars ; var ; var = var->next ) {
		if ( Q_stricmp( var->name, "cl_cdkey" ) == 0 ) {
			continue;
		}
		if ( var->flags & CVAR_ARCHIVE ) {
			// write the latched value, even if it hasn't taken effect yet
			if ( var->latchedString ) {
				Com_sprintf( buffer, sizeof( buffer ), "seta %s \"%s\"\n", var->name, var->latchedString );
			} else {
				Com_sprintf( buffer, sizeof( buffer ), "seta %s \"%s\"\n", var->name, var->string );
			}
			FS_Printf( f, "%s", buffer );
		}
	}
}

/*
============
Cvar_List_f
============
*/
void Cvar_List_f( void ) {
	cvar_t  *var;
	int i;
	char    *match;

	if ( Cmd_Argc() > 1 ) {
		match = Cmd_Argv( 1 );
	} else {
		match = NULL;
	}

	i = 0;
	for ( var = cvar_vars ; var ; var = var->next, i++ )
	{
		if ( match && !Com_Filter( match, var->name, qfalse ) ) {
			continue;
		}

		if ( var->flags & CVAR_SERVERINFO ) {
			Com_Printf( "S" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_USERINFO ) {
			Com_Printf( "U" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_ROM ) {
			Com_Printf( "R" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_INIT ) {
			Com_Printf( "I" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_ARCHIVE ) {
			Com_Printf( "A" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_LATCH ) {
			Com_Printf( "L" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_CHEAT ) {
			Com_Printf( "C" );
		} else {
			Com_Printf( " " );
		}

		Com_Printf( " %s \"%s\"\n", var->name, var->string );
	}

	Com_Printf( "\n%i total cvars\n", i );
	Com_Printf( "%i cvar indexes\n", cvar_numIndexes );
}

/*
============
Cvar_RestrictedList_f

Prints the list of all the restricted/controlled cvars on a server.
============
*/
void Cvar_RestrictedList_f(void) {
	cvar_rest_t* var;
	int i, j = 0, k = 0, v = 0;

#ifndef _DEBUG
	if (!com_dedicated->integer && !clientIsConnected) {
		Com_Printf("Restricted list is not available in offline mode.\n");
		return;
	}
#endif // !_DEBUG

#ifdef DEDICATED
	Com_Printf("Active restricted cvars:\n^7-----------------------------------------\n");
#else
	Com_Printf("^5Active restricted cvars:\n^7-----------------------------------------\n");
#endif

	i = 0;
	for (var = cvar_rest_vars; var; var = var->next, i++) {

		if (!var) {
			continue;
		}

		if (var->type == SVC_NONE) {
			continue;
		}

#ifdef DEDICATED
		Com_Printf("%-32s %s %s %s\n", 
			var->name,
			Cvar_Restriction_Flags[var->type].longDesc, 
			var->sVal1, 
			(var->sVal2 == NULL ? NULL : var->sVal2)
		);
#else
#if _DEBUG
		Com_Printf("%s%-32s %5s %s %s %s ^z[%s]\n", 
			(var->flagged ? "^n" : "^5"), 
			var->name, 
			(var->flagged?"[NO]":"[OK]"),
			Cvar_Restriction_Flags[var->type].longDesc, 
			var->sVal1, 
			(var->sVal2 == NULL ? NULL : var->sVal2),
			Cvar_VariableString(var->name)
		);
#else
		Com_Printf("%s%-32s %5s %s %s %s\n",
			(var->flagged?"^n":"^5"),
			var->name,
			(var->flagged?"[NO]":"[OK]"),
			Cvar_Restriction_Flags[var->type].longDesc,
			var->sVal1,
			(var->sVal2 == NULL ? NULL : var->sVal2)
		);
#endif
#endif
		j++;
		if (var->flagged) {
			v++;
		}
	}

#ifdef DEDICATED
	if (i != j) {
		Com_Printf("\nIgnored cvars due wrong values:\n^7-----------------------------------------\n");

		for (var = cvar_rest_vars; var; var = var->next, k++) {

			if (var->type != SVC_NONE) {
				continue;
			}
			Com_Printf("%-32s %s %s %s\n", var->name, Cvar_Restriction_Flags[var->type].longDesc, var->sVal1, (var->sVal2 == NULL ? NULL : var->sVal2));
		}
	}
#endif

	if (j < 1) {
		Com_Printf("<none set>\n");
	}
	else {
		Com_Printf("-----------------------------------------\n");
#ifdef DEDICATED
		Com_Printf("Active %i restricted cvars [Total %d]\n\n", j, i);
#else
		Com_Printf("^5Total: %i restricted cvar%s ^n[Violations: %i]\n\n", j, (i == 1?"":"s"), v);
#endif
	}
}

/*
============
Cvar_Rest_Reset_Single

Wipes the cvar completely ..
============
*/
void Cvar_Rest_Reset_Single(cvar_rest_t* var) {
	if (var->sVal1 && var->sVal1 != NULL) {
		Z_Free(var->sVal1);
	}
	if (var->sVal2 && var->sVal2 != NULL) {
		Z_Free(var->sVal2);
	}

	memset(var, 0, sizeof(var));
}

/*
============
Cvar_Rest_Reset

Wipes the list completely ..
============
*/
void Cvar_Rest_Reset(void) {
	cvar_rest_t* var;
	cvar_rest_t** prev;

	prev = &cvar_rest_vars;
	while (1) {
		var = *prev;
		if (!var) {
			break;
		}

		*prev = var->next;
		Cvar_Rest_Reset_Single(var);
	}
}

/*
============
Cvar_GetRestrictedList

Builds restricted list that is send to a client
============
*/
char* Cvar_GetRestrictedList(void) {
	cvar_rest_t* var;
	char* out = "";

	for (var = cvar_rest_vars; var; var = var->next) {
		if (var->type == SVC_NONE) {
			continue;
		}

		if (Q_stricmp(var->name, "")) {
			out = va("%s%s\\%d\\%s\\%s|", out, var->name, var->type, var->sVal1, (!Q_stricmp(var->sVal2, "") ? "@" : var->sVal2));
		}
	}
	return out;
}

/*
============
Cvar_Restart_f

Resets all cvars to their hardcoded values
============
*/
void Cvar_Restart_f( void ) {
	cvar_t  *var;
	cvar_t  **prev;

	prev = &cvar_vars;
	while ( 1 ) {
		var = *prev;
		if ( !var ) {
			break;
		}

		// don't mess with rom values, or some inter-module
		// communication will get broken (com_cl_running, etc)
		if ( var->flags & ( CVAR_ROM | CVAR_INIT | CVAR_NORESTART ) ) {
			prev = &var->next;
			continue;
		}

		// throw out any variables the user created
		if ( var->flags & CVAR_USER_CREATED ) {
			*prev = var->next;
			if ( var->name ) {
				Z_Free( var->name );
			}
			if ( var->string ) {
				Z_Free( var->string );
			}
			if ( var->latchedString ) {
				Z_Free( var->latchedString );
			}
			if ( var->resetString ) {
				Z_Free( var->resetString );
			}
			// clear the var completely, since we
			// can't remove the index from the list
			memset( var, 0, sizeof( var ) );
			continue;
		}

		Cvar_Set( var->name, var->resetString );

		prev = &var->next;
	}
}

/*
=====================
Cvar_InfoString
=====================
*/
char* Cvar_InfoString( int bit ) {
	static char info[MAX_INFO_STRING];
	cvar_t  *var;

	info[0] = 0;

	for ( var = cvar_vars ; var ; var = var->next ) {
		if ( var->flags & bit ) {
			Info_SetValueForKey( info, var->name, var->string );
		}
	}
	return info;
}

/*
=====================
Cvar_InfoString_Big

  handles large info strings ( CS_SYSTEMINFO )
=====================
*/
char* Cvar_InfoString_Big( int bit ) {
	static char info[BIG_INFO_STRING];
	cvar_t  *var;

	info[0] = 0;

	for ( var = cvar_vars ; var ; var = var->next ) {
		if ( var->flags & bit ) {
			Info_SetValueForKey_Big( info, var->name, var->string );
		}
	}
	return info;
}

/*
=====================
Cvar_InfoStringBuffer
=====================
*/
void Cvar_InfoStringBuffer( int bit, char* buff, int buffsize ) {
	Q_strncpyz( buff,Cvar_InfoString( bit ),buffsize );
}

/*
=====================
Cvar_Register

basically a slightly modified Cvar_Get for the interpreted modules
=====================
*/
void Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags ) {
	cvar_t  *cv;

	cv = Cvar_Get( varName, defaultValue, flags );
	if ( !vmCvar ) {
		return;
	}

	vmCvar->handle = cv - cvar_indexes;
	vmCvar->modificationCount = -1;
	Cvar_Update( vmCvar );
}

/*
=====================
Cvar_Update

updates an interpreted modules' version of a cvar
=====================
*/
void Cvar_Update( vmCvar_t *vmCvar ) {
	cvar_t  *cv = NULL; // bk001129
	cvar_rest_t* cr = NULL;
	assert( vmCvar ); // bk

	if ( (unsigned)vmCvar->handle >= cvar_numIndexes ) {
		Com_Error( ERR_DROP, "Cvar_Update: handle out of range" );
	}

	cv = cvar_indexes + vmCvar->handle;
	if ( cv->modificationCount == vmCvar->modificationCount ) {
		return;
	}
	if ( !cv->string ) {
		return;     // variable might have been cleared by a cvar_restart
	}

	vmCvar->modificationCount = cv->modificationCount;
	// bk001129 - mismatches.
	if ( strlen( cv->string ) + 1 > MAX_CVAR_VALUE_STRING ) {
		Com_Error( ERR_DROP, "Cvar_Update: src %s length %d exceeds MAX_CVAR_VALUE_STRING",
				   cv->string,
				   strlen( cv->string ),
				   sizeof( vmCvar->string ) );
	}
	// bk001212 - Q_strncpyz guarantees zero padding and dest[MAX_CVAR_VALUE_STRING-1]==0
	// bk001129 - paranoia. Never trust the destination string.
	// bk001129 - beware, sizeof(char*) is always 4 (for cv->string).
	//            sizeof(vmCvar->string) always MAX_CVAR_VALUE_STRING
	//Q_strncpyz( vmCvar->string, cv->string, sizeof( vmCvar->string ) ); // id
	Q_strncpyz( vmCvar->string, cv->string,  MAX_CVAR_VALUE_STRING );

	vmCvar->value = cv->value;
	vmCvar->integer = cv->integer;
}

/*
============
var_RestValueIsValid

Check if value can be applied
============
*/
qboolean Cvar_RestValueIsValid(cvar_rest_t* var, const char* value) {

	if (var && strlen(value) > 0) {
		float fValue = 0.0;

		if (var->type == SVC_NONE) {
			return qtrue;
		}

		if (Q_IsNumeric(value)) {
			fValue = atof(value);
		}

		switch (var->type) {
			case SVC_EQUAL:
				if (Q_IsNumeric(value)) {
					if (var->fVal1 != fValue) {
						return qfalse;
					}
					return qtrue;
				}
				return Q_stricmp(var->sVal1, value);
			break;
			case SVC_NOTEQUAL:
				if (Q_IsNumeric(value)) {
					if (var->fVal1 == fValue) {
						return qfalse;
					}
					return qtrue;
				}
				return !Q_stricmp(var->sVal1, value);
			break;
			case SVC_GREATER:
				return (fValue > var->fVal1);
			break;
			case SVC_GREATEREQUAL:
				return (fValue >= var->fVal1);
			break;
			case SVC_LOWER:
				return (fValue < var->fVal1);
			break;
			case SVC_LOWEREQUAL:
				return (fValue <= var->fVal1);
			break;
			case SVC_INSIDE:
				return (fValue >= var->fVal1 && fValue <= var->fVal2);
			break;
			case SVC_OUTSIDE:
				return !(fValue >= var->fVal1 && fValue <= var->fVal2);
			break;
			case SVC_INCLUDE:
				return (strstr(value, var->sVal1) ? qtrue : qfalse);
			break;
			case SVC_EXCLUDE:
				return (!strstr(value, var->sVal1) ? qtrue : qfalse);
			break;
			case SVC_WITHBITS:
				return !(var->iVal1 & atoi(value));
			break;
			case SVC_WITHOUTBITS:
				return (var->iVal1 & atoi(value));
			break;
		}
	}
	return qtrue;
}

/*
============
Cvar_ValidateRest

Validates cvars and returns violations count
============
*/
int Cvar_ValidateRest(qboolean flagOnly) {
	cvar_t* var;
	cvar_rest_t* cv;
	int i = 0, violations = 0;

	for (cv = cvar_rest_vars; cv; cv = cv->next, i++) {
		cv->flagged = qfalse;

		var = Cvar_FindVar(cv->name);
		if (!var) {
			if (cv->type == SVC_INCLUDE || cv->type == SVC_WITHBITS) {
				violations++;

				if (!flagOnly) {
					cv->flagged = qtrue;
				}
			}
		}
		else if (!Cvar_RestValueIsValid(cv, var->string)) {
			violations++;

			if (!flagOnly) {
				cv->flagged = qtrue;
			}
		}
	}
	return (i > 0 ? violations : -1);
}

/*
=================
Cvar_RestBuildList

Builds actual data
=================
*/
void Cvar_RestBuildList(char* data) {
	Cvar_Rest_Reset();

	if (data) {
		char* next = NULL;
		char* first = strtok_s(data, "|", &next);

		do {
			char* posn = NULL;

			Cmd_TokenizeLine(first, "\\", posn);
			if (Cmd_Argv(0)) {
				Cvar_SetRestricted(Cmd_Argv(0), atoi(Cmd_Argv(1)), Cmd_Argv(2), !Q_stricmp(Cmd_Argv(3), "@") ? "" : Cmd_Argv(3));
			}
		} while ((first = strtok_s(NULL, "|", &next)) != NULL);
	}
	Com_DPrintf("\nRestriction list has been updated.\n");
}

/*
============
Cvar_RestFlagged

Checks if cvar is flagged
============
*/
qboolean Cvar_RestFlagged(const char* var_name) {
	cvar_rest_t* var;

	var = Cvar_Rest_FindVar(var_name);
	if (var) {
		return var->flagged;
	}
	return qfalse;
}

/*
============
Cvar_RestAcceptedValues

Returns which values are accepted for a specific cvar
============
*/
char* Cvar_RestAcceptedValues(const char* var_name) {
	cvar_rest_t* var;
	char* message = "";

	var = Cvar_Rest_FindVar(var_name);
	if (!var || var->type == SVC_NONE) {
		return va("%s is not a restricted variable.", var_name);
	}

	switch (var->type) {
		case SVC_EQUAL:
			message = va("MUST %s %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1);
		break;
		case SVC_NOTEQUAL:
			message = va("MUST %s %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1);
		break;
		case SVC_GREATER:
			message = va("HAS TO BE %s THAN %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1);
		break;
		case SVC_GREATEREQUAL:
			message = va("HAS TO BE %s THAN %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1);
		break;
		case SVC_LOWER:
			message = va("HAS TO BE %s THAN %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1);
		break;
		case SVC_LOWEREQUAL:
			message = va("HAS TO BE %s THAN %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1);
		break;
		case SVC_INSIDE:
			message = va("HAS TO BE %s %s AND %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1, var->sVal2);
		break;
		case SVC_OUTSIDE:
			message = va("HAS TO BE %s %s AND %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1, var->sVal2);
		break;
		case SVC_INCLUDE:
			message = va("MUST %s %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1);
		break;
		case SVC_EXCLUDE:
			message = va("MUST %s %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1);
		break;
		case SVC_WITHBITS:
			message = va("MUST BE %s %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1);
		break;
		case SVC_WITHOUTBITS:
			message = va("MUST %s %s", Cvar_Restriction_Flags[var->type].longDesc, var->sVal1);
		break;
	}
	return message;
}

/*
============
RestrictedTypeToInt

Remaps string to int
============
*/
unsigned int RestrictedTypeToInt(char* str) {
	int i;

	for (i = 0; i < SVC_MAX; i++) {
		if (!Q_stricmp(str, Cvar_Restriction_Flags[i].operator))
			return Cvar_Restriction_Flags[i].type;
	}
	return SVC_NONE;
}

/*
============
Cvar_Init

Reads in all archived cvars
============
*/
void Cvar_Init( void ) {
	cvar_cheats = Cvar_Get( "sv_cheats", "1", CVAR_ROM | CVAR_SYSTEMINFO );

	Cmd_AddCommand( "toggle", Cvar_Toggle_f );
	Cmd_AddCommand( "set", Cvar_Set_f );
	Cmd_AddCommand( "sets", Cvar_SetS_f );
	Cmd_AddCommand( "setu", Cvar_SetU_f );
	Cmd_AddCommand( "seta", Cvar_SetA_f );
	Cmd_AddCommand( "reset", Cvar_Reset_f );
	Cmd_AddCommand( "cvarlist", Cvar_List_f );
#ifdef DEDICATED
	Cmd_AddCommand( "restrictedlist", Cvar_RestrictedList_f);
#else
	Cmd_AddCommand("violations", Cvar_RestrictedList_f);
#endif
	Cmd_AddCommand( "cvar_restart", Cvar_Restart_f );

	// NERVE - SMF - can't rely on autoexec to do this
	Cvar_Get( "devdll", "1", CVAR_ROM );
}
