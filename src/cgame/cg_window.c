/*
===========================================================================

wolfX GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of wolfX source code.  

wolfX Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

wolfX Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with wolfX Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the wolfX Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the wolfX Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
 * name:			cg_window.c
 *
 * desc:			OSP port of cgame window handling
 * Author:			Nate 'L0 (original Author: rhea@OrangeSmoothie.org)
 * created:			14 Jan / 2013
 * Last updated:	31 Jan / 2013
 * Notes:			I've added missing effects / added stats window handling / 
 *					truetype effect font is set to fixed site (for now) / few tweaks up and there.
 */

#include "cg_local.h"
#include "../ui/ui_shared.h"

extern pmove_t cg_pmove;        // cg_predict.c
extern displayContextDef_t cgDC;// L0 - Makes more sense here..	

/*
	Closes any existing window
*/
void CG_closeDemoPopWindow(void) {	
	if (cgs.demoPopUpInfo.show == SHOW_ON) {
		if (cg.time < cgs.demoPopUpInfo.fadeTime) {
			cgs.demoPopUpInfo.fadeTime = 2 * cg.time + STATS_FADE_TIME - cgs.demoPopUpInfo.fadeTime;
		}
		else {
			cgs.demoPopUpInfo.fadeTime = cg.time + STATS_FADE_TIME;
		}
		CG_windowFree(cg.demoPopupWindow);
		cg.demoPopupWindow = NULL;
	}
}

/*
	Pops up for few seconds
*/
void CG_createDemoPopUpWindow( char *str, int sec) {

	if (!demo_popupWindow.integer) {
		return;
	}
	else {
		vec4_t colorGeneralFill = { 0.1f, 0.1f, 0.1f, 0.8f };
		cg_window_t *sw = CG_windowAlloc(WFX_TEXTSIZING | WFX_FADEIN | WFX_SCROLLUP, 120);

		// Close any existing..
		CG_closeDemoPopWindow();

		cg.demoPopupWindow = sw;
		if (sw == NULL) {
			return;
		}

		// Window specific
		sw->id = WID_DEMOPOPUP;
		sw->fontScaleX = 1 * 0.7f;
		sw->fontScaleY = 1 * 0.8f;
		sw->x = 0;
		sw->y = 470;
		sw->flashPeriod = 1500;
		sw->flashMidpoint = sw->flashPeriod * 0.7f;
		memcpy(&sw->colorBackground2, colorGeneralFill, sizeof(vec4_t));

		// Mark it so it can fade away..
		cgs.demoPopUpInfo.requestTime = cg.time + (sec * 1000);

		cg.windowCurrent = sw;
		CG_printWindow((char*)str);
	}
}

/*
	Destroys pop up window 
*/
void CG_destroyDemoPopUpWindow(void) {
	if (!cg.demoPlayback) {
		return;
	}

	if (cgs.demoPopUpInfo.show == SHOW_ON && cg.time > cgs.demoPopUpInfo.requestTime) {
		if (cg.time < cgs.demoPopUpInfo.fadeTime) {
			cgs.demoPopUpInfo.fadeTime = 2 * cg.time + STATS_FADE_TIME - cgs.demoPopUpInfo.fadeTime;
		}
		else {
			cgs.demoPopUpInfo.fadeTime = cg.time + STATS_FADE_TIME;
		}
		CG_windowFree(cg.demoPopupWindow);
		cg.demoPopupWindow = NULL;
	}
}
void CG_createStatsWindow( void ) {
	cg_window_t *sw = CG_windowAlloc( WFX_TEXTSIZING | WFX_FADEIN | WFX_SCROLLUP| WFX_TRUETYPE, 110 );

	cg.statsWindow = sw;
	if ( sw == NULL ) {
		return;
	}

	// Window specific
	sw->id = WID_STATS;
	sw->fontScaleX = cf_wstats.value * 0.2f;
	sw->fontScaleY = cf_wstats.value * 0.2f;
	sw->x = ( cg.snap->ps.pm_type == PM_INTERMISSION ) ?  10 : 4;
	sw->y = ( cg.snap->ps.pm_type == PM_INTERMISSION ) ? -20 : -100;  // Align from bottom minus offset and height
}

// L0 - Adding this for 1.0 alike stats
void CG_createClientStatsWindow( void ) {
	cg_window_t *sw = CG_windowAlloc( WFX_TEXTSIZING | WFX_FADEIN | WFX_SCROLLLEFT | WFX_TRUETYPE, 110 );

	cg.clientStatsWindow = sw;
	if ( sw == NULL ) {
		return;
	}

	// Window specific
	sw->id = WID_CLIENTSTATS;
	sw->fontScaleX = cf_wstats.value * 0.2f;	// Reusing this..
	sw->fontScaleY = cf_wstats.value * 0.2f;	// reusing this..
	sw->x = ( cg.snap->ps.pm_type == PM_INTERMISSION ) ?  10 : 4;
	sw->y = ( cg.snap->ps.pm_type == PM_INTERMISSION ) ? -20 : -100;  // Align from bottom minus offset and height
}

void CG_createTopShotsWindow( void ) {
	// L0 - Made it dark so it matches wolfX
	vec4_t colorGeneralFill   = { 0.1f, 0.1f, 0.1f, 0.8f };

	cg_window_t *sw = CG_windowAlloc( WFX_TEXTSIZING | WFX_FLASH | WFX_FADEIN | WFX_SCROLLUP | WFX_TRUETYPE, 190 );

	cg.topshotsWindow = sw;
	if ( sw == NULL ) {
		return;
	}

	// Window specific
	sw->id = WID_TOPSHOTS;
	sw->fontScaleX = cf_wtopshots.value * 0.2f;
	sw->fontScaleY = cf_wtopshots.value * 0.2f;
	sw->x = ( cg.snap->ps.pm_type == PM_INTERMISSION ) ? -20 : -40;
	sw->y = ( cg.snap->ps.pm_type == PM_INTERMISSION ) ? -20 : -60;   // Align from bottom minus offset and height
	sw->flashMidpoint = sw->flashPeriod * 0.8f;
	memcpy( &sw->colorBackground2, colorGeneralFill, sizeof( vec4_t ) );
}


//////////////////////////////////////////////
//////////////////////////////////////////////
//
//      WINDOW HANDLING AND PRIMITIVES
//
//////////////////////////////////////////////
//////////////////////////////////////////////


// Windowing system setup
void CG_windowInit( void ) {
	int i;

	cg.winHandler.numActiveWindows = 0;
	for ( i = 0; i < MAX_WINDOW_COUNT; i++ ) {
		cg.winHandler.window[i].inuse = qfalse;
	}

	cg.msgWstatsWindow = NULL;
	cg.msgWtopshotsWindow = NULL;
	cg.msgClientStatsWindow = NULL;
	cg.statsWindow = NULL;
	cg.topshotsWindow = NULL;
	cg.clientStatsWindow = NULL;
	cg.demoControlsWindow = NULL;
	cg.demoPopupWindow = NULL;
}


// Window stuct "constructor" with some common defaults
void CG_windowReset( cg_window_t *w, int fx, int startupLength ) {
	// L0 - Made it more darker to match wolfX
	vec4_t colorGeneralBorder = { 0.4f, 0.4f, 0.4f, 0.5f };
	vec4_t colorGeneralFill   = { 0.1f, 0.1f, 0.1f, 0.8f };

	w->effects = fx;
	w->fontScaleX = 0.25;
	w->fontScaleY = 0.25;
	w->flashPeriod = 1000;
	w->flashMidpoint = w->flashPeriod / 2;
	w->id = WID_NONE;
	w->inuse = qtrue;
	w->lineCount = 0;
	w->state = ( fx >= WFX_FADEIN ) ? WSTATE_START : WSTATE_COMPLETE;
	w->targetTime = ( startupLength > 0 ) ? startupLength : 0;
	w->time = trap_Milliseconds();
	w->x = 0;
	w->y = 0;

	memcpy( &w->colorBorder, &colorGeneralBorder, sizeof( vec4_t ) );
	memcpy( &w->colorBackground, &colorGeneralFill, sizeof( vec4_t ) );
}

// Reserve a window
cg_window_t *CG_windowAlloc( int fx, int startupLength ) {
	int i;
	cg_window_t *w;
	cg_windowHandler_t *wh = &cg.winHandler;

	if ( wh->numActiveWindows == MAX_WINDOW_COUNT ) {
		return( NULL );
	}

	for ( i = 0; i < MAX_WINDOW_COUNT; i++ ) {
		w = &wh->window[i];
		if ( w->inuse == qfalse ) {
			CG_windowReset( w, fx, startupLength );
			wh->activeWindows[wh->numActiveWindows++] = i;
			return( w );
		}
	}

	// Fail if we're a full airplane
	return( NULL );
}


// Free up a window reservation
void CG_windowFree( cg_window_t *w ) {
	int i, j;
	cg_windowHandler_t *wh = &cg.winHandler;

	if ( w == NULL ) {
		return;
	}

	if ( w->effects >= WFX_FADEIN && w->state != WSTATE_OFF && w->inuse == qtrue ) {
		w->state = WSTATE_SHUTDOWN;
		w->time = trap_Milliseconds();
		return;
	}

	for ( i = 0; i < wh->numActiveWindows; i++ ) {
		if ( w == &wh->window[wh->activeWindows[i]] ) {
			for ( j = i; j < wh->numActiveWindows; j++ ) {
				if ( j + 1 < wh->numActiveWindows ) {
					wh->activeWindows[j] = wh->activeWindows[j + 1];
				}
			}

			w->id = WID_NONE;
			w->inuse = qfalse;
			w->state = WSTATE_OFF;

			CG_removeStrings( w );

			wh->numActiveWindows--;

			break;
		}
	}
}

void CG_windowCleanup( void ) {
	int i;
	cg_window_t *w;
	cg_windowHandler_t *wh = &cg.winHandler;

	for ( i = 0; i < wh->numActiveWindows; i++ ) {
		w = &wh->window[wh->activeWindows[i]];
		if ( !w->inuse || w->state == WSTATE_OFF ) {
			CG_windowFree( w );
			i--;
		}
	}
}

// Main window-drawing handler
void CG_windowDraw( void ) {
	int h, x, y, i, j, milli, t_offset, tmp;
	cg_window_t *w;
	qboolean fCleanup = qfalse;
	vec4_t *bg;
	vec4_t textColor, borderColor, bgColor;

// L0 - Demo ports - FINISH ME
/*
	if ( cg.winHandler.numActiveWindows == 0 ) {
		// Draw these for demoplayback no matter what
		CG_demoAviFPSDraw();
		CG_demoTimescaleDraw();
		return;
	}
*/

	// OSPx Demo code - TODO what is above?
	CG_demoView();

	if ( cg.winHandler.numActiveWindows == 0 ) {
		// OSPx - Pre-set some stuff
		if (demo_controlsWindow.integer && cg.demoPlayback) {
			cgs.demoControlInfo.show = SHOW_ON;
			CG_createControlsWindow();
		}

		if (demo_popupWindow.integer && cg.demoPlayback && !cg.advertisementDone && !demo_noAdvertisement.integer) {
			cgs.demoPopUpInfo.show = SHOW_ON;
			CG_createDemoPopUpWindow("Upload this demo: ^n/demoupload current <optional: comment>", 10);
			cg.advertisementDone = qtrue;
		}
		// ~OSPx
		return;
	}

	milli = trap_Milliseconds();
	memcpy( textColor, colorWhite, sizeof( vec4_t ) );

	for ( i = 0; i < cg.winHandler.numActiveWindows; i++ ) {
		w = &cg.winHandler.window[cg.winHandler.activeWindows[i]];

		if ( !w->inuse || w->state == WSTATE_OFF ) {
			fCleanup = qtrue;
			continue;
		}

		if ( w->effects & WFX_TEXTSIZING ) {
			CG_windowNormalizeOnText( w );
			w->effects &= ~WFX_TEXTSIZING;
		}

		bg = ( ( w->effects & WFX_FLASH ) && ( milli % w->flashPeriod ) > w->flashMidpoint ) ? &w->colorBackground2 : &w->colorBackground;

		h = w->h;
		x = w->x;
		y = w->y;
		t_offset = milli - w->time;
		textColor[3] = 1.0f;
		memcpy( &borderColor, w->colorBorder, sizeof( vec4_t ) );
		memcpy( &bgColor, bg, sizeof( vec4_t ) );
		
		if ( w->state == WSTATE_START ) {
			tmp = w->targetTime - t_offset;
			if ( w->effects & WFX_SCROLLUP ) {
				if ( tmp > 0 ) {
					y += ( 480 - y ) * tmp / w->targetTime; 
				} else {
					w->state = WSTATE_COMPLETE;
				}

				w->curY = y;
			}
			// L0 - New ones (unsued)
			// NOTE -> Scroll right = (start) right and animate to (end) left and then back..
			//         Side indicates from which corner it pops in and pops out.
			if ( w->effects & WFX_SCROLLRIGHT ) {
				if ( tmp > 0 ) {
					x += ( 680 - x ) * tmp / w->targetTime;
				} else {
					w->state = WSTATE_COMPLETE;
				}
				w->curX = x;
			}
			if ( w->effects & WFX_SCROLLLEFT ) {
				if ( tmp > 0 ) {
					x -= ( 680 + x ) * tmp / w->targetTime;
				} else {
					w->state = WSTATE_COMPLETE;
				}
				w->curX = x;
			} // End
			if ( w->effects & WFX_FADEIN ) {
				if ( tmp > 0 ) {
					textColor[3] = (float)( (float)t_offset / (float)w->targetTime );
				} else { w->state = WSTATE_COMPLETE;}
			}
		} else if ( w->state == WSTATE_SHUTDOWN ) {
			tmp = w->targetTime - t_offset;
			if ( w->effects & WFX_SCROLLUP ) {
				if ( tmp > 0 ) {
					y = w->curY + ( 480 - w->y ) * t_offset / w->targetTime;
				}
				if ( tmp < 0 || y >= 480 ) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			}	
			// L0 - New effects
			if ( w->effects & WFX_SCROLLRIGHT ) {
				if ( tmp > 0 ) {
					x = w->curX + ( 680 - w->x ) * t_offset / w->targetTime;
				}
				if ( tmp < 0 || x >= 680 ) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			}
			if ( w->effects & WFX_SCROLLLEFT ) {
				if ( tmp > 0 ) {
					x = w->curX - ( 680 + w->x ) * t_offset / w->targetTime;
				}
				if ( tmp < 0 || x >= 680 ) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			} // End
			if ( w->effects & WFX_FADEIN ) {
				if ( tmp > 0 ) {
					textColor[3] -= (float)( (float)t_offset / (float)w->targetTime );
				} else {
					textColor[3] = 0.0f;
					w->state = WSTATE_OFF;
				}
			}
		}

		borderColor[3] *= textColor[3];
		bgColor[3] *= textColor[3];

		CG_FillRect( x, y, w->w, h, bgColor );
		CG_DrawRect( x, y, w->w, h, 1, borderColor );

		x += 5;
		y -= ( w->effects & WFX_TRUETYPE ) ? 3 : 0;

		for ( j = w->lineCount - 1; j >= 0; j-- ) {
			if ( w->effects & WFX_TRUETYPE ) {
				// L0 - If i'll ever port the font..this can be restored..
				//CG_Text_Paint_Ext( x, y + h, w->fontScaleX, w->fontScaleY, textColor,
				//				   (char*)w->lineText[j], 0.0f, 0, 0, &cgDC.Assets.textFont );

				// Note that size is fixed (for now)..
				CG_DrawStringExt(x, y - 7 + h, (char*)w->lineText[j], textColor, qfalse, qfalse,5, 10, 70);		
			}

			h -= ( w->lineHeight[j] + 3 );

			if ( !( w->effects & WFX_TRUETYPE ) ) {
				CG_DrawStringExt2( x, y + h, (char*)w->lineText[j], textColor,
								   qfalse, qtrue, w->fontWidth, w->fontHeight, 0 );
			}
		}
	}

	// Extra rate info
//	CG_demoAviFPSDraw();
//	CG_demoTimescaleDraw();
	CG_destroyDemoPopUpWindow();

	if ( fCleanup ) {
		CG_windowCleanup();
	}
}

// Set the window width and height based on the windows text/font parameters
void CG_windowNormalizeOnText( cg_window_t *w ) {
	int i, tmp;

	if ( w == NULL ) {
		return;
	}

	w->w = 0;
	w->h = 0;

	if ( !( w->effects & WFX_TRUETYPE ) ) {
		w->fontWidth = w->fontScaleX * WINDOW_FONTWIDTH;
		w->fontHeight = w->fontScaleY * WINDOW_FONTHEIGHT;
	}

	for ( i = 0; i < w->lineCount; i++ ) {
		if ( w->effects & WFX_TRUETYPE ) {
			//tmp = CG_Text_Width_Ext( (char*)w->lineText[i], w->fontScaleX, 0, &cgs.media.limboFont2 );			
			tmp = CG_Text_Width_Ext( (char*)w->lineText[i], w->fontScaleX, 0, &cgDC.Assets.textFont );
		} else {
			tmp = CG_DrawStrlen( (char*)w->lineText[i] ) * w->fontWidth;
		}

		if ( tmp > w->w ) {
			w->w = tmp;
		}
	}

	for ( i = 0; i < w->lineCount; i++ ) {
		if ( w->effects & WFX_TRUETYPE ) {				
			w->lineHeight[i] = CG_Text_Height_Ext( (char*)w->lineText[i], w->fontScaleY, 0, &cgDC.Assets.textFont );
		} else {
			w->lineHeight[i] = w->fontHeight;
		}

		w->h += w->lineHeight[i] + 3;
	}

	// Border + margins
	w->w += 10;
	w->h += 3;

	// Set up bottom alignment
	if ( w->x < 0 ) {
		w->x += 640 - w->w;
	}
	if ( w->y < 0 ) {
		w->y += 480 - w->h;
	}
}

void CG_printWindow( char *str ) {
	int pos = 0, pos2 = 0;
	char buf[MAX_STRING_CHARS];
	cg_window_t *w = cg.windowCurrent;

	if ( w == NULL ) {
		return;
	}

	// Silly logic for a strict format
	Q_strncpyz( buf, str, MAX_STRING_CHARS );
	while ( buf[pos] > 0 && w->lineCount < MAX_WINDOW_LINES ) {
		if ( buf[pos] == '\n' ) {
			if ( pos2 == pos ) {
				if ( !CG_addString( w, " " ) ) {
					return;
				}
			} else {
				buf[pos] = 0;
				if ( !CG_addString( w, buf + pos2 ) ) {
					return;
				}
			}
			pos2 = ++pos;
			continue;
		}
		pos++;
	}

	if ( pos2 < pos ) {
		CG_addString( w, buf + pos2 );
	}
}

//
// String buffer handling
//
void CG_initStrings( void ) {
	int i;

	for ( i = 0; i < MAX_STRINGS; i++ ) {
		cg.aStringPool[i].fActive = qfalse;
		cg.aStringPool[i].str[0] = 0;
	}
}

qboolean CG_addString( cg_window_t *w, char *buf ) {
	int i;

	// Check if we're reusing the current buf
	if ( w->lineText[w->lineCount] != NULL ) {
		for ( i = 0; i < MAX_STRINGS; i++ ) {
			if ( !cg.aStringPool[i].fActive ) {
				continue;
			}

			if ( w->lineText[w->lineCount] == (char *)&cg.aStringPool[i].str ) {
				w->lineCount++;
				cg.aStringPool[i].fActive = qtrue;
				strcpy( cg.aStringPool[i].str, buf );

				return( qtrue );
			}
		}
	}

	for ( i = 0; i < MAX_STRINGS; i++ ) {
		if ( !cg.aStringPool[i].fActive ) {
			cg.aStringPool[i].fActive = qtrue;
			strcpy( cg.aStringPool[i].str, buf );
			w->lineText[w->lineCount++] = (char *)&cg.aStringPool[i].str;

			return( qtrue );
		}
	}

	return( qfalse );
}

void CG_removeStrings( cg_window_t *w ) {
	int i, j;

	for ( i = 0; i < w->lineCount; i++ ) {
		char *ref = w->lineText[i];

		for ( j = 0; j < MAX_STRINGS; j++ ) {
			if ( !cg.aStringPool[j].fActive ) {
				continue;
			}

			if ( ref == (char *)&cg.aStringPool[j].str ) {
				w->lineText[i] = NULL;
				cg.aStringPool[j].fActive = qfalse;
				cg.aStringPool[j].str[0] = 0;

				break;
			}
		}
	}
}

