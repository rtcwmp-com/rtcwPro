/*
===========================================================================

Return to Castle Wolfenstein multiplayer GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of the Return to Castle Wolfenstein multiplayer GPL Source Code (?RTCW MP Source Code?).

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

/*
===================
L0 - Str replacer

Ported from etPub
===================
*/
char *Q_StrReplace(char *haystack, char *needle, char *newp)
{
	static char final[MAX_STRING_CHARS] = {""};
	char dest[MAX_STRING_CHARS] = {""};
	char newStr[MAX_STRING_CHARS] = {""};
	char *destp;
	int needle_len = 0;
	int new_len = 0;

	if(!*haystack) {
		return final;
	}
	if(!*needle) {
		Q_strncpyz(final, haystack, sizeof(final));
		return final;
	}
	if(*newp) {
		Q_strncpyz(newStr, newp, sizeof(newStr));
	}

	dest[0] = '\0';
	needle_len = strlen(needle);
	new_len = strlen(newStr);
	destp = &dest[0];
	while(*haystack) {
		if(!Q_stricmpn(haystack, needle, needle_len)) {
			Q_strcat(dest, sizeof(dest), newStr);
			haystack += needle_len;
			destp += new_len;
			continue;
		}
		if(MAX_STRING_CHARS > (strlen(dest) + 1)) {
			*destp = *haystack;
			*++destp = '\0';
		}
		haystack++;
	}
	// tjw: don't work with final return value in case haystack
	//      was pointing at it.
	Q_strncpyz(final, dest, sizeof(final));
return final;
}

/*
==================
L0 - Wish it would be like in php and i wouldn't need to bother with this..
==================
*/
int is_numeric(const char *p) {
	if (*p) {
		char c;
		while ((c=*p++)) {
			if (!isdigit(c)) return 0;
		}
		return 1;
	}
return 0;
}

/*
==================
L0 - Alpha numeric check..
==================
*/
int is_alnum(const char *p) {
	if (*p) {
		char c;
		while ((c=*p++)) {
			if (!isalnum(c)) return 0;
		}
		return 1;
	}
return 0;
}


/*
==================
L0 - Strip the chars when need it
==================
*/
void stripChars( char *input, char *output, int cutSize ) {
	int lenght = strlen( input );
	int i = 0, k = 0;

	for ( i = lenght - cutSize; i < lenght; i++ )
		output[k++] = input[i];

output[k++] = '\0';
}

/*
==================
L0 - Ported from et: NQ
DecolorString

Remove color characters
==================
*/
void DecolorString( char *in, char *out)
{
	while(*in) {
		if(*in == 27 || *in == '^') {
			in++;		// skip color code
			if(*in) in++;
			continue;
		}
		*out++ = *in++;
	}
	*out = 0;
}

/*
==========
L0 - setGuid
==========
*/
void setGuid( char *in, char *out ) {
	int length = strlen( in );
	int i = 0, j = 0;

	for ( i = length - GUID_LEN; i < length; i++ )
		out[j++] = in[i];

	out[j++] = '\0';
}


/*
===========
Global sound - Hooked under cg_announced ..
===========
*/
void AAPSound(char *sound) {
	gentity_t *ent;
	gentity_t *te;

	ent = g_entities;

	te = G_TempEntity(ent->s.pos.trBase, EV_ANNOUNCER_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
	te->r.svFlags |= SVF_BROADCAST;
}
