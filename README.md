# Description
MQ2NetBots provides linked MQ2EQBC clients a method of sharing status information and statistics. It also adds a new Top-Level Object ${NetBots}.

# Commands
/netbots on/off - Toggles NetBots on or off.<BR>
/netbots grab=on/off - Toggles receiving status updates from other NetBots clients connected to EQBCS.<BR>
/netbots send=on/off - Toggles NetBots clients sending of status updates to EQBCS on or off.<BR>
/netbots extended=0/1/2/3 - Sets whether client will send extended status updates to EQBCS. 0=None,1=Gems + Buff Durations,2=Add Short Durations,3=Add Pet Buff Durations.<BR>
/netbots simple=on/off - Toggles simple spell name searching (which allows any substring to match) on or off.<BR>
/netbots show - Sets showing the NetBots window to on.<BR>
/netbots hide - Sets showing the NetBots window to off.<BR>
/netbots switch - Will switch between old and new NetBots window format.<BR>
/netbots concolors=on/off - Toggles use of ConColors for Target names in the NetBots window on or off.<BR>
/netbots cleannames=on/off - Toggles use of CleanNames for Target names in the NetBots window on or off.<BR>
/netbots lclick=0/1/2/3 - Set the mouse left-click action on a data row in the NetBots window. 0=None,1=Bring to Foreground,2=Target Player,3=Target Player Target.<BR>
/netbots rclick=0/1/2/3 - Set the mouse right-click action on a data row in the NetBots window. 0=None,1=Bring to Foreground,2=Target Player,3=Target Player Target.<BR>
/netbots load - Will re-load all settings using the .ini file.<BR>
/netnote [note] - Sets the NetBots Note field of the client.<BR>


# INI Examples
## server_Character.INI

[MQ2NetBots]<BR>
Stat=1<BR>
Grab=1<BR>
Send=1<BR>
Extended=0<BR>
UseSimpleSearch=0<BR>
Show=1<BR>
LClick=2<BR>
RClick=3<BR>
ConColors=1<BR>
CleanNames=1<BR>


## MQ2NetBots.INI

[Settings]<BR>
Query=<BR>
WindowTitle=MQ2NetBots (${NetBots.Counts})<BR>
[server.Character]<BR>
ChatTop=742<BR>
ChatBottom=820<BR>
ChatLeft=468<BR>
ChatRight=974<BR>
Locked=0<BR>
Fades=1<BR>
Delay=2000<BR>
Duration=500<BR>
Alpha=255<BR>
FadeToAlpha=154<BR>
BGType=2<BR>
BGTint.alpha=255<BR>
BGTint.red=0<BR>
BGTint.green=0<BR>
BGTint.blue=0<BR>



# INI Settings Definition
## server_Character.INI

All settings are in the [MQ2NetBots] section.

Stat - This enables (set to 1) or disables (set to 0) MQ2NetBots send and receive data and processing. Setting this to 0 effectively disables the plugin. Use the /netbots on or /netbots off commands toggle this in game.<BR><BR>
Grab - Whether to listen for NetBots data. If this particular character needs to send information, but doesn't care about information from other clients, you can set this to 0 so it doesn't process incoming data. Set to 1 to process the data. Use the /netbots grab=on or /netbots grab=off commands toggle this in game.<BR><BR>
Send - Whether to send NetBots data to other clients via eqbc. If this character needs to receive other netbots client info, but doesn't need to send it, you can set this to 0 to disable it. Set to 1 to enable. Use the /netbots send=on or /netbots send=off commands toggle this in game.<BR><BR>
Extended - Whether to send additional detailed data to other netbots clients via EQBC. This includes Gems and Buff/Song Durations. 0=None, 1=Gems+Buff Durations, 2=1+Add Short Durations, 3=2+Add Pet Buff Durations. Use the /netbots ext command to chamge this in game.<BR><BR>
SendEQBCData - Whether to send additional information related to EQBC Packets/Heartbeat. 0=off, 1=on. Recommend off to cut down on unecessary EQBC traffic.<BR><BR>
UseSimpleSearch - Whether to allow simple searching when providing Spell Names. Simple will match any substring of the spell to your search term. Use the /netbots simple=on or /netbots simple=off commands toggle this in game.<BR><BR>
Show - Whether to show the NetBots client information window in game. Set to 0 to disable, 1 to enable. Use the /netbots show or /netbots hide commands toggle this in game.<BR><BR>
UseNewWindow - Whether to use the new client information window format or the old. 0=Old, 1=New. Use the /netbots switch command to toggle this in game.<BR><BR>
ConColors - Whether to use EQ ConColors when displaying the Target names in the NetBots window. Set to 0 to disable, 1 to enable. Use the /netbots concolors=on or /netbots concolors=off commands toggle this in game.<BR><BR>
CleanNames - Whether to use Cleaned Names EQ displaying the Target names in the NetBots window. Set to 0 to disable, 1 to enable. Use the /netbots cleannames=on or /netbots cleannames=off commands toggle this in game.<BR><BR>
LClick - Set the mouse left-click action on a data row in the NetBots window. 0=No Action, 1=Bring the selected client to the foreground, 2=Target the selected client, 3=Target the selected client's target. Use the /netbots lclick command to change this in game.<BR><BR>
RClick - Set the mouse right-click action on a data row in the NetBots window. 0=No Action, 1=Bring the selected client to the foreground, 2=Target the selected client, 3=Target the selected client's target. Use the /netbots rclick command to change this in game.<BR><BR>
TargetRightMost - Can be used to truncate the target in the NetBots window to show only the rightmost # of characters. The old window format had this hardcoded at 12 (if you want to replicate its functionality).<BR><BR>

## MQ2NetBots.INI

[Settings] section

WindowTitle - The Title used for the im-game NetBots UI window (Show=1).<BR><BR>
Query - An additional Macro-type query that will be executed on each client with results accessed by ${NetBots[Name].Query}. Keep this reasonable, the more data sent to clients the easier it is to overwhelm EQBC.<BR>

	Example INI entry:
	Query=${Bot.DoPulls},${Bot.LootRadius},${Pet.Name}
	Example query:  ${NetBots[Charname].Query.Arg[1,,]}
	Example Query data if Bot.DoPulls is true and Bot.LootRadius is 20 on Charname:  TRUE,20
	Output from above example:  TRUE

[server.Character] section

This section contains NetBots UI window related settings (Show=1).


# Macro Data

Examples

/echo ${NetBots[MY\_TOON].CurrentHPs}<BR>
/echo ${NetBots[MY\_TOON].Buff[4]}

# Top-Level Object

${NetBots} Returns information about your client.<BR>
${NetBots[Name]} Returns information about the connected toon Name.

# Data Types - NetBots

${NetBots.Enable} :Bool Returns TRUE/FALSE based on plugin status.<BR>
${NetBots.Listen} :Bool Returns TRUE/FALSE based on grab parameter status.<BR>
${NetBots.Output} :Bool Returns TRUE/FALSE based on send parameter status.<BR>
${NetBots.Counts} :Int Returns count of currently broadcasting NetBots clients connected to EQBCS.<BR>
${NetBots.Client} :String Returns list of currently broadcasting NetBots clients connected to EQBCS.<BR>


# Data Types - NetBots Clients

${NetBots[Name].Name} :String Name of Name.<BR>
${NetBots[Name].Zone} :Int Zone ID of Name.<BR>
${NetBots[Name].Instance} :Int Instance ID of Name.<BR>
${NetBots[Name].ID} :Int Spawn ID of Name.<BR>
${NetBots[Name].Class} :Class Class of Name.<BR>
${NetBots[Name].Level} :Int Level of Name.<BR>
${NetBots[Name].PctExp} :Float Percent Experience of Name.<BR>
${NetBots[Name].PctAAExp} :Float Percent AA Experience of Name.<BR>
${NetBots[Name].PctGroupLeaderExp} :Float Percent Group Leader Experience of Name. EMU servers only.<BR>
${NetBots[Name].CurrentHPs} :Int Current Hitpoints of Name.<BR>
${NetBots[Name].MaxHPs} :Int Total Hitpoints of Name<BR>
${NetBots[Name].PctHPs} :Int Current Hitpoints percentage of Name.<BR>
${NetBots[Name].CurrentEndurance} :Int Current Endurace of Name.<BR>
${NetBots[Name].MaxEndurance} :Int Total Endurance of Name.<BR>
${NetBots[Name].PctEndurace} :Int Current Endurance percentage of Name.<BR>
${NetBots[Name].CurrentMana} :Int Current Mana of Name.<BR>
${NetBots[Name].MaxMana} :Int Total Mana of Name.<BR>
${NetBots[Name].PctMana} :Int Current Mana percentage of Name.<BR>
${NetBots[Name].PetID} :Int Spawn ID of Name's pet.<BR>
${NetBots[Name].PetHP} :Int Hitpoints Percentage of Name's pet. (Same as PetPctHPs)<BR>
${NetBots[Name].PetPctHPs} :Int Hitpoints Percentage of Name's pet. (Same as PetHP)<BR>
${NetBots[Name].TargetID} :Int Spawn ID of Name's target.<BR>
${NetBots[Name].TargetHP} :Int Hitpoints Percentage of Name's target. (Same as TargetPctHPs)<BR>
${NetBots[Name].TargetPctHPs} :Int Hitpoints Percentage of Name's target. (Same as TargetHP)<BR>
${NetBots[Name].Casting} :Spell Spell (if any) Name is currently casting.<BR>
${NetBots[Name].State} :String State of Name (STUN HOVER MOUNT STAND SIT DUCK BIND FEIGN DEAD UNKNOWN).<BR>
${NetBots[Name].Attacking} :Bool Is Name Attacking?<BR>
${NetBots[Name].AFK} :Bool Is Name AFK?<BR>
${NetBots[Name].Binding} :Bool Is Name kneeling?<BR>
${NetBots[Name].Ducking} :Bool Is Name ducking?<BR>
${NetBots[Name].Feigning} :Bool Is Name feigning?<BR>
${NetBots[Name].Grouped} :Bool Is Name in a group?<BR>
${NetBots[Name].InGroup} :Bool Is Name in your group?<BR>
${NetBots[Name].Invis} :Bool Is Name invis (normal)?<BR>
${NetBots[Name].InvisToUndead} :Bool Is Name invis to undead?<BR>
${NetBots[Name].Levitating} :Bool Is Name levitating?<BR>
${NetBots[Name].LFG} :Bool Is Name LFG?<BR>
${NetBots[Name].Mounted} :Bool Is Name on a mount?<BR>
${NetBots[Name].Moving} :Bool Is Name moving?<BR>
${NetBots[Name].Raid} :Bool Is Name in a raid?<BR>
${NetBots[Name].Sitting} :Bool Is Name sitting?<BR>
${NetBots[Name].Standing} :Bool Is Name standing?<BR>
${NetBots[Name].Stunned} :Bool Is Name stunned?<BR>
${NetBots[Name].Dead} :Bool Is Name dead?<BR>
${NetBots[Name].Hover} :Bool Is Name in hover mode?<BR>
${NetBots[Name].AutoFire} :Bool Is Name ranged auto-firing?<BR>
${NetBots[Name].HaveAggro} :Bool Does Name have aggro?<BR>
${NetBots[Name].WantAggro} :Bool Does Name want aggro? (MQ2Melee AggroMode=1)<BR>
${NetBots[Name].PetAffinity} :Bool Does Name have Pet Affinity (or equivalent) AA for pet receiving group buffs?<BR>
${NetBots[Name].Extended} :Bool Is Name sending extended information in MQ2NetBots (Extended=1)?<BR>
${NetBots[Name].FreeBuffSlots} :Int Total free buff slots Name has.<BR>
${NetBots[Name].InZone} :Bool Is Name in the same zone as you?<BR>
${NetBots[Name].Leader} :String The name of Name's group leader. (Same as GroupLeader)<BR>
${NetBots[Name].GroupLeader} :String The name of Name's group leader. (Same as Leader)<BR>
${NetBots[Name].Note} :String The Note string set in Name's MQ2NetBots Note field.<BR>
${NetBots[Name].Location} :String The Y,X,Z location of Name.<BR>
${NetBots[Name].X} :Float The X location of Name.<BR>
${NetBots[Name].Y} :Float The Y location of Name.<BR>
${NetBots[Name].Z} :Float The Z location of Name.<BR>
${NetBots[Name].Distance} :Float The distance you are away from Name.<BR>
${NetBots[Name].FreeInventory} :Int The number of free inventory slots for Name.<BR>
${NetBots[Name].FreeInventory[#]} :Int The number of Name's free inventory slots of at least size [#] for Name.<BR>
${NetBots[Name].Heading} :String The Heading of Name.<BR>
${NetBots[Name].Updated} :Int Time since last NetBots update from Name.<BR>
${NetBots[Name].Version} :Float The version number the MQ2NetBots that Name is running.<BR>
${NetBots[Name].Gem} :String String of spell IDs Name has memorized. Requires Extended=1 in Name's ini.<BR>
${NetBots[Name].Gem[#]}} :Spell Spell Name has memorized in Gem[#]. Requires Extended=1 in Name's ini.<BR>
${NetBots[Name].Gem[spellname]}} :Spell pSpellType of the spell Name has if he currently has [spellname] memorized. Can be full/partial. Requires Extended=1 in Name's ini.<BR>
${NetBots[Name].Buff} :String String of spell IDs of all buffs Name currently has.<BR>
${NetBots[Name].Buff[#]} :Spell Spell Name currently has in buff slot [#].<BR>
${NetBots[Name].Buff[spellname]} :Spell pSpellType of the buff Name has if he has [spellname] buff. Can be full/partial.<BR>
${NetBots[Name].Duration} :String String of the remaining durations of all buffs Name has. Requires Extended=1 in Name's ini. (Same as BuffDuration)<BR>
${NetBots[Name].Duration[#]} :Ticks Duration of the buff on Name in slot [#]. Requires Extended=1 in Name's ini. (Same as BuffDuration)<BR>
${NetBots[Name].Duration[spellname]} :Ticks Duration of the buff on Name if he has [spellname] buff. Requires Extended=1 in Name's ini. (Same as BuffDuration)<BR>
${NetBots[Name].BuffDuration} :String String of the remaining durations of all buffs Name has. Requires Extended=1 in Name's ini. (Same as Duration)<BR>
${NetBots[Name].BuffDuration[#]} :Ticks Duration of the buff on Name in slot [#]. Requires Extended=1 in Name's ini. (Same as Duration)<BR>
${NetBots[Name].BuffDuration[spellname]} :Ticks Duration of the buff on Name if he has [spellname] buff. Requires Extended=1 (or greater) in Name's ini. (Same as Duration)<BR>
${NetBots[Name].ShortBuff} :String String of spell IDs of all short duration buffs Name currently has.<BR>
${NetBots[Name].ShortBuff[#]} :Spell Spell Name currently has in short duration buff slot [#].<BR>
${NetBots[Name].ShortBuff[spellname]} :Spell pSpellType of the short duration buff Name has if he has [spellname] short duration buff. Can be full/partial.<BR>
${NetBots[Name].ShortDuration} :String String of the remaining durations of all short duration buffs Name has. Requires Extended=2 (or greater) in Name's ini.<BR>
${NetBots[Name].ShortDuration[#]} :Ticks Duration of the buff on Name in short duration slot [#]. Requires Extended=2 (or greater) in Name's ini.<BR>
${NetBots[Name].ShortDuration[spellname]} :Ticks Duration of the buff on Name if he has [spellname] short duration buff. Requires Extended=1 in Name's ini.<BR>
${NetBots[Name].PetBuff} :String String of spell IDs of all buffs on Name's pet.<BR>
${NetBots[Name].PetBuff[#]} :Spell Name of the buff in slot [#] of Name's pet.<BR>
${NetBots[Name].PetBuff[spellname]} :Spell pSpellType of the buff Name's pet if it has [spellname] buff. Can be full/partial.<BR>
${NetBots[Name].PetDuration} :String String of the remaining durations of all buffs on Name's pet. Requires Extended=3 in Name's ini.<BR>
${NetBots[Name].PetDuration[#]} :Ticks Duration of the buff on Name's pet in slot [#]. Requires Extended=3 in Name's ini.<BR>
${NetBots[Name].PetDuration[spellname]} :Ticks Duration of the buff on Name's pet if it has [spellname] buff. Requires Extended=1 in Name's ini.<BR>
${NetBots[Name].TotalAA} :Int Total AAs Name has.<BR>
${NetBots[Name].AAPointsTotal} :Int Total AAs Name has. (Same as TotalAA)<BR>
${NetBots[Name].UsedAA} :Int Total spent AAs of Name.<BR>
${NetBots[Name].AAPointsSpent} :Int Total spent AAs of Name. (Same as UsedAA)<BR>
${NetBots[Name].UnusedAA} :Int Total unspent AAs of Name.<BR>
${NetBots[Name].AAPoints} :Int Total unspent AAs of Name. (Same as UnusedAA)<BR>
${NetBots[Name].AAPointsAssigned} :Int Total assigned AAs of Name.<BR>
${NetBots[Name].CombatState} :Int Combat State of Name. 0=COMBAT,1=DEBUFFED,2=COOLDOWN,3=ACTIVE,4=RESTING.<BR>
${NetBots[Name].Stacks[#]} :Bool Returns true if the spell ID number [#] will stack on Name. Implies no overwrites (will not stack if already present).<BR>
${NetBots[Name].Stacks[spellname]} :Bool Returns true if the spell [spellname] will stack on Name. Implies no overwrites (will not stack if already present).<BR>
${NetBots[Name].StacksPet[#]} :Bool Returns true if the spell ID number [#] will stack on Name's pet. Implies no overwrites (will not stack if already present).<BR>
${NetBots[Name].StacksPet[spellname]} :Bool Returns true if the spell [spellname] will stack on Name's pet. Implies no overwrites (will not stack if already present).<BR>
${NetBots[Name].WillLand[#]} :Bool Returns true if the spell ID number [#] will land on Name. Implies overwrites are OK (will land if already present).<BR>
${NetBots[Name].WillLand[spellname]} :Bool Returns true if the spell [spellname] will land on Name. Implies overwrites are OK (will land if already present).<BR>
${NetBots[Name].WillLandPet[#]} :Bool Returns true if the spell ID number [#] will land on Name's pet. Implies overwrites are OK (will land if already present).<BR>
${NetBots[Name].WillLandPet[spellname]} :Bool Returns true if the spell [spellname] will land on Name's pet. Implies overwrites are OK (will land if already present).<BR>
${NetBots[Name].TooPowerful[#]} :Bool Returns true if the spell ID number [#] is too powerful for Name. Can be used in conjunction with Stacks/Land. Optional since some EMU servers do not enforce this rule.<BR>
${NetBots[Name].TooPowerful[spellname]} :Bool Returns true if the spell [spellname] is too powerful for Name. Can be used in conjunction with Stacks/Land. Optional since some EMU servers do not enforce this rule.<BR>
${NetBots[Name].TooPowerfulPet[#]} :Bool Returns true if the spell ID number [#] is too powerful for Name's per. The results are not server-specific (some EMU servers do not enforce this rule).<BR>
${NetBots[Name].TooPowerfulPet[spellname]} :Bool Returns true if the spell [spellname] is too powerful for Name's pet. The results are not server-specific (some EMU servers do not enforce this rule).<BR>
${NetBots[Name].Query} :String The result of the Query in ini file as executed on Name.<BR>
${NetBots[Name].Detrimentals} :Int64 Total number of detrimental spells on Name.<BR>
${NetBots[Name].Counters} :Int64 Total number of detrimental spell counters (Curse/Disease/Poison/Corruption) on Name. (Same as TotalCounters)<BR>
${NetBots[Name].TotalCounters} :Int64 Total number of detrimental spell counters (Curse/Disease/Poison/Corruption) on Name. (Same as Counters)<BR>
${NetBots[Name].CountersCurse} :Int64 Total number of Curse counters on Name.<BR>
${NetBots[Name].CountersDisease} :Int64 Total number of Disease counters on Name.<BR>
${NetBots[Name].CountersPoison} :Int64 Total number of Poison counters on Name.<BR>
${NetBots[Name].CountersCorruption} :Int64 Total number of Corruption counters on Name.<BR>
${NetBots[Name].NoCure} :Int64 Total number of detrimentals marked as NoCure on Name.<BR>
${NetBots[Name].EnduDrain} :Int64 Amount of Endurance drain per tick for Name.<BR>
${NetBots[Name].LifeDrain} :Int64 Amount of HP drain per tick for Name.<BR>
${NetBots[Name].ManaDrain} :Int64 Amount of Mana drain per tick for Name.<BR>
${NetBots[Name].Cursed} :Int Is Name Cursed? (0/1)<BR>
${NetBots[Name].Diseased} :Int Is Name Diseased? (0/1)<BR>
${NetBots[Name].Poisoned} :Int Is Name Poisoned? (0/1)<BR>
${NetBots[Name].Corrupted} :Int Is Name Corrupted? (0/1)<BR>
${NetBots[Name].Blinded} :Int Is Name Blinded? (0/1)<BR>
${NetBots[Name].CastingLevel} :Int Is the effective Casting Level of Name reduced? (0/1)<BR>
${NetBots[Name].Charmed} :Int Is Name Charmed? (0/1)<BR>
${NetBots[Name].Feared} :Int Is Name Feared? (0/1)<BR>
${NetBots[Name].Healing} :Int Is Name Healing taken reduced? (0/1)<BR>
${NetBots[Name].Invulnerable} :Int Is Name Invulnerable? (0/1)<BR>
${NetBots[Name].Mesmerized} :Int Is Name Mesmerized? (0/1)<BR>
${NetBots[Name].Rooted} :Int Is Name Rooted? (0/1)<BR>
${NetBots[Name].Silenced} :Int Is Name Silenced? (0/1)<BR>
${NetBots[Name].Slowed} :Int Is Name Slowed? (0/1)<BR>
${NetBots[Name].Snared} :Int Is Name Snared? (0/1)<BR>
${NetBots[Name].SpellCost} :Int Is Spell Mana Cost of Name increased? (0/1)<BR>
${NetBots[Name].SpellSlowed} :Int Is the Spell Haste of Name reduced? (0/1)<BR>
${NetBots[Name].SpellDamage} :Int Is the Spell Damage of Name reduced? (0/1)<BR>
${NetBots[Name].Trigger} :Int Does Name have a cast on duration fade Trigger spell? (0/1)<BR>
${NetBots[Name].Resistance} :Int Does Name have a Resistance reduction (Fire/Cold/Poison/Disease/Magic/Corruption)? (0/1)<BR>
${NetBots[Name].RevDSed} :Int Does Name have a Reverse Damage shield on them? (0/1)<BR>
${NetBots[Name].Crippled} :Int Is Name Crippled? (0/1)<BR>
${NetBots[Name].Maloed} :Int Is Name Maloed? (0/1)<BR>
${NetBots[Name].Tashed} :Int Is Name Tashed? (0/1)<BR>
${NetBots[Name].Detrimental} :String A string list of all detrimental types affecting Name.<BR>
${NetBots[Name].Hasted} :Int Is Name Hasted? (0/1)<BR>
${NetBots[Name].DSed} :Int Does Name have a Damage Shield buff? (0/1)<BR>
${NetBots[Name].Aego} :Int Does Name have an Cleric Aego line buff? (0/1)<BR>
${NetBots[Name].Skin} :Int Does Name have a Druid Skin line buff? (0/1)<BR>
${NetBots[Name].Focus} :Int Does Name have a Shaman Focus line buff? (0/1)<BR>
${NetBots[Name].Regen} :Int Does Name have a non-Beastlord HP Regeneration buff? (0/1)<BR>
${NetBots[Name].Symbol} :Int Does Name have a Cleric Symbol line buff? (0/1)<BR>
${NetBots[Name].Clarity} :Int Does Name have an Enchanter Clarity line buff? (0/1)<BR>
${NetBots[Name].Pred} :Int Does Name have a Ranger Predator line buff? (0/1)<BR>
${NetBots[Name].Strength} :Int Does Name have a Ranger Strength line buff? (0/1)<BR>
${NetBots[Name].Brells} :Int Does Name have a Paladin Brells line buff? (0/1)<BR>
${NetBots[Name].SV} :Int Does Name have a Beastlord SV line buff? (0/1)<BR>
${NetBots[Name].SE} :Int Does Name have a Beastlord SE line buff? (0/1)<BR>
${NetBots[Name].HybridHP} :Int Does Name have a Ranger HP line buff? (0/1)<BR>
${NetBots[Name].Growth} :Int Does Name have a Druid Growth type HP buff? (0/1)<BR>
${NetBots[Name].Shining} :Int Does Name have a Cleric Shining line buff? (0/1)<BR>
${NetBots[Name].Spell[spellline]} :Spell Returns the pSpellType of the specified spellline (from the above detrimentals/beneficials) on Name.<BR>
${NetBots[Name].MacroState} :Int The macro state for Name. 0=No macro running, 1=Running, 2=Paused<BR>
${NetBots[Name].MacroName} :String The running macro of Name, empty string if none running.<BR>
${NetBots[Name].NavigationActive} :Bool Does Name have a running MQ2Nav path? (MQ2Nav)<BR>
${NetBots[Name].NavigationPaused} :Bool Does Name have a paused MQ2Nav path? (MQ2Nav) <BR>
${NetBots[Name].BotActive} :Bool Does Name have MQ2Bot active? (MQ2Bot)<BR>
${NetBots[Name].MakeCampStatus} :Int The MakeCamp status (0=None/OFF,1=ON,2=PAUSED) of Name. (MQ2MoveUtils)<BR>
${NetBots[Name].MakeCampX} :Float The X-Location of MakeCamp of Name. (MQ2MoveUtils)<BR>
${NetBots[Name].MakeCampY} :Float The Y-location of MakeCamp of Name. (MQ2MoveUtils)<BR>
${NetBots[Name].MakeCampRadius} :Float The Radius setting of MakeCamp of Name. (MQ2MoveUtils)<BR>
${NetBots[Name].MakeCampDistance} :Float The Distance setting of MakeCamp of Name. (MQ2MoveUtils)<BR>
${NetBots[Name].Lua} :String The LUA script names that Name is running. (Lua)<BR>
${NetBots[Name].Packets} :Int64 The EQBC Packets of Name. Requires SendEQBCData=1 in ini. Not recommended. (MQ2EQBC)<BR>
${NetBots[Name].HeartBeat} :Int64 The EQBC HeartBeat of Name. Requires SendEQBCData=1 in ini. Not recommended. (MQ2EQBC)<BR>
