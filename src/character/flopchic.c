/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "flopchic.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend player types
enum
{
	Flopchic_ArcMain_Idle0,
	Flopchic_ArcMain_Idle1,
	Flopchic_ArcMain_Left0,
	Flopchic_ArcMain_Down0,
	Flopchic_ArcMain_Up0,
	Flopchic_ArcMain_Right0,
	
	Flopchic_Arc_Max,
};


typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Flopchic_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;

} Char_Flopchic;

//Boyfriend player definitions
static const CharFrame char_flopchic_frame[] = {
	{Flopchic_ArcMain_Idle0, {  0,   0, 105, 175}, { 42, 195}}, //0 idle 1
	{Flopchic_ArcMain_Idle0, {105,   0, 105, 175}, { 42, 195}}, //1 idle 2
	{Flopchic_ArcMain_Idle1, {  0,   0, 105, 175}, { 42, 195}}, //2 idle 2
	{Flopchic_ArcMain_Idle1, {105,   0, 105, 175}, { 42, 195}}, //3 idle 2


	{Flopchic_ArcMain_Left0, {  0,   0, 105, 175}, { 42, 195}}, //4
	{Flopchic_ArcMain_Left0, {105,   0, 105, 175}, { 42, 195}}, //5

	{Flopchic_ArcMain_Down0, {  0,   0, 105, 175}, { 42, 195}}, //6
	{Flopchic_ArcMain_Down0, {105,   0, 105, 175}, { 42, 195}}, //7

	{Flopchic_ArcMain_Up0,    {  0,   0, 105, 175}, { 42, 195}}, //8
	{Flopchic_ArcMain_Up0,    {105,   0, 105, 175}, { 42, 195}}, //9

	{Flopchic_ArcMain_Right0, {  0,   0, 120, 175}, { 42, 195}},//10
	{Flopchic_ArcMain_Right0, {120,   0, 120, 175}, { 42, 195}},//11

};

static const Animation char_flopchic_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, ASCR_BACK, 3}}, //CharAnim_Idle
	{2, (const u8[]){ 4, 5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6, 7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8, 9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
	

};

//Boyfriend player functions
void Char_Flopchic_SetFrame(void *user, u8 frame)
{
	Char_Flopchic *this = (Char_Flopchic*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_flopchic_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Flopchic_Tick(Character *character)
{
	Char_Flopchic *this = (Char_Flopchic*)character;
	
	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_Flopchic_SetFrame);
	Character_Draw(character, &this->tex, &char_flopchic_frame[this->frame]);
}

void Char_Flopchic_SetAnim(Character *character, u8 anim)
{
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Flopchic_Free(Character *character)
{
	Char_Flopchic *this = (Char_Flopchic*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Flopchic_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_Flopchic *this = Mem_Alloc(sizeof(Char_Flopchic));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Flopchic_New] Failed to allocate flopchic object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Flopchic_Tick;
	this->character.set_anim = Char_Flopchic_SetAnim;
	this->character.free = Char_Flopchic_Free;
	
	Animatable_Init(&this->character.animatable, char_flopchic_anim);
	Character_Init((Character*)this, x, y);
	
	this->character.health_i = 5;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-95,1);
	this->character.focus_zoom = FIXED_DEC(1,1);

	//Load note sprites	
	this->arc_main = IO_Read("\\CHAR\\FLOPCHIC.ARC;1");

	const char **pathp = (const char *[]){
		"idle0.tim", 
		"idle1.tim",  
		"left0.tim", 
		"down0.tim", 
		"up0.tim", 
		"right0.tim", 
		NULL
		};

	IO_Data *arc_ptr = &this->arc_ptr[Flopchic_ArcMain_Idle0];
	for (; *pathp != NULL; pathp++)
	*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
