
textures/alpha/bel_orn_m01
{
// invalid JPW FIXME SP merge	cull front
	{
		map textures/alpha/bel_orn_m01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		depthWrite
		rgbGen vertex

	}
}


textures/snow/alpha_ice2
{		
	surfaceparm alphashadow
	cull none
	{
		map textures/snow/alpha_ice2s.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}
textures/alpha/cweb_m01drk
{
    qer_trans 0.8
    surfaceparm alphashadow
    surfaceparm nomarks
    surfaceparm nonsolid
    cull disable
    deformVertexes wave 10 sin 0 2 0 0.2
    {
        map textures/alpha/cweb_m01drk.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}
textures/alpha/cweb_m02drk
{
    qer_trans 0.8
    surfaceparm alphashadow
    surfaceparm nomarks
    surfaceparm nonsolid
    cull disable
    deformVertexes wave 10 sin 0 2 0 0.2
    {
        map textures/alpha/cweb_m02drk.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/alpha/cweb_m01a
{
    //removelightmap info
    //surfaceparm nolightmap
    qer_trans 0.8
    surfaceparm alphashadow
    surfaceparm nomarks
    surfaceparm nonsolid
    cull disable
    deformVertexes wave 10 sin 0 2 0 0.2
    {
        map textures/alpha/cweb_m01a.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

textures/alpha/cweb_m01b
{
    qer_trans 0.8
    surfaceparm alphashadow
    surfaceparm nomarks
    surfaceparm nonsolid
    cull disable
    deformVertexes wave 10 sin 0 2 0 0.2
    {
        map textures/alpha/cweb_m01b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_COLOR
        depthFunc equal
        rgbGen identity
    }
}

textures/alpha/fence_m01
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_m01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_m01b
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_m01b.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_m01b_snow
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_m01b_snow.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_m02
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_m02.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_m03
{
    qer_trans 0.5
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_m03.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c01
{
	nopicmip
    surfaceparm metalsteps
    cull disable
    {
        map textures/alpha/fence_c01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c02
{
	nopicmip
    surfaceparm metalsteps
    cull disable
    {
        map textures/alpha/fence_c02.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/truss_m06
{
    surfaceparm alphashadow
//  nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/truss_m06.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c03
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c03.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c04
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c04.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c05
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c05.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c09
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c09.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c06
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c06.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c10
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c10.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/fence_c10b
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c10b.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}


textures/alpha/fence_c10a
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c10a.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c11
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c11.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c11b
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c11b.tga
        alphaFunc GE128
        depthWrite
        rgbGen identity
    }
}

textures/alpha/fence_c12
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c12.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c12a
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c12a.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c13
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c13.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c14
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c14.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c15
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c15.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/door_c01
{
    surfaceparm metalsteps
    cull disable
    {
        map textures/alpha/door_c01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/door_c02
{
    surfaceparm metalsteps
    cull disable
    {
        map textures/alpha/door_c02.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

//// maxx fix, new alpha for level fog ///////
textures/alpha/fence_c07
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c07.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_c08
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_c08.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_m04
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_m04.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_m04a
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_m04a.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_m05
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_m05.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_m06
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_m06.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/fence_m06b
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/fence_m06b.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/mesh_c01
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/mesh_c01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/mesh_c02
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/mesh_c02.tga
	//   blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/mesh_c03
{
 //   surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull twosided
    surfaceparm alphashadow
    {
        map textures/alpha/mesh_c03.tga
        alphaFunc GE128
        depthWrite
        rgbGen identity
    }
}

textures/alpha/tree_c01
{
    surfaceparm metalsteps
	nopicmip
    cull disable
    {
        map textures/alpha/tree_c01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/tree_c02
{
    surfaceparm metalsteps
	nopicmip
    cull disable
    {
        map textures/alpha/tree_c02.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/tree_c03
{
    surfaceparm metalsteps
	nopicmip
    cull disable
    {
        map textures/alpha/tree_c03.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/tree_test
{
    surfaceparm metalsteps
	nopicmip
    cull disable
    {
        map textures/alpha/tree_test.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/bars_m01
{
    surfaceparm alphashadow    
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/bars_m01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/ladder
{
	nopicmip
    surfaceparm alphashadow
    surfaceparm metalsteps
    cull disable
    {
        map textures/alpha/ladder.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/ladder_snow
{
	nopicmip
    surfaceparm alphashadow
    surfaceparm metalsteps
    cull disable
    {
        map textures/alpha/ladder_snow.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/ladder2 //CHAD ADDED THIS!!!
{
	nopicmip
    surfaceparm alphashadow
    surfaceparm metalsteps
    cull disable
    {
        map textures/alpha/ladder2.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}


textures/alpha/truss_m01
{
    surfaceparm alphashadow
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/truss_m01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/truss_m02
{
    surfaceparm alphashadow
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/truss_m02.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/truss_m03
{
    surfaceparm alphashadow
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/truss_m03.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/truss_m04
{
    surfaceparm alphashadow
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/truss_m04.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/truss_m05
{
    surfaceparm alphashadow
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/truss_m05.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/truss_m05_snow
{
    surfaceparm alphashadow
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/truss_m05_snow.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/hay
{
    qer_editorimage textures/props/hay.tga
    surfaceparm alphashadow
    cull disable
    {
        map textures/props/hay.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}



textures/alpha/truss_m06r
{
    surfaceparm alphashadow
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/truss_m06r.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/truss_m06a
{
    surfaceparm alphashadow
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/truss_m06a.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}


textures/assault/atruss_m06a
{
    surfaceparm alphashadow
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/assault/atruss_m06a.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/truss_m07
{
    surfaceparm alphashadow
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/truss_m07.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/snow/s_fence_c07
{
    surfaceparm metalsteps
    cull disable
    {
        map textures/snow/s_fence_c07.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/snow/s_fence_c08
{
    surfaceparm metalsteps
    cull disable
    {
        map textures/snow/s_fence_c08.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/snow/s_bars_m01
{
	nopicmip
    surfaceparm metalsteps
    cull disable
    {
        //blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        map textures/snow/s_bars_m01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}


textures/assault_rock/hazz2
{
	nocompress
	sort 16
	{
	//	map textures/assault_rock/haze_vil_night.tga
		map textures/assault_rock/hazz.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	}

}

textures/assault_rock/haze_vil_night
{
	nocompress
	sort 16
	{
		map textures/assault_rock/haze_vil_night.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	}

}

textures/assault_rock/hazz
{
	nocompress
	surfaceparm metalsteps		

	sort 16
	{
		map textures/assault_rock/haze_vil_night.tga
	//	map textures/assault_rock/hazz4.tga
		blendFunc GL_SRC_ALPHA  GL_ONE
		rgbGen identity
	}

}

textures/assault_rock/haze2
{
	nocompress
	surfaceparm metalsteps		
//	cull front // invalid command JPW FIXME SP merge

	{
		map textures/assault_rock/haze2.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	}

}
textures/rock/haze
{
	surfaceparm metalsteps		
// invalid JPW FIXME SP merge	cull front
	nofog

	{
		map textures/rock/haze.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	}

}

textures/assault_rock/haze_horiz
{
	nocompress
	surfaceparm metalsteps		
// invalid JPW FIXME SP merge	cull front
	nofog

	{
		map textures/assault_rock/haze_horiz.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	}

}


textures/rubble/debri_m02
{
	surfaceparm alphashadow		
	cull none
	{
		map textures/rubble/debri_m02.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}





textures/rubble/debri_m03
{
	surfaceparm alphashadow		
	cull none
	{
		map textures/rubble/debri_m03.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}


textures/rubble/brk_window4a
{
	surfaceparm alphashadow		
// invalid JPW FIXME SP merge	cull front
	
	{
		map textures/rubble/brk_window4a.tga
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}

textures/rubble/brk_window4aa
{
	surfaceparm alphashadow		
// invalid JPW FIXME SP merge	cull front
	
	{
		map textures/rubble/brk_window4aa.tga
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}


textures/rubble/floor_m01a
{
	surfaceparm alphashadow		
	cull none
	{
		map textures/rubble/floor_m01a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}

textures/rubble/brk_window4a_sa
{
	surfaceparm alphashadow		
	cull none
	{
		map textures/rubble/brk_window4a_s01.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}


textures/rubble/roof_m01
{
	surfaceparm alphashadow		
	cull none
	{
		map textures/rubble/roof_m01.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/rubble/horizon_rubble_m01
{
	surfaceparm alphashadow		
	cull none
	{
		map textures/rubble/horizon_rubble_m01.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}

textures/rubble/horizon_rubble_m01_sa
{		
	// (SA) this is not right.  I shouldn't have to do a pass
	// first to black out the area, then another to fill in
	// with texture.  I should be able to comment out the first
	// stage and have it solid.  However, that leaves it slightly
	// transparent.  I'll look into why this happens.
	// (plus the fact that doing the first 'clearing' pass
	// causes a black edge around edges that should fade to clear)
//	{
//		map textures/rubble/horizon_rubble_m01.tga
//		blendFunc GL_ZERO GL_ONE_MINUS_SRC_ALPHA
//	}
//	{
//		map textures/rubble/horizon_rubble_m01.tga
//		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
//	}

	// (SA) hmm, okay, inverting the alpha channel in the image
	// then swapping the blend func works.  very odd.
	{
		map textures/rubble/horizon_rubble_m01_s01.tga
		blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
	}
}

textures/rubble/roof_in_m01
{		
	surfaceparm alphashadow
	cull none
	{
		map textures/rubble/roof_in_m01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

textures/rubble/rebar_m01
{
	surfaceparm metalsteps		
	cull none
	{
		map textures/rubble/rebar_m01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	}
}

textures/rubble/burn_flr_m01
{
	surfaceparm woodsteps
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/rubble/burn_flr_m01.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
}

textures/rubble/burn_flr_m01a
{
	qer_editorimage textures/rubble/burn_flr_m01a.tga
	surfaceparm woodsteps

	{
		map textures/props/ember1a.tga
		tcmod rotate 3
		rgbGen wave sin 1 0.5 0 0.5 
	}
	{
		map textures/rubble/burn_flr_m01a.tga
		blendfunc blend
		rgbGen vertex
	}
}

textures/rubble/burn_flr_m01b
{
	qer_editorimage textures/rubble/burn_flr_m01a.tga
	surfaceparm woodsteps

	{
		map textures/props/ember1a.tga
		tcmod rotate -3
		rgbGen wave sin 1 0.45 0 0.5 
	}
	{
		map textures/rubble/burn_flr_m01a.tga
		blendfunc blend
		rgbGen vertex
	}
}

textures/rubble/burn_flr_m01c
{
	qer_editorimage textures/rubble/burn_flr_m01a.tga
	surfaceparm woodsteps

	{
		map textures/props/ember1a.tga
		tcmod rotate 3.2
		rgbGen wave sin 1 0.55 0 0.5 
	}
	{
		map textures/rubble/burn_flr_m01a.tga
		blendfunc blend
		rgbGen vertex
	}
}

textures/props/torch_ember
{
	qer_editorimage textures/props/torch_ember.tga
	surfaceparm woodsteps
	{
		map textures/props/ember1a.tga
		tcmod rotate 3
		rgbGen wave sin 1 0.5 0 0.5 
	}
	{
		map textures/props/torch_ember.tga
		blendfunc blend
		rgbGen lightingdiffuse
	}
}

textures/rubble/burn_flr_m01a_bak2
{      
	{
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
	//	depthFunc equal
	}
      
        {
		map textures/effects/regenmap5.tga
                tcmod rotate 12
	//	tcmod stretch scale 1 .4 0 .1

        //	tcmod scroll 3 1
	//	tcmod scale 2 2
	//	tcmod stretch 1 .5 0 2
                blendFunc GL_ONE GL_ZERO
//		rgbGen wave sin 1 .6 0 .5
	} 
        {
//        	map textures/rubble/burn_flr_m01a.tga
        	map textures/rubble/burn_flr_m01a_sa1.tga
//		blendFunc blend
		blendfunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
		rgbGen identity
//		alphaGen wave sin .2 .7 0 .5
		alphaGen wave sin 1 1 0 2
	}
}

textures/rubble/burn_flr_m01a_bak
{      
	{
		map textures/effects/regenmap5.tga
		tcmod rotate 3
		blendFunc GL_ONE GL_ZERO
		rgbGen wave sin 1 .6 0 .5
	}
	{
        	map textures/rubble/burn_flr_m01a.tga
		blendFunc blend
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}

textures/rubble/burn_flr_m01b
{
	{
		map textures/effects/regenmap5.tga
		tcmod rotate 3
		blendFunc GL_ONE GL_ZERO
		rgbGen wave sin 1 .3 0 .5
	} 
	{
        	map textures/rubble/burn_flr_m01b.tga
		blendFunc blend
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
	}
}

textures/b-25/wire1
{
	surfaceparm metalsteps		
	cull none
	{
		map textures/b-25/wire1.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		depthWrite
		rgbGen vertex
	}

}
textures/b-25/plane_int4
{
	surfaceparm metalsteps		
// invalid JPW FIXME SP merge	cull front
	{
		map textures/b-25/plane_int4.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}

textures/b-25/rib2
{
	surfaceparm metalsteps		
// invalid JPW FIXME SP merge	cull front
	{
		map textures/b-25/rib2.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		depthWrite
		rgbGen vertex
	}

}
textures/b-25/glass3
{
	surfaceparm metalsteps		
// invalid JPW FIXME SP merge	cull front
	{
		map textures/b-25/glass3.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}

textures/graveyard/gy_vine01
{
    qer_trans 0.8
    surfaceparm alphashadow
    surfaceparm nomarks
    surfaceparm nonsolid
    cull disable
    deformVertexes wave 10 sin 0 2 0 0.2
    {
        map textures/graveyard/gy_vine01.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}



textures/terrain/pine3
{
	surfaceparm alphashadow


	deformVertexes wave 194 sin 0 2 0 .1
	deformVertexes wave 30 sin 0 1 0 .3
	deformVertexes wave 194 sin 0 1 0 .4

	cull twosided

	{
        map textures/terrain/pine3.tga
//        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	alphaFunc GE128
	depthWrite
	rgbGen vertex

	}
}

textures/doors/door_m04
{
	surfaceparm metalsteps		
	nopicmip
	cull none
	{
		map textures/doors/door_m04.tga
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaFunc GE128
		rgbGen vertex
	}

}

textures/tree/tree_pine1m
{
	surfaceparm metalsteps		
	nopicmip
	cull none
	{
		map textures/tree/tree_pine1m.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	}

}

textures/tree/tree_m01
{
	nopicmip
	cull twosided
	{
		map textures/tree/tree_m01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen lightingdiffuse
	//	rgbGen vertex
		depthWrite

	}

}

textures/tree/tree_m03
{
	nopicmip
	cull twosided
	{
		map textures/tree/tree_m03.tga
	//	alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m04
{
	nopicmip
	cull twosided
	{
		map textures/tree/tree_m04.tga
		alphaFunc GT0
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m05
{
	nopicmip
	cull twosided
	{
		map textures/tree/tree_m05.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m06
{
	nopicmip
	cull twosided
	{
		map textures/tree/tree_m06.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m07
{
	nopicmip
	//sort 10		
// invalid JPW FIXME SP merge	cull front
	surfaceparm alphashadow
	{
		map textures/tree/tree_m07.tga
	//	alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m08
{
	nopicmip
	//sort 10		
	cull twosided
	surfaceparm alphashadow
	// nopicmip
	// nomipmap
	{

		map textures/tree/tree_m08.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}



textures/tree/tree_m09
{
	nopicmip
	//sort 10		
	cull twosided
	surfaceparm alphashadow
	// nopicmip
	// nomipmap
	{

		map textures/tree/tree_m09.tga
		//blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaFunc GE128
		rgbGen identity

	}

}

textures/tree/tree_m01s
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/tree_m01s.tga
	//	alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m02s
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/tree_m02s.tga
	//	alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m01t
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/tree_m01t.tga
	//	alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m02dm
{
	nopicmip
	//sort 10		
//	cull twosided
	{
		map textures/tree/tree_m02dm.tga
	//	alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m02s_snow
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/tree_m02s_snow.tga
	//	alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/branch_pine2
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/branch_pine2.tga
		alphaFunc GE128
		rgbGen identity

	}

}
textures/tree/branch_pine1
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/branch_pine1.tga
		alphaFunc GE128
		rgbGen identity

	}

}

textures/tree/branch_pine5
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/branch_pine5.tga
		alphaFunc GE128
		rgbGen identity

	}

}

textures/tree/branch_pine5a
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/branch_pine5a.tga
		alphaFunc GE128
		rgbGen identity

	}

}
textures/tree/branch_pine6
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/branch_pine6.tga
		alphaFunc GE128
		rgbGen identity

	}

}

textures/tree/branch_pine7
{
	nopicmip
	//sort 10
	deformVertexes autosprite2		
	cull twosided
	{
		map textures/tree/branch_pine7.tga
	//	alphaFunc GE128
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity

	}

}


textures/tree/trunck3
{
	nopicmip
	cull twosided
	{
		map textures/tree/trunck3.tga
	//	alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/trunck3a
{
	nopicmip
	cull twosided
	{
		map textures/tree/trunck3a.tga
	//	alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/alpha_back
{
	nopicmip
	cull twosided
	{
		map textures/tree/alpha_back.tga
	//	alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m08_lod
{
	// sort 10		
	cull twosided
	nopicmip
	//nomipmap
	// deformVertexes autoSprite2
	{

		map textures/tree/tree_m08_lod.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m07_lod
{
	// sort 10		
	cull twosided
	nopicmip
	// nomipmap
	//deformVertexes autoSprite2

	{

		map textures/tree/tree_m07_lod.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/weed_c01
{
	nopicmip
	cull twosided


	{

		map textures/tree/weed_c01.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/weed_c05
{
	nopicmip
	cull twosided


	{

		map textures/tree/weed_c05.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}

textures/tree/tree_m08snow
{

	surfaceparm alphashadow
	//sort 10		
	cull twosided
	nopicmip
	// nomipmap
	{

		map textures/tree/tree_m08snow.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	//	rgbGen vertex
	//	depthWrite

	}

}



textures/miltary_wall/window_m03
{
	surfaceparm alphashadow		
// invalid JPW FIXME SP merge	cull front
	
	{
		map textures/miltary_wall/window_m03.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
	//	rgbGen vertex
		depthWrite
	}
}

textures/props/fwindow1

{
        {
		map textures/props/fwindow1.tga
		blendFunc GL_ONE GL_SRC_ALPHA
		rgbGen identity
	}

	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}

}
textures/tree/pine_m01
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/pine_m01.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	}

}
textures/alpha/dish2
{
//    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/dish2.tga
	alphaFunc GE128
//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen lightingdiffuse
    }
}

textures/stone/mxsnow0i
{
		
	cull twosided

	{
		map textures/stone/mxsnow0i.tga
		rgbGen identity
	}

}

textures/alpha/barb_wire
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/barb_wire.tga
       alphaFunc GE128
	//blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen identity

    }
}

textures/alpha/latace
{
    surfaceparm woodsteps
    surfaceparm nomarks
    nomipmaps
    nopicmip
    cull disable
//  surfaceparm alphashadow
    {
        map textures/alpha/latace.tga
       //alphaFunc GE128
	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex

    }
}
textures/alpha/ivy_c01
{
    surfaceparm woodsteps
    surfaceparm nomarks
    nomipmaps
    nopicmip
    cull disable
//  surfaceparm alphashadow
    {
        map textures/alpha/ivy_c01.tga
       //alphaFunc GE128
	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen vertex

    }
}


textures/alpha/fan
{
        surfaceparm trans	
        surfaceparm nomarks	
	cull none
        nopicmip
	{
		clampmap textures/alpha/fan.tga
		tcMod rotate 256 
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}

//FastFan, for Chad by Mike
textures/alpha/fastfan
{
        surfaceparm trans	
        surfaceparm nomarks	
	cull none
        nopicmip
	{
		clampmap textures/alpha/fastfan.tga
		tcMod rotate 256 
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}

textures/tree/branch1
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/branch1.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	}

}

textures/tree/branch2
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/branch2.tga
	//	alphaFunc GE128
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen lightingdiffuse
	}

}

textures/tree/branch3
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/branch3.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen lightingdiffuse
	}
}
textures/tree/branch3a
{
	nopicmip
	//sort 10		
	cull twosided


	deformVertexes wave 194 sin 0 2 0 .1
	deformVertexes wave 30 sin 0 1 0 .3
	deformVertexes wave 194 sin 0 1 0 .4

	{
		map textures/tree/branch3a.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen identity
	}
}
textures/tree/branch3b
{
	nopicmip
	//sort 10		
	cull twosided
	{
		map textures/tree/branch3b.tga
		alphaFunc GE128
	//	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	//	blendFunc blend
		rgbGen lightingdiffuse
	}

}
textures/alpha/bench_c01
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateaubench_c01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/bench_c02
{
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateaubench_c02.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}


textures/alpha/chateau_c01
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c02
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c02.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c03
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c03.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c04
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c04.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c05
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c05.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c06
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c06.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c06a
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c06a.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/chateau_c07
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c07.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c08
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c08.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
//        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c09
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c09.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c10
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c10.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c11
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c11.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c12
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c12.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}

textures/alpha/chateau_c13
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c13.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateau_c14
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateau_c14.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateaudoor_c01
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateaudoor_c01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/alpha/chateaudoor_c02
{
    surfaceparm alphashadow
    surfaceparm metalsteps
    nomipmaps
    nopicmip
    cull disable
    {
        map textures/alpha/chateaudoor_c02.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}


textures/training/trees_m01
{

 // invalid JPW FIXME SP merge   cull front
    {  
        map textures/training/trees_m01.tga
	  blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
//      alphaFunc GE128
        depthWrite
        rgbGen identity

    }
}

textures/alpha/flor_glow
{

// invalid JPW FIXME SP merge    cull front
    {  
        map textures/alpha/flor_glow.tga
	blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        rgbGen identity
    }
}
textures/sleepy/tree_c01
{
	nopicmip
    surfaceparm metalsteps
    cull disable
    {
        map textures/sleepy/tree_c01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/sleepy/grouptree_c01
{
	nopicmip
    surfaceparm metalsteps
    cull disable
    {
        map textures/sleepy/grouptree_c01.tga
        alphaFunc GE128
        depthWrite
        rgbGen vertex
    }
}
textures/tree/snowtree_c01
{
	nopicmip
	cull twosided
    surfaceparm alphashadow
	{
		map textures/tree/snowtree_c01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}
textures/tree/snowtree_c01a
{
	nopicmip
	cull twosided
    surfaceparm alphashadow
	{
		map textures/tree/snowtree_c01a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}
textures/tree/snowtree_c02
{
	nopicmip
	cull twosided
    surfaceparm alphashadow
	{
		map textures/tree/snowtree_c02.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}
textures/tree/snowtree_c02a
{
	nopicmip
	cull twosided
    surfaceparm alphashadow
	{
		map textures/tree/snowtree_c02a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}
textures/tree/snowtree_c03
{
	nopicmip
	cull twosided
    surfaceparm alphashadow
	{
		map textures/tree/snowtree_c03.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}
textures/tree/snowtree_c03a
{
	nopicmip
	cull twosided
    surfaceparm alphashadow
	{
		map textures/tree/snowtree_c03a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}
textures/tree/snowtree_c04
{
	nopicmip
	cull twosided
    surfaceparm alphashadow
	{
		map textures/tree/snowtree_c04.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}
textures/tree/snowtree_c04a
{
	nopicmip
	cull twosided
    surfaceparm alphashadow
	{
		map textures/tree/snowtree_c04a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}

}
textures/tree/tree_c01
{
	nopicmip
	cull twosided
    surfaceparm alphashadow
	{
		map textures/tree/tree_c01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c01
{
	nopicmip
	surfaceparm alphashadow
	cull twosided
	{
		map textures/tree/ivy_c01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c01a
{
	nopicmip
	surfaceparm alphashadow		
	cull twosided
	{
		map textures/tree/ivy_c01a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c01b
{
	nopicmip
	surfaceparm alphashadow		
	cull twosided
	{
		map textures/tree/ivy_c01b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c02
{
	nopicmip
	surfaceparm alphashadow		
	cull twosided
	{
		map textures/tree/ivy_c02.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c02a
{
	nopicmip
	    surfaceparm alphashadow		
	cull twosided
	{
		map textures/tree/ivy_c02a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c02b
{
	nopicmip
	surfaceparm alphashadow		
	cull twosided
	{
		map textures/tree/ivy_c02b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}

textures/tree/ivy_c03
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c03.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c03a
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c03a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c03b
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c03b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}

textures/tree/ivy_c04
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c04.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c04a
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c04a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c04b
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c04b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c05
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c05.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}


textures/tree/ivy_c05a
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c05a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c05b
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c05b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c06
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c06.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c07
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c07.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c08
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c058.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/tree/ivy_c09
{
	nopicmip
    surfaceparm alphashadow			
	cull twosided
	{
		map textures/tree/ivy_c09.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}
textures/training/window_m02_alpha
{
//  qer_editorimage textures/training/window_m02.tga
    surfaceparm alphashadow			
	{
		map textures/training/window_m02_alpha.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		depthWrite

	}

}










