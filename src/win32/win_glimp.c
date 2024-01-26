/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of the Wolfenstein: Enemy Territory GPL Source Code (Wolf ET Source Code).

Wolf ET Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Wolf ET Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wolf ET Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolf: ET Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Wolf ET Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
/*
** WIN_GLIMP.C
**
** This file contains ALL Win32 specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_LogComment
** GLimp_Shutdown
**
** Note that the GLW_xxx functions are Windows specific GL-subsystem
** related functions that are relevant ONLY to win_glimp.c
*/

#include "../client/client.h"
#include "resource.h"
#include "win_local.h"
#include "glw_win.h"


typedef enum {
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;

#define TRY_PFD_SUCCESS		0
#define TRY_PFD_FAIL_SOFT	1
#define TRY_PFD_FAIL_HARD	2

#ifndef PFD_SUPPORT_COMPOSITION
#define PFD_SUPPORT_COMPOSITION 0x00008000
#endif

static DEVMODE dm_desktop;
static DEVMODE dm_current;

static rserr_t	GLW_SetMode(int mode, const char* modeFS, int colorbits, qboolean cdsFullscreen);

//
// function declaration
//
#ifdef USE_OPENGL_API
qboolean	QGL_Init(const char* dllname);
void		QGL_Shutdown(qboolean unloadDLL);
#endif

#ifdef USE_VULKAN_API
qboolean	RE_Init(void);
void		RE_Shutdown(qboolean unloadDLL);
#endif

//
// variable declarations
//
glwstate_t glw_state;

/*
** GLW_StartDriverAndSetMode
*/
static rserr_t GLW_StartDriverAndSetMode(int mode, const char* modeFS, int colorbits,
	qboolean cdsFullscreen)
{
	rserr_t err;

	err = GLW_SetMode(mode, modeFS, colorbits, cdsFullscreen);

	switch (err)
	{
	case RSERR_INVALID_FULLSCREEN:
		Com_Printf("...WARNING: fullscreen unavailable in this mode\n");
		return err;
	case RSERR_INVALID_MODE:
		Com_Printf("...WARNING: could not set the given mode (%d)\n", mode);
		return err;
	default:
		break;
	}
	return RSERR_OK;
}





/*
** GLW_CreateWindow
**
** Responsible for creating the Win32 window and initializing the OpenGL/Vulkan drivers.
*/
static qboolean GLW_CreateWindow(int width, int height, int colorbits, qboolean cdsFullscreen)
{
	static qboolean s_classRegistered = qfalse;
	RECT			r;
	int				stylebits;
	int				x, y, w, h;
	int				exstyle;
	qboolean		oldFullscreen;
	char windowTitle[sizeof(cl_title)  - 1 + 6] = { 0 };

	//
	// register the window class if necessary
	//
	if (!s_classRegistered)
	{
		WNDCLASS wc;

		memset(&wc, 0, sizeof(wc));

		wc.style = 0;
		wc.lpfnWndProc = (WNDPROC)MainWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = g_wv.hInstance;
		wc.hIcon = LoadIcon(g_wv.hInstance, MAKEINTRESOURCE(IDI_ICON1));
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(LRESULT)COLOR_GRAYTEXT;
		wc.lpszMenuName = 0;
		wc.lpszClassName = T(CLIENT_WINDOW_CLASS);

		if (!RegisterClass(&wc))
		{
			Com_Error(ERR_VID_FATAL, "%s: could not register window class", __func__);
			return qfalse;
		}
		s_classRegistered = qtrue;
		// Com_Printf( "...registered window class\n" );
	}

	r.left = vid_xpos->integer;
	r.top = vid_ypos->integer;
	r.right = r.left + width;
	r.bottom = r.top + height;

	UpdateMonitorInfo(&r);

	//
	// create the HWND if one does not already exist
	//
	if (!g_wv.hWnd)
	{
		//
		// compute width and height
		//
		//r.left = 0;
		//r.top = 0;
		//r.right  = width;
		//r.bottom = height;

		g_wv.borderless = 0;

		if (cdsFullscreen)
		{
			exstyle = WINDOW_ESTYLE_FULLSCREEN;
			stylebits = WINDOW_STYLE_FULLSCREEN;
		}
		else
		{
			exstyle = WINDOW_ESTYLE_NORMAL;
			if (r_noborder->integer) {
				stylebits = WINDOW_STYLE_NORMAL_NB;
				g_wv.borderless = r_noborder->integer;
			}
			else {
				stylebits = WINDOW_STYLE_NORMAL;
			}
			AdjustWindowRect(&r, stylebits, FALSE);
		}

		w = r.right - r.left;
		h = r.bottom - r.top;

		// select monitor from window rect
		r.left = vid_xpos->integer;
		r.top = vid_ypos->integer;
		r.right = r.left + w;
		r.bottom = r.top + h;
		UpdateMonitorInfo(&r);

		if (cdsFullscreen)
		{
			x = glw_state.desktopX;
			y = glw_state.desktopY;
		}
		else
		{
			x = vid_xpos->integer;
			y = vid_ypos->integer;

			// adjust window coordinates if necessary 
			// so that the window is completely on screen
			if (w < glw_state.desktopWidth && (x + w) > glw_state.desktopWidth + glw_state.desktopX)
				x = (glw_state.desktopWidth + glw_state.desktopX - w);
			if (h < glw_state.desktopHeight && (y + h) > glw_state.desktopHeight + glw_state.desktopY)
				y = (glw_state.desktopHeight + glw_state.desktopY - h);

			if (x < glw_state.desktopX)
				x = glw_state.desktopX;
			if (y < glw_state.desktopY)
				y = glw_state.desktopY;
		}

		stylebits &= ~WS_VISIBLE; // show window only after successive OpenGL/Vulkan initialization

		oldFullscreen = glw_state.cdsFullscreen;
		glw_state.cdsFullscreen = cdsFullscreen;

		Com_sprintf(windowTitle, sizeof(windowTitle), "%s ( %s )", cl_title, ARCH_STRING);

		g_wv.hWnd = CreateWindowEx(exstyle, TEXT(CLIENT_WINDOW_CLASS), AtoW(windowTitle),
			stylebits, x, y, w, h, NULL, NULL, g_wv.hInstance, NULL);

		if (!g_wv.hWnd)
		{
			glw_state.cdsFullscreen = oldFullscreen;
			Com_Error(ERR_VID_FATAL, "GLW_CreateWindow() - Couldn't create window");
			return qfalse;
		}

		// we must reflect actual drawable dimensions in glconfig
		GetClientRect(g_wv.hWnd, &r);
		glw_state.config->vidWidth = r.right - r.left;
		glw_state.config->vidHeight = r.bottom - r.top;

		Com_Printf("...created window@%d,%d (%dx%d)\n", x, y, w, h);
	}
	else
	{
		Com_Printf("...window already present, CreateWindowEx skipped\n");
	}

	if (colorbits == 0)
		colorbits = dm_desktop.dmBitsPerPel;


	int depthbits;
	int stencilbits;

	// implicitly assume Z-buffer depth == desktop color depth
	if (cl_depthbits->integer == 0) {
		if (colorbits > 16) {
			depthbits = 24;
		}
		else {
			depthbits = 16;
		}
	}
	else {
		depthbits = cl_depthbits->integer;
	}

	// do not allow stencil if Z-buffer depth likely won't contain it
	stencilbits = cl_stencilbits->integer;
	if (depthbits < 24) {
		stencilbits = 0;
	}

	glw_state.config->colorBits = colorbits;
	glw_state.config->depthBits = depthbits;
	glw_state.config->stencilBits = stencilbits;


	return qtrue;
}


static void PrintCDSError(int value)
{
	switch (value)
	{
	case DISP_CHANGE_RESTART:
		Com_Printf("restart required\n");
		break;
	case DISP_CHANGE_BADPARAM:
		Com_Printf("bad param\n");
		break;
	case DISP_CHANGE_BADFLAGS:
		Com_Printf("bad flags\n");
		break;
	case DISP_CHANGE_FAILED:
		Com_Printf("DISP_CHANGE_FAILED\n");
		break;
	case DISP_CHANGE_BADMODE:
		Com_Printf("bad mode\n");
		break;
	case DISP_CHANGE_NOTUPDATED:
		Com_Printf("not updated\n");
		break;
	default:
		Com_Printf("unknown error %d\n", value);
		break;
	}
}


static void ResetDisplaySettings(qboolean verbose)
{
	if (verbose)
		Com_Printf("...restoring display settings\n");

	if (glw_state.displayName[0])
		ChangeDisplaySettingsEx(glw_state.displayName, NULL, NULL, 0, NULL);
	else
		ChangeDisplaySettings(NULL, 0);
}


static LONG ApplyDisplaySettings(DEVMODE* dm)
{
	DEVMODE curr;
	LONG lResult;
	BOOL bResult;

	Com_Memset(&curr, 0, sizeof(curr));
	curr.dmSize = sizeof(DEVMODE);

	// Get current display mode on current monitor
	if (glw_state.displayName[0])
		bResult = EnumDisplaySettings(glw_state.displayName, ENUM_CURRENT_SETTINGS, &curr);
	else
		bResult = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &curr);

	if (!bResult)
		return DISP_CHANGE_FAILED;

#ifdef FAST_MODE_SWITCH
	// Check if current resolution is the same as we want to set
	if (curr.dmDisplayFrequency &&
		curr.dmPelsWidth == dm->dmPelsWidth &&
		curr.dmPelsHeight == dm->dmPelsHeight &&
		(curr.dmBitsPerPel == dm->dmBitsPerPel || dm->dmBitsPerPel == 0) &&
		(curr.dmDisplayFrequency == dm->dmDisplayFrequency || dm->dmDisplayFrequency == 0))
	{
		memcpy(&dm_current, &curr, sizeof(dm_current));
		return DISP_CHANGE_SUCCESSFUL; // simulate success
	}
#endif

	// Uninitialized?
	if (dm->dmDisplayFrequency == 0 && dm->dmPelsWidth == 0 &&
		dm->dmPelsHeight == 0 && dm->dmBitsPerPel == 0) {
		if (dm_desktop.dmPelsWidth && dm_desktop.dmPelsHeight) {
			return ApplyDisplaySettings(&dm_desktop);
		}
	}

	// Apply requested mode
	if (glw_state.displayName[0])
		lResult = ChangeDisplaySettingsEx(glw_state.displayName, dm, NULL, CDS_FULLSCREEN, NULL);
	else
		lResult = ChangeDisplaySettings(dm, 0);

	if (lResult == DISP_CHANGE_SUCCESSFUL)
		memcpy(&dm_current, dm, sizeof(dm_current));

	return lResult;
}


void SetGameDisplaySettings(void)
{
	ApplyDisplaySettings(&dm_current);
}


void SetDesktopDisplaySettings(void)
{
	ResetDisplaySettings(qfalse);

	memset(&dm_desktop, 0, sizeof(dm_desktop));
	dm_desktop.dmSize = sizeof(DEVMODE);

	if (glw_state.displayName[0])
		EnumDisplaySettings(glw_state.displayName, ENUM_CURRENT_SETTINGS, &dm_desktop);
	else
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm_desktop);
}


void UpdateMonitorInfo(const RECT* target)
{
	MONITORINFOEX mInfo;
	DEVMODE	devMode;
	HMONITOR hMon;
	const RECT* Rect;
	int w, h, x, y;

	glw_state.monitorCount = GetSystemMetrics(SM_CMONITORS);

	if (target)
		Rect = target;
	else if (g_wv.winRectValid)
		Rect = &g_wv.winRect;
	else
		Rect = &g_wv.conRect;

	// try to get more correct data
	hMon = MonitorFromRect(Rect, MONITOR_DEFAULTTONEAREST);
	memset(&mInfo, 0, sizeof(mInfo));
	mInfo.cbSize = sizeof(MONITORINFOEX);

	memset(&devMode, 0, sizeof(devMode));
	devMode.dmSize = sizeof(DEVMODE);

	if (GetMonitorInfo(hMon, (LPMONITORINFO)&mInfo) && EnumDisplaySettings(mInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode)) {
		w = mInfo.rcMonitor.right - mInfo.rcMonitor.left;
		h = mInfo.rcMonitor.bottom - mInfo.rcMonitor.top;
		x = mInfo.rcMonitor.left;
		y = mInfo.rcMonitor.top;

		// try to detect DPI scale
		// we can't properly handle it but at least detect monitor resolution 
		// and inform user in console
		if (devMode.dmPelsWidth > w || devMode.dmPelsHeight > h) {
			int scaleX, scaleY;
			scaleX = (devMode.dmPelsWidth * 100) / w;
			scaleY = (devMode.dmPelsHeight * 100) / h;
			if (scaleX == scaleY) {
				Com_Printf(S_COLOR_YELLOW "...detected DPI scale: %i%%\n", scaleX);
				w = devMode.dmPelsWidth;
				h = devMode.dmPelsHeight;
			}
		}

		if (glw_state.desktopWidth != w || glw_state.desktopHeight != h ||
			glw_state.desktopX != x || glw_state.desktopY != y ||
			glw_state.hMonitor != hMon) {
			// track monitor and gamma change
			qboolean gammaSet = glw_state.gammaSet;

			GLW_RestoreGamma();

			glw_state.desktopWidth = w;
			glw_state.desktopHeight = h;
			glw_state.desktopX = x;
			glw_state.desktopY = y;
			glw_state.hMonitor = hMon;
			memcpy(glw_state.displayName, mInfo.szDevice, sizeof(glw_state.displayName));

			glw_state.desktopBitsPixel = devMode.dmBitsPerPel;

			Com_Printf("...current monitor: %ix%i@%i,%i %s\n",
				w, h, x, y, WtoA(mInfo.szDevice));

			if (gammaSet && re.SetColorMappings) {
				re.SetColorMappings();
			}
		}

		glw_state.workArea = mInfo.rcWork;

	}
	else {
		// no information about current monitor, get desktop settings
		HDC hDC = GetDC(GetDesktopWindow());
		glw_state.desktopX = 0;
		glw_state.desktopY = 0;
		glw_state.desktopWidth = GetDeviceCaps(hDC, HORZRES);
		glw_state.desktopHeight = GetDeviceCaps(hDC, VERTRES);
		ReleaseDC(GetDesktopWindow(), hDC);

		glw_state.displayName[0] = '\0';

		SystemParametersInfo(SPI_GETWORKAREA, 0, &glw_state.workArea, 0);
	}
}


/*
** GLW_SetMode
*/
static rserr_t GLW_SetMode(int mode, const char* modeFS, int colorbits, qboolean cdsFullscreen)
{
	//HDC hDC;
	RECT r;
	const char* win_fs[] = { "W", "FS" };
	glconfig_t* config = glw_state.config;
	int		cdsRet;
	DEVMODE dm;

	r.left = vid_xpos->integer;
	r.top = vid_ypos->integer;
	r.right = r.left + 320;
	r.bottom = r.top + 240;

	UpdateMonitorInfo(&r);

	if (dm_desktop.dmSize == 0)
	{
		SetDesktopDisplaySettings();
	}

	//
	// print out informational messages
	//
	Com_Printf("...setting mode %d:", mode);
	if (!CL_GetModeInfo(&config->vidWidth, &config->vidHeight, &config->windowAspect,
		mode, modeFS, glw_state.desktopWidth, glw_state.desktopHeight, cdsFullscreen))
	{
		Com_Printf(" invalid mode\n");
		return RSERR_INVALID_MODE;
	}
	Com_Printf(" %d %d %s\n", config->vidWidth, config->vidHeight, win_fs[cdsFullscreen]);

	//
	// verify desktop bit depth
	//
	if (glw_state.desktopBitsPixel < 15 || glw_state.desktopBitsPixel == 24)
	{
		if (colorbits == 0 || (!cdsFullscreen && colorbits >= 15))
		{
			if (MessageBox(NULL,
				T("It is highly unlikely that a correct\n") \
				T("windowed display can be initialized with\n") \
				T("the current desktop display depth.  Select\n") \
				T("'OK' to try anyway.  Press 'Cancel' if you\n") \
				T("have a 3Dfx Voodoo, Voodoo-2, or Voodoo Rush\n") \
				T("3D accelerator installed, or if you otherwise\n") \
				T("wish to quit."), T("Low Desktop Color Depth"),
				MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
			{
				return RSERR_INVALID_MODE;
			}
		}
	}

	// do a CDS if needed
	if (cdsFullscreen)
	{
		memset(&dm, 0, sizeof(dm));

		dm.dmSize = sizeof(dm);

		dm.dmPelsWidth = config->vidWidth;
		dm.dmPelsHeight = config->vidHeight;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		if (Cvar_VariableIntegerValue("r_displayRefresh"))
		{
			dm.dmDisplayFrequency = Cvar_VariableIntegerValue("r_displayRefresh");
			dm.dmFields |= DM_DISPLAYFREQUENCY;
		}
		else // try to set at least desktop refresh rate?
			if ((dm_desktop.dmDisplayFrequency
				&& dm.dmPelsWidth <= dm_desktop.dmPelsWidth
				&& dm.dmPelsHeight <= dm_desktop.dmPelsWidth)
				|| (dm_current.dmDisplayFrequency
					&& dm.dmPelsWidth <= dm_current.dmPelsWidth
					&& dm.dmPelsHeight <= dm_current.dmPelsWidth)) {
				//dm.dmDisplayFrequency = dm_desktop.dmDisplayFrequency;
				//dm.dmFields |= DM_DISPLAYFREQUENCY;
				//Com_Printf("...using display refresh rate: %iHz\n", 
				//	dm_desktop.dmDisplayFrequency );
			}

		// try to change color depth if possible
		if (colorbits != 0)
		{
			dm.dmBitsPerPel = colorbits;
			dm.dmFields |= DM_BITSPERPEL;
			Com_Printf("...using colorsbits of %d\n", colorbits);
		}
		else
		{
			Com_Printf("...using desktop display depth of %d\n", glw_state.desktopBitsPixel);
		}

		//
		// if we're already in fullscreen then just create the window
		//
		if (glw_state.cdsFullscreen)
		{
			Com_Printf("...already fullscreen, avoiding redundant CDS\n");

			if (!GLW_CreateWindow(config->vidWidth, config->vidHeight, colorbits, qtrue))
			{
				ResetDisplaySettings(qtrue);
				glw_state.cdsFullscreen = qfalse;
				return RSERR_INVALID_MODE;
			}
		}
		//
		// need to call CDS
		//
		else
		{
			Com_Printf("...calling CDS: ");

			// try setting the exact mode requested, because some drivers don't report
			// the low res modes in EnumDisplaySettings, but still work
			if ((cdsRet = ApplyDisplaySettings(&dm)) == DISP_CHANGE_SUCCESSFUL)
			{
				Com_Printf("ok\n");

				if (!GLW_CreateWindow(config->vidWidth, config->vidHeight, colorbits, qtrue))
				{
					ResetDisplaySettings(qtrue);
					glw_state.cdsFullscreen = qfalse;
					return RSERR_INVALID_MODE;
				}
			}
			else
			{
				//
				// the exact mode failed, so scan EnumDisplaySettings for the next largest mode
				//
				DEVMODE		devmode;
				int			modeNum;

				Com_Printf("failed, ");

				PrintCDSError(cdsRet);

				Com_Printf("...trying next higher resolution:");

				// we could do a better matching job here...
				for (modeNum = 0; ; modeNum++) {
					BOOL bResult;

					Com_Memset(&devmode, 0, sizeof(devmode));
					devmode.dmSize = sizeof(DEVMODE);

					if (glw_state.displayName[0])
						bResult = EnumDisplaySettings(glw_state.displayName, modeNum, &devmode);
					else
						bResult = EnumDisplaySettings(NULL, modeNum, &devmode);

					if (!bResult) {
						modeNum = -1;
						break;
					}
					if (devmode.dmPelsWidth >= config->vidWidth
						&& devmode.dmPelsHeight >= config->vidHeight
						&& devmode.dmBitsPerPel >= 15) {
						break;
					}
				}

				if (modeNum != -1 && (cdsRet = ApplyDisplaySettings(&devmode)) == DISP_CHANGE_SUCCESSFUL)
				{
					Com_Printf(" ok\n");
					if (!GLW_CreateWindow(config->vidWidth, config->vidHeight, colorbits, qtrue))
					{
						ResetDisplaySettings(qtrue);
						glw_state.cdsFullscreen = qfalse;
						return RSERR_INVALID_MODE;
					}
				}
				else
				{
					Com_Printf(" failed, ");

					PrintCDSError(cdsRet);

					ResetDisplaySettings(qtrue);
					glw_state.cdsFullscreen = qfalse;
					glw_state.config->isFullscreen = qfalse;
					if (!GLW_CreateWindow(config->vidWidth, config->vidHeight, colorbits, qfalse))
					{
						return RSERR_INVALID_MODE;
					}
					return RSERR_INVALID_FULLSCREEN;
				}
			}
		}
	}
	else // !cdsFullscreen
	{
		if (glw_state.cdsFullscreen)
		{
			ResetDisplaySettings(qtrue);
			glw_state.cdsFullscreen = qfalse;
		}

		if (!GLW_CreateWindow(config->vidWidth, config->vidHeight, colorbits, qfalse))
		{
			return RSERR_INVALID_MODE;
		}
	}

	//
	// success, now check display frequency, although this won't be valid on Voodoo(2)
	//
	memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);
	if (EnumDisplaySettings(glw_state.displayName, ENUM_CURRENT_SETTINGS, &dm))
	{
		glw_state.config->displayFrequency = dm.dmDisplayFrequency;
	}

	// NOTE: this is overridden later on standalone 3Dfx drivers
	glw_state.config->isFullscreen = cdsFullscreen;
	glw_state.config->colorBits = dm.dmBitsPerPel;

	return RSERR_OK;
}


static void GLimp_DetectSteamOverlay(void) {
	HMODULE gameoverlaydll = GetModuleHandle(T("GameOverlayRenderer.dll"));

	if (!gameoverlaydll) {
		memset(&glw_state.overlay, 0, sizeof(glw_state.overlay));
		return;
	}

	glw_state.overlay.isattached = qtrue;
	glw_state.overlay.handle = gameoverlaydll;

	Com_Printf(S_COLOR_CYAN "Steam Overlay Detected\n");
}


#ifdef USE_OPENGL_API
#define QGL_List_PROCS \
	GLE( GLuint, glGenLists, GLsizei range ) \
	GLE( void, glDeleteLists, GLuint list, GLsizei range )

#define GLE( ret, name, ... ) ret ( APIENTRY * q##name )( __VA_ARGS__ );
QGL_List_PROCS;
#undef GLE


/*
** GLW_GenDefaultLists
*/
static void GLW_GenDefaultLists(void) {
	HFONT hfont, oldhfont;

	// keep going, we'll probably just leak some stuff
	if (fontbase_init) {
		Com_DPrintf("ERROR: GLW_GenDefaultLists: font base is already marked initialized\n");
	}

	// create font display lists
	gl_NormalFontBase = qglGenLists(256);

	if (gl_NormalFontBase == 0) {
		Com_Printf("ERROR: couldn't create font (glGenLists)\n");
		return;
	}

	hfont = CreateFont(
		12, // logical height of font
		6,  // logical average character width
		0,  // angle of escapement
		0,  // base-line orientation angle
		0,  // font weight
		0,  // italic attribute flag
		0,  // underline attribute flag
		0,  // strikeout attribute flag
		0,  // character set identifier
		0,  // output precision
		0,  // clipping precision
		0,  // output quality
		0,  // pitch and family
		""); // pointer to typeface name string

	if (!hfont) {
		Com_Printf("ERROR: couldn't create font (CreateFont)\n");
		return;
	}

	oldhfont = SelectObject(glw_state.hDC, hfont);
	qwglUseFontBitmaps(glw_state.hDC, 0, 255, gl_NormalFontBase);

	SelectObject(glw_state.hDC, oldhfont);
	DeleteObject(hfont);

	fontbase_init = qtrue;
}

/*
** GLW_DeleteDefaultLists
*/
static void GLW_DeleteDefaultLists(void) {
	if (!fontbase_init) {
		Com_DPrintf("ERROR: GLW_DeleteDefaultLists: no font list initialized\n");
		return;
	}

	qglDeleteLists(gl_NormalFontBase, 256);
	fontbase_init = qfalse;
}


/*
** GLW_LoadOpenGL
**
** GLimp_win.c internal function that attempts to load and use
** a specific OpenGL DLL.
*/
static qboolean GLW_LoadOpenGL(const char* drivername)
{
	char buffer[256];
	qboolean cdsFullscreen;

	glconfig_t* config = glw_state.config;

	Q_strncpyz(buffer, drivername, sizeof(buffer));
	Q_strlwr(buffer);

	if (Q_stricmp(buffer, OPENGL_DRIVER_NAME) == 0 || r_maskMinidriver->integer)
	{
		config->driverType = GLDRV_ICD;
	}
	else
	{
		config->driverType = GLDRV_STANDALONE;
		Com_Printf("...assuming '%s' is a standalone driver\n", drivername);
	}

	//
	// load the driver and bind our function pointers to it
	// 
	if (QGL_Init(buffer))
	{
		cdsFullscreen = (r_fullscreen->integer != 0);

		// create the window and set up the context
		if (GLW_StartDriverAndSetMode(r_mode->integer, r_modeFullscreen->string, r_colorbits->integer, cdsFullscreen, qfalse) != RSERR_OK)
		{
			// if we're on a 24/32-bit desktop try it again but with a 16-bit desktop
			if (r_colorbits->integer != 16 || cdsFullscreen != qtrue || r_mode->integer != 3)
			{
				if (GLW_StartDriverAndSetMode(3, "", 16, qtrue, qfalse) != RSERR_OK)
				{
					goto fail;
				}
			}
		}
		return qtrue;
	}
fail:

	QGL_Shutdown(qtrue);

	return qfalse;
}


static void GLimp_SwapBuffers(void)
{
	if (!SwapBuffers(glw_state.hDC))
	{
		Com_Error(ERR_VID_FATAL, "GLimp_EndFrame() - SwapBuffers() failed!");
	}
}


/*
** GLimp_EndFrame
*/
void GLimp_EndFrame(void)
{
	//
	// swapinterval stuff
	//
	if (r_swapInterval->modified) {
		r_swapInterval->modified = qfalse;

		//if ( !glConfig.stereoEnabled ) {	// why?
		if (qwglSwapIntervalEXT) {
			qwglSwapIntervalEXT(r_swapInterval->integer);
		}
		//}
	}

	// don't flip if drawing to front buffer
	if (Q_stricmp(cl_drawBuffer->string, "GL_FRONT") != 0) {
		GLimp_SwapBuffers();
	}
}


int GLimp_NormalFontBase(void) {
	return gl_NormalFontBase;
}


static qboolean GLW_StartOpenGL(void)
{
	//
	// load and initialize the specific OpenGL driver
	//
	if (!GLW_LoadOpenGL(r_glDriver->string))
	{
		if (Q_stricmp(r_glDriver->string, OPENGL_DRIVER_NAME) != 0)
		{
			// try default driver
			if (GLW_LoadOpenGL(OPENGL_DRIVER_NAME))
			{
				Cvar_Set("r_glDriver", OPENGL_DRIVER_NAME);
				r_glDriver->modified = qfalse;
				return qtrue;
			}
		}

		Com_Error(ERR_VID_FATAL, "GLW_StartOpenGL() - could not load OpenGL subsystem");
		return qfalse;
	}

	return qtrue;
}


/*
** GLimp_Init
**
** This is the platform specific OpenGL initialization function.  It
** is responsible for loading OpenGL, initializing it, setting
** extensions, creating a window of the appropriate size, doing
** fullscreen manipulations, etc.  Its overall responsibility is
** to make sure that a functional OpenGL subsystem is operating
** when it returns to the ref.
*/
void GLimp_Init(glconfig_t* config)
{
	Com_Printf("Initializing OpenGL subsystem\n");

	// glimp-specific

	r_maskMinidriver = Cvar_Get("r_maskMinidriver", "0", CVAR_LATCH);
	Cvar_SetDescription(r_maskMinidriver, "If set to 1, then a mini driver will be treated as a normal ICD");
	r_stereoEnabled = Cvar_Get("r_stereoEnabled", "0", CVAR_ARCHIVE_ND | CVAR_LATCH);
	Cvar_SetDescription(r_stereoEnabled, "Enable stereo rendering for techniques like shutter glasses");
	r_verbose = Cvar_Get("r_verbose", "0", 0);
	Cvar_SetDescription(r_verbose, "Turns on additional startup information when renderer is starting up");

	// feedback to renderer configuration
	glw_state.config = config;

	// load appropriate DLL and initialize subsystem
	if (!GLW_StartOpenGL())
		return;

	GLimp_DetectSteamOverlay();

	//glConfig.driverType = GLDRV_ICD;
	config->hardwareType = GLHW_GENERIC;

	// optional
#define GLE( ret, name, ... ) q##name = GL_GetProcAddress( XSTRING( name ) )
	QGL_Swp_PROCS;
#undef GLE

	if (qwglSwapIntervalEXT) {
		Com_Printf("...using WGL_EXT_swap_control\n");
		r_swapInterval->modified = qtrue; // force a set next frame
	}
	else {
		Com_Printf("...WGL_EXT_swap_control not found\n");
	}

#define GLE( ret, name, ... ) q##name = GL_GetProcAddress( XSTRING( name ) ); if ( !q##name ) Com_Error( ERR_VID_FATAL, "Error resolving core OpenGL functions" );
	QGL_List_PROCS;
#undef GLE

	// initialise default lists
	GLW_GenDefaultLists();

	// show main window after all initializations
	ShowWindow(g_wv.hWnd, SW_SHOW);

	IN_Init();

	HandleEvents();

	Key_ClearStates();
}


/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.
*/
void GLimp_Shutdown(qboolean unloadDLL)
{
	const char* success[] = { "failed", "success" };
	int retVal;

	// FIXME: Brian, we need better fallbacks from partially initialized failures
	if (!qwglMakeCurrent) {
		return;
	}

	IN_Shutdown();

	Com_Printf("Shutting down OpenGL subsystem\n");

	// restore gamma.  We do this first because 3Dfx's extension needs a valid OGL subsystem
	GLW_RestoreGamma();

	// delete display lists
	GLW_DeleteDefaultLists();

#define GLE( ret, name, ... ) q##name = NULL;
	QGL_List_PROCS;
#undef GLE

	// set current context to NULL
	if (qwglMakeCurrent)
	{
		retVal = qwglMakeCurrent(NULL, NULL) != 0;

		Com_Printf("...wglMakeCurrent( NULL, NULL ): %s\n", success[retVal]);
	}

	// delete HGLRC
	if (glw_state.hGLRC)
	{
		retVal = qwglDeleteContext(glw_state.hGLRC) != 0;
		Com_Printf("...deleting GL context: %s\n", success[retVal]);
		glw_state.hGLRC = NULL;
	}

	// release DC
	if (glw_state.hDC)
	{
		retVal = ReleaseDC(g_wv.hWnd, glw_state.hDC) != 0;
		Com_Printf("...releasing DC: %s\n", success[retVal]);
		glw_state.hDC = NULL;
	}

	// destroy window
	if (g_wv.hWnd)
	{
		Com_Printf("...destroying window\n");
		//ShowWindow( g_wv.hWnd, SW_HIDE );
		DestroyWindow(g_wv.hWnd);
		g_wv.hWnd = NULL;
		glw_state.pixelFormatSet = qfalse;
	}

	// reset display settings
	if (glw_state.cdsFullscreen)
	{
		ResetDisplaySettings(qtrue);
		glw_state.cdsFullscreen = qfalse;
	}

	// shutdown QGL subsystem
	QGL_Shutdown(unloadDLL);
}
#endif // USE_OPENGL_API

static qboolean GLW_LoadAPI(void)
{
	//
	// load the driver and bind our function pointers to it
	//
	if (RE_Init())
	{
		qboolean cdsFullscreen = (r_fullscreen->integer != 0);

		// create the window and set up the context
		if (GLW_StartDriverAndSetMode(r_mode->integer, r_modeFullscreen->string, r_colorbits->integer, cdsFullscreen) == RSERR_OK)
			return qtrue;
	}

	RE_Shutdown(qtrue);

	return qfalse;
}


static qboolean GLW_StartAPI(void)
{
	//
	// load and initialize Vulkan driver
	//
	if (!GLW_LoadAPI()) {
		Com_Error(ERR_VID_FATAL, "GLW_StartAPI() - could not load Vulkan subsystem");
		return qfalse;
	}

	return qtrue;
}


/*
** GL_API_Init
**
** This is the platform specific Vulkan initialization function.  It
** is responsible for loading Vulkan, initializing it, setting
** extensions, creating a window of the appropriate size, doing
** fullscreen manipulations, etc.  Its overall responsibility is
** to make sure that a functional Vulkan subsystem is operating
** when it returns to the ref.
*/
void GL_API_Init(glconfig_t* config)
{
	Com_Printf("Initializing Vulkan subsystem\n");

	// feedback to renderer configuration
	glw_state.config = config;

	// load appropriate DLL and initialize subsystem
	if (!GLW_StartAPI())
		return;

	GLimp_DetectSteamOverlay();

	config->driverType = GLDRV_ICD;
	config->hardwareType = GLHW_GENERIC;

	// show main window after all initializations
	ShowWindow(g_wv.hWnd, SW_SHOW);

	IN_Init();

	HandleEvents();

	Key_ClearStates();
}


/*
** GL_API_Shutdown
**
** This routine does all OS specific shutdown procedures for the Vulkan
** subsystem.
*/
void GL_API_Shutdown(qboolean unloadDLL)
{
	IN_Shutdown();

	Com_Printf("Shutting down Vulkan subsystem\n");

	// restore gamma
	GLW_RestoreGamma();

	// destroy window
	if (g_wv.hWnd)
	{
		Com_Printf("...destroying window\n");
		//ShowWindow( g_wv.hWnd, SW_HIDE );
		DestroyWindow(g_wv.hWnd);
		g_wv.hWnd = NULL;
	}

	// reset display settings
	if (glw_state.cdsFullscreen)
	{
		ResetDisplaySettings(qtrue);
		glw_state.cdsFullscreen = qfalse;
	}

	// shutdown QVK subsystem
	RE_Shutdown(unloadDLL);
}