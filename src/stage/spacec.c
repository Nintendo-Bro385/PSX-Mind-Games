/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "spacec.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

//Week 4 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Wall left
	Gfx_Tex tex_back1; //Wall right

	Gfx_Tex tex_floor; //Floor
	
	Gfx_Tex tex_voyager1;
	Gfx_Tex tex_voyager2;
	
	
	
} Back_Space;

void Space_Wickp_SetFrame(void *user, u8 frame)
{
	Back_Space *this = (Back_Space*)user;
	

}
void Back_Space_DrawBG(StageBack *back)
{
	Back_Space *this = (Back_Space*)back;
	

	
	fixed_t fx, fy;
	
	
	
	//Draw room
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT halll_src = {0, 0, 255, 256};
	RECT_FIXED halll_dst = {
		FIXED_DEC(-165 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-100,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	RECT hallr_src = {0, 0, 256, 256};
	RECT_FIXED hallr_dst = {
		FIXED_DEC(90 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-100,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	RECT floorl_src = {0, 0, 255, 105};
	RECT_FIXED floorl_dst = {
		FIXED_DEC(-165 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(76,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(128,1)
	};

	RECT floorr_src = {0, 105, 255, 105};
	RECT_FIXED floorr_dst = {
		FIXED_DEC(91 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(74,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(128,1)
	};
	
	RECT voyl_src = {0, 0, 193, 225};
	RECT_FIXED voyl_dst = {
		FIXED_DEC(-210 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-161,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(340,1)
	};

	RECT voyr_src = {0, 0, 177, 154};
	RECT_FIXED voyr_dst = {
		FIXED_DEC(120 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-131,1) - fy,
		FIXED_DEC(234 + SCREEN_WIDEOADD,1),
		FIXED_DEC(191,1)
	};
			if (stage.prefs.lowquality ==0)
			{
			Stage_DrawTex(&this->tex_voyager1, &voyl_src, &voyl_dst, stage.camera.bzoom);
			Stage_DrawTex(&this->tex_voyager2, &voyr_src, &voyr_dst, stage.camera.bzoom);
			Stage_DrawTex(&this->tex_floor, &floorl_src, &floorl_dst, stage.camera.bzoom);
			Stage_DrawTex(&this->tex_floor, &floorr_src, &floorr_dst, stage.camera.bzoom);
			Stage_DrawTex(&this->tex_back0, &halll_src, &halll_dst, stage.camera.bzoom);
			Stage_DrawTex(&this->tex_back1, &hallr_src, &hallr_dst, stage.camera.bzoom);
			}


}


void Back_Space_Free(StageBack *back)
{
	Back_Space *this = (Back_Space*)back;
	
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Space_New(void)
{
	//Allocate background structure
	Back_Space *this = (Back_Space*)Mem_Alloc(sizeof(Back_Space));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Space_DrawBG;
	this->back.free = Back_Space_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\PSYCHE\\SPACE.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_floor, Archive_Find(arc_back, "floor.tim"), 0);
	Gfx_LoadTex(&this->tex_voyager1, Archive_Find(arc_back, "voyager1.tim"), 0);
	Gfx_LoadTex(&this->tex_voyager2, Archive_Find(arc_back, "voyager2.tim"), 0);
	Mem_Free(arc_back);

	Gfx_SetClear(6, 183, 170);
	
	
	
	return (StageBack*)this;
}
