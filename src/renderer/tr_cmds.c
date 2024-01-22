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
#include "tr_local.h"

/*
=====================
R_PerformanceCounters
=====================
*/
static void R_PerformanceCounters( void ) {
	if ( !r_speeds->integer ) {
		// clear the counters even if we aren't printing
		Com_Memset( &tr.pc, 0, sizeof( tr.pc ) );
		Com_Memset( &backEnd.pc, 0, sizeof( backEnd.pc ) );
		return;
	}

	/*if ( r_speeds->integer )*/ { //%	== 1)
		ri.Printf( PRINT_ALL, "%i/%i shaders/surfs %i leafs %i verts %i/%i tris %.2f mtex %.2f dc\n",
				   backEnd.pc.c_shaders, backEnd.pc.c_surfaces, tr.pc.c_leafs, backEnd.pc.c_vertexes,
				   backEnd.pc.c_indexes / 3, backEnd.pc.c_totalIndexes / 3,
				   R_SumOfUsedImages() / ( 1000000.0f ), backEnd.pc.c_overDraw / (float)( glConfig.vidWidth * glConfig.vidHeight ) );
	}

	if ( r_speeds->integer == 2 ) {
		ri.Printf( PRINT_ALL, "(patch) %i sin %i sclip  %i sout %i bin %i bclip %i bout\n",
				   tr.pc.c_sphere_cull_patch_in, tr.pc.c_sphere_cull_patch_clip, tr.pc.c_sphere_cull_patch_out,
				   tr.pc.c_box_cull_patch_in, tr.pc.c_box_cull_patch_clip, tr.pc.c_box_cull_patch_out );
		ri.Printf( PRINT_ALL, "(md3) %i sin %i sclip  %i sout %i bin %i bclip %i bout\n",
				   tr.pc.c_sphere_cull_md3_in, tr.pc.c_sphere_cull_md3_clip, tr.pc.c_sphere_cull_md3_out,
				   tr.pc.c_box_cull_md3_in, tr.pc.c_box_cull_md3_clip, tr.pc.c_box_cull_md3_out );
		ri.Printf( PRINT_ALL, "(gen) %i sin %i sout %i pin %i pout\n",
				   tr.pc.c_sphere_cull_in, tr.pc.c_sphere_cull_out,
				   tr.pc.c_plane_cull_in, tr.pc.c_plane_cull_out );
	} else if ( r_speeds->integer == 3 )     {
		ri.Printf( PRINT_ALL, "viewcluster: %i\n", tr.viewCluster );
	} else if ( r_speeds->integer == 4 )     {
		if ( backEnd.pc.c_dlightVertexes ) {
			ri.Printf( PRINT_ALL, "dlight srf:%i  culled:%i  verts:%i  tris:%i\n",
					   tr.pc.c_dlightSurfaces, tr.pc.c_dlightSurfacesCulled,
					   backEnd.pc.c_dlightVertexes, backEnd.pc.c_dlightIndexes / 3 );
		}
	}
	else if (r_speeds->integer == 5 )
	{
		ri.Printf( PRINT_ALL, "zFar: %.0f\n", tr.viewParms.zFar );
	}
	else if ( r_speeds->integer == 6 )
	{
		ri.Printf( PRINT_ALL, "flare adds:%i tests:%i renders:%i\n",
				   backEnd.pc.c_flareAdds, backEnd.pc.c_flareTests, backEnd.pc.c_flareRenders );
	} else if ( r_speeds->integer == 7 )    {
		ri.Printf( PRINT_ALL, "decal projectors: %d test surfs: %d clip surfs: %d decal surfs: %d created: %d\n",
				   tr.pc.c_decalProjectors, tr.pc.c_decalTestSurfaces, tr.pc.c_decalClipSurfaces, tr.pc.c_decalSurfaces, tr.pc.c_decalSurfacesCreated );
	}

	memset( &tr.pc, 0, sizeof( tr.pc ) );
	memset( &backEnd.pc, 0, sizeof( backEnd.pc ) );
}


/*
====================
R_IssueRenderCommands
====================
*/
static void R_IssueRenderCommands( void ) {
	renderCommandList_t	*cmdList;

	cmdList = &backEndData->commands;

	// add an end-of-list command
	*(int *)(cmdList->cmds + cmdList->used) = RC_END_OF_LIST;

	// clear it out, in case this is a sync and not a buffer flip
	cmdList->used = 0;

	if ( backEnd.screenshotMask == 0 ) {
		if ( ri.CL_IsMinimized() )
			return; // skip backend when minimized
		if ( backEnd.throttle )
			return; // or throttled on demand
	}

	// actually start the commands going
	if ( !r_skipBackEnd->integer ) {
		// let it start on the new batch
		RB_ExecuteRenderCommands( cmdList->cmds );
	}
}


/*
====================
R_IssuePendingRenderCommands

Issue any pending commands and wait for them to complete.
====================
*/
void R_IssuePendingRenderCommands( void ) {
	if ( !tr.registered ) {
		return;
	}
	R_IssueRenderCommands();
}


/*
============
R_GetCommandBufferReserved

make sure there is enough command space
============
*/
static void *R_GetCommandBufferReserved( int bytes, int reservedBytes ) {
	renderCommandList_t	*cmdList;

	cmdList = &backEndData->commands;
	bytes = PAD(bytes, sizeof(void *));

	// always leave room for the end of list command
	if ( cmdList->used + bytes + sizeof( int ) + reservedBytes > MAX_RENDER_COMMANDS ) {
		if ( bytes > MAX_RENDER_COMMANDS - sizeof( int ) ) {
			ri.Error( ERR_FATAL, "R_GetCommandBuffer: bad size %i", bytes );
		}
		// if we run out of room, just start dropping commands
		return NULL;
	}

	cmdList->used += bytes;

	return cmdList->cmds + cmdList->used - bytes;
}


/*
=============
R_GetCommandBuffer
returns NULL if there is not enough space for important commands
=============
*/
static void *R_GetCommandBuffer( int bytes ) {
	return R_GetCommandBufferReserved( bytes, PAD( sizeof( swapBuffersCommand_t ), sizeof(void *) ) );
}


/*
=============
R_AddDrawSurfCmd
=============
*/
void R_AddDrawSurfCmd( drawSurf_t *drawSurfs, int numDrawSurfs ) {
	drawSurfsCommand_t	*cmd;

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_DRAW_SURFS;

	cmd->drawSurfs = drawSurfs;
	cmd->numDrawSurfs = numDrawSurfs;

	cmd->refdef = tr.refdef;
	cmd->viewParms = tr.viewParms;
}


/*
=============
RE_SetColor

Passing NULL will set the color to white
=============
*/
void RE_SetColor( const float *rgba ) {
	setColorCommand_t	*cmd;

	if ( !tr.registered ) {
		return;
	}
	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SET_COLOR;
	if ( !rgba ) {
		rgba = colorWhite;
	}

	cmd->color[0] = rgba[0];
	cmd->color[1] = rgba[1];
	cmd->color[2] = rgba[2];
	cmd->color[3] = rgba[3];
}


/*
=============
RE_StretchPic
=============
*/
void RE_StretchPic( float x, float y, float w, float h,
					float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	stretchPicCommand_t	*cmd;

	if ( !tr.registered ) {
		return;
	}
	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_STRETCH_PIC;
	cmd->shader = R_GetShaderByHandle( hShader );
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
	cmd->s1 = s1;
	cmd->t1 = t1;
	cmd->s2 = s2;
	cmd->t2 = t2;
}

#define MODE_RED_CYAN	1
#define MODE_RED_BLUE	2
#define MODE_RED_GREEN	3
#define MODE_GREEN_MAGENTA 4
#define MODE_MAX	MODE_GREEN_MAGENTA

static void R_SetColorMode(GLboolean *rgba, stereoFrame_t stereoFrame, int colormode)
{
	rgba[0] = rgba[1] = rgba[2] = rgba[3] = GL_TRUE;

	if(colormode > MODE_MAX)
	{
		if(stereoFrame == STEREO_LEFT)
			stereoFrame = STEREO_RIGHT;
		else if(stereoFrame == STEREO_RIGHT)
			stereoFrame = STEREO_LEFT;

		colormode -= MODE_MAX;
	}

	if(colormode == MODE_GREEN_MAGENTA)
	{
		if(stereoFrame == STEREO_LEFT)
			rgba[0] = rgba[2] = GL_FALSE;
		else if(stereoFrame == STEREO_RIGHT)
			rgba[1] = GL_FALSE;
	}
	else
	{
		if(stereoFrame == STEREO_LEFT)
			rgba[1] = rgba[2] = GL_FALSE;
		else if(stereoFrame == STEREO_RIGHT)
		{
			rgba[0] = GL_FALSE;

			if(colormode == MODE_RED_BLUE)
				rgba[1] = GL_FALSE;
			else if(colormode == MODE_RED_GREEN)
				rgba[2] = GL_FALSE;
		}
	}
}

/*
=============
RE_2DPolyies
=============
*/
extern int r_numpolyverts;

void RE_2DPolyies( polyVert_t* verts, int numverts, qhandle_t hShader ) {
	poly2dCommand_t* cmd;

	if (!tr.registered) {
		return;
	}

	if ( r_numpolyverts + numverts > max_polyverts ) {
		return;
	}

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}

	cmd->commandId =    RC_2DPOLYS;
	cmd->verts =        &backEndData->polyVerts[r_numpolyverts];
	cmd->numverts =     numverts;
	memcpy( cmd->verts, verts, sizeof( polyVert_t ) * numverts );
	cmd->shader =       R_GetShaderByHandle( hShader );

	r_numpolyverts += numverts;
}


/*
=============
RE_RotatedPic
=============
*/
void RE_RotatedPic( float x, float y, float w, float h,
					float s1, float t1, float s2, float t2, qhandle_t hShader, float angle ) {
	stretchPicCommand_t *cmd;

	if (!tr.registered) {
		return;
	}

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_ROTATED_PIC;
	cmd->shader = R_GetShaderByHandle( hShader );
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;

	// fixup
	cmd->w /= 2;
	cmd->h /= 2;
	cmd->x += cmd->w;
	cmd->y += cmd->h;
	cmd->w = sqrt( ( cmd->w * cmd->w ) + ( cmd->h * cmd->h ) );
	cmd->h = cmd->w;

	cmd->angle = angle;
	cmd->s1 = s1;
	cmd->t1 = t1;
	cmd->s2 = s2;
	cmd->t2 = t2;
}

//----(SA)	added
/*
==============
RE_StretchPicGradient
==============
*/
void RE_StretchPicGradient( float x, float y, float w, float h,
							float s1, float t1, float s2, float t2, qhandle_t hShader, const float *gradientColor, int gradientType ) {
	stretchPicCommand_t *cmd;

	if (!tr.registered) {
		return;
	}

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_STRETCH_PIC_GRADIENT;
	cmd->shader = R_GetShaderByHandle( hShader );
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
	cmd->s1 = s1;
	cmd->t1 = t1;
	cmd->s2 = s2;
	cmd->t2 = t2;

	if ( !gradientColor ) {
		gradientColor = colorWhite;
	}

	cmd->gradientColor[0] = gradientColor[0] * 255;
	cmd->gradientColor[1] = gradientColor[1] * 255;
	cmd->gradientColor[2] = gradientColor[2] * 255;
	cmd->gradientColor[3] = gradientColor[3] * 255;
	cmd->gradientType = gradientType;
}
//----(SA)	end

/*
====================
RE_SetGlobalFog
	rgb = colour
	depthForOpaque is depth for opaque

	the restore flag can be used to restore the original level fog
	duration can be set to fade over a certain period
====================
*/
void RE_SetGlobalFog( qboolean restore, int duration, float r, float g, float b, float depthForOpaque ) {
	if ( restore ) {
		if ( duration > 0 ) {
			VectorCopy( tr.world->fogs[tr.world->globalFog].shader->fogParms.color, tr.world->globalTransStartFog );
			tr.world->globalTransStartFog[ 3 ] = tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque;

			Vector4Copy( tr.world->globalOriginalFog, tr.world->globalTransEndFog );

			tr.world->globalFogTransStartTime = tr.refdef.time;
			tr.world->globalFogTransEndTime = tr.refdef.time + duration;
		} else {
			VectorCopy( tr.world->globalOriginalFog, tr.world->fogs[tr.world->globalFog].shader->fogParms.color );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.colorInt = ColorBytes4( tr.world->globalOriginalFog[ 0 ] * tr.identityLight,
																						 tr.world->globalOriginalFog[ 1 ] * tr.identityLight,
																						 tr.world->globalOriginalFog[ 2 ] * tr.identityLight, 1.0 );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque = tr.world->globalOriginalFog[ 3 ];
			tr.world->fogs[tr.world->globalFog].shader->fogParms.tcScale = 1.0f / ( tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque );
		}
	} else {
		if ( duration > 0 ) {
			VectorCopy( tr.world->fogs[tr.world->globalFog].shader->fogParms.color, tr.world->globalTransStartFog );
			tr.world->globalTransStartFog[ 3 ] = tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque;

			VectorSet( tr.world->globalTransEndFog, r, g, b );
			tr.world->globalTransEndFog[ 3 ] = depthForOpaque < 1 ? 1 : depthForOpaque;

			tr.world->globalFogTransStartTime = tr.refdef.time;
			tr.world->globalFogTransEndTime = tr.refdef.time + duration;
		} else {
			VectorSet( tr.world->fogs[tr.world->globalFog].shader->fogParms.color, r, g, b );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.colorInt = ColorBytes4( r * tr.identityLight,
																						 g * tr.identityLight,
																						 b * tr.identityLight, 1.0 );
			tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque = depthForOpaque < 1 ? 1 : depthForOpaque;
			tr.world->fogs[tr.world->globalFog].shader->fogParms.tcScale = 1.0f / ( tr.world->fogs[tr.world->globalFog].shader->fogParms.depthForOpaque );
		}
	}
}

/*
====================
RE_BeginFrame

If running in stereo, RE_BeginFrame will be called twice
for each RE_EndFrame
====================
*/
void RE_BeginFrame( stereoFrame_t stereoFrame ) {
	drawBufferCommand_t	*cmd = NULL;
	colorMaskCommand_t *colcmd = NULL;
	clearColorCommand_t *clrcmd = NULL;

	if ( !tr.registered ) {
		return;
	}

	glState.finishCalled = qfalse;

	tr.frameCount++;
	tr.frameSceneNum = 0;

	backEnd.doneBloom = qfalse;

	Byte4Set(backEnd.color2D, 0xFF, 0xFF, 0xFF, 0xFF);

	// check for errors
	GL_CheckErrors();

	if ( ( cmd = R_GetCommandBuffer( sizeof( *cmd ) ) ) == NULL )
		return;
	cmd->commandId = RC_DRAW_BUFFER;

	if ( glConfig.stereoEnabled ) {
		if ( stereoFrame == STEREO_LEFT ) {
			cmd->buffer = (int)GL_BACK_LEFT;
		} else if ( stereoFrame == STEREO_RIGHT ) {
			cmd->buffer = (int)GL_BACK_RIGHT;
		} else {
			ri.Error( ERR_FATAL, "RE_BeginFrame: Stereo is enabled, but stereoFrame was %i", stereoFrame );
		}
	}
	else
	{
		if ( !Q_stricmp( r_drawBuffer->string, "GL_FRONT" ) )
			cmd->buffer = (int)GL_FRONT;
		else
			cmd->buffer = (int)GL_BACK;

		if ( r_anaglyphMode->integer )
		{
			if ( r_anaglyphMode->modified )
			{
				clrcmd = R_GetCommandBuffer( sizeof( *clrcmd ) );
				if ( clrcmd ) {
					Com_Memset( clrcmd, 0, sizeof( *clrcmd ) );
					clrcmd->commandId = RC_CLEARCOLOR;
				} else {
					return;
				}
				clrcmd->colorMask = qtrue;
#ifdef USE_FBO
				if ( !fboEnabled )
#endif
				{
					// clear both, front and backbuffer.
					clrcmd->frontAndBack = qtrue;
				}
			}

			if ( stereoFrame == STEREO_LEFT )
			{
				// first frame
			}
			else if ( stereoFrame == STEREO_RIGHT )
			{
				clearDepthCommand_t *cldcmd;
				
				if ( (cldcmd = R_GetCommandBuffer(sizeof(*cldcmd))) == NULL )
					return;

				cldcmd->commandId = RC_CLEARDEPTH;
			}
			else
				ri.Error( ERR_FATAL, "RE_BeginFrame: Stereo is enabled, but stereoFrame was %i", stereoFrame );

			if ( (colcmd = R_GetCommandBuffer(sizeof(*colcmd))) == NULL )
				return;

			R_SetColorMode( colcmd->rgba, stereoFrame, r_anaglyphMode->integer );
			colcmd->commandId = RC_COLORMASK;
		}
		else // !r_anaglyphMode->integer
		{
			if ( stereoFrame != STEREO_CENTER )
				ri.Error( ERR_FATAL, "RE_BeginFrame: Stereo is disabled, but stereoFrame was %i", stereoFrame );

			// reset color mask
			if ( r_anaglyphMode->modified )	{
				if ( ( colcmd = R_GetCommandBuffer( sizeof( *colcmd ) ) ) == NULL )
					return;

				R_SetColorMode( colcmd->rgba, stereoFrame, r_anaglyphMode->integer );
				colcmd->commandId = RC_COLORMASK;
			}
		}
	}

	if ( r_fastsky->integer ) {
		if ( stereoFrame != STEREO_RIGHT ) {
			if ( !clrcmd ) {
				clrcmd = R_GetCommandBuffer( sizeof( *clrcmd ) );
				if ( clrcmd ) {
					Com_Memset( clrcmd, 0, sizeof( *clrcmd ) );
					clrcmd->commandId = RC_CLEARCOLOR;
				} else {
					return;
				}
			}
			clrcmd->fullscreen = qtrue;
			if ( r_anaglyphMode->integer ) {
				clrcmd->colorMask = qtrue;
			}
		}
	}

	tr.refdef.stereoFrame = stereoFrame;
}


/*
=============
RE_EndFrame

Returns the number of msec spent in the back end
=============
*/
void RE_EndFrame( int *frontEndMsec, int *backEndMsec ) {

	swapBuffersCommand_t *cmd;

	if ( !tr.registered ) {
		return;
	}

	cmd = R_GetCommandBufferReserved( sizeof( *cmd ), 0 );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SWAP_BUFFERS;

	R_PerformanceCounters();

	R_IssueRenderCommands();

	R_InitNextFrame();

	if ( frontEndMsec ) {
		*frontEndMsec = tr.frontEndMsec;
	}
	tr.frontEndMsec = 0;
	if ( backEndMsec ) {
		*backEndMsec = backEnd.pc.msec;
	}
	backEnd.pc.msec = 0;
	backEnd.throttle = qfalse;

	if ( r_trisColor->modified )
	{
		R_SetTrisColor();
		r_trisColor->modified = qfalse;
	}

	// recompile GPU shaders if needed
	if ( ri.Cvar_CheckGroup( CVG_RENDERER ) )
	{
		ARB_UpdatePrograms();

#ifdef USE_FBO
		if ( r_ext_multisample->modified || r_hdr->modified )
			QGL_InitFBO();
#endif

		if ( r_textureMode->modified )
			GL_TextureMode( r_textureMode->string );

		if ( r_gamma->modified )
			R_SetColorMappings();

		ri.Cvar_ResetGroup( CVG_RENDERER, qtrue );
	}
}


/*
=============
RE_TakeVideoFrame
=============
*/
void RE_TakeVideoFrame( int width, int height,
		byte *captureBuffer, byte *encodeBuffer, qboolean motionJpeg )
{
	videoFrameCommand_t	*cmd;

	if( !tr.registered ) {
		return;
	}

	backEnd.screenshotMask |= SCREENSHOT_AVI;

	cmd = &backEnd.vcmd;

	//cmd->commandId = RC_VIDEOFRAME;

	cmd->width = width;
	cmd->height = height;
	cmd->captureBuffer = captureBuffer;
	cmd->encodeBuffer = encodeBuffer;
	cmd->motionJpeg = motionJpeg;
}


void RE_ThrottleBackend( void )
{
	backEnd.throttle = qtrue;
}


void RE_FinishBloom( void )
{
#ifdef USE_FBO
	finishBloomCommand_t *cmd;

	if ( !tr.registered ) {
		return;
	}

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}

	cmd->commandId = RC_FINISHBLOOM;
#endif // USE_FBO
}


qboolean RE_CanMinimize( void )
{
#ifdef USE_FBO
	return fboEnabled;
#else
	return qfalse;
#endif
}


const glconfig_t *RE_GetConfig( void )
{
	return &glConfig;
}


//bani
/*
==================
RE_RenderToTexture
==================
*/
void RE_RenderToTexture( int textureid, int x, int y, int w, int h ) {
	renderToTextureCommand_t    *cmd;

//	ri.Printf( PRINT_ALL, "RE_RenderToTexture\n" );

	if ( !tr.registered ) {
		return;
	}

	// Can't do GL_GENERATE_MIPMAP_SGIS without GL 1.4
	if (gl_version < 14) {
		ri.Printf( PRINT_ALL, "WARNING: RE_RenderToTexture requires OpenGL 1.4 or newer!\n" );
		return;
	}

	if ( textureid > tr.numImages || textureid < 0 ) {
		ri.Printf( PRINT_ALL, "Warning: trap_R_RenderToTexture textureid %d out of range.\n", textureid );
		return;
	}

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_RENDERTOTEXTURE;
	cmd->image = tr.images[textureid];
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
}

//bani
/*
==================
RE_Finish
==================
*/
void RE_Finish( void ) {
	renderFinishCommand_t   *cmd;

	if (!tr.registered) {
		return;
	}

	ri.Printf( PRINT_ALL, "RE_Finish\n" );

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_FINISH;
}


/*
==================
RE_RenderOmnibot
==================
*/
void RE_RenderOmnibot( void ) {
	renderOmnibot_t *cmd;
	
	if (!tr.registered) {
		return;
	}
	cmd = R_GetCommandBuffer( sizeof( *cmd ) );

	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_DRAW_OMNIBOT;
}
