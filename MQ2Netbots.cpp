//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Projet: MQ2NetBots.cpp   | Beta (Need Fixes i posted on mq2eqbc forums)
// Author: s0rCieR          |
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// 
// Deadchicken added .Duration on or about September 2007 and tried not to 
// mangle s0rCieR's code to much.  Thanks to Kroak for helping debug. 
//  Updated for 10/9 release of MQ2
// CombatState member added  Thanks mijuki.
// .Stacks member added
// 
//v2.1 Added String Safety - eqmule

#define        PLUGIN_NAME "MQ2NetBots"
#define        PLUGIN_DATE     20160727
#define        PLUGIN_VERS        2.1

#define        GEMS_MAX              NUM_SPELL_GEMS
#define        BUFF_MAX              NUM_LONG_BUFFS
#define        SONG_MAX              NUM_SHORT_BUFFS
#define        PETS_MAX              65 
#define        NOTE_MAX		    500
#define        DETR_MAX		     30

#define        NETTICK               50
#define        REFRESH            60000
#define        UPDATES             6000

#define        DEBUGGING             0

#ifndef PLUGIN_API
  #include "../MQ2Plugin.h"
using namespace std;
  PreSetup(PLUGIN_NAME);
  PLUGIN_VERSION(PLUGIN_VERS);
  #include <string>
  #include <map>
  #include "../Blech/Blech.h"
#endif PLUGIN_API

enum { STATE_DEAD =0x0001, STATE_FEIGN =0x0002, STATE_DUCK  =0x0004, STATE_BIND =0x0008,
       STATE_STAND=0x0010, STATE_SIT   =0x0020, STATE_MOUNT =0x0040, STATE_INVIS=0x0080,
       STATE_LEV  =0x0100, STATE_ATTACK=0x0200, STATE_MOVING=0x0400, STATE_STUN =0x0800,
       STATE_RAID =0x1000, STATE_GROUP =0x2000, STATE_LFG   =0x4000, STATE_AFK  =0x8000 };

enum { BUFFS,CASTD,ENDUS,EXPER,LEADR,LEVEL,LIFES,MANAS, 
       PBUFF,PETIL,SPGEM,SONGS,STATE,TARGT,ZONES,DURAS, 
       AAPTS,OOCST,NOTE ,DETR ,ESIZE };

enum { RESERVED,DETRIMENTALS,COUNTERS,BLINDED,CASTINGLEVEL,CHARMED,CURSED,DISEASED,ENDUDRAIN,
       FEARED,HEALING,INVULNERABLE,LIFEDRAIN,MANADRAIN,MESMERIZED,POISONED,RESISTANCE,ROOTED,
       SILENCED,SLOWED,SNARED,SPELLCOST,SPELLDAMAGE,SPELLSLOWED,TRIGGR,CORRUPTED,NOCURE, DSIZE }; 

class BotInfo {
public:
	CHAR              Name[0x40];          // Client NAME
	CHAR              Leader[0x40];        // Leader Name
	WORD              State;               // State
	long              ZoneID;              // Zone ID
	long              InstID;              // Instance ID
	long              SpawnID;             // Spawn ID
	long              ClassID;             // Class ID
	long              Level;               // Level
	long              CastID;              // Casting Spell ID
	long              LifeCur;             // HP Current
	long              LifeMax;             // HP Maximum
	long              EnduCur;             // ENDU Current
	long              EnduMax;             // ENDU Maximum
	long              ManaCur;             // MANA Current
	long              ManaMax;             // MANA Maximum
	long              PetID;               // PET ID
	long              PetHP;               // PET HP Percentage
	long              TargetID;            // Target ID
	long              TargetHP;            // Target HP Percentage
//	long              Gem[GEMS_MAX];       // Spell Memorized
	long              Pets[PETS_MAX];      // Spell Pets
	long              Song[SONG_MAX];      // Spell Song
	long              Buff[BUFF_MAX];      // Spell Buff
//	long              Duration[BUFF_MAX];  // Buff duration 
	long              FreeBuff;            // FreeBuffSlot;
//	double            glXP;                // glXP
	DWORD             aaXP;                // aaXP
	DWORD             XP;                  // XP
	DWORD             Updated;             // Update
	long              TotalAA;             // totalAA 
	long              UsedAA;              // usedAA 
	long              UnusedAA;            // unusedAA 
	DWORD             CombatState;         // CombatState 
	CHAR              Note[NOTE_MAX];      // User Mesg
        int               Detrimental[DSIZE];
};

long                NetInit=0;           // Plugin Initialized?
long                NetStat=0;           // Plugin is On?
long                NetGrab=0;           // Grab Information?
long                NetSend=0;           // Send Information?
long                NetLast=0;           // Last Send Time Mark
char                NetNote[NOTE_MAX];   // Network Note

map<string,BotInfo> NetMap;              // BotInfo Mapped List
Blech               Packet('#');         // BotInfo Event Triggers
BotInfo            *CurBot=0;            // BotInfo Current

long                sTimers[ESIZE];      // Save Timers
char                sBuffer[ESIZE][256]; // Save Buffer
char                wBuffer[ESIZE][256]; // Work Buffer
bool                wChange[ESIZE];      // Work Change
bool                wUpdate[ESIZE];      // Work Update

int                 dValues[DSIZE];

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

bool EQBCConnected() {
  typedef WORD (__cdecl *fEqbcIsConnected)(VOID);
  PMQPLUGIN pLook=pPlugins;
  while(pLook && _strnicmp(pLook->szFilename,"mq2eqbc",8)) pLook=pLook->pNext;
  if(pLook)
    if(fEqbcIsConnected checkf=(fEqbcIsConnected)GetProcAddress(pLook->hModule,"isConnected"))
      if(checkf()) return true;
  return false;
}

void EQBCBroadCast(PCHAR Buffer) {
  typedef VOID (__cdecl *fEqbcNetBotSendMsg)(PCHAR);
  if(strlen(Buffer)>9) {
    PMQPLUGIN pLook=pPlugins;
    while(pLook && _strnicmp(pLook->szFilename,"mq2eqbc",8)) pLook=pLook->pNext;
    if(pLook)
      if(fEqbcNetBotSendMsg requestf=(fEqbcNetBotSendMsg)GetProcAddress(pLook->hModule,"NetBotSendMsg")) {
        #if    DEBUGGING>1
          DebugSpewAlways("%s->BroadCasting(%s)",PLUGIN_NAME,Buffer);
        #endif DEBUGGING
        requestf(Buffer);
      }
  }
}

BotInfo* BotFind(PCHAR Name) {
  map<string,BotInfo>::iterator f=NetMap.find(Name);
  return(NetMap.end()==f)?NULL:&(*f).second;
}

BotInfo* BotLoad(PCHAR Name) {
  map<string,BotInfo>::iterator f=NetMap.find(Name);
  if(NetMap.end()==f) {
    BotInfo RecInfo;
    ZeroMemory(&RecInfo.Name,sizeof(BotInfo));
    strcpy_s(RecInfo.Name,Name);
    NetMap.insert(map<string,BotInfo>::value_type(RecInfo.Name,RecInfo));
    f=NetMap.find(Name);
  }
  return &(*f).second;
}

void BotQuit(PCHAR Name) {
  map<string,BotInfo>::iterator f=NetMap.find(Name);
  if(NetMap.end()!=f) NetMap.erase(f);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

bool inGroup(unsigned long ID) {
  PSPAWNINFO pID=(PSPAWNINFO)GetSpawnByID(ID);
  if(pID && pID->MasterID) pID=(PSPAWNINFO)GetSpawnByID(pID->MasterID);
  if(pID && GetCharInfo() && GetCharInfo()->pSpawn) {
    if(GetCharInfo()->pSpawn->SpawnID==pID->SpawnID)   return true;
    if(GetCharInfo()->pGroupInfo) for(int G=1; G<6; G++) 
      if(PSPAWNINFO pSpawn=GetGroupMember(G)) 
        if(pID->SpawnID==pSpawn->SpawnID)  return true;
  }
  return false;
}

bool inZoned(unsigned long zID, unsigned long iID) {
  return (GetCharInfo() && GetCharInfo()->zoneId==zID && GetCharInfo()->instance==iID);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void InfoSong(PCHAR Line) {
  char Buf[MAX_STRING];
  for(long Idx=0; Idx<SONG_MAX; Idx++) {
    GetArg(Buf,Line,Idx+1,FALSE,FALSE,FALSE,':');
    CurBot->Song[Idx]=atol(Buf);
  }
}


void InfoPets(PCHAR Line) {
  char Buf[MAX_STRING];
  for(long Idx=0; Idx<PETS_MAX; Idx++) {
    GetArg(Buf,Line,Idx+1,FALSE,FALSE,FALSE,':');
    CurBot->Pets[Idx]=atol(Buf);
  }
}

/*
void InfoGems(PCHAR Line) {
  char Buf[MAX_STRING];
  for(long Idx=0; Idx<GEMS_MAX; Idx++) {
    GetArg(Buf,Line,Idx+1,FALSE,FALSE,FALSE,':');
    CurBot->Gem[Idx]=atol(Buf);
  }
}
*/

void InfoBuff(PCHAR Line) {
  char Buf[MAX_STRING];
  for(long Idx=0; Idx<BUFF_MAX; Idx++) {
    GetArg(Buf,Line,Idx+1,FALSE,FALSE,FALSE,':');
    CurBot->Buff[Idx]=atol(Buf);
  }
}

/*
void InfoDura(PCHAR Line) {
  char Buf[MAX_STRING];
  for(long Idx=0; Idx<BUFF_MAX; Idx++) {
//  WriteChatf("We got=%s", Line);
    GetArg(Buf,Line,Idx+1,FALSE,FALSE,FALSE,':');
    CurBot->Duration[Idx]=atol(Buf);
//   WriteChatf("Set Duration to %s", Buf);
//   WriteChatf("CurBot->Duration is %d", CurBot->Duration[Idx]);
  }
}
*/

void InfoDetr(PCHAR Line) {
  char Buf[MAX_STRING];
  for(long Idx=0; Idx<DSIZE; Idx++) {
    GetArg(Buf,Line,Idx+1,FALSE,FALSE,FALSE,':');
    CurBot->Detrimental[Idx]=atoi(Buf);
  }
}

int SlotCalculate(PSPELL spell, int slot) {
    char Buf[MAX_STRING]={0};
    SlotValueCalculate(Buf,spell,slot,1);
    return atoi(Buf);
}

void __stdcall ParseInfo(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  if(CurBot) while(pValues) {
    //WriteChatf("Parsing=%s", pValues->Name);
    switch(atoi(pValues->Name)) {
      case  1: CurBot->ZoneID   =atol(pValues->Value);        break;
      case  2: CurBot->InstID   =atol(pValues->Value);        break;
      case  3: CurBot->SpawnID  =atol(pValues->Value);        break;
      case  4: CurBot->Level    =atol(pValues->Value);        break;
      case  5: CurBot->ClassID  =atol(pValues->Value);        break;
      case  6: CurBot->LifeCur  =atol(pValues->Value);        break;
      case  7: CurBot->LifeMax  =atol(pValues->Value);        break;
      case  8: CurBot->EnduCur  =atol(pValues->Value);        break;
      case  9: CurBot->EnduMax  =atol(pValues->Value);        break;
      case 10: CurBot->ManaCur  =atol(pValues->Value);        break;
      case 11: CurBot->ManaMax  =atol(pValues->Value);        break;
      case 12: CurBot->PetID    =atol(pValues->Value);        break;
      case 13: CurBot->PetHP    =atol(pValues->Value);        break;
      case 14: CurBot->TargetID =atol(pValues->Value);        break;
      case 15: CurBot->TargetHP =atol(pValues->Value);        break;
      case 16: CurBot->CastID   =atol(pValues->Value);        break;
      case 17: CurBot->State    =(WORD)atol(pValues->Value);  break;
      case 18: CurBot->XP       =(DWORD)atol(pValues->Value); break;
      case 19: CurBot->aaXP     =(DWORD)atol(pValues->Value); break;
//    case 20: CurBot->glXP     =atof(pValues->Value);        break;
      case 21: CurBot->FreeBuff =atol(pValues->Value);        break;
      case 22: strcpy_s(CurBot->Leader,pValues->Value);         break;
//    case 30: InfoGems(pValues->Value);                      break;
      case 31: InfoBuff(pValues->Value);                      break;
      case 32: InfoSong(pValues->Value);                      break;
      case 33: InfoPets(pValues->Value);                      break;
//    case 34: InfoDura(pValues->Value);                      break;
      case 35: CurBot->TotalAA  =atol(pValues->Value);        break; 
      case 36: CurBot->UsedAA   =atol(pValues->Value);        break; 
      case 37: CurBot->UnusedAA =atol(pValues->Value);        break; 
      case 38: CurBot->CombatState=atol(pValues->Value);      break; 
      case 39: strcpy_s(CurBot->Note,pValues->Value);	      break;
      case 40: InfoDetr(pValues->Value);                      break; 
    }
    pValues=pValues->pNext;
  }
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

/*
PSTR MakeDURAS(CHAR(&Buffer)[_Size]) {
  long Duration; char tmp[MAX_STRING]; Buffer[0]=0;
  for(int b=0; b<BUFF_MAX; b++)
    if((Duration=GetCharInfo2()->Buff[b].Duration)>0) {
      sprintf_s(tmp,"%d:",Duration);
//     WriteChatf("Spell %d:%d", GetCharInfo2()->Buff[b].SpellID, GetCharInfo2()->Buff[b].Duration );
      strcat_s(Buffer,tmp);
    }

  return Buffer;
}
*/

template <unsigned int _Size>PSTR MakeBUFFS(CHAR(&Buffer)[_Size]) {
  long SpellID; char tmp[MAX_STRING]; Buffer[0]=0;
  for(int b=0; b<BUFF_MAX; b++)
    if((SpellID=GetCharInfo2()->Buff[b].SpellID)>0) {
      sprintf_s(tmp,"%d:",SpellID);
      strcat_s(Buffer,tmp);
    }
  sprintf_s(tmp,"|F=${Me.FreeBuffSlots}");
  ParseMacroData(tmp, sizeof(tmp));
  strcat_s(Buffer,tmp);
  return Buffer;
}

template <unsigned int _Size>PSTR MakeCASTD(CHAR(&Buffer)[_Size]) {
  long Casting=GetCharInfo()->pSpawn->CastingData.SpellID;
  if(Casting>0)
	  _itoa_s(Casting,Buffer,10);
  else
	  Buffer[0]=0;
  return Buffer;
}

template <unsigned int _Size>PSTR MakeENDUS(CHAR(&Buffer)[_Size]) {
  if(long EnduMax=GetMaxEndurance()) sprintf_s(Buffer,"%d/%d",GetCharInfo2()->Endurance,EnduMax);
  else strcpy_s(Buffer,"/");
  return Buffer;
}

template <unsigned int _Size>PSTR MakeEXPER(CHAR(&Buffer)[_Size]) {
  sprintf_s(Buffer,"%I64d:%d",GetCharInfo()->Exp,GetCharInfo()->AAExp);
  return Buffer;
}

template <unsigned int _Size>PSTR MakeLEADR(CHAR(&Buffer)[_Size]) {
  if (GetCharInfo()->pGroupInfo) 
   GetCXStr(GetCharInfo()->pGroupInfo->pLeader->pName,Buffer,MAX_STRING); 
  else 
    Buffer[0] = 0; 
  return Buffer;
}

template <unsigned int _Size>PSTR MakeNOTE(CHAR(&Buffer)[_Size]) {
	strcpy_s(Buffer,NetNote);
	return Buffer;
}

template <unsigned int _Size>PSTR MakeLEVEL(CHAR(&Buffer)[_Size]) {
  sprintf_s(Buffer,"%d:%d",GetCharInfo2()->Level,GetCharInfo2()->Class);
  return Buffer;
}

template <unsigned int _Size>PSTR MakeLIFES(CHAR(&Buffer)[_Size]) {
  sprintf_s(Buffer,"%d/%d",GetCurHPS(),GetMaxHPS());
  return Buffer;
}

template <unsigned int _Size>PSTR MakeMANAS(CHAR(&Buffer)[_Size]) {
  if(long ManaMax=GetMaxMana()) sprintf_s(Buffer,"%d/%d",GetCharInfo2()->Mana,ManaMax);
  else strcpy_s(Buffer,"/");
  return Buffer;
}

template <unsigned int _Size>PSTR MakePBUFF(CHAR(&Buffer)[_Size]) {
  long SpellID; char tmp[MAX_STRING]; Buffer[0]=0;
  PSPAWNINFO Pet=(PSPAWNINFO)GetSpawnByID(GetCharInfo()->pSpawn->PetID);
  for(int b=0; b<PETS_MAX; b++)
    if((SpellID=(Pet && pPetInfoWnd)?((PEQPETINFOWINDOW)pPetInfoWnd)->Buff[b]:0)>0) {
      sprintf_s(tmp,"%d:",SpellID);
      strcat_s(Buffer,tmp);
    }
//  WriteChatf("MakePBUFF: [%s]", Buffer);
  return Buffer;
}

template <unsigned int _Size>PSTR MakePETIL(CHAR(&Buffer)[_Size]) {
  PSPAWNINFO Pet=(PSPAWNINFO)GetSpawnByID(GetCharInfo()->pSpawn->PetID);
  if(pPetInfoWnd && Pet) sprintf_s(Buffer,"%d:%d",Pet->SpawnID,(Pet->HPCurrent*100/Pet->HPMax));
  else strcpy_s(Buffer,":");
  return Buffer;
}

/*
PSTR MakeSPGEM(CHAR(&Buffer)[_Size]) {
  long SpellID; char tmp[32]; Buffer[0]=0;
  for(int g=0; g<GEMS_MAX; g++)
    if((SpellID=(pCastSpellWnd && GetMaxMana()>0)?GetCharInfo2()->MemorizedSpells[g]:0)>0) {
      sprintf_s(tmp,"%d:",SpellID);
      strcat_s(Buffer,tmp);
    }
  return Buffer;
}
*/

template <unsigned int _Size>PSTR MakeSONGS(CHAR(&Buffer)[_Size]) {
  long SpellID; char tmp[MAX_STRING]; Buffer[0]=0;
  for(int b=0; b<SONG_MAX; b++)
    if((SpellID=GetCharInfo2()->ShortBuff[b].SpellID)>0) {
      sprintf_s(tmp,"%d:",SpellID);
      strcat_s(Buffer,tmp);
    }
  return Buffer;
}

template <unsigned int _Size>PSTR MakeSTATE(CHAR(&Buffer)[_Size]) {
  WORD Status=0;
  if(*EQADDR_ATTACK)                                      Status |= STATE_ATTACK;
  if(pRaid && pRaid->RaidMemberCount)                     Status |= STATE_RAID;
  if(GetCharInfo()->Stunned)                              Status |= STATE_STUN;
  if(GetCharInfo()->pGroupInfo)                           Status |= STATE_GROUP;
  if(FindSpeed(GetCharInfo()->pSpawn))                    Status |= STATE_MOVING;
  if(GetCharInfo()->pSpawn->Mount)                        Status |= STATE_MOUNT;
  if(GetCharInfo()->pSpawn->AFK)                          Status |= STATE_AFK;
  if(GetCharInfo()->pSpawn->HideMode)                     Status |= STATE_INVIS;
  if(GetCharInfo()->pSpawn->mPlayerPhysicsClient.Levitate)                     Status |= STATE_LEV;
  if(GetCharInfo()->pSpawn->LFG)                          Status |= STATE_LFG;
  if(GetCharInfo()->pSpawn->StandState==STANDSTATE_DEAD)  Status |= STATE_DEAD;
  if(GetCharInfo()->pSpawn->StandState==STANDSTATE_FEIGN) Status |= STATE_FEIGN;
  if(GetCharInfo()->pSpawn->StandState==STANDSTATE_DUCK)  Status |= STATE_DUCK;
  if(GetCharInfo()->pSpawn->StandState==STANDSTATE_BIND)  Status |= STATE_BIND;
  if(GetCharInfo()->pSpawn->StandState==STANDSTATE_STAND) Status |= STATE_STAND;
  if(GetCharInfo()->pSpawn->StandState==STANDSTATE_SIT)   Status |= STATE_SIT;
  _itoa_s(Status,Buffer,10);
  return Buffer;
}
 
template <unsigned int _Size>PSTR MakeOOCST(CHAR(&Buffer)[_Size]) { 
   _itoa_s(((PCPLAYERWND) pPlayerWnd)->CombatState,Buffer,10); 
   return Buffer; 
} 

template <unsigned int _Size>PSTR MakeAAPTS(CHAR(&Buffer)[_Size]) { 
   sprintf_s(Buffer,"%d:%d:%d",GetCharInfo2()->AAPoints+GetCharInfo2()->AAPointsSpent,GetCharInfo2()->AAPointsSpent,GetCharInfo2()->AAPoints); 
   return Buffer; 
} 

template <unsigned int _Size>PSTR MakeTARGT(CHAR(&Buffer)[_Size]) {
  PSPAWNINFO Tar=pTarget?((PSPAWNINFO)pTarget):NULL;
  if(Tar) sprintf_s(Buffer,"%d:%d",Tar->SpawnID,(Tar->HPCurrent*100/Tar->HPMax));
  else strcpy_s(Buffer,":");
  return Buffer;
}

template <unsigned int _Size>PSTR MakeZONES(CHAR(&Buffer)[_Size]) {
  sprintf_s(Buffer,"%d:%d>%d",GetCharInfo()->zoneId,GetCharInfo()->instance,GetCharInfo()->pSpawn->SpawnID);
  return Buffer;
}

template <unsigned int _Size>PSTR MakeDETR(CHAR(&Buffer)[_Size]) {
  char tmp[MAX_STRING]; Buffer[0]=0;
  ZeroMemory(&dValues,sizeof(dValues));
  for(int b=0; b<BUFF_MAX; b++) {
    if(PSPELL spell=GetSpellByID(GetCharInfo2()->Buff[b].SpellID)) {
      if(!spell->SpellType) {
        bool d=false;
        bool r=false;
        for(int s=0; s<GetSpellNumEffects(spell); s++) switch(GetSpellAttrib(spell, s)) {
          case   0: if(GetSpellBase(spell, s) < 0) { dValues[LIFEDRAIN]+=SlotCalculate(spell,s); d=true; } break;
          case   3: if(GetSpellBase(spell, s) < 0) { dValues[SNARED]++; d=true; } break;
          case  11: if(((GetSpellMax(spell, s))?GetSpellMax(spell, s):GetSpellBase(spell, s))-100<0) { dValues[SLOWED]++; d=true; } break;
          case  15: if(GetSpellBase(spell, s) < 0) { dValues[MANADRAIN]+=SlotCalculate(spell,s); d=true; } break;
          case  20: dValues[BLINDED]++; d=true; break;
          case  22: dValues[CHARMED]++;  d=true; break;
          case  23: dValues[FEARED]++; d=true; break;
          case  31: dValues[MESMERIZED]++; d=true; break;
          case  35: dValues[DISEASED]+=(int)GetSpellBase(spell, s); d=true; break;
          case  36: dValues[POISONED]+=(int)GetSpellBase(spell, s); d=true; break;
          case  40: dValues[INVULNERABLE]++; d=true; break;
          case  46: if(GetSpellBase(spell, s) < 0) { r=true; d=true; } break;
          case  47: if(GetSpellBase(spell, s) < 0) { r=true; d=true; } break;
          case  48: if(GetSpellBase(spell, s) < 0) { r=true; d=true; } break;
          case  49: if(GetSpellBase(spell, s) < 0) { r=true; d=true; } break;
          case  50: if(GetSpellBase(spell, s) < 0) { r=true; d=true; } break;
          case  96: dValues[SILENCED]++; d=true; break;
          case  99: dValues[ROOTED]++; d=true; break;
          case 112: if(GetSpellBase(spell, s) < 0) { dValues[CASTINGLEVEL]++; d=true; } break;
          case 116: dValues[CURSED]+=(int)GetSpellBase(spell, s); d=true; break;
          case 120: if(GetSpellBase(spell, s) < 0) { dValues[HEALING]++; d=true; } break;
          case 124: if(GetSpellBase(spell, s) < 0) { dValues[SPELLDAMAGE]++; d=true; } break;
          case 127: if(GetSpellBase(spell, s) < 0) { dValues[SPELLSLOWED]++; d=true; } break;
          case 132: if(GetSpellBase(spell, s) < 0) { dValues[SPELLCOST]++; d=true; } break;
          case 189: if(GetSpellBase(spell, s) < 0) { dValues[ENDUDRAIN]+=SlotCalculate(spell,s); d=true; } break;
          case 289: dValues[TRIGGR]++; d=true; break;
	  case 369: dValues[CORRUPTED]+=(int)GetSpellBase(spell, s); d=true; break;
        }
        if(d) {
          dValues[DETRIMENTALS]++;
          if (spell->NoDisspell && !spell->WearOff[0] || spell->TargetType == 6) dValues[NOCURE]++;
        }
        if(r) dValues[RESISTANCE]++;
      }
    }
  }
  dValues[COUNTERS]=dValues[CURSED]+dValues[DISEASED]+dValues[POISONED]+dValues[CORRUPTED];
  for(int a=0; a<DSIZE; a++) {
    sprintf_s(tmp,"%d:",dValues[a]);
    strcat_s(Buffer,tmp);
  }
  return Buffer;
}

void BroadCast() {
  char Buffer[MAX_STRING];
  long nChange=false;
  long nUpdate=false;
  ZeroMemory(wBuffer,sizeof(wBuffer));
  ZeroMemory(wChange,sizeof(wChange));
  ZeroMemory(wUpdate,sizeof(wUpdate));
  sprintf_s(wBuffer[BUFFS],"B=%s|",MakeBUFFS(Buffer));
  sprintf_s(wBuffer[CASTD],"C=%s|",MakeCASTD(Buffer));
  sprintf_s(wBuffer[ENDUS],"E=%s|",MakeENDUS(Buffer));
  sprintf_s(wBuffer[EXPER],"X=%s|",MakeEXPER(Buffer));
  sprintf_s(wBuffer[LEADR],"N=%s|",MakeLEADR(Buffer));
  sprintf_s(wBuffer[LEVEL],"L=%s|",MakeLEVEL(Buffer));
  sprintf_s(wBuffer[LIFES],"H=%s|",MakeLIFES(Buffer));
  sprintf_s(wBuffer[MANAS],"M=%s|",MakeMANAS(Buffer));
  sprintf_s(wBuffer[PBUFF],"W=%s|",MakePBUFF(Buffer));
  sprintf_s(wBuffer[PETIL],"P=%s|",MakePETIL(Buffer));
//  sprintf_s(wBuffer[SPGEM],"G=%s|",MakeSPGEM(Buffer));
  sprintf_s(wBuffer[SONGS],"S=%s|",MakeSONGS(Buffer));
  sprintf_s(wBuffer[STATE],"Y=%s|",MakeSTATE(Buffer));
  sprintf_s(wBuffer[TARGT],"T=%s|",MakeTARGT(Buffer));
  sprintf_s(wBuffer[ZONES],"Z=%s|",MakeZONES(Buffer));
//  sprintf_s(wBuffer[DURAS],"D=%s|",MakeDURAS(Buffer));
  sprintf_s(wBuffer[AAPTS],"A=%s|",MakeAAPTS(Buffer));
  sprintf_s(wBuffer[OOCST],"O=%s|",MakeOOCST(Buffer)); 
  sprintf_s(wBuffer[NOTE] ,"U=%s|",MakeNOTE(Buffer));
  sprintf_s(wBuffer[DETR] ,"R=%s|",MakeDETR(Buffer));


  for(int i=0; i<ESIZE; i++)
    if((clock()>sTimers[i] && clock()>sTimers[i]+UPDATES) || 0!=strcmp(wBuffer[i],sBuffer[i])) {
      wChange[i]=true;
      nChange++;
    } else if(clock()<sTimers[i] && clock()+UPDATES>sTimers[i]) {
      wUpdate[i]=true;
      nUpdate++;
    }
  if(nChange) {
    strcpy_s(Buffer,"[NB]|");
    for(int i=0; i<ESIZE ; i++) 
      if(wChange[i] || wUpdate[i] && (strlen(Buffer)+strlen(wBuffer[i]))<MAX_STRING-5) {
        strcat_s(Buffer,wBuffer[i]);
        sTimers[i]=(long)clock()+REFRESH;
      }
    strcat_s(Buffer,"[NB]");
 //   DebugSpewAlways("MQ2NetBots::Broadcast(%d/%d) %s",strlen(Buffer),MAX_STRING,Buffer);

    EQBCBroadCast(Buffer);
    memcpy(sBuffer,wBuffer,sizeof(wBuffer));
    if(CurBot=BotLoad(GetCharInfo()->Name)) {
      Packet.Feed(Buffer);
      CurBot->Updated=clock();
      CurBot=0;
    }
  }
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

class MQ2NetBotsType *pNetBotsType=0;
class MQ2NetBotsType : public MQ2Type {

private:
  map<string,BotInfo>::iterator l;
  BotInfo *BotRec;
  char Temps[MAX_STRING];
  char Works[MAX_STRING];
  long Cpt;

public:
  enum Information {
    Enable=1,
    Listen=2,
    Output=3,
    Counts=5,
    Client=6,
    Name=7,
    Zone=8,
    Instance=9,
    ID=10,
    Class=11,
    Level=12,
    PctExp=13,
    PctAAExp=14,
//    PctGroupLeaderExp=15,
    CurrentHPs=16,
    MaxHPs=17,
    PctHPs=18,
    CurrentEndurance=19,
    MaxEndurance=20,
    PctEndurance=21,
    CurrentMana=22,
    MaxMana=23,
    PctMana=24,
    PetID=25,
    PetHP=26,
    TargetID=27,
    TargetHP=28,
    Casting=29,
    State=30,
    Attacking=31,
    AFK=32,
    Binding=33,
    Ducking=34,
    Invis=35,
    Feigning=36,
    Grouped=37,
    Levitating=38,
    LFG=39,
    Mounted=40,
    Moving=41,
    Raid=42,
    Sitting=43,
    Standing=44,
    Stunned=45,
//    Gem=46,
    Buff=47,
    ShortBuff=48,
    PetBuff=49,
    FreeBuffSlots=50,
    InZone=51,
    InGroup=52,
    Leader=53,
    Updated=54,
//    Duration=55,
    TotalAA=56, 
    UsedAA=57, 
    UnusedAA=58, 
    CombatState=59,
    Stacks=60,
    Note=61,
    Detrimentals=62,
    Counters=63,
    Cursed=64,
    Diseased=65,
    Poisoned=66,
    Corrupted=67,
    EnduDrain=68,
    LifeDrain=69,
    ManaDrain=70,
    Blinded=71,
    CastingLevel=72,
    Charmed=73,
    Feared=74,
    Healing=75,
    Invulnerable=76,
    Mesmerized=77,
    Rooted=78,
    Silenced=79,
    Slowed=80,
    Snared=81,
    SpellCost=82,
    SpellSlowed=83,
    SpellDamage=84,
    Trigger=85,
    Resistance=86,
    Detrimental=87,
    NoCure=88,
   };

  MQ2NetBotsType():MQ2Type("NetBots") {
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
//    TypeMember(PctGroupLeaderExp);
    TypeMember(CurrentHPs);
    TypeMember(MaxHPs);
    TypeMember(PctHPs);
    TypeMember(CurrentEndurance);
    TypeMember(MaxEndurance);
    TypeMember(PctEndurance);
    TypeMember(CurrentMana);
    TypeMember(MaxMana);
    TypeMember(PctMana);
    TypeMember(PetID);
    TypeMember(PetHP);
    TypeMember(TargetID);
    TypeMember(TargetHP);
    TypeMember(Casting);
    TypeMember(State);
    TypeMember(Attacking);
    TypeMember(AFK);
    TypeMember(Binding);
    TypeMember(Ducking);
    TypeMember(Invis);
    TypeMember(Feigning);
    TypeMember(Grouped);
    TypeMember(Levitating);
    TypeMember(LFG);
    TypeMember(Mounted);
    TypeMember(Moving);
    TypeMember(Raid);
    TypeMember(Sitting);
    TypeMember(Standing);
    TypeMember(Stunned);
//    TypeMember(Gem);
    TypeMember(Buff);
    TypeMember(ShortBuff);
    TypeMember(PetBuff);
    TypeMember(FreeBuffSlots);
    TypeMember(InZone);
    TypeMember(InGroup);
    TypeMember(Leader);
    TypeMember(Updated);
//    TypeMember(Duration);
    TypeMember(TotalAA); 
    TypeMember(UsedAA); 
    TypeMember(UnusedAA); 
    TypeMember(CombatState);
    TypeMember(Stacks);
    TypeMember(Note);
    TypeMember(Detrimentals);
    TypeMember(Counters);
    TypeMember(Cursed);
    TypeMember(Diseased);
    TypeMember(Poisoned);
    TypeMember(Corrupted);
    TypeMember(EnduDrain);
    TypeMember(LifeDrain);
    TypeMember(ManaDrain);
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
    TypeMember(NoCure);
  }

  void Search(PCHAR Index) {
    if(!Index[0]) BotRec=0;
    else if(!BotRec || strcmp(Index,BotRec->Name)!=0)
      BotRec=BotFind(Index);
  }

  bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
    if(PMQ2TYPEMEMBER pMember=MQ2NetBotsType::FindMember(Member)) {
      switch((Information)pMember->ID) {
        case Enable:
          Dest.Type=pBoolType;
          Dest.DWord=NetStat;
          return true;
        case Listen:
          Dest.Type=pBoolType;
          Dest.DWord=NetGrab;
          return true;
        case Output:
          Dest.Type=pBoolType;
          Dest.DWord=NetSend;
          return true;
        case Counts:
          Cpt=0;
          if(NetStat && NetGrab)
            for(l=NetMap.begin(); l!=NetMap.end(); l++) {
              BotRec=&(*l).second;
              if(BotRec->SpawnID==0) continue;
              Cpt++;
            }
          Dest.Type=pIntType;
          Dest.Int=Cpt;
          return true;
        case Client:
          Cpt=0; Temps[0]=0;
          if(NetStat && NetGrab)
            for(l=NetMap.begin(); l!=NetMap.end(); l++) {
              BotRec=&(*l).second;
              if(BotRec->SpawnID==0) continue;
              if(Cpt++) strcat_s(Temps," ");
              strcat_s(Temps,BotRec->Name);
            }
          if(IsNumber(Index)) { 
            int n=atoi(Index); 
            if (n<0||n>Cpt) break; 
            strcpy_s(Temps,GetArg(Works,Temps,n)); 
          } 

          Dest.Type=pStringType;
          Dest.Ptr=Temps;
          return true;
      }
      if(BotRec) switch((Information)pMember->ID) {
        case Name:
          Dest.Type=pStringType;
          Dest.Ptr=Temps;
          strcpy_s(Temps,BotRec->Name);
          return true;
        case Zone:
          Dest.Type=pIntType;
          Dest.DWord=BotRec->ZoneID;
          return true;
        case Instance:
          Dest.Type=pIntType;
          Dest.DWord=BotRec->InstID;
          return true;
        case ID:
          Dest.Type=pIntType;
          Dest.DWord=BotRec->SpawnID;
          return true;
        case Class:
          Dest.Type=pClassType;
          Dest.DWord=BotRec->ClassID;
          return true;
        case Level:
          Dest.Type=pIntType;
          Dest.DWord=BotRec->Level;
          return true;
        case PctExp:
          Dest.Type=pFloatType;
          Dest.Float=(float)(BotRec->XP/3.30f);
          return true;
        case PctAAExp:
          Dest.Type=pFloatType;
          Dest.Float=(float)(BotRec->aaXP/3.30f);
          return true;
/*
        case PctGroupLeaderExp:
          Dest.Type=pFloatType;
          Dest.Float=(float)(BotRec->glXP/10.0f);
          return true;
*/
        case CurrentHPs:
          Dest.Type=pIntType;
          Dest.Int=BotRec->LifeCur;
          return true;
        case MaxHPs:
          Dest.Type=pIntType;
          Dest.Int=BotRec->LifeMax;
          return true;
        case PctHPs:
          Dest.Type=pIntType;
          Dest.Int=(BotRec->LifeMax<1 || BotRec->LifeCur<1)?0:BotRec->LifeCur*100/BotRec->LifeMax;
          return true;
        case CurrentEndurance:
          Dest.Type=pIntType;
          Dest.Int=BotRec->EnduCur;
          return true;
        case MaxEndurance:
          Dest.Type=pIntType;
          Dest.Int=BotRec->EnduMax;
          return true;
        case PctEndurance:
          Dest.Type=pIntType;
          Dest.Int=(BotRec->EnduMax<1 || BotRec->EnduCur<1)?0:BotRec->EnduCur*100/BotRec->EnduMax;
          return true;
        case CurrentMana:
          Dest.Type=pIntType;
          Dest.Int=BotRec->ManaCur;
          return true;
        case MaxMana:
          Dest.Type=pIntType;
          Dest.Int=BotRec->ManaMax;
          return true;
        case PctMana:
          Dest.Type=pIntType;
          Dest.Int=(BotRec->ManaMax<1 || BotRec->ManaCur<1)?0:BotRec->ManaCur*100/BotRec->ManaMax;
          return true;
        case PetID:
          Dest.Type=pIntType;
          Dest.DWord=BotRec->PetID;
          return true;
        case PetHP:
          Dest.Type=pIntType;
          Dest.Int=BotRec->PetHP;
          return true;
        case TargetID:
          Dest.Type=pIntType;
          Dest.DWord=BotRec->TargetID;
          return true;
        case TargetHP:
          Dest.Type=pIntType;
          Dest.Int=BotRec->TargetHP;
          return true;
        case Casting:
          if(BotRec->CastID) {
            Dest.Type=pSpellType;
            Dest.Ptr=GetSpellByID(BotRec->CastID);
            return true;
          }
          break;
        case State:
          Dest.Type=pStringType;
          Dest.Ptr=Temps;
          if(BotRec->State & STATE_STUN)       strcpy_s(Temps,"STUN");
          else if(BotRec->State & STATE_STAND) strcpy_s(Temps,"STAND");
          else if(BotRec->State & STATE_SIT)   strcpy_s(Temps,"SIT");
          else if(BotRec->State & STATE_DUCK)  strcpy_s(Temps,"DUCK");
          else if(BotRec->State & STATE_BIND)  strcpy_s(Temps,"BIND");
          else if(BotRec->State & STATE_FEIGN) strcpy_s(Temps,"FEIGN");
          else if(BotRec->State & STATE_DEAD)  strcpy_s(Temps,"DEAD");
          else strcpy_s(Temps,"UNKNOWN");
          return true;
        case Attacking:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_ATTACK;
          return true;
        case AFK:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_AFK;
          return true;
        case Binding:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_BIND;
          return true;
        case Ducking:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_DUCK;
          return true;
        case Feigning:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_FEIGN;
          return true;
        case Grouped:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_GROUP;
          return true;
        case Invis:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_INVIS;
          return true;
        case Levitating:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_LEV;
          return true;
        case LFG:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_LFG;
          return true;
        case Mounted:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_MOUNT;
          return true;
        case Moving:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_MOVING;
          return true;
        case Raid:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_RAID;
          return true;
        case Sitting:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_SIT;
          return true;
        case Standing:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_STAND;
          return true;
        case Stunned:
          Dest.Type=pBoolType;
          Dest.DWord=BotRec->State & STATE_STUN;
          return true;
        case FreeBuffSlots:
          Dest.Type=pIntType;
          Dest.Int=BotRec->FreeBuff;
          return true;
        case InZone:
          Dest.Type=pBoolType;
          Dest.DWord=(inZoned(BotRec->ZoneID,BotRec->InstID));
          return true;
        case InGroup:
          Dest.Type=pBoolType;
          Dest.DWord=(inZoned(BotRec->ZoneID,BotRec->InstID) && inGroup(BotRec->SpawnID));
          return true;
        case Leader:
          Dest.Type=pStringType;
          Dest.Ptr=Temps;
          strcpy_s(Temps,BotRec->Leader);
          return true;
        case Note:
          Dest.Type=pStringType;
          Dest.Ptr=Temps;
          strcpy_s(Temps,BotRec->Note);
          return true;

        case Updated:
          Dest.Type=pIntType;
          Dest.Int=clock()-BotRec->Updated;
          return true;
/*
        case Gem:
          if(!Index[0]) {
            Temps[0]=0;
            for (Cpt=0; Cpt<GEMS_MAX; Cpt++) {
              sprintf_s(Works,"%d ",BotRec->Gem[Cpt]);
              strcat_s(Temps,Works);
            }
            Dest.Ptr=Temps;
            Dest.Type=pStringType;
            return true;
          }
          Cpt=atoi(Index);
          if(Cpt<GEMS_MAX && Cpt>-1)
            if(Dest.Ptr=GetSpellByID(BotRec->Gem[Cpt])) {
              Dest.Type=pSpellType;
              return true;
            }
        	break;
*/
	case Buff:
          if(!Index[0]) {
			sprintf_s(Temps," ");
            for (Cpt=0; Cpt<BUFF_MAX && BotRec->Buff[Cpt]; Cpt++) {
              sprintf_s(Works,"%d ",BotRec->Buff[Cpt]);
              strcat_s(Temps,Works);
            }
            Dest.Ptr=Temps;
            Dest.Type=pStringType;
            return true;
          }
          Cpt=atoi(Index);
          if(Cpt<BUFF_MAX && Cpt>-1)
            if(Dest.Ptr=GetSpellByID(BotRec->Buff[Cpt])) {
              Dest.Type=pSpellType;
              return true;
            }
          break;
/*
	case Duration:
          if(!Index[0]) {
            Temps[0]=0;
            for (Cpt=0; Cpt<BUFF_MAX && BotRec->Duration[Cpt]; Cpt++) {
              sprintf_s(Works,"%d ",BotRec->Duration[Cpt]);
              strcat_s(Temps,Works);
            }
            Dest.Ptr=Temps;
            Dest.Type=pStringType;
            return true;
          }
          Cpt=atoi(Index);
          if(Cpt<BUFF_MAX && Cpt>-1)
      //   WriteChatf("Duration: %d", BotRec->Duration[Cpt]);
            if(Dest.Int=BotRec->Duration[Cpt]) {
              Dest.Type=pIntType;
              return true;
             }
          break;
*/
        case ShortBuff:
          if(!Index[0]) {
            Temps[0]=0;
            for (Cpt=0; Cpt<SONG_MAX && BotRec->Song[Cpt]; Cpt++) {
              sprintf_s(Works,"%d ",BotRec->Song[Cpt]);
              strcat_s(Temps,Works);
            }
            Dest.Ptr=Temps;
            Dest.Type=pStringType;
            return true;
          }
          Cpt=atoi(Index);
          if(Cpt<SONG_MAX && Cpt>-1)
            if(Dest.Ptr=GetSpellByID(BotRec->Song[Cpt])) {
              Dest.Type=pSpellType;
              return true;
            }
         	break;

        case PetBuff:
          if(!Index[0]) {
            Temps[0]=0;
            for (Cpt=0; Cpt<PETS_MAX && BotRec->Pets[Cpt]; Cpt++) {
              sprintf_s(Works,"%d ",BotRec->Pets[Cpt]);
              strcat_s(Temps,Works);
            }
            Dest.Ptr=Temps;
            Dest.Type=pStringType;
            return true;
          }
          Cpt=atoi(Index);
          if(Cpt<PETS_MAX && Cpt>-1)
            if(Dest.Ptr=GetSpellByID(BotRec->Pets[Cpt])) {
              Dest.Type=pSpellType;
              return true;
            }
         	break;

         case TotalAA: 
           Dest.Type=pIntType; 
           Dest.DWord=BotRec->TotalAA; 
           return true; 
         case UsedAA: 
           Dest.Type=pIntType; 
           Dest.DWord=BotRec->UsedAA; 
           return true; 
         case UnusedAA: 
           Dest.Type=pIntType; 
           Dest.DWord=BotRec->UnusedAA; 
           return true; 
         case CombatState: 
           Dest.Type=pIntType; 
           Dest.DWord=BotRec->CombatState; 
           return true; 
         case Stacks: 
           if(Index[0]) { 
             Cpt=atoi(Index); 
             PSPELL pSpll = GetSpellByID(Cpt); 
             if (!pSpll) { 
                Dest.Type=pBoolType; 
                Dest.DWord=false; 
                return true; 
             } 
             // If pSpll have duration then check stacking 
             if (pSpll->DurationValue1>0) { 
                // Check if pSpll go to ShortBuff Window or to Buff Window 
                if (pSpll->DurationWindow) { 
                   for (Cpt=0; Cpt<SONG_MAX && BotRec->Song[Cpt]; Cpt++) { 
                      if ((pSpll->ID == BotRec->Song[Cpt]) || !BuffStackTest(pSpll, GetSpellByID(BotRec->Song[Cpt]))) { 
                         Dest.Type=pBoolType; 
                         Dest.DWord=false; 
                         return true; 
                      } 
                   } 
                } else { 
                   for (Cpt=0; Cpt<BUFF_MAX && BotRec->Buff[Cpt]; Cpt++) { 
                      if ((pSpll->ID == BotRec->Buff[Cpt]) || !BuffStackTest(pSpll, GetSpellByID(BotRec->Buff[Cpt]))) { 
                         Dest.Type=pBoolType; 
                         Dest.DWord=false; 
                         return true; 
                      } 
                   } 
                } 
             } 
             // if we here spell will stack or no duration 
             Dest.Type=pBoolType; 
             Dest.DWord=true; 
             return true; 
           } 
           break; 
        case Detrimentals:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[DETRIMENTALS];
          return true;
        case Counters:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[COUNTERS];
          return true;
        case Cursed:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[CURSED];
          return true;
        case Diseased:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[DISEASED];
          return true;
        case Poisoned:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[POISONED];
          return true;
        case Corrupted:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[CORRUPTED];
          return true;
        case EnduDrain:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[ENDUDRAIN];
          return true;
        case LifeDrain:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[LIFEDRAIN];
          return true;
        case ManaDrain:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[MANADRAIN];
          return true;
        case Blinded:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[BLINDED];
          return true;
        case CastingLevel:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[CASTINGLEVEL];
          return true;
        case Charmed:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[CHARMED];
          return true;
        case Feared:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[FEARED];
          return true;
        case Healing:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[HEALING];
          return true;
        case Invulnerable:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[INVULNERABLE];
          return true;
        case Mesmerized:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[MESMERIZED];
          return true;
        case Rooted:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[ROOTED];
          return true;
        case Silenced:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[SILENCED];
          return true;
        case Slowed:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[SLOWED];
          return true;
        case Snared:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[SNARED];
          return true;
        case SpellCost:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[SPELLCOST];
          return true;
        case SpellSlowed:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[SPELLSLOWED];
          return true;
        case SpellDamage:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[SPELLDAMAGE];
          return true;
        case Trigger:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[TRIGGR];
          return true;
        case Resistance:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[RESISTANCE];
          return true;
        case Detrimental:
          Temps[0]=0;  
	  if(BotRec->Detrimental[CURSED])       strcat_s(Temps,"Cursed ");
          if(BotRec->Detrimental[DISEASED])     strcat_s(Temps,"Diseased ");
          if(BotRec->Detrimental[POISONED])     strcat_s(Temps,"Poisoned ");
          if(BotRec->Detrimental[ENDUDRAIN])    strcat_s(Temps,"EnduDrain ");
          if(BotRec->Detrimental[LIFEDRAIN])    strcat_s(Temps,"LifeDrain ");
          if(BotRec->Detrimental[MANADRAIN])    strcat_s(Temps,"ManaDrain ");
          if(BotRec->Detrimental[BLINDED])      strcat_s(Temps,"Blinded ");
          if(BotRec->Detrimental[CASTINGLEVEL]) strcat_s(Temps,"CastingLevel ");
          if(BotRec->Detrimental[CHARMED])      strcat_s(Temps,"Charmed ");
          if(BotRec->Detrimental[FEARED])       strcat_s(Temps,"Feared ");
          if(BotRec->Detrimental[HEALING])      strcat_s(Temps,"Healing ");
          if(BotRec->Detrimental[INVULNERABLE]) strcat_s(Temps,"Invulnerable ");
          if(BotRec->Detrimental[MESMERIZED])   strcat_s(Temps,"Mesmerized ");
          if(BotRec->Detrimental[ROOTED])       strcat_s(Temps,"Rooted ");
          if(BotRec->Detrimental[SILENCED])     strcat_s(Temps,"Silenced ");
          if(BotRec->Detrimental[SLOWED])       strcat_s(Temps,"Slowed ");
          if(BotRec->Detrimental[SNARED])       strcat_s(Temps,"Snared ");
          if(BotRec->Detrimental[SPELLCOST])    strcat_s(Temps,"SpellCost ");
          if(BotRec->Detrimental[SPELLDAMAGE])  strcat_s(Temps,"SpellDamage ");
          if(BotRec->Detrimental[SPELLSLOWED])  strcat_s(Temps,"SpellSlowed ");
          if(BotRec->Detrimental[TRIGGR])       strcat_s(Temps,"Trigger ");
	  if(BotRec->Detrimental[CORRUPTED])    strcat_s(Temps,"Corrupted ");
          if(BotRec->Detrimental[RESISTANCE])   strcat_s(Temps,"Resistance ");
          if(long len=strlen(Temps)) Temps[--len]=0;
          Dest.Type=pStringType;
          Dest.Ptr=Temps;
          return true;
	case NoCure:
          Dest.Type=pIntType;
          Dest.Int=BotRec->Detrimental[NOCURE];
          return true;
      }
    }
    strcpy_s(Temps,"NULL");
    Dest.Type=pStringType;
    Dest.Ptr=Temps;
    return true;
  }

bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
    strcpy_s(Destination, MAX_STRING, "TRUE");
    return true;
  }

bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
    return false;
  }

  bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
    return false;
  }

  ~MQ2NetBotsType() { }
};

BOOL dataNetBots(PCHAR Index, MQ2TYPEVAR &Dest) {
  Dest.DWord=1;
  Dest.Type=pNetBotsType;
  pNetBotsType->Search(Index);
  return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void Command(PSPAWNINFO pChar, PCHAR Cmd) {
  char Tmp[MAX_STRING]; BYTE Parm=1;
  char Var[MAX_STRING];
  char Set[MAX_STRING];
  do {
    GetArg(Tmp,Cmd,Parm++);
    GetArg(Var,Tmp,1,FALSE,FALSE,FALSE,'=');
    GetArg(Set,Tmp,2,FALSE,FALSE,FALSE,'=');
    if(!_strnicmp(Tmp,"on",2) || !_strnicmp(Tmp,"off",3)) {
      NetStat=_strnicmp(Tmp,"off",3);
      WritePrivateProfileString(PLUGIN_NAME,"Stat",NetStat?"1":"0",INIFileName);
    } else if(!_strnicmp(Var,"stat",4) || !_strnicmp(Var,"plugin",6)) {
      NetStat=_strnicmp(Set,"off",3);
      WritePrivateProfileString(PLUGIN_NAME,"Stat",NetStat?"1":"0",INIFileName);

    } else if(!_strnicmp(Var,"grab",4)) {
      NetGrab=_strnicmp(Set,"off",3);
      WritePrivateProfileString(PLUGIN_NAME,"Grab",NetGrab?"1":"0",INIFileName);
    } else if(!_strnicmp(Var,"send",4)) {
      NetSend=_strnicmp(Set,"off",3);
      WritePrivateProfileString(PLUGIN_NAME,"Send",NetSend?"1":"0",INIFileName);
    }
  } while(strlen(Tmp));
  WriteChatf("%s:: (%s) Grab (%s) Send (%s).",PLUGIN_NAME,NetStat?"\agon\ax":"\aroff\ax",NetGrab?"\agon\ax":"\aroff\ax",NetSend?"\agon\ax":"\aroff\ax");
}

void CommandNote(PSPAWNINFO pChar, PCHAR Cmd) {
	strcpy_s(NetNote,Cmd);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

PLUGIN_API VOID OnBeginZone(VOID) {
  #if    DEBUGGING>0
    DebugSpewAlways("%s->OnBeginZone()",PLUGIN_NAME);
  #endif DEBUGGING
  ZeroMemory(sTimers,sizeof(sTimers));
  if(NetStat && NetSend && EQBCConnected()) EQBCBroadCast("[NB]|Z=:>|[NB]");
}

PLUGIN_API VOID OnNetBotEVENT(PCHAR Msg) {
  #if    DEBUGGING>0
    DebugSpewAlways("%s->OnNetBotEVENT(%s)",PLUGIN_NAME,Msg);
  #endif DEBUGGING
  if(!strncmp(Msg,"NBQUIT=",7))      BotQuit(&Msg[7]);
  else if(!strncmp(Msg,"NBJOIN=",7)) ZeroMemory(sTimers,sizeof(sTimers));
  else if(!strncmp(Msg,"NBEXIT",6))  NetMap.clear();
}

PLUGIN_API VOID OnNetBotMSG(PCHAR Name, PCHAR Msg) {
  if(NetStat && NetGrab && !strncmp(Msg,"[NB]|",5) && GetCharInfo() && GetCharInfo()->pSpawn && strcmp(GetCharInfo()->Name,Name)) {
    #if    DEBUGGING>1
      DebugSpewAlways("%s->OnNetBotMSG(From:[%s] Msg[%s])",PLUGIN_NAME,Name,Msg);
    #endif DEBUGGING
	  CHAR szCmd[MAX_STRING] = { 0 };
	  strcpy_s(szCmd, Msg);
    if(CurBot=BotLoad(Name)) {
      Packet.Feed(szCmd);
      CurBot->Updated=clock();
      CurBot=0;
    }
  }
}

PLUGIN_API VOID OnPulse(VOID) {
  if(NetStat && NetSend && gbInZone && (long)clock()>NetLast) {
    NetLast=(long)clock()+NETTICK;
    if(EQBCConnected() && GetCharInfo() && GetCharInfo()->pSpawn && GetCharInfo2()) BroadCast();
  }
}

PLUGIN_API VOID SetGameState(DWORD GameState) {
  #if    DEBUGGING>0
    DebugSpewAlways("%s->SetGameState(%d)",PLUGIN_NAME,GameState);
  #endif DEBUGGING
  if(GameState==GAMESTATE_INGAME) {
    if(!NetInit) {
      #if    DEBUGGING>0
        DebugSpewAlways("%s->SetGameState(%d)->Loading",PLUGIN_NAME,GameState);
      #endif DEBUGGING
      sprintf_s(INIFileName,"%s\\%s_%s.ini",gszINIPath,EQADDR_SERVERNAME,GetCharInfo()->Name);
      NetStat=GetPrivateProfileInt(PLUGIN_NAME,"Stat",0,INIFileName);
      NetGrab=GetPrivateProfileInt(PLUGIN_NAME,"Grab",0,INIFileName);
      NetSend=GetPrivateProfileInt(PLUGIN_NAME,"Send",0,INIFileName);
      NetInit=true;
    }
  } else if(GameState!=GAMESTATE_LOGGINGIN) {
    if(NetInit) {
      #if    DEBUGGING>0
        DebugSpewAlways("%s->SetGameState(%d)->Flushing",PLUGIN_NAME,GameState);
      #endif DEBUGGING
      NetStat=false;
      NetGrab=false;
      NetSend=false;
      NetInit=false;
    }
  }
}

PLUGIN_API VOID InitializePlugin(VOID) {
  #if    DEBUGGING>0
    DebugSpewAlways("%s->ShutdownPlugin()",PLUGIN_NAME);
  #endif DEBUGGING
  Packet.Reset();
  NetMap.clear();
  Packet.AddEvent("#*#[NB]#*#|Z=#1#:#2#>#3#|#*#[NB]",    ParseInfo, (void *) 3);
  Packet.AddEvent("#*#[NB]#*#|L=#4#:#5#|#*#[NB]",        ParseInfo, (void *) 5);
  Packet.AddEvent("#*#[NB]#*#|H=#6#/#7#|#*#[NB]",        ParseInfo, (void *) 7);
  Packet.AddEvent("#*#[NB]#*#|E=#8#/#9#|#*#[NB]",        ParseInfo, (void *) 9);
  Packet.AddEvent("#*#[NB]#*#|M=#10#/#11#|#*#[NB]",      ParseInfo, (void *) 11);
  Packet.AddEvent("#*#[NB]#*#|P=#12#:#13#|#*#[NB]",      ParseInfo, (void *) 13);
  Packet.AddEvent("#*#[NB]#*#|T=#14#:#15#|#*#[NB]",      ParseInfo, (void *) 15);
  Packet.AddEvent("#*#[NB]#*#|C=#16#|#*#[NB]",           ParseInfo, (void *) 16);
  Packet.AddEvent("#*#[NB]#*#|Y=#17#|#*#[NB]",           ParseInfo, (void *) 17);
  Packet.AddEvent("#*#[NB]#*#|X=#18#:#19#|#*#[NB]",      ParseInfo, (void *) 19);
  Packet.AddEvent("#*#[NB]#*#|F=#21#|#*#[NB]",           ParseInfo, (void *) 21);
  Packet.AddEvent("#*#[NB]#*#|N=#22#|#*#[NB]",           ParseInfo, (void *) 22);
  Packet.AddEvent("#*#[NB]#*#|G=#30#|#*#[NB]",           ParseInfo, (void *) 30);
  Packet.AddEvent("#*#[NB]#*#|B=#31#|#*#[NB]",           ParseInfo, (void *) 31);
  Packet.AddEvent("#*#[NB]#*#|S=#32#|#*#[NB]",           ParseInfo, (void *) 32);
  Packet.AddEvent("#*#[NB]#*#|W=#33#|#*#[NB]",           ParseInfo, (void *) 33);
  Packet.AddEvent("#*#[NB]#*#|D=#34#|#*#[NB]",           ParseInfo, (void *) 34);
  Packet.AddEvent("#*#[NB]#*#|A=#35#:#36#:#37#|#*#[NB]", ParseInfo, (void *) 37); 
  Packet.AddEvent("#*#[NB]#*#|O=#38#|#*#[NB]",           ParseInfo, (void *) 38); 
  Packet.AddEvent("#*#[NB]#*#|U=#39#|#*#[NB]",           ParseInfo, (void *) 39); 
  Packet.AddEvent("#*#[NB]#*#|R=#40#|#*#[NB]",           ParseInfo, (void *) 40);
  ZeroMemory(sTimers,sizeof(sTimers));
  ZeroMemory(sBuffer,sizeof(sBuffer));
  ZeroMemory(wBuffer,sizeof(wBuffer));
  ZeroMemory(wChange,sizeof(wChange));
  ZeroMemory(wUpdate,sizeof(wUpdate));
  pNetBotsType  = new MQ2NetBotsType;
  NetNote[0] = 0;
  AddMQ2Data("NetBots" ,dataNetBots);
  AddCommand("/netbots",Command);
  AddCommand("/netnote",CommandNote);
}

PLUGIN_API VOID ShutdownPlugin(VOID) {
  #if    DEBUGGING>0
    DebugSpewAlways("%s->ShutdownPlugin()",PLUGIN_NAME);
  #endif DEBUGGING
  RemoveCommand("/netbots");
  RemoveCommand("/netnote");
  RemoveMQ2Data("NetBots");
  delete pNetBotsType;
}
