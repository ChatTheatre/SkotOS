/*
 * ~TextIF/grammar/guide.y
 *
 * Parser for the (few) storyguide commands.
 */

# include <System.h>
# include <SkotOS.h>
# include "/usr/SkotOS/include/inglish.h"
# include <base.h>

inherit "/lib/string";
inherit "/usr/TextIF/lib/numbers";
inherit "/usr/TextIF/lib/social_grammar";

# include "/usr/SkotOS/include/noun.h"

GrammarBegin

<COMMANDS>
    <!-- A set of commands: Soul! -->
    <SOCIAL>
        <!--#include file="/usr/TextIF/grammar/social_formats.y" -->
        <!-- Soul verbs are inserted automatically by ~TextIF/sys/parser/guide.c -->
        <!--SOUL_VERBS-->
    </SOCIAL>

    <!-- Freeform commands -->
    <COMMAND verb="whereis" format="*"/>

    <!-- SG aliases of SH command(s) -->
    <COMMAND verb="hotspots"  format="*"/>

    <!-- Regular commands -->
    <COMMAND verb="page"    format="%s Words EvokeP"/>
    <COMMAND verb="page"/>
    <COMMAND verb="swap"/>

    <!-- SG aliases of SH command(s) -->
    
    <COMMAND verb="accfinger" format="%s cardinal" function="return ({ tree[0], ({ tree[1] }) })"/>
    <COMMAND verb="accfinger"/>
    <COMMAND verb="broadcast" format="%s EvokeP"/>
    <COMMAND verb="finger" format="%s Words"/>
    <COMMAND verb="finger"/>
    <COMMAND verb="customers"/>
    <COMMAND verb="people"/>
    <COMMAND verb="notes"     format="%s Word"/>
    <COMMAND verb="addnote"   format="%s Word EvokeP"/>
    <COMMAND verb="killnote"  format="%s Word"/>
    <COMMAND verb="badname"   format="%s EvokeP"/>
    <COMMAND verb="badnames"  format="%s EvokeP"/>
    <COMMAND verb="goodname"  format="%s EvokeP"/>
    <COMMAND verb="drag">
	<FORMAT value="%s 'nothing'" keywords="nothing"/>
	<FORMAT value="%s 'nobody'"  keywords="nobody"/>
	<FORMAT value="%s 'noone'"   keywords="noone"/>
	<FORMAT value="%s Obs"/>
	<FORMAT value="%s"/>
    </COMMAND>
    <COMMAND verb="busy">
	<FORMAT value="%s 'on'"/>
	<FORMAT value="%s 'off'" keywords="off"/>
	<FORMAT value="%s 'yes'" keywords="yes"/>
	<FORMAT value="%s 'no'"  keywords="no"/>
        <FORMAT value="%s"/>
    </COMMAND>
    <COMMAND verb="infoline" keywords="recall">
      <FORMAT value="%s 'on'"/>
      <FORMAT value="%s 'off'"/>
      <FORMAT value="%s 'recall'"/>
      <FORMAT value="%s"/>
    </COMMAND>
    <COMMAND verb="log" keywords="assign,assignees,grep,show">
      <FORMAT value="%s"/>
      <FORMAT value="%s 'grep' Words EvokeP"          function="return ({ \"loggrep\",   tree[2], tree[3] });"/>
      <FORMAT value="%s 'grep' EvokeP"                function="return ({ \"loggrep\",   nil,     tree[2] });"/>
      <FORMAT value="%s 'assign' CardinalList EvokeP" function="return ({ \"logassign\", tree[2], tree[3] });"/>
      <FORMAT value="%s 'assignees'"                  function="return ({ \"logassignees\"                });"/>
      <FORMAT value="%s 'show' cardinal"              function="return ({ \"logshow\",   tree[2]          });"/>
      <FORMAT value="%s 'show' cardinal"              function="return ({ \"logshow\",   tree[2]          });"/>
    </COMMAND>
    <COMMAND verb="disconnect" format="%s Obs"/>
    <COMMAND verb="suspend" keywords="add,remove,status">
      <FORMAT value="%s"/>
      <FORMAT value="%s 'add' Word EvokeP"/>
      <FORMAT value="%s 'remove' Word"/>
      <FORMAT value="%s 'status' Word"/>
    </COMMAND>
    <COMMAND verb="grab" keywords="override">
        <FORMAT value="%s"/>
        <FORMAT value="%s 'override' Words" function="return ({ \"grab\", tree[2], TRUE })"/>
        <FORMAT value="%s Words"/>
    </COMMAND>
    <COMMAND verb="ungrab">
        <FORMAT value="%s"/>
        <FORMAT value="%s Words"/>
    </COMMAND>


    <!--#additional -->

</COMMANDS>

CardinalList:              cardinal {{ return ({ tree });                    }}
CardinalList: CardinalList cardinal {{ return ({ tree[0] + ({ tree[1] }) }); }}

_Word: _Art
_Word: _Ob
_Word: _PrepWords
_Word: __KEYWORDS__
_Word: __VERBS__
_Word: __FREE__

# include "~/grammar/always.y"
# include "~/grammar/amount.y"
# include "~/grammar/art.y"
# include "~/grammar/evoke.y"
# include "~/grammar/numbers.y"
# include "~/grammar/obs.y"
# include "~/grammar/prep.y"
# include "~/grammar/social_ob.y"
# include "~/grammar/words.y"

GrammarEnd
