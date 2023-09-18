/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "menu.h"

#include "mem.h"
#include "main.h"
#include "timer.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"
#include "archive.h"
#include "mutil.h"
#include "network.h"

#include "font.h"
#include "trans.h"
#include "loadscr.h"

#include "stage.h"
#include "save.h"
#include "character/gf.h"

#include "stdlib.h"

#include "character/titlepsy.h"
#include "character/mbf.h"
#include "character/mgf.h"

//int menumusic = 0;
//boolean secretunlocked = true;
boolean secretunlocked2 = false;
const char *storylock = "9";
int iconlock = 4;
int iconlock2 = 4;
const char *lockpsyo = "UNKNOWN";
const char *locklate = "UNKNOWN";
const char *lockflop = "UNKNOWN";
int wicon =10;
int mwicon =10;
int mpicon =10;
int mwticon =10;
int muicon =10;
int duicon =10;
int udicon =10;
int fgicon =10;
int dbicon =10;


//Menu messages
static const char *funny_messages[][2] = {
    {"PSX PORT BY CUCKYDEV", "YOU KNOW IT"},
    {"PORTED BY CUCKYDEV", "WHAT YOU GONNA DO"},
    {"FUNKIN", "FOREVER"},
    {"WHAT THE HELL", "RITZ PSX"},
    {"LIKE PARAPPA", "BUT COOLER"},
    {"THE JAPI", "EL JAPI"},
    {"PICO FUNNY", "PICO FUNNY"},
    {"OPENGL BACKEND", "BY CLOWNACY"},
    {"CUCKYFNF", "SETTING STANDARDS"},
    {"lool", "inverted colours"},
    {"NEVER LOOK AT", "THE ISSUE TRACKER"},
    {"PSXDEV", "HOMEBREW"},
    {"ZERO POINT ZERO TWO TWO EIGHT", "ONE FIVE NINE ONE ZERO FIVE"},
    {"DOPE ASS GAME", "PLAYSTATION MAGAZINE"},
    {"NEWGROUNDS", "FOREVER"},
    {"NO FPU", "NO PROBLEM"},
    {"OK OKAY", "WATCH THIS"},
    {"ITS MORE MALICIOUS", "THAN ANYTHING"},
    {"USE A CONTROLLER", "LOL"},
    {"SNIPING THE KICKSTARTER", "HAHA"},
    {"SHITS UNOFFICIAL", "NOT A PROBLEM"},
    {"SYSCLK", "RANDOM SEED"},
    {"THEY DIDNT HIT THE GOAL", "STOP"},
    {"FCEFUWEFUETWHCFUEZDSLVNSP", "PQRYQWENQWKBVZLZSLDNSVPBM"},
    {"THE FLOORS ARE", "THREE DIMENSIONAL"},
    {"PSXFUNKIN BY CUCKYDEV", "SUCK IT DOWN"},
    {"PLAYING ON EPSXE HUH", "YOURE THE PROBLEM"},
    {"NEXT IN LINE", "ATARI"},
    {"HAXEFLIXEL", "COME ON"},
    {"HAHAHA", "I DONT CARE"},
    {"GET ME TO STOP", "TRY"},
    {"FNF MUKBANG GIF", "THATS UNRULY"},
    {"OPEN SOURCE", "FOREVER"},
    {"ITS A PORT", "ITS WORSE"},
    {"WOW GATO", "WOW GATO"},
    {"BALLS FISH", "BALLS FISH"},
};

#ifdef PSXF_NETWORK

//Menu string type
#define MENUSTR_CHARS 0x20
typedef char MenuStr[MENUSTR_CHARS + 1];

#endif

//Menu state
static struct
{
    //Menu state
    u8 page, next_page;
    boolean page_swap;
    u8 select, next_select;
    
    fixed_t scroll; 
    fixed_t trans_time;
    
    //Page specific state
    union
    {
        struct
        {
            u8 funny_message;
        } opening;
        struct
        {
            fixed_t logo_bump;
            fixed_t fade, fadespd;
        } title;
        struct
        {
            fixed_t fade, fadespd;
        } story;
        struct
        {
            fixed_t back_r, back_g, back_b;
        } freeplay;
    #ifdef PSXF_NETWORK
        struct
        {
            boolean type;
            MenuStr port;
            MenuStr pass;
        } net_host;
        struct
        {
            boolean type;
            MenuStr ip;
            MenuStr port;
            MenuStr pass;
        } net_join;
        struct
        {
            boolean swap;
        } net_op;
    #endif
    } page_state;
    
    union
    {
        struct
        {
            u8 id, diff;
            boolean story;
        } stage;
    } page_param;
    
    //Menu assets
    Gfx_Tex tex_back, tex_ng, tex_story, tex_title, tex_week;
    IO_Data weeks, weeks_ptrs[128];
    int curweek;

    FontData font_bold, font_arial, font_arialr, font_arialb;
    
    Character *Titlepsy; //Title Psychic
    Character *mbf; //Menu Bf
    Character *mgf; //Menu Gf
} menu;

static void CheckAndLoadWeek(int week)
{
    if (menu.curweek != week)
    {
        char weektxt[20];
        sprintf(weektxt, "week%d.tim", week);
        Gfx_LoadTex(&menu.tex_week, Archive_Find(menu.weeks, weektxt), 0);
        menu.curweek = week;
    }

}
#ifdef PSXF_NETWORK

//Menu string functions
static void MenuStr_Process(MenuStr *this, s32 x, s32 y, const char *fmt, boolean select, boolean type)
{
    //Append typed input
    if (select && type)
    {
        if (pad_type[0] != '\0')
            strncat(*this, pad_type, MENUSTR_CHARS - strlen(*this));
        if (pad_backspace)
        {
            size_t i = strlen(*this);
            if (i != 0)
                (*this)[i - 1] = '\0';
        }
    }
    
    //Get text to draw
    char buf[0x100];
    sprintf(buf, fmt, *this);
    if (select && type && (animf_count & 2))
        strcat(buf, "_");
    
    //Draw text
    menu.font_arial.draw_col(&menu.font_arial, buf, x, y, FontAlign_Left, 0x80, 0x80, select ? 0x00 : 0x80);
    menu.font_arial.draw_col(&menu.font_arial, buf, x+1, y+1, FontAlign_Left, 0x00, 0x00, 0x00);
    
    //Draw text
    menu.font_arialr.draw_col(&menu.font_arialr, buf, x, y, FontAlign_Left, 0x80, 0x80, select ? 0x00 : 0x80);
    menu.font_arialr.draw_col(&menu.font_arialr, buf, x+1, y+1, FontAlign_Left, 0x00, 0x00, 0x00);
    
    //Draw text
    menu.font_arialb.draw_col(&menu.font_arialb, buf, x, y, FontAlign_Left, 0x80, 0x80, select ? 0x00 : 0x80);
    menu.font_arialb.draw_col(&menu.font_arialb, buf, x+1, y+1, FontAlign_Left, 0x00, 0x00, 0x00);
}

#endif


//Internal menu functions
char menu_text_buffer[0x100];

static const char *Menu_LowerIf(const char *text, boolean lower)
{
    //Copy text
    char *dstp = menu_text_buffer;
    if (lower)
    {
        for (const char *srcp = text; *srcp != '\0'; srcp++)
        {
            if (*srcp >= 'A' && *srcp <= 'Z')
                *dstp++ = *srcp | 0x20;
            else
                *dstp++ = *srcp;
        }
    }
    else
    {
        for (const char *srcp = text; *srcp != '\0'; srcp++)
        {
            if (*srcp >= 'a' && *srcp <= 'z')
                *dstp++ = *srcp & ~0x20;
            else
                *dstp++ = *srcp;
        }
    }
    
    //Terminate text
    *dstp++ = '\0';
    return menu_text_buffer;
}

static void Menu_DrawBack(boolean flash, s32 scroll, u8 r0, u8 g0, u8 b0, u8 r1, u8 g1, u8 b1)
{
    RECT back_src = {0, 0, 255, 255};
    RECT back_dst = {0, -scroll - SCREEN_WIDEADD2, SCREEN_WIDTH, SCREEN_WIDTH * 4 / 5};
    
    if (flash || (animf_count & 4) == 0)
        Gfx_DrawTexCol(&menu.tex_back, &back_src, &back_dst, r0, g0, b0);
    else
        Gfx_DrawTexCol(&menu.tex_back, &back_src, &back_dst, r1, g1, b1);
}

static void Menu_DifficultySelector(s32 x, s32 y)
{
    //Change difficulty
    if (menu.next_page == menu.page && Trans_Idle())
    {
        if (pad_state.press & PAD_LEFT)
        {
            if (menu.page_param.stage.diff > StageDiff_Easy)
                menu.page_param.stage.diff--;
            else
                menu.page_param.stage.diff = StageDiff_Hard;
        }
        if (pad_state.press & PAD_RIGHT)
        {
            if (menu.page_param.stage.diff < StageDiff_Hard)
                menu.page_param.stage.diff++;
            else
                menu.page_param.stage.diff = StageDiff_Easy;
        }
    }
    
    //Draw difficulty arrows
    static const RECT arrow_src[2][2] = {
        {{224, 64, 16, 32}, {224, 96, 16, 32}}, //left
        {{240, 64, 16, 32}, {240, 96, 16, 32}}, //right
    };
    
    Gfx_BlitTex(&menu.tex_story, &arrow_src[0][(pad_state.held & PAD_LEFT) != 0], x - 40 - 16, y - 16);
    Gfx_BlitTex(&menu.tex_story, &arrow_src[1][(pad_state.held & PAD_RIGHT) != 0], x + 40, y - 16);
    
    //Draw difficulty
    static const RECT diff_srcs[] = {
        {  0, 96, 64, 18},
        { 64, 96, 80, 18},
        {144, 96, 64, 18},
    };
    
    const RECT *diff_src = &diff_srcs[menu.page_param.stage.diff];
    Gfx_BlitTex(&menu.tex_story, diff_src, x - (diff_src->w >> 1), y - 9 + ((pad_state.press & (PAD_LEFT | PAD_RIGHT)) != 0));
}
static void Menu_DrawWeek(const char *week, s32 x, s32 y)
{
    //Draw label
    if (week == NULL)
    {
        //Tutorial
        RECT label_src = {0, 0, 112, 32};
        Gfx_BlitTex(&menu.tex_story, &label_src, x, y);
    }
    else
    {
        //Week
        RECT label_src = {0, 32, 80, 32};
        Gfx_BlitTex(&menu.tex_story, &label_src, x, y);
        
        //Number
        x += 80;
        for (; *week != '\0'; week++)
        {
            //Draw number
            u8 i = *week - '0';
            
            RECT num_src = {128 + ((i & 3) << 5), ((i >> 2) << 5), 32, 32};
            Gfx_BlitTex(&menu.tex_story, &num_src, x, y);
            x += 32;
        }
    }
}

static void Menu_DrawBG(s32 x, s32 y)
{
    //Draw Track
    RECT week1_src = {0, 0, 320, 136};
    Gfx_BlitTex(&menu.tex_week, &week1_src, x, y);
}
static void Menu_DrawTrack(s32 x, s32 y)
{
    //Draw Track
    RECT track_src = {0, 64, 80, 16};
    Gfx_BlitTex(&menu.tex_story, &track_src, x, y);
}
static void Menu_DrawSavingicon(s32 x, s32 y)
{
    //Draw Track
    RECT save_src = {192, 224, 32, 32};
    Gfx_BlitTex(&stage.tex_hud1, &save_src, x, y);
}
static void Menu_DrawNB(s32 x, s32 y)
{
    RECT nb_src = {160, 0, 64, 64};
    Gfx_BlitTex(&stage.tex_hud1, &nb_src, x, y);
}
static void Menu_DrawCKD(s32 x, s32 y)
{
    RECT ckd_src = {160, 64, 64, 64};
    Gfx_BlitTex(&stage.tex_hud1, &ckd_src, x, y);
}
static void Menu_DrawBD(s32 x, s32 y)
{
    RECT bd_src = {160, 128, 64, 64};
    Gfx_BlitTex(&stage.tex_hud1, &bd_src, x, y);
}
static void Menu_DrawHealth(u8 i, s16 x, s16 y, boolean is_selected)
{
    //Icon Size
    u8 icon_size = 32;

    u8 col = (is_selected) ? 128 : 64;

    //Get src and dst
    RECT src = {
        (i % 5) * icon_size,
        (i / 5) * icon_size,
        icon_size,
        icon_size
    };
    RECT dst = {
        x,
        y,
        32,
        32
    };
    
    //Draw health icon
    Gfx_DrawTexCol(&stage.tex_hud1, &src, &dst, col, col, col);
}

//Menu functions
void Menu_Load(MenuPage page)
{
    readSaveFile();
    //Load menu assets
    IO_Data menu_arc = IO_Read("\\MENU\\MENU.ARC;1");
    Gfx_LoadTex(&menu.tex_back,  Archive_Find(menu_arc, "back.tim"),  0);
    Gfx_LoadTex(&menu.tex_ng,    Archive_Find(menu_arc, "ng.tim"),    0);
    Gfx_LoadTex(&menu.tex_story, Archive_Find(menu_arc, "story.tim"), 0);
    Gfx_LoadTex(&menu.tex_title, Archive_Find(menu_arc, "title.tim"), 0);
    Gfx_LoadTex(&stage.tex_hud1, Archive_Find(menu_arc, "hud1.tim"), 0);
    Mem_Free(menu_arc);
    
    menu.weeks = IO_Read("\\MENU\\WEEK.ARC;1");
    Gfx_LoadTex(&menu.tex_week, Archive_Find(menu.weeks, "week0.tim"), 0);
    
    FontData_Load(&menu.font_bold, Font_Bold);
    FontData_Load(&menu.font_arial, Font_Arial);
    FontData_Load(&menu.font_arialr, Font_Arialr);
    FontData_Load(&menu.font_arialb, Font_Arialb);
    
    menu.Titlepsy = Char_Titlepsy_New(FIXED_DEC(45,1), FIXED_DEC(94,1));
    menu.mbf = Char_Mbf_New(FIXED_DEC(-0,1), FIXED_DEC(40,1));
    menu.mgf = Char_Mgf_New(FIXED_DEC(100,1), FIXED_DEC(30,1));
    stage.camera.x = stage.camera.y = FIXED_DEC(0,1);
    stage.camera.bzoom = FIXED_UNIT;
    stage.gf_speed = 4;
    
    //Initialize menu state
    menu.select = menu.next_select = 0;
    
    switch (menu.page = menu.next_page = page)
    {
        case MenuPage_Opening:
            //Get funny message to use
            //Do this here so timing is less reliant on VSync
            #ifdef PSXF_PC
                menu.page_state.opening.funny_message = time(NULL) % COUNT_OF(funny_messages);
            #else
                menu.page_state.opening.funny_message = ((*((volatile u32*)0xBF801120)) >> 3) % COUNT_OF(funny_messages); //sysclk seeding
            #endif
            break;
        default:
            break;
    }
    menu.page_swap = true;
    
    menu.trans_time = 0;
    Trans_Clear();
    
    stage.song_step = 0;
    
    /*//Play menu music
	    Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
	    Audio_WaitPlayXA();*/
    if (stage.prefs.menumusic == 1)
    {
	    //Play menu music
	    Audio_PlayXA_Track(XA_Freeky, 0x40, 2, 1);
	    Audio_WaitPlayXA();
    }
    else if (stage.prefs.menumusic ==2)
    {
    	//Play menu music
	    Audio_PlayXA_Track(XA_Ludum, 0x40, 3, 1);
	    Audio_WaitPlayXA();
    }
    else
    {
    	//Play menu music
	    Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
	    Audio_WaitPlayXA();
    }
    
    //Set background colour
    Gfx_SetClear(0, 0, 0);
}

void Menu_Unload(void)
{
    //Free title Girlfriend
    Character_Free(menu.Titlepsy);
    Character_Free(menu.mgf);
    Character_Free(menu.mbf);
    Mem_Free(menu.weeks);
}

void Menu_ToStage(StageId id, StageDiff diff, boolean story)
{
    menu.next_page = MenuPage_Stage;
    menu.page_param.stage.id = id;
    menu.page_param.stage.story = story;
    menu.page_param.stage.diff = diff;
    Trans_Start();
}

void Menu_Tick(void)
{
    //Clear per-frame flags
    stage.flag &= ~STAGE_FLAG_JUST_STEP;
    
    //Get song position
    u16 next_step = Audio_TellXA_Milli() / 147; //100 BPM
    if (next_step != stage.song_step)
    {
        if (next_step >= stage.song_step)
            stage.flag |= STAGE_FLAG_JUST_STEP;
        stage.song_step = next_step;
    }
    
    //Handle transition out
    if (Trans_Tick())
    {
        //Change to set next page
        menu.page_swap = true;
        menu.page = menu.next_page;
        menu.select = menu.next_select;
    }
    
    //Tick menu page
    MenuPage exec_page;
    switch (exec_page = menu.page)
    {
        case MenuPage_Opening:
        {
            u16 beat = stage.song_step >> 2;
            
            //Start title screen if opening ended
            if (beat >= 16)
            {
                menu.page = menu.next_page = MenuPage_Title;
                menu.page_swap = true;
                //Fallthrough
            }
            else
            {
                //Start title screen if start pressed
                if (pad_state.held & PAD_START)
                    menu.page = menu.next_page = MenuPage_Title;
                
                //Draw different text depending on beat
                RECT src_ng = {0, 0, 128, 128};
                const char **funny_message = funny_messages[menu.page_state.opening.funny_message];
                
                switch (beat)
                {
                    case 3:
                        menu.font_bold.draw(&menu.font_bold, "SOME CLEVER FOLK", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
                //Fallthrough
                    case 2:
                    case 1:
                        menu.font_bold.draw(&menu.font_bold, "PSX MIND GAMES",   SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
			menu.font_bold.draw(&menu.font_bold, "BROUGHT TO YOU BY", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48, FontAlign_Center);
                        break;
                    
                    case 7:
                        menu.font_bold.draw(&menu.font_bold, "NEWGROUNDS",    SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
                        Gfx_BlitTex(&menu.tex_ng, &src_ng, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 16);
                //Fallthrough
                    case 6:
                    case 5:
                        menu.font_bold.draw(&menu.font_bold, "NOT IN ASSOCIATION", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
                        menu.font_bold.draw(&menu.font_bold, "WITH",           SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48, FontAlign_Center);
                        break;
                    
                    case 11:
                        menu.font_bold.draw(&menu.font_bold, funny_message[1], SCREEN_WIDTH2, SCREEN_HEIGHT2, FontAlign_Center);
                //Fallthrough
                    case 10:
                    case 9:
                        menu.font_bold.draw(&menu.font_bold, funny_message[0], SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16, FontAlign_Center);
                        break;
                    
                    case 15:
                        menu.font_bold.draw(&menu.font_bold, "FUNKIN", SCREEN_WIDTH2, SCREEN_HEIGHT2 + 8, FontAlign_Center);
                //Fallthrough
                    case 14:
                        menu.font_bold.draw(&menu.font_bold, "NIGHT", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 8, FontAlign_Center);
                //Fallthrough
                    case 13:
                        menu.font_bold.draw(&menu.font_bold, "FRIDAY", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 24, FontAlign_Center);
                        break;
                }
                break;
            }
        }
    //Fallthrough
        case MenuPage_Title:
        {
            //Initialize page
            if (menu.page_swap)
            {
                menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
                menu.page_state.title.fade = FIXED_DEC(255,1);
                menu.page_state.title.fadespd = FIXED_DEC(90,1);
            }
            
            //Draw white fade
            if (menu.page_state.title.fade > 0)
            {
                static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
                Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
                menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
            }
            
            //Go to main menu when start is pressed
            if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
                Trans_Start();
            
            if ((pad_state.press & PAD_START) && menu.next_page == menu.page && Trans_Idle())
            {
                menu.trans_time = FIXED_UNIT;
                menu.page_state.title.fade = FIXED_DEC(255,1);
                menu.page_state.title.fadespd = FIXED_DEC(300,1);
                menu.next_page = MenuPage_Main;
                menu.next_select = 0;
            }
            
            //Draw Friday Night Funkin' logo
            if ((stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0x3) == 0 && menu.page_state.title.logo_bump == 0)
                menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
            
            static const fixed_t logo_scales[] = {
                FIXED_DEC(1,1),
                FIXED_DEC(101,100),
                FIXED_DEC(102,100),
                FIXED_DEC(103,100),
                FIXED_DEC(105,100),
                FIXED_DEC(110,100),
                FIXED_DEC(97,100),
            };
            fixed_t logo_scale = logo_scales[(menu.page_state.title.logo_bump * 24) >> FIXED_SHIFT];
            u32 x_rad = (logo_scale * (176 >> 1)) >> FIXED_SHIFT;
            u32 y_rad = (logo_scale * (112 >> 1)) >> FIXED_SHIFT;
            
            RECT logo_src = {0, 0, 176, 112};
            RECT logo_dst = {
                100 - x_rad + (SCREEN_WIDEADD2 >> 1),
                68 - y_rad,
                x_rad << 1,
                y_rad << 1
            };
            Gfx_DrawTex(&menu.tex_title, &logo_src, &logo_dst);
            
            if (menu.page_state.title.logo_bump > 0)
                if ((menu.page_state.title.logo_bump -= timer_dt) < 0)
                    menu.page_state.title.logo_bump = 0;
            
            //Draw "Press Start to Begin"
            if (menu.next_page == menu.page)
            {
                //Blinking blue
                s16 press_lerp = (MUtil_Cos(animf_count << 3) + 0x100) >> 1;
                u8 press_r = 51 >> 1;
                u8 press_g = (58  + ((press_lerp * (255 - 58))  >> 8)) >> 1;
                u8 press_b = (206 + ((press_lerp * (255 - 206)) >> 8)) >> 1;
                
                RECT press_src = {0, 112, 256, 32};
                Gfx_BlitTexCol(&menu.tex_title, &press_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 48, press_r, press_g, press_b);
            }
            else
            {
                //Flash white
                RECT press_src = {0, (animf_count & 1) ? 144 : 112, 256, 32};
                Gfx_BlitTex(&menu.tex_title, &press_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 48);
            }
            
            //Draw Title psychic
            menu.Titlepsy->tick(menu.Titlepsy);
            break;
        }
        case MenuPage_Debug:
        {
            static const struct
            {
                const char *text;
            } menu_options[] = {
                {"UNLOCK MIND GAMES DEMO"},
                {"UNLOCK FLOP GAMES"},
                {"WILD CARACAL"},
                {"PUMBA"},
            };
            if (stage.prefs.secretunlocked == true)
            {
            	lockpsyo = "PSYCHIC";
            	locklate = "LATE DRIVE";
            	iconlock = 2;
            	storylock = "1";
            	stage.awards.udemo =true;
            	if (menu.select == 0)
            	{
            	//Draw page label
            menu.font_arial.draw(&menu.font_arial,
                "Unlocked",
                255,
                217,
                FontAlign_Left
            );
            }
            }
            else
            {
            	lockpsyo = "UNKNOWN";
            	locklate = "UNKNOWN";
            	iconlock = 4;
            	storylock = "9";
            	if (menu.select == 0)
            	{
            	//Draw page label
            menu.font_arial.draw(&menu.font_arial,
                "Locked",
                255,
                217,
                FontAlign_Left
            );
            }
            }
            //Initialize page
            if (menu.page_swap)
            {
                menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
            }
            stage.awards.debugger = true;
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "DEBUG MENU",
                16,
                SCREEN_HEIGHT - 32,
                FontAlign_Left
            );
            
            //Handle option and selection
            if (menu.next_page == menu.page && Trans_Idle())
            {
                //Change option
                if (pad_state.press & PAD_UP)
                {
                    if (menu.select > 0)
                        menu.select--;
                    else
                        menu.select = COUNT_OF(menu_options) - 1;
                }
                if (pad_state.press & PAD_DOWN)
                {
                    if (menu.select < COUNT_OF(menu_options) - 1)
                        menu.select++;
                    else
                        menu.select = 0;
                }
                
                if (menu.select == 0)
                {
                	if (stage.prefs.secretunlocked == false)
                	{
                		if (pad_state.press & (PAD_START | PAD_CROSS))
		        	{
		        		stage.prefs.secretunlocked = true;
		        	}
                	}
                }
                
                //Return to main menu if circle is pressed
                if (pad_state.press & PAD_CIRCLE)
                {
                    menu.next_page = MenuPage_Main;
                    menu.next_select = 0;
                    menu.select = 0;
                    Trans_Start();
                }
            }
            
            //Draw box at the bottom
            RECT song_box = {250, 210, 60, 30};
            Gfx_BlendRect(&song_box, 32, 32, 32, 0);
            
            //Draw options
            s32 next_scroll = menu.select * FIXED_DEC(40,1);
            menu.scroll += (next_scroll - menu.scroll) >> 4;
            
            for (u8 i = 0; i < COUNT_OF(menu_options); i++)
            {
                //Get position on screen
                s32 y = (i * 40) - 8 - (menu.scroll >> FIXED_SHIFT);
                if (y <= -SCREEN_HEIGHT2 - 8)
                    continue;
                if (y >= SCREEN_HEIGHT2 + 8)
                    break;
                
                //Draw text
                menu.font_bold.draw_col(&menu.font_bold,
                    Menu_LowerIf(menu_options[i].text, NULL),
                    30 + (y >> 2),
                    SCREEN_HEIGHT2 + y - 8,
                    FontAlign_Left,
                    (menu.select == i) ? 128 : 64,
                    (menu.select == i) ? 128 : 64,
                    (menu.select == i) ? 128 : 64
                );
            }
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                49 >> 1, 49 >> 1, 49 >> 1,
                0, 0, 0
            );
            break;
        }
        case MenuPage_Main:
        {
            static const char *menu_options[] = {
                "STORY MODE",
                "FREEPLAY",
                "AWARDS",
                "CREDITS",
                "OPTIONS",
                "SAVE",
                #ifdef PSXF_NETWORK
                    "JOIN SERVER",
                    "HOST SERVER",
                #endif
            };
            
            //Initialize page
            if (menu.page_swap)
                menu.scroll = menu.select *
                #ifndef PSXF_NETWORK
                    FIXED_DEC(8,1);
                #else
                    FIXED_DEC(12,1);
                #endif
            if (stage.prefs.secretunlocked == true)
            {
            	lockpsyo = "PSYCHIC";
            	locklate = "LATE DRIVE";
            	iconlock = 2;
            	storylock = "1";
            	stage.awards.udemo =true;
            }
            else
            {
            	lockpsyo = "UNKNOWN";
            	locklate = "UNKNOWN";
            	iconlock = 4;
            	storylock = "9";
            }

		if(stage.awards.bweek ==true){wicon=11;}
		else{wicon=10;}

		if(stage.awards.mweek ==true){mwicon=12;}
		else{mwicon=10;}

		if(stage.awards.mpsy ==true){mpicon=13;}
		else{mpicon=10;}

		if(stage.awards.mwilt ==true){mwticon=14;}
		else{mwticon=10;}

		if(stage.awards.mup ==true){muicon=15;}
		else{muicon=10;}

		if(stage.awards.dsu ==true){duicon=16;}
		else{duicon=10;}

		if(stage.awards.udemo ==true){udicon=17;}
		else{udicon=10;}

		if(stage.awards.flopg ==true){fgicon=18;}
	 	else{fgicon=10;}
	 	
	 	if(stage.awards.debugger ==true){dbicon=19;}
	 	else{dbicon=10;}
	 	
	 	if(stage.prefs.lowquality ==1)
	 	{
	 		stage.awards.dsu = true;
	 	}
            
            //Draw version identification
            menu.font_arial.draw(&menu.font_arial,
                "Flop Engine v0.1",
                16,
                SCREEN_HEIGHT - 32,
                FontAlign_Left
            );
            
            //Handle option and selection
            if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
                Trans_Start();
            
            if (menu.next_page == menu.page && Trans_Idle())
            {
                //Change option
                if (pad_state.press & PAD_UP)
                {
                    if (menu.select > 0)
                        menu.select--;
                    else
                        menu.select = COUNT_OF(menu_options) - 1;
                }
                if (pad_state.press & PAD_DOWN)
                {
                    if (menu.select < COUNT_OF(menu_options) - 1)
                        menu.select++;
                    else
                        menu.select = 0;
                }
                
                //Select option if cross is pressed
                if (pad_state.press & (PAD_START | PAD_CROSS))
                {
                    switch (menu.select)
                    {
                        case 0: //Story Mode
                            menu.next_page = MenuPage_Story;
                            break;
                        case 1: //Freeplay
                            menu.next_page = MenuPage_Freeplay;
                            break;
                        case 2: //Mods
                            menu.next_page = MenuPage_Awards;
                            break;
                        case 3: //Credits
                            menu.next_page = MenuPage_Credits;
                            break;
                        case 4: //Options
                            menu.next_page = MenuPage_Options;
                            break;
                        case 5: //save game
                		Menu_DrawSavingicon( 284, 204);
                		WriteSaveDataStructToBinaryAndSaveItOnTheFuckingMemoryCard();
                	    break;
                    #ifdef PSXF_NETWORK
                        case 6: //Join Server
                            menu.next_page = Network_Inited() ? MenuPage_NetJoin : MenuPage_NetInitFail;
                            break;
                        case 7: //Host Server
                            menu.next_page = Network_Inited() ? MenuPage_NetHost : MenuPage_NetInitFail;
                            break;
                    #endif
                    }
                    if (menu.select !=5)
                    {
                    menu.next_select = 0;
                    menu.trans_time = FIXED_UNIT;
                    }
                }
                
                //Return to title screen if circle is pressed
                if (pad_state.press & PAD_CIRCLE)
                {
                    menu.next_page = MenuPage_Title;
                    Trans_Start();
                }
                if (pad_state.press & PAD_TRIANGLE)
                {
                    menu.next_page = MenuPage_Debug;
                    Trans_Start();
                }
            }
            
            //Draw options
            s32 next_scroll = menu.select *
            #ifndef PSXF_NETWORK
                FIXED_DEC(8,1);
            #else
                FIXED_DEC(12,1);
            #endif
            menu.scroll += (next_scroll - menu.scroll) >> 2;
            
            if (menu.next_page == menu.page || (menu.next_page == MenuPage_Title || menu.next_page == MenuPage_Debug))
            {
                //Draw all options
                for (u8 i = 0; i < COUNT_OF(menu_options); i++)
                {
                    menu.font_bold.draw(&menu.font_bold,
                        Menu_LowerIf(menu_options[i], menu.select != i),
                        SCREEN_WIDTH2,
                        SCREEN_HEIGHT2 + (i << 5) - 48 - (menu.scroll >> FIXED_SHIFT),
                        FontAlign_Center
                    );
                }
            }
            else if (animf_count & 2)
            {
                //Draw selected option
                menu.font_bold.draw(&menu.font_bold,
                    menu_options[menu.select],
                    SCREEN_WIDTH2,
                    SCREEN_HEIGHT2 + (menu.select << 5) - 48 - (menu.scroll >> FIXED_SHIFT),
                    FontAlign_Center
                );
            }
            
            //Draw background
            Menu_DrawBack(
                menu.next_page == menu.page || (menu.next_page == MenuPage_Title || menu.next_page == MenuPage_Debug),
            #ifndef PSXF_NETWORK
                menu.scroll >> (FIXED_SHIFT + 1),
            #else
                menu.scroll >> (FIXED_SHIFT + 3),
            #endif
                155 >> 1, 48 >> 1, 243 >> 1,
                253 >> 1, 231 >> 1, 113 >> 1
            );
            break;
        }
        case MenuPage_Story:
        {
            struct
            {
                const char *week;
                StageId stage;
                const char *name;
                const char *tracks[3];
                int bg;
            } menu_options[] = {
                {"1", StageId_1_1, "MIND GAMES", {"PSYCHIC", "WILTER", "UPROAR"}, 1},
                {storylock, StageId_2_1, "MIND GAMES", {"PSYCHIC", NULL, NULL}, 0},
            };
            
            //Initialize page
            if (menu.page_swap)
            {
                menu.scroll = 0;
                menu.page_param.stage.diff = StageDiff_Normal;
                menu.page_state.title.fade = FIXED_DEC(0,1);
                menu.page_state.title.fadespd = FIXED_DEC(0,1);
            }
            
            //Draw white fade
            if (menu.page_state.title.fade > 0)
            {
                static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
                Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
                menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
            }
            
            //Draw difficulty selector
            Menu_DifficultySelector(SCREEN_WIDTH - 55, 176);
            
            //Handle option and selection
            if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
                Trans_Start();
            
            if (menu.next_page == menu.page && Trans_Idle())
            {
                //Change option
                if (pad_state.press & PAD_UP)
                {
                    if (menu.select > 0)
                        menu.select--;
                    else
                        menu.select = COUNT_OF(menu_options) - 1;
                    CheckAndLoadWeek(menu.select);
                }
                if (pad_state.press & PAD_DOWN)
                {
                    if (menu.select < COUNT_OF(menu_options) - 1)
                        menu.select++;
                    else
                        menu.select = 0;
                    CheckAndLoadWeek(menu.select);
                }
                if  (menu_options[menu.select].week != "9")
                {
		        //Select option if cross is pressed
		        if (pad_state.press & (PAD_START | PAD_CROSS))
		        {
		            menu.next_page = MenuPage_Stage;
		            menu.page_param.stage.id = menu_options[menu.select].stage;
		            menu.page_param.stage.story = true;
		            menu.trans_time = FIXED_UNIT;
		            menu.page_state.title.fade = FIXED_DEC(255,1);
		            menu.page_state.title.fadespd = FIXED_DEC(510,1);
		            menu.mbf->set_anim(menu.mbf, CharAnim_Up);
		        }
                }
                else
                {
                	 //Select option if cross is pressed
		        if (pad_state.press & (PAD_START | PAD_CROSS))
		        {
		            	//Draw box at the bottom
            			RECT song_box = {90, 60, 160, 120};
            			Gfx_BlendRect(&song_box, 32, 32, 32, -50);
            			 //Draw page label
				menu.font_arial.draw(&menu.font_arial,
				"LOCKED",
					160,
					120,
					FontAlign_Center
				);
		         }
                }
                
                //Return to main menu if circle is pressed
                if (pad_state.press & PAD_CIRCLE)
                {
                    menu.next_page = MenuPage_Main;
                    menu.next_select = 0; //Story Mode
                    Trans_Start();
                }
            }
            
            //Draw week name and tracks
            menu.font_bold.draw(&menu.font_bold,
                menu_options[menu.select].name,
                SCREEN_WIDTH - 6,
                6,
                FontAlign_Right
            );
            
            const char * const *trackp = menu_options[menu.select].tracks;
            for (size_t i = 0; i < COUNT_OF(menu_options[menu.select].tracks); i++, trackp++)
            {
                if (*trackp != NULL)
                    menu.font_arialr.draw(&menu.font_arialr,
                        *trackp,
                        40,
                        SCREEN_HEIGHT - (4 * 14) + (i * 10),
                        FontAlign_Center
                    );
            }
            //Draw menu characters
            menu.mbf->tick(menu.mbf);
            
            //Draw menu characters
            menu.mgf->tick(menu.mgf);
            
            Menu_DrawBG( 0, 24);
            /*switch (menu_options[menu.select].bg)
            {
                case 0:
                {
                    //Draw upper strip
                    RECT name_bar = {0, 24, SCREEN_WIDTH, 136};
                    Gfx_DrawRect(&name_bar, 249, 207, 81);
                break;
                }
                case 1:
                {
                break;
                }
            }*/
            
            char weektext[30];
            sprintf(weektext, "\\MENU\\WEEK%d.TIM;1", menu.select);

            //Draw behind week name strip
            RECT coverup_bar = {0, 0, SCREEN_WIDTH, 24};
            Gfx_DrawRect(&coverup_bar, 0, 0, 0);
            
            //Draw options
            s32 next_scroll = menu.select * FIXED_DEC(42,1);
            menu.scroll += (next_scroll - menu.scroll) >> 3;
            
            Menu_DrawTrack( 0, 165);
            
            if (menu.next_page == menu.page || menu.next_page == MenuPage_Main)
            {
                //Draw all options
                for (u8 i = 0; i < COUNT_OF(menu_options); i++)
                {
                    s32 y = 161 + (i * 42) - (menu.scroll >> FIXED_SHIFT);
                    if (y <= 16)
                        continue;
                    if (y >= SCREEN_HEIGHT)
                        break;
                    Menu_DrawWeek(menu_options[i].week, SCREEN_WIDTH - 230, y);
                }
            }
            else if (animf_count & 2)
            {
                //Draw selected option
                Menu_DrawWeek(menu_options[menu.select].week, SCREEN_WIDTH - 230, 161 + (menu.select * 42) - (menu.scroll >> FIXED_SHIFT));
            }
            
            break;
        }
        case MenuPage_Freeplay:
        {
		    struct
		    {
		        StageId stage;
		        u32 col;
		        const char *text;
		        u8 icon;
		        int music;
		        boolean lock;
		    } menu_options[] = {
		        {StageId_1_1, 0x580093, "PSYCHIC", 2, 0, true},
		        {StageId_1_2, 0x580093, "WILTER", 2, 1, true},
		        {StageId_1_3, 0x580093, "UPROAR", 2, 2, true},
		        {StageId_2_1, 0xFF223344, lockpsyo, iconlock, 3, stage.prefs.secretunlocked},
		        {StageId_2_2, 0xFF223344, locklate, iconlock, 4, stage.prefs.secretunlocked},
		        {StageId_2_3, 0xFF223344, lockflop, 4, NULL, secretunlocked2},
		    };
            if (menu_options[menu.select].lock == true)
		    {
		    switch (menu_options[menu.select].music)
		    {
		        case 0:
		        {   
		            if (pad_state.press & PAD_R1)
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_Psychic, 0x40, 0, 1);
		                Audio_WaitPlayXA();
		            }
		            if (pad_state.press & PAD_L1)
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
		                Audio_WaitPlayXA();
		            }
		            break;
		        }
		        case 1:
		        {   
		            if (pad_state.press & PAD_R1)
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_Wilter, 0x40, 2, 1);
		                Audio_WaitPlayXA();
		            }
		            if (pad_state.press & PAD_L1)
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
		                Audio_WaitPlayXA();
		            }
		            break;
		        }
		        case 2:
		        {   
		            if (pad_state.press & PAD_R1)
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_Uproar, 0x40, 0, 1);
		                Audio_WaitPlayXA();
		            }
		            if (pad_state.press & PAD_L1)
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
		                Audio_WaitPlayXA();
		            }
		            break;
		        }
		        case 3:
		        {   
		            if (pad_state.press & PAD_R1)
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_Psychico, 0x40, 0, 1);
		                Audio_WaitPlayXA();
		            }
		            if (pad_state.press & PAD_L1)
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
		                Audio_WaitPlayXA();
		            }
		            break;
		        }
		        case 4:
		        {   
		            if (pad_state.press & PAD_R1)
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_Latedrive, 0x40, 2, 1);
		                Audio_WaitPlayXA();
		            }
		            if (pad_state.press & PAD_L1)
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
		                Audio_WaitPlayXA();
		            }
		            break;
		        }
		    }
            }
            //Initialize page
            if (menu.page_swap)
            {
                menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
                menu.page_param.stage.diff = StageDiff_Normal;
                menu.page_state.freeplay.back_r = FIXED_DEC(255,1);
                menu.page_state.freeplay.back_g = FIXED_DEC(255,1);
                menu.page_state.freeplay.back_b = FIXED_DEC(255,1);
            }
            if (menu_options[menu.select].lock == true)
            {
            //Draw page label
            menu.font_arial.draw(&menu.font_arial,
                "Press R1 to listen to song/Press L1 to stop song",
                16,
                217,
                FontAlign_Left
            );
            }
            else
            {
            	//Draw page label
            menu.font_arial.draw(&menu.font_arial,
                "A secret is required to unlock this song",
                16,
                217,
                FontAlign_Left
            );
            }
            
            //Draw difficulty selector
            Menu_DifficultySelector(SCREEN_WIDTH - 100, SCREEN_HEIGHT2 - 48);
            
            //Handle option and selection
            if (menu.next_page == menu.page && Trans_Idle())
            {
                //Change option
                if (pad_state.press & PAD_UP)
                {
                    if (menu.select > 0)
                        menu.select--;
                    else
                        menu.select = COUNT_OF(menu_options) - 1;
                }
                if (pad_state.press & PAD_DOWN)
                {
                    if (menu.select < COUNT_OF(menu_options) - 1)
                        menu.select++;
                    else
                        menu.select = 0;
                }
                if (menu_options[menu.select].lock == true)
                {
		        //Select option if cross is pressed
		        if (pad_state.press & (PAD_START | PAD_CROSS))
		        {
		            menu.next_page = MenuPage_Stage;
		            menu.page_param.stage.id = menu_options[menu.select].stage;
		            menu.page_param.stage.story = false;
		            Trans_Start();
		        }
                }
                else
                {
                	 //Select option if cross is pressed
		        if (pad_state.press & (PAD_START | PAD_CROSS))
		        {
		            	//Draw box at the bottom
            			RECT song_box = {90, 60, 160, 120};
            			Gfx_BlendRect(&song_box, 32, 32, 32, -50);
            			 //Draw page label
				menu.font_arial.draw(&menu.font_arial,
				"LOCKED",
					160,
					120,
					FontAlign_Center
				);
		         }
                }
                
                //Return to main menu if circle is pressed
                if (pad_state.press & PAD_CIRCLE)
                {
                    menu.next_page = MenuPage_Main;
                    menu.next_select = 1; //Freeplay
                    Trans_Start();
                }
            }
            
            //Draw box at the bottom
            RECT song_box = {0, 210, 320, 30};
            Gfx_BlendRect(&song_box, 32, 32, 32, 0);
            
            //Draw options
            s32 next_scroll = menu.select * FIXED_DEC(40,1);
            menu.scroll += (next_scroll - menu.scroll) >> 3;
            
            for (u8 i = 0; i < COUNT_OF(menu_options); i++)
            {
                //Get position on screen
                s32 y = (i * 40) - 8 - (menu.scroll >> FIXED_SHIFT);
                if (y <= -SCREEN_HEIGHT2 - 8)
                    continue;
                if (y >= SCREEN_HEIGHT2 + 8)
                    break;
                
                //Draw Icon
                Menu_DrawHealth(menu_options[i].icon, strlen(menu_options[i].text) * 15 + 32 + 8 + (y / 4) -18, SCREEN_HEIGHT2 + y - 17, menu.select == i);
                
                //Draw text
                menu.font_bold.draw_col(&menu.font_bold,
                    Menu_LowerIf(menu_options[i].text, NULL),
                    30 + (y >> 2),
                    SCREEN_HEIGHT2 + y - 8,
                    FontAlign_Left,
                    (menu.select == i) ? 128 : 64,
                    (menu.select == i) ? 128 : 64,
                    (menu.select == i) ? 128 : 64
                );
            }
            
            //Draw background
            fixed_t tgt_r = (fixed_t)((menu_options[menu.select].col >> 16) & 0xFF) << FIXED_SHIFT;
            fixed_t tgt_g = (fixed_t)((menu_options[menu.select].col >>  8) & 0xFF) << FIXED_SHIFT;
            fixed_t tgt_b = (fixed_t)((menu_options[menu.select].col >>  0) & 0xFF) << FIXED_SHIFT;
            
            menu.page_state.freeplay.back_r += (tgt_r - menu.page_state.freeplay.back_r) >> 4;
            menu.page_state.freeplay.back_g += (tgt_g - menu.page_state.freeplay.back_g) >> 4;
            menu.page_state.freeplay.back_b += (tgt_b - menu.page_state.freeplay.back_b) >> 4;
            
            Menu_DrawBack(
                true,
                8,
                menu.page_state.freeplay.back_r >> (FIXED_SHIFT + 1),
                menu.page_state.freeplay.back_g >> (FIXED_SHIFT + 1),
                menu.page_state.freeplay.back_b >> (FIXED_SHIFT + 1),
                0, 0, 0
            );
            break;
        }
        case MenuPage_Awards:
        {
            struct
            {
            	u8 icon;
                u8 achievement;
            } menu_options[] = {
                {wicon, 0},
                {mwicon, 1},
                {mpicon, 2},
                {mwticon, 3},
                {muicon, 4},
                {duicon, 5},
                {udicon, 6},
                {fgicon, 7},
                {dbicon, 8},
            };
            
            switch (menu_options[menu.select].achievement)
		    {
		    	case 0:
		    	{
		    	//Draw page label
			    menu.font_arial.draw(&menu.font_arial,
				"Beat the mind games full week",
				16,
				217,
				FontAlign_Left
			    );
			    
			    break;
            		}
            		case 1:
		    	{
		    	//Draw page label
			    menu.font_arial.draw(&menu.font_arial,
				"No miss the mind games full week",
				16,
				217,
				FontAlign_Left
			    );
			    break;
            		}
            		case 2:
		    	{
		    	//Draw page label
			    menu.font_arial.draw(&menu.font_arial,
				"No miss psychic",
				16,
				217,
				FontAlign_Left
			    );
			    break;
            		}
            		case 3:
		    	{
		    	//Draw page label
			    menu.font_arial.draw(&menu.font_arial,
				"No miss wilter",
				16,
				217,
				FontAlign_Left
			    );
			    break;
            		}
            		case 4:
		    	{
		    	//Draw page label
			    menu.font_arial.draw(&menu.font_arial,
				"No miss uproar",
				16,
				217,
				FontAlign_Left
			    );
			    break;
            		}
            		case 5:
		    	{
		    	//Draw page label
			    menu.font_arial.draw(&menu.font_arial,
				"3ds user? turn on low quality mode",
				16,
				217,
				FontAlign_Left
			    );
			    break;
            		}
            		case 6:
		    	{
		    	//Draw page label
			    menu.font_arial.draw(&menu.font_arial,
				"Unlock the mind games demo",
				16,
				217,
				FontAlign_Left
			    );
			    break;
            		}
            		case 7:
		    	{
		    	//Draw page label
			    menu.font_arial.draw(&menu.font_arial,
				"??????",
				16,
				217,
				FontAlign_Left
			    );
			    break;
            		}
            		case 8:
		    	{
		    	//Draw page label
			    menu.font_arial.draw(&menu.font_arial,
				"Debugger enter the debug menu",
				16,
				217,
				FontAlign_Left
			    );
			    break;
            		}
		    }
            //Initialize page
            if (menu.page_swap)
            {
                menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
                menu.page_param.stage.diff = StageDiff_Normal;
            }
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "AWARDS",
                16,
                SCREEN_HEIGHT - 48,
                FontAlign_Left
            );
            
            //Handle option and selection
            if (menu.next_page == menu.page && Trans_Idle())
            {
                //Change option
                if (pad_state.press & PAD_UP)
                {
                    if (menu.select > 0)
                        menu.select--;
                    else
                        menu.select = COUNT_OF(menu_options) - 1;
                }
                if (pad_state.press & PAD_DOWN)
                {
                    if (menu.select < COUNT_OF(menu_options) - 1)
                        menu.select++;
                    else
                        menu.select = 0;
                }
                
                //Return to main menu if circle is pressed
                if (pad_state.press & PAD_CIRCLE)
                {
                    menu.next_page = MenuPage_Main;
                    menu.next_select = 2; //Mods
                    menu.select = 0;
                    Trans_Start();
                }
            }
            
            //Draw box at the bottom
            RECT song_box = {0, 210, 320, 30};
            Gfx_BlendRect(&song_box, 32, 32, 32, 0);
            
            //Draw options
            s32 next_scroll = menu.select * FIXED_DEC(40,1);
            menu.scroll += (next_scroll - menu.scroll) >> 3;
            
            for (u8 i = 0; i < COUNT_OF(menu_options); i++)
            {
                //Get position on screen
                s32 y = (i * 40) - 8 - (menu.scroll >> FIXED_SHIFT);
                if (y <= -SCREEN_HEIGHT2 - 8)
                    continue;
                if (y >= SCREEN_HEIGHT2 + 8)
                    break;
                    
                //Draw Icon
               	Menu_DrawHealth(menu_options[i].icon, 48 + (y / 4) -18, SCREEN_HEIGHT2 + y - 17, menu.select == i);
            }
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                197 >> 1, 240 >> 1, 95 >> 1,
                0, 0, 0
            );
            break;
        }
        case MenuPage_Credits:
        {
            static const struct
            {
                const char *bold;
            } menu_headers[] = {
            	{"mind games crew"},
            	{" "},
            	{" "},
            	{" "},
            	{" "},
            	{" "},
            	{" "},
            	{" "},
            	{"special thanks"},
            	{" "},
            	{" "},
            	{" "},
                {"psx mind games"},
                {" "},
                {" "},
                {" "},
                {"flop engine"},
                {" "},
                {" "},
                {"psxfunkin"},
                {" "},
                {" "},
            };
            
            static const struct
            {
                const char *text;
                int about;
                u8 icon;
                u32 col; 
            } menu_options[] = {
            	{"RiverOaken", 1, 25, 0xFFb2007c},
            	{"Shadow Mario", 2, 26, 0xFF403f40},
            	{"iFlicky", 3, 27, 0xFFb844cb},
            	{"Leomming", 4, 28, 0xFFeb8077},
            	{"PJsVoiceArts", 5, 29, 0xFFea384b},
            	{"Iymph", 6, 30, 0xFF5f5e5f},
            	{" ", 0, 6, 0xFF9b30f3},
            	{" ", 0, 6, 0xFF9b30f3},
            	{"Salvati", 7, 31, 0xFF32ebed},
            	{"slabraccoon", 8, 32, 0xFFaf4c22},
            	{" ", 0, 6, 0xFF9b30f3},
            	{" ", 0, 6, 0xFF9b30f3},
                {"nintendo bro", 9, 20, 0xFF51ffb3},
                {"biliousdata", 10, 22, 0xFF1b16af},
                {" ", 0, 6, 0xFF9b30f3},
                {" ", 0, 6, 0xFF9b30f3},
                {"nintendo bro", 9, 20, 0xFF51ffb3},
                {" ", 0, 6, 0xFF9b30f3},
                {" ", 0, 6, 0xFF9b30f3},
                {"ckdev" , 11, 21,0xFFc5f05f},
            };
            switch (menu_options[menu.select].about)
            {
                case 0:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        " ",
                        256,
                        126,
                        FontAlign_Left
                    );
                    break;
                }
                case 1:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "RiverOaken", 
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Project Lead",
                        256,
                        110,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "& Animator",
                        256,
                        122,
                        FontAlign_Center
                    );
                    break;
                }
                case 2:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "Shadow Mario", 
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Main Programmer",
                        256,
                        110,
                        FontAlign_Center
                    );
                    break;
                }
                case 3:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "iFlicky", 
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Main Composer",
                        256,
                        110,
                        FontAlign_Center
                    );
                    break;
                }
                case 4:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "Leomming", 
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Additional Arts",
                        256,
                        110,
                        FontAlign_Center
                    );
                    break;
                }
                case 5:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "PJsVoiceArts", 
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Psychic and Spirit's",
                        256,
                        110,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Voice Actor",
                        256,
                        122,
                        FontAlign_Center
                    );
                    break;
                }
                case 6:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "Iymph", 
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Uproar's Charter",
                        256,
                        110,
                        FontAlign_Center
                    );
                    break;
                }
                case 7:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "Salvati", 
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Psychic's Soundfont",
                        256,
                        110,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Creator",
                        256,
                        122,
                        FontAlign_Center
                    );
                    break;
                }
                case 8:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "slabraccoon", 
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Emotional Support",
                        256,
                        110,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Raccoon",
                        256,
                        122,
                        FontAlign_Center
                    );
                    break;
                }
                case 9:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "Nintendo Bro385", 
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Flop Engine",
                        256,
                        110,
                        FontAlign_Center
                    );
                    //Draw nintendobro about pic
                    Menu_DrawNB( 224, 16);
                    break;
                }
                case 10:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "BiliousData", 
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "Originally worked on",
                        256,
                        110,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "mind games port",
                        256,
                        122,
                        FontAlign_Center
                    );
                    //Draw biliousdata about pic
                    Menu_DrawBD( 224, 16);
                    break;
                }
                case 11:
                {
                    //Draw text
                    menu.font_arial.draw(&menu.font_arial,
                        "CKDEV",
                        256,
                        98,
                        FontAlign_Center
                    );
                    menu.font_arial.draw(&menu.font_arial,
                        "PSXFunkin Creator",
                        256,
                        110,
                        FontAlign_Center
                    );
                    //Draw ckdev about pic
                    Menu_DrawCKD( 224, 16);
                    break;
                }
                    
            }
            //Initialize page
            if (menu.page_swap)
            {
                menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
                menu.page_param.stage.diff = StageDiff_Normal;
                menu.page_state.freeplay.back_r = FIXED_DEC(255,1);
                menu.page_state.freeplay.back_g = FIXED_DEC(255,1);
                menu.page_state.freeplay.back_b = FIXED_DEC(255,1);
            }
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "Credits",
                16,
                SCREEN_HEIGHT - 32,
                FontAlign_Left
            );
            
            //Handle option and selection
            if (menu.next_page == menu.page && Trans_Idle())
            {
                //Change option
                if (pad_state.press & PAD_UP)
                {
                    if (menu.select > 0)
                        menu.select--;
                    else
                        menu.select = COUNT_OF(menu_options) - 1;
                }
                if (pad_state.press & PAD_DOWN)
                {
                    if (menu.select < COUNT_OF(menu_options) - 1)
                        menu.select++;
                    else
                        menu.select = 0;
                }
                
                //Return to main menu if circle is pressed
                if (pad_state.press & PAD_CIRCLE)
                {
                    menu.next_page = MenuPage_Main;
                    menu.next_select = 3; //Mods
                    menu.select = 0;
                    Trans_Start();
                }
            }
            
            //Draw options
            s32 next_scroll = menu.select * FIXED_DEC(24,1);
            menu.scroll += (next_scroll - menu.scroll) >> 4;
            
            for (u8 i = 0; i < COUNT_OF(menu_options); i++)
            {
                //Get position on screen
                s32 y = (i * 24) - 6 - (menu.scroll >> FIXED_SHIFT);
                if (y <= -SCREEN_HEIGHT2 - 6)
                    continue;
                if (y >= SCREEN_HEIGHT2 + 6)
                    break;
                //Draw Icon
                Menu_DrawHealth(menu_options[i].icon, strlen(menu_options[i].text) * 7 + 32 + 35, SCREEN_HEIGHT2 + y - 15, menu.select == i);
                
                //Draw headers
                menu.font_bold.draw(&menu.font_bold,
                    Menu_LowerIf(menu_headers[i].bold, NULL),
                    10,
                    SCREEN_HEIGHT2 + y - 8 - 24,
                    FontAlign_Left
                );
                
                //Draw text
                menu.font_arialb.draw(&menu.font_arialb,
                    Menu_LowerIf(menu_options[i].text, menu.select != i),
                    60,
                    SCREEN_HEIGHT2 + y - 6,
                    FontAlign_Left
                );
                
            }
            
            //Draw box for about credits
            RECT about_box = {192, 0, 128, 240};
            Gfx_BlendRect(&about_box, 62, 62, 62, 0);
            
            //Draw background
            fixed_t tgt_r = (fixed_t)((menu_options[menu.select].col >> 16) & 0xFF) << FIXED_SHIFT;
            fixed_t tgt_g = (fixed_t)((menu_options[menu.select].col >>  8) & 0xFF) << FIXED_SHIFT;
            fixed_t tgt_b = (fixed_t)((menu_options[menu.select].col >>  0) & 0xFF) << FIXED_SHIFT;
            
            menu.page_state.freeplay.back_r += (tgt_r - menu.page_state.freeplay.back_r) >> 4;
            menu.page_state.freeplay.back_g += (tgt_g - menu.page_state.freeplay.back_g) >> 4;
            menu.page_state.freeplay.back_b += (tgt_b - menu.page_state.freeplay.back_b) >> 4;
            
            Menu_DrawBack(
                true,
                8,
                menu.page_state.freeplay.back_r >> (FIXED_SHIFT + 1),
                menu.page_state.freeplay.back_g >> (FIXED_SHIFT + 1),
                menu.page_state.freeplay.back_b >> (FIXED_SHIFT + 1),
                0, 0, 0
            );
            break;
        }
        case MenuPage_Options:
        {
            static const char *gamemode_strs[] = {"NORMAL", "SWAP", "TWO PLAYER"};
            static const struct
            {
                enum
                {
                    OptType_Boolean,
                    OptType_Enum,
                } type;
                const char *text;
                void *value;
                union
                {
                    struct
                    {
                        int a;
                    } spec_boolean;
                    struct
                    {
                        s32 max;
                        const char **strs;
                    } spec_enum;
                } spec;
            } menu_options[] = {
                {OptType_Enum,    "GAMEMODE", &stage.mode, {.spec_enum = {COUNT_OF(gamemode_strs), gamemode_strs}}},
                //{OptType_Boolean, "INTERPOLATION", &stage.expsync},
                {OptType_Boolean, "GHOST TAP ", &stage.prefs.ghost, {.spec_boolean = {0}}},
                {OptType_Boolean, "DOWNSCROLL", &stage.prefs.downscroll, {.spec_boolean = {0}}},
                {OptType_Boolean, "BOTPLAY", &stage.prefs.botplay, {.spec_boolean = {0}}},
                {OptType_Boolean, "LOW QUALITY", &stage.prefs.lowquality, {.spec_boolean = {0}}},
            };
            
            //Initialize page
            if (menu.page_swap)
                menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "OPTIONS",
                16,
                SCREEN_HEIGHT - 48,
                FontAlign_Left
            );
            //Draw page label
            menu.font_arial.draw(&menu.font_arial,
                "Press Triangle for Music Options",
                1,
                217,
                FontAlign_Left
            );
            
            //Handle option and selection
            if (menu.next_page == menu.page && Trans_Idle())
            {
                //Change option
                if (pad_state.press & PAD_UP)
                {
                    if (menu.select > 0)
                        menu.select--;
                    else
                        menu.select = COUNT_OF(menu_options) - 1;
                }
                if (pad_state.press & PAD_DOWN)
                {
                    if (menu.select < COUNT_OF(menu_options) - 1)
                        menu.select++;
                    else
                        menu.select = 0;
                }
                
                //Handle option changing
                switch (menu_options[menu.select].type)
                {
                    case OptType_Boolean:
                        if (pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
                            *((boolean*)menu_options[menu.select].value) ^= 1;
                        break;
                    case OptType_Enum:
                        if (pad_state.press & PAD_LEFT)
                            if (--*((s32*)menu_options[menu.select].value) < 0)
                                *((s32*)menu_options[menu.select].value) = menu_options[menu.select].spec.spec_enum.max - 1;
                        if (pad_state.press & PAD_RIGHT)
                            if (++*((s32*)menu_options[menu.select].value) >= menu_options[menu.select].spec.spec_enum.max)
                                *((s32*)menu_options[menu.select].value) = 0;
                        break;
                }
                
                //Return to main menu if circle is pressed
                if (pad_state.press & PAD_TRIANGLE)
                {
                    menu.next_page = MenuPage_Music;
                    menu.next_select = 0; //music
                    Trans_Start();
                }
                //Return to main menu if circle is pressed
                if (pad_state.press & PAD_CIRCLE)
                {	
                    menu.next_page = MenuPage_Main;
                    menu.next_select = 4; //Options
                    Trans_Start();
                }
            }
            
             //Draw box at the bottom
            RECT song_box = {0, 210, 320, 30};
            Gfx_BlendRect(&song_box, 32, 32, 32, 0);
            
            //Draw options
            s32 next_scroll = menu.select * FIXED_DEC(24,1);
            menu.scroll += (next_scroll - menu.scroll) >> 4;
            
            for (u8 i = 0; i < COUNT_OF(menu_options); i++)
            {
                //Get position on screen
                s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
                if (y <= -SCREEN_HEIGHT2 - 8)
                    continue;
                if (y >= SCREEN_HEIGHT2 + 8)
                    break;
                
                //Draw text
                char text[0x80];
                switch (menu_options[i].type)
                {
                    case OptType_Boolean:
                        sprintf(text, "%s %s", menu_options[i].text, *((boolean*)menu_options[i].value) ? "ON" : "OFF");
                        break;
                    case OptType_Enum:
                        sprintf(text, "%s %s", menu_options[i].text, menu_options[i].spec.spec_enum.strs[*((s32*)menu_options[i].value)]);
                        break;
                }
                menu.font_bold.draw(&menu.font_bold,
                    Menu_LowerIf(text, menu.select != i),
                    48 + (y >> 2),
                    SCREEN_HEIGHT2 + y - 8,
                    FontAlign_Left
                );
            }
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                253 >> 1, 113 >> 1, 155 >> 1,
                0, 0, 0
            );
            break;
        }
        case MenuPage_Music:
        {
            static const struct
            {
                u32 col;
		const char *text;
		int music;
            } menu_options[] = {
                {0xFF9b30f3, "GETTIN FREAKY", 0},
                {0xFFfde771, "OG GETTIN FREAKY", 1},
                {0xFFfd719b, "LUDUM", 2},
            };
            switch (menu_options[menu.select].music)
		    {
		        case 0:
		        {   
		            if (pad_state.press & (PAD_START | PAD_CROSS))
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
		                Audio_WaitPlayXA();
		                stage.prefs.menumusic=0;
		                /*menu.next_page = MenuPage_Opening;
				    menu.next_select = 0; //Mods
				    Trans_Start();*/
		            }
		            break;
		        }
		        case 1:
		        {   
		            if (pad_state.press & (PAD_START | PAD_CROSS))
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_Freeky, 0x40, 2, 1);
		                Audio_WaitPlayXA();
		                stage.prefs.menumusic=1;
		                /*menu.next_page = MenuPage_Opening;
				    menu.next_select = 0; //Mods
				    Trans_Start();*/
		            }
		            break;
		        }
		        case 2:
		        {   
		            if (pad_state.press & (PAD_START | PAD_CROSS))
		            {
		                //Play Tutorial Music
		                Audio_StopXA();
		                Audio_PlayXA_Track(XA_Ludum, 0x40, 3, 1);
		                Audio_WaitPlayXA();
		                stage.prefs.menumusic=2;
		                /*menu.next_page = MenuPage_Opening;
				    menu.next_select = 0; //Mods
				    Trans_Start();*/
		            }
		            break;
		        }
		    }
            //Initialize page
            if (menu.page_swap)
            {
                menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
                menu.page_param.stage.diff = StageDiff_Normal;
            }
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "MUSIC",
                16,
                SCREEN_HEIGHT - 32,
                FontAlign_Left
            );
            //Handle option and selection
            if (menu.next_page == menu.page && Trans_Idle())
            {
                //Change option
                if (pad_state.press & PAD_UP)
                {
                    if (menu.select > 0)
                        menu.select--;
                    else
                        menu.select = COUNT_OF(menu_options) - 1;
                }
                if (pad_state.press & PAD_DOWN)
                {
                    if (menu.select < COUNT_OF(menu_options) - 1)
                        menu.select++;
                    else
                        menu.select = 0;
                }
                
                //Return to main menu if circle is pressed
                if (pad_state.press & PAD_CIRCLE)
                {
                    menu.next_page = MenuPage_Options;
                    menu.next_select = 0; //Mods
                    Trans_Start();
                }
            }
            
            //Draw options
            s32 next_scroll = menu.select * FIXED_DEC(24,1);
            menu.scroll += (next_scroll - menu.scroll) >> 4;
            
            for (u8 i = 0; i < COUNT_OF(menu_options); i++)
            {
                //Get position on screen
                s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
                if (y <= -SCREEN_HEIGHT2 - 8)
                    continue;
                if (y >= SCREEN_HEIGHT2 + 8)
                    break;
                
                //Draw text
                menu.font_bold.draw(&menu.font_bold,
                    Menu_LowerIf(menu_options[i].text, menu.select != i),
                    48 + (y >> 2),
                    SCREEN_HEIGHT2 + y - 8,
                    FontAlign_Left
                );
            }
            
            //Draw background
            fixed_t tgt_r = (fixed_t)((menu_options[menu.select].col >> 16) & 0xFF) << FIXED_SHIFT;
            fixed_t tgt_g = (fixed_t)((menu_options[menu.select].col >>  8) & 0xFF) << FIXED_SHIFT;
            fixed_t tgt_b = (fixed_t)((menu_options[menu.select].col >>  0) & 0xFF) << FIXED_SHIFT;
            
            menu.page_state.freeplay.back_r += (tgt_r - menu.page_state.freeplay.back_r) >> 4;
            menu.page_state.freeplay.back_g += (tgt_g - menu.page_state.freeplay.back_g) >> 4;
            menu.page_state.freeplay.back_b += (tgt_b - menu.page_state.freeplay.back_b) >> 4;
            
            Menu_DrawBack(
                true,
                8,
                menu.page_state.freeplay.back_r >> (FIXED_SHIFT + 1),
                menu.page_state.freeplay.back_g >> (FIXED_SHIFT + 1),
                menu.page_state.freeplay.back_b >> (FIXED_SHIFT + 1),
                0, 0, 0
            );
            break;
        }
    #ifdef PSXF_NETWORK
        case MenuPage_NetHost:
        {
            const size_t menu_options = 3;
            
            //Initialize page
            if (menu.page_swap)
            {
                menu.page_state.net_host.type = false;
                menu.page_state.net_host.port[0] = '\0';
                menu.page_state.net_host.pass[0] = '\0';
            }
            
            //Handle option and selection
            if (menu.next_page == menu.page && Trans_Idle())
            {
                if (!menu.page_state.net_host.type)
                {
                    //Change option
                    if (pad_state.press & PAD_UP)
                    {
                        if (menu.select > 0)
                            menu.select--;
                        else
                            menu.select = menu_options - 1;
                    }
                    if (pad_state.press & PAD_DOWN)
                    {
                        if (menu.select < menu_options - 1)
                            menu.select++;
                        else
                            menu.select = 0;
                    }
                    
                    //Handle selection when cross is pressed
                    if (pad_state.press & (PAD_START | PAD_CROSS))
                    {
                        switch (menu.select)
                        {
                            case 0: //Port
                            case 1: //Pass
                                menu.page_state.net_host.type = true;
                                break;
                            case 2: //Host
                                if (!Network_HostPort(menu.page_state.net_host.port, menu.page_state.net_host.pass))
                                {
                                    menu.next_page = MenuPage_NetOpWait;
                                    menu.next_select = 0;
                                    Trans_Start();
                                }
                                break;
                        }
                    }
                    
                    //Return to main menu if circle is pressed
                    if (pad_state.press & PAD_CIRCLE)
                    {
                        menu.next_page = MenuPage_Main;
                        menu.next_select = 5; //Host Server
                        Trans_Start();
                    }
                }
                else
                {
                    //Stop typing when start is pressed
                    if (pad_state.press & PAD_START)
                    {
                        switch (menu.select)
                        {
                            case 0: //Port
                            case 1: //Pass
                                menu.page_state.net_host.type = false;
                                break;
                        }
                    }
                }
            }
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "HOST SERVER",
                16,
                SCREEN_HEIGHT - 32,
                FontAlign_Left
            );
            
            //Draw options
            MenuStr_Process(&menu.page_state.net_host.port, 64 + 3 * 0, 64 + 16 * 0, "Port: %s", menu.select == 0, menu.page_state.net_host.type);
            MenuStr_Process(&menu.page_state.net_host.pass, 64 + 3 * 1, 64 + 16 * 1, "Pass: %s", menu.select == 1, menu.page_state.net_host.type);
            menu.font_bold.draw(&menu.font_bold, Menu_LowerIf("HOST", menu.select != 2), 64 + 3 * 2, 64 + 16 * 2, FontAlign_Left);
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                146 >> 1, 113 >> 1, 253 >> 1,
                0, 0, 0
            );
            break;
        }
        case MenuPage_NetJoin:
        {
            const size_t menu_options = 4;
            
            //Initialize page
            if (menu.page_swap)
            {
                menu.page_state.net_join.type = false;
                menu.page_state.net_join.ip[0] = '\0';
                menu.page_state.net_join.port[0] = '\0';
                menu.page_state.net_join.pass[0] = '\0';
            }
            
            //Handle option and selection
            if (menu.next_page == menu.page && Trans_Idle())
            {
                if (!menu.page_state.net_join.type)
                {
                    //Change option
                    if (pad_state.press & PAD_UP)
                    {
                        if (menu.select > 0)
                            menu.select--;
                        else
                            menu.select = menu_options - 1;
                    }
                    if (pad_state.press & PAD_DOWN)
                    {
                        if (menu.select < menu_options - 1)
                            menu.select++;
                        else
                            menu.select = 0;
                    }
                    
                    //Handle selection when cross is pressed
                    if (pad_state.press & (PAD_START | PAD_CROSS))
                    {
                        switch (menu.select)
                        {
                            case 0: //Ip
                            case 1: //Port
                            case 2: //Pass
                                menu.page_state.net_join.type = true;
                                break;
                            case 3: //Join
                                if (!Network_Join(menu.page_state.net_join.ip, menu.page_state.net_join.port, menu.page_state.net_join.pass))
                                {
                                    menu.next_page = MenuPage_NetConnect;
                                    menu.next_select = 0;
                                    Trans_Start();
                                }
                                break;
                        }
                    }
                    
                    //Return to main menu if circle is pressed
                    if (pad_state.press & PAD_CIRCLE)
                    {
                        menu.next_page = MenuPage_Main;
                        menu.next_select = 4; //Join Server
                        Trans_Start();
                    }
                }
                else
                {
                    //Stop typing when start is pressed
                    if (pad_state.press & PAD_START)
                    {
                        switch (menu.select)
                        {
                            case 0: //Join
                            case 1: //Port
                            case 2: //Pass
                                menu.page_state.net_join.type = false;
                                break;
                        }
                    }
                }
            }
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "JOIN SERVER",
                16,
                SCREEN_HEIGHT - 32,
                FontAlign_Left
            );
            
            //Draw options
            MenuStr_Process(&menu.page_state.net_join.ip, 64 + 3 * 0, 64 + 16 * 0, "Address: %s", menu.select == 0, menu.page_state.net_join.type);
            MenuStr_Process(&menu.page_state.net_join.port, 64 + 3 * 1, 64 + 16 * 1, "Port: %s", menu.select == 1, menu.page_state.net_join.type);
            MenuStr_Process(&menu.page_state.net_join.pass, 64 + 3 * 2, 64 + 16 * 2, "Pass: %s", menu.select == 2, menu.page_state.net_join.type);
            menu.font_bold.draw(&menu.font_bold, Menu_LowerIf("JOIN", menu.select != 3), 64 + 3 * 3, 64 + 16 * 3, FontAlign_Left);
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                146 >> 1, 113 >> 1, 253 >> 1,
                0, 0, 0
            );
            break;
        }
        case MenuPage_NetConnect:
        {
            //Change state according to network state
            if (menu.next_page == menu.page && Trans_Idle())
            {
                if (!Network_Connected())
                {
                    //Disconnected
                    menu.next_page = MenuPage_NetFail;
                    menu.next_select = 0;
                    Trans_Start();
                }
                else if (Network_Allowed())
                {
                    //Allowed to join
                    menu.next_page = MenuPage_NetLobby;
                    menu.next_select = 0;
                    Trans_Start();
                }
            }
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "CONNECTING",
                SCREEN_WIDTH2,
                SCREEN_HEIGHT2 - 8,
                FontAlign_Center
            );
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                113 >> 1, 146 >> 1, 253 >> 1,
                0, 0, 0
            );
            break;
        }
        case MenuPage_NetOpWait:
        {
            //Change state according to network state
            if (menu.next_page == menu.page && Trans_Idle())
            {
                if (!Network_Connected())
                {
                    //Disconnected
                    menu.next_page = MenuPage_NetFail;
                    menu.next_select = 0;
                    Trans_Start();
                }
                else if (Network_HasPeer())
                {
                    //Peer has joined
                    menu.next_page = MenuPage_NetOp;
                    menu.next_select = 0;
                    Trans_Start();
                }
            }
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "WAITING FOR PEER",
                SCREEN_WIDTH2,
                SCREEN_HEIGHT2 - 8,
                FontAlign_Center
            );
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                113 >> 1, 146 >> 1, 253 >> 1,
                0, 0, 0
            );
            break;
        }
        case MenuPage_NetOp:
        {
            static const struct
            {
                boolean diff;
                StageId stage;
                const char *text;
            } menu_options[] = {
                //{StageId_4_4, "TEST"},
                {true,  StageId_1_4, "TUTORIAL"},
                {true,  StageId_1_1, "BOPEEBO"},
                {true,  StageId_1_2, "FRESH"},
                {true,  StageId_1_3, "DADBATTLE"},
                {true,  StageId_2_1, "SPOOKEEZ"},
                {true,  StageId_2_2, "SOUTH"},
                {true,  StageId_2_3, "MONSTER"},
                {true,  StageId_3_1, "PICO"},
                {true,  StageId_3_2, "PHILLY NICE"},
                {true,  StageId_3_3, "BLAMMED"},
                {true,  StageId_4_1, "SATIN PANTIES"},
                {true,  StageId_4_2, "HIGH"},
                {true,  StageId_4_3, "MILF"},
                {true,  StageId_5_1, "COCOA"},
                {true,  StageId_5_2, "EGGNOG"},
                {true,  StageId_5_3, "WINTER HORRORLAND"},
                {true,  StageId_6_1, "SENPAI"},
                {true,  StageId_6_2, "ROSES"},
                {true,  StageId_6_3, "THORNS"},
                {true,  StageId_7_1, "UGH"},
                {true,  StageId_7_2, "GUNS"},
                {true,  StageId_7_3, "STRESS"},
                {false, StageId_Kapi_1, "WOCKY"},
                {false, StageId_Kapi_2, "BEATHOVEN"},
                {false, StageId_Kapi_3, "HAIRBALL"},
                {false, StageId_Kapi_4, "NYAW"},
                {true,  StageId_Clwn_1, "IMPROBABLE OUTSET"},
                {true,  StageId_Clwn_2, "MADNESS"},
                {true,  StageId_Clwn_3, "HELLCLOWN"},
                {false, StageId_Clwn_4, "EXPURGATION"},
                {false, StageId_2_4, "CLUCKED"},
            };
            
            //Initialize page
            if (menu.page_swap)
            {
                menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
                menu.page_param.stage.diff = StageDiff_Normal;
                menu.page_state.net_op.swap = false;
            }
            
            //Handle option and selection
            if (menu.next_page == menu.page && Trans_Idle())
            {
                //Check network state
                if (!Network_Connected())
                {
                    //Disconnected
                    menu.next_page = MenuPage_NetFail;
                    menu.next_select = 0;
                    Trans_Start();
                }
                else if (!Network_HasPeer())
                {
                    //Peer disconnected
                    menu.next_page = MenuPage_NetOpWait;
                    menu.next_select = 0;
                    Trans_Start();
                }
                
                //Change option
                if (pad_state.press & PAD_UP)
                {
                    if (menu.select > 0)
                        menu.select--;
                    else
                        menu.select = COUNT_OF(menu_options) - 1;
                }
                if (pad_state.press & PAD_DOWN)
                {
                    if (menu.select < COUNT_OF(menu_options) - 1)
                        menu.select++;
                    else
                        menu.select = 0;
                }
                
                //Select option if cross is pressed
                if (pad_state.press & (PAD_START | PAD_CROSS))
                {
                    //Load stage
                    Network_SetReady(false);
                    stage.mode = menu.page_state.net_op.swap ? StageMode_Net2 : StageMode_Net1;
                    menu.next_page = MenuPage_Stage;
                    menu.page_param.stage.id = menu_options[menu.select].stage;
                    if (!menu_options[menu.select].diff)
                        menu.page_param.stage.diff = StageDiff_Hard;
                    menu.page_param.stage.story = false;
                    Trans_Start();
                }
                
                //Swap characters if triangle is pressed
                if (pad_state.press & PAD_TRIANGLE)
                    menu.page_state.net_op.swap ^= true;
            }
            
            //Draw op controls
            const char *control_txt;
            
            control_txt = menu.page_state.net_op.swap ? "You will be Player 2. Press Triangle to swap." : "You will be Player 1. Press Triangle to swap.";
            menu.font_arial.draw_col(&menu.font_arial, control_txt, 24, SCREEN_HEIGHT - 24 - 12, FontAlign_Left, 0x80, 0x80, 0x80);
            menu.font_arial.draw_col(&menu.font_arial, control_txt, 24 + 1, SCREEN_HEIGHT - 24 - 12 + 1, FontAlign_Left, 0x00, 0x00, 0x00);
            
            //Draw difficulty selector
            if (menu_options[menu.select].diff)
                Menu_DifficultySelector(SCREEN_WIDTH - 100, SCREEN_HEIGHT2 - 48);
            
            //Draw options
            s32 next_scroll = menu.select * FIXED_DEC(24,1);
            menu.scroll += (next_scroll - menu.scroll) >> 4;
            
            for (u8 i = 0; i < COUNT_OF(menu_options); i++)
            {
                //Get position on screen
                s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
                if (y <= -SCREEN_HEIGHT2 - 8)
                    continue;
                if (y >= SCREEN_HEIGHT2 + 8)
                    break;
                
                //Draw text
                menu.font_bold.draw(&menu.font_bold,
                    Menu_LowerIf(menu_options[i].text, menu.select != i),
                    48 + (y >> 2),
                    SCREEN_HEIGHT2 + y - 8,
                    FontAlign_Left
                );
            }
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                113 >> 1, 253 >> 1, 146 >> 1,
                0, 0, 0
            );
            break;
        }
        case MenuPage_NetLobby:
        {
            //Check network state
            if (menu.next_page == menu.page && Trans_Idle())
            {
                if (!Network_Connected())
                {
                    //Disconnected
                    menu.next_page = MenuPage_NetFail;
                    menu.next_select = 0;
                    Trans_Start();
                }
            }
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "WAITING FOR HOST",
                SCREEN_WIDTH2,
                SCREEN_HEIGHT2 - 8,
                FontAlign_Center
            );
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                253 >> 1, 146 >> 1, 113 >> 1,
                0, 0, 0
            );
            break;
        }
        case MenuPage_NetFail:
        {
            //Return to main menu if circle or start is pressed
            if (menu.next_page == menu.page && Trans_Idle())
            {
                if (pad_state.press & (PAD_CIRCLE | PAD_START))
                {
                    menu.next_page = MenuPage_Main;
                    menu.next_select = 0;
                    Trans_Start();
                }
            }
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "DISCONNECTED",
                SCREEN_WIDTH2,
                SCREEN_HEIGHT2 - 8,
                FontAlign_Center
            );
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                253 >> 1, 30 >> 1, 15 >> 1,
                0, 0, 0
            );
            break;
        }
        case MenuPage_NetInitFail:
        {
            //Return to main menu if circle or start is pressed
            if (menu.next_page == menu.page && Trans_Idle())
            {
                if (pad_state.press & (PAD_CIRCLE | PAD_START))
                {
                    menu.next_page = MenuPage_Main;
                    menu.next_select = 0;
                    Trans_Start();
                }
            }
            
            //Draw page label
            menu.font_bold.draw(&menu.font_bold,
                "WSA INIT FAILED",
                SCREEN_WIDTH2,
                SCREEN_HEIGHT2 - 8,
                FontAlign_Center
            );
            
            //Draw background
            Menu_DrawBack(
                true,
                8,
                253 >> 1, 30 >> 1, 15 >> 1,
                0, 0, 0
            );
            break;
        }
    #endif
        case MenuPage_Stage:
        {
            //Unload menu state
            Menu_Unload();
            
            //Load new stage
            LoadScr_Start();
            Stage_Load(menu.page_param.stage.id, menu.page_param.stage.diff, menu.page_param.stage.story);
            gameloop = GameLoop_Stage;
            LoadScr_End();
            break;
        }
        default:
            break;
    }
    
    //Clear page swap flag
    menu.page_swap = menu.page != exec_page;
}