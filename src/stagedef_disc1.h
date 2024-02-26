	{ //StageId_1_1 (Psychic)
		//Characters
		{Char_BF_New,    FIXED_DEC(225,1),  FIXED_DEC(138,1)},
		{Char_Psychic_New, FIXED_DEC(16,1),  FIXED_DEC(118,1)},
		{NULL},
		
		//Stage background
		Back_Chop_New,
		
		//Song info
		{FIXED_DEC(15,10),FIXED_DEC(2,1),FIXED_DEC(25,10)},
		1, 1,
		XA_Psychic, 0,
		1, //dialogue check
		XA_Psy, 0, //dialogue song and it's channel
		
		StageId_1_5, STAGE_LOAD_FLAG || STAGE_LOAD_PLAYER || STAGE_LOAD_OPPONENT
	},
	{ //StageId_1_2 (Wilter)
		//Characters
		{Char_SenpaiB_New,    FIXED_DEC(225,1),  FIXED_DEC(138,1)},
		{Char_Psychic_New, FIXED_DEC(16,1),  FIXED_DEC(118,1)},
		{NULL},
		
		//Stage background
		Back_Chop_New,
		
		//Song info
		{FIXED_DEC(12,10),FIXED_DEC(2,1),FIXED_DEC(26,10)},
		1, 2,
		XA_Wilter, 2,
		1, //dialogue check
		XA_Loveisaflower, 1,
		
		StageId_1_6, STAGE_LOAD_FLAG | STAGE_LOAD_PLAYER | STAGE_LOAD_STAGE
	},
	{ //StageId_1_3 (Uproar)
		//Characters
		{Char_BFS_New,    FIXED_DEC(190,1),  FIXED_DEC(140,1)},
		{Char_Psychic_New, FIXED_DEC(16,1),  FIXED_DEC(118,1)},
		{NULL},
		
		//Stage background
		Back_FlameC_New,
		
		//Song info
		{FIXED_DEC(17,10),FIXED_DEC(23,10),FIXED_DEC(3,1)},
		1, 3,
		XA_Uproar, 0,
		1, //dialogue check
		XA_Spiritualunrest, 2,
		
		StageId_1_3, 0
	},
	{ //StageId_1_4 (Tutorial)
		//Characters
		{Char_BF_New, FIXED_DEC(60,1),  FIXED_DEC(100,1)},
		{Char_GF_New,  FIXED_DEC(0,1),  FIXED_DEC(-15,1)}, //TODO
		{NULL,           FIXED_DEC(0,1),  FIXED_DEC(-10,1)},
		
		//Stage background
		Back_Week1_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(1,1),FIXED_DEC(1,1)},
		1, 4,
		XA_Ludum, 2,
		0,
		XA_Clucked, 2,
		
		StageId_1_4, 0
	},
	{ //StageId_1_5 (Bf Transformation cutscene)
		//Characters
		{Char_BFT_New,    FIXED_DEC(225,1),  FIXED_DEC(138,1)},
		{Char_Psychic_New, FIXED_DEC(-15,1),  FIXED_DEC(108,1)},
		{NULL},
		
		//Stage background
		Back_Chop_New,
		
		//Song info
		{FIXED_DEC(15,10),FIXED_DEC(2,1),FIXED_DEC(25,10)},
		1, 5,
		//XA_BfTransform, 0,
		XA_BFtransform, 0,
		0,
		XA_Clucked, 2,
		
		StageId_1_2, STAGE_LOAD_FLAG || STAGE_LOAD_PLAYER || STAGE_LOAD_OPPONENT
	},
	{ //StageId_1_6 (Uproar Cutscene)
		//Characters
		{Char_Sendai_New,    FIXED_DEC(225,1),  FIXED_DEC(138,1)},
		{Char_Psychic_New, FIXED_DEC(-15,1),  FIXED_DEC(108,1)},
		{NULL},
		
		//Stage background
		Back_Chop_New,
		
		//Song info
		{FIXED_DEC(15,10),FIXED_DEC(2,1),FIXED_DEC(25,10)},
		1, 6,
		//XA_Sendai, 0,
		XA_Sendai, 1,
		0,
		XA_Clucked, 2,
		
		StageId_1_3, 0
	},
	{ //StageId_2_1 (Spookeez)
		//Characters
		{Char_BF_New,    FIXED_DEC(190,1),  FIXED_DEC(130,1)},
		{Char_OldPsychic_New, FIXED_DEC(-16,1),  FIXED_DEC(112,1)},
		{Char_GF_New,      FIXED_DEC(115,1),  FIXED_DEC(37,1)},
		
		//Stage background
		Back_Old_New,
		
		//Song info
		{FIXED_DEC(15,10),FIXED_DEC(2,1),FIXED_DEC(25,10)},
		2, 1,
		XA_Psychico, 0,
		1,
		XA_Psy, 0,
		
		StageId_2_2, STAGE_LOAD_FLAG | STAGE_LOAD_STAGE
	},
	{ //StageId_2_2 (South)
		//Characters
		{Char_BF_New,    FIXED_DEC(190,1),  FIXED_DEC(130,1)},
		{Char_OldPsychic_New, FIXED_DEC(-16,1),  FIXED_DEC(112,1)},
		{Char_GF_New,      FIXED_DEC(115,1),  FIXED_DEC(37,1)},
		
		//Stage background
		Back_Space_New,
		
		//Song info
		{FIXED_DEC(25,10),FIXED_DEC(25,10),FIXED_DEC(25,10)},
		2, 2,
		XA_Latedrive, 2,
		1,
		XA_Itwillbeokay, 3,
		
		StageId_2_2, 0
	},
	{ //StageId_2_3 (Monster)
		//Characters
		{Char_BF_New,      FIXED_DEC(225,1),  FIXED_DEC(138,1)},
		{Char_Flopchic_New,  FIXED_DEC(16-10,1),  FIXED_DEC(118+46,1)},
		{Char_OldPsychic_New,  FIXED_DEC(16-100,1),  FIXED_DEC(118+20,1)},
		
		//Stage background
		Back_FChop_New,
		
		//Song info
		{FIXED_DEC(15,10),FIXED_DEC(2,1),FIXED_DEC(25,10)},
		2, 3,
		XA_Flopchic, 0,
		1,
		XA_Psy, 0,
		
		StageId_2_3, 0
	},
	
	{ //StageId_3_1 (Pico)
		//Characters
		{Char_BF_New,      FIXED_DEC(225,1),  FIXED_DEC(138,1)},
		{Char_OldPsychic_New,  FIXED_DEC(16,1),  FIXED_DEC(118,1)},
		{Char_Psychic_New,       FIXED_DEC(115,1),  FIXED_DEC(37,1)},
		
		//Stage background
		Back_Chop_New,
		
		//Song info
		{FIXED_DEC(12,10),FIXED_DEC(14,10),FIXED_DEC(16,10)},
		3, 1,
		XA_Flopchic, 0,
		0,
		XA_Clucked, 2,
		
		StageId_3_2, STAGE_LOAD_FLAG
	},
	{ //StageId_3_2 (Philly)
		//Characters
		{Char_BF_New,      FIXED_DEC(225,1),  FIXED_DEC(138,1)},
		{Char_OldPsychic_New,  FIXED_DEC(16,1),  FIXED_DEC(118,1)},
		{Char_Psychic_New,       FIXED_DEC(115,1),  FIXED_DEC(37,1)},
		
		//Stage background
		Back_Chop_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(13,10),FIXED_DEC(2,1)},
		3, 2,
		XA_Philly, 2,
		0,
		XA_Clucked, 2,
		
		StageId_3_2, 0
	},
	{ //StageId_2_4 (Clucked)
		//Characters
		{Char_BF_New,       FIXED_DEC(60,1),  FIXED_DEC(100,1)},
		{Char_Clucky_New, FIXED_DEC(-120,1),  FIXED_DEC(100,1)},
		{Char_GF_New,        FIXED_DEC(0,1),    FIXED_DEC(0,1)},
		
		//Stage background
		Back_Week1_New,
		
		//Song info
		{FIXED_DEC(25,10),FIXED_DEC(25,10),FIXED_DEC(25,10)},
		2, 4,
		XA_Clucked, 2,
		0,
		XA_Clucked, 2,
		
		StageId_2_4, 0
	},
