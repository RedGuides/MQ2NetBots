---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2netbots.148/"
support_link: "https://www.redguides.com/community/threads/mq2netbots.66856/"
repository: "https://github.com/RedGuides/MQ2NetBots"
config: "<server>_<character>.ini"
authors: "s0rCieR, Deadchicken, Kroak, mijuki, woobs, eqmule, SwiftyMuse"
tagline: "Allow linked clients to share information between one another programatically"
---

# MQ2NetBots

<!--desc-start-->
Provides linked MQ2EQBC clients a way to share information, and also makes that information available in a TLO for other developers.
<!--desc-end-->

## Commands

<a href="cmd-netbots/">
{% 
  include-markdown "projects/mq2netbots/cmd-netbots.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2netbots/cmd-netbots.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2netbots/cmd-netbots.md') }}

<a href="cmd-netnote/">
{% 
  include-markdown "projects/mq2netbots/cmd-netnote.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2netbots/cmd-netnote.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2netbots/cmd-netnote.md') }}

## Settings

`<server>_<character>.ini` example,

```ini
[MQ2NetBots]
Stat=1
;send and receive data and processing, basically turn the plugin on or off
Grab=1
;Listen for NetBots data
Send=1
;Send Netbots data
```

## See also

- [MQ2EQBC](../mq2eqbc/index.md)
- [MQ2Status](../mq2status/index.md)
- [MQ2DanNet](../mq2dannet/index.md)

## Top-Level Objects

## [NetBots](tlo-netbots.md)
{% include-markdown "projects/mq2netbots/tlo-netbots.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2netbots/tlo-netbots.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2netbots/tlo-netbots.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2netbots/tlo-netbots.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}

## DataTypes

## [NetBots](datatype-netbots.md)
{% include-markdown "projects/mq2netbots/datatype-netbots.md" start="<!--dt-desc-start-->" end="<!--dt-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2netbots/datatype-netbots.md') }}

<h2>Members</h2>
{% include-markdown "projects/mq2netbots/datatype-netbots.md" start="<!--dt-members-start-->" end="<!--dt-members-end-->" %}
{% include-markdown "projects/mq2netbots/datatype-netbots.md" start="<!--dt-linkrefs-start-->" end="<!--dt-linkrefs-end-->" %}
