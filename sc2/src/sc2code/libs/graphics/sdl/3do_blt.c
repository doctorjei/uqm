//Copyright Paul Reiche, Fred Ford. 1992-2002

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef GFXMODULE_SDL

#include "sdl_common.h"
#include "graphics/tfb_draw.h"

#define GSCALE_SHIFT 2 // controls the number of scale steps
#define GSCALE_IDENTITY (256 >> GSCALE_SHIFT) // 'identity scale'

static int gscale = GSCALE_IDENTITY;

void
SetGraphicScale (int scale)
{
	gscale = scale >> GSCALE_SHIFT;
	if (gscale == 0)
		gscale = GSCALE_IDENTITY;
}

int
GetGraphicScale ()
{
	return gscale;
}

int
GetGraphicScaleIdentity (void) 
{
	return GSCALE_IDENTITY;
}

static void
read_screen (PRECT lpRect, FRAMEPTR DstFramePtr)
{
	if (TYPE_GET (_CurFramePtr->TypeIndexAndFlags) != SCREEN_DRAWABLE
			|| TYPE_GET (DstFramePtr->TypeIndexAndFlags) == SCREEN_DRAWABLE
			|| !(TYPE_GET (GetFrameParentDrawable (DstFramePtr)
			->FlagsAndIndex)
			& ((DWORD) MAPPED_TO_DISPLAY << FTYPE_SHIFT)))
	{
		fprintf (stderr, "Unimplemented function activated: read_screen()\n");
	}
	else
	{
		TFB_Image *img = DstFramePtr->image;
		TFB_DrawScreen_CopyToImage (img, lpRect, TFB_SCREEN_MAIN);
	}
}

static DRAWABLE
alloc_image (COUNT NumFrames, DRAWABLE_TYPE DrawableType, CREATE_FLAGS
		flags, SIZE width, SIZE height)
{
        /* dodge compiler warnings */
	(void)DrawableType;
	(void)flags;
	(void)width;
	(void)height;
	return AllocDrawable (NumFrames, 0);
}

static DISPLAY_INTERFACE DisplayInterface =
{
	WANT_MASK,

	16, // SCREEN_DEPTH,
	320,
	240,

	alloc_image,
	read_screen,
};

void
LoadDisplay (PDISPLAY_INTERFACE *pDisplay)
{
	*pDisplay = &DisplayInterface;
}

#endif
