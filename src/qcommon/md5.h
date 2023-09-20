/*
* This code implements the MD5 message-digest algorithm.
* The algorithm is due to Ron Rivest.  This code was
* written by Colin Plumb in 1993, no copyright is claimed.
* This code is in the public domain; do with it what you wish.
*
* Equivalent code is available from RSA Data Security, Inc.
* This code has been tested against that, and is equivalent,
* except that you don't need to include two pages of legalese
* with every copy.
*
* To compute the message digest of a chunk of bytes, declare an
* MD5Context structure, pass it to MD5Init, call MD5Update as
* needed on buffers full of bytes, and then call MD5Final, which
* will fill a supplied 16-byte array with the digest.
*/
#ifndef __MD5_H
#define __MD5_H

#include <stdint.h>
#include "../qcommon/q_shared.h"
#include "qcommon.h"

typedef struct MD5Context {
	uint32_t buf[4];
	uint32_t bits[2];
	unsigned char in[64];
} MD5_CTX;

// MD5
char* Com_MD5File(const char* fn, int length, const char* prefix, int prefix_len);
char* Com_MD5(const void* data, int length, const char* prefix, int prefix_len, int hexcase);

#endif // __MD5_H
