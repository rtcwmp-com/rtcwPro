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

#include "q_shared.h"
#include "qcommon.h"


/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/


void MSG_Init( msg_t *buf, byte *data, int length ) {
	memset( buf, 0, sizeof( *buf ) );
	buf->data = data;
	buf->maxsize = length;
}

void MSG_InitOOB( msg_t *buf, byte *data, int length ) {
	memset( buf, 0, sizeof( *buf ) );
	buf->data = data;
	buf->maxsize = length;
	buf->oob = qtrue;
}

void MSG_Clear( msg_t *buf ) {
	buf->cursize = 0;
	buf->overflowed = qfalse;
	buf->bit = 0;                   //<- in bits
}


void MSG_Bitstream( msg_t *buf ) {
	buf->oob = qfalse;
}

void MSG_BeginReading( msg_t *msg ) {
	msg->readcount = 0;
	msg->bit = 0;
	msg->oob = qfalse;
}

void MSG_BeginReadingOOB( msg_t *msg ) {
	msg->readcount = 0;
	msg->bit = 0;
	msg->oob = qtrue;
}

void MSG_Copy( msg_t *buf, byte *data, int length, msg_t *src ) {
	if ( length < src->cursize ) {
		Com_Error( ERR_DROP, "MSG_Copy: can't copy into a smaller msg_t buffer" );
	}
	Com_Memcpy( buf, src, sizeof( msg_t ) );
	buf->data = data;
	Com_Memcpy( buf->data, src->data, src->cursize );
}

/*
=============================================================================

bit functions

=============================================================================
*/

// negative bit values include signs
void MSG_WriteBits( msg_t *msg, int value, int bits ) {
	int i, bitIndex;


	msg->uncompsize += bits;            // NERVE - SMF - net debugging

	// this isn't an exact overflow check, but close enough
	if ( msg->maxsize - msg->cursize < 4 ) {
		msg->overflowed = qtrue;
		return;
	}

	if ( bits == 0 || bits < -31 || bits > 32 ) {
		Com_Error( ERR_DROP, "MSG_WriteBits: bad bits %i", bits );
	}

	if ( bits < 0 ) {
		bits = -bits;
	}
	if ( msg->oob ) {
		if ( bits == 8 ) {
			msg->data[msg->cursize] = value;
			msg->cursize += 1;
			msg->bit += 8;
		} else if ( bits == 16 ) {
			unsigned short *sp = (unsigned short *)&msg->data[msg->cursize];
			*sp = LittleShort( value );
			msg->cursize += 2;
			msg->bit += 16;
		} else if ( bits == 32 ) {
			unsigned int *ip = (unsigned int *)&msg->data[msg->cursize];
			*ip = LittleLong( value );
			msg->cursize += 4;
			msg->bit += 32;
		} else {
			Com_Error( ERR_DROP, "can't read %d bits\n", bits );
		}
	} else {
		value &= ( 0xffffffff >> ( 32 - bits ) );
		if ( bits & 7 ) {
			int nbits;
			nbits = bits & 7;
			bitIndex = msg->bit;
			for ( i = 0; i < nbits; i++ ) {
				StatHuff_WriteBit((value & 1), msg->data, bitIndex);
				value = ( value >> 1 );
				bitIndex++;
			}
			msg->bit = bitIndex;
			bits = bits - nbits;
		}
		if ( bits ) {
			bitIndex = msg->bit;
			for ( i = 0; i < bits; i += 8 ) {
				bitIndex += StatHuff_WriteSymbol((value & 0xff), msg->data, bitIndex);
				value = ( value >> 8 );
			}
			msg->bit = bitIndex;
		}
		msg->cursize = ( msg->bit >> 3 ) + 1;
	}
}

int MSG_ReadBits( msg_t *msg, int bits ) {
	int value;
	int get;
	qboolean sgn;
	int i, nbits, bitIndex;

	value = 0;

	if ( bits < 0 ) {
		bits = -bits;
		sgn = qtrue;
	} else {
		sgn = qfalse;
	}

	if ( msg->oob ) {
		if ( bits == 8 ) {
			value = msg->data[msg->readcount];
			msg->readcount += 1;
			msg->bit += 8;
		} else if ( bits == 16 ) {
			unsigned short *sp = (unsigned short *)&msg->data[msg->readcount];
			value = LittleShort( *sp );
			msg->readcount += 2;
			msg->bit += 16;
		} else if ( bits == 32 ) {
			unsigned int *ip = (unsigned int *)&msg->data[msg->readcount];
			value = LittleLong( *ip );
			msg->readcount += 4;
			msg->bit += 32;
		} else {
			Com_Error( ERR_DROP, "can't read %d bits\n", bits );
		}
	} else {
		nbits = 0;
		if ( bits & 7 ) {
			nbits = bits & 7;
			bitIndex = msg->bit;
			for ( i = 0; i < nbits; i++ ) {
				value |= StatHuff_ReadBit(msg->data, bitIndex) << i;
				bitIndex++;
			}
			msg->bit = bitIndex;
			bits = bits - nbits;
		}
		if ( bits ) {
			bitIndex = msg->bit;
			for ( i = 0; i < bits; i += 8 ) {
				bitIndex += StatHuff_ReadSymbol(&get, msg->data, bitIndex);
				value |= ( get << ( i + nbits ) );
			}
			msg->bit = bitIndex;
		}
		msg->readcount = ( msg->bit >> 3 ) + 1;
	}
	if ( sgn ) {
		if ( value & ( 1 << ( bits - 1 ) ) ) {
			value |= -1 ^ ( ( 1 << bits ) - 1 );
		}
	}

	return value;
}



//================================================================================

//
// writing functions
//

void MSG_WriteChar( msg_t *sb, int c ) {
#ifdef PARANOID
	if ( c < -128 || c > 127 ) {
		Com_Error( ERR_FATAL, "MSG_WriteChar: range error" );
	}
#endif

	MSG_WriteBits( sb, c, 8 );
}

void MSG_WriteByte( msg_t *sb, int c ) {
#ifdef PARANOID
	if ( c < 0 || c > 255 ) {
		Com_Error( ERR_FATAL, "MSG_WriteByte: range error" );
	}
#endif

	MSG_WriteBits( sb, c, 8 );
}

void MSG_WriteData( msg_t *buf, const void *data, int length ) {
	int i;
	for ( i = 0; i < length; i++ ) {
		MSG_WriteByte( buf, ( (byte *)data )[i] );
	}
}

void MSG_WriteShort( msg_t *sb, int c ) {
#ifdef PARANOID
	if ( c < ( (short)0x8000 ) || c > (short)0x7fff ) {
		Com_Error( ERR_FATAL, "MSG_WriteShort: range error" );
	}
#endif

	MSG_WriteBits( sb, c, 16 );
}

void MSG_WriteLong( msg_t *sb, int c ) {
	MSG_WriteBits( sb, c, 32 );
}

void MSG_WriteFloat( msg_t *sb, float f ) {
	union {
		float f;
		int l;
	} dat;

	dat.f = f;
	MSG_WriteBits( sb, dat.l, 32 );
}

void MSG_WriteString( msg_t *sb, const char *s ) {
	if ( !s ) {
		MSG_WriteData( sb, "", 1 );
	} else {
		int l,i;
		char string[MAX_STRING_CHARS];

		l = strlen( s );
		if ( l >= MAX_STRING_CHARS ) {
			Com_Printf( "MSG_WriteString: MAX_STRING_CHARS" );
			MSG_WriteData( sb, "", 1 );
			return;
		}
		Q_strncpyz( string, s, sizeof( string ) );

		// get rid of 0xff chars, because old clients don't like them
		for ( i = 0 ; i < l ; i++ ) {
			if ( ( (byte *)string )[i] > 127 ) {
				string[i] = '.';
			}
		}

		MSG_WriteData( sb, string, l + 1 );
	}
}

void MSG_WriteBigString( msg_t *sb, const char *s ) {
	if ( !s ) {
		MSG_WriteData( sb, "", 1 );
	} else {
		int l, i;
		char string[BIG_INFO_STRING];

		l = strlen( s );
		if ( l >= BIG_INFO_STRING ) {
			Com_Printf( "MSG_WriteString: BIG_INFO_STRING" );
			MSG_WriteData( sb, "", 1 );
			return;
		}
		Q_strncpyz( string, s, sizeof( string ) );

		// get rid of 0xff chars, because old clients don't like them
		for ( i = 0 ; i < l ; i++ ) {
			if ( ( (byte *)string )[i] > 127 ) {
				string[i] = '.';
			}
		}

		MSG_WriteData( sb, string, l + 1 );
	}
}

void MSG_WriteAngle( msg_t *sb, float f ) {
	MSG_WriteByte( sb, (int)( f * 256 / 360 ) & 255 );
}

void MSG_WriteAngle16( msg_t *sb, float f ) {
	MSG_WriteShort( sb, ANGLE2SHORT( f ) );
}


//============================================================

//
// reading functions
//

// returns -1 if no more characters are available
int MSG_ReadChar( msg_t *msg ) {
	int c;

	c = (signed char)MSG_ReadBits( msg, 8 );
	if ( msg->readcount > msg->cursize ) {
		c = -1;
	}

	return c;
}

int MSG_ReadByte( msg_t *msg ) {
	int c;

	c = (unsigned char)MSG_ReadBits( msg, 8 );
	if ( msg->readcount > msg->cursize ) {
		c = -1;
	}
	return c;
}

int MSG_ReadShort( msg_t *msg ) {
	int c;

	c = (short)MSG_ReadBits( msg, 16 );
	if ( msg->readcount > msg->cursize ) {
		c = -1;
	}

	return c;
}

int MSG_ReadLong( msg_t *msg ) {
	int c;

	c = MSG_ReadBits( msg, 32 );
	if ( msg->readcount > msg->cursize ) {
		c = -1;
	}

	return c;
}

float MSG_ReadFloat( msg_t *msg ) {
	union {
		byte b[4];
		float f;
		int l;
	} dat;

	dat.l = MSG_ReadBits( msg, 32 );
	if ( msg->readcount > msg->cursize ) {
		dat.f = -1;
	}

	return dat.f;
}

//
// Notes about the MSG_Read*String* functions
//
// If N is the string buffer size, then:
// - the max. string length is N-1 because
//   the NULL-terminator is part of the buffer
// - the loop can call MSG_ReadByte at most N times
// - the loop can write at most N-1 chars
//   to leave space for the NULL terminator
//
// The "q3msgboom" bug was happening because a string
// of length >= N-1 was invoking MSG_ReadByte N-1 times
// instead of N times, leaving the bit stream in an
// invalid state and later leading to the
// "Illegible server message" drop error.
//

char *MSG_ReadString( msg_t *msg ) {
	static char	string[MAX_STRING_CHARS];

	int l = 0;
	for (;;) {
		const int c = MSG_ReadByte(msg);
		if (c <= 0 || l >= sizeof(string) - 1)
			break;

		string[l++] = c == '%' || c > 127 ? '.' : c;
	}
	string[l] = '\0';

	return string;
}

char *MSG_ReadBigString( msg_t *msg ) {
	static char	string[BIG_INFO_STRING];

	int l = 0;
	for (;;) {
		const int c = MSG_ReadByte(msg);
		if (c <= 0 || l >= sizeof(string) - 1)
			break;

		string[l++] = c == '%' || c > 127 ? '.' : c;
	}
	string[l] = '\0';

	return string;
}

char *MSG_ReadStringLine( msg_t *msg ) {
	static char	string[MAX_STRING_CHARS];

	int l = 0;
	for (;;) {
		const int c = MSG_ReadByte(msg);
		if (c <= 0 || c == '\n' || l >= sizeof(string) - 1)
			break;

		string[l++] = c == '%' || c > 127 ? '.' : c;
	}
	string[l] = '\0';

	return string;
}

float MSG_ReadAngle16( msg_t *msg ) {
	return SHORT2ANGLE( MSG_ReadShort( msg ) );
}

void MSG_ReadData( msg_t *msg, void *data, int len ) {
	int i;

	for ( i = 0 ; i < len ; i++ ) {
		( (byte *)data )[i] = MSG_ReadByte( msg );
	}
}



/*
=============================================================================

delta functions with keys

=============================================================================
*/


extern cvar_t* cl_shownet;

#define LOG( x ) if ( cl_shownet && cl_shownet->integer == 4 ) { Com_Printf( "%s ", x ); };


static const int kbitmask[32] = {
	0x00000001, 0x00000003, 0x00000007, 0x0000000F,
	0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
	0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
	0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
	0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
	0x001FFFFf, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
	0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
	0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF,
};

void MSG_WriteDeltaKey( msg_t *msg, int key, int oldV, int newV, int bits ) {
	if ( oldV == newV ) {
		MSG_WriteBits( msg, 0, 1 );
		return;
	}
	MSG_WriteBits( msg, 1, 1 );
	MSG_WriteBits( msg, newV ^ key, bits );
}

int MSG_ReadDeltaKey( msg_t *msg, int key, int oldV, int bits ) {
	if ( MSG_ReadBits( msg, 1 ) ) {
		return MSG_ReadBits( msg, bits ) ^ ( key & kbitmask[bits] );
	}
	return oldV;
}

void MSG_WriteDeltaKeyFloat( msg_t *msg, int key, float oldV, float newV ) {
	if ( oldV == newV ) {
		MSG_WriteBits( msg, 0, 1 );
		return;
	}
	MSG_WriteBits( msg, 1, 1 );
	MSG_WriteBits( msg, ( *(int *)&newV ) ^ key, 32 );
}

float MSG_ReadDeltaKeyFloat( msg_t *msg, int key, float oldV ) {
	if ( MSG_ReadBits( msg, 1 ) ) {
		float newV;

		*(int *)&newV = MSG_ReadBits( msg, 32 ) ^ key;
		return newV;
	}
	return oldV;
}


/*
============================================================================

usercmd_t communication

============================================================================
*/

// ms is allways sent, the others are optional
#define CM_ANGLE1   ( 1 << 0 )
#define CM_ANGLE2   ( 1 << 1 )
#define CM_ANGLE3   ( 1 << 2 )
#define CM_FORWARD  ( 1 << 3 )
#define CM_SIDE     ( 1 << 4 )
#define CM_UP       ( 1 << 5 )
#define CM_BUTTONS  ( 1 << 6 )
#define CM_WEAPON   ( 1 << 7 )


/*
=====================
MSG_WriteDeltaUsercmd
=====================
*/
void MSG_WriteDeltaUsercmdKey( msg_t *msg, int key, usercmd_t *from, usercmd_t *to ) {
	if ( to->serverTime - from->serverTime < 256 ) {
		MSG_WriteBits( msg, 1, 1 );
		MSG_WriteBits( msg, to->serverTime - from->serverTime, 8 );
	} else {
		MSG_WriteBits( msg, 0, 1 );
		MSG_WriteBits( msg, to->serverTime, 32 );
	}
	if ( from->angles[0] == to->angles[0] &&
		 from->angles[1] == to->angles[1] &&
		 from->angles[2] == to->angles[2] &&
		 from->forwardmove == to->forwardmove &&
		 from->rightmove == to->rightmove &&
		 from->upmove == to->upmove &&
		 from->buttons == to->buttons &&
		 from->wbuttons == to->wbuttons &&
		 from->weapon == to->weapon &&
		 from->holdable == to->holdable &&
		 from->wolfkick == to->wolfkick &&
		 from->mpSetup == to->mpSetup &&            // NERVE - SMF
		 from->identClient == to->identClient ) {   // NERVE - SMF
		MSG_WriteBits( msg, 0, 1 );                 // no change
		return;
	}
	key ^= to->serverTime;
	MSG_WriteBits( msg, 1, 1 );
	MSG_WriteDeltaKey( msg, key, from->angles[0], to->angles[0], 16 );
	MSG_WriteDeltaKey( msg, key, from->angles[1], to->angles[1], 16 );
	MSG_WriteDeltaKey( msg, key, from->angles[2], to->angles[2], 16 );
	MSG_WriteDeltaKey( msg, key, from->forwardmove, to->forwardmove, 8 );
	MSG_WriteDeltaKey( msg, key, from->rightmove, to->rightmove, 8 );
	MSG_WriteDeltaKey( msg, key, from->upmove, to->upmove, 8 );
	MSG_WriteDeltaKey( msg, key, from->buttons, to->buttons, 8 );
	MSG_WriteDeltaKey( msg, key, from->wbuttons, to->wbuttons, 8 );
	MSG_WriteDeltaKey( msg, key, from->weapon, to->weapon, 8 );
	MSG_WriteDeltaKey( msg, key, from->holdable, to->holdable, 8 );
	MSG_WriteDeltaKey( msg, key, from->wolfkick, to->wolfkick, 8 );
	MSG_WriteDeltaKey( msg, key, from->mpSetup, to->mpSetup, 8 );               // NERVE - SMF
	MSG_WriteDeltaKey( msg, key, from->identClient, to->identClient, 8 );       // NERVE - SMF
}


/*
=====================
MSG_ReadDeltaUsercmd
=====================
*/
void MSG_ReadDeltaUsercmdKey( msg_t *msg, int key, usercmd_t *from, usercmd_t *to ) {
	if ( MSG_ReadBits( msg, 1 ) ) {
		to->serverTime = from->serverTime + MSG_ReadBits( msg, 8 );
	} else {
		to->serverTime = MSG_ReadBits( msg, 32 );
	}
	if ( MSG_ReadBits( msg, 1 ) ) {
		key ^= to->serverTime;
		to->angles[0] = MSG_ReadDeltaKey( msg, key, from->angles[0], 16 );
		to->angles[1] = MSG_ReadDeltaKey( msg, key, from->angles[1], 16 );
		to->angles[2] = MSG_ReadDeltaKey( msg, key, from->angles[2], 16 );
		to->forwardmove = MSG_ReadDeltaKey( msg, key, from->forwardmove, 8 );
		to->rightmove = MSG_ReadDeltaKey( msg, key, from->rightmove, 8 );
		to->upmove = MSG_ReadDeltaKey( msg, key, from->upmove, 8 );
		to->buttons = MSG_ReadDeltaKey( msg, key, from->buttons, 8 );
		to->wbuttons = MSG_ReadDeltaKey( msg, key, from->wbuttons, 8 );
		to->weapon = MSG_ReadDeltaKey( msg, key, from->weapon, 8 );
		to->holdable = MSG_ReadDeltaKey( msg, key, from->holdable, 8 );
		to->wolfkick = MSG_ReadDeltaKey( msg, key, from->wolfkick, 8 );
		to->mpSetup = MSG_ReadDeltaKey( msg, key, from->mpSetup, 8 );                    // NERVE - SMF
		to->identClient = MSG_ReadDeltaKey( msg, key, from->identClient, 8 );            // NERVE - SMF
	} else {
		to->angles[0] = from->angles[0];
		to->angles[1] = from->angles[1];
		to->angles[2] = from->angles[2];
		to->forwardmove = from->forwardmove;
		to->rightmove = from->rightmove;
		to->upmove = from->upmove;
		to->buttons = from->buttons;
		to->wbuttons = from->wbuttons;
		to->weapon = from->weapon;
		to->holdable = from->holdable;
		to->wolfkick = from->wolfkick;
		to->mpSetup = from->mpSetup;                    // NERVE - SMF
		to->identClient = from->identClient;            // NERVE - SMF
	}
}


/*
=============================================================================

entityState_t communication

=============================================================================
*/


typedef struct {
	char    *name;
	int offset;
	int bits;           // 0 = float
} netField_t;

// using the stringizing operator to save typing...
#define NETF( x ) # x,(int)&( (entityState_t*)0 )->x

netField_t entityStateFields[] =
{
	{ NETF( eType ), 8 },
	{ NETF( eFlags ), 24 },
	{ NETF( pos.trType ), 8 },
	{ NETF( pos.trTime ), 32 },
	{ NETF( pos.trDuration ), 32 },
	{ NETF( pos.trBase[0] ), 0 },
	{ NETF( pos.trBase[1] ), 0 },
	{ NETF( pos.trBase[2] ), 0 },
	{ NETF( pos.trDelta[0] ), 0 },
	{ NETF( pos.trDelta[1] ), 0 },
	{ NETF( pos.trDelta[2] ), 0 },
	{ NETF( apos.trType ), 8 },
	{ NETF( apos.trTime ), 32 },
	{ NETF( apos.trDuration ), 32 },
	{ NETF( apos.trBase[0] ), 0 },
	{ NETF( apos.trBase[1] ), 0 },
	{ NETF( apos.trBase[2] ), 0 },
	{ NETF( apos.trDelta[0] ), 0 },
	{ NETF( apos.trDelta[1] ), 0 },
	{ NETF( apos.trDelta[2] ), 0 },
	{ NETF( time ), 32 },
	{ NETF( time2 ), 32 },
	{ NETF( origin[0] ), 0 },
	{ NETF( origin[1] ), 0 },
	{ NETF( origin[2] ), 0 },
	{ NETF( origin2[0] ), 0 },
	{ NETF( origin2[1] ), 0 },
	{ NETF( origin2[2] ), 0 },
	{ NETF( angles[0] ), 0 },
	{ NETF( angles[1] ), 0 },
	{ NETF( angles[2] ), 0 },
	{ NETF( angles2[0] ), 0 },
	{ NETF( angles2[1] ), 0 },
	{ NETF( angles2[2] ), 0 },
	{ NETF( otherEntityNum ), GENTITYNUM_BITS },
	{ NETF( otherEntityNum2 ), GENTITYNUM_BITS },
	{ NETF( groundEntityNum ), GENTITYNUM_BITS },
	{ NETF( loopSound ), 8 },
	{ NETF( constantLight ), 32 },
	{ NETF( dl_intensity ), 32 }, //----(SA)	longer now to carry the corona colors
	{ NETF( modelindex ), 9 },
	{ NETF( modelindex2 ), 9 },
	{ NETF( frame ), 16 },
	{ NETF( clientNum ), 8 },
	{ NETF( solid ), 24 },
	{ NETF( event ), 10 },
	{ NETF( eventParm ), 8 },
	{ NETF( eventSequence ), 8 }, // warning: need to modify cg_event.c at "// check the sequencial list" if you change this
	{ NETF( events[0] ), 8 },
	{ NETF( events[1] ), 8 },
	{ NETF( events[2] ), 8 },
	{ NETF( events[3] ), 8 },
	{ NETF( eventParms[0] ), 8 },
	{ NETF( eventParms[1] ), 8 },
	{ NETF( eventParms[2] ), 8 },
	{ NETF( eventParms[3] ), 8 },
	{ NETF( powerups ), 16 },
	{ NETF( weapon ), 8 },
	{ NETF( legsAnim ), ANIM_BITS },
	{ NETF( torsoAnim ), ANIM_BITS },
	{ NETF( density ), 10},
	{ NETF( dmgFlags ), 32 },   //----(SA)	additional info flags for damage
	{ NETF( onFireStart ), 32},
	{ NETF( onFireEnd ), 32},
	{ NETF( aiChar ), 8},
	{ NETF( teamNum ), 8},
	{ NETF( effect1Time ), 32},
	{ NETF( effect2Time ), 32},
	{ NETF( effect3Time ), 32},
	{ NETF( aiState ), 2},
	{ NETF( animMovetype ), 4},
};


// if (int)f == f and (int)f + ( 1<<(FLOAT_INT_BITS-1) ) < ( 1 << FLOAT_INT_BITS )
// the float will be sent with FLOAT_INT_BITS, otherwise all 32 bits will be sent
#define FLOAT_INT_BITS  13
#define FLOAT_INT_BIAS  ( 1 << ( FLOAT_INT_BITS - 1 ) )

/*
==================
MSG_WriteDeltaEntity

Writes part of a packetentities message, including the entity number.
Can delta from either a baseline or a previous packet_entity
If to is NULL, a remove entity update will be sent
If force is not set, then nothing at all will be generated if the entity is
identical, under the assumption that the in-order delta code will catch it.
==================
*/
void MSG_WriteDeltaEntity( msg_t *msg, struct entityState_s *from, struct entityState_s *to,
						   qboolean force ) {
	int i, lc;
	int numFields;
	netField_t  *field;
	int trunc;
	float fullFloat;
	int         *fromF, *toF;

	numFields = sizeof( entityStateFields ) / sizeof( entityStateFields[0] );

	// all fields should be 32 bits to avoid any compiler packing issues
	// the "number" field is not part of the field list
	// if this assert fails, someone added a field to the entityState_t
	// struct without updating the message fields
	assert( numFields + 1 == sizeof( *from ) / 4 );

	// a NULL to is a delta remove message
	if ( to == NULL ) {
		if ( from == NULL ) {
			return;
		}
		if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) ) {
			Com_Printf( "W|%3i: #%-3i remove\n", msg->cursize, from->number );
		}
		MSG_WriteBits( msg, from->number, GENTITYNUM_BITS );
		MSG_WriteBits( msg, 1, 1 );
		return;
	}

	if ( to->number < 0 || to->number >= MAX_GENTITIES ) {
		Com_Error( ERR_FATAL, "MSG_WriteDeltaEntity: Bad entity number: %i", to->number );
	}

	lc = 0;
	// build the change vector as bytes so it is endien independent
	for ( i = 0, field = entityStateFields ; i < numFields ; i++, field++ ) {
		fromF = ( int * )( (byte *)from + field->offset );
		toF = ( int * )( (byte *)to + field->offset );
		if ( *fromF != *toF ) {
			lc = i + 1;
		}
	}

	if ( lc == 0 ) {
		// nothing at all changed
		if ( !force ) {
			return;     // nothing at all
		}
		// write two bits for no change
		MSG_WriteBits( msg, to->number, GENTITYNUM_BITS );
		MSG_WriteBits( msg, 0, 1 );     // not removed
		MSG_WriteBits( msg, 0, 1 );     // no delta
		return;
	}

	MSG_WriteBits( msg, to->number, GENTITYNUM_BITS );
	MSG_WriteBits( msg, 0, 1 );         // not removed
	MSG_WriteBits( msg, 1, 1 );         // we have a delta

	MSG_WriteByte( msg, lc );   // # of changes


	for ( i = 0, field = entityStateFields ; i < lc ; i++, field++ ) {
		fromF = ( int * )( (byte *)from + field->offset );
		toF = ( int * )( (byte *)to + field->offset );

		if ( *fromF == *toF ) {
			MSG_WriteBits( msg, 0, 1 ); // no change
			continue;
		}

		MSG_WriteBits( msg, 1, 1 ); // changed

		if ( field->bits == 0 ) {
			// float
			fullFloat = *(float *)toF;
			trunc = (int)fullFloat;

			if ( fullFloat == 0.0f ) {
				MSG_WriteBits( msg, 0, 1 );
			} else {
				MSG_WriteBits( msg, 1, 1 );
				if ( trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 &&
					 trunc + FLOAT_INT_BIAS < ( 1 << FLOAT_INT_BITS ) ) {
					// send as small integer
					MSG_WriteBits( msg, 0, 1 );
					MSG_WriteBits( msg, trunc + FLOAT_INT_BIAS, FLOAT_INT_BITS );
/*					if ( print ) {
						Com_Printf( "%s:%i ", field->name, trunc );
					}*/
				} else {
					// send as full floating point value
					MSG_WriteBits( msg, 1, 1 );
					MSG_WriteBits( msg, *toF, 32 );
/*					if ( print ) {
						Com_Printf( "%s:%f ", field->name, *(float *)toF );
					}*/
				}
			}
		} else {
			if ( *toF == 0 ) {
				MSG_WriteBits( msg, 0, 1 );
			} else {
				MSG_WriteBits( msg, 1, 1 );
				// integer
				MSG_WriteBits( msg, *toF, field->bits );
/*				if ( print ) {
					Com_Printf( "%s:%i ", field->name, *toF );
				}*/
			}
		}
	}

/*
	c = msg->cursize - c;

	if ( print ) {
		if ( msg->bit == 0 ) {
			endBit = msg->cursize * 8 - GENTITYNUM_BITS;
		} else {
			endBit = ( msg->cursize - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
		}
		Com_Printf( " (%i bits)\n", endBit - startBit  );
	}
*/
}

/*
==================
MSG_ReadDeltaEntity

The entity number has already been read from the message, which
is how the from state is identified.

If the delta removes the entity, entityState_t->number will be set to MAX_GENTITIES-1

Can go from either a baseline or a previous packet_entity
==================
*/
extern cvar_t  *cl_shownet;

void MSG_ReadDeltaEntity( msg_t *msg, entityState_t *from, entityState_t *to,
						  int number ) {
	int i, lc;
	int numFields;
	netField_t  *field;
	int         *fromF, *toF;
	int print;
	int trunc;
	int startBit, endBit;

	if ( number < 0 || number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "Bad delta entity number: %i", number );
	}

	if ( msg->bit == 0 ) {
		startBit = msg->readcount * 8 - GENTITYNUM_BITS;
	} else {
		startBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
	}

	// check for a remove
	if ( MSG_ReadBits( msg, 1 ) == 1 ) {
		memset( to, 0, sizeof( *to ) );
		to->number = MAX_GENTITIES - 1;
		if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) ) {
			Com_Printf( "%3i: #%-3i remove\n", msg->readcount, number );
		}
		return;
	}

	// check for no delta
	if ( MSG_ReadBits( msg, 1 ) == 0 ) {
		*to = *from;
		to->number = number;
		return;
	}

	numFields = sizeof( entityStateFields ) / sizeof( entityStateFields[0] );
	lc = MSG_ReadByte( msg );

	if ( lc > numFields ) { //DAJ FIXME have gotten lc = 76 which is bigger than numFields!
		lc = numFields;
	}

	// shownet 2/3 will interleave with other printed info, -1 will
	// just print the delta records`
	if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) ) {
		print = 1;
		Com_Printf( "%3i: #%-3i ", msg->readcount, to->number );
	} else {
		print = 0;
	}

	to->number = number;

	for ( i = 0, field = entityStateFields ; i < lc ; i++, field++ ) {
		fromF = ( int * )( (byte *)from + field->offset );
		toF = ( int * )( (byte *)to + field->offset );

		if ( !MSG_ReadBits( msg, 1 ) ) {
			// no change
			*toF = *fromF;
		} else {
			if ( field->bits == 0 ) {
				// float
				if ( MSG_ReadBits( msg, 1 ) == 0 ) {
					*(float *)toF = 0.0f;
				} else {
					if ( MSG_ReadBits( msg, 1 ) == 0 ) {
						// integral float
						trunc = MSG_ReadBits( msg, FLOAT_INT_BITS );
						// bias to allow equal parts positive and negative
						trunc -= FLOAT_INT_BIAS;
						*(float *)toF = trunc;
						if ( print ) {
							Com_Printf( "%s:%i ", field->name, trunc );
						}
					} else {
						// full floating point value
						*toF = MSG_ReadBits( msg, 32 );
						if ( print ) {
							Com_Printf( "%s:%f ", field->name, *(float *)toF );
						}
					}
				}
			} else {
				if ( MSG_ReadBits( msg, 1 ) == 0 ) {
					*toF = 0;
				} else {
					// integer
					*toF = MSG_ReadBits( msg, field->bits );
					if ( print ) {
						Com_Printf( "%s:%i ", field->name, *toF );
					}
				}
			}
//			pcount[i]++;
		}
	}
	for ( i = lc, field = &entityStateFields[lc] ; i < numFields ; i++, field++ ) {
		fromF = ( int * )( (byte *)from + field->offset );
		toF = ( int * )( (byte *)to + field->offset );
		// no change
		*toF = *fromF;
	}

	if ( print ) {
		if ( msg->bit == 0 ) {
			endBit = msg->readcount * 8 - GENTITYNUM_BITS;
		} else {
			endBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
		}
		Com_Printf( " (%i bits)\n", endBit - startBit  );
	}
}


/*
============================================================================

player_state_t communication

============================================================================
*/

// using the stringizing operator to save typing...
#define PSF( x ) # x,(int)&( (playerState_t*)0 )->x

netField_t playerStateFields[] =
{
	{ PSF( commandTime ), 32 },
	{ PSF( pm_type ), 8 },
	{ PSF( bobCycle ), 8 },
	{ PSF( pm_flags ), 16 },
	{ PSF( pm_time ), -16 },
	{ PSF( origin[0] ), 0 },
	{ PSF( origin[1] ), 0 },
	{ PSF( origin[2] ), 0 },
	{ PSF( velocity[0] ), 0 },
	{ PSF( velocity[1] ), 0 },
	{ PSF( velocity[2] ), 0 },
	{ PSF( weaponTime ), -16 },
	{ PSF( weaponDelay ), -16 },
	{ PSF( grenadeTimeLeft ), -16 },
	{ PSF( gravity ), 16 },
	{ PSF( leanf ), 0 },
	{ PSF( speed ), 16 },
	{ PSF( delta_angles[0] ), 16 },
	{ PSF( delta_angles[1] ), 16 },
	{ PSF( delta_angles[2] ), 16 },
	{ PSF( groundEntityNum ), GENTITYNUM_BITS },
	{ PSF( legsTimer ), 16 },
	{ PSF( torsoTimer ), 16 },
	{ PSF( legsAnim ), ANIM_BITS },
	{ PSF( torsoAnim ), ANIM_BITS },
	{ PSF( movementDir ), 8 },
	{ PSF( eFlags ), 24 },
	{ PSF( eventSequence ), 8 },
	{ PSF( events[0] ), 8 },
	{ PSF( events[1] ), 8 },
	{ PSF( events[2] ), 8 },
	{ PSF( events[3] ), 8 },
	{ PSF( eventParms[0] ), 8 },
	{ PSF( eventParms[1] ), 8 },
	{ PSF( eventParms[2] ), 8 },
	{ PSF( eventParms[3] ), 8 },
	{ PSF( clientNum ), 8 },
	{ PSF( weapons[0] ), 32 },
	{ PSF( weapons[1] ), 32 },
	{ PSF( weapon ), 7 }, // (SA) yup, even more
	{ PSF( weaponstate ), 4 },
	{ PSF( weapAnim ), 10 },
	{ PSF( viewangles[0] ), 0 },
	{ PSF( viewangles[1] ), 0 },
	{ PSF( viewangles[2] ), 0 },
	{ PSF( viewheight ), -8 },
	{ PSF( damageEvent ), 8 },
	{ PSF( damageYaw ), 8 },
	{ PSF( damagePitch ), 8 },
	{ PSF( damageCount ), 8 },
	{ PSF( mins[0] ), 0 },
	{ PSF( mins[1] ), 0 },
	{ PSF( mins[2] ), 0 },
	{ PSF( maxs[0] ), 0 },
	{ PSF( maxs[1] ), 0 },
	{ PSF( maxs[2] ), 0 },
	{ PSF( crouchMaxZ ), 0 },
	{ PSF( crouchViewHeight ), 0 },
	{ PSF( standViewHeight ), 0 },
	{ PSF( deadViewHeight ), 0 },
	{ PSF( runSpeedScale ), 0 },
	{ PSF( sprintSpeedScale ), 0 },
	{ PSF( crouchSpeedScale ), 0 },
	{ PSF( friction ), 0 },
	{ PSF( viewlocked ), 8 },
	{ PSF( viewlocked_entNum ), 16 },
	{ PSF( aiChar ), 8 },
	{ PSF( teamNum ), 8 },
	{ PSF( gunfx ), 8},
	{ PSF( onFireStart ), 32},
	{ PSF( curWeapHeat ), 8 },
	{ PSF( sprintTime ), 16}, // FIXME: to be removed
	{ PSF( aimSpreadScale ), 8},
	{ PSF( aiState ), 2},
	{ PSF( serverCursorHint ), 8}, //----(SA)	added
	{ PSF( serverCursorHintVal ), 8}, //----(SA)	added
	{ PSF( classWeaponTime ), 32}, // JPW NERVE
};

/*
=============
MSG_WriteDeltaPlayerstate

=============
*/
void MSG_WriteDeltaPlayerstate( msg_t *msg, struct playerState_s *from, struct playerState_s *to ) {
	int i, j, lc;
	playerState_t dummy;
	int statsbits;
	int persistantbits;
	int ammobits[4];                //----(SA)	modified
	int clipbits;                   //----(SA)	added
	int powerupbits;
	int holdablebits;
	int numFields;
	int c;
	netField_t      *field;
	int             *fromF, *toF;
	float fullFloat;
	int trunc;
	int startBit, endBit;
	int print;

	if ( !from ) {
		from = &dummy;
		memset( &dummy, 0, sizeof( dummy ) );
	}

	if ( msg->bit == 0 ) {
		startBit = msg->cursize * 8 - GENTITYNUM_BITS;
	} else {
		startBit = ( msg->cursize - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
	}

	// shownet 2/3 will interleave with other printed info, -2 will
	// just print the delta records
	if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -2 ) ) {
		print = 1;
		Com_Printf( "W|%3i: playerstate ", msg->cursize );
	} else {
		print = 0;
	}

	c = msg->cursize;

	numFields = sizeof( playerStateFields ) / sizeof( playerStateFields[0] );

	lc = 0;
	for ( i = 0, field = playerStateFields ; i < numFields ; i++, field++ ) {
		fromF = ( int * )( (byte *)from + field->offset );
		toF = ( int * )( (byte *)to + field->offset );
		if ( *fromF != *toF ) {
			lc = i + 1;
		}
	}

	MSG_WriteByte( msg, lc );   // # of changes


	for ( i = 0, field = playerStateFields ; i < lc ; i++, field++ ) {
		fromF = ( int * )( (byte *)from + field->offset );
		toF = ( int * )( (byte *)to + field->offset );

		if ( *fromF == *toF ) {
			MSG_WriteBits( msg, 0, 1 ); // no change
			continue;
		}

		MSG_WriteBits( msg, 1, 1 ); // changed
//		pcount[i]++;

		if ( field->bits == 0 ) {
			// float
			fullFloat = *(float *)toF;
			trunc = (int)fullFloat;

			if ( trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 &&
				 trunc + FLOAT_INT_BIAS < ( 1 << FLOAT_INT_BITS ) ) {
				// send as small integer
				MSG_WriteBits( msg, 0, 1 );
				MSG_WriteBits( msg, trunc + FLOAT_INT_BIAS, FLOAT_INT_BITS );
				if ( print ) {
					Com_Printf( "%s:%i ", field->name, trunc );
				}
			} else {
				// send as full floating point value
				MSG_WriteBits( msg, 1, 1 );
				MSG_WriteBits( msg, *toF, 32 );
				if ( print ) {
					Com_Printf( "%s:%f ", field->name, *(float *)toF );
				}
			}
		} else {
			// integer
			MSG_WriteBits( msg, *toF, field->bits );
			if ( print ) {
				Com_Printf( "%s:%i ", field->name, *toF );
			}
		}
	}
	c = msg->cursize - c;


	//
	// send the arrays
	//
	statsbits = 0;
	for ( i = 0 ; i < 16 ; i++ ) {
		if ( to->stats[i] != from->stats[i] ) {
			statsbits |= 1 << i;
		}
	}
	persistantbits = 0;
	for ( i = 0 ; i < 16 ; i++ ) {
		if ( to->persistant[i] != from->persistant[i] ) {
			persistantbits |= 1 << i;
		}
	}
	holdablebits = 0;
	for ( i = 0 ; i < 16 ; i++ ) {
		if ( to->holdable[i] != from->holdable[i] ) {
			holdablebits |= 1 << i;
		}
	}
	powerupbits = 0;
	for ( i = 0 ; i < 16 ; i++ ) {
		if ( to->powerups[i] != from->powerups[i] ) {
			powerupbits |= 1 << i;
		}
	}


	if ( statsbits || persistantbits || holdablebits || powerupbits ) {

		MSG_WriteBits( msg, 1, 1 ); // something changed

		if ( statsbits ) {
			MSG_WriteBits( msg, 1, 1 ); // changed
			MSG_WriteShort( msg, statsbits );
			for ( i = 0 ; i < 16 ; i++ )
				if ( statsbits & ( 1 << i ) ) {
					// RF, changed to long to allow more flexibility
//					MSG_WriteLong (msg, to->stats[i]);
					MSG_WriteShort( msg, to->stats[i] );  //----(SA)	back to short since weapon bits are handled elsewhere now
				}
		} else {
			MSG_WriteBits( msg, 0, 1 ); // no change to stats
		}


		if ( persistantbits ) {
			MSG_WriteBits( msg, 1, 1 ); // changed
			MSG_WriteShort( msg, persistantbits );
			for ( i = 0 ; i < 16 ; i++ )
				if ( persistantbits & ( 1 << i ) ) {
					MSG_WriteShort( msg, to->persistant[i] );
				}
		} else {
			MSG_WriteBits( msg, 0, 1 ); // no change to persistant
		}


		if ( holdablebits ) {
			MSG_WriteBits( msg, 1, 1 ); // changed
			MSG_WriteShort( msg, holdablebits );
			for ( i = 0 ; i < 16 ; i++ )
				if ( holdablebits & ( 1 << i ) ) {
					MSG_WriteShort( msg, to->holdable[i] );
				}
		} else {
			MSG_WriteBits( msg, 0, 1 ); // no change to holdables
		}


		if ( powerupbits ) {
			MSG_WriteBits( msg, 1, 1 ); // changed
			MSG_WriteShort( msg, powerupbits );
			for ( i = 0 ; i < 16 ; i++ )
				if ( powerupbits & ( 1 << i ) ) {
					MSG_WriteLong( msg, to->powerups[i] );
				}
		} else {
			MSG_WriteBits( msg, 0, 1 ); // no change to powerups
		}
	} else {
		MSG_WriteBits( msg, 0, 1 ); // no change to any
	}


#if 0
// RF, optimization
//		Send a single bit to signify whether or not the ammo/clip info changed.
//		If it did, send individual segments specifying offset values for each item.
	{
		int ammo_ofs;
		int clip_ofs;

		ammobits = 0;

		// ammo
		for ( i = 0 ; i < 32 ; i++ ) {
			if ( to->ammo[i] != from->ammo[i] ) {
				ammobits |= 1 << i;
			}
		}
		// ammoclip (just add these changes to the ammo changes. if either changes, we should send both, since they are likely to both change at once anyway)
		for ( i = 0 ; i < 32 ; i++ ) {
			if ( to->ammoclip[i] != from->ammoclip[i] ) {
				ammobits |= 1 << i;
			}
		}

		if ( ammobits ) {
			MSG_WriteBits( msg, 1, 1 ); // changed

			// send each changed item
			for ( i = 0 ; i < 32 ; i++ ) {
				if ( ammobits & ( 1 << i ) ) {
					ammo_ofs = to->ammo[i] - from->ammo[i];
					clip_ofs = to->ammoclip[i] - from->ammoclip[i];

					while ( ammo_ofs || clip_ofs ) {
						MSG_WriteBits( msg, 1, 1 );  // signify that another index is present
						MSG_WriteBits( msg, i, 5 );  // index number

						// ammo
						if ( abs( ammo_ofs ) > 127 ) {
							if ( ammo_ofs > 0 ) {
								MSG_WriteChar( msg, 127 );
								ammo_ofs -= 127;
							} else {
								MSG_WriteChar( msg, -127 );
								ammo_ofs += 127;
							}
						} else {
							MSG_WriteChar( msg, ammo_ofs );
							ammo_ofs = 0;
						}

						// clip
						if ( abs( clip_ofs ) > 127 ) {
							if ( clip_ofs > 0 ) {
								MSG_WriteChar( msg, 127 );
								clip_ofs -= 127;
							} else {
								MSG_WriteChar( msg, -127 );
								clip_ofs += 127;
							}
						} else {
							MSG_WriteChar( msg, clip_ofs );
							clip_ofs = 0;
						}
					}
				}
			}

			// signify the end of changes
			MSG_WriteBits( msg, 0, 1 );

		} else {
			MSG_WriteBits( msg, 0, 1 ); // no change
		}
	}

#else
//----(SA)	I split this into two groups using shorts so it wouldn't have
//			to use a long every time ammo changed for any weap.
//			this seemed like a much friendlier option than making it
//			read/write a long for any ammo change.

	// j == 0 : weaps 0-15
	// j == 1 : weaps 16-31
	// j == 2 : weaps 32-47	//----(SA)	now up to 64 (but still pretty net-friendly)
	// j == 3 : weaps 48-63

	// ammo stored
	for ( j = 0; j < 4; j++ ) {  //----(SA)	modified for 64 weaps
		ammobits[j] = 0;
		for ( i = 0 ; i < 16 ; i++ ) {
			if ( to->ammo[i + ( j * 16 )] != from->ammo[i + ( j * 16 )] ) {
				ammobits[j] |= 1 << i;
			}
		}
	}

//----(SA)	also encapsulated ammo changes into one check.  clip values will change frequently,
	// but ammo will not.  (only when you get ammo/reload rather than each shot)
	if ( ammobits[0] || ammobits[1] || ammobits[2] || ammobits[3] ) {  // if any were set...
		MSG_WriteBits( msg, 1, 1 ); // changed
		for ( j = 0; j < 4; j++ ) {
			if ( ammobits[j] ) {
				MSG_WriteBits( msg, 1, 1 ); // changed
				MSG_WriteShort( msg, ammobits[j] );
				for ( i = 0 ; i < 16 ; i++ )
					if ( ammobits[j] & ( 1 << i ) ) {
						MSG_WriteShort( msg, to->ammo[i + ( j * 16 )] );
					}
			} else {
				MSG_WriteBits( msg, 0, 1 ); // no change
			}
		}
	} else {
		MSG_WriteBits( msg, 0, 1 ); // no change
	}

	// ammo in clip
	for ( j = 0; j < 4; j++ ) {  //----(SA)	modified for 64 weaps
		clipbits = 0;
		for ( i = 0 ; i < 16 ; i++ ) {
			if ( to->ammoclip[i + ( j * 16 )] != from->ammoclip[i + ( j * 16 )] ) {
				clipbits |= 1 << i;
			}
		}
		if ( clipbits ) {
			MSG_WriteBits( msg, 1, 1 ); // changed
			MSG_WriteShort( msg, clipbits );
			for ( i = 0 ; i < 16 ; i++ )
				if ( clipbits & ( 1 << i ) ) {
					MSG_WriteShort( msg, to->ammoclip[i + ( j * 16 )] );
				}
		} else {
			MSG_WriteBits( msg, 0, 1 ); // no change
		}
	}
#endif


	if ( print ) {
		if ( msg->bit == 0 ) {
			endBit = msg->cursize * 8 - GENTITYNUM_BITS;
		} else {
			endBit = ( msg->cursize - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
		}
		Com_Printf( " (%i bits)\n", endBit - startBit  );
	}

}


/*
===================
MSG_ReadDeltaPlayerstate
===================
*/
void MSG_ReadDeltaPlayerstate( msg_t *msg, playerState_t *from, playerState_t *to ) {
	int i, j, lc;
	int bits;
	netField_t  *field;
	int numFields;
	int startBit, endBit;
	int print;
	int         *fromF, *toF;
	int trunc;
	playerState_t dummy;

	if ( !from ) {
		from = &dummy;
		memset( &dummy, 0, sizeof( dummy ) );
	}
	*to = *from;

	if ( msg->bit == 0 ) {
		startBit = msg->readcount * 8 - GENTITYNUM_BITS;
	} else {
		startBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
	}

	// shownet 2/3 will interleave with other printed info, -2 will
	// just print the delta records
	if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -2 ) ) {
		print = 1;
		Com_Printf( "%3i: playerstate ", msg->readcount );
	} else {
		print = 0;
	}

	numFields = sizeof( playerStateFields ) / sizeof( playerStateFields[0] );
	lc = MSG_ReadByte( msg );

	for ( i = 0, field = playerStateFields ; i < lc ; i++, field++ ) {
		fromF = ( int * )( (byte *)from + field->offset );
		toF = ( int * )( (byte *)to + field->offset );

		if ( !MSG_ReadBits( msg, 1 ) ) {
			// no change
			*toF = *fromF;
		} else {
			if ( field->bits == 0 ) {
				// float
				if ( MSG_ReadBits( msg, 1 ) == 0 ) {
					// integral float
					trunc = MSG_ReadBits( msg, FLOAT_INT_BITS );
					// bias to allow equal parts positive and negative
					trunc -= FLOAT_INT_BIAS;
					*(float *)toF = trunc;
					if ( print ) {
						Com_Printf( "%s:%i ", field->name, trunc );
					}
				} else {
					// full floating point value
					*toF = MSG_ReadBits( msg, 32 );
					if ( print ) {
						Com_Printf( "%s:%f ", field->name, *(float *)toF );
					}
				}
			} else {
				// integer
				*toF = MSG_ReadBits( msg, field->bits );
				if ( print ) {
					Com_Printf( "%s:%i ", field->name, *toF );
				}
			}
		}
	}
	for ( i = lc,field = &playerStateFields[lc]; i < numFields; i++, field++ ) {
		fromF = ( int * )( (byte *)from + field->offset );
		toF = ( int * )( (byte *)to + field->offset );
		// no change
		*toF = *fromF;
	}


	// read the arrays
	if ( MSG_ReadBits( msg, 1 ) ) {  // one general bit tells if any of this infrequently changing stuff has changed
		// parse stats
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG( "PS_STATS" );
			bits = MSG_ReadShort( msg );
			for ( i = 0 ; i < 16 ; i++ ) {
				if ( bits & ( 1 << i ) ) {
					// RF, changed to long to allow more flexibility
//					to->stats[i] = MSG_ReadLong(msg);
					to->stats[i] = MSG_ReadShort( msg );  //----(SA)	back to short since weapon bits are handled elsewhere now

				}
			}
		}

		// parse persistant stats
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG( "PS_PERSISTANT" );
			bits = MSG_ReadShort( msg );
			for ( i = 0 ; i < 16 ; i++ ) {
				if ( bits & ( 1 << i ) ) {
					to->persistant[i] = MSG_ReadShort( msg );
				}
			}
		}

		// parse holdable stats
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG( "PS_HOLDABLE" );
			bits = MSG_ReadShort( msg );
			for ( i = 0 ; i < 16 ; i++ ) {
				if ( bits & ( 1 << i ) ) {
					to->holdable[i] = MSG_ReadShort( msg );
				}
			}
		}

		// parse powerups
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG( "PS_POWERUPS" );
			bits = MSG_ReadShort( msg );
			for ( i = 0 ; i < 16 ; i++ ) {
				if ( bits & ( 1 << i ) ) {
					to->powerups[i] = MSG_ReadLong( msg );
				}
			}
		}
	}

#if 0
// RF, optimization
//		Send a single bit to signify whether or not the ammo/clip info changed.
//		If it did, send individual segments specifying offset values for each item.

	if ( MSG_ReadBits( msg, 1 ) ) {     // it changed
		while ( MSG_ReadBits( msg, 1 ) ) {
			i = MSG_ReadBits( msg, 5 );     // read the index number
			// now read the offsets
			to->ammo[i] += MSG_ReadChar( msg );
			to->ammoclip[i] += MSG_ReadChar( msg );
		}
	}

#else
//----(SA)	I split this into two groups using shorts so it wouldn't have
//			to use a long every time ammo changed for any weap.
//			this seemed like a much friendlier option than making it
//			read/write a long for any ammo change.

	// parse ammo

	// j == 0 : weaps 0-15
	// j == 1 : weaps 16-31
	// j == 2 : weaps 32-47	//----(SA)	now up to 64 (but still pretty net-friendly)
	// j == 3 : weaps 48-63

	// ammo stored
	if ( MSG_ReadBits( msg, 1 ) ) {     // check for any ammo change (0-63)
		for ( j = 0; j < 4; j++ ) {
			if ( MSG_ReadBits( msg, 1 ) ) {
				LOG( "PS_AMMO" );
				bits = MSG_ReadShort( msg );
				for ( i = 0 ; i < 16 ; i++ ) {
					if ( bits & ( 1 << i ) ) {
						to->ammo[i + ( j * 16 )] = MSG_ReadShort( msg );
					}
				}
			}
		}
	}

	// ammo in clip
	for ( j = 0; j < 4; j++ ) {
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG( "PS_AMMOCLIP" );
			bits = MSG_ReadShort( msg );
			for ( i = 0 ; i < 16 ; i++ ) {
				if ( bits & ( 1 << i ) ) {
					to->ammoclip[i + ( j * 16 )] = MSG_ReadShort( msg );
				}
			}
		}
	}

#endif


	if ( print ) {
		if ( msg->bit == 0 ) {
			endBit = msg->readcount * 8 - GENTITYNUM_BITS;
		} else {
			endBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
		}
		Com_Printf( " (%i bits)\n", endBit - startBit  );
	}
}

//===========================================================================
