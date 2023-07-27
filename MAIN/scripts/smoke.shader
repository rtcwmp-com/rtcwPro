// SHADER for Airstrike and Artillery Smoke with nopicmip option

smokePuffPro
{
	nopicmip
	nofog
	cull none
	entityMergable		// allow all the sprites to be merged together
	{
		map gfx/misc/smokepuff.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen		vertex
		alphaGen	vertex
	}
}

// Rafael - black smoke
// prerotated texture for smoke
smokePuffblackPro1
{
//	nofog
	nopicmip
	cull none
	{
		map gfx/misc/smokepuff_b1.tga
//		blendFunc GL_ZERO GL_ONE_MINUS_SRC_ALPHA
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA	// (SA) I put it back for DM
		alphaGen vertex
	}

}

smokePuffblackPro2
{
//	nofog
	nopicmip
	cull none
	{
		map gfx/misc/smokepuff_b2.tga
//		blendFunc GL_ZERO GL_ONE_MINUS_SRC_ALPHA
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA	// (SA) I put it back for DM
		alphaGen vertex
	}

}

smokePuffblackPro3
{
//	nofog
	nopicmip
	cull none
	{
		map gfx/misc/smokepuff_b3.tga
//		blendFunc GL_ZERO GL_ONE_MINUS_SRC_ALPHA
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA	// (SA) I put it back for DM
		alphaGen vertex
	}

}

smokePuffblackPro4
{
//	nofog
	nopicmip
	cull none
	{
		map gfx/misc/smokepuff_b4.tga
//		blendFunc GL_ZERO GL_ONE_MINUS_SRC_ALPHA
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA	// (SA) I put it back for DM
		alphaGen vertex
	}

}

smokePuffblackPro5
{
//	nofog
	nopicmip
	cull none
	{
		map gfx/misc/smokepuff_b5.tga
//		blendFunc GL_ZERO GL_ONE_MINUS_SRC_ALPHA
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA	// (SA) I put it back for DM
		alphaGen vertex
	}

}
// done