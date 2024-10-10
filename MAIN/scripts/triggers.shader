gfx/2d/customTrigger
{
	cull none
	noPicmip
	surfaceparm trans
	{
		map gfx/2d/trigger.jpg
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen const ( 1 0 1 )
		alphaGen const 0.1
	}
}

gfx/2d/objTrigger
{
	cull none
	noPicmip
	surfaceparm trans
	{
		map gfx/2d/trigger.jpg
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen const ( 0 1 0 )
		alphaGen const 0.1
	}
}

gfx/2d/transmitTrigger
{
	cull none
	noPicmip
	surfaceparm trans
	{
		map gfx/2d/trigger.jpg
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen const ( 1 0 0 )
		alphaGen const 0.1
	}
}

gfx/2d/customTriggerEdges
{
	cull none
	noPicmip
	surfaceparm trans
	{
		map gfx/2d/trigger.jpg
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen const ( 1 0 1 )
		alphaGen const 1
	}
}

gfx/2d/objTriggerEdges
{
	cull none
	noPicmip
	surfaceparm trans
	{
		map gfx/2d/trigger.jpg
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen const ( 0 1 0 )
		alphaGen const 1
	}
}

gfx/2d/transmitTriggerEdges
{
	cull none
	noPicmip
	surfaceparm trans
	{
		map gfx/2d/trigger.jpg
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen const ( 1 0 0 )
		alphaGen const 1
	}
}