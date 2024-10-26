/*
===========================================================================
Parts taken from CNQ3:
Copyright (C) 2017-2019 Gian 'myT' Schellenbaum

This file is part of RtcwPro.

RtcwPro is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

RtcwPro is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RtcwPro. If not, see <https://www.gnu.org/licenses/>.
===========================================================================
*/

/*
** GLW_IMP.C
**
** This file contains ALL Linux specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
** GLimp_SetGamma
**
*/

#include <dlfcn.h>

#include "../renderer/tr_local.h"
#include "linux_local.h"

#include "unix_glw.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include "sdl_local.h"

#define WINDOW_CLASS_NAME   "Return to Castle Wolfenstein"

glImp_t glimp;
glwstate_t glw_state;


static qbool sdl_IsMonitorListValid()
{
	const int count = glimp.monitorCount;
	const int curr = glimp.monitor;

	return
		count >= 1 && count <= MAX_MONITOR_COUNT &&
		curr >= 0 && curr < count;
}

static int sdl_CompareMonitors( const void* aPtr, const void* bPtr )
{
	const SDL_Rect* const a = &((const monitor_t*)aPtr)->rect;
	const SDL_Rect* const b = &((const monitor_t*)bPtr)->rect;
	const int dy = a->y - b->y;
	if (dy != 0)
		return dy;

	return a->x - b->x;
}

static void sdl_CreateMonitorList()
{
	glimp.monitorCount = 0;

	const int count = SDL_GetNumVideoDisplays();
	if (count <= 0)
		return;

	int gi = 0;
	for (int si = 0; si < count; ++si) {
		if (gi >= MAX_MONITOR_COUNT)
			break;
		if (SDL_GetDisplayBounds(si, &glimp.monitors[gi].rect) == 0) {
			glimp.monitors[gi].sdlIndex = si;
			++gi;
		}
	}
	glimp.monitorCount = gi;

	if (sdl_IsMonitorListValid())
		qsort(glimp.monitors, (size_t)glimp.monitorCount, sizeof(glimp.monitors[0]), &sdl_CompareMonitors);
	else
		glimp.monitorCount = 0;
}

// call this before creating the window
static void sdl_UpdateMonitorIndexFromCvar()
{
	if (glimp.monitorCount <= 0 || glimp.monitorCount >= MAX_MONITOR_COUNT)
		return;

	const int monitor = Cvar_Get("r_monitor", "0", CVAR_ARCHIVE | CVAR_LATCH)->integer;
	if (monitor < 0 || monitor >= glimp.monitorCount) {
		glimp.monitor = 0;
		return;
	}
	glimp.monitor = monitor;
}

static void sdl_PrintMonitorList()
{
	const int count = glimp.monitorCount;
	if (count <= 0) {
		Com_Printf("No monitor detected.\n");
		return;
	}

	Com_Printf("Monitors detected (left is r_monitor ^7value):\n");
	for (int i = 0; i < count; ++i) {
		const SDL_Rect rect = glimp.monitors[i].rect;
		Com_Printf( "%d ^7%dx%d at %d,%d\n", i, rect.w, rect.h, rect.x, rect.y);
	}
}


static void sdl_MonitorList_f()
{
	sdl_CreateMonitorList();
	sdl_UpdateMonitorIndexFromCvar();
	sdl_PrintMonitorList();
}


// call this after the window has been moved
void sdl_UpdateMonitorIndexFromWindow()
{
	if (glimp.monitorCount <= 0)
		return;

	// try to find the glimp index and update data accordingly
	const int sdlIndex = SDL_GetWindowDisplayIndex(glimp.window);
	for (int i = 0; i < glimp.monitorCount; ++i) {
		if (glimp.monitors[i].sdlIndex == sdlIndex) {
			glimp.monitor = i;
			Cvar_Set("r_monitor", va("%d", i));
			break;
		}
	}
}


void sdl_Window( const SDL_WindowEvent* event )
{
	// events of interest:
	//SDL_WINDOWEVENT_SHOWN
	//SDL_WINDOWEVENT_HIDDEN
	//SDL_WINDOWEVENT_RESIZED
	//SDL_WINDOWEVENT_SIZE_CHANGED // should prevent this from happening except on creation?
	//SDL_WINDOWEVENT_MINIMIZED
	//SDL_WINDOWEVENT_MAXIMIZED
	//SDL_WINDOWEVENT_RESTORED
	//SDL_WINDOWEVENT_ENTER // mouse focus gained
	//SDL_WINDOWEVENT_LEAVE // mouse focus lost
	//SDL_WINDOWEVENT_FOCUS_GAINED // kb focus gained
	//SDL_WINDOWEVENT_FOCUS_LOST // kb focus lost
	//SDL_WINDOWEVENT_CLOSE
	//SDL_WINDOWEVENT_MOVED

	switch (event->event) {
		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_RESTORED:
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_MOVED:
			// if this turns out to be too expensive, track movement and
			// only call when movement stops
			sdl_UpdateMonitorIndexFromWindow();
			break;

		default:
			break;
	}

}


static void sdl_GetSafeDesktopRect( SDL_Rect* rect )
{
	if (!sdl_IsMonitorListValid()) {
		rect->x = 0;
		rect->y = 0;
		rect->w = 1280;
		rect->h = 720;
	}

	*rect = glimp.monitors[glimp.monitor].rect;
}

typedef enum
{
	RSERR_OK,
	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,
	RSERR_UNKNOWN
} rserr_t;

/*
** GLW_StartDriverAndSetMode
*/
// bk001204 - prototype needed
int GLW_SetMode( const char *drivername, int mode, qboolean fullscreen );
static qboolean GLW_StartDriverAndSetMode( const char *drivername,
										   int mode,
										   qboolean fullscreen ) {
	rserr_t err;
	err = GLW_SetMode( drivername, mode, fullscreen );

	switch ( err )
	{
	case RSERR_INVALID_FULLSCREEN:
		ri.Printf( PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n" );
		return qfalse;
	case RSERR_INVALID_MODE:
		ri.Printf( PRINT_ALL, "...WARNING: could not set the given mode (%d)\n", mode );
		return qfalse;
	default:
		break;
	}
	return qtrue;
}

/*
** GLW_SetMode
*/
int GLW_SetMode( const char *drivername, int mode, qboolean fullscreen ) {

	const char*   glstring; // bk001130 - from cvs1.17 (mkv)

	ri.Printf( PRINT_ALL, "Initializing OpenGL display\n" );

	ri.Printf( PRINT_ALL, "...setting mode %d:", mode );

	if ( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, mode, 0, 0 ) ) {
		ri.Printf( PRINT_ALL, " invalid mode\n" );
		return RSERR_INVALID_MODE;
	}
	ri.Printf( PRINT_ALL, " %d %d\n", glConfig.vidWidth, glConfig.vidHeight );

	// bk001130 - from cvs1.17 (mkv)
	glstring = qglGetString( GL_RENDERER );
	ri.Printf( PRINT_ALL, "GL_RENDERER: %s\n", glstring );
	return RSERR_OK;
}

/*
** GLW_LoadOpenGL
**
** GLimp_win.c internal function that that attempts to load and use
** a specific OpenGL DLL.
*/
static qboolean GLW_LoadOpenGL( const char *name ) {
	qboolean fullscreen;

	ri.Printf( PRINT_ALL, "...loading %s: ", name );

	// load the QGL layer
	if ( QGL_Init( name ) ) {
		fullscreen = r_fullscreen->integer;

		// create the window and set up the context
		if ( !GLW_StartDriverAndSetMode( name, r_mode->integer, fullscreen ) ) {
			if ( r_mode->integer != 3 ) {
				if ( !GLW_StartDriverAndSetMode( name, 3, fullscreen ) ) {
					goto fail;
				}
			} else {
				goto fail;
			}
		}

		return qtrue;
	} else
	{
		ri.Printf( PRINT_ALL, "failed\n" );
	}
fail:
	QGL_Shutdown();
	return qfalse;
}

/*
* Find the first occurrence of find in s.
*/
// bk001130 - from cvs1.17 (mkv), const
// bk001130 - made first argument const
static const char *Q_stristr( const char *s, const char *find ) {
	register char c, sc;
	register size_t len;

	if ( ( c = *find++ ) != 0 ) {
		if ( c >= 'a' && c <= 'z' ) {
			c -= ( 'a' - 'A' );
		}
		len = strlen( find );
		do
		{
			do
			{
				if ( ( sc = *s++ ) == 0 ) {
					return NULL;
				}
				if ( sc >= 'a' && sc <= 'z' ) {
					sc -= ( 'a' - 'A' );
				}
			} while ( sc != c );
		} while ( Q_stricmpn( s, find, len ) != 0 );
		s--;
	}
	return s;
}

/*
** GLW_InitExtensions
*/
static void GLW_InitExtensions( void ) {
	if ( !r_allowExtensions->integer ) {
		ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

	// GL_S3_s3tc
	if ( Q_stristr( glConfig.extensions_string, "GL_S3_s3tc" ) ) {
		if ( r_ext_compressed_textures->value ) {
			glConfig.textureCompression = TC_S3TC;
			ri.Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
		} else
		{
			glConfig.textureCompression = TC_NONE;
			ri.Printf( PRINT_ALL, "...ignoring GL_S3_s3tc\n" );
		}
	} else
	{
		glConfig.textureCompression = TC_NONE;
		ri.Printf( PRINT_ALL, "...GL_S3_s3tc not found\n" );
	}

	// GL_EXT_texture_env_add
	glConfig.textureEnvAddAvailable = qfalse;
	if ( Q_stristr( glConfig.extensions_string, "EXT_texture_env_add" ) ) {
		if ( r_ext_texture_env_add->integer ) {
			glConfig.textureEnvAddAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
		} else
		{
			glConfig.textureEnvAddAvailable = qfalse;
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
		}
	} else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
	}

	// GL_ARB_multitexture
	qglMultiTexCoord2fARB = NULL;
	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;
	if ( Q_stristr( glConfig.extensions_string, "GL_ARB_multitexture" ) ) {
		if ( r_ext_multitexture->value ) {
			qglMultiTexCoord2fARB = ( PFNGLMULTITEXCOORD2FARBPROC ) dlsym( glw_state.OpenGLLib, "glMultiTexCoord2fARB" );
			qglActiveTextureARB = ( PFNGLACTIVETEXTUREARBPROC ) dlsym( glw_state.OpenGLLib, "glActiveTextureARB" );
			qglClientActiveTextureARB = ( PFNGLCLIENTACTIVETEXTUREARBPROC ) dlsym( glw_state.OpenGLLib, "glClientActiveTextureARB" );

			if ( qglActiveTextureARB ) {
				qglGetIntegerv( GL_MAX_ACTIVE_TEXTURES_ARB, &glConfig.maxActiveTextures );

				if ( glConfig.maxActiveTextures > 1 ) {
					ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
				} else
				{
					qglMultiTexCoord2fARB = NULL;
					qglActiveTextureARB = NULL;
					qglClientActiveTextureARB = NULL;
					ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
				}
			}
		} else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
		}
	} else
	{
		ri.Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
	}

	// GL_EXT_compiled_vertex_array
	if ( Q_stristr( glConfig.extensions_string, "GL_EXT_compiled_vertex_array" ) ) {
		if ( r_ext_compiled_vertex_array->value ) {
			ri.Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
			qglLockArraysEXT = ( void ( APIENTRY * )( int, int ) )dlsym( glw_state.OpenGLLib, "glLockArraysEXT" );
			qglUnlockArraysEXT = ( void ( APIENTRY * )( void ) )dlsym( glw_state.OpenGLLib, "glUnlockArraysEXT" );
			if ( !qglLockArraysEXT || !qglUnlockArraysEXT ) {
				ri.Error( ERR_FATAL, "bad getprocaddress" );
			}
		} else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	} else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}

	// GL_NV_fog_distance
	if ( Q_stristr( glConfig.extensions_string, "GL_NV_fog_distance" ) ) {
		if ( r_ext_NV_fog_dist->integer ) {
			glConfig.NVFogAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_NV_fog_distance\n" );
		} else {
			ri.Printf( PRINT_ALL, "...ignoring GL_NV_fog_distance\n" );
			ri.Cvar_Set( "r_ext_NV_fog_dist", "0" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_NV_fog_distance not found\n" );
		ri.Cvar_Set( "r_ext_NV_fog_dist", "0" );
	}

}


/*
** GLimp_EndFrame
**
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame( void ) {
	if (r_swapInterval->modified) {
		r_swapInterval->modified = qfalse;
		SDL_GL_SetSwapInterval(r_swapInterval->integer);
	}

	SDL_GL_SwapWindow(glimp.window);

	// check logging
	QGL_EnableLogging( (qboolean)r_logFile->integer ); // bk001205 - was ->value
}

/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.
*/
void GLimp_Init( void ) {
	qbool attemptedlibGL = qfalse;
	qbool success = qfalse;
	qbool attempted3Dfx = qfalse;

	if (glimp.window != NULL)
		return;

	//Cvar_RegisterArray(glimp_cvars, MODULE_CLIENT);

	static qbool firstInit = qtrue;
	if (firstInit) {
		//Cmd_RegisterArray(glimp_cmds, MODULE_CLIENT);
		firstInit = qfalse;
	}

	sdl_CreateMonitorList();
	sdl_UpdateMonitorIndexFromCvar();
	sdl_PrintMonitorList();

	SDL_Rect deskropRect;
	sdl_GetSafeDesktopRect(&deskropRect);
	R_ConfigureVideoMode(deskropRect.w, deskropRect.h);

	Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	if (glInfo.winFullscreen) {
		if (glInfo.vidFullscreen)
			windowFlags |= SDL_WINDOW_FULLSCREEN;
		else
			windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	// SDL docs: "All three attributes must be set prior to creating the first window"
	//const int debugFlags = CL_GL_WantDebug() ? SDL_GL_CONTEXT_DEBUG_FLAG : 0;
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);

	// load and initialize the specific OpenGL driver
	//
	if ( !GLW_LoadOpenGL( r_glDriver->string ) ) {
		if ( !Q_stricmp( r_glDriver->string, OPENGL_DRIVER_NAME ) ) {
			attemptedlibGL = qtrue;
		} 

		if ( !attemptedlibGL && !success ) {
			attemptedlibGL = qtrue;
			if ( GLW_LoadOpenGL( OPENGL_DRIVER_NAME ) ) {
				ri.Cvar_Set( "r_glDriver", OPENGL_DRIVER_NAME );
				r_glDriver->modified = qfalse;
				success = qtrue;
			}
		}

		if ( !success ) {
			ri.Error( ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem\n" );
		}
	}

	glimp.window = SDL_CreateWindow(WINDOW_CLASS_NAME, deskropRect.x, deskropRect.y, glConfig.vidWidth, glConfig.vidHeight, windowFlags);
	if (glimp.window == NULL)
		ri.Error(ERR_FATAL, "SDL_CreateWindow failed: %s\n", SDL_GetError());

	glimp.glContext = SDL_GL_CreateContext(glimp.window);
	if (glimp.glContext == NULL)
		ri.Error(ERR_FATAL, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
	glConfig.colorBits = 32;
	glConfig.depthBits = 24;
	glConfig.stencilBits = 8;

	if (SDL_GL_MakeCurrent(glimp.window, glimp.glContext) < 0)
		ri.Error(ERR_FATAL, "SDL_GL_MakeCurrent failed: %s\n", SDL_GetError());


	// This values force the UI to disable driver selection
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;

	// get our config strings
	Q_strncpyz( glConfig.vendor_string, qglGetString( GL_VENDOR ), sizeof( glConfig.vendor_string ) );
	Q_strncpyz( glConfig.renderer_string, qglGetString( GL_RENDERER ), sizeof( glConfig.renderer_string ) );
	if ( *glConfig.renderer_string && glConfig.renderer_string[strlen( glConfig.renderer_string ) - 1] == '\n' ) {
		glConfig.renderer_string[strlen( glConfig.renderer_string ) - 1] = 0;
	}
	Q_strncpyz( glConfig.version_string, qglGetString( GL_VERSION ), sizeof( glConfig.version_string ) );
	Q_strncpyz( glConfig.extensions_string, qglGetString( GL_EXTENSIONS ), sizeof( glConfig.extensions_string ) );
	// TTimo - safe check
	if ( strlen( qglGetString( GL_EXTENSIONS ) ) >= sizeof( glConfig.extensions_string ) ) {
		Com_Printf( S_COLOR_YELLOW "WARNNING: GL extensions string too long (%d), truncated to %d\n", strlen( qglGetString( GL_EXTENSIONS ) ), sizeof( glConfig.extensions_string ) );
	}

	// initialize extensions
	GLW_InitExtensions();
	return;
}


/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void GLimp_Shutdown( void ) {
	if (glimp.glContext != NULL) {
		SDL_GL_DeleteContext(glimp.glContext);
		glimp.glContext = NULL;
	}

	if (glimp.window != NULL) {
		SDL_DestroyWindow(glimp.window);
		glimp.window = NULL;
	}

	SDL_GL_UnloadLibrary();


	QGL_Shutdown();
	memset( &glConfig, 0, sizeof( glConfig ) );
	memset( &glState, 0, sizeof( glState ) );
}

/*****************************************************************************/
/*
** GLimp_SetGamma
**
** This routine should only be called if glConfig.deviceSupportsGamma is TRUE
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] ) {
}

/*
** GLimp_LogComment
*/
void GLimp_LogComment( char *comment ) {
	if ( glw_state.log_fp ) {
		fprintf( glw_state.log_fp, "%s", comment );
	}
}

void GLimp_RenderThreadWrapper( void *stub ) {}
qboolean GLimp_SpawnRenderThread( void ( *function )( void ) ) {
	return qfalse;
}
void *GLimp_RendererSleep( void ) {
	return NULL;
}
void GLimp_FrontEndSleep( void ) {}
void GLimp_WakeRenderer( void *data ) {}