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


/*****************************************************************************
 * name:		l_util.h
 *
 * desc:		utils
 *
 *
 *****************************************************************************/

#define Vector2Angles( v,a )      vectoangles( v,a )
#define MAX_PATH                MAX_QPATH
#define Maximum( x,y )            ( x > y ? x : y )
#define Minimum( x,y )            ( x < y ? x : y )

#ifndef MAX_QPATH
  #define MAX_QPATH 64
#endif

// compiler specific attribute translation
// msvc should come first, so if clang is in msvc mode it gets the right defines

// NOTE: this will only do checking in msvc with versions that support /analyze
#ifdef _MSC_VER
#include <stddef.h>
#ifdef _USE_ATTRIBUTES_FOR_SAL
#undef _USE_ATTRIBUTES_FOR_SAL
#endif
/* nolint */
#define _USE_ATTRIBUTES_FOR_SAL 1
#include <sal.h> // @manual
#define PRINTF_FORMAT _Printf_format_string_
#define PRINTF_FORMAT_ATTR(format_param, dots_param) /**/
#else
#define PRINTF_FORMAT /**/
#define PRINTF_FORMAT_ATTR(format_param, dots_param) \
  __attribute__((__format__(__printf__, format_param, dots_param)))
#endif

