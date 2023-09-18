/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_AUDIO_H
#define PSXF_GUARD_AUDIO_H

#include "psx.h"

//XA enumerations
typedef enum
{
	XA_Menu,   //MENU.XA
	XA_Week1A, //WEEK1A.XA
	XA_Week1B, //WEEK1B.XA
	XA_Week2A, //WEEK2A.XA
	XA_Week2B, //WEEK2B.XA
	XA_Dia, //DIA.XA
	XA_Cutscene, //CUTSCENE.XA
	XA_Week3A, //WEEK3A.XA
	
	XA_Max,
} XA_File;

typedef enum
{
	//MENU.XA
	XA_GettinFreaky, //Gettin' Freaky
	XA_GameOver,     //Game Over
	//WEEK1A.XA
	XA_Psychic, //Psychic
	XA_Wilter,   //Wilter
	//WEEK1B.XA
	XA_Uproar, //Uproar
	XA_Freeky,  //Tutorial
	XA_Ludum,  //Tutorial
	//WEEK2A.XA
	XA_Psychico, //Psychic Old
	XA_Latedrive,    //Late Drive
	//WEEK2B.XA
	XA_Monster, //Monster
	XA_Clucked, //Clucked
	//DIA.XA
	XA_Psy, //Monster
	XA_Loveisaflower, //Clucked
	XA_Spiritualunrest,
	XA_Itwillbeokay,
	//CUTSCENE.XA
	XA_BFtransform,
	XA_Sendai,
	XA_BFtdemo,
	//WEEK3A.XA
	XA_Flopchic,   //Flopchic
	XA_Philly, //Philly
	
	XA_TrackMax,
} XA_Track;

//Audio functions
void Audio_Init(void);
void Audio_Quit(void);
void Audio_PlayXA_Track(XA_Track track, u8 volume, u8 channel, boolean loop);
void Audio_SeekXA_Track(XA_Track track);
void Audio_PauseXA(void);
void Audio_ResumeXA(void);
void Audio_StopXA(void);
void Audio_ChannelXA(u8 channel);
s32 Audio_TellXA_Sector(void);
s32 Audio_TellXA_Milli(void);
boolean Audio_PlayingXA(void);
void Audio_WaitPlayXA(void);
void Audio_ProcessXA(void);

#endif
