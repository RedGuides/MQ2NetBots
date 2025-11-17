
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Projet: MQ2NetBots.cpp
// Author: s0rCieR
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//
// Deadchicken added .Duration on or about September 2007 and tried not to
// mangle s0rCieR's code to much.  Thanks to Kroak for helping debug.
//  Updated for 10/9 release of MQ2
// CombatState member added  Thanks mijuki.
// .Stacks member added
//
// v2.1 woobs
//    - Upped most of the maximum values to handle more buffs.
//    - Add Detrimental information to merge in MQ2Debuffs functions
// v2.2 eqmule
//    - Added String Safety
// v3.0 woobs
//    - Updated .Stacks
//    - Added   .StacksPet
// v3.1 woobs
//    - Updated for Spell Blocker (148) fix from swifty.
// v3.2 woobs
//    - Added NBGetEffectAmt to get adjusted values for compare.
//    - Removed 85/419 from triggering formula. These Procs stack/don't stack based on
//      the normal base check (for these, it is SpellID). It seems newer spells simply
//      overwrite older spells.
// v4.0 woobs
//    - Merged in MMOBUGS data items and in-game window (thank you MMOBUGS).
//    - Added Detrimental/Beneficial buff type data items.
//    - Remove internal Stacks/StacksPet. Use new EQ/MQ coding.
//    - Added WillLand,WillLandPet,TooPowerful,TooPowerfulPet.
//    - Added Extended data options for Gems and Buff/Song/Pet Durations.
//    - Added TotalCounters,CountersCurse,CountersDisease,CountersPoison,CountersCorruption
//      to accurately report current counters on the client (not counters on the spell).
//    - Added PetAffinity and AAPointsAssigned.
//    - Added a new version of the in-game window (switchable with old) along with new
//      ini and command options. LClick,RClick,ConColors,CleanNames. 
//    - Code clean-up. 

#include <mq/Plugin.h>

#include <Blech/Blech.h>

#include <string>

PreSetup("MQ2NetBots");
PLUGIN_VERSION(4.0);

#define PLUGIN_NAME "MQ2NetBots"

bool DEBUGGING = false;

constexpr int PETS_MAX	= MAX_TOTAL_BUFFS_NPC;
constexpr int NOTE_MAX	= 500;
constexpr int NETTICK	= 50;
constexpr int REFRESH	= 60000;
constexpr int UPDATES	= 6000;

#if defined(ROF2EMU)
constexpr char PetAAName[] = "Pet Affinity";
constexpr int  PetAARank   = 1;
#else
constexpr char PetAAName[] = "Companion's Suspension";
constexpr int  PetAARank   = 3;
#endif

enum eStates : uint32_t {
	STATE_DEAD 		= 0x00000001,
	STATE_FEIGN 	= 0x00000002,
	STATE_DUCK		= 0x00000004,
	STATE_BIND		= 0x00000008,
	STATE_STAND		= 0x00000010,
	STATE_SIT		= 0x00000020,
	STATE_MOUNT		= 0x00000040,
	STATE_INVIS		= 0x00000080,
	STATE_ITU		= 0x00000100,
	STATE_ATTACK	= 0x00000200,
	STATE_MOVING	= 0x00000400,
	STATE_STUN		= 0x00000800,
	STATE_RAID		= 0x00001000,
	STATE_GROUP		= 0x00002000,
	STATE_LFG		= 0x00004000,
	STATE_AFK		= 0x00008000,
	STATE_LEV		= 0x00010000,
	STATE_RANGED	= 0x00020000,
	STATE_WANTAGGRO	= 0x00040000,
	STATE_HAVEAGGRO	= 0x00080000,
	STATE_HOVER		= 0x00100000,
	STATE_NAVACTIVE	= 0x00200000,
	STATE_NAVPAUSED	= 0x00400000,
	STATE_BOTACTIVE	= 0x00800000,
	STATE_HASPETAA  = 0x01000000
};

enum eElements {
	E_BUFFS,
	E_CASTD,
	E_ENDUS,
	E_EXPER,
	E_LEADR,
	E_LEVEL,
	E_HPS,
	E_MANAS,
	E_PBUFF,
	E_PETIL,
	E_SPGEM,
	E_SONGS,
	E_STATE,
	E_TARGT,
	E_ZONES,
	E_BUFFD,
	E_LOCAT,
	E_HEADN,
	E_AAPTS,
	E_OOCST,
	E_NOTE,
	E_DETR,
	E_FREEB,
	E_EXTEND,
	E_MACRO,
	E_FREEI,
	E_VERSN,
	E_CAMPS,
	E_LUAINFO,
	E_EQBC,
	E_QUERY,
	E_BSTAT,
	E_SONGD,
	E_PETD,
	ESIZE
};

enum buffTypes {
	BT_DETRIMENTAL,
	BT_BENEFICIAL
};

enum buffDetrimentalStates : uint32_t {
	BD_SLOWED		= 0x00000001,
	BD_ROOTED		= 0x00000002,
	BD_MESMERIZED	= 0x00000004,
	BD_CRIPPLED		= 0x00000008,
	BD_MALOED		= 0x00000010,
	BD_TASHED		= 0x00000020,
	BD_SNARED		= 0x00000040,
	BD_REVDSED		= 0x00000080,
	BD_CHARMED		= 0x00000100,
	BD_DISEASED		= 0x00000200,
	BD_POISONED		= 0x00000400,
	BD_CURSED		= 0x00000800,
	BD_CORRUPTED	= 0x00001000,
	BD_BLINDED		= 0x00002000,
	BD_CASTINGLEVEL	= 0x00004000,
	BD_ENDUDRAIN	= 0x00008000,
	BD_FEARED		= 0x00010000,
	BD_HEALING		= 0x00020000,
	BD_INVULNERABLE	= 0x00040000,
	BD_LIFEDRAIN	= 0x00080000,
	BD_MANADRAIN	= 0x00100000,
	BD_RESISTANCE	= 0x00200000,
	BD_SILENCED		= 0x00400000,
	BD_SPELLCOST	= 0x00800000,
	BD_SPELLDAMAGE	= 0x01000000,
	BD_SPELLSLOWED	= 0x02000000,
	BD_TRIGGER		= 0x04000000 
};

enum buffBeneficialStates : uint32_t {
	BB_DSED			= 0x00000001,
	BB_AEGO			= 0x00000002,
	BB_SKIN			= 0x00000004,
	BB_FOCUS		= 0x00000008,
	BB_REGEN		= 0x00000010,
	BB_SYMBOL		= 0x00000020,
	BB_CLARITY		= 0x00000040,
	BB_PRED			= 0x00000080,
	BB_STRENGTH		= 0x00000100,
	BB_BRELLS		= 0x00000200,
	BB_SV			= 0x00000400,
	BB_SE			= 0x00000800,
	BB_HYBRIDHP		= 0x00001000,
	BB_GROWTH		= 0x00002000,
	BB_SHINING		= 0x00004000,
	BB_HASTED		= 0x00008000
};

enum eSpellTypes {
	ST_SLOWED = 1,
	ST_ROOTED,
	ST_MESMERIZED,
	ST_CRIPPLED,
	ST_MALOED,
	ST_TASHED,
	ST_SNARED,
	ST_REVDSED,
	ST_CHARMED,
	ST_DISEASED,
	ST_POISONED,
	ST_CURSED,
	ST_CORRUPTED,
	ST_BLINDED,
	ST_CASTINGLEVEL,
	ST_ENDUDRAIN,
	ST_FEARED,
	ST_HEALING,
	ST_INVULNERABLE,
	ST_LIFEDRAIN,
	ST_MANADRAIN,
	ST_RESISTANCE,
	ST_SILENCED,
	ST_SPELLCOST,
	ST_SPELLDAMAGE,
	ST_SPELLSLOWED,
	ST_TRIGGER,
	ST_DSED,
	ST_AEGO,
	ST_SKIN,
	ST_FOCUS,
	ST_REGEN,
	ST_SYMBOL,
	ST_CLARITY,
	ST_PRED,
	ST_STRENGTH,
	ST_BRELLS,
	ST_SV,
	ST_SE,
	ST_HYBRIDHP,
	ST_GROWTH,
	ST_SHINING,
	ST_HASTED
};

enum eDetrimentals {
	D_DETRIMENTALS,
	D_COUNTERS,
	D_CURSED,
	D_DISEASED,
	D_POISONED,
	D_CORRUPTED,
	D_NOCURE,
	D_LIFEDRAIN,
	D_MANADRAIN,
	D_ENDUDRAIN,
	DSIZE
};

enum eItemSize {
	I_TINY = 0,
	I_SMALL = 1,
	I_MEDIUM = 2,
	I_LARGE = 3,
	I_GIANT = 4,
	ISIZE
};

enum eMacroStatus {
	MACRO_NONE,
	MACRO_RUNNING,
	MACRO_PAUSED
};

enum eClickActions {
	CA_NO_ACTION = 0,
	CA_BRING_TO_FOREGROUND = 1,
	CA_TARGET_PLAYER = 2,
	CA_TARGET_PLAYER_TARGET = 3,
	CASIZE
};

enum eInvisStates : uint32_t {
	INVIS_NORMAL = 0x00000001,
	INVIS_UNDEAD = 0x00000002
};

enum eExtended {
	EX_NONE = 0,
	EX_GEMS_AND_BUFF_DURATIONS = 1,
	EX_SONG_DURATIONS = 2,
	EX_PET_DURATIONS = 3,
	EXSIZE
};

struct CaseInsensitiveComparator
{
    bool operator() (const std::string& a, const std::string& b) const noexcept
    {
        return _stricmp(a.c_str(), b.c_str()) < 0;
    }
};

class BotInfo {
public:
	char        Name[EQ_MAX_NAME];          // Client Name
	int         ClassID;                    // Class ID
	int         Level;                      // Level
	int         SpawnID;                    // Spawn ID
	int         ZoneID;                     // Zone ID
	int         InstanceID;                 // Instance ID
	int         HPCurrent;                  // HP Current
	int         HPMax;                      // HP Maximum
	int         ManaCurrent;                // Mana Current
	int         ManaMax;                    // Mana Maximum
	int         EnduranceCurrent;           // Endurance Current
	int         EnduranceMax;               // Endurance Maximum
	int         PetID;                      // Pet ID
	int         PetHPPct;                   // Pet HP Percentage
	int         TargetID;                   // Target ID
	int         TargetHPPct;                // Target HP Percentage
    int         Gem[NUM_SPELL_GEMS];        // Spells Memorized
	int         Buff[NUM_LONG_BUFFS];       // Buffs
	int         Song[NUM_SHORT_BUFFS];      // Songs
	int         Pets[PETS_MAX];             // Pet Buffs
    int         BDuration[NUM_LONG_BUFFS];  // Buff durations
    int         SDuration[NUM_SHORT_BUFFS]; // Song durations
    int         PDuration[PETS_MAX];        // Pet Buff durations
	int         FreeBuff;                   // Free Buff Slots
	int         CastID;                     // Casting Spell ID
	int         CombatState;                // Combat State
	int64_t     Exp;                        // Experience
	uint32_t    AAExp;                      // AA Experience
#if HAS_LEADERSHIP_EXPERIENCE
	double      GroupLeaderExp;             // Group Leadership Experience
#endif
	char        Leader[EQ_MAX_NAME];        // Group Leader Name
	char        Location[64];               // Y,X,Z
    float       X;                          // Location X-value
    float       Y;                          // Location Y-value
    float       Z;                          // Location Z-value
	char		Heading[64];                // Heading
	int         AAPoints;                   // Unused AA Points
	int         AAPointsSpent;              // Spent AA Points
	int         AAPointsAssigned;           // Assigned AA Points
	uint32_t    State;                      // State
	uint32_t    DetrState;                  // Detrimental Buff States
	uint32_t    BeneState;                  // Beneficial Buff States
	int64_t     Detrimental[DSIZE];         // Detrimentals
	int         FreeInventory[ISIZE];       // Free Inventory Slots of, at least, size 0-4
	float       Version;                    // Plugin Version
	int         Extended;                   // Extended Setting
	int         MacroState;                 // Current Macro State (0=No Macro Running,1=Running,2=Paused)
	char        MacroName[MAX_PATH];        // Current Macro Name
	int         MakeCampStatus;             // MakeCamp Status (0=None/OFF,1=ON,2=PAUSED)
	float       MakeCampX;                  // MakeCamp AnchorX
	float       MakeCampY;                  // MakeCamp AnchorY
	float       MakeCampRadius;             // MakeCamp CampRadius
	float       MakeCampDistance;           // MakeCamp CampDistance
	char        LuaInfo[MAX_STRING];        // LUA Process Info
	uint64_t    EQBC_Packets;               // EQBC Packets
	uint64_t    EQBC_HeartBeat;             // EQBC HeartBeat
	char        Note[NOTE_MAX];             // User Message
	char        Query[MAX_STRING];          // Result of NetQuery
	uint32_t    Updated;                    // Update Time
};

int             NetInit = 0;                // Plugin Initialized?
int             NetStat = 0;                // Plugin is On?
int             NetGrab = 0;                // Grab Information?
int             NetSend = 0;                // Send Information?
int             NetExtended = 0;            // Send Extended Information (0=None, 1=Gems+Buff Durations, 2=Add Short Durations, 3= Add Pet Buff Durations)
int             NetSimple = 0;              // Allow any substring of Buff/ShortBuff/PetBuff to return result
int             NetShow = 0;                // Show UI window?
int             NetLClick = 0;              // UI Window Left Click Action
int             NetRClick = 0;              // UI Window Right Click Action
int             NetConColors = 0;           // Use NPC Con Colors in UI Window
int             NetCleanNames = 0;          // Use NPC Clean Names in UI Window
int             NetRightMost = 0;           // If > 0, only send this number right-most characters of target name in UI Window
int             NetUseNewWindow = -1;       // 0/1 Use Old/New NetBots Window layout (requires the associated MQUI_NetBotsWnd.xml file)
int             NetEQBCData = 0;            // Send EQBC Packets/Heartbeat data
int             NetLast = 0;                // Last Send Time Mark
uint64_t        ShowTime = 0;               // Window Show Time Mark
char            NetNote[NOTE_MAX];          // Network Note
char            NetQuery[MAX_STRING];       // Network Query
char            NetWindowTitle[MAX_STRING]; // UI Window Title (Can be Macro-Parsed)

std::map<std::string, std::shared_ptr<BotInfo>, CaseInsensitiveComparator> NetMap;  // BotInfo Mapped List
Blech           Packet('#');                                                        // BotInfo Event Triggers
BotInfo         *CurBot = 0;                                                        // BotInfo Current Bot

int             sTimers[ESIZE];      		// Save Timers
char            sBuffer[ESIZE][MAX_STRING]; // Save Buffer
char            wBuffer[ESIZE][MAX_STRING]; // Work Buffer
bool            wChange[ESIZE];     		// Work Change
bool            wUpdate[ESIZE];    			// Work Update

char            bBuffs[MAX_STRING];         // Buff List
char            bSongs[MAX_STRING];         // Song List
char            bPBuffs[MAX_STRING];        // Pet Buff List
char            bBuffd[MAX_STRING];         // Buff Duration List
char            bSongd[MAX_STRING];         // Song Duration List
char            bPBuffd[MAX_STRING];        // Pet Buff Duration List
uint32_t        detrStatus;                 // Detrimental Buff Status (States)
uint32_t        beneStatus;                 // Beneficial Buff Status (States)
uint32_t        invisStatus;                // Invisible Buff Status (States)
int64_t         dValues[DSIZE];             // Detrimental Counters and Drain Values
int             AvailBuffSlots;             // Available Buff Slots

char            GlobalINIFileName[MAX_STRING] = { 0 };
char            PlayerSectionName[MAX_STRING] = { 0 };
char            WindowID[MAX_STRING]          = { 0 };
char            ScreenID[MAX_STRING]          = { 0 };

struct BuffData {							// Buff Data (for mapped list)
	int 		Type;						// Buff Type (BT_) Enum
	uint32_t	State;						// Buff Detrimental/Benefical State (BD_/BB_) Enum
	int			SpellType;					// Buff Spell Type (ST_) Enum
	
	BuffData(int i, uint32_t j, int k) : Type(i), State(j), SpellType(k) {}
};
std::map<std::string, BuffData> BuffMap;	// Buff Data Mapped List

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

bool EQBCConnected() {
	using fEqbcIsConnected = uint16_t(*)();
	if (fEqbcIsConnected checkf = (fEqbcIsConnected)GetPluginProc("MQ2EQBC", "isConnected")) {
		return checkf();
	}
	return false;
}

void EQBCBroadCast(const char* Buffer) {
	using fEqbcNetBotSendMsg = void(*)(const char*);
	if (strlen(Buffer) > 9) {
		if (fEqbcNetBotSendMsg requestf = (fEqbcNetBotSendMsg)GetPluginProc("MQ2EQBC", "NetBotSendMsg")) {
			if (DEBUGGING) {
				DebugSpewAlways("%s->BroadCasting(%s)", PLUGIN_NAME, Buffer);
			}
			requestf(Buffer);
		}
	}
}

std::shared_ptr<BotInfo> BotLoad(const char* Name) {
	BotInfo RecInfo;
	ZeroMemory(&RecInfo.Name, sizeof(BotInfo));
	strcpy_s(RecInfo.Name, Name);
	auto [f, _] = NetMap.emplace(RecInfo.Name, std::make_shared<BotInfo>(RecInfo));
	return f->second;
}

void BotQuit(const char* Name) {
	auto f = NetMap.find(Name);
	if (NetMap.end() != f)
		NetMap.erase(f);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void InfoSong(BotInfo* botInfo, const char* Line) {
	char szTemp[MAX_STRING];
	for (int i = 0; i < NUM_SHORT_BUFFS; i++) {
		GetArg(szTemp, Line, i + 1, FALSE, FALSE, FALSE, ':');
		botInfo->Song[i] = GetIntFromString(szTemp, 0);
	}
}

void InfoPets(BotInfo* botInfo, const char* Line) {
	char szTemp[MAX_STRING];
	for (int i = 0; i < PETS_MAX; i++) {
		GetArg(szTemp, Line, i + 1, FALSE, FALSE, FALSE, ':');
		botInfo->Pets[i] = GetIntFromString(szTemp, 0);
	}
}

void InfoGems(BotInfo* botInfo, const char* Line) {
	char szTemp[MAX_STRING];
	for(int i = 0; i < NUM_SPELL_GEMS; i++) {
		GetArg(szTemp, Line, i + 1, FALSE, FALSE, FALSE, ':');
		botInfo->Gem[i] = GetIntFromString(szTemp, 0);
	}
}

void InfoBuff(BotInfo* botInfo, const char* Line) {
	char szTemp[MAX_STRING];
	for (int i = 0; i < NUM_LONG_BUFFS; i++) {
		GetArg(szTemp, Line, i + 1, FALSE, FALSE, FALSE, ':');
		botInfo->Buff[i] = GetIntFromString(szTemp, 0);
	}
}

void InfoBDura(BotInfo* botInfo, const char* Line) {
	char szTemp[MAX_STRING];
	for(int i = 0; i < NUM_LONG_BUFFS; i++) {
		GetArg(szTemp, Line, i + 1, FALSE, FALSE, FALSE, ':');
		botInfo->BDuration[i] = GetIntFromString(szTemp, 0);
	}
}

void InfoSDura(BotInfo* botInfo, const char* Line) {
	char szTemp[MAX_STRING];
	for(int i = 0; i < NUM_SHORT_BUFFS; i++) {
		GetArg(szTemp, Line, i + 1, FALSE, FALSE, FALSE, ':');
		botInfo->SDuration[i] = GetIntFromString(szTemp, 0);
	}
}

void InfoPDura(BotInfo* botInfo, const char* Line) {
	char szTemp[MAX_STRING];
	for(int i = 0; i < PETS_MAX; i++) {
		GetArg(szTemp, Line, i + 1, FALSE, FALSE, FALSE, ':');
		botInfo->PDuration[i] = GetIntFromString(szTemp, 0);
	}
}

void InfoDetr(BotInfo* botInfo, const char* Line) {
	char szTemp[MAX_STRING];
	for (int i = 0; i < DSIZE; i++) {
		GetArg(szTemp, Line, i + 1, FALSE, FALSE, FALSE, ':');
		botInfo->Detrimental[i] = GetInt64FromString(szTemp, 0);
	}
}

void InfoLoc(BotInfo* botInfo, const char* Line) {
	char szTemp[MAX_STRING];
	strcpy_s(botInfo->Location, Line);
	GetArg(szTemp, Line, 1, FALSE, FALSE, FALSE, ':');
	botInfo->Y = GetFloatFromString(szTemp, 0);
	GetArg(szTemp, Line, 2, FALSE, FALSE, FALSE, ':');
	botInfo->X = GetFloatFromString(szTemp, 0);
	GetArg(szTemp, Line, 3, FALSE, FALSE, FALSE, ':');
	botInfo->Z = GetFloatFromString(szTemp, 0);
}

void InfoFreeI(BotInfo* botInfo, const char* Line) {
	char szTemp[MAX_STRING];
	for (int i = 0; i < ISIZE; i++) {
		GetArg(szTemp, Line, i + 1, FALSE, FALSE, FALSE, ':');
		botInfo->FreeInventory[i] = GetIntFromString(szTemp, 0);
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

bool Evaluate(const char* expression) {
	char szTemp[MAX_STRING] = { 0 };
	strcpy_s(szTemp, expression);
	ParseMacroData(szTemp, sizeof(szTemp));
	return GetIntFromString(szTemp, 0) != 0;
}

int GetAARankByName(const char* AAName) {
	int level = pLocalPlayer->Level;
	for (int nAbility = 0; nAbility < AA_CHAR_MAX_REAL; nAbility++) {
		if (CAltAbilityData* pAbility = GetAAById(pLocalPC->GetAlternateAbilityId(nAbility), level)) {
			if (const char* pName = pCDBStr->GetString(pAbility->nName, eAltAbilityName)) {
				if (!_stricmp(AAName, pName)) {
					int CurrentRank = pAbility->CurrentRank - 1;
					if (pLocalPC->HasAlternateAbility(pAbility->Index)) {
						CurrentRank++;
					}
					return CurrentRank;
				}
			}
		}
	}
	return 0;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

template <unsigned int _Size>char* MakeBUFFS(char(&Buffer)[_Size]) {
	strcpy_s(Buffer, bBuffs);
	return Buffer;
}

template <unsigned int _Size>char* MakeBUFFD(char(&Buffer)[_Size]) {
	if (NetExtended >= EX_GEMS_AND_BUFF_DURATIONS) { 
		strcpy_s(Buffer, bBuffd);
	} else {
		Buffer[0] = 0;
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeSONGS(char(&Buffer)[_Size]) {
	strcpy_s(Buffer, bSongs);
	return Buffer;
}

template <unsigned int _Size>char* MakeSONGD(char(&Buffer)[_Size]) {
	if (NetExtended >= EX_SONG_DURATIONS) { 
		strcpy_s(Buffer, bSongd);
	} else {
		Buffer[0] = 0;
	}
	return Buffer;
}

template <unsigned int _Size>char* MakePBUFF(char(&Buffer)[_Size]) {
	strcpy_s(Buffer, bPBuffs);
	return Buffer;
}

template <unsigned int _Size>char* MakePETD(char(&Buffer)[_Size]) {
	if (NetExtended >= EX_PET_DURATIONS) { 
		strcpy_s(Buffer, bPBuffd);
	} else {
		Buffer[0] = 0;
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeFREEB(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%d", AvailBuffSlots);
	return Buffer;
}

template <unsigned int _Size>char* MakeCASTD(char(&Buffer)[_Size]) {
	int Casting = pLocalPlayer->CastingData.SpellID;
	if (Casting > 0) {
		sprintf_s(Buffer, "%d", Casting);
	} else {
		Buffer[0] = 0;
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeHEADN(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%4.2f", pLocalPlayer->Heading);
	return Buffer;
}

template <unsigned int _Size>char* MakeLOCAT(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%4.2f:%4.2f:%4.2f:%d", pLocalPlayer->Y, pLocalPlayer->X, pLocalPlayer->Z, pLocalPlayer->Animation);
	return Buffer;
}

template <unsigned int _Size>char* MakeENDUS(char(&Buffer)[_Size]) {
	if (int Max = GetMaxEndurance()) {
		sprintf_s(Buffer, "%d/%d", GetCurEndurance(), Max);
	} else {
		strcpy_s(Buffer, "/");
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeMACRO(char(&Buffer)[_Size]) {
	int Status = MACRO_NONE;
	if (gszMacroName[0]) {
		Status = MACRO_RUNNING;
		if (MQMacroBlockPtr pBlock = GetCurrentMacroBlock()) {
			if (pBlock->Paused) {
				Status = MACRO_PAUSED;
			}
		}
		sprintf_s(Buffer, "%d:%s", Status, gszMacroName);
	} else {
		sprintf_s(Buffer, "%d:", Status);
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeEXPER(char(&Buffer)[_Size]) {
#if HAS_LEADERSHIP_EXPERIENCE
	sprintf_s(Buffer, "%I64d:%d:%02.3f", pLocalPC->Exp, pLocalPC->AAExp, pLocalPC->GroupLeadershipExp);
#else
	sprintf_s(Buffer, "%I64d:%d", pLocalPC->Exp, pLocalPC->AAExp);
#endif
	return Buffer;
}

template <unsigned int _Size>char* MakeLEADR(char(&Buffer)[_Size]) {
	Buffer[0] = 0;
	if (pLocalPC && pLocalPC->Group && pLocalPC->Group->GetGroupLeader()) {
		strcpy_s(Buffer, pLocalPC->Group->GetGroupLeader()->GetName());
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeNOTE(char(&Buffer)[_Size]) {
	strcpy_s(Buffer, NetNote);
	return Buffer;
}

template <unsigned int _Size>char* MakeLEVEL(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%d:%d", GetPcProfile()->Level, GetPcProfile()->Class);
	return Buffer;
}

template <unsigned int _Size>char* MakeHPS(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%d/%d", GetCurHPS(), GetMaxHPS());
	return Buffer;
}

template <unsigned int _Size>char* MakeMANAS(char(&Buffer)[_Size]) {
	if (int Max = GetMaxMana()) {
		sprintf_s(Buffer, "%d/%d", GetCurMana(), Max);
	} else {
		strcpy_s(Buffer, "/");
	}
	return Buffer;
}

template <unsigned int _Size>char* MakePETIL(char(&Buffer)[_Size]) {
	auto Pet = GetSpawnByID(pLocalPlayer->PetID);
	if (Pet && pPetInfoWnd) {
		sprintf_s(Buffer, "%d:%d", Pet->SpawnID, static_cast<int32_t>(Pet->HPMax == 0 ? 0 : Pet->HPCurrent * 100 / Pet->HPMax));
	} else {
		strcpy_s(Buffer, ":");
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeSPGEM(char(&Buffer)[_Size]) {
	Buffer[0] = '\0';
	if (NetExtended >= EX_GEMS_AND_BUFF_DURATIONS) {
		char szTemp[MAX_STRING] = { 0 };
		for (int i = 0; i < NUM_SPELL_GEMS; i++) {
			if (auto pSpell = GetSpellByID(GetMemorizedSpell(i))) {
				sprintf_s(szTemp, "%d:", pSpell->ID);
	  			strcat_s(Buffer, szTemp);
			}
			else {
				strcat_s(Buffer,"0:");
			}
		}
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeBSTAT(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%u:%u", detrStatus, beneStatus);
	return Buffer;
}

template <unsigned int _Size>char* MakeSTATE(char(&Buffer)[_Size]) {
	uint32_t Status = 0;
	if (pEverQuestInfo->bAutoAttack) {
		Status |= STATE_ATTACK;
	}
	if (pEverQuestInfo->bAutoRangeAttack) {
		Status |= STATE_RANGED;
	}
	if (pRaid && pRaid->RaidMemberCount) {
		Status |= STATE_RAID;
	}
	if (pLocalPlayer->PlayerState & 0x20) {
		Status |= STATE_STUN;
	}
	if (pLocalPC->pGroupInfo) {
		Status |= STATE_GROUP;
	}
	if (fabs(pLocalPlayer->SpeedRun) > 0.0f) {
		Status |= STATE_MOVING;
	}
	if (pLocalPlayer->Mount) {
		Status |= STATE_MOUNT;
	}
	if (pLocalPlayer->AFK) {
		Status |= STATE_AFK;
	}
	if ((pLocalPlayer->HideMode & 0x01) ||
		(invisStatus & INVIS_NORMAL)) {
		Status |= STATE_INVIS;
	}
	if (invisStatus & INVIS_UNDEAD) {
		Status |= STATE_ITU;
	}
	if (pLocalPlayer->mPlayerPhysicsClient.Levitate == 2) {
		Status |= STATE_LEV;
	}
	if (pLocalPlayer->LFG) {
		Status |= STATE_LFG;
	}
	if (pLocalPlayer->RespawnTimer != 0) {
		Status |= STATE_HOVER;
	}
	switch (pLocalPlayer->StandState) {
		case STANDSTATE_STAND:
			Status |= STATE_STAND;
			break;
		case STANDSTATE_SIT:
			Status |= STATE_SIT;
			break;
		case STANDSTATE_DUCK:
			Status |= STATE_DUCK;
			break;
		case STANDSTATE_BIND:
			Status |= STATE_BIND;
			break;
		case STANDSTATE_FEIGN:
			Status |= STATE_FEIGN;
			break;
		case STANDSTATE_DEAD:
			Status |= STATE_DEAD;
			break;
		default:
			break;
	}
	if (pTarget && pLocalPlayer->TargetOfTarget == pLocalPlayer->GetId()) {
		if (GetSpawnType(pTarget) == NPC) {
			Status |= STATE_HAVEAGGRO;
		}
	}
	if (int i = IsPluginLoaded("MQ2Melee") ? Evaluate("${If[${Melee.AggroMode},1,0]}") : 0) {
		Status |= STATE_WANTAGGRO;
	}
	if (IsPluginLoaded("MQ2Nav")) {
		if (Evaluate("${If[${Navigation.Active},1,0]}")) {
			Status |= STATE_NAVACTIVE;
		}
		if (Evaluate("${If[${Navigation.Paused},1,0]}")) {
			Status |= STATE_NAVPAUSED;
		}
	}
	if (int i = IsPluginLoaded("MQ2Bot") ? Evaluate("${If[${Bot.Active},1,0]}") : 0) {
		Status |= STATE_BOTACTIVE;
	}
	if (int Rank = GetAARankByName(PetAAName)) {
		if (Rank >= PetAARank) {
			Status |= STATE_HASPETAA;
		}
	}
	sprintf_s(Buffer, "%u", Status);
	return Buffer;
}

template <unsigned int _Size>char* MakeOOCST(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%d", GetCombatState());
	return Buffer;
}

template <unsigned int _Size>char* MakeAAPTS(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%d:%d:%d", GetPcProfile()->AAPoints, GetPcProfile()->AAPointsSpent, GetPcProfile()->AAPointsAssigned[0]);
	return Buffer;
}

template <unsigned int _Size>char* MakeTARGT(char(&Buffer)[_Size]) {
	if (pTarget) {
		sprintf_s(Buffer, "%d:%d", pTarget->SpawnID, static_cast<int32_t>(pTarget->HPMax == 0 ? 0 : pTarget->HPCurrent * 100 / pTarget->HPMax));
	} else {
		strcpy_s(Buffer, ":");
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeZONES(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%d:%d>%d", pLocalPC->zoneId, pLocalPC->instance, pLocalPlayer->SpawnID);
	return Buffer;
}

template <unsigned int _Size>char* MakeDETR(char(&Buffer)[_Size]) {
	char szTemp[MAX_STRING];
	Buffer[0] = 0;
	for (int i = 0; i < DSIZE; i++) {
		sprintf_s(szTemp, "%lld:", dValues[i]);
		strcat_s(Buffer, szTemp);
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeFREEI(char(&Buffer)[_Size]) {
	char szTemp[MAX_STRING];
	Buffer[0] = 0;
	int freeSlots[ISIZE] = { 0 };
	int slotMax = ISIZE - 1;
	for (int slot = InvSlot_FirstBagSlot; slot <= GetHighestAvailableBagSlot(); slot++) {
		if (ItemPtr pItem = GetPcProfile()->InventoryContainer.GetItem(slot)) {
			if (pItem->IsContainer()) {
				int iSize = std::clamp((int)pItem->GetItemDefinition()->SizeCapacity, 0, slotMax);
				freeSlots[iSize] += pItem->GetHeldItems().GetSize() - pItem->GetHeldItems().GetCount();
			}
		} else {
			freeSlots[slotMax]++;
		}
	}
	for (int slot = slotMax; slot > 0; slot--) {
		freeSlots[slot - 1] += freeSlots[slot];
	}
	for (int i = 0; i < ISIZE; i++) {
		sprintf_s(szTemp, "%d:", freeSlots[i]);
		strcat_s(Buffer, szTemp);
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeVERSN(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%.2f", MQ2Version);
	return Buffer;
}

template <unsigned int _Size>char* MakeEXTEND(char(&Buffer)[_Size]) {
	sprintf_s(Buffer, "%d", NetExtended);
	return Buffer;
}

template <unsigned int _Size>char* MakeCAMPS(char(&Buffer)[_Size]) {
	char szTemp[MAX_STRING];
	if (IsPluginLoaded("MQ2MoveUtils")) {
		strcpy_s(szTemp, "${Select[${MakeCamp.Status},ON,PAUSED]}:${MakeCamp.AnchorX}:${MakeCamp.AnchorY}:${MakeCamp.CampRadius}:${MakeCamp.CampDist}");
		ParseMacroData(szTemp, sizeof(szTemp));
		strcpy_s(Buffer, szTemp);
	} else {
		strcpy_s(Buffer, "0:0:0:0:0");
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeLUA(char(&Buffer)[_Size]) {
	char szTemp[MAX_STRING] = { 0 };
	char szScriptName[MAX_STRING] = { 0 };
	char* token = NULL;
	char* next_token = NULL;
	Buffer[0] = '\0';
	if (IsPluginLoaded("Lua")) {
		strcpy_s(szTemp, "${Lua.PIDs}");
		ParseMacroData(szTemp, sizeof(szTemp));
		token = strtok_s(szTemp, ",", &next_token);
		while (token != NULL) {
			sprintf_s(szScriptName, "${Lua.Script[%s].Name}", token);
			ParseMacroData(szScriptName, sizeof(szScriptName));
			if (szScriptName[0] && _stricmp(szScriptName, "NULL")) {
				if (strlen(Buffer)) {
					strcat_s(Buffer, ",");
				}
				strcat_s(Buffer, szScriptName);
			}
			token = strtok_s(NULL, ",", &next_token);
		}
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeEQBC(char(&Buffer)[_Size]) {
	char szTemp[MAX_STRING];
	if (NetEQBCData && IsPluginLoaded("MQ2EQBC")) {
		strcpy_s(szTemp, "${EQBC.Packets}:${EQBC.HeartBeat}");
		ParseMacroData(szTemp, sizeof(szTemp));
		strcpy_s(Buffer, szTemp);
	} else {
		strcpy_s(Buffer, "0:0");
	}
	return Buffer;
}

template <unsigned int _Size>char* MakeQUERY(char(&Buffer)[_Size]) {
	char szTemp[MAX_STRING] = { 0 };
	strcpy_s(szTemp, NetQuery);
	ParseMacroData(szTemp, sizeof(szTemp));
	strcpy_s(Buffer, szTemp);
	return Buffer;
}

int SlotCalculate(EQ_Spell* spell, int slot) {
	char szTemp[MAX_STRING] = { 0 };
	SlotValueCalculate(szTemp, spell, slot, 1);
	return GetIntFromString(szTemp, 0);
}

unsigned int SpellClassMask(std::initializer_list<int> classlist) {
	unsigned int mask = 0;
	for (int num : classlist) {
		mask |= (1 << num);
	}
	return mask;
}

void EvalDetrimental(EQ_Spell* spell) {
	bool detr = false;
	bool counter = false;
	int subcat = GetSpellSubcategory(spell);
	int attrib = 0;
	int64_t base = 0;
	for (int s = 0; s < GetSpellNumEffects(spell); s++) {
		attrib = GetSpellAttrib(spell, s);
		base   = GetSpellBase(spell, s);
		switch (attrib) {
			case SPA_CHA:
				break;
			case SPA_HP:
				if (base < 0) {
					detrStatus |= BD_LIFEDRAIN;
					dValues[D_LIFEDRAIN] += SlotCalculate(spell, s);
					detr = true;
				}
				break;
			case SPA_MOVEMENT_RATE:
				if (base < 0) {
					detrStatus |= BD_SNARED;
					detr = true;
				}
				break;
			case SPA_HASTE:
				if (base < 100) {
					detrStatus |= BD_SLOWED;
					detr = true;
				}
				break;
			case SPA_MANA:
				if (base < 0) {
					detrStatus |= BD_MANADRAIN;
					dValues[D_MANADRAIN] += SlotCalculate(spell, s);
					detr = true;
				}
				break;
			case SPA_BLINDNESS:
				detrStatus |= BD_BLINDED;
				detr = true;
				break;
			case SPA_CHARM:
				detrStatus |= BD_CHARMED;
				detr = true;
				break;
			case SPA_FEAR:
				detrStatus |= BD_FEARED;
				detr = true;
				break;
			case SPA_ENTHRALL:
				detrStatus |= BD_MESMERIZED;
				detr = true;
				break;
			case SPA_DISEASE:
				detrStatus |= BD_DISEASED;
				detr = true;
				counter = true;
				break;	
			case SPA_POISON:
				detrStatus |= BD_POISONED;
				detr = true;
				counter = true;
				break;
			case SPA_INVULNERABILITY:
				detrStatus |= BD_INVULNERABLE;
				detr = true;
				break;
			case SPA_RESIST_FIRE:
			case SPA_RESIST_COLD:
			case SPA_RESIST_POISON:
			case SPA_RESIST_DISEASE:
			case SPA_RESIST_MAGIC:
			case SPA_RESIST_CORRUPTION:
			case SPA_RESIST_ALL:
				if (base < 0) {
					detrStatus |= BD_RESISTANCE;
					detr = true;
				}
				break;
			case SPA_SILENCE:
				detrStatus |= BD_SILENCED;
				detr = true;
				break;
			case SPA_ROOT:
				detrStatus |= BD_ROOTED;
				detr = true;
				break;
			case SPA_FIZZLE_SKILL:
				if (base < 0) {
					detrStatus |= BD_CASTINGLEVEL;
					detr = true;
				}
				break;
			case SPA_CURSE:
				detrStatus |= BD_CURSED;
				detr = true;
				counter = true;
				break;
			case SPA_HEALMOD:
				if (base < 0) {
					detrStatus |= BD_HEALING;
					detr = true;
				}
				break;
			case SPA_IRONMAIDEN:
				if (base < 0) {
					detrStatus |= BD_REVDSED;
					detr = true;
				}
				break;
			case SPA_FOCUS_DAMAGE_MOD:
				if (base < 0) {
					detrStatus |= BD_SPELLDAMAGE;
					detr = true;
				}
				break;
			case SPA_FOCUS_CAST_TIME_MOD:
				if (base < 0) {
					detrStatus |= BD_SPELLSLOWED;
					detr = true;
				}
				break;
			case SPA_FOCUS_MANACOST_MOD:
				if (base < 0) {
					detrStatus |= BD_SPELLCOST;
					detr = true;
				}
				break;
			case SPA_ENDURANCE:
				if (base < 0) {
					detrStatus |= BD_ENDUDRAIN;
					dValues[D_ENDUDRAIN] += SlotCalculate(spell, s);
					detr = true;
				}
				break;
			case SPA_DOOM_EFFECT:
				detrStatus |= BD_TRIGGER;
				detr = true;
				break;
			case SPA_CORRUPTION:
				detrStatus |= BD_CORRUPTED;
				detr = true;
				counter = true;
				break;
			default:
				detr = true;
				break;
		}
	}
	switch (subcat) {
		case SPELLCAT_DISEMPOWERING:
			detrStatus |= BD_CRIPPLED;
			detr = true;
			break;
		case SPELLCAT_RESIST_DEBUFFS:
			if (IsSpellUsableForClass(spell, SpellClassMask({ Shaman, Mage }))) {
				detrStatus |= BD_MALOED;
				detr = true;
			} else if (IsSpellUsableForClass(spell, SpellClassMask({ Enchanter }))) {
				detrStatus |= BD_TASHED;
				detr = true;
			}
			break;
		default:
			break;
	}
	if (detr) {
		dValues[D_DETRIMENTALS]++;
		if ((spell->IsNoDispell() && !counter) || spell->TargetType == TargetType_Self) {
			dValues[D_NOCURE]++;
		}
	}
}

void EvalBeneficial(EQ_Spell* spell) {
	int cat = GetSpellCategory(spell);
	int subcat = GetSpellSubcategory(spell);
	int attrib = 0;
	int64_t base = 0;
	for (int s = 0; s < GetSpellNumEffects(spell); s++) {
		attrib = GetSpellAttrib(spell, s);
		base   = GetSpellBase(spell, s);
		switch (attrib) {
			case SPA_CHA:
				break;
			case SPA_HP:
				if (base > 0) {
					beneStatus |= BB_REGEN;
				}
				break;
			case SPA_HASTE:
				if (base > 100) {
					beneStatus |= BB_HASTED;
				}
				break;
			case SPA_MANA:
				if (base > 0) {
					if (IsSpellUsableForClass(spell, SpellClassMask({ Enchanter }))) {
						beneStatus |= BB_CLARITY;
					}
				}
				break;
			case SPA_DAMAGE_SHIELD:
				if (base < 0) {
					beneStatus |= BB_DSED;
				}
				break;
			case SPA_AC:
				if (base > 0) {
					if (cat == SPELLCAT_HP_BUFFS &&
						subcat == SPELLCAT_AEGOLISM &&
						IsSpellUsableForClass(spell, SpellClassMask({ Cleric }))) {
						beneStatus |= BB_AEGO;
					}
				}
				break;
			case SPA_MELEE_GUARD:
				if (cat == SPELLCAT_UTILITY_BENEFICIAL &&
					subcat == SPELLCAT_MELEE_GUARD &&
					IsSpellUsableForClass(spell, SpellClassMask({ Cleric }))) {
					beneStatus |= BB_SHINING;
				}
				break;
			case SPA_INVISIBILITY:
			case SPA_IMPROVED_INVIS:
				invisStatus |= INVIS_NORMAL;
				break;
			case SPA_INVIS_VS_UNDEAD:
			case SPA_IMPROVED_INVIS_UNDEAD:
				invisStatus |= INVIS_UNDEAD;
				break;
			default:
				break;
		}
	}
	switch (cat) {
		case SPELLCAT_HP_BUFFS:
			switch (subcat) {
				case SPELLCAT_HP_TYPE_ONE:
					if (IsSpellUsableForClass(spell,SpellClassMask({ Druid }))) {
						beneStatus |= BB_SKIN;
					} else if (IsSpellUsableForClass(spell, SpellClassMask({ Ranger }))) {
						beneStatus |= BB_HYBRIDHP;
					}
					break;
				case SPELLCAT_HP_TYPE_TWO:
					if (IsSpellUsableForClass(spell, SpellClassMask({ Ranger }))) {
						beneStatus |= BB_STRENGTH;
					} else if (IsSpellUsableForClass(spell, SpellClassMask({ Paladin }))) {
						beneStatus |= BB_BRELLS;
					} else if (IsSpellUsableForClass(spell, SpellClassMask({ Beastlord }))) {
						beneStatus |= BB_SV;
					}
					break;
				case SPELLCAT_TEMPORARY:
					if (IsSpellUsableForClass(spell, SpellClassMask({ Druid }))) {
						beneStatus |= BB_GROWTH;
					}
					break;
				case SPELLCAT_SYMBOL:
					if (IsSpellUsableForClass(spell, SpellClassMask({ Cleric }))) {
						beneStatus |= BB_SYMBOL;
					}
					break;
				case SPELLCAT_SHIELDING:
					if (IsSpellUsableForClass(spell, SpellClassMask({ Shaman }))) {
						beneStatus |= BB_FOCUS;
					}
					break;
				default:
					break;
			}
			break;
		case SPELLCAT_STATISTIC_BUFFS:
			if (subcat == SPELLCAT_ATTACK && IsSpellUsableForClass(spell, SpellClassMask({ Ranger }))) {
				beneStatus |= BB_PRED;
			}
			break;
		case SPELLCAT_REGEN:
			if (subcat == SPELLCAT_HEALTH_MANA && IsSpellUsableForClass(spell, SpellClassMask({ Beastlord }))) {
				beneStatus |= BB_SE;
			}
			break;
		case SPELLCAT_UTILITY_BENEFICIAL:
			if (subcat == SPELLCAT_MELEE_GUARD && IsSpellUsableForClass(spell, SpellClassMask({ Ranger }))) {
				beneStatus |= BB_PRED;
			}
			break;
		default:
			break;
	}
}

void ScanBuffs() {
	char szTemp[MAX_STRING] = { 0 };
	bBuffs[0] = '\0';
	bSongs[0] = '\0';
	bBuffd[0] = '\0';	
	bSongd[0] = '\0';
	ZeroMemory(&dValues, sizeof(dValues));
	AvailBuffSlots = GetCharMaxBuffSlots();
	detrStatus = 0;
	beneStatus = 0;
	invisStatus = 0;
	for (int i = 0; i < NUM_LONG_BUFFS; i++) {
		if (auto spell = GetSpellByID(GetPcProfile()->GetEffect(i).SpellID)) {
			if (spell->ID > 0) {
				sprintf_s(szTemp, "%d:", spell->ID);
				strcat_s(bBuffs, szTemp);
				AvailBuffSlots--;
				if (NetExtended >= EX_GEMS_AND_BUFF_DURATIONS) {
					int t = GetSpellBuffTimer(spell->ID);
					// Send as Ticks (to help reduce EQBC spam). Negative is 'permanent', show as 9999.
					sprintf_s(szTemp, "%d:", static_cast<int32_t>(t < 0 ? 9999 : ((t / 1000) + 5) / 6));
					strcat_s(bBuffd, szTemp);
				}
				if (spell->SpellType == SpellType_Detrimental) {
					EvalDetrimental(spell);
				} else {
					EvalBeneficial(spell);
				}
			}
		}
	}
	for (int i = 0; i < NUM_SHORT_BUFFS; i++) {
		if (auto spell = GetSpellByID(GetPcProfile()->GetTempEffect(i).SpellID)) {
			if (spell->ID > 0) {
				sprintf_s(szTemp, "%d:", spell->ID);
				strcat_s(bSongs, szTemp);
				if (NetExtended >= EX_SONG_DURATIONS) {
					int t = GetSpellBuffTimer(spell->ID);
					// Send as Ticks (to help reduce EQBC spam). Negative is 'permanent', show as 9999.
					sprintf_s(szTemp, "%d:", static_cast<int32_t>(t < 0 ? 9999 : ((t / 1000) + 5) / 6));
					strcat_s(bSongd, szTemp);
				}
				if(spell->SpellType == SpellType_Detrimental) {
					EvalDetrimental(spell);
				} else {
					EvalBeneficial(spell);
				}
			}
		}
	}
	if (detrStatus & BD_DISEASED) {
		dValues[D_DISEASED] += GetMySpellCounters(SPA_DISEASE);
		dValues[D_COUNTERS] += dValues[D_DISEASED];
	}
	if (detrStatus & BD_POISONED) {
		dValues[D_POISONED] += GetMySpellCounters(SPA_POISON);
		dValues[D_COUNTERS] += dValues[D_POISONED];
	}
	if (detrStatus & BD_CURSED) {
		dValues[D_CURSED] += GetMySpellCounters(SPA_CURSE);
		dValues[D_COUNTERS] += dValues[D_CURSED];
	}
	if (detrStatus & BD_CORRUPTED) {
		dValues[D_CORRUPTED] += GetMySpellCounters(SPA_CORRUPTION);
		dValues[D_COUNTERS] += dValues[D_CORRUPTED];
	}
}

void ScanPetBuffs() {
	char szTemp[MAX_STRING] = { 0 };
	bPBuffs[0] = '\0';
	bPBuffd[0] = '\0';	
	auto Pet = GetSpawnByID(pLocalPlayer->PetID);
	if (Pet && pPetInfoWnd)	{
		for (int i = 0; i < PETS_MAX; i++) {
			if (auto spell = GetSpellByID(pPetInfoWnd->GetBuff(i))) {
				if (spell->ID > 0) {
					sprintf_s(szTemp, "%d:", spell->ID);
					strcat_s(bPBuffs, szTemp);
					if (NetExtended >= EX_PET_DURATIONS) {
						int t = pPetInfoWnd->GetBuffTimer(i);
						// Send as Ticks (to help reduce EQBC spam). Negative is 'permanent', show as 9999.
						sprintf_s(szTemp, "%d:", static_cast<int32_t>(t < 0 ? 9999 : ((t / 1000) + 5) / 6));
						strcat_s(bPBuffd, szTemp);
					}
				}
			}
		}
	}
}

bool SpellHasSPA(EQ_Spell* pSpell, eEQSPA eSPA, bool bIncrease) {
	if (!pSpell) {
		return false;
	}
	const SpellAffectData* spellAffect = nullptr;
	for (int index = 0; index < pSpell->GetNumEffects(); ++index) {
		if (const SpellAffectData* sad = pSpell->GetSpellAffectByIndex(index)) {
			if (sad->Attrib == eSPA) {
				spellAffect = sad;
				break;
			}
		}
	}
	if (spellAffect == nullptr) {
		return false;
	}
	auto base = spellAffect->Base;
	switch (eSPA) {
		case SPA_HP: 			// HP regen or drain/DoT. Below 0 is a drain/DoT or lich-like spell.
		case SPA_MANA: 			// Mana regen or drain. Below 0 is a drain.
		case SPA_MOVEMENT_RATE: // Movement Rate. Below 0 is a snare above is runspeed increase.
		case SPA_FIZZLE_SKILL:
		case SPA_ENDURANCE:  	// Endurance regen or drain. Below 0 is a drain.
		case SPA_HEALMOD:
		case SPA_FOCUS_MANACOST_MOD:
		case SPA_FOCUS_DAMAGE_MOD:
		case SPA_FOCUS_CAST_TIME_MOD:
		case SPA_AC:
			return (!bIncrease && base < 0) || (bIncrease && base > 0);
		case SPA_HASTE: 		// Melee Speed. Below 100 is a slow, above is haste.
			return (!bIncrease && base < 100) || (bIncrease && base > 100);
		case SPA_DAMAGE_SHIELD: // Damage Shield
			return (!bIncrease && base > 0) || (bIncrease && base < 0);
		case SPA_IRONMAIDEN: 	// Reverse Damage Shield
			return (!bIncrease && base > 0) || (bIncrease && base < 0);
		default:				// Has the SPA
			return true;
	}
}

bool CheckSpellValues(EQ_Spell* pSpell, int spellSPA, bool bIncrease, int spellCategory, int spellSubcategory, int spellMask, bool hasMask = true) {
	if (spellSPA >= 0 && !SpellHasSPA(pSpell, static_cast<eEQSPA>(spellSPA), bIncrease)) {
		return false;
	}
	if (spellCategory && GetSpellCategory(pSpell) != spellCategory) {
		return false;
	}
	if (spellSubcategory && GetSpellSubcategory(pSpell) != spellSubcategory) {
		return false;
	}
	return (!spellMask || (hasMask == IsSpellUsableForClass(pSpell, spellMask)));
}

bool SpellHasAnyResistSPA(EQ_Spell* pSpell, bool bIncrease) {
	if (!pSpell) {
		return false;
	}
	const SpellAffectData* spellAffect = nullptr;
	for (int index = 0; index < pSpell->GetNumEffects(); ++index) {
		if (const SpellAffectData* sad = pSpell->GetSpellAffectByIndex(index)) {
			if (sad->Attrib == SPA_RESIST_FIRE ||
				sad->Attrib == SPA_RESIST_COLD ||
				sad->Attrib == SPA_RESIST_POISON ||
				sad->Attrib == SPA_RESIST_DISEASE ||
				sad->Attrib == SPA_RESIST_MAGIC ||
				sad->Attrib == SPA_RESIST_CORRUPTION ||
				sad->Attrib == SPA_RESIST_ALL) {
				spellAffect = sad;
				auto base = spellAffect->Base;
				if ((!bIncrease && base < 0) || (bIncrease && base > 0)) {
					return true;
				}
			}
		}
	}
	return false;
}
	
bool CheckBotSpell(EQ_Spell* pSpell, int spellType) {
	bool result = false;
	switch (spellType) {
		case ST_SLOWED:
			result = CheckSpellValues(pSpell, SPA_HASTE, false, 0, 0, 0);
			break;
		case ST_ROOTED:
			result = CheckSpellValues(pSpell, SPA_ROOT, true, 0, 0, 0);
			break;
		case ST_MESMERIZED:
			result = CheckSpellValues(pSpell, SPA_ENTHRALL, true, 0, 0, 0);
			break;
		case ST_CRIPPLED:
			result = CheckSpellValues(pSpell, -1, true, 0, SPELLCAT_DISEMPOWERING, 0);
			break;
		case ST_MALOED:
			result = CheckSpellValues(pSpell, -1, true, 0, SPELLCAT_RESIST_DEBUFFS, SpellClassMask({ Shaman, Mage }));
			break;
		case ST_TASHED:
			result = CheckSpellValues(pSpell, -1, true, 0, SPELLCAT_RESIST_DEBUFFS, SpellClassMask({ Enchanter }));
			break;
		case ST_SNARED:
			result = CheckSpellValues(pSpell, SPA_MOVEMENT_RATE, false, 0, 0, 0);
			break;
		case ST_REVDSED:
			result = CheckSpellValues(pSpell, SPA_IRONMAIDEN, true, 0, 0, 0);
			break;
		case ST_CHARMED:
			result = CheckSpellValues(pSpell, SPA_CHARM, true, 0, 0, 0);
			break;
		case ST_DISEASED:
			result = CheckSpellValues(pSpell, SPA_DISEASE, true, 0, 0, 0);
			break;
		case ST_POISONED:
			result = CheckSpellValues(pSpell, SPA_POISON, true, 0, 0, 0);
			break;
		case ST_CURSED:
			result = CheckSpellValues(pSpell, SPA_CURSE, true, 0, 0, 0);
			break;
		case ST_CORRUPTED:
			result = CheckSpellValues(pSpell, SPA_CORRUPTION, true, 0, 0, 0);
			break;
		case ST_BLINDED:
			result = CheckSpellValues(pSpell, SPA_BLINDNESS, true, 0, 0, 0);
			break;
		case ST_CASTINGLEVEL:
			result = CheckSpellValues(pSpell, SPA_FIZZLE_SKILL, false, 0, 0, 0);
			break;			
		case ST_ENDUDRAIN:
			result = CheckSpellValues(pSpell, SPA_ENDURANCE, false, 0, 0, 0);
			break;
		case ST_FEARED:
			result = CheckSpellValues(pSpell, SPA_FEAR, true, 0, 0, 0);
			break;
		case ST_HEALING:
			result = CheckSpellValues(pSpell, SPA_HEALMOD, false, 0, 0, 0);
			break;
		case ST_INVULNERABLE:
			result = CheckSpellValues(pSpell, SPA_INVULNERABILITY, true, 0, 0, 0);
			break;
		case ST_LIFEDRAIN:
			result = CheckSpellValues(pSpell, SPA_HP, false, 0, 0, 0);
			break;
		case ST_MANADRAIN:
			result = CheckSpellValues(pSpell, SPA_MANA, false, 0, 0, 0);
			break;
		case ST_RESISTANCE:
			result = SpellHasAnyResistSPA(pSpell, false);
			break;
		case ST_SILENCED:
			result = CheckSpellValues(pSpell, SPA_SILENCE, true, 0, 0, 0);
			break;
		case ST_SPELLCOST:
			result = CheckSpellValues(pSpell, SPA_FOCUS_MANACOST_MOD, false, 0, 0, 0);
			break;
		case ST_SPELLDAMAGE:
			result = CheckSpellValues(pSpell, SPA_FOCUS_DAMAGE_MOD, false, 0, 0, 0);
			break;
		case ST_SPELLSLOWED:
			result = CheckSpellValues(pSpell, SPA_FOCUS_CAST_TIME_MOD, false, 0, 0, 0);
			break;
		case ST_TRIGGER:
			result = CheckSpellValues(pSpell, SPA_DOOM_EFFECT, true, 0, 0, 0);
			break;
		case ST_DSED:
			result = CheckSpellValues(pSpell, SPA_DAMAGE_SHIELD, true, 0, 0, 0);
			break;
		case ST_AEGO:
			result = CheckSpellValues(pSpell, SPA_AC, true, SPELLCAT_HP_BUFFS, SPELLCAT_AEGOLISM, SpellClassMask({ Cleric }));
			break;
		case ST_SKIN:
			result = CheckSpellValues(pSpell, -1, true, SPELLCAT_HP_BUFFS, SPELLCAT_HP_TYPE_ONE, SpellClassMask({ Druid }));
			break;
		case ST_FOCUS:
			result = CheckSpellValues(pSpell, -1, true, SPELLCAT_HP_BUFFS, SPELLCAT_SHIELDING, SpellClassMask({ Shaman }));
			break;
		case ST_REGEN:
			result = CheckSpellValues(pSpell, SPA_HP, true, 0, 0, 0);
			break;
		case ST_SYMBOL:
			result = CheckSpellValues(pSpell, -1, true, SPELLCAT_HP_BUFFS, SPELLCAT_SYMBOL, SpellClassMask({ Cleric }));
			break;
		case ST_CLARITY:
			result = CheckSpellValues(pSpell, SPA_MANA, true, 0, 0, SpellClassMask({ Enchanter }));
			break;
		case ST_PRED:
			result = CheckSpellValues(pSpell, -1, true, SPELLCAT_STATISTIC_BUFFS, SPELLCAT_ATTACK, SpellClassMask({ Ranger }));
			break;
		case ST_STRENGTH:
			result = CheckSpellValues(pSpell, -1, true, SPELLCAT_HP_BUFFS, SPELLCAT_HP_TYPE_TWO, SpellClassMask({ Ranger }));
			break;
		case ST_BRELLS:
			result = CheckSpellValues(pSpell, -1, true, SPELLCAT_HP_BUFFS, SPELLCAT_HP_TYPE_TWO, SpellClassMask({ Paladin }));
			break;
		case ST_SV:
			result = CheckSpellValues(pSpell, -1, true, SPELLCAT_HP_BUFFS, SPELLCAT_HP_TYPE_TWO, SpellClassMask({ Beastlord }));
			break;
		case ST_SE:
			result = CheckSpellValues(pSpell, -1, true, SPELLCAT_REGEN, SPELLCAT_HEALTH_MANA, SpellClassMask({ Beastlord }));
			break;
		case ST_HYBRIDHP:
			result = CheckSpellValues(pSpell, -1, true, SPELLCAT_HP_BUFFS, SPELLCAT_HP_TYPE_ONE, SpellClassMask({ Ranger }));
			break;
		case ST_GROWTH:
			result = CheckSpellValues(pSpell, -1, true, SPELLCAT_HP_BUFFS, SPELLCAT_TEMPORARY, SpellClassMask({ Druid }));
			break;
		case ST_SHINING:
			result = CheckSpellValues(pSpell, SPA_MELEE_GUARD, true, SPELLCAT_UTILITY_BENEFICIAL, SPELLCAT_MELEE_GUARD, SpellClassMask({ Cleric }));
			break;
		case ST_HASTED:
			result = CheckSpellValues(pSpell, SPA_HASTE, true, 0, 0, 0);
			break;
		default:
			break;
	}
	return result;
}

void BroadCast() {
	char Buffer[MAX_STRING];
	int nChange = 0;
	ZeroMemory(wBuffer, sizeof(wBuffer));
	ZeroMemory(wChange, sizeof(wChange));
	ZeroMemory(wUpdate, sizeof(wUpdate));
	
	ScanBuffs();
	ScanPetBuffs();
	
	sprintf_s(wBuffer[E_BUFFS],   "B=%s|", MakeBUFFS(Buffer));	// Buff
	sprintf_s(wBuffer[E_CASTD],   "C=%s|", MakeCASTD(Buffer));	// CastID
	sprintf_s(wBuffer[E_ENDUS],   "E=%s|", MakeENDUS(Buffer));	// EnduranceCurrent, EnduranceMax
	sprintf_s(wBuffer[E_EXPER],   "X=%s|", MakeEXPER(Buffer));	// Exp, AAExp, (GroupLeaderExp)
	sprintf_s(wBuffer[E_LEADR],   "N=%s|", MakeLEADR(Buffer));	// Leader
	sprintf_s(wBuffer[E_LEVEL],   "L=%s|", MakeLEVEL(Buffer));	// Level, ClassID
	sprintf_s(wBuffer[E_HPS],     "H=%s|", MakeHPS(Buffer));	// HPCurrent, HPMax
	sprintf_s(wBuffer[E_MANAS],   "M=%s|", MakeMANAS(Buffer));	// ManaCurrent, ManaMax		
	sprintf_s(wBuffer[E_PBUFF],   "W=%s|", MakePBUFF(Buffer));	// Pets
	sprintf_s(wBuffer[E_PETIL],   "P=%s|", MakePETIL(Buffer));	// PetID, PetHPPct
	sprintf_s(wBuffer[E_SONGS],   "S=%s|", MakeSONGS(Buffer));	// Song
	sprintf_s(wBuffer[E_STATE],   "Y=%s|", MakeSTATE(Buffer));	// State
	sprintf_s(wBuffer[E_TARGT],   "T=%s|", MakeTARGT(Buffer));	// TargetID, TargetHPPct
	sprintf_s(wBuffer[E_ZONES],   "Z=%s|", MakeZONES(Buffer));	// ZoneID, InstanceID, SpawnID
	sprintf_s(wBuffer[E_AAPTS],   "A=%s|", MakeAAPTS(Buffer));	// AAPoints, AAPointsSpent, AAPointsAssigned
	sprintf_s(wBuffer[E_OOCST],   "O=%s|", MakeOOCST(Buffer));	// CombatState
	sprintf_s(wBuffer[E_NOTE],    "U=%s|", MakeNOTE(Buffer));	// Note
	sprintf_s(wBuffer[E_DETR],    "R=%s|", MakeDETR(Buffer));	// Detrimental
	sprintf_s(wBuffer[E_LOCAT],   "@=%s|", MakeLOCAT(Buffer));	// Location (and X,Y,Z derived)
	sprintf_s(wBuffer[E_HEADN],   "$=%s|", MakeHEADN(Buffer));	// Heading
	sprintf_s(wBuffer[E_FREEB],   "F=%s|", MakeFREEB(Buffer));	// FreeBuff
	sprintf_s(wBuffer[E_EXTEND],  "<=%s|", MakeEXTEND(Buffer));	// Extended
	sprintf_s(wBuffer[E_MACRO],   "&=%s|", MakeMACRO(Buffer));	// MacroState, MacroName
	sprintf_s(wBuffer[E_FREEI],   "I=%s|", MakeFREEI(Buffer));	// FreeInventory
	sprintf_s(wBuffer[E_VERSN],   "V=%s|", MakeVERSN(Buffer));	// Version
	sprintf_s(wBuffer[E_CAMPS],   "K=%s|", MakeCAMPS(Buffer));	// MakeCampStatus, MakeCampX, MakeCampY, MakeCampRadius, MakeCampDistance
	sprintf_s(wBuffer[E_LUAINFO], "+=%s|", MakeLUA(Buffer));	// LuaInfo
	sprintf_s(wBuffer[E_EQBC],    "-=%s|", MakeEQBC(Buffer));	// EQBC_Packets, EQBC_HeartBeat
	sprintf_s(wBuffer[E_QUERY],   "Q=%s|", MakeQUERY(Buffer));	// Query
	sprintf_s(wBuffer[E_BSTAT],   "J=%s|", MakeBSTAT(Buffer));	// DetrState, BeneState
    sprintf_s(wBuffer[E_SPGEM],   "G=%s|", MakeSPGEM(Buffer));	// Gem
    sprintf_s(wBuffer[E_BUFFD],   "D=%s|", MakeBUFFD(Buffer));	// BDuration
	sprintf_s(wBuffer[E_SONGD],   ":=%s|", MakeSONGD(Buffer));	// SDuration
	sprintf_s(wBuffer[E_PETD],    ";=%s|", MakePETD(Buffer));	// PDuration

	for (int i = 0; i < ESIZE; i++) {
		if ((clock() > sTimers[i] + UPDATES) || strcmp(wBuffer[i],sBuffer[i]) != 0) {
			wChange[i] = true;
			nChange++;
		} else if (clock() < sTimers[i] && clock() + UPDATES > sTimers[i]) {
			wUpdate[i] = true;
		}
	}
	if (nChange) {
		strcpy_s(Buffer, "[NB]|");
		for (int i = 0; i < ESIZE; i++) {
			if (wChange[i] || wUpdate[i]) {
				if (strlen(Buffer) + strlen(wBuffer[i]) < MAX_STRING - 5) {
					strcat_s(Buffer, wBuffer[i]);
					sTimers[i] = (int)clock() + REFRESH;
				}
//				else WriteChatf("Skipping i=%d buffer=%d wbuffer=%d", i, strlen(Buffer), strlen(wBuffer[i]));
			}
		}
		strcat_s(Buffer, "[NB]");
		
//		WriteChatf("Broadcast %s", Buffer);

		EQBCBroadCast(Buffer);
		memcpy(sBuffer, wBuffer, sizeof(wBuffer));
		if (CurBot = BotLoad(pLocalPC->Name).get()) {
			Packet.Feed(Buffer);
			CurBot->Updated = clock();
			CurBot = 0;
		}
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

class MQ2NetBotsType *pNetBotsType = 0;
class MQ2NetBotsType : public MQ2Type {

public:
	enum Information {
		Enable = 1,
		Listen,
		Output,
		Counts,
		Client,
		Name,
		Zone,
		Instance,
		ID,
		Class,
		Level,
		PctExp,
		PctAAExp,
		PctGroupLeaderExp,
		CurrentHPs,
		MaxHPs,
		PctHPs,
		CurrentMana,
		MaxMana,
		PctMana,
		CurrentEndurance,
		MaxEndurance,
		PctEndurance,
		PetID,
		PetHP,
		PetPctHPs,
		TargetID,
		TargetHP,
		TargetPctHPs,
		Casting,
		State,
		Attacking,
		AFK,
		Binding,
		Ducking,
		Feigning,
		Grouped,
		Invis,
		Levitating,
		LFG,
		Mounted,
		Moving,
		Raid,
		Sitting,
		Standing,
		Stunned,
		Dead,
		Hover,
		InvisToUndead,
		AutoFire,
		HaveAggro,
		WantAggro,
		NavigationActive,
		NavigationPaused,
		BotActive,
		PetAffinity,
		Extended,
		FreeBuffSlots,
		InZone,
		InGroup,
		Leader,
		GroupLeader,
		Note,
		Location,
		X,
		Y,
		Z,
		Distance,
		Heading,
		Updated,
		Gem,
		Buff,
		Duration,
		BuffDuration,
		ShortBuff,
		ShortDuration,
		PetBuff,
		PetDuration,
		TotalAA,
		AAPointsTotal,
		UsedAA,
		AAPointsSpent,
		UnusedAA,
		AAPoints,
		AAPointsAssigned,
		CombatState,
		Stacks,
		StacksPet,
		WillLand,
		WillLandPet,
		TooPowerful,
		TooPowerfulPet,
		Detrimentals,
		Counters,
		TotalCounters,
		CountersCurse,
		CountersDisease,
		CountersPoison,
		CountersCorruption,
		NoCure,
		LifeDrain,
		ManaDrain,
		EnduDrain,
		Cursed,
		Diseased,
		Poisoned,
		Corrupted,
		Blinded,
		CastingLevel,
		Charmed,
		Feared,
		Healing,
		Invulnerable,
		Mesmerized,
		Rooted,
		Silenced,
		Slowed,
		Snared,
		SpellCost,
		SpellSlowed,
		SpellDamage,
		Trigger,
		Resistance,
		Detrimental,
		RevDSed,
		MacroState,
		MacroName,
		FreeInventory,
		Version,
		MakeCampStatus,
		MakeCampX,
		MakeCampY,
		MakeCampRadius,
		MakeCampDistance,
		Lua,
		Packets,
		HeartBeat,
		Query,
		Crippled,
		Maloed,
		Tashed,
		Hasted,
		DSed,
		Aego,
		Skin,
		Focus,
		Regen,
		Symbol,
		Clarity,
		Pred,
		Strength,
		Brells,
		SV,
		SE,
		HybridHP,
		Growth,
		Shining,
		Spell
	};

	MQ2NetBotsType() :MQ2Type("NetBots") {
		TypeMember(Enable);
		TypeMember(Listen);
		TypeMember(Output);
		TypeMember(Counts);
		TypeMember(Client);
		TypeMember(Name);
		TypeMember(Zone);
		TypeMember(Instance);
		TypeMember(ID);
		TypeMember(Class);
		TypeMember(Level);
		TypeMember(PctExp);
		TypeMember(PctAAExp);
		TypeMember(PctGroupLeaderExp);
		TypeMember(CurrentHPs);
		TypeMember(MaxHPs);
		TypeMember(PctHPs);
		TypeMember(CurrentMana);
		TypeMember(MaxMana);
		TypeMember(PctMana);
		TypeMember(CurrentEndurance);
		TypeMember(MaxEndurance);
		TypeMember(PctEndurance);
		TypeMember(PetID);
		TypeMember(PetHP);
		TypeMember(PetPctHPs);		// Same as PetHP
		TypeMember(TargetID);
		TypeMember(TargetHP);
		TypeMember(TargetPctHPs);	// Same as TargetHP
		TypeMember(Casting);
		TypeMember(State);
		TypeMember(Attacking);
		TypeMember(AFK);
		TypeMember(Binding);
		TypeMember(Ducking);
		TypeMember(Feigning);
		TypeMember(Grouped);
		TypeMember(Invis);
		TypeMember(Levitating);
		TypeMember(LFG);
		TypeMember(Mounted);
		TypeMember(Moving);
		TypeMember(Raid);
		TypeMember(Sitting);
		TypeMember(Standing);
		TypeMember(Stunned);
		TypeMember(Dead);
		TypeMember(Hover);
		TypeMember(InvisToUndead);
		TypeMember(AutoFire);
		TypeMember(HaveAggro);
		TypeMember(WantAggro);
		TypeMember(NavigationActive);
		TypeMember(NavigationPaused);
		TypeMember(BotActive);
		TypeMember(PetAffinity);
		TypeMember(Extended);
		TypeMember(FreeBuffSlots);
		TypeMember(InZone);
		TypeMember(InGroup);
		TypeMember(Leader);
		TypeMember(GroupLeader);    // Same as Leader
		TypeMember(Note);
		TypeMember(Location);
		TypeMember(X);
		TypeMember(Y);
		TypeMember(Z);
		TypeMember(Distance);
		TypeMember(Heading);
		TypeMember(Updated);
		TypeMember(Gem);
		TypeMember(Buff);
		TypeMember(Duration);
		TypeMember(BuffDuration);   // Same as Duration
		TypeMember(ShortBuff);
		TypeMember(ShortDuration);
		TypeMember(PetBuff);
		TypeMember(PetDuration);
		TypeMember(TotalAA);
		TypeMember(AAPointsTotal);	// Same as TotalAA
		TypeMember(UsedAA);
		TypeMember(AAPointsSpent);	// Same as UsedAA
		TypeMember(UnusedAA);
		TypeMember(AAPoints);		// Same as UnusedAA
		TypeMember(AAPointsAssigned);
		TypeMember(CombatState);
		TypeMember(Stacks);
		TypeMember(StacksPet);
		TypeMember(WillLand);
		TypeMember(WillLandPet);
		TypeMember(TooPowerful);
		TypeMember(TooPowerfulPet);
		TypeMember(Detrimentals);
		TypeMember(Counters);
		TypeMember(TotalCounters);
		TypeMember(CountersCurse);
		TypeMember(CountersDisease);
		TypeMember(CountersPoison);
		TypeMember(CountersCorruption);
		TypeMember(NoCure);
		TypeMember(LifeDrain);
		TypeMember(ManaDrain);
		TypeMember(EnduDrain);
		TypeMember(Cursed);
		TypeMember(Diseased);
		TypeMember(Poisoned);
		TypeMember(Corrupted);
		TypeMember(Blinded);
		TypeMember(CastingLevel);
		TypeMember(Charmed);
		TypeMember(Feared);
		TypeMember(Healing);
		TypeMember(Invulnerable);
		TypeMember(Mesmerized);
		TypeMember(Rooted);
		TypeMember(Silenced);
		TypeMember(Slowed);
		TypeMember(Snared);
		TypeMember(SpellCost);
		TypeMember(SpellSlowed);
		TypeMember(SpellDamage);
		TypeMember(Trigger);
		TypeMember(Resistance);
		TypeMember(Detrimental);
		TypeMember(RevDSed);
		TypeMember(MacroState);
		TypeMember(MacroName);
		TypeMember(FreeInventory);
		TypeMember(Version);
		TypeMember(MakeCampStatus);
		TypeMember(MakeCampX);
		TypeMember(MakeCampY);
		TypeMember(MakeCampRadius);
		TypeMember(MakeCampDistance);
		TypeMember(Lua);
		TypeMember(Packets);
		TypeMember(HeartBeat);
		TypeMember(Query);
		TypeMember(Crippled);
		TypeMember(Maloed);
		TypeMember(Tashed);
		TypeMember(Hasted);
		TypeMember(DSed);
		TypeMember(Aego);
		TypeMember(Skin);
		TypeMember(Focus);
		TypeMember(Regen);
		TypeMember(Symbol);
		TypeMember(Clarity);
		TypeMember(Pred);
		TypeMember(Strength);
		TypeMember(Brells);
		TypeMember(SV);
		TypeMember(SE);
		TypeMember(HybridHP);
		TypeMember(Growth);
		TypeMember(Shining);
		TypeMember(Spell);
	}

	virtual bool GetMember(MQVarPtr VarPtr, const char* Member, char* Index, MQTypeVar& Dest) override {
		if (auto pMember = MQ2NetBotsType::FindMember(Member)) {
			switch ((Information)pMember->ID) {
				case Enable:
					Dest.DWord = NetStat;
					Dest.Type = mq::datatypes::pBoolType;
					return true;
				case Listen:
					Dest.DWord = NetGrab;
					Dest.Type = mq::datatypes::pBoolType;
					return true;
				case Output:
					Dest.DWord = NetSend;
					Dest.Type = mq::datatypes::pBoolType;
					return true;
				case Counts: {
					int iCount = 0;
					if (NetStat && NetGrab) {
						for (auto& [_, botInfo] : NetMap) {
							if (botInfo->SpawnID != 0) {
								iCount++;
							}
						}
					}
					Dest.Int = iCount;
					Dest.Type = mq::datatypes::pIntType;
					return true;
				}
				case Client: {
					int iCount = 0;
					DataTypeTemp[0] = '\0';
					int n = ((Index[0] && IsNumber(Index)) ? GetIntFromString(Index, 0) : 0);
					if (NetStat && NetGrab) {
						for (auto& [_, botInfo] : NetMap) {
							if (botInfo->SpawnID != 0) {
								iCount++;
								if (n > 0) {
									if (n == iCount) {
										strcpy_s(DataTypeTemp, botInfo->Name);
										Dest.Ptr = &DataTypeTemp[0];
										Dest.Type = mq::datatypes::pStringType;
										return true;
									}
								} else {
									if (iCount > 1) {
										strcat_s(DataTypeTemp, " ");
									}
									strcat_s(DataTypeTemp, botInfo->Name);
								}
							}
						}
					}					
					if(!Index[0]) {
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					}
					break;
				}
				default:
					break;
			}
			if (auto botRec = VarPtr.Get<BotInfo>()) {
				switch ((Information)pMember->ID) {
					case Name:
						strcpy_s(DataTypeTemp, botRec->Name);
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					case Zone:
						Dest.DWord = botRec->ZoneID;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case Instance:
						Dest.DWord = botRec->InstanceID;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case ID:
						Dest.DWord = botRec->SpawnID;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case Class:
						Dest.DWord = botRec->ClassID;
						Dest.Type = mq::datatypes::pClassType;
						return true;
					case Level:
						Dest.DWord = botRec->Level;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case PctExp:
						Dest.Float = static_cast<float>(botRec->Exp) / EXP_TO_PCT_RATIO;
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case PctAAExp:
						Dest.Float = static_cast<float>(botRec->AAExp) / EXP_TO_PCT_RATIO;
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case PctGroupLeaderExp:
#if HAS_LEADERSHIP_EXPERIENCE
						Dest.Float = static_cast<float>(botRec->GroupLeaderExp) / 10.0f;
#else
						Dest.Float = 0.0f;
#endif
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case CurrentHPs:
						Dest.Int = botRec->HPCurrent;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case MaxHPs:
						Dest.Int = botRec->HPMax;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case PctHPs:
						Dest.Int = (botRec->HPMax == 0) ? 0 : botRec->HPCurrent * 100 / botRec->HPMax;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case CurrentMana:
						Dest.Int = botRec->ManaCurrent;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case MaxMana:
						Dest.Int = botRec->ManaMax;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case PctMana:
						Dest.Int = (botRec->ManaMax == 0) ? 0 : botRec->ManaCurrent * 100 / botRec->ManaMax;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case CurrentEndurance:
						Dest.Int = botRec->EnduranceCurrent;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case MaxEndurance:
						Dest.Int = botRec->EnduranceMax;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case PctEndurance:
						Dest.Int = (botRec->EnduranceMax == 0) ? 0 : botRec->EnduranceCurrent * 100 / botRec->EnduranceMax;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case PetID:
						Dest.DWord = botRec->PetID;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case PetHP:
					case PetPctHPs:
						Dest.Int = botRec->PetHPPct;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case TargetID:
						Dest.DWord = botRec->TargetID;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case TargetHP:
					case TargetPctHPs:
						Dest.Int = botRec->TargetHPPct;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case Casting:
						if (botRec->CastID) {
							Dest.Ptr = GetSpellByID(botRec->CastID);
							Dest.Type = mq::datatypes::pSpellType;
							return true;
						}
						break;
					case State:
						if (botRec->State & STATE_STUN) {
							strcpy_s(DataTypeTemp, "STUN");
						} else if (botRec->State & STATE_HOVER) {
							strcpy_s(DataTypeTemp, "HOVER");
						} else if (botRec->State & STATE_MOUNT) {
							strcpy_s(DataTypeTemp, "MOUNT");
						} else if (botRec->State & STATE_STAND) {
							strcpy_s(DataTypeTemp, "STAND");
						} else if (botRec->State & STATE_SIT) {
							strcpy_s(DataTypeTemp, "SIT");
						} else if (botRec->State & STATE_DUCK) {
							strcpy_s(DataTypeTemp, "DUCK");
						} else if (botRec->State & STATE_BIND) {
							strcpy_s(DataTypeTemp, "BIND");
						} else if (botRec->State & STATE_FEIGN) {
							strcpy_s(DataTypeTemp, "FEIGN");
						} else if (botRec->State & STATE_DEAD) {
							strcpy_s(DataTypeTemp, "DEAD");
						} else {
							strcpy_s(DataTypeTemp, "UNKNOWN");
						}
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					case Attacking:
						Dest.DWord = botRec->State & STATE_ATTACK;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case AFK:
						Dest.DWord = botRec->State & STATE_AFK;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Binding:
						Dest.DWord = botRec->State & STATE_BIND;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Ducking:
						Dest.DWord = botRec->State & STATE_DUCK;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Feigning:
						Dest.DWord = botRec->State & STATE_FEIGN;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Grouped:
						Dest.DWord = botRec->State & STATE_GROUP;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Invis:
						Dest.DWord = botRec->State & STATE_INVIS;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Levitating:
						Dest.DWord = botRec->State & STATE_LEV;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case LFG:
						Dest.DWord = botRec->State & STATE_LFG;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Mounted:
						Dest.DWord = botRec->State & STATE_MOUNT;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Moving:
						Dest.DWord = botRec->State & STATE_MOVING;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Raid:
						Dest.DWord = botRec->State & STATE_RAID;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Sitting:
						Dest.DWord = botRec->State & STATE_SIT;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Standing:
						Dest.DWord = botRec->State & STATE_STAND;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Stunned:
						Dest.DWord = botRec->State & STATE_STUN;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Dead:
						Dest.DWord = botRec->State & STATE_DEAD;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Hover:
						Dest.DWord = botRec->State & STATE_HOVER;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case InvisToUndead:
						Dest.DWord = botRec->State & STATE_ITU;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case AutoFire:
						Dest.DWord = botRec->State & STATE_RANGED;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case HaveAggro:
						Dest.DWord = botRec->State & STATE_HAVEAGGRO;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case WantAggro:
						Dest.DWord = botRec->State & STATE_WANTAGGRO;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case NavigationActive:
						Dest.DWord = botRec->State & STATE_NAVACTIVE;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case NavigationPaused:
						Dest.DWord = botRec->State & STATE_NAVPAUSED;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case BotActive:
						Dest.DWord = botRec->State & STATE_BOTACTIVE;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case PetAffinity:
						Dest.DWord = botRec->State & STATE_HASPETAA;
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Extended:
						Dest.Int = botRec->Extended;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case FreeBuffSlots:
						Dest.Int = botRec->FreeBuff;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case InZone:
						Dest.DWord = (pLocalPC && pLocalPC->zoneId == botRec->ZoneID && pLocalPC->instance == botRec->InstanceID);
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case InGroup:
						Dest.DWord = IsGroupMember(botRec->Name);
						Dest.Type = mq::datatypes::pBoolType;
						return true;
					case Leader:
					case GroupLeader:
						strcpy_s(DataTypeTemp, botRec->Leader);
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					case Note:
						strcpy_s(DataTypeTemp, botRec->Note);
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					case Location:
						strcpy_s(DataTypeTemp, botRec->Location);
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					case X:
						Dest.Float = botRec->X;
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case Y:
						Dest.Float = botRec->Y;
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case Z:
						Dest.Float = botRec->Z;
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case Distance:
						Dest.Float = Get3DDistance(pLocalPlayer->X, pLocalPlayer->Y, pLocalPlayer->Z, botRec->X, botRec->Y, botRec->Z);
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case Heading:
						strcpy_s(DataTypeTemp, botRec->Heading);
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					case Updated:
						Dest.Int = clock() - botRec->Updated;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case Gem:
						if(!Index[0]) {
							char szTemp[MAX_STRING];
							DataTypeTemp[0] = '\0';
							for (int i = 0; i < NUM_SPELL_GEMS; i++) {
								if (botRec->Gem[i]) {
									sprintf_s(szTemp, "%d ", botRec->Gem[i]);
									strcat_s(DataTypeTemp, szTemp);
								} else {
									strcat_s(DataTypeTemp, "0 ");
								}
							}
							Dest.Ptr = &DataTypeTemp[0];
							Dest.Type = mq::datatypes::pStringType;
							return true;
						}
						if (IsNumber(Index)) {
							if (int i = GetIntFromString(Index, 0); i > 0 && i <= NUM_SPELL_GEMS) {
								if (auto pSpell = GetSpellByID(botRec->Gem[i-1])) {
									Dest.Ptr = pSpell;
									Dest.Type = mq::datatypes::pSpellType;
									return true;
								}
							}
						} else {
							for (int i = 0; i < NUM_SPELL_GEMS; i++) {
								if (auto pSpell = GetSpellByID(botRec->Gem[i])) {
									if (NetSimple) {
										if (ci_find_substr(pSpell->Name, Index) != -1) {
											Dest.Ptr = pSpell;
											Dest.Type = mq::datatypes::pSpellType;
											return true;
										}
									} else {
										if (!_strnicmp(Index, pSpell->Name, strlen(Index))) {
											Dest.Ptr = pSpell;
											Dest.Type = mq::datatypes::pSpellType;
											return true;
										}
									}
								}
							}
						}
						break;					
					case Buff:
						if (!Index[0]) {
							char szTemp[MAX_STRING];
							DataTypeTemp[0] = '\0';
							for (int i = 0; i < NUM_LONG_BUFFS; i++) {	
								if (!botRec->Buff[i]) {
									break;
								}
								sprintf_s(szTemp, "%d ", botRec->Buff[i]);
								strcat_s(DataTypeTemp, szTemp);
							}
							Dest.Ptr = &DataTypeTemp[0];
							Dest.Type = mq::datatypes::pStringType;
							return true;
						}
						if (IsNumber(Index)) {
							if (int i = GetIntFromString(Index, -1); i >= 0 && i < NUM_LONG_BUFFS) {
								if (!botRec->Buff[i]) {
									break;
								}
								if (auto pSpell = GetSpellByID(botRec->Buff[i])) {
									Dest.Ptr = pSpell;
									Dest.Type = mq::datatypes::pSpellType;
									return true;
								}
							}
						} else {
							for (int i = 0; i < NUM_LONG_BUFFS; i++) {
								if (!botRec->Buff[i]) {
									break;
								}
								if (auto pSpell = GetSpellByID(botRec->Buff[i])) {
									if (NetSimple) {
										if (ci_find_substr(pSpell->Name, Index) != -1) {
											Dest.Ptr = pSpell;
											Dest.Type = mq::datatypes::pSpellType;
											return true;
										}
									} else {
										if (!_strnicmp(Index, pSpell->Name, strlen(Index))) {
											Dest.Ptr = pSpell;
											Dest.Type = mq::datatypes::pSpellType;
											return true;
										}
									}
								}
							}
						}
						break;
					case Duration:
					case BuffDuration:
						if(!Index[0]) {
							char szTemp[MAX_STRING];
							DataTypeTemp[0]=0;
							for (int i = 0; i < NUM_LONG_BUFFS; i++) {
								if (!botRec->Buff[i]) {
									break;
								}
								sprintf_s(szTemp,"%d ",botRec->BDuration[i]);
								strcat_s(DataTypeTemp,szTemp);
							}
							Dest.Ptr = &DataTypeTemp[0];
							Dest.Type = mq::datatypes::pStringType;
							return true;
						}
						if (IsNumber(Index)) {
							if (int i = GetIntFromString(Index, -1); i >= 0 && i < NUM_LONG_BUFFS) {
								if (!botRec->Buff[i]) {
									break;
								}
								Dest.DWord = botRec->BDuration[i];
								Dest.Type = mq::datatypes::pTicksType;
								return true;
							}
						} else {
							for (int i = 0; i < NUM_LONG_BUFFS; i++) {
								if (!botRec->Buff[i]) {
									break;
								}
								if (auto pSpell = GetSpellByID(botRec->Buff[i])) {
									if (NetSimple) {
										if (ci_find_substr(pSpell->Name, Index) != -1) {
											Dest.DWord = botRec->BDuration[i];
											Dest.Type = mq::datatypes::pTicksType;
											return true;
										}
									} else {
										if (!_strnicmp(Index, pSpell->Name, strlen(Index))) {
											Dest.DWord = botRec->BDuration[i];
											Dest.Type = mq::datatypes::pTicksType;
											return true;
										}
									}
								}
							}
						}
						break;
					case ShortBuff:
						if (!Index[0]) {
							char szTemp[MAX_STRING];
							DataTypeTemp[0] = 0;
							for (int i = 0; i < NUM_SHORT_BUFFS; i++) {
								if (!botRec->Song[i]) {
									break;
								}
								sprintf_s(szTemp, "%d ", botRec->Song[i]);
								strcat_s(DataTypeTemp, szTemp);
							}
							Dest.Ptr = &DataTypeTemp[0];
							Dest.Type = mq::datatypes::pStringType;
							return true;
						}
						if (IsNumber(Index)) {
							if (int i = GetIntFromString(Index, -1); i >= 0 && i < NUM_SHORT_BUFFS) {
								if (!botRec->Song[i]) {
									break;
								}
								if (auto pSpell = GetSpellByID(botRec->Song[i])) {
									Dest.Ptr = pSpell;
									Dest.Type = mq::datatypes::pSpellType;
									return true;
								}
							}
						} else {
							for (int i = 0; i < NUM_SHORT_BUFFS; i++) {
								if (!botRec->Song[i]) {
									break;
								}
								if (auto pSpell = GetSpellByID(botRec->Song[i])) {
									if (NetSimple) {
										if (ci_find_substr(pSpell->Name, Index) != -1) {
											Dest.Ptr = pSpell;
											Dest.Type = mq::datatypes::pSpellType;
											return true;
										}
									} else {
										if (!_strnicmp(Index, pSpell->Name, strlen(Index))) {
											Dest.Ptr = pSpell;
											Dest.Type = mq::datatypes::pSpellType;
											return true;
										}
									}
								}
							}
						}
						break;
					case ShortDuration:
						if(!Index[0]) {
							char szTemp[MAX_STRING];
							DataTypeTemp[0]=0;
							for (int i = 0; i < NUM_SHORT_BUFFS; i++) {
								if (!botRec->Song[i]) {
									break;
								}
								sprintf_s(szTemp,"%d ",botRec->SDuration[i]);
								strcat_s(DataTypeTemp,szTemp);
							}
							Dest.Ptr=&DataTypeTemp[0];
							Dest.Type=mq::datatypes::pStringType;
							return true;
						}
						if (IsNumber(Index)) {
							if (int i = GetIntFromString(Index, -1); i >= 0 && i < NUM_SHORT_BUFFS) {
								if (!botRec->Song[i]) {
									break;
								}
								Dest.DWord = botRec->SDuration[i];
								Dest.Type=mq::datatypes::pTicksType;
								return true;
							}
						} else {
							for (int i = 0; i < NUM_SHORT_BUFFS; i++) {
								if (!botRec->Song[i]) {
									break;
								}
								if (auto pSpell = GetSpellByID(botRec->Song[i])) {
									if (NetSimple) {
										if (ci_find_substr(pSpell->Name, Index) != -1) {
											Dest.DWord = botRec->SDuration[i];
											Dest.Type=mq::datatypes::pTicksType;
											return true;
										}
									} else {
										if (!_strnicmp(Index, pSpell->Name, strlen(Index))) {
											Dest.DWord = botRec->SDuration[i];
											Dest.Type=mq::datatypes::pTicksType;
											return true;
										}
									}
								}
							}
						}
						break;
					case PetBuff:
						if (!Index[0]) {
							char szTemp[MAX_STRING];
							DataTypeTemp[0] = '\0';
							for (int i = 0; i < PETS_MAX; i++) {
								if (!botRec->Pets[i]) {
									break;
								}
								sprintf_s(szTemp, "%d ", botRec->Pets[i]);
								strcat_s(DataTypeTemp, szTemp);
							}
							Dest.Ptr = &DataTypeTemp[0];
							Dest.Type = mq::datatypes::pStringType;
							return true;
						}
						if (IsNumber(Index)) {
							if (int i = GetIntFromString(Index, -1); i >= 0 && i < PETS_MAX) {
								if (!botRec->Pets[i]) {
									break;
								}
								if (auto pSpell = GetSpellByID(botRec->Pets[i])) {
									Dest.Ptr = pSpell;
									Dest.Type = mq::datatypes::pSpellType;
									return true;
								}
							}
						} else {
							for (int i = 0; i < PETS_MAX; i++) {
								if (botRec->Pets[i]) {
									break;
								}
								if (auto pSpell = GetSpellByID(botRec->Pets[i])) {
									if (NetSimple) {
										if (ci_find_substr(pSpell->Name, Index) != -1) {
											Dest.Ptr = pSpell;
											Dest.Type = mq::datatypes::pSpellType;
											return true;
										}
									} else {
										if (!_strnicmp(Index, pSpell->Name, strlen(Index))) {
											Dest.Ptr = pSpell;
											Dest.Type = mq::datatypes::pSpellType;
											return true;
										}
									}
								}
							}
						}
						break;
					case PetDuration:
						if(!Index[0]) {
							char szTemp[MAX_STRING];
							DataTypeTemp[0]=0;
							for (int i = 0; i < PETS_MAX; i++) {
								if (!botRec->Pets[i]) {
									break;
								}
								sprintf_s(szTemp,"%d ",botRec->PDuration[i]);
								strcat_s(DataTypeTemp,szTemp);
							}
							Dest.Ptr=&DataTypeTemp[0];
							Dest.Type=mq::datatypes::pStringType;
							return true;
						}
						if (IsNumber(Index)) {
							if (int i = GetIntFromString(Index, -1); i >= 0 && i < PETS_MAX) {
								if (!botRec->Pets[i]) {
									break;
								}
								Dest.DWord = botRec->PDuration[i];
								Dest.Type=mq::datatypes::pTicksType;
								return true;
							}
						} else {
							for (int i = 0; i < PETS_MAX; i++) {
								if (!botRec->Pets[i]) {
									break;
								}
								if (auto pSpell = GetSpellByID(botRec->Pets[i])) {
									if (NetSimple) {
										if (ci_find_substr(pSpell->Name, Index) != -1) {
											Dest.DWord = botRec->PDuration[i];
											Dest.Type=mq::datatypes::pTicksType;
											return true;
										}
									} else {
										if (!_strnicmp(Index, pSpell->Name, strlen(Index))) {
											Dest.DWord = botRec->PDuration[i];
											Dest.Type=mq::datatypes::pTicksType;
											return true;
										}
									}
								}
							}
						}
						break;
					case TotalAA:
					case AAPointsTotal:
						Dest.DWord = botRec->AAPoints + botRec->AAPointsSpent;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case UsedAA:
					case AAPointsSpent:
						Dest.DWord = botRec->AAPointsSpent;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case UnusedAA:
					case AAPoints:
						Dest.DWord = botRec->AAPoints;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case AAPointsAssigned:
						Dest.DWord = botRec->AAPointsAssigned;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case CombatState:
						Dest.DWord = botRec->CombatState;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case Stacks:
						Dest.DWord = false;
						Dest.Type = mq::datatypes::pBoolType;
						if (!Index[0]) {
							return true;
						}
						if (auto testSpell = GetSpellByName(Index)) {		// Accepts Name or ID
							for (int i = 0; i < NUM_LONG_BUFFS; i++) {
								if (!botRec->Buff[i]) {
									break;
								}
								if (auto buffSpell = GetSpellByID(botRec->Buff[i])) {
									if ((buffSpell == testSpell) || !WillStackWith(testSpell, buffSpell)) {
										return true;
									}
								}
							}
							for (int i = 0; i < NUM_SHORT_BUFFS; i++) {
								if (!botRec->Song[i]) {
									break;
								}
								if (auto buffSpell = GetSpellByID(botRec->Song[i])) {
									if (!IsBardSong(buffSpell) && !((IsSPAEffect(testSpell, SPA_CHANGE_FORM) && !testSpell->DurationWindow))) {
										if ((buffSpell == testSpell) || !WillStackWith(testSpell, buffSpell)) {
											return true;
										}
									}
								}
							}
							Dest.DWord = true;
						}
						return true;
					case StacksPet:
						Dest.DWord = false;
						Dest.Type = mq::datatypes::pBoolType;
						if (!Index[0] || botRec->PetID == 0) {
							return true;
						}
						if (auto testSpell = GetSpellByName(Index)) {		// Accepts Name or ID
							for (int i = 0; i < PETS_MAX; i++) {
								if (!botRec->Pets[i]) {
									break;
								}
								if (auto buffSpell = GetSpellByID(botRec->Pets[i])) {
									if ((buffSpell == testSpell) || !WillStackWith(testSpell, buffSpell)) {
										return true;
									}
								}
							}
							Dest.DWord = true;
						}
						return true;
					case WillLand:
						Dest.DWord = false;
						Dest.Type = mq::datatypes::pBoolType;
						if (!Index[0]) {
							return true;
						}
						if (auto testSpell = GetSpellByName(Index)) {		// Accepts Name or ID
							for (int i = 0; i < NUM_LONG_BUFFS; i++) {
								if (!botRec->Buff[i]) {
									break;
								}
								if (auto buffSpell = GetSpellByID(botRec->Buff[i])) {
									if (!WillStackWith(testSpell, buffSpell)) {
										return true;
									}
								}
							}
							for (int i = 0; i < NUM_SHORT_BUFFS; i++) {
								if (!botRec->Song[i]) {
									break;
								}
								if (auto buffSpell = GetSpellByID(botRec->Song[i])) {
									if (!IsBardSong(buffSpell) && !((IsSPAEffect(testSpell, SPA_CHANGE_FORM) && !testSpell->DurationWindow))) {
										if (!WillStackWith(testSpell, buffSpell)) {
											return true;
										}
									}
								}
							}
							Dest.DWord = true;
						}
						return true;
					case WillLandPet:
						Dest.DWord = false;
						Dest.Type = mq::datatypes::pBoolType;
						if (!Index[0] || botRec->PetID == 0) {
							return true;
						}
						if (auto testSpell = GetSpellByName(Index)) {		// Accepts Name or ID
							for (int i = 0; i < PETS_MAX; i++) {
								if (!botRec->Pets[i]) {
									break;
								}
								if (auto buffSpell = GetSpellByID(botRec->Pets[i])) {
									if (!WillStackWith(testSpell, buffSpell)) {
										return true;
									}
								}
							}
							Dest.DWord = true;
						}
						return true;
					case TooPowerfulPet:
						if (botRec->PetID == 0) {
							Dest.DWord = false;
							Dest.Type = mq::datatypes::pBoolType;
							return true;
						}
					case TooPowerful:		// TooPowerfulPet falls thru to here
						Dest.DWord = false;
						Dest.Type = mq::datatypes::pBoolType;
						if (!Index[0]) {
							return true;
						}
						if (auto pSpell = GetSpellByName(Index)) {		// Accepts Name or ID
							if (pSpell->TargetType == TargetType_Self || pSpell->SpellType == SpellType_Detrimental || pSpell->DurationType == 0) {
								return true;
							}
							int spellLevel = pSpell->GetSpellLevelNeeded(GetPcProfile()->Class);
							if (spellLevel == 0 || spellLevel > MAX_PC_LEVEL) {
								spellLevel = CalcMinSpellLevel(pSpell);
							}
							int targetLevel = botRec->Level;  			// Pet also uses Master's Level
							if (spellLevel > 65 && targetLevel < 61) {
								Dest.DWord = true;
								return true;
							}
							if (spellLevel > 50 && targetLevel < floor(spellLevel / 2) + 15) {
								Dest.DWord = true;
								return true;
							}
						}
						return true;
					case Detrimentals:
						Dest.Int64 = botRec->Detrimental[D_DETRIMENTALS];
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case Counters:
					case TotalCounters:
						Dest.Int64 = botRec->Detrimental[D_COUNTERS];
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case CountersCurse:
						Dest.Int64 = botRec->Detrimental[D_CURSED];
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case CountersDisease:
						Dest.Int64 = botRec->Detrimental[D_DISEASED];
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case CountersPoison:
						Dest.Int64 = botRec->Detrimental[D_POISONED];
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case CountersCorruption:
						Dest.Int64 = botRec->Detrimental[D_CORRUPTED];
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case NoCure:
						Dest.Int64 = botRec->Detrimental[D_NOCURE];
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case LifeDrain:
						Dest.Int64 = botRec->Detrimental[D_LIFEDRAIN];
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case ManaDrain:
						Dest.Int64 = botRec->Detrimental[D_MANADRAIN];
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case EnduDrain:
						Dest.Int64 = botRec->Detrimental[D_ENDUDRAIN];
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case Detrimental:
						DataTypeTemp[0] = '\0';
						for (const auto& pair : BuffMap) {
							if (pair.second.Type == BT_DETRIMENTAL && (botRec->DetrState & pair.second.State)) {
								if (DataTypeTemp[0]) {
									strcat_s(DataTypeTemp, " ");
								}
								strcat_s(DataTypeTemp, pair.first.c_str());
							}
						}
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					case MacroState:
						Dest.Int = botRec->MacroState;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case MacroName:
						strcpy_s(DataTypeTemp, botRec->MacroName);
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					case FreeInventory:
						if(!Index[0]) {
							Dest.Int = botRec->FreeInventory[0];
							Dest.Type = mq::datatypes::pIntType;
							return true;
						}
						if (IsNumber(Index)) {
							if (int i = GetIntFromString(Index, -1); i >= 0 && i < ISIZE) {
								Dest.Int = botRec->FreeInventory[i];
								Dest.Type = mq::datatypes::pIntType;
								return true;
							}
						}
						break;
					case Version:
						Dest.Float = botRec->Version;
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case MakeCampStatus:
						Dest.Int = botRec->MakeCampStatus;
						Dest.Type = mq::datatypes::pIntType;
						return true;
					case MakeCampX:
						Dest.Float = botRec->MakeCampX;
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case MakeCampY:
						Dest.Float = botRec->MakeCampY;
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case MakeCampRadius:
						Dest.Float = botRec->MakeCampRadius;
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case MakeCampDistance:
						Dest.Float = botRec->MakeCampDistance;
						Dest.Type = mq::datatypes::pFloatType;
						return true;
					case Lua:
						strcpy_s(DataTypeTemp, botRec->LuaInfo);
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					case Packets:
						Dest.Int64 = botRec->EQBC_Packets;
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case HeartBeat:
						Dest.Int64 = botRec->EQBC_HeartBeat;
						Dest.Type = mq::datatypes::pInt64Type;
						return true;
					case Query:
						strcpy_s(DataTypeTemp, botRec->Query);
						Dest.Ptr = &DataTypeTemp[0];
						Dest.Type = mq::datatypes::pStringType;
						return true;
					case Slowed:
					case Rooted:
					case Mesmerized:
					case Crippled:
					case Maloed:
					case Tashed:
					case Snared:
					case RevDSed:
					case Charmed:
					case Diseased:
					case Poisoned:
					case Cursed:
					case Corrupted:
					case Blinded:
					case CastingLevel:
					case Feared:
					case Healing:
					case Invulnerable:
					case Resistance:
					case Silenced:
					case SpellCost:
					case SpellDamage:
					case SpellSlowed:
					case Trigger:
					case DSed:
					case Aego:
					case Skin:
					case Focus:
					case Regen:
					case Symbol:
					case Clarity:
					case Pred:
					case Strength:
					case Brells:
					case SV:
					case SE:
					case HybridHP:
					case Growth:
					case Shining:
					case Hasted: {
						auto f = BuffMap.find(pMember->Name);
						Dest.Int = ((f != BuffMap.end()) && 
									((f->second.Type == BT_DETRIMENTAL && (botRec->DetrState & f->second.State)) ||
									 (f->second.Type == BT_BENEFICIAL  && (botRec->BeneState & f->second.State)))); 
						Dest.Type = mq::datatypes::pIntType;
						return true;
					}
					case Spell: {
						if (!Index[0] || IsNumber(Index)) {
							break;
						}
						int bType = 0;
						int sType = 0;
						auto f = BuffMap.find(Index);
						if (f != BuffMap.end()) {
							if ((f->second.Type == BT_DETRIMENTAL && (botRec->DetrState & f->second.State)) ||
								(f->second.Type == BT_BENEFICIAL  && (botRec->BeneState & f->second.State))) {
								bType = f->second.Type;
								sType = f->second.SpellType;
							}
						}
						if (!sType) {
							break;
						}
						int sResult = -1;
						for (int i = 0; i < NUM_LONG_BUFFS; i++) {
							if (!botRec->Buff[i]) {
								break;
							}
							if (auto pSpell = GetSpellByID(botRec->Buff[i])) {
								if ((pSpell->SpellType == SpellType_Detrimental) == (bType == BT_DETRIMENTAL)) {
									if (CheckBotSpell(pSpell, sType)) {
										sResult = i;
										break;
									}
								}
							}
						}
						if (sResult >= 0) {
							if (auto pSpell = GetSpellByID(botRec->Buff[sResult])) {
								Dest.Ptr = pSpell;
								Dest.Type = mq::datatypes::pSpellType;
								return true;
							}
							break;
						}
						for (int i = 0; i < NUM_SHORT_BUFFS; i++) {
							if (!botRec->Song[i]) {
								break;
							}
							if (auto pSpell = GetSpellByID(botRec->Song[i])) {
								if ((pSpell->SpellType == SpellType_Detrimental) == (bType == BT_DETRIMENTAL)) {
									if (CheckBotSpell(pSpell, sType)) {
										sResult = i;
										break;
									}
								}
							}
						}
						if (sResult >= 0) {
							if (auto pSpell = GetSpellByID(botRec->Song[sResult])) {
								Dest.Ptr = pSpell;
								Dest.Type = mq::datatypes::pSpellType;
								return true;
							}
							break;
						}
						break;
					}
					default:
						break;
				}
			}
		}
		strcpy_s(DataTypeTemp, "NULL");
		Dest.Ptr = &DataTypeTemp[0];
		Dest.Type = mq::datatypes::pStringType;
		return true;
	}

	~MQ2NetBotsType() {
	}
};

bool dataNetBots(const char* szIndex, MQTypeVar& Ret) {
	Ret.Type = pNetBotsType;
	if (szIndex != nullptr && szIndex[0] != '\0') {
		auto f = NetMap.find(szIndex);
		if (f != NetMap.end()) {
			Ret.Set(f->second);
			return true;
		}
		return false;
	}
	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void WriteWindowINI(CSidlScreenWnd* pWindow);
void ReadWindowINI(CSidlScreenWnd* pWindow);
void DestroyMyWindow();
void CreateMyWindow();
void HideMyWindow();
void ShowMyWindow();
void WindowUpdate();

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void LoadINIValues() {
	char szTemp[MAX_STRING] = { 0 };
	int oldUseNewWindow = NetUseNewWindow;
	
	GetPrivateProfileString("Settings", "Query",       "", NetQuery,       MAX_STRING, GlobalINIFileName);
	GetPrivateProfileString("Settings", "WindowTitle", "", NetWindowTitle, MAX_STRING, GlobalINIFileName);
	// Handle empty title as 1/Default.
	if (!NetWindowTitle[0]) {
		strcpy_s(NetWindowTitle, "1");
	}
	
	NetStat         = GetPrivateProfileInt(PLUGIN_NAME, "Stat",            0, INIFileName);
	NetGrab         = GetPrivateProfileInt(PLUGIN_NAME, "Grab",            0, INIFileName);
	NetSend         = GetPrivateProfileInt(PLUGIN_NAME, "Send",            0, INIFileName);
	NetSimple       = GetPrivateProfileInt(PLUGIN_NAME, "UseSimpleSearch", 0, INIFileName);
	NetShow         = GetPrivateProfileInt(PLUGIN_NAME, "Show",            0, INIFileName);
	NetUseNewWindow = GetPrivateProfileInt(PLUGIN_NAME, "UseNewWindow",    1, INIFileName);
	NetConColors    = GetPrivateProfileInt(PLUGIN_NAME, "ConColors",       0, INIFileName);
	NetCleanNames   = GetPrivateProfileInt(PLUGIN_NAME, "CleanNames",      0, INIFileName);
	NetEQBCData     = GetPrivateProfileInt(PLUGIN_NAME, "SendEQBCData",    0, INIFileName);
	NetExtended     = std::clamp(GetPrivateProfileInt(PLUGIN_NAME, "Extended",        0, INIFileName), 0, EXSIZE - 1);
	NetLClick       = std::clamp(GetPrivateProfileInt(PLUGIN_NAME, "LClick",          0, INIFileName), 0, CASIZE - 1);
	NetRClick       = std::clamp(GetPrivateProfileInt(PLUGIN_NAME, "RClick",          0, INIFileName), 0, CASIZE - 1);
	NetRightMost    = std::clamp(GetPrivateProfileInt(PLUGIN_NAME, "TargetRightMost", 0, INIFileName), 0, EQ_MAX_NAME);

	WritePrivateProfileString("Settings",  "Query",           NetQuery,       GlobalINIFileName);
	WritePrivateProfileString("Settings",  "WindowTitle",     NetWindowTitle, GlobalINIFileName);
	
	WritePrivateProfileInt(PLUGIN_NAME, "Stat",            NetStat ? 1 : 0, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "Grab",            NetGrab ? 1 : 0, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "Send",            NetSend ? 1 : 0, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "UseSimpleSearch", NetSimple ? 1 : 0, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "Show",            NetShow ? 1 : 0, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "UseNewWindow",    NetUseNewWindow ? 1 : 0, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "ConColors",       NetConColors ? 1 : 0, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "CleanNames",      NetCleanNames ? 1 : 0, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "SendEQBCData",    NetEQBCData ? 1 : 0, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "Extended",        NetExtended, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "LClick",          NetLClick, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "RClick",          NetRClick, INIFileName);
	WritePrivateProfileInt(PLUGIN_NAME, "TargetRightMost", NetRightMost, INIFileName);

	// WindowTItle - 0=None, 1=Default, anything else is as-entered. Empty is treated as 1/Default (see above).
	if (!_stricmp(NetWindowTitle, "0")) {
		NetWindowTitle[0] = '\0';
	} else if (!_stricmp(NetWindowTitle, "1")) {
		sprintf_s(NetWindowTitle, "%s (${NetBots.Counts})", PLUGIN_NAME);
	}
	if (NetUseNewWindow != oldUseNewWindow) {
		DestroyMyWindow();
	}
	if (NetShow) {
		ShowMyWindow();
	}
}

void Command(PlayerClient* pChar, const char* Cmd) {
	char szTemp[MAX_STRING];
	char Var[MAX_STRING];
	char Set[MAX_STRING];
	int Parm = 1;
	do {
		GetArg(szTemp, Cmd, Parm++);
		GetArg(Var, szTemp, 1, FALSE, FALSE, FALSE, '=');
		GetArg(Set, szTemp, 2, FALSE, FALSE, FALSE, '=');
		if (!_stricmp(szTemp, "show")) {
			NetShow = 1;
			WritePrivateProfileInt(PLUGIN_NAME, "Show", NetShow, INIFileName);
			ShowMyWindow();
			return;
		}
		if (!_stricmp(szTemp, "hide")) {
			NetShow = 0;
			WritePrivateProfileInt(PLUGIN_NAME, "Show", NetShow, INIFileName);
			HideMyWindow();
			return;
		}
		if (!_stricmp(szTemp, "switch")) {
			DestroyMyWindow();
			NetUseNewWindow = (NetUseNewWindow ? 0 : 1);
	        WritePrivateProfileInt(PLUGIN_NAME, "UseNewWindow", NetUseNewWindow, INIFileName);
			if (NetShow) {
				ShowMyWindow();
			}
			return;
		}
		if (!_stricmp(szTemp, "on") || !_stricmp(szTemp, "off")) {
			NetStat = _stricmp(szTemp, "off");
			WritePrivateProfileInt(PLUGIN_NAME, "Stat", NetStat ? 1 : 0, INIFileName);
		} else if (!_stricmp(Var, "stat") || !_stricmp(Var, "plugin")) {
			NetStat = _stricmp(Set, "off");
			WritePrivateProfileInt(PLUGIN_NAME, "Stat", NetStat ? 1 : 0, INIFileName);
		} else if (!_stricmp(Var, "grab")) {
			NetGrab = _stricmp(Set, "off");
			WritePrivateProfileInt(PLUGIN_NAME, "Grab", NetGrab ? 1 : 0, INIFileName);
		} else if (!_stricmp(Var, "send")) {
			NetSend = _stricmp(Set, "off");
			WritePrivateProfileInt(PLUGIN_NAME, "Send", NetSend ? 1 : 0, INIFileName);
		} else if (!_stricmp(Var, "extended") || !_stricmp(Var, "ext")) {
			// Support old off/on values as 0/1
			if (!_stricmp(Set, "off")) {
				NetExtended = 0;
			} else if (!_stricmp(Set, "on")) {
				NetExtended = 1;
			} else {
				NetExtended = std::clamp(GetIntFromString(Set, 0), 0, EXSIZE - 1);
			}
			WritePrivateProfileInt(PLUGIN_NAME, "Extended", NetExtended, INIFileName);
		} else if (!_stricmp(Var, "usesimplesearch") || !_stricmp(Var, "simple")) {
			NetSimple = _stricmp(Set, "off");
			WritePrivateProfileInt(PLUGIN_NAME, "UseSimpleSearch", NetSimple ? 1 : 0, INIFileName);
		} else if (!_stricmp(Var, "LClick")) {
			if (!_stricmp(Set, "foreground")) {
				NetLClick = CA_BRING_TO_FOREGROUND;
			} else if (!_stricmp(Set, "target")) {
				NetLClick = CA_TARGET_PLAYER;
			} else if (!_stricmp(Set, "tot")) {
				NetLClick = CA_TARGET_PLAYER_TARGET;
			} else {
				NetLClick = std::clamp(GetIntFromString(Set, 0), 0, CASIZE - 1);
			}
			WritePrivateProfileInt(PLUGIN_NAME, "LClick", NetLClick, INIFileName);
		} else if (!_stricmp(Var, "RClick")) {
			if (!_stricmp(Set, "foreground")) {
				NetRClick = CA_BRING_TO_FOREGROUND;
			} else if (!_stricmp(Set, "target")) {
				NetRClick = CA_TARGET_PLAYER;
			} else if (!_stricmp(Set, "tot")) {
				NetRClick = CA_TARGET_PLAYER_TARGET;
			} else {
				NetRClick = std::clamp(GetIntFromString(Set, 0), 0, CASIZE - 1);
			}
			WritePrivateProfileInt(PLUGIN_NAME, "RClick", NetRClick, INIFileName);
		} else if (!_stricmp(Var, "concolors")) {
			NetConColors = _stricmp(Set, "off");
			WritePrivateProfileInt(PLUGIN_NAME, "ConColors", NetConColors ? 1 : 0, INIFileName);
		} else if (!_stricmp(Var, "cleannames")) {
			NetCleanNames = _stricmp(Set, "off");
			WritePrivateProfileInt(PLUGIN_NAME, "CleanNames", NetCleanNames ? 1 : 0, INIFileName);
		} else if (!_stricmp(Var, "targetrightmost") || !_stricmp(Var, "rightmost")) {
			NetRightMost = std::clamp(GetIntFromString(Set, 0), 0, EQ_MAX_NAME);
			WritePrivateProfileInt(PLUGIN_NAME, "TargetRightMost", NetRightMost, INIFileName);
		} else if (!_stricmp(Var, "sendeqbcdata")) {
			NetEQBCData = _stricmp(Set, "off");
			WritePrivateProfileInt(PLUGIN_NAME, "SendEQBCData", NetEQBCData ? 1 : 0, INIFileName);
		} else if (!_stricmp(Var, "load")) {
			LoadINIValues();
		}
	} while (strlen(szTemp));

	WriteChatf("%s:: (%s) Grab (%s) Send (%s) Extended (\at%d\ax) Simple (%s) Show (%s). Version=\at%.2f",
				PLUGIN_NAME,
				NetStat ? "\agon\ax" : "\aroff\ax",
				NetGrab ? "\agon\ax" : "\aroff\ax",
				NetSend ? "\agon\ax" : "\aroff\ax",
				NetExtended,
				NetSimple ? "\agon\ax" : "\aroff\ax",
				NetShow ? "\agon\ax" : "\aroff\ax",
				MQ2Version);

	if (NetShow) {			
		WriteChatf("%s:: ConColors (%s) CleanNames (%s) RightMost (\at%d\ax). LClick=\at%s\ax, RClick=\at%s\ax.",
				PLUGIN_NAME,
				NetConColors ? "\agon\ax" : "\aroff\ax",
				NetCleanNames ? "\agon\ax" : "\aroff\ax",
				NetRightMost,
				(NetLClick == CA_BRING_TO_FOREGROUND) ? "1/Foreground" : (NetLClick == CA_TARGET_PLAYER) ? "2/Target" : (NetLClick == CA_TARGET_PLAYER_TARGET) ? "3/ToT" : "0/None",
				(NetRClick == CA_BRING_TO_FOREGROUND) ? "1/Foreground" : (NetRClick == CA_TARGET_PLAYER) ? "2/Target" : (NetRClick == CA_TARGET_PLAYER_TARGET) ? "3/ToT" : "0/None");
	}
}

void CommandNote(PlayerClient* pChar, const char* Cmd) {
	strcpy_s(NetNote, Cmd);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

PLUGIN_API void OnBeginZone() {
	if (DEBUGGING) {
		DebugSpewAlways("%s->OnBeginZone()", PLUGIN_NAME);
	}
	ZeroMemory(sTimers, sizeof(sTimers));
	if (NetStat && NetSend && EQBCConnected()) {
		EQBCBroadCast("[NB]|Z=:>|[NB]");
	}
}

PLUGIN_API void OnNetBotEVENT(const char* Msg) {
	if (DEBUGGING) {
		DebugSpewAlways("%s->OnNetBotEVENT(%s)", PLUGIN_NAME, Msg);
	}
	if (!strncmp(Msg, "NBQUIT=", 7)) {
		BotQuit(&Msg[7]);
	} else if (!strncmp(Msg, "NBJOIN=", 7)) {
		ZeroMemory(sTimers, sizeof(sTimers));
	} else if (!strncmp(Msg, "NBEXIT", 6)) {
		NetMap.clear();
	}
}

PLUGIN_API void OnNetBotMSG(const char* Name, const char* Msg) {
	if (NetStat && NetGrab && !strncmp(Msg, "[NB]|", 5) && pLocalPC && pLocalPlayer && strcmp(pLocalPC->Name, Name)) {
		if (DEBUGGING) {
			DebugSpewAlways("%s->OnNetBotMSG(From:[%s] Msg[%s])", PLUGIN_NAME, Name, Msg);
		}
		char szCmd[MAX_STRING] = { 0 };
		strcpy_s(szCmd, Msg);
		if (CurBot = BotLoad(Name).get()) {
			Packet.Feed(szCmd);
			CurBot->Updated = clock();
			CurBot = 0;
		}
	}
}

PLUGIN_API void OnPulse() {
	if (NetStat && NetSend && gbInZone && (int)clock() > NetLast) {
		NetLast = (int)clock() + NETTICK;
		if (EQBCConnected() && pLocalPC && pLocalPlayer && GetPcProfile()) {
			BroadCast();
		}
	}
	if (NetShow && NetStat && NetSend && MQGetTickCount64() > ShowTime) {
		WindowUpdate();
	}
}

PLUGIN_API void SetGameState(int GameState) {
	if (DEBUGGING) {
		DebugSpewAlways("%s->SetGameState(%d)", PLUGIN_NAME, GameState);
	}
	if (GameState == GAMESTATE_INGAME) {
		if (!NetInit) {
			if (DEBUGGING) {
				DebugSpewAlways("%s->SetGameState(%d)->Loading", PLUGIN_NAME, GameState);
			}
			sprintf_s(GlobalINIFileName, "%s\\%s.ini", gPathConfig, PLUGIN_NAME);
			sprintf_s(INIFileName, "%s\\%s_%s.ini", gPathConfig, GetServerShortName(), pLocalPC->Name);
			sprintf_s(PlayerSectionName, "%s.%s", GetServerShortName(), pLocalPC->Name);
			LoadINIValues();
			NetInit = 1;
		}
	} else if (GameState != GAMESTATE_LOGGINGIN) {
		if (NetInit) {
			if (DEBUGGING) {
				DebugSpewAlways("%s->SetGameState(%d)->Flushing", PLUGIN_NAME, GameState);
			}
			NetStat = 0;
			NetGrab = 0;
			NetSend = 0;
			NetExtended = 0;
			NetShow = 0;
			NetUseNewWindow = -1;
			NetLClick = 0;
			NetRClick = 0;
			NetConColors = 0;
			NetCleanNames = 0;
			NetRightMost = 0;
			NetEQBCData = 0;
			NetInit = 0;
		}
	}
}

void __stdcall ParseInfo(unsigned int ID, void *pData, PBLECHVALUE pValues) {
	if (CurBot) while (pValues) {
		switch (GetIntFromString(pValues->Name, 0)) {
			case  1:
				CurBot->ZoneID = GetIntFromString(pValues->Value, 0);
				break;
			case  2:
				CurBot->InstanceID = GetIntFromString(pValues->Value, 0);
				break;
			case  3:
				CurBot->SpawnID = GetIntFromString(pValues->Value, 0);
				break;
			case  4:
				CurBot->Level = GetIntFromString(pValues->Value, 0);
				break;
			case  5:
				CurBot->ClassID = GetIntFromString(pValues->Value, 0);
				break;
			case  6:
				CurBot->HPCurrent = GetIntFromString(pValues->Value, 0);
				break;
			case  7:
				CurBot->HPMax = GetIntFromString(pValues->Value, 0);
				break;
			case  8:
				CurBot->EnduranceCurrent = GetIntFromString(pValues->Value, 0);
				break;
			case  9:
				CurBot->EnduranceMax = GetIntFromString(pValues->Value, 0);
				break;
			case 10:
				CurBot->ManaCurrent = GetIntFromString(pValues->Value, 0);
				break;
			case 11:
				CurBot->ManaMax = GetIntFromString(pValues->Value, 0);
				break;
			case 12:
				CurBot->PetID = GetIntFromString(pValues->Value, 0);
				break;
			case 13:
				CurBot->PetHPPct = GetIntFromString(pValues->Value, 0);
				break;
			case 14:
				CurBot->TargetID = GetIntFromString(pValues->Value, 0);
				break;
			case 15:
				CurBot->TargetHPPct = GetIntFromString(pValues->Value, 0);
				break;
			case 16:
				CurBot->CastID = GetIntFromString(pValues->Value, 0);
				break;
			case 17:
				CurBot->State = (uint32_t)GetIntFromString(pValues->Value, 0);
				break;
			case 18:
				CurBot->Exp = (int64_t)GetIntFromString(pValues->Value, 0);
				break;
			case 19:
				CurBot->AAExp = (uint32_t)GetIntFromString(pValues->Value, 0);
				break;
#if HAS_LEADERSHIP_EXPERIENCE
			case 20:
				CurBot->GroupLeaderExp = GetDoubleFromString(pValues->Value, 0.0);
				break;
#endif
			case 21:
				CurBot->FreeBuff = GetIntFromString(pValues->Value, 0);
				break;
			case 22:
				strcpy_s(CurBot->Leader, pValues->Value.c_str());
				break;
			case 30: 
				InfoGems(CurBot, pValues->Value.c_str());
				break;
			case 31:
				InfoBuff(CurBot, pValues->Value.c_str());
				break;
			case 32:
				InfoSong(CurBot, pValues->Value.c_str());
				break;
			case 33:
				InfoPets(CurBot, pValues->Value.c_str());
				break;
			case 34:
				InfoBDura(CurBot, pValues->Value.c_str());
				break;
			case 35:
				CurBot->AAPoints = GetIntFromString(pValues->Value, 0);
				break;
			case 36:
				CurBot->AAPointsSpent = GetIntFromString(pValues->Value, 0);
				break;
			case 37:
				CurBot->AAPointsAssigned = GetIntFromString(pValues->Value, 0);
				break;
			case 38:
				CurBot->CombatState = GetIntFromString(pValues->Value, 0);
				break;
			case 39:
				strcpy_s(CurBot->Note, pValues->Value.c_str());
				break;
			case 40:
				InfoDetr(CurBot, pValues->Value.c_str());
				break;
			case 89:
				InfoLoc(CurBot, pValues->Value.c_str());
				break;
			case 90:
				strcpy_s(CurBot->Heading, pValues->Value.c_str());
				break;
			case 91:
				CurBot->Extended = GetIntFromString(pValues->Value, 0);
				break;
			case 94:
				CurBot->MacroState = GetIntFromString(pValues->Value, 0);
				break;
			case 95:
				strcpy_s(CurBot->MacroName, pValues->Value.c_str());
				break;
			case 110: 
				InfoFreeI(CurBot, pValues->Value.c_str());
				break;
			case 111:
				CurBot->Version = GetFloatFromString(pValues->Value, 0);
				break;
			case 112:
				CurBot->MakeCampStatus = GetIntFromString(pValues->Value, 0);
				break;
			case 113:
				CurBot->MakeCampX = GetFloatFromString(pValues->Value, 0);
				break;
			case 114:
				CurBot->MakeCampY = GetFloatFromString(pValues->Value, 0);
				break;
			case 115:
				CurBot->MakeCampRadius = GetFloatFromString(pValues->Value, 0);
				break;
			case 116:
				CurBot->MakeCampDistance = GetFloatFromString(pValues->Value, 0);
				break;
			case 117:
				strcpy_s(CurBot->LuaInfo, pValues->Value.c_str());
				break;
			case 118:
				CurBot->EQBC_Packets = GetUInt64FromString(pValues->Value, 0);
			case 119:
				CurBot->EQBC_HeartBeat = GetUInt64FromString(pValues->Value, 0);
			case 120:
				strcpy_s(CurBot->Query, pValues->Value.c_str());
				break;
			case 121:
				CurBot->DetrState = (uint32_t)GetIntFromString(pValues->Value, 0);
				break;
			case 122:
				CurBot->BeneState = (uint32_t)GetIntFromString(pValues->Value, 0);
				break;
			case 123:
				InfoSDura(CurBot, pValues->Value.c_str());
				break;
			case 124:
				InfoPDura(CurBot, pValues->Value.c_str());
				break;
			default:
				break;
		}
		pValues = pValues->pNext;
	}
}

void LoadEvents() {
	Packet.Reset();
	Packet.AddEvent("#*#[NB]#*#|Z=#1#:#2#>#3#|#*#[NB]",                   ParseInfo, (void *)3);	// ZoneID, InstanceID, SpawnID
	Packet.AddEvent("#*#[NB]#*#|L=#4#:#5#|#*#[NB]",                       ParseInfo, (void *)5);	// Level, ClassID
	Packet.AddEvent("#*#[NB]#*#|H=#6#/#7#|#*#[NB]",                       ParseInfo, (void *)7);	// HPCurrent, HPMax
	Packet.AddEvent("#*#[NB]#*#|E=#8#/#9#|#*#[NB]",                       ParseInfo, (void *)9);	// EnduranceCurrent, EnduranceMax
	Packet.AddEvent("#*#[NB]#*#|M=#10#/#11#|#*#[NB]",                     ParseInfo, (void *)11);	// ManaCurrent, ManaMax
	Packet.AddEvent("#*#[NB]#*#|P=#12#:#13#|#*#[NB]",                     ParseInfo, (void *)13);	// PetID, PetHPPct
	Packet.AddEvent("#*#[NB]#*#|T=#14#:#15#|#*#[NB]",                     ParseInfo, (void *)15);	// TargetID, TargetHPPct
	Packet.AddEvent("#*#[NB]#*#|C=#16#|#*#[NB]",                          ParseInfo, (void *)16);	// CastID
	Packet.AddEvent("#*#[NB]#*#|Y=#17#|#*#[NB]",                          ParseInfo, (void *)17);	// State
#if HAS_LEADERSHIP_EXPERIENCE
	Packet.AddEvent("#*#[NB]#*#|X=#18#:#19#:#20#|#*#[NB]",                ParseInfo, (void *)20);	// Exp, AAExp, GroupLeaderExp
#else
	Packet.AddEvent("#*#[NB]#*#|X=#18#:#19#|#*#[NB]",                     ParseInfo, (void *)19);	// Exp, AAExp
#endif
	Packet.AddEvent("#*#[NB]#*#|F=#21#|#*#[NB]",                          ParseInfo, (void *)21);	// FreeBuff
	Packet.AddEvent("#*#[NB]#*#|N=#22#|#*#[NB]",                          ParseInfo, (void *)22);	// Leader
	Packet.AddEvent("#*#[NB]#*#|G=#30#|#*#[NB]",                          ParseInfo, (void *)30);	// Gem
	Packet.AddEvent("#*#[NB]#*#|B=#31#|#*#[NB]",                          ParseInfo, (void *)31);	// Buff
	Packet.AddEvent("#*#[NB]#*#|S=#32#|#*#[NB]",                          ParseInfo, (void *)32);	// Song
	Packet.AddEvent("#*#[NB]#*#|W=#33#|#*#[NB]",                          ParseInfo, (void *)33);	// Pets
	Packet.AddEvent("#*#[NB]#*#|D=#34#|#*#[NB]",                          ParseInfo, (void *)34);	// BDuration
	Packet.AddEvent("#*#[NB]#*#|A=#35#:#36#:#37#|#*#[NB]",                ParseInfo, (void *)37);	// AAPoints, AAPointsSpent, AAPointsAssigned
	Packet.AddEvent("#*#[NB]#*#|O=#38#|#*#[NB]",                          ParseInfo, (void *)38);	// CombatState
	Packet.AddEvent("#*#[NB]#*#|U=#39#|#*#[NB]",                          ParseInfo, (void *)39);	// Note
	Packet.AddEvent("#*#[NB]#*#|R=#40#|#*#[NB]",                          ParseInfo, (void *)40);	// Detrimental
	Packet.AddEvent("#*#[NB]#*#|@=#89#|#*#[NB]",                          ParseInfo, (void *)89);	// Location (and X,Y,Z derived)
	Packet.AddEvent("#*#[NB]#*#|$=#90#|#*#[NB]",                          ParseInfo, (void *)90);	// Heading
	Packet.AddEvent("#*#[NB]#*#|<=#91#|#*#[NB]",                          ParseInfo, (void *)91);	// Extended
	Packet.AddEvent("#*#[NB]#*#|&=#94#:#95#|#*#[NB]",                     ParseInfo, (void *)95);	// MacroState, MacroName
	Packet.AddEvent("#*#[NB]#*#|I=#110#|#*#[NB]",                         ParseInfo, (void *)110);	// FreeInventory
	Packet.AddEvent("#*#[NB]#*#|V=#111#|#*#[NB]",                         ParseInfo, (void *)111);	// Version
	Packet.AddEvent("#*#[NB]#*#|K=#112#:#113#:#114#:#115#:#116#|#*#[NB]", ParseInfo, (void *)116);	// MakeCampStatus, MakeCampX, MakeCampY, MakeCampRadius, MakeCampDistance
	Packet.AddEvent("#*#[NB]#*#|+=#117#|#*#[NB]",                         ParseInfo, (void *)117);	// LuaInfo
	Packet.AddEvent("#*#[NB]#*#|-=#118#:#119#|#*#[NB]",                   ParseInfo, (void *)119);	// EQBC_Packets, EQBC_HeartBeat
	Packet.AddEvent("#*#[NB]#*#|Q=#120#|#*#[NB]",                         ParseInfo, (void *)120);	// Query
	Packet.AddEvent("#*#[NB]#*#|J=#121#:#122#|#*#[NB]",                   ParseInfo, (void *)122);	// DetrState, BeneState
	Packet.AddEvent("#*#[NB]#*#|:=#123#|#*#[NB]",                         ParseInfo, (void *)123);	// SDuration
	Packet.AddEvent("#*#[NB]#*#|;=#124#|#*#[NB]",                         ParseInfo, (void *)124);	// PDuration
}

void LoadBuffMap() {
	BuffMap.clear();
	BuffMap.emplace("Slowed",		BuffData(BT_DETRIMENTAL,	BD_SLOWED,			ST_SLOWED));
	BuffMap.emplace("Rooted",		BuffData(BT_DETRIMENTAL,	BD_ROOTED,			ST_ROOTED));
	BuffMap.emplace("Mesmerized",	BuffData(BT_DETRIMENTAL,	BD_MESMERIZED,		ST_MESMERIZED));
	BuffMap.emplace("Crippled",		BuffData(BT_DETRIMENTAL,	BD_CRIPPLED,		ST_CRIPPLED));
	BuffMap.emplace("Maloed",		BuffData(BT_DETRIMENTAL,	BD_MALOED,			ST_MALOED));
	BuffMap.emplace("Tashed",		BuffData(BT_DETRIMENTAL,	BD_TASHED,			ST_TASHED));
	BuffMap.emplace("Snared",		BuffData(BT_DETRIMENTAL,	BD_SNARED,			ST_SNARED));
	BuffMap.emplace("RevDSed",		BuffData(BT_DETRIMENTAL,	BD_REVDSED,			ST_REVDSED));
	BuffMap.emplace("Charmed",		BuffData(BT_DETRIMENTAL,	BD_CHARMED,			ST_CHARMED));
	BuffMap.emplace("Diseased",		BuffData(BT_DETRIMENTAL,	BD_DISEASED,		ST_DISEASED));
	BuffMap.emplace("Poisoned",		BuffData(BT_DETRIMENTAL,	BD_POISONED,		ST_POISONED));
	BuffMap.emplace("Cursed",		BuffData(BT_DETRIMENTAL,	BD_CURSED,			ST_CURSED));
	BuffMap.emplace("Corrupted",	BuffData(BT_DETRIMENTAL,	BD_CORRUPTED,		ST_CORRUPTED));
	BuffMap.emplace("Blinded",		BuffData(BT_DETRIMENTAL,	BD_BLINDED,			ST_BLINDED));
	BuffMap.emplace("CastingLevel",	BuffData(BT_DETRIMENTAL,	BD_CASTINGLEVEL,	ST_CASTINGLEVEL));
	BuffMap.emplace("EnduDrain",	BuffData(BT_DETRIMENTAL,	BD_ENDUDRAIN,		ST_ENDUDRAIN));
	BuffMap.emplace("Feared",		BuffData(BT_DETRIMENTAL,	BD_FEARED,			ST_FEARED));
	BuffMap.emplace("Healing",		BuffData(BT_DETRIMENTAL,	BD_HEALING,			ST_HEALING));
	BuffMap.emplace("Invulnerable",	BuffData(BT_DETRIMENTAL,	BD_INVULNERABLE,	ST_INVULNERABLE));
	BuffMap.emplace("LifeDrain",	BuffData(BT_DETRIMENTAL,	BD_LIFEDRAIN,		ST_LIFEDRAIN));
	BuffMap.emplace("ManaDrain",	BuffData(BT_DETRIMENTAL,	BD_MANADRAIN,		ST_MANADRAIN));
	BuffMap.emplace("Resistance",	BuffData(BT_DETRIMENTAL,	BD_RESISTANCE,		ST_RESISTANCE));
	BuffMap.emplace("Silenced",		BuffData(BT_DETRIMENTAL,	BD_SILENCED,		ST_SILENCED));
	BuffMap.emplace("SpellCost",	BuffData(BT_DETRIMENTAL,	BD_SPELLCOST,		ST_SPELLCOST));
	BuffMap.emplace("SpellDamage",	BuffData(BT_DETRIMENTAL,	BD_SPELLDAMAGE,		ST_SPELLDAMAGE));
	BuffMap.emplace("SpellSlowed",	BuffData(BT_DETRIMENTAL,	BD_SPELLSLOWED,		ST_SPELLSLOWED));
	BuffMap.emplace("Trigger",		BuffData(BT_DETRIMENTAL,	BD_TRIGGER,			ST_TRIGGER));
	BuffMap.emplace("DSed",			BuffData(BT_BENEFICIAL,		BB_DSED,			ST_DSED));
	BuffMap.emplace("Aego",			BuffData(BT_BENEFICIAL,		BB_AEGO,			ST_AEGO));
	BuffMap.emplace("Skin",			BuffData(BT_BENEFICIAL,		BB_SKIN,			ST_SKIN));
	BuffMap.emplace("Focus",		BuffData(BT_BENEFICIAL,		BB_FOCUS,			ST_FOCUS));
	BuffMap.emplace("Regen",		BuffData(BT_BENEFICIAL,		BB_REGEN,			ST_REGEN));
	BuffMap.emplace("Symbol",		BuffData(BT_BENEFICIAL,		BB_SYMBOL,			ST_SYMBOL));
	BuffMap.emplace("Clarity",		BuffData(BT_BENEFICIAL,		BB_CLARITY,			ST_CLARITY));
	BuffMap.emplace("Pred",			BuffData(BT_BENEFICIAL,		BB_PRED,			ST_PRED));
	BuffMap.emplace("Strength",		BuffData(BT_BENEFICIAL,		BB_STRENGTH,		ST_STRENGTH));
	BuffMap.emplace("Brells",		BuffData(BT_BENEFICIAL,		BB_BRELLS,			ST_BRELLS));
	BuffMap.emplace("SV",			BuffData(BT_BENEFICIAL,		BB_SV,				ST_SV));
	BuffMap.emplace("SE",			BuffData(BT_BENEFICIAL,		BB_SE,				ST_SE));
	BuffMap.emplace("HybridHP",		BuffData(BT_BENEFICIAL,		BB_HYBRIDHP,		ST_HYBRIDHP));
	BuffMap.emplace("Growth",		BuffData(BT_BENEFICIAL,		BB_GROWTH,			ST_GROWTH));
	BuffMap.emplace("Shining",		BuffData(BT_BENEFICIAL,		BB_SHINING,			ST_SHINING));
	BuffMap.emplace("Hasted",		BuffData(BT_BENEFICIAL,		BB_HASTED,			ST_HASTED));
}

PLUGIN_API void InitializePlugin() {
	if (DEBUGGING) {
		DebugSpewAlways("%s->InitializePlugin()", PLUGIN_NAME);
	}
	NetMap.clear();
	LoadEvents();
	LoadBuffMap();
	ZeroMemory(sTimers, sizeof(sTimers));
	ZeroMemory(sBuffer, sizeof(sBuffer));
	ZeroMemory(wBuffer, sizeof(wBuffer));
	ZeroMemory(wChange, sizeof(wChange));
	ZeroMemory(wUpdate, sizeof(wUpdate));
	pNetBotsType = new MQ2NetBotsType;
	NetNote[0] = '\0';
	AddMQ2Data("NetBots", dataNetBots);
	AddCommand("/netbots", Command);
	AddCommand("/netnote", CommandNote);
	AddXMLFile("MQUI_NetBotsWnd.xml");
}

PLUGIN_API void ShutdownPlugin() {
	if (DEBUGGING) {
		DebugSpewAlways("%s->ShutdownPlugin()", PLUGIN_NAME);
	}
	RemoveCommand("/netbots");
	RemoveCommand("/netnote");
	RemoveMQ2Data("NetBots");
	delete pNetBotsType;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//
//  Window Info and Class - Uses custom XML Window - MQUI_NetBotsWnd.xml
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

enum eTextColors {
	TEXT_WHITE 			= 0xFFFFFFFF,
	TEXT_GREEN 			= 0xFF00FF00,
	TEXT_YELLOW			= 0xFFFFFF00,
	TEXT_RED 			= 0xFFFF0000,
	TEXT_SKY_BLUE		= 0xFF87CEEB,
	TEXT_SALMON			= 0xFFFA8072,
	TEXT_DODGER_BLUE	= 0xFF1E90FF
};

enum eListBoxColumns {
	LBC_NAME,
	LBC_LEVEL,
	LBC_NULL_SPACER,
	LBC_CLASS,
	LBC_HP,
	LBC_MANA,
	LBC_ENDUR,
	LBC_DISTANCE,
	LBC_STATE,
	LBC_INVIS,
	LBC_ACTION,
	LBC_TARGET,
	LBCSIZE
};

enum eOldListBoxColumns {
	OLBC_MANA,
	OLBC_ENDUR,
	OLBC_HP,
	OLBC_DISTANCE,
	OLBC_STATE,
	OLBC_NAME,
	OLBC_TARGET,
	OLBC_ACTION,
	OLBCSIZE
};

class CMyWnd : public CCustomWnd {
public:
	CListWnd* List = nullptr;
	int ErrorLoading = 0;
	int wClick = 0;

	CMyWnd() : CCustomWnd(WindowID) {
		List = (CListWnd*)GetChildItem(ScreenID);
		if (!List) {
			ErrorLoading = 1;
			WriteChatf("MQ2NetBots::Could not find %s. Make sure your MQUI_NetBotsWnd.xml is up to date.", ScreenID);
			return;
		}
	}

	void ShowWin() {
		Show(true);
		SetVisible(true);
	}
	
	void HideWin() {
		Show(false);
		SetVisible(false);
	}

	virtual int WndNotification(CXWnd *pWnd, unsigned int Message, void *unknown) override {
		if (pWnd == nullptr) {
			if (Message == XWM_CLOSE) {
				CreateMyWindow();
				ShowWin();
				return 0;
			}
		}
		if (Message == XWM_LCLICK) {
			wClick = NetLClick;
		} else if (Message == XWM_RCLICK) {
			wClick = NetRClick;
		}
		return CSidlScreenWnd::WndNotification(pWnd, Message, unknown);
	};

};

CMyWnd *MyWnd=0;

void CreateMyWindow() {
	if (NetUseNewWindow) {
		strcpy_s(WindowID, "NBClientWnd");
		strcpy_s(ScreenID, "Client_List");
	} else {
		strcpy_s(WindowID, "NetBotsWnd");
		strcpy_s(ScreenID, "NBW_List");
	}
	if (IsScreenPieceLoaded(WindowID)) {
		MyWnd = new CMyWnd;
		if (MyWnd && MyWnd->ErrorLoading) {
			delete MyWnd;
			MyWnd = 0;
		}
		if (MyWnd) {
			ReadWindowINI(MyWnd);
			WriteWindowINI(MyWnd);
			if (NetUseNewWindow) {
				MyWnd->List->SetColumnJustification(LBC_LEVEL, 2);  // Right-justify Level
			}
		}
	} else {
		WriteChatf("MQ2NetBots::Could not find %s. Make sure your MQUI_NetBotsWnd.xml is up to date.", WindowID);
	}
}

void DestroyMyWindow() {
	if (MyWnd) {
		WriteWindowINI(MyWnd);
		MyWnd->List->DeleteAll();
		delete MyWnd;
		MyWnd = 0;
	}
}

void ShowMyWindow() {
	if (gGameState != GAMESTATE_INGAME || !pLocalPlayer) {
		return;
	}
	if (!MyWnd) {
		CreateMyWindow();
		if (!MyWnd) {
			return;
		}
	}
	MyWnd->ShowWin();
	WindowUpdate();
}

void HideMyWindow() {
	if (!MyWnd) {
		return;
	}
	MyWnd->HideWin();
}

void ReadWindowINI(CSidlScreenWnd* pWindow) {
	pWindow->SetEscapable(0);
	pWindow->SetLocation(
		{
			GetPrivateProfileInt(PlayerSectionName, "ChatLeft",   175, GlobalINIFileName),
			GetPrivateProfileInt(PlayerSectionName, "ChatTop",    350, GlobalINIFileName),
			GetPrivateProfileInt(PlayerSectionName, "ChatRight",  675, GlobalINIFileName),
			GetPrivateProfileInt(PlayerSectionName, "ChatBottom", 625, GlobalINIFileName)
		}
	);

	pWindow->SetLocked      ((GetPrivateProfileInt(PlayerSectionName, "Locked",         0, GlobalINIFileName) ? true : false));
	pWindow->SetFades       ((GetPrivateProfileInt(PlayerSectionName, "Fades",          1, GlobalINIFileName) ? true : false));
	pWindow->SetFadeDelay    (GetPrivateProfileInt(PlayerSectionName, "Delay",       2000, GlobalINIFileName));
	pWindow->SetFadeDuration (GetPrivateProfileInt(PlayerSectionName, "Duration",     500, GlobalINIFileName));
	pWindow->SetAlpha        (GetPrivateProfileInt(PlayerSectionName, "Alpha",        255, GlobalINIFileName));
	pWindow->SetFadeToAlpha  (GetPrivateProfileInt(PlayerSectionName, "FadeToAlpha",  255, GlobalINIFileName));
	pWindow->SetBGType       (GetPrivateProfileInt(PlayerSectionName, "BGType",         1, GlobalINIFileName));
	
	ARGBCOLOR col = { 0 };
	col.A = GetPrivateProfileInt(PlayerSectionName, "BGTint.alpha", 255, GlobalINIFileName);
	col.R = GetPrivateProfileInt(PlayerSectionName, "BGTint.red",     0, GlobalINIFileName);
	col.G = GetPrivateProfileInt(PlayerSectionName, "BGTint.green",   0, GlobalINIFileName);
	col.B = GetPrivateProfileInt(PlayerSectionName, "BGTint.blue",    0, GlobalINIFileName);
	pWindow->SetBGColor(col.ARGB);
}

void WriteWindowINI(CSidlScreenWnd* pWindow) {
	char szTemp[MAX_STRING] = { 0 };
	if (pWindow->IsMinimized())	{
		WritePrivateProfileInt(PlayerSectionName, "ChatTop",    pWindow->GetOldLocation().top,    GlobalINIFileName);
		WritePrivateProfileInt(PlayerSectionName, "ChatBottom", pWindow->GetOldLocation().bottom, GlobalINIFileName);
		WritePrivateProfileInt(PlayerSectionName, "ChatLeft",   pWindow->GetOldLocation().left,   GlobalINIFileName);
		WritePrivateProfileInt(PlayerSectionName, "ChatRight",  pWindow->GetOldLocation().right,  GlobalINIFileName);
	} else {
		WritePrivateProfileInt(PlayerSectionName, "ChatTop",    pWindow->GetLocation().top,    GlobalINIFileName);
		WritePrivateProfileInt(PlayerSectionName, "ChatBottom", pWindow->GetLocation().bottom, GlobalINIFileName);
		WritePrivateProfileInt(PlayerSectionName, "ChatLeft",   pWindow->GetLocation().left,   GlobalINIFileName);
		WritePrivateProfileInt(PlayerSectionName, "ChatRight",  pWindow->GetLocation().right,  GlobalINIFileName);
	}
	WritePrivateProfileInt(PlayerSectionName, "Locked",      pWindow->IsLocked(),        GlobalINIFileName);
	WritePrivateProfileInt(PlayerSectionName, "Fades",       pWindow->GetFades(),        GlobalINIFileName);
	WritePrivateProfileInt(PlayerSectionName, "Delay",       pWindow->GetFadeDelay(),    GlobalINIFileName);
	WritePrivateProfileInt(PlayerSectionName, "Duration",    pWindow->GetFadeDuration(), GlobalINIFileName);
	WritePrivateProfileInt(PlayerSectionName, "Alpha",       pWindow->GetAlpha(),        GlobalINIFileName);
	WritePrivateProfileInt(PlayerSectionName, "FadeToAlpha", pWindow->GetFadeToAlpha(),  GlobalINIFileName);
	WritePrivateProfileInt(PlayerSectionName, "BGType",      pWindow->GetBGType(),       GlobalINIFileName);
	ARGBCOLOR col = { 0 };
	col.ARGB = pWindow->GetBGColor();
	WritePrivateProfileInt(PlayerSectionName, "BGTint.alpha", col.A, GlobalINIFileName);
	WritePrivateProfileInt(PlayerSectionName, "BGTint.red",   col.R, GlobalINIFileName);
	WritePrivateProfileInt(PlayerSectionName, "BGTint.green", col.G, GlobalINIFileName);
	WritePrivateProfileInt(PlayerSectionName, "BGTint.blue",  col.B, GlobalINIFileName);
}
	
void WndListPrintf(CListWnd* pWnd, int R, int C, int Color, const char* zFormat, ...) {
	va_list vaList;
	va_start(vaList, zFormat);
	char szTemp[MAX_STRING];
	vsprintf_s(szTemp, MAX_STRING, zFormat, vaList);
	if (pWnd) {
		pWnd->SetItemText(R, C, szTemp);
		pWnd->SetItemColor(R, C, Color);
	}
}

void WndListPrintPerc(CListWnd* pWnd, int R, int C, int Cur, int Max) {
	if (Max < 1) {
		WndListPrintf(pWnd, R, C, TEXT_WHITE, "");
		return;
	}
	int cColor = 0;
	float v = (float)100.0 * Cur / Max;
	if (v > 70) {
		cColor = TEXT_GREEN;
	} else if (v > 40) {
		cColor = TEXT_YELLOW;
	} else {
		cColor = TEXT_RED;
	}
	WndListPrintf(pWnd, R, C, cColor, "%3.0f", v);
}

void WndListPrintDist(CListWnd* pWnd, int R, int C, int Distance) {
	int cColor = 0;
	if (Distance < 30) {
		cColor = TEXT_GREEN;
	} else if (Distance < 65) {
		cColor = TEXT_DODGER_BLUE;
	} else if (Distance < 100) {
		cColor = TEXT_YELLOW;
	} else {
		cColor = TEXT_SALMON;
	}
	WndListPrintf(pWnd, R, C, cColor, "%d", (Distance >= 9999 ? 9999 : Distance));
}

void WindowSetHeight(CSidlScreenWnd* pWindow, int height) {
	if (height != pWindow->GetLocation().bottom - pWindow->GetLocation().top) {
		int width  = pWindow->GetLocation().right - pWindow->GetLocation().left;
		((CXWnd*)(pWindow))->Resize(width, height);
	}
}

void WindowUpdate() {
	static int MaxLines = 0;
	char szOutput[MAX_STRING] = { 0 };
	if (!MyWnd || !MyWnd->List) {
		return;
	}
	ShowTime = MQGetTickCount64() + 100;
	if (MyWnd->IsVisible()) {
		strcpy_s(szOutput, NetWindowTitle);
		ParseMacroData(szOutput, MAX_STRING);
		MyWnd->SetWindowText(szOutput);
		int Distance = 0;
		bool inZone = false;
		bool isSelf = false;
		int cColor = 0;
		int iCount = MyWnd->List->GetItemCount();
		int CurSel = MyWnd->List->GetCurSel();
		int R = 0;
		int C = 0;
		for (auto& [_, botInfo] : NetMap) {
			if (botInfo->SpawnID == 0) {
				continue;
			}
			if (R >= iCount) {
				MyWnd->List->AddString("", TEXT_WHITE, 0, 0);
//				WriteChatf("Added a row. Now: %d", MyWnd->List->GetItemCount());
			}
			if (_stricmp(pLocalPC->Name, botInfo->Name) == 0) {
				isSelf = true;
				inZone = true;
				Distance = 0;
			} else {
				isSelf = false;
				if (pLocalPC && pLocalPC->zoneId == botInfo->ZoneID && pLocalPC->instance == botInfo->InstanceID) {
					inZone = true;
					Distance = static_cast<int>(std::round(Get3DDistance(pLocalPlayer->X, pLocalPlayer->Y, pLocalPlayer->Z, botInfo->X, botInfo->Y, botInfo->Z)));
				} else {
					inZone = false;
					Distance = 0;
				}
			}
			
			// Name
			if (!inZone) {
				cColor = TEXT_SKY_BLUE;
			} else if (Distance > 100) {
				cColor = TEXT_SALMON;
			} else {
				cColor = TEXT_WHITE;
			}
			WndListPrintf(MyWnd->List, R, (NetUseNewWindow ? LBC_NAME : OLBC_NAME), cColor, botInfo->Name);
			
			if (NetUseNewWindow) {
				// Level
				WndListPrintf(MyWnd->List, R, LBC_LEVEL, TEXT_WHITE, "%3d", botInfo->Level);
				// Class
				WndListPrintf(MyWnd->List, R, LBC_CLASS, TEXT_WHITE, pEverQuest->GetClassThreeLetterCode(botInfo->ClassID));
			}

			// HP Percent
			WndListPrintPerc(MyWnd->List, R, (NetUseNewWindow ? LBC_HP : OLBC_HP), botInfo->HPCurrent, botInfo->HPMax);
			// Mana Percent
			WndListPrintPerc(MyWnd->List, R, (NetUseNewWindow ? LBC_MANA : OLBC_MANA), botInfo->ManaCurrent, botInfo->ManaMax);
			// Endurance Percent
			WndListPrintPerc(MyWnd->List, R, (NetUseNewWindow ? LBC_ENDUR : OLBC_ENDUR), botInfo->EnduranceCurrent, botInfo->EnduranceMax);

			// Distance
			C = (NetUseNewWindow ? LBC_DISTANCE : OLBC_DISTANCE);
			if (!inZone) {
				WndListPrintf(MyWnd->List, R, C, TEXT_SKY_BLUE, " ----");
			} else if (isSelf) {
				WndListPrintf(MyWnd->List, R, C, TEXT_WHITE, "");
			} else {
				WndListPrintDist(MyWnd->List, R, C, Distance);
			}

			// State
			sprintf_s(szOutput, " ");
			cColor = TEXT_WHITE;
			if (botInfo->State & STATE_DEAD) {
				szOutput[0] = 'x';
				cColor = TEXT_RED;
			} else if (botInfo->State & STATE_STUN) {
				szOutput[0] = '-';
				cColor = TEXT_RED;
			} else if (!NetUseNewWindow && (botInfo->State & STATE_INVIS) && (botInfo->State & STATE_ITU)) {
				szOutput[0] = 'B';
			} else if (!NetUseNewWindow && (botInfo->State & STATE_INVIS)) {
				szOutput[0] = 'I';
			} else if (!NetUseNewWindow && (botInfo->State & STATE_ITU)) {
				szOutput[0] = 'U';
			} else if (botInfo->State & STATE_SIT) {
				szOutput[0] = 'S';
			} else if (botInfo->State & STATE_HAVEAGGRO) {
				szOutput[0] = 'T';
			} else if (botInfo->State & STATE_WANTAGGRO) {
				szOutput[0] = 'A';
			} else if (botInfo->State & STATE_MOUNT) {
				szOutput[0] = 'M';
			}
			WndListPrintf(MyWnd->List, R, (NetUseNewWindow ? LBC_STATE : OLBC_STATE), cColor, szOutput);

			if (NetUseNewWindow) {
				// Invis
				if ((botInfo->State & STATE_INVIS) && (botInfo->State & STATE_ITU)) {
					WndListPrintf(MyWnd->List, R, LBC_INVIS, TEXT_DODGER_BLUE, "BOTH");
				} else if (botInfo->State & STATE_INVIS) {
					WndListPrintf(MyWnd->List, R, LBC_INVIS, TEXT_GREEN, "INV");
				} else if (botInfo->State & STATE_ITU) {
					WndListPrintf(MyWnd->List, R, LBC_INVIS, TEXT_YELLOW, "IVU");
				} else {
					WndListPrintf(MyWnd->List, R, LBC_INVIS, TEXT_WHITE, "");
				}
			}

			// Action
			C = (NetUseNewWindow ? LBC_ACTION : OLBC_ACTION);
			if (botInfo->CastID) {
				WndListPrintf(MyWnd->List, R, C, TEXT_WHITE, GetSpellByID(botInfo->CastID)->Name);
			} else if (botInfo->State & STATE_ATTACK) {
				WndListPrintf(MyWnd->List, R, C, TEXT_WHITE, "Melee");
			} else if (botInfo->State & STATE_RANGED) {
				WndListPrintf(MyWnd->List, R, C, TEXT_WHITE, "Ranged");
			} else {
				WndListPrintf(MyWnd->List, R, C, TEXT_WHITE, "");
			}

			// Target
			char* p = szOutput;
			szOutput[0] = 0;
			cColor = TEXT_WHITE;
			if (inZone && botInfo->TargetID) {
				auto pBotTarget = GetSpawnByID(botInfo->TargetID);
				if (!pBotTarget) {
					sprintf_s(szOutput, "id %d", botInfo->TargetID);
				} else {
					if (GetSpawnType(pBotTarget)==NPC) {
						if (NetCleanNames) {
							sprintf_s(szOutput, "%s", pBotTarget->DisplayedName);
						} else {
							sprintf_s(szOutput, "%s", pBotTarget->Name);
						}
						if (NetConColors) {
							cColor = ConColorToARGB(ConColor(pBotTarget));
						}
					} else {
						sprintf_s(szOutput, "%s", pBotTarget->DisplayedName);
					}
				}
				if (NetRightMost > 0) {
					int zlen = static_cast<int>(strlen(szOutput));
					if (zlen > NetRightMost) {
						p = &szOutput[zlen - NetRightMost];
					}
				}
			}
			WndListPrintf(MyWnd->List, R, (NetUseNewWindow ? LBC_TARGET : OLBC_TARGET), cColor, p);

			// If we have selected someone from the list, take action
			if (CurSel == R) {
				switch (MyWnd->wClick) {
					case CA_NO_ACTION:
						break;
					case CA_BRING_TO_FOREGROUND:
						if (_stricmp(pLocalPC->Name, botInfo->Name) != 0) {
							sprintf_s(szOutput, "/bct %s //foreground", botInfo->Name);
							EzCommand(szOutput);
						}
						break;
					case CA_TARGET_PLAYER:
						if (inZone && botInfo->SpawnID) {
							sprintf_s(szOutput, "/mqtarget id %d", botInfo->SpawnID);
							EzCommand(szOutput);
						}
						break;
					case CA_TARGET_PLAYER_TARGET:
						if (inZone && botInfo->TargetID) {
							sprintf_s(szOutput, "/mqtarget id %d", botInfo->TargetID);
							EzCommand(szOutput);
						}
						break;
					default:
						break;
				}
			}
			R++;
		}
		// If we lost clients, clear the old data. Leave the rows, we will adjust height to hide the empty ones.
		// No need to constantly delete and re-add. Especially since the clients disappear/reappear when they zone.
		int cSize = (NetUseNewWindow ? LBCSIZE : OLBCSIZE);
		for (int i = R; i < MaxLines; i++) {
			for (int C = 0; C < cSize; C++) {
				WndListPrintf(MyWnd->List, i, C, TEXT_WHITE, "");
			}
		}
		WindowSetHeight(MyWnd, 50 + 14 * R);
		MaxLines = R;
	}
	MyWnd->List->SetCurSel(-1);
}

PLUGIN_API void OnCleanUI() {
	DestroyMyWindow();
}

PLUGIN_API void OnReloadUI() {
	if (gGameState == GAMESTATE_INGAME && pLocalPlayer) {
		if (NetShow) {
			ShowMyWindow();
		}
	}
}