/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "tr_local.h"

static int			r_firstSceneDrawSurf;
#ifdef USE_PMLIGHT
static int			r_firstSceneLitSurf;
#endif

int			r_numdlights;
static int			r_firstSceneDlight;

static int			r_numcoronas;
static int			r_firstSceneCorona;
static int			r_numentities;
static int			r_firstSceneEntity;

static int			r_numpolys;
static int			r_firstScenePoly;

int			r_numpolyverts;

static int			r_firstScenePolybuffer;
static int			r_numpolybuffers;

// ydnar: decals
static int			r_firstSceneDecalProjector;
int			r_numDecalProjectors;
int			r_firstSceneDecal;

int skyboxportal; // signifies if the loaded map has a skyboxportal
/*
====================
R_InitNextFrame

====================
*/
void R_InitNextFrame( void ) {

	backEndData->commands.used = 0;

	r_firstSceneDrawSurf = 0;
#ifdef USE_PMLIGHT
	r_firstSceneLitSurf = 0;
#endif

	r_numdlights = 0;
	r_firstSceneDlight = 0;

	r_numcoronas = 0;
	r_firstSceneCorona = 0;

	r_numentities = 0;
	r_firstSceneEntity = 0;

	r_numpolys = 0;
	r_firstScenePoly = 0;

	r_numpolyverts = 0;

	r_numpolybuffers = 0;
	r_firstScenePolybuffer = 0;

	// ydnar: decals
	r_numDecalProjectors = 0;
	r_firstSceneDecalProjector = 0;
	r_firstSceneDecal = 0;
}


/*
====================
RE_ClearScene

====================
*/
void RE_ClearScene( void ) {
	int i;

	// ydnar: clear model stuff for dynamic fog
	if ( tr.world != NULL ) {
		for ( i = 0; i < tr.world->numBModels; i++ )
			tr.world->bmodels[ i ].visible = qfalse;
	}

	// everything else
	r_firstSceneDlight = r_numdlights;
	r_firstSceneCorona = r_numcoronas;
	r_firstSceneEntity = r_numentities;
	r_firstScenePoly = r_numpolys;
	r_firstScenePolybuffer = r_numpolybuffers;
}

/*
===========================================================================

DISCRETE POLYS

===========================================================================
*/

/*
=====================
R_AddPolygonSurfaces

Adds all the scene's polys into this view's drawsurf list
=====================
*/
void R_AddPolygonSurfaces( void ) {
	int			i;
	shader_t	*sh;
	srfPoly_t	*poly;

	tr.currentEntityNum = REFENTITYNUM_WORLD;
	tr.shiftedEntityNum = tr.currentEntityNum << QSORT_REFENTITYNUM_SHIFT;

	for ( i = 0, poly = tr.refdef.polys; i < tr.refdef.numPolys ; i++, poly++ ) {
		sh = R_GetShaderByHandle( poly->hShader );
		R_AddDrawSurf( ( void * )poly, sh, poly->fogIndex, 0 );
	}
}

/*
=====================
RE_AddPolyToScene

=====================
*/
void RE_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts ) {
	srfPoly_t	*poly;
	int			i;
	int			fogIndex;
	fog_t		*fog;
	vec3_t		bounds[2];

	if ( !tr.registered ) {
		return;
	}
#if 0
	if ( !hShader ) {
		ri.Printf( PRINT_WARNING, "WARNING: RE_AddPolyToScene: NULL poly shader\n");
		return;
	}
#endif
	if ( ( ( r_numpolyverts + numVerts ) > max_polyverts ) || ( r_numpolys >= max_polys ) ) {
		return;
	}

	poly = &backEndData->polys[r_numpolys];
	poly->surfaceType = SF_POLY;
	poly->hShader = hShader;
	poly->numVerts = numVerts;
	poly->verts = &backEndData->polyVerts[r_numpolyverts];
	
	Com_Memcpy( poly->verts, verts, numVerts * sizeof( *verts ) );
#if 0
	if ( glConfig.hardwareType == GLHW_RAGEPRO ) {
		poly->verts->modulate[0] = 255;
		poly->verts->modulate[1] = 255;
		poly->verts->modulate[2] = 255;
		poly->verts->modulate[3] = 255;
	}
#endif
	// done.
	r_numpolys++;
	r_numpolyverts += numVerts;

	// if no world is loaded
	if ( tr.world == NULL ) {
		fogIndex = 0;
	}
	// see if it is in a fog volume
	else if ( tr.world->numfogs == 1 ) {
		fogIndex = 0;
	} else {
		// find which fog volume the poly is in
		VectorCopy( poly->verts[0].xyz, bounds[0] );
		VectorCopy( poly->verts[0].xyz, bounds[1] );
		for ( i = 1 ; i < poly->numVerts ; i++ ) {
			AddPointToBounds( poly->verts[i].xyz, bounds[0], bounds[1] );
		}
		for ( fogIndex = 1 ; fogIndex < tr.world->numfogs ; fogIndex++ ) {
			fog = &tr.world->fogs[fogIndex]; 
			if ( bounds[1][0] >= fog->bounds[0][0]
				&& bounds[1][1] >= fog->bounds[0][1]
				&& bounds[1][2] >= fog->bounds[0][2]
				&& bounds[0][0] <= fog->bounds[1][0]
				&& bounds[0][1] <= fog->bounds[1][1]
				&& bounds[0][2] <= fog->bounds[1][2] ) {
				break;
			}
		}
		if ( fogIndex == tr.world->numfogs ) {
			fogIndex = 0;
		}
	}
	poly->fogIndex = fogIndex;
}

/*
=====================
RE_AddPolysToScene

=====================
*/
void RE_AddPolysToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys ) {
	srfPoly_t	*poly;
	int			i, j;
	int			fogIndex;
	fog_t		*fog;
	vec3_t		bounds[2];

	if ( !tr.registered ) {
		return;
	}
#if 0
	if ( !hShader ) {
		ri.Printf( PRINT_WARNING, "WARNING: RE_AddPolysToScene: NULL poly shader\n");
		return;
	}
#endif
	for ( j = 0; j < numPolys; j++ ) {
		if ( r_numpolyverts + numVerts > max_polyverts || r_numpolys >= max_polys ) {
//			ri.Printf( PRINT_WARNING, "WARNING: RE_AddPolysToScene: MAX_POLYS or MAX_POLYVERTS reached\n");
			return;
		}

		poly = &backEndData->polys[r_numpolys];
		poly->surfaceType = SF_POLY;
		poly->hShader = hShader;
		poly->numVerts = numVerts;
		poly->verts = &backEndData->polyVerts[r_numpolyverts];
		
		Com_Memcpy( poly->verts, &verts[numVerts*j], numVerts * sizeof( *verts ) );
#if 0
		if ( glConfig.hardwareType == GLHW_RAGEPRO ) {
			poly->verts->modulate[0] = 255;
			poly->verts->modulate[1] = 255;
			poly->verts->modulate[2] = 255;
			poly->verts->modulate[3] = 255;
		}
#endif
		// done.
		r_numpolys++;
		r_numpolyverts += numVerts;

		// if no world is loaded
		if ( tr.world == NULL ) {
			fogIndex = 0;
		}
		// see if it is in a fog volume
		else if ( tr.world->numfogs == 1 ) {
			fogIndex = 0;
		} else {
			// find which fog volume the poly is in
			VectorCopy( poly->verts[0].xyz, bounds[0] );
			VectorCopy( poly->verts[0].xyz, bounds[1] );
			for ( i = 1 ; i < poly->numVerts ; i++ ) {
				AddPointToBounds( poly->verts[i].xyz, bounds[0], bounds[1] );
			}
			for ( fogIndex = 1 ; fogIndex < tr.world->numfogs ; fogIndex++ ) {
				fog = &tr.world->fogs[fogIndex]; 
				if ( bounds[1][0] >= fog->bounds[0][0]
					&& bounds[1][1] >= fog->bounds[0][1]
					&& bounds[1][2] >= fog->bounds[0][2]
					&& bounds[0][0] <= fog->bounds[1][0]
					&& bounds[0][1] <= fog->bounds[1][1]
					&& bounds[0][2] <= fog->bounds[1][2] ) {
					break;
				}
			}
			if ( fogIndex == tr.world->numfogs ) {
				fogIndex = 0;
			}
		}
		poly->fogIndex = fogIndex;
	}
}


/*
=====================
R_AddPolygonBufferSurfaces

Adds all the scene's polys into this view's drawsurf list
=====================
*/
void R_AddPolygonBufferSurfaces( void ) {
	int i;
	shader_t        *sh;
	srfPolyBuffer_t *polybuffer;

	tr.currentEntityNum = REFENTITYNUM_WORLD;
	tr.shiftedEntityNum = tr.currentEntityNum << QSORT_REFENTITYNUM_SHIFT;

	for ( i = 0, polybuffer = tr.refdef.polybuffers; i < tr.refdef.numPolyBuffers ; i++, polybuffer++ ) {
		sh = R_GetShaderByHandle( polybuffer->pPolyBuffer->shader );

		R_AddDrawSurf( ( void * )polybuffer, sh, polybuffer->fogIndex, 0 );
	}
}

/*
=====================
RE_AddPolyBufferToScene

=====================
*/
void RE_AddPolyBufferToScene( polyBuffer_t* pPolyBuffer ) {
	srfPolyBuffer_t*    pPolySurf;
	int fogIndex;
	fog_t*              fog;
	vec3_t bounds[2];
	int i;

	if ( r_numpolybuffers >= MAX_POLYS ) {
		return;
	}

	pPolySurf = &backEndData->polybuffers[r_numpolybuffers];
	r_numpolybuffers++;

	pPolySurf->surfaceType = SF_POLYBUFFER;
	pPolySurf->pPolyBuffer = pPolyBuffer;

	VectorCopy( pPolyBuffer->xyz[0], bounds[0] );
	VectorCopy( pPolyBuffer->xyz[0], bounds[1] );
	for ( i = 1 ; i < pPolyBuffer->numVerts ; i++ ) {
		AddPointToBounds( pPolyBuffer->xyz[i], bounds[0], bounds[1] );
	}
	for ( fogIndex = 1 ; fogIndex < tr.world->numfogs ; fogIndex++ ) {
		fog = &tr.world->fogs[fogIndex];
		if ( bounds[1][0] >= fog->bounds[0][0]
			 && bounds[1][1] >= fog->bounds[0][1]
			 && bounds[1][2] >= fog->bounds[0][2]
			 && bounds[0][0] <= fog->bounds[1][0]
			 && bounds[0][1] <= fog->bounds[1][1]
			 && bounds[0][2] <= fog->bounds[1][2] ) {
			break;
		}
	}
	if ( fogIndex == tr.world->numfogs ) {
		fogIndex = 0;
	}

	pPolySurf->fogIndex = fogIndex;
}


//=================================================================================

static int isnan_fp( const float *f )
{
	uint32_t u = *( (uint32_t*) f );
	u = 0x7F800000 - ( u & 0x7FFFFFFF );
	return (int)( u >> 31 );
}


/*
=====================
RE_AddRefEntityToScene
=====================
*/
void RE_AddRefEntityToScene( const refEntity_t *ent, qboolean intShaderTime ) {
	if ( !tr.registered ) {
		return;
	}
	if ( r_numentities >= MAX_REFENTITIES ) {
		ri.Printf( PRINT_DEVELOPER, "RE_AddRefEntityToScene: Dropping refEntity, reached MAX_REFENTITIES\n" );
		return;
	}
	if ( isnan_fp( &ent->origin[0] ) || isnan_fp( &ent->origin[1] ) || isnan_fp( &ent->origin[2] ) ) {
		static qboolean first_time = qtrue;
		if ( first_time ) {
			first_time = qfalse;
			ri.Printf( PRINT_WARNING, "RE_AddRefEntityToScene passed a refEntity which has an origin with a NaN component\n" );
		}
		return;
	}
	if ( (unsigned)ent->reType >= RT_MAX_REF_ENTITY_TYPE ) {
		ri.Error( ERR_DROP, "RE_AddRefEntityToScene: bad reType %i", ent->reType );
	}

	backEndData->entities[r_numentities].e = *ent;
	backEndData->entities[r_numentities].lightingCalculated = qfalse;
	backEndData->entities[r_numentities].intShaderTime = intShaderTime;

	r_numentities++;

	// ydnar: add projected shadows for this model
	R_AddModelShadow( ent );
}

extern qboolean modIsETF;

/*
=====================
RE_AddDynamicLightToScene
=====================
*/
void RE_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags ) {
	dlight_t	*dl;

	if ( !tr.registered ) {
		return;
	}
	if ( r_numdlights >= ARRAY_LEN( backEndData->dlights ) ) {
		return;
	}
	if ( radius <= 0 || intensity <= 0 ) {
		return;
	}
#ifndef USE_VULKAN
	// these cards don't have the correct blend mode
	if ( glConfig.hardwareType == GLHW_RIVA128 || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		return;
	}
#endif

	// RF, allow us to force some dlights under all circumstances
	// Ensiform - notice: This is changing of behavior of this flag.
	// Reason for change is that 1 << 31 does not fit properly into type `int`
	// There may be alternative solutions to retain compatibility but I can't think of them
	// Obviously would have been better if they didn't use 1<<31 at all here
	// However all uses in etmain seem to only use it by itself and no other bitmask
	if ( !((unsigned)flags & REF_FORCE_DLIGHT) ) {
		if ( r_dynamiclight->integer == 0 ) {
			return;
		}
	}

#ifdef USE_PMLIGHT
#ifdef USE_LEGACY_DLIGHTS
	if ( r_dlightMode->integer )
#endif
	{
		// ETF powerup hack
		if ( modIsETF && ( radius == 200 || radius == 150 ) && intensity >= 1.25f )
			intensity = 1.25f;

		// ENSI NOTE FIXME this is quick and dirty hack to support intensity dlights (dyna)
		r *= intensity;
		g *= intensity;
		b *= intensity;
		// END ENSI

		r *= r_dlightIntensity->value;
		g *= r_dlightIntensity->value;
		b *= r_dlightIntensity->value;
		radius *= r_dlightScale->value;
		//intensity *= r_dlightScale->value;
	}
#endif

	if ( r_dlightSaturation->value != 1.0 )
	{
		float luminance = LUMA( r, g, b );
		r = LERP( luminance, r, r_dlightSaturation->value );
		g = LERP( luminance, g, r_dlightSaturation->value );
		b = LERP( luminance, b, r_dlightSaturation->value );
	}

	dl = &backEndData->dlights[r_numdlights++];
	VectorCopy( org, dl->origin );
	dl->radius = radius;
	dl->radiusInverseCubed = ( 1.0 / dl->radius );
	dl->radiusInverseCubed = dl->radiusInverseCubed * dl->radiusInverseCubed * dl->radiusInverseCubed;
	dl->intensity = intensity;
	dl->color[ 0 ] = r;
	dl->color[ 1 ] = g;
	dl->color[ 2 ] = b;
	dl->shader = R_GetShaderByHandle( hShader );
	if ( dl->shader == tr.defaultShader ) {
		dl->shader = NULL;
	}
	dl->flags = (unsigned)flags;
	dl->linear = qfalse;
}


/*
=====================
RE_AddLinearLightToScene
=====================
*/
void RE_AddLinearLightToScene( const vec3_t start, const vec3_t end, float intensity, float r, float g, float b  ) {
	dlight_t	*dl;
	if ( VectorCompare( start, end ) ) {
		RE_AddLightToScene( start, intensity, intensity, r, g, b, 0, 0 );
		return;
	}
	if ( !tr.registered ) {
		return;
	}
	if ( r_numdlights >= ARRAY_LEN( backEndData->dlights ) ) {
		return;
	}
	if ( intensity <= 0 ) {
		return;
	}
#ifdef USE_PMLIGHT
#ifdef USE_LEGACY_DLIGHTS
	if ( r_dlightMode->integer )
#endif
	{
		r *= r_dlightIntensity->value;
		g *= r_dlightIntensity->value;
		b *= r_dlightIntensity->value;
		intensity *= r_dlightScale->value;
	}
#endif

	if ( r_dlightSaturation->value != 1.0 )
	{
		float luminance = LUMA( r, g, b );
		r = LERP( luminance, r, r_dlightSaturation->value );
		g = LERP( luminance, g, r_dlightSaturation->value );
		b = LERP( luminance, b, r_dlightSaturation->value );
	}

	dl = &backEndData->dlights[ r_numdlights++ ];
	VectorCopy( start, dl->origin );
	VectorCopy( end, dl->origin2 );
	dl->radius = intensity;
	dl->radiusInverseCubed = ( 1.0 / dl->radius );
	dl->radiusInverseCubed = dl->radiusInverseCubed * dl->radiusInverseCubed * dl->radiusInverseCubed;
	dl->intensity = intensity;
	dl->color[0] = r;
	dl->color[1] = g;
	dl->color[2] = b;
	dl->shader = NULL;
	dl->flags = 0;
	dl->linear = qtrue;
}


/*
==============
RE_AddCoronaToScene
==============
*/
void RE_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible ) {
	corona_t    *cor;

	if ( !tr.registered ) {
		return;
	}
	if ( r_numcoronas >= MAX_CORONAS ) {
		return;
	}

	cor = &backEndData->coronas[r_numcoronas++];
	VectorCopy( org, cor->origin );
	cor->color[0] = r;
	cor->color[1] = g;
	cor->color[2] = b;
	cor->scale = scale;
	cor->id = id;
	cor->visible = visible;
}


void *R_GetCommandBuffer( int bytes );

/*
@@@@@@@@@@@@@@@@@@@@@
RE_RenderScene

Draw a 3D view into a part of the window, then return
to 2D drawing.

Rendering a scene may require multiple views to be rendered
to handle mirrors,
@@@@@@@@@@@@@@@@@@@@@
*/
void RE_RenderScene( const refdef_t *fd ) {
#ifdef USE_VULKAN
	renderCommand_t	lastRenderCommand;
#endif
	viewParms_t		parms;
	int				startTime;

	if ( !tr.registered ) {
		return;
	}

	if ( r_norefresh->integer ) {
		return;
	}

	startTime = ri.Milliseconds();

	if (!tr.world && !( fd->rdflags & RDF_NOWORLDMODEL ) ) {
		ri.Error (ERR_DROP, "R_RenderScene: NULL worldmodel");
	}

	Com_Memcpy( tr.refdef.text, fd->text, sizeof( tr.refdef.text ) );

	tr.refdef.x = fd->x;
	tr.refdef.y = fd->y;
	tr.refdef.width = fd->width;
	tr.refdef.height = fd->height;
	tr.refdef.fov_x = fd->fov_x;
	tr.refdef.fov_y = fd->fov_y;

	VectorCopy( fd->vieworg, tr.refdef.vieworg );
	VectorCopy( fd->viewaxis[0], tr.refdef.viewaxis[0] );
	VectorCopy( fd->viewaxis[1], tr.refdef.viewaxis[1] );
	VectorCopy( fd->viewaxis[2], tr.refdef.viewaxis[2] );

	tr.refdef.time = fd->time;
	tr.refdef.rdflags = fd->rdflags;

	if ( fd->rdflags & RDF_SKYBOXPORTAL ) {
		skyboxportal = 1;
	}

	// copy the areamask data over and note if it has changed, which
	// will force a reset of the visible leafs even if the view hasn't moved
	tr.refdef.areamaskModified = qfalse;
	if ( ! (tr.refdef.rdflags & RDF_NOWORLDMODEL) ) {
		int		areaDiff;
		int		i;

		// compare the area bits
		areaDiff = 0;
		for ( i = 0; i < MAX_MAP_AREA_BYTES/sizeof(int); i++ ) {
			areaDiff |= ((int *)tr.refdef.areamask)[i] ^ ((int *)fd->areamask)[i];
			((int *)tr.refdef.areamask)[i] = ((int *)fd->areamask)[i];
		}

		if ( areaDiff ) {
			// a door just opened or something
			tr.refdef.areamaskModified = qtrue;
		}
	}


	// derived info

	tr.refdef.floatTime = (double)tr.refdef.time * 0.001; // -EC-: cast to double

	tr.refdef.numDrawSurfs = r_firstSceneDrawSurf;
	tr.refdef.drawSurfs = backEndData->drawSurfs;

#ifdef USE_PMLIGHT
	tr.refdef.numLitSurfs = r_firstSceneLitSurf;
	tr.refdef.litSurfs = backEndData->litSurfs;
#endif

	tr.refdef.num_entities = r_numentities - r_firstSceneEntity;
	tr.refdef.entities = &backEndData->entities[r_firstSceneEntity];

	tr.refdef.num_dlights = r_numdlights - r_firstSceneDlight;
	tr.refdef.dlights = &backEndData->dlights[r_firstSceneDlight];

	tr.refdef.num_coronas = r_numcoronas - r_firstSceneCorona;
	tr.refdef.coronas = &backEndData->coronas[r_firstSceneCorona];

	tr.refdef.numPolys = r_numpolys - r_firstScenePoly;
	tr.refdef.polys = &backEndData->polys[r_firstScenePoly];

	tr.refdef.numPolyBuffers = r_numpolybuffers - r_firstScenePolybuffer;
	tr.refdef.polybuffers = &backEndData->polybuffers[r_firstScenePolybuffer];

	tr.refdef.numDecalProjectors = r_numDecalProjectors - r_firstSceneDecalProjector;
	tr.refdef.decalProjectors = &backEndData->decalProjectors[ r_firstSceneDecalProjector ];

	tr.refdef.numDecals = 0;
	tr.refdef.decals = &backEndData->decals[ r_firstSceneDecal ];

#ifndef USE_VULKAN
	// turn off dynamic lighting globally by clearing all the
	// dlights if it needs to be disabled
	if ( /*r_dynamiclight->integer == 0 || */glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		tr.refdef.num_dlights = 0;
	}
#endif

	// a single frame may have multiple scenes draw inside it --
	// a 3D game view, 3D status bar renderings, 3D menus, etc.
	// They need to be distinguished by the light flare code, because
	// the visibility state for a given surface may be different in
	// each scene / view.
	tr.frameSceneNum++;
	tr.sceneCount++;

	// setup view parms for the initial view
	//
	// set up viewport
	// The refdef takes 0-at-the-top y coordinates, so
	// convert to GL's 0-at-the-bottom space
	//
	Com_Memset( &parms, 0, sizeof( parms ) );
	parms.viewportX = tr.refdef.x;
	parms.viewportY = glConfig.vidHeight - ( tr.refdef.y + tr.refdef.height );
	parms.viewportWidth = tr.refdef.width;
	parms.viewportHeight = tr.refdef.height;

	parms.scissorX = parms.viewportX;
	parms.scissorY = parms.viewportY;
	parms.scissorWidth = parms.viewportWidth;
	parms.scissorHeight = parms.viewportHeight;

	parms.portalView = PV_NONE;

#ifdef USE_PMLIGHT
	parms.dlights = tr.refdef.dlights;
	parms.num_dlights = tr.refdef.num_dlights;
#endif

	parms.fovX = tr.refdef.fov_x;
	parms.fovY = tr.refdef.fov_y;
	
	parms.stereoFrame = tr.refdef.stereoFrame;

	VectorCopy( fd->vieworg, parms.orientation.origin );
	VectorCopy( fd->viewaxis[0], parms.orientation.axis[0] );
	VectorCopy( fd->viewaxis[1], parms.orientation.axis[1] );
	VectorCopy( fd->viewaxis[2], parms.orientation.axis[2] );

	VectorCopy( fd->vieworg, parms.pvsOrigin );

#ifdef USE_VULKAN
	lastRenderCommand = tr.lastRenderCommand;
	tr.drawSurfCmd = NULL;
	tr.numDrawSurfCmds = 0;
#endif

	R_RenderView( &parms );

#ifndef USE_VULKAN
	if ( fd->rdflags & RDF_RENDEROMNIBOT )
		RE_RenderOmnibot();
#endif

#ifdef USE_VULKAN
	if ( tr.needScreenMap )
	{
		if ( lastRenderCommand == RC_DRAW_BUFFER )
		{
			// duplicate all views, including portals
			drawSurfsCommand_t *cmd, *src = NULL;
			int i;

			for ( i = 0; i < tr.numDrawSurfCmds; i++ )
			{
				cmd = R_GetCommandBuffer( sizeof( *cmd ) );
				if ( cmd )
				{
					src = tr.drawSurfCmd + i;
					*cmd = *src;
				}
				else
				{
					break;
				}
			}

			if ( src )
			{
				// first drawsurface
				tr.drawSurfCmd[0].refdef.needScreenMap = qtrue;
				// last drawsurface
				src->refdef.switchRenderPass = qtrue;
			}
		}

		tr.needScreenMap = 0;
	}
#endif

	// the next scene rendered in this frame will tack on after this one
	r_firstSceneDrawSurf = tr.refdef.numDrawSurfs;
#ifdef USE_PMLIGHT
	r_firstSceneLitSurf = tr.refdef.numLitSurfs;
#endif
	r_firstSceneDecal += tr.refdef.numDecals;
	r_firstSceneEntity = r_numentities;
	r_firstSceneDlight = r_numdlights;
	r_firstScenePoly = r_numpolys;
	r_firstScenePolybuffer = r_numpolybuffers;

	tr.frontEndMsec += ri.Milliseconds() - startTime;
}
