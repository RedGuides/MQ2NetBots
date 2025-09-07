---
tags:
  - datatype
---
# `NetBots`

<!--dt-desc-start-->
Data Types for MQ2NetBots, the bread and butter of this plugin. Returns information from character name to hit points to levitation status. Most types will require that you use a character name on the index of the TLO. e.g. `${NetBots[Name].AFK}`
<!--dt-desc-end-->

## Members
<!--dt-members-start-->
### {{ renderMember(type='string', name='Client') }}

:   Returns a list of client names currently broadcasting

### {{ renderMember(type='int', name='Counts') }}

:   Number of broadcasting NetBots clients connected to EQBCS

### {{ renderMember(type='bool', name='Enable') }}

:   Returns plugin on/off status

### {{ renderMember(type='bool', name='Listen') }}

:   Returns "grab" status as true/false

### {{ renderMember(type='bool', name='Output') }}

:   Returns "send" status as true/false

### {{ renderMember(type='string', name='Name') }}

:   Returns the name of [Name]

### {{ renderMember(type='int', name='Zone') }}

:   Returns zone id of [Name]

### {{ renderMember(type='int', name='Instance') }}

:   returns instance id of [Name]

### {{ renderMember(type='int', name='ID') }}

:   SpawnID of [Name]

### {{ renderMember(type='class', name='Class') }}

:   Returns class of [Name]

### {{ renderMember(type='int', name='Level') }}

:   Returns level of [Name]

### {{ renderMember(type='float', name='PctExp') }}

:   Percent EXP of [Name]

### {{ renderMember(type='float', name='PctAAExp') }}

:   Percent AA exp of [Name]

### {{ renderMember(type='float', name='PctGroupLeaderExp') }}

:   Returns group leader EXP on servers that have this feature (emu only)

### {{ renderMember(type='int', name='CurrentHPs') }}

:   Current HP of [Name]

### {{ renderMember(type='int', name='MaxHPs') }}

:   Max HP of [Name]

### {{ renderMember(type='int', name='PctHPs') }}

:   percent HP of [Name]

### {{ renderMember(type='int', name='CurrentEndurance') }}

:   Endurance of [Name]

### {{ renderMember(type='int', name='MaxEndurance') }}

:   Total endurance of [Name]

### {{ renderMember(type='int', name='PctEndurance') }}

:   Percent endurance of [Name]

### {{ renderMember(type='int', name='CurrentMana') }}

:   Current mana of [Name]

### {{ renderMember(type='int', name='MaxMana') }}

:   Max mana of [Name]

### {{ renderMember(type='int', name='PctMana') }}

:   Percent mana of [Name]

### {{ renderMember(type='int', name='PetID') }}

:   Id of [Name]'s pet

### {{ renderMember(type='int', name='PetHP') }}

:   HP of [Name]'s pet

### {{ renderMember(type='int', name='TargetID') }}

:   id of [Name]'s target

### {{ renderMember(type='int', name='TargetHP') }}

:   HP of [Name]'s target

### {{ renderMember(type='spell', name='Casting') }}

:   the spell [Name] is casting

### {{ renderMember(type='string', name='State') }}

:   State of [Name]  
STUN STAND SIT DUCK BIND FEIGN DEAD UNKNOWN

### {{ renderMember(type='bool', name='Attacking') }}

:   is [Name] attacking

### {{ renderMember(type='bool', name='AFK') }}

:   is [Name] afk

### {{ renderMember(type='bool', name='Binding') }}

:   whether [Name] is kneeling

### {{ renderMember(type='bool', name='Ducking') }}

:   is [Name] ducking

### {{ renderMember(type='bool', name='Feigning') }}

:   is [Name] feigning

### {{ renderMember(type='bool', name='Grouped') }}

:   is [Name] grouped

### {{ renderMember(type='bool', name='Invis') }}

:   is [Name] invis

### {{ renderMember(type='bool', name='Levitating') }}

:   is [Name] floating

### {{ renderMember(type='bool', name='LFG') }}

:   is [Name] LFG

### {{ renderMember(type='bool', name='Mounted') }}

:   is [Name] on a mount

### {{ renderMember(type='bool', name='Moving') }}

:   is [Name] moving

### {{ renderMember(type='bool', name='Raid') }}

:   is [Name] in a raid

### {{ renderMember(type='bool', name='Sitting') }}

:   is [Name] sitting

### {{ renderMember(type='bool', name='Standing') }}

:   is [Name] standing

### {{ renderMember(type='bool', name='Stunned') }}

:   is [Name] stunned

### {{ renderMember(type='int', name='FreebuffSlots') }}

:   number of free buff slots on [Name]

### {{ renderMember(type='bool', name='InZone') }}

:   is [Name] in my current zone

### {{ renderMember(type='bool', name='InGroup') }}

:   is [Name] in my group

### {{ renderMember(type='string', name='Leader') }}

:   is [Name] the leader

### {{ renderMember(type='string', name='Note') }}

:   Returns the custom note on [Name] (see [/netnote](cmd-netnote.md) on how to set this)

### {{ renderMember(type='string', name='Location') }}

:   The coordinates of [Name]

### {{ renderMember(type='string', name='Heading') }}

:   The heading of [Name]

### {{ renderMember(type='int', name='Updated') }}

:   last netbots update from [Name]

### {{ renderMember(type='string', name='Buff') }}

:   A list of all buffs on [Name]

### {{ renderMember(type='spell', name='Buff', params='#') }}

:   The name of the buff [Name] has in slot &lt;#&gt;

### {{ renderMember(type='string', name='ShortBuff') }}

:   A list of short duration buffs on [Name]

### {{ renderMember(type='spell', name='ShortBuff', params='#') }}

:   Name of the short duration buff [Name] has in slot &lt;#&gt;

### {{ renderMember(type='string', name='PetBuff') }}

:   A list of buffs on [Name]'s pet

### {{ renderMember(type='spell', name='PetBuff', params='#') }}

:   The buff [Name]'s pet has in slot &lt;#&gt;

### {{ renderMember(type='int', name='TotalAA') }}

:   The total AA of [Name]

### {{ renderMember(type='int', name='UsedAA') }}

:   The number of AA's [Name] spent

### {{ renderMember(type='int', name='UnusedAA') }}

:   Number of AA unspent on [Name]

### {{ renderMember(type='int', name='CombatState') }}

:   [Name]'s combat state

### {{ renderMember(type='bool', name='Stacks', params='SpellID') }}

:   True if spell will stack on [Name]. Usage `NetBots[Name].Stacks[SpellID]`

### {{ renderMember(type='bool', name='StacksPet', params='SpellID') }}

:   True if spell will stack on [Name]'s pet.

### {{ renderMember(type='int', name='Detrimentals') }}

:   number of detrimanls on [Name]

### {{ renderMember(type='int', name='Counters') }}

:   Negative effect counters on [Name]

### {{ renderMember(type='int', name='Cursed') }}

:   Cursed counters on [Name]

### {{ renderMember(type='int', name='Diseased') }}

:   Diseased counters on [Name]

### {{ renderMember(type='int', name='Poisoned') }}

:   Poison counters on [Name]

### {{ renderMember(type='int', name='Corrupted') }}

:   Corruption counters on [Name]

### {{ renderMember(type='int', name='EnduDrain') }}

:   endurance drain counters on [Name]

### {{ renderMember(type='int', name='LifeDrain') }}

:   life drain counters on [Name]

### {{ renderMember(type='int', name='ManaDrain') }}

:   mana drain counters on [Name]

### {{ renderMember(type='int', name='Blinded') }}

:   blinded counters on [Name]

### {{ renderMember(type='int', name='CastingLevel') }}

:   Casting level counters on [Name]

### {{ renderMember(type='int', name='Charmed') }}

:   charm counters on [Name]

### {{ renderMember(type='int', name='Feared') }}

:   fear counters on [Name]

### {{ renderMember(type='int', name='Healing') }}

:   Healing counters on [Name]

### {{ renderMember(type='int', name='Invulnerable') }}

:   Invulnerable counters on [Name]

### {{ renderMember(type='int', name='Mesmerized') }}

:   mez counters on [Name]

### {{ renderMember(type='int', name='Rooted') }}

:   root counters on [Name]

### {{ renderMember(type='int', name='Silenced') }}

:   silence counters on [Name]

### {{ renderMember(type='int', name='Slowed') }}

:   slow counters on [Name]

### {{ renderMember(type='int', name='Snared') }}

:   snare counters on [Name]

### {{ renderMember(type='int', name='SpellCost') }}

:   spellcost counters on [Name]

### {{ renderMember(type='int', name='SpellSlowed') }}

:   slow spell counters on [Name]

### {{ renderMember(type='int', name='SpellDamage') }}

:   spell damage counters on [Name]

### {{ renderMember(type='int', name='Trigger') }}

:   trigger counters on [Name]

### {{ renderMember(type='int', name='Resistance') }}

:   resistance counters on [Name]

### {{ renderMember(type='string', name='Detrimental') }}

:   All detrimental types affecting [Name]

### {{ renderMember(type='int', name='NoCure') }}

:   nocure counters on [Name]

<!--dt-members-end-->

## Examples
<!--dt-examples-start-->
- Displays the duration remaining on the buff *Samwell* has in buff slot 4
: `/echo ${NetBots[Samwell].Duration[4]}`
<!--dt-examples-end-->

<!--dt-linkrefs-start-->
[bool]: ../macroquest/reference/data-types/datatype-bool.md
[class]: ../macroquest/reference/data-types/datatype-class.md
[float]: ../macroquest/reference/data-types/datatype-float.md
[int]: ../macroquest/reference/data-types/datatype-int.md
[spell]: ../macroquest/reference/data-types/datatype-spell.md
[string]: ../macroquest/reference/data-types/datatype-string.md
<!--dt-linkrefs-end-->
