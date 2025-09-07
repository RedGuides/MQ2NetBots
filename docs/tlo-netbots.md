---
tags:
  - tlo
---
# `NetBots`

<!--tlo-desc-start-->
Returns information about the client, but also holds a data type of the same name which is much more interesting.
<!--tlo-desc-end-->

## Forms
<!--tlo-forms-start-->
### {{ renderMember(type='NetBots', name='NetBots') }}

:   Returns information about the client, and is used without index for data type members: Enable, Listen, Output, Counts and Client.

### {{ renderMember(type='NetBots', name='NetBots', params='charactername') }}

:   Returns information about the connected character, and is used for nearly all of the data types.

<!--tlo-forms-end-->

## Associated DataTypes
<!--tlo-datatypes-start-->
## [`NetBots`](datatype-netbots.md)
{% include-markdown "projects/mq2netbots/datatype-netbots.md" start="<!--dt-desc-start-->" end="<!--dt-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2netbots/datatype-netbots.md') }}
:    <h3>Members</h3>
    {% include-markdown "projects/mq2netbots/datatype-netbots.md" start="<!--dt-members-start-->" end="<!--dt-members-end-->" %}
    {% include-markdown "projects/mq2netbots/datatype-netbots.md" start="<!--dt-linkrefs-start-->" end="<!--dt-linkrefs-end-->" %}
    <!--tlo-datatypes-end-->

## Examples
<!--tlo-examples-start-->
`/echo ${NetBots.Client}` Returns a list of clients currently broadcasting.
<!--tlo-examples-end-->

<!--tlo-linkrefs-start-->
[netbots]: datatype-netbots.md
<!--tlo-linkrefs-end-->