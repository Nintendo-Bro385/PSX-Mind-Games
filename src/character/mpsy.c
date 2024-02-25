/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mpsy.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Mpsy character structure
enum
{
	Mpsy_ArcMain_Sheet,
	
	Mpsy_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Mpsy_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Mpsy;

//Mpsy character definitions
static const CharFrame char_Mpsy_frame[] = {
	{Mpsy_ArcMain_Sheet, {  0,   0,  62,  95}, { 42, 183}}, //0 idle 1
	{Mpsy_ArcMain_Sheet, { 62,   0,  61,  95}, { 42, 184}}, //1 idle 2
	{Mpsy_ArcMain_Sheet, {123,   0,  60,  95}, { 41, 184}}, //2 idle 3
	{Mpsy_ArcMain_Sheet, {183,   0,  60,  97}, { 41, 185}}, //3 idle 4
};

static const Animation char_Mpsy_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Mpsy character functions
void Char_Mpsy_SetFrame(void *user, u8 frame)
{
	Char_Mpsy *this = (Char_Mpsy*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_Mpsy_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Mpsy_Tick(Character *character)
{
	Char_Mpsy *this = (Char_Mpsy*)character;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{	
		//Perform dance
		if (stage.note_scroll >= character->sing_end && (stage.song_step % stage.gf_speed) == 0)
			character->set_anim(character, CharAnim_Idle);
		
	}
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Mpsy_SetFrame);
	Character_Draw(character, &this->tex, &char_Mpsy_frame[this->frame]);
}

void Char_Mpsy_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Mpsy_Free(Character *character)
{
	Char_Mpsy *this = (Char_Mpsy*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Mpsy_New(fixed_t x, fixed_t y)
{
	//Allocate Mpsy object
	Char_Mpsy *this = Mem_Alloc(sizeof(Char_Mpsy));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Mpsy_New] Failed to allocate Mpsy object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Mpsy_Tick;
	this->character.set_anim = Char_Mpsy_SetAnim;
	this->character.free = Char_Mpsy_Free;
	
	Animatable_Init(&this->character.animatable, char_Mpsy_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MPSY.ARC;1");
	
	const char **pathp = (const char *[]){
		"mpsy.tim", //Mpsy_ArcMain_Idle0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
