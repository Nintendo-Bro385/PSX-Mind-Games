/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfspirit.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend skull fragments
static SkullFragment char_bfs_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend player types
enum
{
	BFS_ArcMain_Idle0,
	BFS_ArcMain_Idle1,
	BFS_ArcMain_Idle2,
	BFS_ArcMain_Idle3,
	BFS_ArcMain_Left0,
	BFS_ArcMain_Left1,
	BFS_ArcMain_Left2,
	BFS_ArcMain_Left3,
	BFS_ArcMain_Down0,
	BFS_ArcMain_Down1,
	BFS_ArcMain_Up0,
	BFS_ArcMain_Up1,
	BFS_ArcMain_Right0,
	BFS_ArcMain_Right1,
	BFS_ArcMain_Miss0,
	BFS_ArcMain_Miss1,
	BFS_ArcMain_Miss2,
	BFS_ArcMain_Miss3,
	BFS_ArcMain_Leftalt0,
	BFS_ArcMain_Leftalt1,
	BFS_ArcMain_Downalt,
	BFS_ArcMain_Upalt,
	BFS_ArcMain_Rightalt,
	BFS_ArcMain_Dead0, //BREAK
	
	BFS_ArcMain_Max,
};

enum
{
	BFS_ArcDead_Dead1, //Mic Drop
	BFS_ArcDead_Dead2, //Twitch
	BFS_ArcDead_Retry, //Retry prompt
	
	BFS_ArcDead_Max,
};

#define BFS_Arc_Max BFS_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BFS_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bfs_skull)];
	u8 skull_scale;
} Char_BFS;

//Boyfriend player definitions
static const CharFrame char_bfs_frame[] = {
	{BFS_ArcMain_Idle0, {  0,   0, 104, 199}, { 42, 189}}, //0 idle 1
	{BFS_ArcMain_Idle0, {  104,   0, 104, 199}, { 42, 189}}, //1 idle 2
	{BFS_ArcMain_Idle1, {  0,   0, 104, 199}, { 42, 189}}, //2 idle 3
	{BFS_ArcMain_Idle1, {  104,   0, 104, 199}, { 42, 189}}, //3 idle 4
	{BFS_ArcMain_Idle2, {  0,   0, 104, 199}, { 42, 189}}, //4
	{BFS_ArcMain_Idle2, {  104,   0, 104, 199}, { 42, 189}}, //5
	{BFS_ArcMain_Idle3, {  0,   0, 104, 199}, { 42, 189}}, //6

	{BFS_ArcMain_Left0, {  0,   0, 136, 215}, { 74+7, 205}}, //7
	{BFS_ArcMain_Left1, {  0,   0, 136, 215}, { 74+7, 205}}, //8
	{BFS_ArcMain_Left2, {  0,   0, 136, 215}, { 74+7, 205}}, //9
	{BFS_ArcMain_Left3, {  0,   0, 136, 215}, { 74+7, 205}}, //10

	{BFS_ArcMain_Down0, {  0,   0, 128, 199}, { 66-19, 189}}, //11
	{BFS_ArcMain_Down0, {128,   0, 128, 199}, { 66-19, 189}}, //12
	{BFS_ArcMain_Down1, {  0,   0, 128, 199}, { 66-19, 189}}, //13
	{BFS_ArcMain_Down1, {128,   0, 128, 199}, { 66-19, 189}}, //14

	{BFS_ArcMain_Up0,    {  0,   0, 96, 255}, { 34-3, 245}}, //15 86
	{BFS_ArcMain_Up0,    { 96,   0, 96, 255}, { 34-3, 245}}, //16 86
	{BFS_ArcMain_Up1,    {  0,   0, 96, 255}, { 34-3, 245}}, //17 80
	{BFS_ArcMain_Up1,    { 96,   0, 96, 255}, { 34-3, 245}}, //18 87

	{BFS_ArcMain_Right0, {  0,   0, 128, 230}, { 66-36, 220}},//19
	{BFS_ArcMain_Right0, {  0,   0, 128, 230}, { 66-36, 220}},//20
	{BFS_ArcMain_Right1, {  0,   0, 128, 230}, { 66-36, 220}},//21
	{BFS_ArcMain_Right1, {  0,   0, 128, 230}, { 66-36, 220}},//22
	
	{BFS_ArcMain_Miss0, {  0,   0, 136, 215}, { 74+7, 205}},//23
	{BFS_ArcMain_Miss1, {  0,   0, 128, 204}, { 66-19, 194}},//24
	{BFS_ArcMain_Miss2, {  0,   0, 96, 255}, { 34-3, 245}},//25
	{BFS_ArcMain_Miss3, {  0,   0, 128, 230}, { 66-36, 220}},//26
	
	{BFS_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //27 dead0 0
	{BFS_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //28 dead0 1
	{BFS_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //29 dead0 2
	{BFS_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //30 dead0 3
	
	{BFS_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //31 dead1 0
	{BFS_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //32 dead1 1
	{BFS_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //33 dead1 2
	{BFS_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //34 dead1 3
	
	{BFS_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //35 dead2 body twitch 0
	{BFS_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //36 dead2 body twitch 1
	{BFS_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //37 dead2 balls twitch 0
	{BFS_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //38 dead2 balls twitch 1
	
	{BFS_ArcMain_Leftalt0, {  0,   0, 136, 215}, { 74+7, 205}},//39
	{BFS_ArcMain_Leftalt1, {  0,   0, 136, 215}, { 74+7, 205}},//40
	{BFS_ArcMain_Downalt, {  0,   0, 128, 199}, { 66-19, 189}},//41
	{BFS_ArcMain_Downalt, {128,   0, 128, 199}, { 66-19, 189}},//42
	{BFS_ArcMain_Upalt, {  0,   0, 104, 225}, { 42-8, 114}},//43
	{BFS_ArcMain_Upalt, {  104,   0, 104, 225}, { 42-8, 114}},//44
	{BFS_ArcMain_Rightalt, {  0,   0, 120, 222}, { 58-20, 212}},//45
	{BFS_ArcMain_Rightalt, {  120,   0, 120, 222}, { 58-20, 212}},//46
};

static const Animation char_bfs_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5,  6, ASCR_BACK, 1}}, //CharAnim_Idle
	{1, (const u8[]){ 7,  8,  9,  10, ASCR_BACK, 2}},             //CharAnim_Left
	{2, (const u8[]){ 39,  40, ASCR_BACK, 1}},       //CharAnim_LeftAlt
	{1, (const u8[]){ 11,  12,  13,  14, ASCR_BACK, 2}},             //CharAnim_Down
	{2, (const u8[]){41,  42, ASCR_BACK, 1}},       //CharAnim_DownAlt
	{1, (const u8[]){ 15,  16,  17,  18,  ASCR_BACK, 2}},             //CharAnim_Up
	{2, (const u8[]){43,  44, ASCR_BACK, 1}},       //CharAnim_UpAlt
	{1, (const u8[]){ 19,  20,  21,  22, ASCR_BACK, 2}},             //CharAnim_Right
	{2, (const u8[]){45,  46, ASCR_BACK, 1}},       //CharAnim_RightAlt
	
	{1, (const u8[]){ 7, 23, 23, 23, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 11, 24, 24, 24, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 15, 25, 25, 25, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){19, 26, 26, 26, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{5, (const u8[]){27, 28, 29, 30, 30, 30, 30, 30, 30, 30, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){30, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){31, 32, 33, 34, 34, 34, 34, 34, 34, 34, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){34, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){35, 36, 34, 34, 34, 34, 34, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){37, 38, 34, 34, 34, 34, 34, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){34, 34, 34, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){37, 38, 34, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_BFS_SetFrame(void *user, u8 frame)
{
	Char_BFS *this = (Char_BFS*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfs_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFS_Tick(Character *character)
{
	Char_BFS *this = (Char_BFS*)character;
	
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
		
		//Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_1_4: //Tutorial peace
					if (stage.song_step > 64 && stage.song_step < 192 && (stage.song_step & 0x3F) == 60)
						character->set_anim(character, PlayerAnim_Peace);
					break;
				case StageId_1_1: //Bopeebo peace
					if ((stage.song_step & 0x1F) == 28)
						character->set_anim(character, PlayerAnim_Peace);
					break;
				default:
					break;
			}
		}
	}
	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFS, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(56,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_BFS_SetFrame);
	Character_Draw(character, &this->tex, &char_bfs_frame[this->frame]);
}

void Char_BFS_SetAnim(Character *character, u8 anim)
{
	Char_BFS *this = (Char_BFS*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;
			
			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //BFS_ArcDead_Dead1
				"dead2.tim", //BFS_ArcDead_Dead2
				"retry.tim", //BFS_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BFS_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BFS_Free(Character *character)
{
	Char_BFS *this = (Char_BFS*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_BFS_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BFS *this = Mem_Alloc(sizeof(Char_BFS));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFS_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFS_Tick;
	this->character.set_anim = Char_BFS_SetAnim;
	this->character.free = Char_BFS_Free;
	
	Animatable_Init(&this->character.animatable, char_bfs_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 4;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFS.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFSDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",   //BFS_ArcMain_BFS0
		"idle1.tim",   //BFS_ArcMain_BFS1
		"idle2.tim",   //BFS_ArcMain_BFS2
		"idle3.tim",   //BFS_ArcMain_BFS3
		"left0.tim",   //BFS_ArcMain_BFS4
		"left1.tim",   //BFS_ArcMain_BFS5
		"left2.tim",   //BFS_ArcMain_BFS6
		"left3.tim",
		"down0.tim",
		"down1.tim",
		"up0.tim",
		"up1.tim",
		"right0.tim",
		"right1.tim",
		"miss0.tim",
		"miss1.tim",
		"miss2.tim",
		"miss3.tim",
		"leftalt0.tim",
		"leftalt1.tim",
		"downalt.tim",
		"upalt.tim",
		"rightalt.tim",
		"dead0.tim", //BFS_ArcMain_Dead0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_bfs_skull, sizeof(char_bfs_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFS, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
