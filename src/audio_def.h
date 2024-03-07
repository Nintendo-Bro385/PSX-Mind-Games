#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{XA_Menu, XA_LENGTH(17000)}, //XA_GettinFreaky
	{XA_Menu, XA_LENGTH(3840)},  //XA_GameOver
	{XA_Menu, XA_LENGTH(960)},  //XA_GameOveruproar
	//WEEK1A.XA
	{XA_Week1A, XA_LENGTH(11800)}, //XA_Psychic
	{XA_Week1A, XA_LENGTH(11400)}, //XA_Wilter
	//WEEK1B.XA
	{XA_Week1B, XA_LENGTH(12000)}, //XA_Uproar
	{XA_Week1B, XA_LENGTH(11295)}, //XA_Freeky
	{XA_Week1B, XA_LENGTH(3700)}, //XA_Ludum
	//WEEK2A.XA
	{XA_Week2A, XA_LENGTH(11800)}, //XA_Psychic Old
	{XA_Week2A, XA_LENGTH(11100)}, //XA_Late Drive
	//WEEK2B.XA
	{XA_Week2B, XA_LENGTH(17778)}, //XA_Monster
	{XA_Week2B, XA_LENGTH(11000)}, //XA_Clucked
	//DIA.XA
	{XA_Dia, XA_LENGTH(4800)}, //XA_Psy
	{XA_Dia, XA_LENGTH(6400)}, //XA_Loveisaflower
	{XA_Dia, XA_LENGTH(7000)}, //XA_Spiritualunrest
	{XA_Dia, XA_LENGTH(2800)}, //XA_Itwillbeokay
	//CUTSCENE.XA
	{XA_Cutscene, XA_LENGTH(1600)}, //XA_BFtransform
	{XA_Cutscene, XA_LENGTH(800)}, //XA_Sendi
	{XA_Cutscene, XA_LENGTH(1300)}, //XA_BFtdemo
	//WEEK3A.XA
	{XA_Week3A, XA_LENGTH(11800)},  //XA_Flopchic
	{XA_Week3A, XA_LENGTH(10000)}, //XA_Philly
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\WEEK1A.XA;1", //XA_Week1A
	"\\MUSIC\\WEEK1B.XA;1", //XA_Week1B
	"\\MUSIC\\WEEK2A.XA;1", //XA_Week2A
	"\\MUSIC\\WEEK2B.XA;1", //XA_Week2B
	"\\MUSIC\\DIA.XA;1", //XA_DIA
	"\\MUSIC\\CUTSCENE.XA;1", //XA_CUTSCENE
	"\\MUSIC\\WEEK3A.XA;1", //XA_Week3A
	NULL,
};

typedef struct
{
	const char *name;
	boolean vocal;
} XA_Mp3;

static const XA_Mp3 xa_mp3s[] = {
	//MENU.XA
	{"freaky", false},   //XA_GettinFreaky
	{"gameover", false}, //XA_GameOver
	//WEEK1A.XA
	{"psychic", true}, //XA_Psychic
	{"wilter", true},   //XA_Wilter
	//WEEK1B.XA
	{"uproar", true}, //XA_Uproar
	{"freeky", false}, //XA_Freeky
	{"ludum", false}, //XA_Ludum
	//WEEK2A.XA
	{"psychico", true}, //XA_Psychico
	{"latedrive", true},    //XA_Latedrive
	//WEEK2B.XA
	{"monster", true}, //XA_Monster
	{"clucked", true}, //XA_Clucked
	//DIA.XA
	{"psy", false}, //XA_Psy
	{"loveisaflower", false}, //XA_Loveisaflower
	{"spiritualunrest", false}, //XA_Spiritualunrest
	{"itwillbeokay", false}, //XA_Itwillbeokay
	//CUTSCENE
	{"bftransform", false}, //XA_BFtransform
	{"sendai", false}, //XA_Sendai
	{"bftdemo", false}, //XA_BFtdemo
	//WEEK3A.XA
	{"flopchic", true},   //XA_Flopchic
	{"philly", true}, //XA_Philly
	
	{NULL, false}
};
