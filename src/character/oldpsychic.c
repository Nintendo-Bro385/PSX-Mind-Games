/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "oldpsychic.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend player types
enum
{
	OldPsychic_ArcMain_Idle0,
	OldPsychic_ArcMain_Idle1,
	OldPsychic_ArcMain_Idle2,
	OldPsychic_ArcMain_Idle3,
	OldPsychic_ArcMain_Idle4,
	OldPsychic_ArcMain_Idle5,
	OldPsychic_ArcMain_Idle6,
	OldPsychic_ArcMain_Idle7,
	OldPsychic_ArcMain_Left0,
	OldPsychic_ArcMain_Left1,
	OldPsychic_ArcMain_Down0,
	OldPsychic_ArcMain_Down1,
	OldPsychic_ArcMain_Up,
	OldPsychic_ArcMain_Right0,
	OldPsychic_ArcMain_Right1,
	
	OldPsychic_Arc_Max,
};


typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[OldPsychic_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;

} Char_OldPsychic;

//Boyfriend player definitions
static const CharFrame char_oldpsychic_frame[] = {
	{OldPsychic_ArcMain_Idle0, {  0,   0, 142, 224}, { 42, 195}}, //0 idle 1
	{OldPsychic_ArcMain_Idle1, {  0,   0, 142, 224}, { 42, 195}}, //1 idle 2
	{OldPsychic_ArcMain_Idle2, {  0,   0, 142, 224}, { 42, 195}}, //2 idle 3
	{OldPsychic_ArcMain_Idle3, {  0,   0, 142, 224}, { 42, 195}}, //3 idle 4
	{OldPsychic_ArcMain_Idle4, {  0,   0, 142, 224}, { 42, 195}}, //4
	{OldPsychic_ArcMain_Idle5, {  0,   0, 142, 224}, { 42, 195}}, //5
	{OldPsychic_ArcMain_Idle6, {  0,   0, 142, 224}, { 42, 195}}, //6
	{OldPsychic_ArcMain_Idle7, {  0,   0, 142, 224}, { 42, 195}}, //7

	{OldPsychic_ArcMain_Left0, {  0,   0, 116, 226}, { 14+31, 197-2}}, //8
	{OldPsychic_ArcMain_Left0, {116,   0, 116, 226}, { 14+31, 197-2}}, //9
	{OldPsychic_ArcMain_Left1, {  0,   0, 116, 226}, { 14+31, 197-2}}, //10
	{OldPsychic_ArcMain_Left1, {116,   0, 116, 226}, { 14+31, 197-2}}, //11

	{OldPsychic_ArcMain_Down0, {  0,   0, 128, 194}, { 26+22, 167-2}}, //12
	{OldPsychic_ArcMain_Down0, {128,   0, 128, 194}, { 26+20, 167-2}}, //13
	{OldPsychic_ArcMain_Down1, {  0,   0, 128, 194}, { 26+21, 167-2}}, //14
	{OldPsychic_ArcMain_Down1, {128,   0, 128, 194}, { 26+21, 167-2}}, //15

	{OldPsychic_ArcMain_Up,    {  0,   0, 108, 236}, { 6+25, 209-4}}, //16
	{OldPsychic_ArcMain_Up,    {108,   0, 108, 236}, { 6+25, 209-4}}, //17

	{OldPsychic_ArcMain_Right0, {  0,   0, 138, 210}, { 34+6, 179-2}},//18
	{OldPsychic_ArcMain_Right1, {  0,   0, 138, 210}, { 34+6, 179-2}},//19

};

static const Animation char_oldpsychic_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, ASCR_BACK, 3}}, //CharAnim_Idle
	{2, (const u8[]){ 8, 9, 10, 11, ASCR_BACK, 2}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 2}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 16, 17, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 18, 19, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
	

};

//Boyfriend player functions
void Char_OldPsychic_SetFrame(void *user, u8 frame)
{
	Char_OldPsychic *this = (Char_OldPsychic*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_oldpsychic_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_OldPsychic_Tick(Character *character)
{
	Char_OldPsychic *this = (Char_OldPsychic*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_OldPsychic_SetFrame);
	Character_Draw(character, &this->tex, &char_oldpsychic_frame[this->frame]);
}

void Char_OldPsychic_SetAnim(Character *character, u8 anim)
{
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_OldPsychic_Free(Character *character)
{
	Char_OldPsychic *this = (Char_OldPsychic*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_OldPsychic_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_OldPsychic *this = Mem_Alloc(sizeof(Char_OldPsychic));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_OldPsychic_New] Failed to allocate oldpsychic object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_OldPsychic_Tick;
	this->character.set_anim = Char_OldPsychic_SetAnim;
	this->character.free = Char_OldPsychic_Free;
	
	Animatable_Init(&this->character.animatable, char_oldpsychic_anim);
	Character_Init((Character*)this, x, y);
	
	this->character.health_i = 2;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-87,1);
	this->character.focus_zoom = FIXED_DEC(1,1);

	//Load note sprites	
	this->arc_main = IO_Read("\\CHAR\\PSYCHICO.ARC;1");

	const char **pathp = (const char *[]){
		"idle0.tim", 
		"idle1.tim", 
		"idle2.tim", 
		"idle3.tim", 
		"idle4.tim", 
		"idle5.tim", 
		"idle6.tim", 
		"idle7.tim", 
		"left0.tim", 
		"left1.tim", 
		"down0.tim", 
		"down1.tim", 
		"up.tim", 
		"right0.tim", 
		"right1.tim", 
		NULL
		};

	IO_Data *arc_ptr = &this->arc_ptr[OldPsychic_ArcMain_Idle0];
	for (; *pathp != NULL; pathp++)
	*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
