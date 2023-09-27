/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "stage.h"

#include "mem.h"
#include "timer.h"
#include "audio.h"
#include "pad.h"
#include "main.h"
#include "random.h"
#include "movie.h"
#include "network.h"
#include "save.h"
//#include "pause.h"

#include "menu.h"
#include "trans.h"
#include "loadscr.h"
#include "str.h"

#include "object/combo.h"
#include "object/splash.h"

//Stage constants
boolean shakey; //Uproar note shake
boolean noheadbump;
u8 cambump;

boolean loadonce;
const char *pausediff = "EASY";
const char *anotherfuckingvarible = "EASY";
const char *pausestage = "PSYCHIC";
int pausediff2;

boolean normo;
boolean paused;

//Stage constants
//#define STAGE_PERFECT //Play all notes perfectly
//#define STAGE_NOHUD //Disable the HUD

//#define STAGE_FREECAM //Freecam
int stopdownscroll;
//welcome to the shitshow
int note_x[8] = {
    //BF
     FIXED_DEC(26,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 26
     FIXED_DEC(60,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 60
     FIXED_DEC(94,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 94
     FIXED_DEC(128,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 128
    //Opponent
     FIXED_DEC(-128,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -128
     FIXED_DEC(-94,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -94
     FIXED_DEC(-60,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -60
     FIXED_DEC(-26,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -26
};

int note_flip[8] = {
    //BF
     FIXED_DEC(-128,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 26
     FIXED_DEC(-94,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 60
     FIXED_DEC(-60,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 94
     FIXED_DEC(-26,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 128
    //Opponent
     FIXED_DEC(26,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -128
     FIXED_DEC(60,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -94
     FIXED_DEC(94,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -60
     FIXED_DEC(128,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -26
};
//for noflip notes
int note_norm[8] = {
    //BF
     FIXED_DEC(26,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 26
     FIXED_DEC(60,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 60
     FIXED_DEC(94,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 94
     FIXED_DEC(128,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 128
    //Opponent
     FIXED_DEC(-128,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -128
     FIXED_DEC(-94,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -94
     FIXED_DEC(-60,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -60
     FIXED_DEC(-26,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -26
};

//static const fixed_t note_y = FIXED_DEC(32 - SCREEN_HEIGHT2, 1);

int note_y[8] = {
    //BF
    FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
    FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
    FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
    FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
    //Opponent
    FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
    FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
    FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
    FIXED_DEC(32 - SCREEN_HEIGHT2, 1),
};

static const u16 note_key[] = {INPUT_LEFT, INPUT_DOWN, INPUT_UP, INPUT_RIGHT};
static const u8 note_anims[4][3] = {
    {CharAnim_Left,  CharAnim_LeftAlt,  PlayerAnim_LeftMiss},
    {CharAnim_Down,  CharAnim_DownAlt,  PlayerAnim_DownMiss},
    {CharAnim_Up,    CharAnim_UpAlt,    PlayerAnim_UpMiss},
    {CharAnim_Right, CharAnim_RightAlt, PlayerAnim_RightMiss},
};

//Stage definitions
boolean nohud;

#include "character/bf.h"
#include "character/bfweeb.h"
#include "character/gf.h"
#include "character/gfweeb.h"
#include "character/clucky.h"
#include "character/oldpsychic.h"
#include "character/psychic.h"
#include "character/senpaib.h"
#include "character/bfspirit.h"
#include "character/sendai.h"
#include "character/bft.h"

#include "stage/dummy.h"
#include "stage/week1.h"
#include "stage/chop.h"
#include "stage/fchop.h"
#include "stage/flamec.h"
#include "stage/old.h"
#include "stage/spacec.h"

static void Menu_DrawSavingicon(s32 x, s32 y)
{
    //Draw Track
    RECT save_src = {192, 224, 32, 32};
    Gfx_BlitTex(&stage.tex_saving, &save_src, x, y);
}
static const StageDef stage_defs[StageId_Max] = {
    #include "stagedef_disc1.h"
};

//Stage state
Stage stage;

//Stage music functions
static void Stage_StartVocal(void)
{
    if (!(stage.flag & STAGE_FLAG_VOCAL_ACTIVE))
    {
        Audio_ChannelXA(stage.stage_def->music_channel);
        stage.flag |= STAGE_FLAG_VOCAL_ACTIVE;
    }
}

static void Stage_CutVocal(void)
{
    if (stage.flag & STAGE_FLAG_VOCAL_ACTIVE)
    {
        Audio_ChannelXA(stage.stage_def->music_channel + 1);
        stage.flag &= ~STAGE_FLAG_VOCAL_ACTIVE;
    }
}

//Stage camera functions
static void Stage_FocusCharacter(Character *ch, fixed_t div)
{
    //Use character focus settings to update target position and zoom
    stage.camera.tx = ch->x + ch->focus_x;
    stage.camera.ty = ch->y + ch->focus_y;
    stage.camera.tz = ch->focus_zoom;
    stage.camera.td = div;
}

static void Stage_ScrollCamera(void)
{
    #ifdef STAGE_FREECAM
        if (pad_state.held & PAD_LEFT)
            stage.camera.x -= FIXED_DEC(2,1);
        if (pad_state.held & PAD_UP)
            stage.camera.y -= FIXED_DEC(2,1);
        if (pad_state.held & PAD_RIGHT)
            stage.camera.x += FIXED_DEC(2,1);
        if (pad_state.held & PAD_DOWN)
            stage.camera.y += FIXED_DEC(2,1);
        if (pad_state.held & PAD_TRIANGLE)
            stage.camera.zoom -= FIXED_DEC(1,100);
        if (pad_state.held & PAD_CROSS)
            stage.camera.zoom += FIXED_DEC(1,100);
    #else
        //Get delta position
        fixed_t dx = stage.camera.tx - stage.camera.x;
        fixed_t dy = stage.camera.ty - stage.camera.y;
        fixed_t dz = stage.camera.tz - stage.camera.zoom;
        
        //Scroll based off current divisor
        stage.camera.x += FIXED_MUL(dx, stage.camera.td);
        stage.camera.y += FIXED_MUL(dy, stage.camera.td);
        stage.camera.zoom += FIXED_MUL(dz, stage.camera.td);
    #endif
    
    //Scroll based off current divisor
		switch (stage.camode)
		{
			case 0: //normal
			{
				stage.camera.x += FIXED_MUL(dx, stage.camera.td);
				stage.camera.y += FIXED_MUL(dy, stage.camera.td);
				stage.camera.zoom += FIXED_MUL(dz, stage.camera.td);
				break;
			}
			case 1: //slow
			{
				stage.camera.x += FIXED_MUL(dx, stage.camera.td) / 8;
				stage.camera.y += FIXED_MUL(dy, stage.camera.td) / 8;
				stage.camera.zoom += FIXED_MUL(dz, stage.camera.td) / 8;
				break;
			}
			case 2: //fast
			{
				stage.camera.x += FIXED_MUL(dx, stage.camera.td) * 2;
				stage.camera.y += FIXED_MUL(dy, stage.camera.td) * 2;
				stage.camera.zoom += FIXED_MUL(dz, stage.camera.td) * 2;
			}
		}
    
    //Update other camera stuff
    stage.camera.bzoom = FIXED_MUL(stage.camera.zoom, stage.bump);
}

//Stage section functions
static void Stage_ChangeBPM(u16 bpm, u16 step)
{
    //Update last BPM
    stage.last_bpm = bpm;
    
    //Update timing base
    if (stage.step_crochet)
        stage.time_base += FIXED_DIV(((fixed_t)step - stage.step_base) << FIXED_SHIFT, stage.step_crochet);
    stage.step_base = step;
    
    //Get new crochet and times
    stage.step_crochet = ((fixed_t)bpm << FIXED_SHIFT) * 8 / 240; //15/12/24
    stage.step_time = FIXED_DIV(FIXED_DEC(12,1), stage.step_crochet);
    
    //Get new crochet based values
    stage.early_safe = stage.late_safe = stage.step_crochet / 6; //10 frames
    stage.late_sus_safe = stage.late_safe;
    stage.early_sus_safe = stage.early_safe * 2 / 5;
}

static Section *Stage_GetPrevSection(Section *section)
{
    if (section > stage.sections)
        return section - 1;
    return NULL;
}

static u16 Stage_GetSectionStart(Section *section)
{
    Section *prev = Stage_GetPrevSection(section);
    if (prev == NULL)
        return 0;
    return prev->end;
}

//Section scroll structure
typedef struct
{
    fixed_t start;   //Seconds
    fixed_t length;  //Seconds
    u16 start_step;  //Sub-steps
    u16 length_step; //Sub-steps
    
    fixed_t size; //Note height
} SectionScroll;

static void Stage_GetSectionScroll(SectionScroll *scroll, Section *section)
{
    //Get BPM
    u16 bpm = section->flag & SECTION_FLAG_BPM_MASK;
    
    //Get section step info
    scroll->start_step = Stage_GetSectionStart(section);
    scroll->length_step = section->end - scroll->start_step;
    
    //Get section time length
    scroll->length = (scroll->length_step * FIXED_DEC(15,1) / 12) * 24 / bpm;
    
    //Get note height
    scroll->size = FIXED_MUL(stage.speed, scroll->length * (12 * 150) / scroll->length_step) + FIXED_UNIT;
}

//Note hit detection
static u8 Stage_HitNote(PlayerState *this, u8 type, fixed_t offset)
{
    //Get hit type
    if (offset < 0)
        offset = -offset;
    
    u8 hit_type;
    if (offset > stage.late_safe * 9 / 11)
        hit_type = 3; //SHIT
    else if (offset > stage.late_safe * 6 / 11)
        hit_type = 2; //BAD
    else if (offset > stage.late_safe * 3 / 11)
        hit_type = 1; //GOOD
    else
        hit_type = 0; //SICK
    
    //Increment combo and score
    this->combo++;
    
    static const s32 score_inc[] = {
        35, //SICK
        20, //GOOD
        10, //BAD
         5, //SHIT
    };
    this->score += score_inc[hit_type];
    this->refresh_score = true;
    
    //Restore vocals and health
    Stage_StartVocal();
    this->health += 230;
    
    //Create combo object telling of our combo
    Obj_Combo *combo = Obj_Combo_New(
        this->character->focus_x,
        this->character->focus_y,
        hit_type,
        this->combo >= 10 ? this->combo : 0xFFFF
    );
    if (combo != NULL)
        ObjectList_Add(&stage.objlist_fg, (Object*)combo);
    
    //Create note splashes if SICK
    if (hit_type == 0)
    {
        for (int i = 0; i < 3; i++)
        {
            //Create splash object
            Obj_Splash *splash = Obj_Splash_New(
                
                note_x[type ^ stage.note_swap],
                note_y[type ^ stage.note_swap] * (stage.prefs.downscroll ? -1 : 1),
                type & 0x3
            );
            if (splash != NULL)
                ObjectList_Add(&stage.objlist_splash, (Object*)splash);
        }
    }
    
    return hit_type;
}

static void Stage_MissNote(PlayerState *this)
{
    if (this->combo)
    {
        //Kill combo
        if (stage.gf != NULL && this->combo > 5)
            stage.gf->set_anim(stage.gf, CharAnim_DownAlt); //Cry if we lost a large combo
        this->combo = 0;
        
        //Create combo object telling of our lost combo
        Obj_Combo *combo = Obj_Combo_New(
            this->character->focus_x,
            this->character->focus_y,
            0xFF,
            0
        );
        if (combo != NULL)
            ObjectList_Add(&stage.objlist_fg, (Object*)combo);
    }
}

static void Stage_NoteCheck(PlayerState *this, u8 type)
{
    //Perform note check
    for (Note *note = stage.cur_note;; note++)
    {
        if (!(note->type & NOTE_FLAG_MINE))
        {
            //Check if note can be hit
            fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
            if (note_fp - stage.early_safe > stage.note_scroll)
                break;
            if (note_fp + stage.late_safe < stage.note_scroll)
                continue;
            if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
                continue;
            
            //Hit the note
            note->type |= NOTE_FLAG_HIT;
            
            this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
            u8 hit_type = Stage_HitNote(this, type, stage.note_scroll - note_fp);
            this->arrow_hitan[type & 0x3] = stage.step_time;
            
            #ifdef PSXF_NETWORK
                if (stage.mode >= StageMode_Net1)
                {
                    //Send note hit packet
                    Packet note_hit;
                    note_hit[0] = PacketType_NoteHit;
                    
                    u16 note_i = note - stage.notes;
                    note_hit[1] = note_i >> 0;
                    note_hit[2] = note_i >> 8;
                    
                    note_hit[3] = this->score >> 0;
                    note_hit[4] = this->score >> 8;
                    note_hit[5] = this->score >> 16;
                    note_hit[6] = this->score >> 24;
                    
                    note_hit[7] = hit_type;
                    
                    note_hit[8] = this->combo >> 0;
                    note_hit[9] = this->combo >> 8;
                    
                    Network_Send(&note_hit);
                }
            #else
                (void)hit_type;
            #endif
            return;
        }
        else
        {
            //Check if mine can be hit
            fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
            if (note_fp - (stage.late_safe * 3 / 5) > stage.note_scroll)
                break;
            if (note_fp + (stage.late_safe * 2 / 5) < stage.note_scroll)
                continue;
            if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
                continue;
            
            //Hit the mine
            note->type |= NOTE_FLAG_HIT;
            
            if (this->character->spec & CHAR_SPEC_MISSANIM)
                this->character->set_anim(this->character, note_anims[type & 0x3][2]);
            else
                this->character->set_anim(this->character, note_anims[type & 0x3][0]);
            this->arrow_hitan[type & 0x3] = -1;
            
            #ifdef PSXF_NETWORK
                if (stage.mode >= StageMode_Net1)
                {
                    //Send note hit packet
                    Packet note_hit;
                    note_hit[0] = PacketType_NoteHit;
                    
                    u16 note_i = note - stage.notes;
                    note_hit[1] = note_i >> 0;
                    note_hit[2] = note_i >> 8;
                    
                    note_hit[3] = this->score >> 0;
                    note_hit[4] = this->score >> 8;
                    note_hit[5] = this->score >> 16;
                    note_hit[6] = this->score >> 24;
                    
                    /*
                    note_hit[7] = 0xFF;
                    
                    note_hit[8] = this->combo >> 0;
                    note_hit[9] = this->combo >> 8;
                    */
                    
                    Network_Send(&note_hit);
                }
            #endif
            return;
        }
    }
    if (this->character->spec & CHAR_SPEC_MISSANIM)
            this->character->set_anim(this->character, note_anims[type & 0x3][2]);
        else
            this->character->set_anim(this->character, note_anims[type & 0x3][0]);
    
    //Missed a note
    this->arrow_hitan[type & 0x3] = -1;
    
    if (!stage.prefs.ghost)
    {
        if (this->character->spec & CHAR_SPEC_MISSANIM)
            this->character->set_anim(this->character, note_anims[type & 0x3][2]);
        else
            this->character->set_anim(this->character, note_anims[type & 0x3][0]);
        Stage_MissNote(this);
        
        this->health -= 400;
        this->score -= 1;
        this->refresh_score = true;
        
        #ifdef PSXF_NETWORK
            if (stage.mode >= StageMode_Net1)
            {
                //Send note hit packet
                Packet note_hit;
                note_hit[0] = PacketType_NoteMiss;
                note_hit[1] = type & 0x3;
                
                note_hit[2] = this->score >> 0;
                note_hit[3] = this->score >> 8;
                note_hit[4] = this->score >> 16;
                note_hit[5] = this->score >> 24;
                
                Network_Send(&note_hit);
            }
        #endif
    }
}

static void Stage_SustainCheck(PlayerState *this, u8 type)
{
    //Perform note check
    for (Note *note = stage.cur_note;; note++)
    {
        //Check if note can be hit
        fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
        if (note_fp - stage.early_sus_safe > stage.note_scroll)
            break;
        if (note_fp + stage.late_sus_safe < stage.note_scroll)
            continue;
        if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || !(note->type & NOTE_FLAG_SUSTAIN))
            continue;
        
        //Hit the note
        note->type |= NOTE_FLAG_HIT;
        
        this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
        
        Stage_StartVocal();
        this->health += 230;
        this->arrow_hitan[type & 0x3] = stage.step_time;
            
        #ifdef PSXF_NETWORK
            if (stage.mode >= StageMode_Net1)
            {
                //Send note hit packet
                Packet note_hit;
                note_hit[0] = PacketType_NoteHit;
                
                u16 note_i = note - stage.notes;
                note_hit[1] = note_i >> 0;
                note_hit[2] = note_i >> 8;
                
                note_hit[3] = this->score >> 0;
                note_hit[4] = this->score >> 8;
                note_hit[5] = this->score >> 16;
                note_hit[6] = this->score >> 24;
                
                /*
                note_hit[7] = 0xFF;
                
                note_hit[8] = this->combo >> 0;
                note_hit[9] = this->combo >> 8;
                */
                
                Network_Send(&note_hit);
            }
        #endif
    }
}

static void Stage_ProcessPlayer(PlayerState *this, Pad *pad, boolean playing)
{
	//Handle player note presses
	if (stage.prefs.botplay == 0) {
		if (playing)
		{
			u8 i = (this->character == stage.opponent) ? NOTE_FLAG_OPPONENT : 0;
			
			this->pad_held = this->character->pad_held = pad->held;
			this->pad_press = pad->press;
			
			if (this->pad_held & INPUT_LEFT)
				Stage_SustainCheck(this, 0 | i);
			if (this->pad_held & INPUT_DOWN)
				Stage_SustainCheck(this, 1 | i);
			if (this->pad_held & INPUT_UP)
				Stage_SustainCheck(this, 2 | i);
			if (this->pad_held & INPUT_RIGHT)
				Stage_SustainCheck(this, 3 | i);
			
			if (this->pad_press & INPUT_LEFT)
				Stage_NoteCheck(this, 0 | i);
			if (this->pad_press & INPUT_DOWN)
				Stage_NoteCheck(this, 1 | i);
			if (this->pad_press & INPUT_UP)
				Stage_NoteCheck(this, 2 | i);
			if (this->pad_press & INPUT_RIGHT)
				Stage_NoteCheck(this, 3 | i);
		}
		else
		{
			this->pad_held = this->character->pad_held = 0;
			this->pad_press = 0;
		}
	}
	
	if (stage.prefs.botplay == 1) {
		//Do perfect note checks
		if (playing)
		{
			u8 i = (this->character == stage.opponent) ? NOTE_FLAG_OPPONENT : 0;
			
			u8 hit[4] = {0, 0, 0, 0};
			for (Note *note = stage.cur_note;; note++)
			{
				//Check if note can be hit
				fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
				if (note_fp - stage.early_safe - FIXED_DEC(12,1) > stage.note_scroll)
					break;
				if (note_fp + stage.late_safe < stage.note_scroll)
					continue;
				if ((note->type & NOTE_FLAG_OPPONENT) != i)
					continue;
				
				//Handle note hit
				if (!(note->type & NOTE_FLAG_SUSTAIN))
				{
					if (note->type & NOTE_FLAG_HIT)
						continue;
					if (stage.note_scroll >= note_fp)
						hit[note->type & 0x3] |= 1;
					else if (!(hit[note->type & 0x3] & 8))
						hit[note->type & 0x3] |= 2;
				}
				else if (!(hit[note->type & 0x3] & 2))
				{
					if (stage.note_scroll <= note_fp)
						hit[note->type & 0x3] |= 4;
					hit[note->type & 0x3] |= 8;
				}
			}
			
			//Handle input
			this->pad_held = 0;
			this->pad_press = 0;
			
			for (u8 j = 0; j < 4; j++)
			{
				if (hit[j] & 5)
				{
					this->pad_held |= note_key[j];
					Stage_SustainCheck(this, j | i);
				}
				if (hit[j] & 1)
				{
					this->pad_press |= note_key[j];
					Stage_NoteCheck(this, j | i);
				}
			}
			
			this->character->pad_held = this->pad_held;
		}
		else
		{
			this->pad_held = this->character->pad_held = 0;
			this->pad_press = 0;
		}
	}
}


//Stage drawing functions
void Stage_DrawTexCol(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 cr, u8 cg, u8 cb)
{
	fixed_t xz = dst->x;
	fixed_t yz = dst->y;
	fixed_t wz = dst->w;
	fixed_t hz = dst->h;
	
	//Don't draw if HUD and is disabled
	if (tex == &stage.tex_hud0 || tex == &stage.tex_hud1)
	{
		if (nohud)
			return;
	}
	
	fixed_t l = (SCREEN_WIDTH2  << FIXED_SHIFT) + FIXED_MUL(xz, zoom);// + FIXED_DEC(1,2);
	fixed_t t = (SCREEN_HEIGHT2 << FIXED_SHIFT) + FIXED_MUL(yz, zoom);// + FIXED_DEC(1,2);
	fixed_t r = l + FIXED_MUL(wz, zoom);
	fixed_t b = t + FIXED_MUL(hz, zoom);
	
	l >>= FIXED_SHIFT;
	t >>= FIXED_SHIFT;
	r >>= FIXED_SHIFT;
	b >>= FIXED_SHIFT;
	
	RECT sdst = {
		l,
		t,
		r - l,
		b - t,
	};
	Gfx_DrawTexCol(tex, src, &sdst, cr, cg, cb);
}

void Stage_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom)
{
    Stage_DrawTexCol(tex, src, dst, zoom, 0x80, 0x80, 0x80);
}

void Stage_DrawTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom)
{
	//Don't draw if HUD and HUD is disabled
	if (nohud) {
		if (tex == &stage.tex_hud0 || tex == &stage.tex_hud1)
			return;
	}

	//Get screen-space points
	POINT s0 = {SCREEN_WIDTH2 + (FIXED_MUL(p0->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p0->y, zoom) >> FIXED_SHIFT)};
	POINT s1 = {SCREEN_WIDTH2 + (FIXED_MUL(p1->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p1->y, zoom) >> FIXED_SHIFT)};
	POINT s2 = {SCREEN_WIDTH2 + (FIXED_MUL(p2->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p2->y, zoom) >> FIXED_SHIFT)};
	POINT s3 = {SCREEN_WIDTH2 + (FIXED_MUL(p3->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p3->y, zoom) >> FIXED_SHIFT)};
	
	Gfx_DrawTexArb(tex, src, &s0, &s1, &s2, &s3);
}

void Stage_BlendTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom, u8 mode)
{
	//Don't draw if HUD and HUD is disabled
	if (nohud) {
		if (tex == &stage.tex_hud0 || tex == &stage.tex_hud1)
			return;
	}
	
	//Get screen-space points
	POINT s0 = {SCREEN_WIDTH2 + (FIXED_MUL(p0->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p0->y, zoom) >> FIXED_SHIFT)};
	POINT s1 = {SCREEN_WIDTH2 + (FIXED_MUL(p1->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p1->y, zoom) >> FIXED_SHIFT)};
	POINT s2 = {SCREEN_WIDTH2 + (FIXED_MUL(p2->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p2->y, zoom) >> FIXED_SHIFT)};
	POINT s3 = {SCREEN_WIDTH2 + (FIXED_MUL(p3->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p3->y, zoom) >> FIXED_SHIFT)};
	
	Gfx_BlendTexArb(tex, src, &s0, &s1, &s2, &s3, mode);
}

//Stage HUD functions
static void Stage_DrawHealth(s16 health, u8 i, s8 ox)
{
    //Check if we should use 'dying' frame
    s8 dying;
    if (ox < 0)
        dying = (health >= 18000) * 32;
    else
        dying = (health <= 2000) * 32;
    
    //Get src and dst
    fixed_t hx = (128 << FIXED_SHIFT) * (10000 - health) / 10000;
    RECT src = {
        (i % 4) * 64 + dying,
        48 + (i / 4) * 32,
        32,
        32
    };
    RECT_FIXED dst = {
        hx + ox * FIXED_DEC(15,1) - FIXED_DEC(16,1),
        FIXED_DEC(SCREEN_HEIGHT2 - 35 + 4 - 16, 1),
        src.w << FIXED_SHIFT,
        src.h << FIXED_SHIFT
    };
    if (stage.prefs.downscroll)
        dst.y = -dst.y - dst.h;
    
    //Draw health icon
    Stage_DrawTex(&stage.tex_hud1, &src, &dst, FIXED_MUL(stage.bump, stage.sbump));
}
static void Stage_DrawStrum(u8 i, RECT *note_src, RECT_FIXED *note_dst)
{
    (void)note_dst;
    
    PlayerState *this = &stage.player_state[(i & NOTE_FLAG_OPPONENT) != 0];
    i &= 0x3;
    
    if (this->arrow_hitan[i] > 0)
    {
        //Play hit animation
        u8 frame = ((this->arrow_hitan[i] << 1) / stage.step_time) & 1;
        note_src->x = (i + 1) << 5;
        note_src->y = 64 - (frame << 5);
        
        this->arrow_hitan[i] -= timer_dt;
        if (this->arrow_hitan[i] <= 0)
        {
            if (this->pad_held & note_key[i])
                this->arrow_hitan[i] = 1;
            else
                this->arrow_hitan[i] = 0;
        }
    }
    else if (this->arrow_hitan[i] < 0)
    {
        //Play depress animation
        note_src->x = (i + 1) << 5;
        note_src->y = 96;
        if (!(this->pad_held & note_key[i]))
            this->arrow_hitan[i] = 0;
    }
    else
    {
        note_src->x = 0;
        note_src->y = i << 5;
    }
}

static void Stage_DrawNotes(void)
{
    //Check if opponent should draw as bot
    u8 bot = (stage.mode == StageMode_2P) ? 0 : NOTE_FLAG_OPPONENT;
    
    //Initialize scroll state
    SectionScroll scroll;
    scroll.start = stage.time_base;
    
    Section *scroll_section = stage.section_base;
    Stage_GetSectionScroll(&scroll, scroll_section);
    
    //Push scroll back until cur_note is properly contained
    while (scroll.start_step > stage.cur_note->pos)
    {
        //Look for previous section
        Section *prev_section = Stage_GetPrevSection(scroll_section);
        if (prev_section == NULL)
            break;
        
        //Push scroll back
        scroll_section = prev_section;
        Stage_GetSectionScroll(&scroll, scroll_section);
        scroll.start -= scroll.length;
    }
    
    //Draw notes
    for (Note *note = stage.cur_note; note->pos != 0xFFFF; note++)
    {
        //Update scroll
        while (note->pos >= scroll_section->end)
        {
            //Push scroll forward
            scroll.start += scroll.length;
            Stage_GetSectionScroll(&scroll, ++scroll_section);
        }

        
        //Get note information
        u8 i = (note->type & NOTE_FLAG_OPPONENT) != 0;
        PlayerState *this = &stage.player_state[i];
        
        fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
        fixed_t time = (scroll.start - stage.song_time) + (scroll.length * (note->pos - scroll.start_step) / scroll.length_step);
        fixed_t y = note_y[(note->type & 0x7) ^ stage.note_swap] + FIXED_MUL(stage.speed, time * 150);
        
        //Check if went above screen
        if (y < FIXED_DEC(-16 - SCREEN_HEIGHT2, 1))
        {
            //Wait for note to exit late time
            if (note_fp + stage.late_safe >= stage.note_scroll)
                continue;
            
            //Miss note if player's note
            if (!(note->type & (bot | NOTE_FLAG_HIT | NOTE_FLAG_MINE)))
            {
                if (stage.mode < StageMode_Net1 || i == ((stage.mode == StageMode_Net1) ? 0 : 1))
                {
                    //Missed note
                    Stage_CutVocal();
                    Stage_MissNote(this);
                    this->health -= 475;
                    stage.misses++;
                    this->refresh_misses = true;
                    //this->character->spec & CHAR_SPEC_MISSANIM;
                    
                    //Send miss packet
                    #ifdef PSXF_NETWORK
                        if (stage.mode >= StageMode_Net1)
                        {
                            //Send note hit packet
                            Packet note_hit;
                            note_hit[0] = PacketType_NoteMiss;
                            note_hit[1] = 0xFF;
                            
                            note_hit[2] = this->score >> 0;
                            note_hit[3] = this->score >> 8;
                            note_hit[4] = this->score >> 16;
                            note_hit[5] = this->score >> 24;
                            
                            Network_Send(&note_hit);
                        }
                    #endif
                }
            }
            
            //Update current note
            stage.cur_note++;
        }
        else
        {
            //Don't draw if below screen
            RECT note_src;
            RECT_FIXED note_dst;
            if (y > (FIXED_DEC(SCREEN_HEIGHT,2) + scroll.size) || note->pos == 0xFFFF)
                break;
            
            //Draw note
            if (note->type & NOTE_FLAG_SUSTAIN)
            {
                //Check for sustain clipping
                fixed_t clip;
                y -= scroll.size;
                if ((note->type & (bot | NOTE_FLAG_HIT)) || ((this->pad_held & note_key[note->type & 0x3]) && (note_fp + stage.late_sus_safe >= stage.note_scroll)))
                {
                    clip = FIXED_DEC(32 - SCREEN_HEIGHT2, 1) - y;
                    if (clip < 0)
                        clip = 0;
                }
                else
                {
                    clip = 0;
                }
                
                //Draw sustain
                if (note->type & NOTE_FLAG_SUSTAIN_END)
                {
                    if (clip < (24 << FIXED_SHIFT))
                    {
                        note_src.x = 160;
                        note_src.y = ((note->type & 0x3) << 5) + 4 + (clip >> FIXED_SHIFT);
                        note_src.w = 32;
                        note_src.h = 28 - (clip >> FIXED_SHIFT);
                        
                        if (normo == true && note->type & NOTE_FLAG_FLIPX)
                            note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                        else if (note->type & NOTE_FLAG_FLIPX)
                            note_dst.x = note_flip[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                        else if (normo == true)
                            note_dst.x = note_norm[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                        else
                            note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                        note_dst.y = y + clip;
                        note_dst.w = note_src.w << FIXED_SHIFT;
                        note_dst.h = (note_src.h << FIXED_SHIFT);
                    
                        
                        if (stage.prefs.downscroll)
                        {
                            note_dst.y = -note_dst.y;
                            note_dst.h = -note_dst.h;
                        }
                        Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
                    }
                }
                else
                {
                    //Get note height
                    fixed_t next_time = (scroll.start - stage.song_time) + (scroll.length * (note->pos + 12 - scroll.start_step) / scroll.length_step);
                    fixed_t next_y = note_y[(note->type & 0x7) ^ stage.note_swap] + FIXED_MUL(stage.speed, next_time * 150) - scroll.size;
                    fixed_t next_size = next_y - y;
                    
                    if (clip < next_size)
                    {
                        note_src.x = 160;
                        note_src.y = ((note->type & 0x3) << 5);
                        note_src.w = 32;
                        note_src.h = 16;
                        
                        if (normo == true && note->type & NOTE_FLAG_FLIPX)
                            note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                        else if (note->type & NOTE_FLAG_FLIPX)
                            note_dst.x = note_flip[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                        else if (normo == true)
                            note_dst.x = note_norm[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                        else
                            note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                        note_dst.y = y + clip;
                        note_dst.w = note_src.w << FIXED_SHIFT;
                        note_dst.h = (next_y - y) - clip;
                        
                        if (stage.prefs.downscroll)
                            note_dst.y = -note_dst.y - note_dst.h;
                        Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
                    }
                }
            }
            else if (note->type & NOTE_FLAG_FLIPX)
            {
                //Don't draw if already hit
                if (note->type & NOTE_FLAG_HIT)
                    continue;
                
                //Draw note
                note_src.x = 32 + ((note->type & 0x3) << 5);
                note_src.y = 0;
                note_src.w = 32;
                note_src.h = 32;
            
                note_dst.x = note_flip[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                note_dst.y = y - FIXED_DEC(16,1);
                note_dst.w = note_src.w << FIXED_SHIFT;
                note_dst.h = note_src.h << FIXED_SHIFT;
            
                if (stage.prefs.downscroll)
                    note_dst.y = -note_dst.y - note_dst.h;
                Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
            }
            else if (note->type & NOTE_FLAG_NOFLIP)
            {
                //Don't draw if already hit
                if (note->type & NOTE_FLAG_HIT)
                    continue;
                
                //Draw note
                note_src.x = 32 + ((note->type & 0x3) << 5);
                note_src.y = 0;
                note_src.w = 32;
                note_src.h = 32;
            
                note_dst.x = note_norm[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                note_dst.y = y - FIXED_DEC(16,1);
                note_dst.w = note_src.w << FIXED_SHIFT;
                note_dst.h = note_src.h << FIXED_SHIFT;
            
                if (stage.prefs.downscroll)
                    note_dst.y = -note_dst.y - note_dst.h;
                Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
            }
            else if (note->type & NOTE_FLAG_MINE)
            {
                //Don't draw if already hit
                if (note->type & NOTE_FLAG_HIT)
                    continue;
                
                //Draw note body
                note_src.x = 192 + ((note->type & 0x1) << 5);
                note_src.y = (note->type & 0x2) << 4;
                note_src.w = 32;
                note_src.h = 32;
                
                note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                note_dst.y = y - FIXED_DEC(16,1);
                note_dst.w = note_src.w << FIXED_SHIFT;
                note_dst.h = note_src.h << FIXED_SHIFT;
                
                if (stage.prefs.downscroll)
                    note_dst.y = -note_dst.y - note_dst.h;
                Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
                
                if (stage.stage_id == StageId_1_4)
                {
                    //Draw note halo
                    note_src.x = 160;
                    note_src.y = 128 + ((animf_count & 0x3) << 3);
                    note_src.w = 32;
                    note_src.h = 8;
                    
                    note_dst.y -= FIXED_DEC(6,1);
                    note_dst.h >>= 2;
                    
                    Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
                }
                else
                {
                    //Draw note fire
                    note_src.x = 192 + ((animf_count & 0x1) << 5);
                    note_src.y = 64 + ((animf_count & 0x2) * 24);
                    note_src.w = 32;
                    note_src.h = 48;
                    
                    if (stage.prefs.downscroll)
                    {
                        note_dst.y += note_dst.h;
                        note_dst.h = note_dst.h * -3 / 2;
                    }
                    else
                    {
                        note_dst.h = note_dst.h * 3 / 2;
                    }
                    Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
                }
            }
            else
            {
                //Don't draw if already hit
                if (note->type & NOTE_FLAG_HIT)
                    continue;
                
                //Draw note
                note_src.x = 32 + ((note->type & 0x3) << 5);
                note_src.y = 0;
                note_src.w = 32;
                note_src.h = 32;
            
                if (stage.stage_id == StageId_1_2 || normo == 1)
                {
                    
                    note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                    //note_dst.x = note_norm[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                }
                else
                        note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
                note_dst.y = y - FIXED_DEC(16,1);
                note_dst.w = note_src.w << FIXED_SHIFT;
                note_dst.h = note_src.h << FIXED_SHIFT;
            
                if (stage.prefs.downscroll)
                    note_dst.y = -note_dst.y - note_dst.h;
                Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
            }
        }
    }
}
void Stage_DrawBox()
{
    RECT dia_src = {0, 0, 227, 63};
    RECT_FIXED dia_dst = {FIXED_DEC(0,1), FIXED_DEC(0,1), FIXED_DEC(120,1), FIXED_DEC(63,1)};

    Stage_DrawTex(&stage.tex_dia, &dia_src, &dia_dst, stage.bump);
}
static void Stage_DrawDiaPorts(u8 i, s16 x, s16 y)
{
    //Port Size
    u8 port_size = 64;

    //Get src and dst
    RECT Port_src = {
        (i % 4) * port_size,
        (i / 4) * port_size,
        port_size,
        port_size
    };
    RECT Port_dst = {
        x,
        y,
        64,
        64
    };
    //Port_dst.w = Port_dst.w * 2;
    //Port_dst.h = Port_dst.h * 2;
    //Draw dia port
    Gfx_DrawTex(&stage.tex_hud2, &Port_src, &Port_dst);
}
//Stage loads
static void Stage_SwapChars(void)
{
    if (stage.mode == StageMode_Swap)
    {
        Character *temp = stage.player;
        stage.player = stage.opponent;
        stage.opponent = temp;
    }
}

static void Stage_LoadPlayer(void)
{
    //Load player character
    Character_Free(stage.player);
    stage.player = stage.stage_def->pchar.new(stage.stage_def->pchar.x, stage.stage_def->pchar.y);
}

static void Stage_LoadOpponent(void)
{
    //Load opponent character
    Character_Free(stage.opponent);
    stage.opponent = stage.stage_def->ochar.new(stage.stage_def->ochar.x, stage.stage_def->ochar.y);
}

static void Stage_LoadGirlfriend(void)
{
    //Load girlfriend character
    Character_Free(stage.gf);
    if (stage.stage_def->gchar.new != NULL)
        stage.gf = stage.stage_def->gchar.new(stage.stage_def->gchar.x, stage.stage_def->gchar.y);
    else
        stage.gf = NULL;
}

static void Stage_LoadStage(void)
{
    //Load back
    if (stage.back != NULL)
        stage.back->free(stage.back);
    stage.back = stage.stage_def->back();
}

static void Stage_LoadChart(void)
{
    //Load stage data
    char chart_path[64];
    
    //Use standard path convention
    sprintf(chart_path, "\\WEEK%d\\%d.%d%c.CHT;1", stage.stage_def->week, stage.stage_def->week, stage.stage_def->week_song, "ENH"[stage.stage_diff]);
    
    
    if (stage.chart_data != NULL)
        Mem_Free(stage.chart_data);
    stage.chart_data = IO_Read(chart_path);
    u8 *chart_byte = (u8*)stage.chart_data;
    
    #ifdef PSXF_PC
        //Get lengths
        u16 note_off = chart_byte[0] | (chart_byte[1] << 8);
        
        u8 *section_p = chart_byte + 2;
        u8 *note_p = chart_byte + note_off;
        
        u8 *section_pp;
        u8 *note_pp;
        
        size_t sections = (note_off - 2) >> 2;
        size_t notes = 0;
        
        for (note_pp = note_p;; note_pp += 4)
        {
            notes++;
            u16 pos = note_pp[0] | (note_pp[1] << 8);
            if (pos == 0xFFFF)
                break;
        }
        
        if (notes)
            stage.num_notes = notes - 1;
        else
            stage.num_notes = 0;
        
        //Realloc for separate structs
        size_t sections_size = sections * sizeof(Section);
        size_t notes_size = notes * sizeof(Note);
        size_t notes_off = MEM_ALIGN(sections_size);
        
        u8 *nchart = Mem_Alloc(notes_off + notes_size);
        
        Section *nsection_p = stage.sections = (Section*)nchart;
        section_pp = section_p;
        for (size_t i = 0; i < sections; i++, section_pp += 4, nsection_p++)
        {
            nsection_p->end = section_pp[0] | (section_pp[1] << 8);
            nsection_p->flag = section_pp[2] | (section_pp[3] << 8);
        }
        
        Note *nnote_p = stage.notes = (Note*)(nchart + notes_off);
        note_pp = note_p;
        for (size_t i = 0; i < notes; i++, note_pp += 4, nnote_p++)
        {
            nnote_p->pos = note_pp[0] | (note_pp[1] << 8);
            nnote_p->type = note_pp[2] | (note_pp[3] << 8);
        }
        
        //Use reformatted chart
        Mem_Free(stage.chart_data);
        stage.chart_data = (IO_Data)nchart;
    #else
        //Directly use section and notes pointers
        stage.sections = (Section*)(chart_byte + 2);
        stage.notes = (Note*)(chart_byte + *((u16*)stage.chart_data));
        
        for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
            stage.num_notes++;
    #endif
    
    //Swap chart
    if (stage.mode == StageMode_Swap)
    {
        for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
            note->type ^= NOTE_FLAG_OPPONENT;
        for (Section *section = stage.sections;; section++)
        {
            section->flag ^= SECTION_FLAG_OPPFOCUS;
            if (section->end == 0xFFFF)
                break;
        }
    }
    
    //Count max scores
    stage.player_state[0].max_score = 0;
    stage.player_state[1].max_score = 0;
    for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
    {
        if (note->type & (NOTE_FLAG_SUSTAIN | NOTE_FLAG_MINE))
            continue;
        if (note->type & NOTE_FLAG_OPPONENT)
            stage.player_state[1].max_score += 35;
        else
            stage.player_state[0].max_score += 35;
    }
    if (stage.mode >= StageMode_2P && stage.player_state[1].max_score > stage.player_state[0].max_score)
        stage.max_score = stage.player_state[1].max_score;
    else
        stage.max_score = stage.player_state[0].max_score;
    
    stage.cur_section = stage.sections;
    stage.cur_note = stage.notes;
    
    stage.speed = stage.stage_def->speed[stage.stage_diff];
    
    stage.step_crochet = 0;
    stage.time_base = 0;
    stage.step_base = 0;
    stage.section_base = stage.cur_section;
    Stage_ChangeBPM(stage.cur_section->flag & SECTION_FLAG_BPM_MASK, 0);
}

static void Stage_LoadMusic(void)
{
    //Offset sing ends
    stage.player->sing_end -= stage.note_scroll;
    stage.opponent->sing_end -= stage.note_scroll;
    if (stage.gf != NULL)
        stage.gf->sing_end -= stage.note_scroll;
    
    //Find music file and begin seeking to it
    Audio_SeekXA_Track(stage.stage_def->music_track);
    
    //Initialize music state
    stage.note_scroll = FIXED_DEC(-5 * 4 * 12,1);
    stage.song_time = FIXED_DIV(stage.note_scroll, stage.step_crochet);
    stage.interp_time = 0;
    stage.interp_ms = 0;
    stage.interp_speed = 0;
    
    //Offset sing ends again
    stage.player->sing_end += stage.note_scroll;
    stage.opponent->sing_end += stage.note_scroll;
    if (stage.gf != NULL)
        stage.gf->sing_end += stage.note_scroll;
}

static void Stage_LoadState(void)
{
    //Initialize stage state
    stage.flag = STAGE_FLAG_VOCAL_ACTIVE;
    
    stage.gf_speed = 1 << 2;
    
    //check if song has dialogue
    if (stage.story)
    {
        printf("Checking for dialogue...\n");
        if (stage.stage_def->dialogue == 1)
        {
            printf("Dialogue found!\n");
            Stage_LoadDia();
            stage.state = StageState_Dialogue;
        }
        else
        {
            printf("Dialogue not found.\n");
            stage.state = StageState_Play;
        }
    }
    else
    {
        stage.state = StageState_Play;
    }
    
    stage.player_state[0].character = stage.player;
    stage.player_state[1].character = stage.opponent;
    for (int i = 0; i < 2; i++)
    {
        memset(stage.player_state[i].arrow_hitan, 0, sizeof(stage.player_state[i].arrow_hitan));
        
        stage.player_state[i].health = 10000;
        stage.player_state[i].combo = 0;
        
        stage.player_state[i].refresh_score = true;
        stage.player_state[i].refresh_misses = false;
        stage.player_state[i].score = 0;
        strcpy(stage.player_state[i].score_text, "0");
        strcpy(stage.player_state[i].misses_text, "0");
        
        stage.delect = 0;
        
        stage.player_state[i].pad_held = stage.player_state[i].pad_press = 0;
    }
    
    stage.misses = 0;
    ObjectList_Free(&stage.objlist_splash);
    ObjectList_Free(&stage.objlist_fg);
    ObjectList_Free(&stage.objlist_bg);
    
    stage.prefs.ghost = true;
    stage.notemode = 0;
    shakey = 0;
    noheadbump = 0;
    stage.fadeinwhite = 0;
    
    normo = false;
    paused = false;
    
        if(stage.stage_diff == StageDiff_Easy){pausediff = "EASY";}
	if(stage.stage_diff == StageDiff_Normal){pausediff = "NORMAL";}
	if(stage.stage_diff == StageDiff_Hard){pausediff = "HARD";}
	
	if (stage.stage_id == StageId_1_1){pausestage = "PSYCHIC";}
	if (stage.stage_id == StageId_1_2){pausestage = "WILTER";}
	if (stage.stage_id == StageId_1_3){pausestage = "UPROAR";}
	if (stage.stage_id == StageId_2_1){pausestage = "PSYCHIC";}
	if (stage.stage_id == StageId_2_2){pausestage = "LATE DRIVE";}
	if (stage.stage_id == StageId_2_3){pausestage = "FLOPCHIC";}
	
    loadonce =false;
    //FontData_Load(&stage.font_cdr, Font_CDR);
}
int note1x = 26; //players
int note2x = 60;
int note3x = 94;
int note4x = 128;

int note5x = -128; //opponents
int note6x = -94;
int note7x = -60;
int note8x = -26;

int note1y = 32;
int note2y = 32;
int note3y = 32;
int note4y = 32;

int note5y = 32;
int note6y = 32;
int note7y = 32;
int note8y = 32;

//Welcome to the shitshow 2: Electric Boogaloo
void Stage_Note_Move(void)
{

    if (shakey == 0)
    {
        note_x[0] = FIXED_DEC(note1x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[1] = FIXED_DEC(note2x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[2] = FIXED_DEC(note3x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[3] = FIXED_DEC(note4x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[4] = FIXED_DEC(note5x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[5] = FIXED_DEC(note6x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[6] = FIXED_DEC(note7x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[7] = FIXED_DEC(note8x,1) + FIXED_DEC(SCREEN_WIDEADD,4);

        note_y[0] = FIXED_DEC(note1y - SCREEN_HEIGHT2, 1);
        note_y[1] = FIXED_DEC(note2y - SCREEN_HEIGHT2, 1);
        note_y[2] = FIXED_DEC(note3y - SCREEN_HEIGHT2, 1);
        note_y[3] = FIXED_DEC(note4y - SCREEN_HEIGHT2, 1);
        note_y[4] = FIXED_DEC(note5y - SCREEN_HEIGHT2, 1);
        note_y[5] = FIXED_DEC(note6y - SCREEN_HEIGHT2, 1);
        note_y[6] = FIXED_DEC(note7y - SCREEN_HEIGHT2, 1);
        note_y[7] = FIXED_DEC(note8y - SCREEN_HEIGHT2, 1);
    }
    else
    {
        note_x[0] = FIXED_DEC(note1x,1) + RandomRange(FIXED_DEC(-15,10), FIXED_DEC(15,10)) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[1] = FIXED_DEC(note2x,1) + RandomRange(FIXED_DEC(-15,10), FIXED_DEC(15,10)) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[2] = FIXED_DEC(note3x,1) + RandomRange(FIXED_DEC(-15,10), FIXED_DEC(15,10)) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[3] = FIXED_DEC(note4x,1) + RandomRange(FIXED_DEC(-15,10), FIXED_DEC(15,10)) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[4] = FIXED_DEC(note5x,1) + RandomRange(FIXED_DEC(-15,10), FIXED_DEC(15,10)) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[5] = FIXED_DEC(note6x,1) + RandomRange(FIXED_DEC(-15,10), FIXED_DEC(15,10)) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[6] = FIXED_DEC(note7x,1) + RandomRange(FIXED_DEC(-15,10), FIXED_DEC(15,10)) + FIXED_DEC(SCREEN_WIDEADD,4);
        note_x[7] = FIXED_DEC(note8x,1) + RandomRange(FIXED_DEC(-15,10), FIXED_DEC(15,10)) + FIXED_DEC(SCREEN_WIDEADD,4);

        note_y[0] = FIXED_DEC(note1y - SCREEN_HEIGHT2, 1) + RandomRange(FIXED_DEC(-1,1), FIXED_DEC(1,1));
        note_y[1] = FIXED_DEC(note2y - SCREEN_HEIGHT2, 1) + RandomRange(FIXED_DEC(-1,1), FIXED_DEC(1,1));
        note_y[2] = FIXED_DEC(note3y - SCREEN_HEIGHT2, 1) + RandomRange(FIXED_DEC(-1,1), FIXED_DEC(1,1));
        note_y[3] = FIXED_DEC(note4y - SCREEN_HEIGHT2, 1) + RandomRange(FIXED_DEC(-1,1), FIXED_DEC(1,1));
        note_y[4] = FIXED_DEC(note5y - SCREEN_HEIGHT2, 1) + RandomRange(FIXED_DEC(-1,1), FIXED_DEC(1,1));
        note_y[5] = FIXED_DEC(note6y - SCREEN_HEIGHT2, 1) + RandomRange(FIXED_DEC(-1,1), FIXED_DEC(1,1));
        note_y[6] = FIXED_DEC(note7y - SCREEN_HEIGHT2, 1) + RandomRange(FIXED_DEC(-1,1), FIXED_DEC(1,1));
        note_y[7] = FIXED_DEC(note8y - SCREEN_HEIGHT2, 1) + RandomRange(FIXED_DEC(-1,1), FIXED_DEC(1,1));
    }
    

    switch (stage.notemode)
    {
        case 0: //normal, just for when stage starts
        {
            note1x = 26;
            note2x = 60;
            note3x = 94;
            note4x = 128;

            note5x = -128;
            note6x = -94;
            note7x = -60;
            note8x = -26;

            note1y = 32;
            note2y = 32;
            note3y = 32;
            note4y = 32;

            note5y = 32;
            note6y = 32;
            note7y = 32;
            note8y = 32;
            break;
        }
        case 1: //left and right flip
        {
            if (note1x < 128)
                note1x += 8;
            if (note4x > 26)
                note4x -= 8;
            //READJUST CUZ' IT LANDS LIKE 2 PIXELS OFF
            if (note1x == 130)
                note1x = 128;
            if (note4x == 24)
                note4x = 26;
            break;
        }
        case 2: //return to normal
        {
            if (note1x > 26)
                note1x -= 8;
            if (note1x == 24)
                note1x = 26;
            
            if (note2x > 60)
                note2x -= 4;
            if (note2x == 58)
                note2x = 60;

            if (note3x < 94)
                note3x += 4;
            if (note3x == 96)
                note3x = 94;

            if (note4x < 128)
                note4x += 8;
            if (note4x == 130)
                note4x = 128;
            break;
        }
        case 3: //full flip
        {
            if (note1x < 128)
                note1x += 8;

            if (note2x < 94)
                note2x += 4;

            if (note3x > 60)
                note3x -= 4;

            if (note4x > 26)
                note4x -= 8;

            //READJUST CUZ' IT LANDS LIKE 2 PIXELS OFF
            if (note1x == 130)
                note1x = 128;
            if (note2x == 96)
                note2x = 94;
            if (note3x == 58)
                note3x = 60;
            if (note4x == 24)
                note4x = 26;
            break;
        }
        case 4: //swap player 1 and 2's notes
        {
            if (note1x > -128)
                note1x -= 8;
            
            if (note2x > -94)
                note2x -= 8;
            
            if (note3x > -60)
                note3x -= 8;

            if (note4x > -26)
                note4x -= 8;

            if (note5x < 26)
                note5x += 8;

            if (note6x < 60)
                note6x += 8;

            if (note7x < 94)
                note7x += 8;

            if (note8x < 128)
                note8x += 8;
            //READJUUUUUUUUUST
            if (note1x == -134)
                note1x = -128;

            if (note2x == -100)
                note2x = -94;

            if (note3x == -66)
                note3x = -60;

            if (note4x == -32)
                note4x = -26;

            if (note5x == 32)
                note5x = 26;

            if (note6x == 66)
                note6x = 60;

            if (note7x == 100)
                note7x = 94;
                
            if (note8x == 134)
                note8x = 128;
            break;
        }
        case 5:
        {
            if (note1x < 26)
                note1x += 8;
            
            if (note2x < 60)
                note2x += 8;
            
            if (note3x < 94)
                note3x += 8;

            if (note4x < 128)
                note4x += 8;

            if (note5x > -128)
                note5x -= 8;

            if (note6x > -94)
                note6x -= 8;

            if (note7x > -60)
                note7x -= 8;

            if (note8x > -26)
                note8x -= 8;
            //it's such a baaaad sign
            if (note5x == -134)
                note5x = -128;

            if (note6x == -100)
                note6x = -94;

            if (note7x == -66)
                note7x = -60;

            if (note8x == -32)
                note8x = -26;

            if (note1x == 32)
                note1x = 26;

            if (note2x == 66)
                note2x = 60;

            if (note3x == 100)
                note3x = 94;
                
            if (note4x == 134)
                note4x = 128;
           break;
        }
        case 6: //swap player 1 and 2's notes and flip player1s notes
        {
            if (note1x > -26)//-128
                note1x -= 7;
            
            if (note2x > -60)//-94
                note2x -= 7;
            
            if (note3x > -94)//-60
                note3x -= 7;

            if (note4x > -128)//-26
                note4x -= 7;

            if (note5x < 26)
                note5x += 8;

            if (note6x < 60)
                note6x += 8;

            if (note7x < 94)
                note7x += 8;

            if (note8x < 128)
                note8x += 8;
            //READJUUUUUUUUUST
            if (note1x == -32)//-128
                note1x = -26;

            if (note2x == -66)//-94
                note2x = -60;

            if (note3x == -100)//-60
                note3x = -94;

            if (note4x == -130)//-26
                note4x = -128;

            if (note5x == 32)
                note5x = 26;

            if (note6x == 66)
                note6x = 60;

            if (note7x == 100)
                note7x = 94;
                
            if (note8x == 134)
                note8x = 128;
            break;
        }
        case 7:
        {
            if (note1x < 26)
                note1x += 8;
            
            if (note2x < 60)
                note2x += 7;
            
            if (note3x < 94)
                note3x += 7;

            if (note4x < 128)
                note4x += 7;

            if (note5x > -128)
                note5x -= 8;

            if (note6x > -94)
                note6x -= 8;

            if (note7x > -60)
                note7x -= 8;


            if (note8x > -26)
                note8x -= 8;
            //it's such a baaaad sign
            if (note5x == -130)
                note5x = -128;

            if (note6x == -100)
                note6x = -94;

            if (note7x == -66)
                note7x = -60;

            if (note8x == -32)
                note8x = -26;

            if (note1x == 32)
                note1x = 26;

            if (note2x == 66)
                note2x = 60;

            if (note3x == 100)
                note3x = 94;
                
            if (note4x == 134)
                note4x = 128;
            break;
        }
        case 8: //Shitfest
        {   
            if (note2x > -94)
                note2x -= 8;

            if (note4x > -26)
                note4x -= 8;

            if (note6x < 60)
                note6x += 8;

            if (note8x < 128)
                note8x += 8;
            //READJUUUUUUUUUST

            if (note2x == -100)
                note2x = -94;

            if (note4x == -32)
                note4x = -26;

            if (note6x == 66)
                note6x = 60;
                
            if (note8x == 134)
                note8x = 128;
            
            note1y = 64;
            note2y = 32;
            note3y = 64;
            note4y = 32;

            note5y = 64;
            note6y = 32;
            note7y = 64;
            note8y = 32;
            break;
        }
        case 9:
        {   
            if (note2x < 60)
                note2x += 8;

            if (note4x < 128)
                note4x += 8;

            if (note6x > -94)
                note6x -= 8;

            if (note8x > -26)
                note8x -= 8;
            //it's such a baaaad sign

            if (note6x == -100)
                note6x = -94;

            if (note8x == -32)

                note8x = -26;

            if (note2x == 66)
                note2x = 60;
                
            if (note4x == 134)
                note4x = 128;
                
            note1y = 32;
            note2y = 32;
            note3y = 32;
            note4y = 32;

            note5y = 32;
            note6y = 32;
            note7y = 32;
            note8y = 32;
           break;
        }
    }
}
static void Menu_DrawBot(s32 x, s32 y)
{
    //Draw Track
    RECT bot_src = {189, 240, 67, 16};
    Gfx_BlitTex(&stage.tex_hud0, &bot_src, x, y);
}
//Stage functions
void Stage_Load(StageId id, StageDiff difficulty, boolean story)
{
    //Get stage definition
    stage.stage_def = &stage_defs[stage.stage_id = id];
    stage.stage_diff = difficulty;
    stage.story = story;
    stage.song_completed = false;

    // Dont play str if song has been reloaded
    if (stage.trans != StageTrans_Reload)
    {
        Str_CanPlayBegin();
    }
    
    //Load HUD textures
    Gfx_LoadTex(&stage.tex_hud0, IO_Read("\\STAGE\\HUD0.TIM;1"), GFX_LOADTEX_FREE);
    Gfx_LoadTex(&stage.tex_hud1, IO_Read("\\STAGE\\HUD1.TIM;1"), GFX_LOADTEX_FREE);
    
    //Load stage background
    Stage_LoadStage();
    
    //Load characters
    Stage_LoadPlayer();
    Stage_LoadOpponent();
    Stage_LoadGirlfriend();
    Stage_SwapChars();
    
    //Load stage chart
    Stage_LoadChart();
    
    //Initialize stage state
    stage.story = story;
    
    Stage_LoadState();
    
    //Initialize camera
    if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
        Stage_FocusCharacter(stage.opponent, FIXED_UNIT);
    else
        Stage_FocusCharacter(stage.player, FIXED_UNIT);
    stage.camera.x = stage.camera.tx;
    stage.camera.y = stage.camera.ty;
    stage.camera.zoom = stage.camera.tz;
    
    stage.bump = FIXED_UNIT;
    stage.sbump = FIXED_UNIT;
    
    //Initialize stage according to mode
    stage.note_swap = (stage.mode == StageMode_Swap) ? 4 : 0;
    
    //Load music
    stage.note_scroll = 0;
    Stage_LoadMusic();
    
    //Test offset
    stage.offset = 0;
    
    #ifdef PSXF_NETWORK
    if (stage.mode >= StageMode_Net1 && Network_IsHost())
    {
        //Send ready packet to peer
        Packet ready;
        ready[0] = PacketType_Ready;
        ready[1] = id;
        ready[2] = difficulty;
        ready[3] = (stage.mode == StageMode_Net1) ? 1 : 0;
        Network_Send(&ready);
    }
    #endif
}

void Stage_Unload(void)
{
    //Disable net mode to not break the game
    if (stage.mode >= StageMode_Net1)
        stage.mode = StageMode_Normal;
    
    //Unload stage background
    if (stage.back != NULL)
        stage.back->free(stage.back);
    stage.back = NULL;
    
    //Unload stage data
    Mem_Free(stage.chart_data);
    stage.chart_data = NULL;
    
    //Free objects
    ObjectList_Free(&stage.objlist_splash);
    ObjectList_Free(&stage.objlist_fg);
    ObjectList_Free(&stage.objlist_bg);
    
    //Free characters
    Character_Free(stage.player);
    stage.player = NULL;
    Character_Free(stage.opponent);
    stage.opponent = NULL;
    Character_Free(stage.gf);
    stage.gf = NULL;
}

static boolean Stage_NextLoad(void)
{
    u8 load = stage.stage_def->next_load;
    if (load == 0)
    {
        //Do stage transition if full reload
        stage.trans = StageTrans_NextSong;
        Trans_Start();
        return false;
    }
    else
    {
        //Get stage definition
        stage.stage_def = &stage_defs[stage.stage_id = stage.stage_def->next_stage];

        Str_CanPlayBegin();

        stage.song_completed = false;
        
        //Load stage background
        if (load & STAGE_LOAD_STAGE)
            Stage_LoadStage();
        
        //Load characters
        Stage_SwapChars();
        if (load & STAGE_LOAD_PLAYER)
        {
            Stage_LoadPlayer();
        }
        else
        {
            stage.player->x = stage.stage_def->pchar.x;
            stage.player->y = stage.stage_def->pchar.y;
        }
        if (load & STAGE_LOAD_OPPONENT)
        {
            Stage_LoadOpponent();
        }
        else
        {
            stage.opponent->x = stage.stage_def->ochar.x;
            stage.opponent->y = stage.stage_def->ochar.y;
        }
        Stage_SwapChars();
        if (load & STAGE_LOAD_GIRLFRIEND)
        {
            Stage_LoadGirlfriend();
        }
        else if (stage.gf != NULL)
        {
            stage.gf->x = stage.stage_def->gchar.x;
            stage.gf->y = stage.stage_def->gchar.y;
        }
        
        //Load stage chart
        Stage_LoadChart();
        
        //Initialize stage state
        Stage_LoadState();
        
        //Load music
        Stage_LoadMusic();
        
        //Reset timer
        Timer_Reset();
        return true;
    }
}
//load dialogue related files
void Stage_LoadDia(void)
{
    Gfx_LoadTex(&stage.tex_dia, IO_Read("\\STAGE\\DIA.TIM;1"), GFX_LOADTEX_FREE);
    Gfx_LoadTex(&stage.tex_hud2, IO_Read("\\STAGE\\HUD2.TIM;1"), GFX_LOADTEX_FREE);
    
    FontData_Load(&stage.font_arial, Font_Arial);

}
void Stage_Tick(void)
{
    SeamLoad:;
    
    //Tick transition
    #ifdef PSXF_NETWORK
    if (stage.mode >= StageMode_Net1)
    {
        //Show disconnect screen when disconnected
        if (!(Network_Connected() && Network_HasPeer()))
        {
            stage.trans = StageTrans_Disconnect;
            Trans_Start();
        }
    }
    else
    #endif
    {
        //Return to menu when start is pressed
        /*if (stage.state != StageState_Dialogue){ 
        if (pad_state.press & PAD_START)
        {
            stage.trans = (stage.state == StageState_Play) ? StageTrans_Menu : StageTrans_Reload;
            Trans_Start();
        }
        }*/
    }
    
    if (Trans_Tick())
    {
        switch (stage.trans)
        {
            case StageTrans_Menu:
                //Load appropriate menu
                Stage_Unload();

                if (stage.song_completed)
                {
                    Str_CanPlayFinal();
                }
                
                LoadScr_Start();
                #ifdef PSXF_NETWORK
                if (Network_Connected())
                {
                    if (Network_IsHost())
                        Menu_Load(MenuPage_NetOp);
                    else
                        Menu_Load(MenuPage_NetLobby);
                }
                else
                #endif
                {
                    if (stage.stage_id <= StageId_LastVanilla)
                    {
                        if (stage.story)
                            Menu_Load(MenuPage_Story);
                        else
                            Menu_Load(MenuPage_Freeplay);
                    }
                    else
                    {
                        Menu_Load(MenuPage_Main);
                    }
                }
                LoadScr_End();
                
                gameloop = GameLoop_Menu;
                return;
            case StageTrans_NextSong:
            	if (stage.stage_id == StageId_1_3 && stage.story)
                	{
                	stage.prefs.bweek_awards =true;
                	}
                //Load next song
                Stage_Unload();
                
                LoadScr_Start();
                Stage_Load(stage.stage_def->next_stage, stage.stage_diff, stage.story);
                LoadScr_End();
                break;
            case StageTrans_Reload:
                //Reload song
                Stage_Unload();
                
                LoadScr_Start();
                Stage_Load(stage.stage_id, stage.stage_diff, stage.story);
                LoadScr_End();
                break;
            case StageTrans_Disconnect:
        #ifdef PSXF_NETWORK
                //Disconnect screen
                Stage_Unload();
                
                LoadScr_Start();
                if (Network_Connected() && Network_IsHost())
                    Menu_Load(MenuPage_NetOpWait);
                else
                    Menu_Load(MenuPage_NetFail);
                LoadScr_End();
                
                gameloop = GameLoop_Menu;
        #endif
                return;
        }
    }
    
    switch (stage.state)
    {
        case StageState_Play:
        {
        	if(loadonce ==false)
        	{
        	FontData_Load(&stage.font_bold, Font_Bold);
        	FontData_Load(&stage.font_cdr, Font_CDR);
        	loadonce =true;
        	}
        	if(paused == false)
        	{
		        if (pad_state.press & PAD_START && stage.stage_id != StageId_1_5 && stage.stage_id != StageId_1_6)
			{
			    stage.pause_select = 0;
			    paused =true;
			}
		}
	            if (paused ==true)
			{
			    switch(pausediff2)
			    {
			    	case 0:
			    	{
			    		stage.stage_diff = StageDiff_Easy;
			    		anotherfuckingvarible = "EASY";
			    		break;
			    	}
			    	case 1:
			    	{
			    		stage.stage_diff = StageDiff_Normal;
			    		anotherfuckingvarible = "NORMAL";
			    		break;
			    	}
			    	case 2:
			    	{
			    		stage.stage_diff = StageDiff_Hard;
			    		anotherfuckingvarible = "HARD";
			    		break;
			    	}
			    }
			    	
			    Audio_PauseXA();
			    static const char *stage_options[] = {
				"RESUME",
				"RESTART SONG",
				"CHANGE DIFFICULTY",
				"EXIT TO MENU"
			      };
			 //Draw stage
			    stage.font_bold.draw(&stage.font_bold,
				pausestage,
				308,
				18,
				FontAlign_Right
			    );
			 //Draw diff
			    stage.font_bold.draw(&stage.font_bold,
				pausediff,
				308,
				39,
				FontAlign_Right
			    );
			 //Change option
		        if (pad_state.press & PAD_UP)
		        {
		            if (stage.pause_select > 0)
		                stage.pause_select--;
		            else
		                stage.pause_select = 3;
		        }
		        if (pad_state.press & PAD_DOWN)
		        {
		            if (stage.pause_select < 3)
		                stage.pause_select++;
		            else
		                stage.pause_select = 0;
		        }
			  //Select option if cross or start is pressed
			    switch (stage.pause_select)
			    {
			      case 0: //Resume
			        if (pad_state.press & PAD_CROSS && paused ==true)
			  	{
					paused = false;
					Audio_ResumeXA();
				}
				break;
			      case 1: //Retry
			        if (pad_state.press & PAD_CROSS && paused ==true)
			  	{
					stage.trans = StageTrans_Reload;
					Trans_Start();
				}
				break;
			      case 2: //Change difficulty
			        if (pad_state.press & PAD_LEFT)
				{
				    if (pausediff2 > 0)
					pausediff2--;
				    else
					pausediff2 = 2;
				}
				if (pad_state.press & PAD_RIGHT)
				{
				    if (pausediff2 < 2)
					pausediff2++;
				    else
					pausediff2 = 0;
				}
				if (pad_state.press & PAD_CROSS && paused ==true)
			  	{
			  		stage.trans = StageTrans_Reload;
					Trans_Start();
			  	}
			  	//Draw stage
				    stage.font_bold.draw(&stage.font_bold,
					anotherfuckingvarible,
					160,
				        166,
					FontAlign_Center
				    );
				break;
			      case 3: //Quit
			        if (pad_state.press & PAD_CROSS && paused ==true)
			  	{
				stage.trans = StageTrans_Menu;
				Trans_Start();
				}
				break;
			    }
			  s32 ext_scroll = stage.pause_select * FIXED_DEC(12,1);
			  stage.pause_scroll += (ext_scroll - stage.pause_scroll) >> 2;
			  //Draw all options
				for (u8 i = 0; i < COUNT_OF(stage_options); i++)
				{
				    stage.font_bold.draw_col(&stage.font_bold,
				    stage_options[i],
				        SCREEN_WIDTH2,
				        SCREEN_HEIGHT2 + (i << 5) - 48 - (stage.pause_scroll >> FIXED_SHIFT),
				        FontAlign_Center,
				        (stage.pause_select == i) ? 128 : 64,
					(stage.pause_select == i) ? 128 : 64,
					(stage.pause_select == i) ? 128 : 64
				    );
				}
			  
			  //50% Blend
			  RECT screen_src = {0, 0 ,SCREEN_WIDTH, SCREEN_HEIGHT};

			  Gfx_BlendRect(&screen_src, 0, 0, 0, 0);
			}//end of pause menu
            //Clear per-frame flags
            stage.flag &= ~(STAGE_FLAG_JUST_STEP | STAGE_FLAG_SCORE_REFRESH);
            
            //Get song position
            boolean playing;
            fixed_t next_scroll;
            
            Stage_Note_Move();
	    if (stage.prefs.botplay)
	    {
    			Menu_DrawBot(126, 49);
	    }
            
            if (stage.stage_id == StageId_1_1)
           {
                switch (stage.song_step)
                {
                    case 190:
                        stage.notemode = 1;
                        break;
                    case 318:
                        stage.notemode = 2;
                        break;
                    case 572:
                        stage.notemode = 3;
                        break;
                    case 701:
                        stage.notemode = 2;
                        break;
                }
            }

            if (stage.stage_id == StageId_1_2)
            {
                switch (stage.song_step)
                {
                    case 368:
                        cambump = 3;
                        break;
                    case 381:
                        stage.notemode = 4;
                        cambump = 0;
                        break;
                    case 496:
                        cambump = 3;
                        break;
                    case 510:
                        stage.notemode = 5;
                        cambump = 0;
                        break;
                    case 688:
                        cambump = 3;
                        break;
                    case 702:
                        stage.notemode = 4;
                        cambump = 0;
                        break;
                    case 1008:
                        cambump = 3;
                        break;
                    case 1022:
                        stage.notemode = 5;
                        cambump = 0;
                        break;
                    
                }
            }

            if (stage.stage_id == StageId_1_3)
            {
                switch (stage.song_step)
                {
                    case 253:
                        stage.notemode = 4;
                        break;
                    case 362:
                        normo = true;
                        break;
                    case 378:
                        stage.notemode = 5;
                        break;
                    case 416:
                        normo = false;
                        break;
                    case 441:
                        stage.notemode = 1;
                        break;
                    case 509:
                        stage.notemode = 2;
                        break;
                    case 637:
                        stage.notemode = 4;
                        break;
                    case 748:
                        normo = true;
                        break;
                    case 765:
                        stage.notemode = 5;
                        break;
                    case 814:
                        normo = false;
                        break;
                    case 956:
                        stage.notemode = 3;
                        break;
                    case 1019:
                        stage.notemode = 2;
                        break;
                }
            }

            if (stage.stage_id == StageId_2_2)
            {
                switch (stage.song_step)
                {
                    case 531:
                        stage.notemode = 3;
                        break;
                    case 767:
                        stage.notemode = 2;
                        break;
                }
            }

            if (stage.stage_id == StageId_2_1)
            {
                switch (stage.song_step)
                {
                    case 190:
                        stage.notemode = 3;
                        break;
                    case 318:
                        stage.notemode = 2;
                        break;
                    case 572:
                        stage.notemode = 3;
                        break;
                    case 701:
                        stage.notemode = 2;
                        break;
                }
            }
            if (stage.stage_id == StageId_2_3)
           {
                switch (stage.song_step)
                {
                    case 0:
                    	if (stage.prefs.downscroll == 0)
                    	{
                    	stage.upscroll = 0;
                    	}
                    	else
                    	{
                    	stage.upscroll = 1;
                    	}
                    	break;
                    case 128:
                        stage.notemode = 3;
                        break;
                    case 190:
                        stage.notemode = 2;
                        break;
                    case 511:
                        stage.notemode = 6;
                        break;
                    case 575:
                        stage.notemode = 7;
                   	if (stage.prefs.downscroll == 0 && stopdownscroll == 0)
                   	{
                        stage.prefs.downscroll =1;
                        stopdownscroll = 1;
                        }
                        else if (stage.prefs.downscroll == 1 && stopdownscroll == 0)
                        {
                        stage.prefs.downscroll =0;
                        stopdownscroll = 1;
                        }
                        break;
                    case 580:
                    	stage.notemode = 8;
                    	stopdownscroll = 0;
                    	break;
                    case 700:
                    {
                    	stage.notemode = 9;
                    	break;
                    }
                    case 703:
                    	if (stage.prefs.downscroll == 1 && stopdownscroll == 0)
                   	{
                        stage.prefs.downscroll =0;
                        stopdownscroll = 1;
                        }
                        else if (stage.prefs.downscroll == 0 && stopdownscroll == 0)
                        {
                        stage.prefs.downscroll =1;
                        stopdownscroll = 1;
                        }
                    	stage.notemode = 3;
                    	break;
                    case 831:
                    	stopdownscroll = 0;
                    	stage.notemode = 2;
                    	break;
                    case 960:
                    	stage.notemode = 1;
                    	break;
                    case 1092:
                    	stage.notemode = 2;
                    	break;
                    
                }
                
            }
            
            if (stage.stage_id == StageId_1_1)
               nohud = 0;
            if (stage.stage_id == StageId_1_2)
               nohud = 0;
            if (stage.stage_id == StageId_1_3)
               nohud = 0;
            if (stage.stage_id == StageId_1_4)
               nohud = 1;
            if (stage.stage_id == StageId_1_5)
              nohud = 1;
            if (stage.stage_id == StageId_1_6)
              nohud = 1;
            if (stage.stage_id == StageId_2_1)
               nohud = 0;
            if (stage.stage_id == StageId_2_2)
               nohud = 0;
            if (stage.stage_id == StageId_2_3)
               nohud = 0;
		
		if (stage.stage_id == StageId_2_3)
            {
                switch (stage.song_step)
                {
                case 704:
                {
                shakey = 1;
                break;
                }
                
                case 960:
                {
                shakey = 0;
                break;
                }
                }
            }
            if (stage.stage_id == StageId_1_3)
            {
                switch (stage.song_step)
                {
                    case 896:
                        {
                            stage.fadewhite = FIXED_DEC(255,1);
                            stage.fadeextra = FIXED_DEC(0,1);
                            stage.fadespeed = FIXED_DEC(90,1);
                            shakey = 1;
                            break;
                        }
                    case 1150:
                        {
                            stage.fadeblack = 265200;
                            stage.fadeextra = FIXED_DEC(0,1);
                            stage.fadespeed = FIXED_DEC(80,1);
                            break;
                        }
                    case 1152:
                        {
                            shakey = 0;
                            noheadbump = 1;
                            break;
                        }
                    case 1213:
                        {
                            stage.fadeblack = 0;
                            stage.fadewhite = FIXED_DEC(255,1);
                            stage.fadespeed = FIXED_DEC(100,1);
                            shakey = 1;
                            noheadbump = 0;
                            break;
                        }
                    case 1470:
                        {
                            stage.fadeinwhite = 265200;
                            stage.fadespeed = FIXED_DEC(80,1);
                            break;
                        }
                    case 1472:
                        {
                            shakey = 0;
                            break;
                        }
                    case 1493:
                        {
                            //fadewhitemode = 1;
                            stage.opponent->x = FIXED_DEC(-97,1); //launch psychic off screen
                            stage.fadeinwhite = 0;
                            stage.fadewhite = FIXED_DEC(255,1);
                            stage.fadeextra = FIXED_DEC(0,1);
                            stage.fadespeed = FIXED_DEC(80,1);
                            break;
                        }
                    case 1544:
                        {
                            stage.fadeinwhite = 265200;
                            stage.fadespeed = FIXED_DEC(80,1);
                    	    break;
                        }
                }
            }
            
            #ifdef PSXF_NETWORK
            if (stage.mode >= StageMode_Net1 && !Network_IsReady())
            {
                if (!Network_IsHost())
                {
                    //Send ready packet
                    Packet ready;
                    ready[0] = PacketType_Ready;
                    Network_Send(&ready);
                    Network_SetReady(true);
                }
                next_scroll = stage.note_scroll;
            }
            else
            #endif
            {
                const fixed_t interp_int = FIXED_UNIT * 8 / 75;
                if (stage.note_scroll < 0)
                {
                    //Play countdown sequence
                    stage.song_time += timer_dt;
                    
                    //Update song
                    if (stage.song_time >= 0)
                    {
                        //Song has started
                        playing = true;
                        Audio_PlayXA_Track(stage.stage_def->music_track, 0x40, stage.stage_def->music_channel, 0);
                        
                        //Update song time
                        fixed_t audio_time = (fixed_t)Audio_TellXA_Milli() - stage.offset;
                        if (audio_time < 0)
                            audio_time = 0;
                        stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
                        stage.interp_time = 0;
                        stage.song_time = stage.interp_ms;
                    }
                    else
                    {
                        //Still scrolling
                        playing = false;
                    }
                    
                    //Update scroll
                    next_scroll = FIXED_MUL(stage.song_time, stage.step_crochet);
                }
                else if (Audio_PlayingXA())
                {
                    fixed_t audio_time_pof = (fixed_t)Audio_TellXA_Milli();
                    fixed_t audio_time = (audio_time_pof > 0) ? (audio_time_pof - stage.offset) : 0;
                    
                    if (stage.prefs.expsync)
                    {
                        //Get playing song position
                        if (audio_time_pof > 0)
                        {
                            stage.song_time += timer_dt;
                            stage.interp_time += timer_dt;
                        }
                        
                        if (stage.interp_time >= interp_int)
                        {
                            //Update interp state
                            while (stage.interp_time >= interp_int)
                                stage.interp_time -= interp_int;
                            stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
                        }
                        
                        //Resync
                        fixed_t next_time = stage.interp_ms + stage.interp_time;
                        if (stage.song_time >= next_time + FIXED_DEC(25,1000) || stage.song_time <= next_time - FIXED_DEC(25,1000))
                        {
                            stage.song_time = next_time;
                        }
                        else
                        {
                            if (stage.song_time < next_time - FIXED_DEC(1,1000))
                                stage.song_time += FIXED_DEC(1,1000);
                            if (stage.song_time > next_time + FIXED_DEC(1,1000))
                                stage.song_time -= FIXED_DEC(1,1000);
                        }
                    }
                    else
                    {
                        //Old sync
                        stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
                        stage.interp_time = 0;
                        stage.song_time = stage.interp_ms;
                    }
                    
                    playing = true;
                    
                    //Update scroll
                    next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
                }
                else
                {
                    if(paused ==false)
                    {
                    //Song has ended
                    playing = false;
                    stage.song_time += timer_dt;
                    
                    //Update scroll
                    next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
                     Gfx_LoadTex(&stage.tex_saving, IO_Read("\\MENU\\SAVING.TIM;1"), GFX_LOADTEX_FREE);
                    Menu_DrawSavingicon( 284, 204);
                    WriteSaveDataStructToBinaryAndSaveItOnTheFuckingMemoryCard();
                    
                    if (stage.prefs.botplay ==0)
                    	{
		            	if(stage.story)
		            		{
		            		if (stage.stage_diff == StageDiff_Hard)
		            		{
				    	if (stage.stage_id == StageId_1_1 && stage.misses == 0)
				    	{
				    		stage.prefs.mpsy_awards =true;
				    		stage.psymisses = stage.misses;
				    	}
				    	if (stage.stage_id == StageId_1_2 && stage.misses == 0)
				    	{
				    		stage.prefs.mwilt_awards =true;
				    		stage.wiltmisses = stage.misses;
				    	}
				    	if (stage.stage_id == StageId_1_3)
				    	{
		            	    stage.prefs.bweek_awards =true;
				    		if(stage.misses == 0)
				    		{
				    		stage.prefs.mup_awards =true;
				    		stage.uproarmisses = stage.misses;

				    		if (stage.psymisses == 0 && stage.wiltmisses == 0 && stage.uproarmisses == 0)
				    		{
				    			stage.prefs.nomissfw = true;
				    		}
		            			}
				    		
				    	}
				    	if (stage.stage_id == StageId_2_1 && stage.misses == 0)
				    	{
				    		stage.psyomisses = stage.misses;
				    	}
				    	if (stage.stage_id == StageId_2_2 && stage.misses == 0)
				    	{
				    		stage.latedrivemisses = stage.misses;
				    		if (stage.psyomisses == 0 && stage.latedrivemisses == 0)
				    		{
				    			stage.prefs.nomissdw = true;
				    			stage.prefs.secretunlocked2 = true;
				    		}
				    		
				    	}
		            		}
		            		}
		            		}
                    //Transition to menu or next song
                    if (stage.story && stage.stage_def->next_stage != stage.stage_id)
                    {
                        stage.song_completed = true;
                        if (Stage_NextLoad())
                            goto SeamLoad;
                    }
                    else
                    {	if (stage.prefs.botplay ==0)
                    	{
		            	if(stage.story && stage.stage_id == StageId_2_2)
		            		{ 
		            		    stage.prefs.bdweek_awards =true;
		            		}
		            	if (stage.stage_diff == StageDiff_Hard)
		            	{
				    	if (stage.stage_id == StageId_1_1 && stage.misses == 0)
				    	{
				    		stage.prefs.mpsy_awards =true;
				    	}
				    	if (stage.stage_id == StageId_1_2 && stage.misses == 0)
				    	{
				    		stage.prefs.mwilt_awards =true;
				    	}
				    	if (stage.stage_id == StageId_1_3 && stage.misses == 0)
				    	{
				    		stage.prefs.mup_awards =true;
				    	}
		            	}
                    	}

                        stage.song_completed = true;
                        stage.trans = StageTrans_Menu;
                        Trans_Start();
                    }
                    }
                }
            }
            
            RecalcScroll:;
            //Update song scroll and step
            if (next_scroll > stage.note_scroll)
            {
                if (((stage.note_scroll / 12) & FIXED_UAND) != ((next_scroll / 12) & FIXED_UAND))
                    stage.flag |= STAGE_FLAG_JUST_STEP;
                stage.note_scroll = next_scroll;
                stage.song_step = (stage.note_scroll >> FIXED_SHIFT);
                if (stage.note_scroll < 0)
                    stage.song_step -= 11;
                stage.song_step /= 12;
            }
            
            //Update section
            if (stage.note_scroll >= 0)
            {
                //Check if current section has ended
                u16 end = stage.cur_section->end;
                if ((stage.note_scroll >> FIXED_SHIFT) >= end)
                {
                    //Increment section pointer
                    stage.cur_section++;
                    
                    //Update BPM
                    u16 next_bpm = stage.cur_section->flag & SECTION_FLAG_BPM_MASK;
                    Stage_ChangeBPM(next_bpm, end);
                    stage.section_base = stage.cur_section;
                    
                    //Recalculate scroll based off new BPM
                    next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
                    goto RecalcScroll;
                }
            }
            
            //Handle bump
            if ((stage.bump = FIXED_UNIT + FIXED_MUL(stage.bump - FIXED_UNIT, FIXED_DEC(95,100))) <= FIXED_DEC(1003,1000))
                stage.bump = FIXED_UNIT;
            stage.sbump = FIXED_UNIT + FIXED_MUL(stage.sbump - FIXED_UNIT, FIXED_DEC(60,100));
            
            if (playing && (stage.flag & STAGE_FLAG_JUST_STEP))
            {
                //Check if screen should bump
                boolean is_bump_step = (stage.song_step & 0xF) == 0;

                //if stage is cutscene, or cool uproar moment, don't bump
                if (stage.stage_id == StageId_1_5)
                    is_bump_step = NULL;
                if (stage.stage_id == StageId_1_3)
                {
                    if (stage.song_step >= 1150 && stage.song_step <= 1213)
                        is_bump_step = NULL;
                }

                switch (cambump)
                {
                    case 0: //normal, bump every 4 beats
                    {
                        if (is_bump_step)
                            stage.bump = FIXED_DEC(103,100);
                        break;
                    }
                    case 1: //no bumping
                    {
                        is_bump_step = NULL;
                        if (is_bump_step)
                            stage.bump = FIXED_DEC(103,100);
                        break;
                    }
                    case 2: //bump every 2 beats
                    {
                        is_bump_step = (stage.song_step  & 0x7) == 0;
                        if (is_bump_step)
                            stage.bump = FIXED_DEC(103,100);
                        break;
                    }
                    case 3: //bump violently every beat
                    {
                        is_bump_step = (stage.song_step & 0x3) == 0;
                        if (is_bump_step)
                            stage.bump = FIXED_DEC(109,100);
                        break;
                    }
                }
                
                //M.I.L.F bumps
                //if (stage.stage_id == StageId_4_3 && stage.song_step >= (168 << 2) && stage.song_step < (200 << 2))
                //  is_bump_step = (stage.song_step & 0x3) == 0;
                
                //Bump health every 4 steps
                if (noheadbump == 1)
                {
                    if ((stage.song_step & 0x3) == 0)
                    stage.sbump = FIXED_DEC(100,100);
                }
                else
                {
                    if ((stage.song_step & 0x3) == 0)
                    stage.sbump = FIXED_DEC(103,100);
                }
            }
            
            //Scroll camera
            if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
                Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
            else
                Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
            Stage_ScrollCamera();
            
            switch (stage.mode)
            {
                case StageMode_Normal:
                case StageMode_Swap:
                {
                    //Handle player 1 inputs
                    Stage_ProcessPlayer(&stage.player_state[0], &pad_state, playing);
                    
                    //Handle opponent notes
                    u8 opponent_anote = CharAnim_Idle;
                    u8 opponent_snote = CharAnim_Idle;
                    
                    for (Note *note = stage.cur_note;; note++)
                    {
                        if (note->pos > (stage.note_scroll >> FIXED_SHIFT))
                            break;
                        
                        //Opponent note hits
                        if (playing && (note->type & NOTE_FLAG_OPPONENT) && !(note->type & NOTE_FLAG_HIT))
                        {
                            //Opponent hits note
                            Stage_StartVocal();
                            if (note->type & NOTE_FLAG_SUSTAIN)
                                opponent_snote = note_anims[note->type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0];
                            else
                                opponent_anote = note_anims[note->type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0];
                            note->type |= NOTE_FLAG_HIT;
                        }
                    }
                    
                    if (opponent_anote != CharAnim_Idle)
                        stage.opponent->set_anim(stage.opponent, opponent_anote);
                    else if (opponent_snote != CharAnim_Idle)
                        stage.opponent->set_anim(stage.opponent, opponent_snote);
                    break;
                }
                case StageMode_2P:
                {
                    //Handle player 1 and 2 inputs
                    Stage_ProcessPlayer(&stage.player_state[0], &pad_state, playing);
                    Stage_ProcessPlayer(&stage.player_state[1], &pad_state_2, playing);
                    break;
                }
            #ifdef PSXF_NETWORK
                case StageMode_Net1:
                {
                    //Handle player 1 inputs
                    Stage_ProcessPlayer(&stage.player_state[0], &pad_state, playing);
                    break;
                }
                case StageMode_Net2:
                {
                    //Handle player 2 inputs
                    Stage_ProcessPlayer(&stage.player_state[1], &pad_state, playing);
                    break;
                }
            #endif
            }
            
            //Tick note splashes
            ObjectList_Tick(&stage.objlist_splash);
            
            //Draw stage notes
            Stage_DrawNotes();
            
            //Draw note HUD
            RECT note_src = {0, 0, 32, 32};
            RECT_FIXED note_dst = {0, FIXED_DEC(16,1), FIXED_DEC(32,1), FIXED_DEC(32,1)};
 
            for (u8 i = 0; i < 4; i++)
            {
            
               //BF
               note_dst.x = note_x[i ^ stage.note_swap] - FIXED_DEC(16,1);
               note_dst.y = note_y[i ^ stage.note_swap] - FIXED_DEC(16,1);           if (stage.prefs.downscroll)
                note_dst.y = -note_dst.y - note_dst.h;
            
               Stage_DrawStrum(i, &note_src, &note_dst);
               Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
               
               //Opponent
               note_dst.x = note_x[(i | 0x4) ^ stage.note_swap] - FIXED_DEC(16,1);
               note_dst.y = note_y[(i | 0x4) ^ stage.note_swap] - FIXED_DEC(16,1);           if (stage.prefs.downscroll)
                note_dst.y = -note_dst.y - note_dst.h;
            
               Stage_DrawStrum(i | 4, &note_src, &note_dst);
               Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
            
            }
            
            //Draw score
			for (int i = 0; i < ((stage.mode >= StageMode_2P) ? 2 : 1); i++)
			{
				PlayerState *this = &stage.player_state[i];
				
				if(stage.stage_id != StageId_1_5 && stage.stage_id != StageId_1_6)
				{
				if ((this->refresh_score || this->refresh_misses))
				{
					if(stage.prefs.botplay ==0)
					{
						if ((this->score != 0 || this->misses != 0))
							sprintf(this->score_text, "Score: %d0  |  Misses: %d", this->score * stage.max_score / this->max_score, stage.misses);
						else
						{
							strcpy(this->score_text, "Score: 0  |  Misses: ?");
						}
				        }
				        else
				        {
				        	strcpy(this->score_text, "Score: ?  |  Misses: ? | Botplay");
				        }
					this->refresh_score = false;
					this->refresh_misses = false;
				}
				if(stage.mode == StageMode_2P)
				{
					stage.font_cdr.draw(&stage.font_cdr, this->score_text, (i ^ (stage.mode == StageMode_Swap)) ? 160 - 80 : 160 + 80, 220, FontAlign_Center);
				}
				else
				{
					stage.font_cdr.draw(&stage.font_cdr, this->score_text, 160, 220, FontAlign_Center);
				}
				}
					
			}
            
            if (stage.mode < StageMode_2P)
            {
                //Perform health checks
                if (stage.player_state[0].health <= 0)
                {
                    //Player has died
                    stage.player_state[0].health = 0;
                    stage.state = StageState_Dead;
                }
                if (stage.player_state[0].health > 20000)
                    stage.player_state[0].health = 20000;
                
                //Draw health heads
                Stage_DrawHealth(stage.player_state[0].health, stage.player->health_i,    1);
                Stage_DrawHealth(stage.player_state[0].health, stage.opponent->health_i, -1);
                if (stage.stage_id == StageId_1_1)
                {
                //Draw health bar
                RECT health_fill = {0, 0, 256 - (256 * stage.player_state[0].health / 20000), 8};
                RECT health_back = {0, 8, 256, 8};
                RECT_FIXED health_dst = {FIXED_DEC(-128,1), (SCREEN_HEIGHT2 - 32) << FIXED_SHIFT, 0, FIXED_DEC(8,1)};
                if (stage.prefs.downscroll)
                    health_dst.y = -health_dst.y - health_dst.h;
                
                health_dst.w = health_fill.w << FIXED_SHIFT;
                Stage_DrawTex(&stage.tex_hud1, &health_fill, &health_dst, stage.bump);
                health_dst.w = health_back.w << FIXED_SHIFT;
                Stage_DrawTex(&stage.tex_hud1, &health_back, &health_dst, stage.bump);
                }
                else if (stage.stage_id == StageId_1_2)
                {
                //Draw health bar
                RECT health_fill = {0, 0, 256 - (256 * stage.player_state[0].health / 20000), 8};
                RECT health_back = {0, 16, 256, 8};
                RECT_FIXED health_dst = {FIXED_DEC(-128,1), (SCREEN_HEIGHT2 - 32) << FIXED_SHIFT, 0, FIXED_DEC(8,1)};
                if (stage.prefs.downscroll)
                    health_dst.y = -health_dst.y - health_dst.h;
                
                health_dst.w = health_fill.w << FIXED_SHIFT;
                Stage_DrawTex(&stage.tex_hud1, &health_fill, &health_dst, stage.bump);
                health_dst.w = health_back.w << FIXED_SHIFT;
                Stage_DrawTex(&stage.tex_hud1, &health_back, &health_dst, stage.bump);
                }
                else if (stage.stage_id == StageId_1_3)
                {
                //Draw health bar
                RECT health_fill = {0, 0, 256 - (256 * stage.player_state[0].health / 20000), 8};
                RECT health_back = {0, 24, 256, 8};
                RECT_FIXED health_dst = {FIXED_DEC(-128,1), (SCREEN_HEIGHT2 - 32) << FIXED_SHIFT, 0, FIXED_DEC(8,1)};
                if (stage.prefs.downscroll)
                    health_dst.y = -health_dst.y - health_dst.h;
                
                health_dst.w = health_fill.w << FIXED_SHIFT;
                Stage_DrawTex(&stage.tex_hud1, &health_fill, &health_dst, stage.bump);
                health_dst.w = health_back.w << FIXED_SHIFT;
                Stage_DrawTex(&stage.tex_hud1, &health_back, &health_dst, stage.bump);
                }
                else
                {
                //Draw health bar
                RECT health_fill = {0, 32, 256 - (256 * stage.player_state[0].health / 20000), 8};
                RECT health_back = {0, 40, 256, 8};
                RECT_FIXED health_dst = {FIXED_DEC(-128,1), (SCREEN_HEIGHT2 - 32) << FIXED_SHIFT, 0, FIXED_DEC(8,1)};
                if (stage.prefs.downscroll)
                    health_dst.y = -health_dst.y - health_dst.h;
                
                health_dst.w = health_fill.w << FIXED_SHIFT;
                Stage_DrawTex(&stage.tex_hud1, &health_fill, &health_dst, stage.bump);
                health_dst.w = health_back.w << FIXED_SHIFT;
                Stage_DrawTex(&stage.tex_hud1, &health_back, &health_dst, stage.bump);
                }
            }
            
            //uproar fade shit
            if (stage.fadeblack > 0)
            {
                if (stage.fadeblack >= 522000)
                    break;
                else
                {
                    static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                    u8 flash_col = stage.fadeblack >> FIXED_SHIFT;
                    u8 flash_col2 = stage.fadeextra >> FIXED_SHIFT;
                    Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 2);
                    stage.fadeblack += FIXED_MUL(stage.fadespeed, timer_dt*3);  
                }

            
            }

            if (stage.fadeinwhite > 0)
            {

                if (stage.fadeinwhite >= 522000)
                {
                    //This sucks
                    static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                    u8 flash_col = stage.fadeinwhite >> FIXED_SHIFT;
                    u8 flash_col2 = stage.fadeextra >> FIXED_SHIFT;
                    Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
                    break;
                }
                else
                {
                    static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                    u8 flash_col = stage.fadeinwhite >> FIXED_SHIFT;
                    u8 flash_col2 = stage.fadeextra >> FIXED_SHIFT;
                    Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
                    stage.fadeinwhite += FIXED_MUL(stage.fadespeed, timer_dt*3);  
                }

            }

            if (stage.fadewhite > 0)
            {
                static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                u8 flash_col = stage.fadewhite >> FIXED_SHIFT;
                u8 flash_col2 = stage.fadeextra >> FIXED_SHIFT;
                Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
                stage.fadewhite -= FIXED_MUL(stage.fadespeed, timer_dt*3);  

            
            }
            
            //Draw stage foreground
            if (stage.back->draw_fg != NULL)
                stage.back->draw_fg(stage.back);
            
            //Tick foreground objects
            ObjectList_Tick(&stage.objlist_fg);
            
            //Tick characters
            stage.player->tick(stage.player);
            stage.opponent->tick(stage.opponent);
            
            //Draw stage middle
            if (stage.back->draw_md != NULL)
                stage.back->draw_md(stage.back);
            
            //Tick girlfriend
            if (stage.gf != NULL)
                stage.gf->tick(stage.gf);
            
            //Tick background objects
            ObjectList_Tick(&stage.objlist_bg);
            
            //Draw stage background
            if (stage.back->draw_bg != NULL)
                stage.back->draw_bg(stage.back);
            break;
        }
        case StageState_Dead: //Start BREAK animation and reading extra data from CD
        {
            //Stop music immediately
            Audio_StopXA();
            
            //Unload stage data
            Mem_Free(stage.chart_data);
            stage.chart_data = NULL;
            
            //Free background
            stage.back->free(stage.back);
            stage.back = NULL;
            
            //Free objects
            ObjectList_Free(&stage.objlist_fg);
            ObjectList_Free(&stage.objlist_bg);
            
            //Free opponent and girlfriend
            Stage_SwapChars();
            Character_Free(stage.opponent);
            stage.opponent = NULL;
            Character_Free(stage.gf);
            stage.gf = NULL;
            
            //Reset stage state
            stage.flag = 0;
            stage.bump = stage.sbump = FIXED_UNIT;
            
            //Change background colour to black
            Gfx_SetClear(0, 0, 0);
            
            //Run death animation, focus on player, and change state
            stage.player->set_anim(stage.player, PlayerAnim_Dead0);
            
            Stage_FocusCharacter(stage.player, 0);
            stage.song_time = 0;
            
            stage.state = StageState_DeadLoad;
        }
    //Fallthrough
        case StageState_DeadLoad:
        {
            //Scroll camera and tick player
            if (stage.song_time < FIXED_UNIT)
                stage.song_time += FIXED_UNIT / 60;
            stage.camera.td = FIXED_DEC(-2, 100) + FIXED_MUL(stage.song_time, FIXED_DEC(45, 1000));
            if (stage.camera.td > 0)
                Stage_ScrollCamera();
            stage.player->tick(stage.player);
            
            //Drop mic and change state if CD has finished reading and animation has ended
            if (IO_IsReading() || stage.player->animatable.anim != PlayerAnim_Dead1)
                break;
            
            stage.player->set_anim(stage.player, PlayerAnim_Dead2);
            stage.camera.td = FIXED_DEC(25, 1000);
            stage.state = StageState_DeadDrop;
            break;
        }
        case StageState_DeadDrop:
        {
            //Scroll camera and tick player
            Stage_ScrollCamera();
            stage.player->tick(stage.player);
            
            //Enter next state once mic has been dropped
            if (stage.player->animatable.anim == PlayerAnim_Dead3)
            {
                stage.state = StageState_DeadRetry;
                Audio_PlayXA_Track(XA_GameOver, 0x40, 1, true);
            }
            break;
        }
        case StageState_DeadRetry:
        {
        
            if (pad_state.press & PAD_CROSS)
		{
		    stage.trans = StageTrans_Reload;
		    Trans_Start();
		}
	    if (pad_state.press & PAD_CIRCLE)
		{
		    stage.trans = StageTrans_Menu;
		    Trans_Start();
		}
		
            //Randomly twitch
            if (stage.player->animatable.anim == PlayerAnim_Dead3)
            {
                if (RandomRange(0, 29) == 0)
                    stage.player->set_anim(stage.player, PlayerAnim_Dead4);
                if (RandomRange(0, 29) == 0)
                    stage.player->set_anim(stage.player, PlayerAnim_Dead5);
            }
            
            //Scroll camera and tick player
            Stage_ScrollCamera();
            stage.player->tick(stage.player);
            break;
        }
        case StageState_Dialogue:
        {
            //oh boy

            RECT dia_src = {0, 0, 227, 63};
            RECT_FIXED dia_dst = {FIXED_DEC(-150,1), FIXED_DEC(30,1), FIXED_DEC(297,1), FIXED_DEC(76,1)};

            //???
            Stage *this = (Stage*)this;

            static const struct
            {
                const char *text; //The text that is displayed
                u8 camera; //Who the camera is pointing at, 0 for bf, 1 for dad
                int port1;//opponent
                int port2;//player
            }psydia[] = {
                {"What brings you here so late at night?",1, 0, 15},
                {"Beep.",0, 15, 6},
                {"Drop the act already.",1, 3, 15},
                {"I could feel your malicious intent the\nmoment you set foot in here.",1, 0, 15},
                {"Bep bee aa skoo dep?",0, 15, 12},
                {"I wouldn't try the door if I were you.",1, 4, 15},
                {"Now...",1, 2, 15},
                {"I have a couple of questions to ask you...",1, 0, 15},
                {"And you WILL answer them.",1, 3, 15},
            };
            

            static const struct
            {
                const char *text;
                u8 camera;
                int port1;
                int port2;
            }wiltdia[] = {
                {"Welp, you got me!",0, 15, 7},
                {"You're very clever, I'll give you that much.",0, 15, 7},
                {"No ordinary person would have seen\nthrough my facade.",0, 15, 7},
                {"Yeah, um...",1, 5, 15},
                {"...Who are you again?",1, 4, 15},
                {"Kh...!",0, 15, 8},
                {"You don't even remember me?!",0, 15, 8},
                {"Not in the slightest.",1, 2, 15},
                {"Seriously?! W-Whatever!",0, 15, 9},
                {"Now listen here!",0, 15, 9},
                {"I've taken this body hostage, so\ndon't even try anything!",0, 15, 9},
                {"Summon Daddy Dearest here this instant,\nor else he gets it!",0, 15, 9},
                {"...Daddy Dearest, huh..?",1, 2, 15},
                {"I don't know what your deal is, but...",1, 5, 15},
                {"I don't take commands from freaks of\nnature like you.",1, 3, 15},
                {"What did you just call me?!",0, 15, 10},
            };

            static const struct
            {
                const char *text;
                u8 camera;
                int port1;
                int port2;
            }uproardia[] = {
            
                {"At least that guy's gone, he was\ngetting on my nerves.",1, 1, 15},
                {"Let me guess, you're another thorn\nin my side, huh?",1, 5, 15},
                {". . .",0, 15, 11},
                {"...You took the words straight\nfrom my mouth...",0, 15, 11},
                {"I had finally escaped that wretched\ngame, and of course YOU are here to greet me...",0, 15, 11},
                {"...No matter...",0, 15, 11},
                {"...I'll just kill you and finally get\nmy revenge... It's really that simple...",0, 15, 11},
                {"...You don't mind your body being\ndissipated, right? It's only fair...",0, 15, 11},
                {"You took the words straight from my mouth.",1, 3, 15},
            };

            static const struct
            {
                const char *text;
                u8 camera;
            }latedrivedia[] = {
                {"Huh?",1},
                {"Where are we?",1},
                {"Beep?",0},
                {"That much is obvious.",1},
                {"It seems that we are stuck in some sort of\nalternate reality...",1},
                {"Eep skee dah?",0},
                {"Since we're here anyway, I suppose one song\ncouldn't hurt.",1},
            };
            static const struct
            {
                const char *text; //The text that is displayed
                u8 camera; //Who the camera is pointing at, 0 for bf, 1 for dad
                int port1;
                int port2;
            }flopdia[] = {
                {"Meow!",1, 13, 15},
                {"Beep.",0, 15, 6},
                {"HISSS",1, 14, 15},
                {"Bep bee aa skoo dep?",0, 15, 12},
                {"Meow Meow",1, 13, 15},
                {"Hiss",1, 14, 15},
                {"Beep bop!",0, 15, 6},
                {"HISSSS!!",1, 14, 15},
            };

            //Clear per-frame flags
            stage.flag &= ~(STAGE_FLAG_JUST_STEP | STAGE_FLAG_SCORE_REFRESH);

            //play dialogue song
            if (Audio_PlayingXA() != 1)
            {
                Audio_PlayXA_Track(stage.stage_def->diasong, 0x40, stage.stage_def->dia_channel, true); //read stagedef and play song
            }

            //text drawing shit
            switch (stage.stage_id)
            {
                case StageId_1_1:
                {
                    //Animatable_Animate(&this->psytalk_animatable, (void*)this, PsyTalk_SetFrame);

                    stage.font_arial.draw_col(&stage.font_arial,
                        psydia[stage.delect].text,
                        25,
                        170,
                        FontAlign_Left,
                        0 >> 1,
                        0 >> 1,
                        0 >> 1
                    );
                    Stage_DrawDiaPorts(psydia[stage.delect].port1, 70, 97);
                    Stage_DrawDiaPorts(psydia[stage.delect].port2, 180, 94);
                    if (stage.delect == 9)
                    {
                        Audio_StopXA();
                        Stage_LoadStage();
                        stage.state = StageState_Play;
                    }

                    //camera shit
                    if (psydia[stage.delect].camera == 1)
                        Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
                    else
                        Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
                        
                    break;
                }

                case StageId_1_2:
                {
                    stage.font_arial.draw_col(&stage.font_arial,
                        wiltdia[stage.delect].text,
                        25,
                        170,
                        FontAlign_Left,
                        0 >> 1,
                        0 >> 1,
                        0 >> 1
                    );
		    Stage_DrawDiaPorts(wiltdia[stage.delect].port1, 70, 97);
                    Stage_DrawDiaPorts(wiltdia[stage.delect].port2, 180, 94);
                    if (stage.delect == 16)
                    {
                        Audio_StopXA();
                        Stage_LoadStage();
                        stage.state = StageState_Play;
                    }

                    if (wiltdia[stage.delect].camera == 1)
                        Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
                    else
                        Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
                    break;
                }

                case StageId_1_3:
                {
                    stage.font_arial.draw_col(&stage.font_arial,
                        uproardia[stage.delect].text,
                        25,
                        170,
                        FontAlign_Left,
                        0 >> 1,
                        0 >> 1,
                        0 >> 1
                    );
                    Stage_DrawDiaPorts(uproardia[stage.delect].port1, 70, 97);
                    Stage_DrawDiaPorts(uproardia[stage.delect].port2, 180, 94);
                    if (stage.delect == 9)
                    {
                        Audio_StopXA();
                        //Stage_LoadStage();
                        stage.state = StageState_Play;
                    }
                    if (uproardia[stage.delect].camera == 1)
                        Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
                    else
                        Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
                    break;
                }

                case StageId_2_2:
                {//sus
                    stage.font_arial.draw_col(&stage.font_arial,
                        latedrivedia[stage.delect].text,
                        25,
                        170,
                        FontAlign_Left,
                        0 >> 1,
                        0 >> 1,
                        0 >> 1
                    );

                    if (stage.delect == 7)
                    {
                        Audio_StopXA();
                        Stage_LoadStage();
                        stage.state = StageState_Play;
                    }

                    if (latedrivedia[stage.delect].camera == 1)
                        Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
                    else
                        Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
                    break;
                }
                case StageId_2_3:
                {
                    //Animatable_Animate(&this->psytalk_animatable, (void*)this, PsyTalk_SetFrame);

                    stage.font_arial.draw_col(&stage.font_arial,
                        flopdia[stage.delect].text,
                        25,
                        170,
                        FontAlign_Left,
                        0 >> 1,
                        0 >> 1,
                        0 >> 1
                    );
                    Stage_DrawDiaPorts(flopdia[stage.delect].port1, 70, 105);
                    Stage_DrawDiaPorts(flopdia[stage.delect].port2, 180, 94);
                    if (stage.delect == 8)
                    {
                        Audio_StopXA();
                        Stage_LoadStage();
                        stage.state = StageState_Play;
                    }

                    //camera shit
                    if (flopdia[stage.delect].camera == 1)
                        Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
                    else
                        Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
                        
                    break;
                }

                default:
                    break;
            }

            Stage_DrawTex(&stage.tex_dia, &dia_src, &dia_dst, stage.bump);
            
            static const RECT walterwhite = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

            Gfx_BlendRect(&walterwhite, 255, 255, 255, 255);


            //Draw stage foreground
            if (stage.back->draw_fg != NULL)
                stage.back->draw_fg(stage.back);
            
            //Tick foreground objects
            ObjectList_Tick(&stage.objlist_fg);
            
            //Tick characters
            stage.player->tick(stage.player);
            stage.opponent->tick(stage.opponent);
            
            //Draw stage middle
            if (stage.back->draw_md != NULL)
                stage.back->draw_md(stage.back);
            
            //Tick girlfriend
            if (stage.gf != NULL)
                stage.gf->tick(stage.gf);
            
            //Tick background objects
            ObjectList_Tick(&stage.objlist_bg);
            
            //Draw stage background
            if (stage.back->draw_bg != NULL)
                stage.back->draw_bg(stage.back);

            //skip dialogue
            if (pad_state.press & PAD_START)
            {
            	if (stage.stage_id != StageId_1_3)
            	{
                Audio_StopXA();
                Stage_LoadStage();
                stage.state = StageState_Play;
		        if (stage.stage_id == StageId_1_2)
		        {
		        stage.delect =16;
		        }
                }
                else
                {
                Audio_StopXA();
                stage.state = StageState_Play;
                }
            }
            //progress to next message
            if (pad_state.press & PAD_CROSS)
            {
                stage.delect++;
            }

            Stage_ScrollCamera();






            
            break;
        }
        default:
            break;
    }
}

#ifdef PSXF_NETWORK
void Stage_NetHit(Packet *packet)
{
    //Reject if not in stage
    if (gameloop != GameLoop_Stage)
        return;
    
    //Get packet info
    u16 i = ((*packet)[1] << 0) | ((*packet)[2] << 8);
    u32 hit_score = ((*packet)[3] << 0) | ((*packet)[4] << 8) | ((*packet)[5] << 16) | ((*packet)[6] << 24);
    u8 hit_type = (*packet)[7];
    u16 hit_combo = ((*packet)[8] << 0) | ((*packet)[9] << 8);
    
    //Get note pointer
    if (i >= stage.num_notes)
        return;
    
    Note *note = &stage.notes[i];
    u8 type = note->type & 0x3;
    
    u8 opp_flag = (stage.mode == StageMode_Net1) ? NOTE_FLAG_OPPONENT : 0;
    if ((note->type & NOTE_FLAG_OPPONENT) != opp_flag)
        return;
    
    //Update game state
    PlayerState *this = &stage.player_state[(stage.mode == StageMode_Net1) ? 1 : 0];
    stage.notes[i].type |= NOTE_FLAG_HIT;
    
    this->score = hit_score;
    this->refresh_score = true;
    this->combo = hit_combo;
    
    if (note->type & NOTE_FLAG_SUSTAIN)
    {
        //Hit a sustain
        Stage_StartVocal();
        this->arrow_hitan[type] = stage.step_time;
        this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
    }
    else if (!(note->type & NOTE_FLAG_MINE))
    {
        //Hit a note
        Stage_StartVocal();
        this->arrow_hitan[type] = stage.step_time;
        this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
        
        //Create combo object
        Obj_Combo *combo = Obj_Combo_New(
            this->character->focus_x,
            this->character->focus_y,
            hit_type,
            this->combo >= 10 ? this->combo : 0xFFFF
        );
        if (combo != NULL)
            ObjectList_Add(&stage.objlist_fg, (Object*)combo);
        
        //Create note splashes if SICK
        if (hit_type == 0)
        {
            for (int i = 0; i < 3; i++)
            {
                //Create splash object
                Obj_Splash *splash = Obj_Splash_New(
                    note_x[(note->type & 0x7) ^ stage.note_swap],
                    note_y * (stage.prefs.downscroll ? -1 : 1),
                    type
                );
                if (splash != NULL)
                    ObjectList_Add(&stage.objlist_splash, (Object*)splash);
            }
        }
    }
    else
    {
        //Hit a mine
        this->arrow_hitan[type & 0x3] = -1;
        if (this->character->spec & CHAR_SPEC_MISSANIM)
            this->character->set_anim(this->character, note_anims[type & 0x3][2]);
        else
            this->character->set_anim(this->character, note_anims[type & 0x3][0]);
    }
}

void Stage_NetMiss(Packet *packet)
{
    //Reject if not in stage
    if (gameloop != GameLoop_Stage)
        return;
    
    //Get packet info
    u8 type = (*packet)[1];
    u32 hit_score = ((*packet)[2] << 0) | ((*packet)[3] << 8) | ((*packet)[4] << 16) | ((*packet)[5] << 24);
    
    //Update game state
    PlayerState *this = &stage.player_state[(stage.mode == StageMode_Net1) ? 1 : 0];
    
    this->score = hit_score;
    this->refresh_score = true;
    
    //Missed
    if (!(type & ~0x3))
    {
        this->arrow_hitan[type] = -1;
        if (this->character->spec & CHAR_SPEC_MISSANIM)
            this->character->set_anim(this->character, note_anims[type][2]);
        else
            this->character->set_anim(this->character, note_anims[type][0]);
    }
    Stage_MissNote(this);
}
#endif
