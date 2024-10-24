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
/*
-----------------------------
	Country Flags (by mcwf)
	This is a modified version of GeoIP API (simplified to use in our case)

	OSPx NOTE - Has no IPv6 functionality but default 1.4 doesn't support it anyway..
*/
GeoIP * gidb = NULL;

unsigned long GeoIP_addr_to_num(const char *addr) {
	int i;
	char tok[4];
	int octet;
	int j = 0, k = 0;
	unsigned long ipnum = 0;
	char c = 0;

	for (i=0; i<4; i++) {
		for (;;) {
			c = addr[k++];
			if (c == '.' || c == ':' || c == '\0') {
				tok[j] = '\0';
				octet = atoi(tok);
				if (octet > 255)
					return 0;
				ipnum += (octet << ((3-i)*8));
				j = 0;
				break;
			} else if (c >= '0' && c<= '9') {
				if (j > 2) {
					return 0;
				}
				tok[j++] = c;
			} else {
				return 0;
			}
		}
		if(c == '\0' && i<3) {
			return 0;
		}
	}
	return ipnum;
}

unsigned int GeoIP_seek_record(GeoIP *gi, unsigned long ipnum) {

	int depth;
	unsigned int x = 0;
	unsigned int step = 0;
	const unsigned char *buf = NULL;

	for (depth = 31; depth >= 0; depth--) {

			step = 6 * x;

			if (step + 6 >= gi->memsize) {
				G_LogPrintf("GeoIP: Error Traversing Database for ipnum = %lu - Perhaps database is corrupt?\n",ipnum);
				return 255;
			}

			buf = gi->cache + step;

			if (ipnum & (1 << depth)) {
					x = (buf[3] << 0) + (buf[4] << 8) + (buf[5] << 16);
			} else {
					x = (buf[0] << 0) + (buf[1] << 8) + (buf[2] << 16);
			}

		if (x >= 16776960) {
			return x - 16776960;
		}
	}

	G_LogPrintf("GeoIP: Error Traversing Database for ipnum = %lu - Perhaps database is corrupt?\n",ipnum);
	return 255;
}

void GeoIP_open(void) {

	// redeye - basically not necessary but to get sure we close a maybe existing database
	GeoIP_close();

	if (!g_showFlags.integer) {
		return;
	}

	gidb = (GeoIP *)G_Alloc(sizeof(GeoIP));

	if (gidb == NULL) {
		G_LogPrintf("GeoIP: Memory allocation error for GeoIP struct\n");
		return;
	}

	gidb->memsize = trap_FS_FOpenFile("GeoIP.dat",&gidb->GeoIPDatabase,FS_READ);

	if ((int)gidb->memsize < 0) {
		G_LogPrintf("GeoIP: Error opening database\n");
		G_Free(gidb);
		gidb = NULL;
		return;

	} else if (gidb->memsize == 0) {
		G_LogPrintf("GeoIP: Error zero-sized database file\n");
		trap_FS_FCloseFile(gidb->GeoIPDatabase);
		G_Free(gidb);
		gidb = NULL;
		return;

	} else {

			gidb->cache = (unsigned char *) calloc(gidb->memsize + 1,sizeof(unsigned char));
			if (gidb->cache != NULL) {
				trap_FS_Read(gidb->cache, gidb->memsize, gidb->GeoIPDatabase);
				trap_FS_FCloseFile(gidb->GeoIPDatabase);
				return;
			}

		G_LogPrintf("GeoIP: Memory allocation error for GeoIP cache\n");
		trap_FS_FCloseFile(gidb->GeoIPDatabase);
		G_Free(gidb);
		gidb = NULL;
		return;
	}
}

void GeoIP_close(void) {
	if (gidb != NULL) {
		free(gidb->cache);
		gidb->cache = NULL;
		G_Free(gidb);
		gidb = NULL;
	}
}
