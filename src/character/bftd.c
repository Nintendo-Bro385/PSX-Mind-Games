/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bftd.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Bftd character structure
enum
{
	Bftd_ArcMain_Idle0,
	Bftd_ArcMain_Idle1,
	Bftd_ArcMain_Idle2,
	Bftd_ArcMain_Idle3,
	
	Bftd_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Bftd_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Bftd;

//Bftd character definitions
static const CharFrame char_bftd_frame[] = {
	{Bftd_ArcMain_Idle0, {  0,   0, 122, 111}, { 53,  92}}, //0
	{Bftd_ArcMain_Idle0, {  122,   0, 122, 111}, { 53,  92}}, //1
	{Bftd_ArcMain_Idle0, {  0,   111, 122, 111}, { 53,  92}}, //2
	{Bftd_ArcMain_Idle0, {  122, 111, 122, 111}, { 53,  92}}, //3
	
	{Bftd_ArcMain_Idle1, {  0,   0, 122, 111}, { 53,  92}}, //4
	{Bftd_ArcMain_Idle1, {  122,   0, 122, 111}, { 53,  92}}, //5
	{Bftd_ArcMain_Idle1, {  0,   111, 122, 111}, { 53,  92}}, //6
	{Bftd_ArcMain_Idle1, {  122, 111, 122, 111}, { 53,  92}}, //7
	
	{Bftd_ArcMain_Idle2, {  0,   0, 122, 111}, { 53,  92}}, //8
	{Bftd_ArcMain_Idle2, {  122,   0, 122, 111}, { 53,  92}}, //9
	{Bftd_ArcMain_Idle2, {  0,   111, 122, 111}, { 53,  92}}, //10
	{Bftd_ArcMain_Idle2, {  122, 111, 122, 111}, { 53,  92}}, //11
	
	{Bftd_ArcMain_Idle3, {  0,   0, 122, 111}, { 53,  92}}, //12
	{Bftd_ArcMain_Idle3, {  122,   0, 122, 111}, { 53,  92}}, //13
	{Bftd_ArcMain_Idle3, {  0,   111, 122, 111}, { 53,  92}}, //14 Scream
	{Bftd_ArcMain_Idle3, {  122, 111, 122, 111}, { 53,  92}}, //15 Scream


};

static const Animation char_bftd_anim[CharAnim_Max] = {
	{1, (const u8[]){ 0, ASCR_BACK, 1}}, //CharAnim_Idle
	{3, (const u8[]){ 2, 3, 4, 5, 6, 7, ASCR_BACK, 6}},         //CharAnim_Left shake
	{2, (const u8[]){ 1, ASCR_BACK, 1}},   //CharAnim_LeftAlt
	{5, (const u8[]){ 12, 13, ASCR_BACK, 2}},         //CharAnim_Down shake up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8, 9, 10, 11, ASCR_BACK, 4}},         //CharAnim_Up 
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{1, (const u8[]){ 14, 15, ASCR_BACK, 2}},         //CharAnim_Right scream
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Bftd character functions
void Char_Bftd_SetFrame(void *user, u8 frame)
{
	Char_Bftd *this = (Char_Bftd*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bftd_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Bftd_Tick(Character *character)
{
	Char_Bftd *this = (Char_Bftd*)character;
	
	//Perform idle dance
	if (stage.song_step <= 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Bftd_SetFrame);
	Character_Draw(character, &this->tex, &char_bftd_frame[this->frame]);

	if (stage.flag & STAGE_FLAG_JUST_STEP)
    {   //Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_3_1: //Animations
				{
					switch (stage.song_step)
						{
							case 10:
								this->character.focus_x = FIXED_DEC(-16,1);
								this->character.focus_y = FIXED_DEC(-55,1);
								this->character.focus_zoom = FIXED_DEC(10,10);
								stage.camode = 1;
								break;
							case 21: 
								character->set_anim(character, CharAnim_LeftAlt);
								break;
							case 22: 
								character->set_anim(character, CharAnim_Left);
								break;
							case 44:
								character->set_anim(character, CharAnim_Up);
								break;
							case 46:
								character->set_anim(character, CharAnim_Down);
								break;
							case 70:
								character->set_anim(character, CharAnim_Right);
								break;
						}
				}
			}
		}
	}
}

void Char_Bftd_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Bftd_Free(Character *character)
{
	Char_Bftd *this = (Char_Bftd*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Bftd_New(fixed_t x, fixed_t y)
{
	//Allocate bftd object
	Char_Bftd *this = Mem_Alloc(sizeof(Char_Bftd));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Bftd_New] Failed to allocate bftd object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Bftd_Tick;
	this->character.set_anim = Char_Bftd_SetAnim;
	this->character.free = Char_Bftd_Free;
	
	Animatable_Init(&this->character.animatable, char_bftd_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 2;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-70,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SCREAM.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",
		"idle1.tim",
		"idle2.tim",
		"idle3.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
