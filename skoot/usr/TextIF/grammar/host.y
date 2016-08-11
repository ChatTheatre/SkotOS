/*
 * ~TextIF/sys/grammar/host.y
 *
 * Parser for the (few) storyhost commands.
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
        <!-- Soul verbs are inserted automatically by ~TextIF/sys/parser/host.c -->
        <!--SOUL_VERBS-->
    </SOCIAL>

    <!-- Freeform commands -->

    <COMMAND verb="whereis"   format="*"/>
    <COMMAND verb="emote"     format="*"/>
    <COMMAND verb="echo"      format="*"/>
    <COMMAND verb="echoall"   format="*"/>
    <COMMAND verb="emit"      format="*"/>
    <COMMAND verb="emitall"   format="*"/>
    <COMMAND verb="summon"    format="*"/>
    <COMMAND verb="unsummon"  format="*"/>
    <COMMAND verb="partition" format="*"/>
    <COMMAND verb="obstat"    format="*"/>
    <COMMAND verb="refresh"   format="*"/>
    <COMMAND verb="clearprop" format="*"/>
    <COMMAND verb="inventory" format="*"/>

    <!-- Regular commands -->

    <COMMAND verb="stat" format="%s Obs EvokeP"/>

    <COMMAND verb="make" format="%s Obs"/>

    <COMMAND verb="resetprops" format="%s Obs"/>
    <COMMAND verb="resetprops"/>

    <COMMAND verb="resetpropsandclear" format="%s Obs"/>
    <COMMAND verb="resetpropsandclear"/>

    <COMMAND verb="use" format="%s Obs"/>

    <COMMAND verb="skills" keywords="list">
      <FORMAT value="%s"     function="return ({ tree[0], nil })"/>
      <FORMAT value="%s 'list'"       function="return ({ \"skills_list\", nil })"/>
      <FORMAT value="%s 'list' Evoke" function="return ({ \"skills_list\", tree[2][\"evoke\"] })"/>
      <FORMAT value="%s Ob"/>
    </COMMAND>

    <COMMAND verb="heal"/>

    <COMMAND verb="broadcast" format="%s EvokeP"/>

    <COMMAND verb="sh" keywords="all,off,who,recall,secure">
        <FORMAT value="%s EvokeP"       function="return ({ \"sh_chat\", FALSE, tree[1] });"/>
        <FORMAT value="%s 'all' EvokeP" function="return ({ \"sh_chat\", TRUE,  tree[2] });"/>
        <FORMAT value="%s 'on'"         function="return ({ \"sh_set\", TRUE            });"/>
        <FORMAT value="%s 'off'"        function="return ({ \"sh_set\", FALSE           });"/>
        <FORMAT value="%s 'who'"        function="return ({ \"sh_who\"                  });"/>
        <FORMAT value="%s 'recall'"     function="return ({ \"sh_recall\"               });"/>
        <FORMAT value="%s 'secure'"     function="return ({ \"sh_secure\"               });"/>
    </COMMAND>

    <COMMAND verb="setpass"/>
    <COMMAND verb="setpass" format="%s Word Word"/>

    <COMMAND verb="force" format="%s Obs EvokeP"/>
    <COMMAND verb="force"/>

    <COMMAND verb="woe" format="%s Words"/>
    <COMMAND verb="woe"/>

    <COMMAND verb="slay" format="%s 'override' Obs"/>
    <COMMAND verb="slay" format="%s Obs" function="return ({ tree[0], nil, tree[1] });"/>

    <COMMAND verb="whois" format="%s Word"/>
    <COMMAND verb="whois"/>

    <COMMAND verb="who" format="%s Word"/>
    <COMMAND verb="who"/>

    <COMMAND verb="accfinger" format="%s Words"/>
    <COMMAND verb="accfinger" format="%s cardinal" function="return ({ tree[0], ({ tree[1] }) })"/>
    <COMMAND verb="accfinger"/>

    <COMMAND verb="transfer" format="%s Words 'from' Words 'to' Words"/>
    <COMMAND verb="transfer"/>

    <COMMAND verb="finger" format="%s Words"/>
    <COMMAND verb="finger"/>

    <COMMAND verb="customers"/>
    <COMMAND verb="staff"/>
    <COMMAND verb="people"/>

    <COMMAND verb="page" format="%s Words EvokeP"/>
    <COMMAND verb="page" format="%s remoteid EvokeP"
                         function="return ({ \"page_remote\", ({ tree[1] }), tree[2] })"/>
    <COMMAND verb="page" format="%s Words remoteid EvokeP"
                         function="return ({ \"page_remote\", tree[1] + ({ tree[2] }), tree[3] })"/>
    <COMMAND verb="page"/>

    <COMMAND verb="busy">
	<FORMAT value="%s 'on'"/>
	<FORMAT value="%s 'off'" keywords="off"/>
	<FORMAT value="%s 'yes'" keywords="yes"/>
	<FORMAT value="%s 'no'"  keywords="no"/>
        <FORMAT value="%s"/>
    </COMMAND>

    <COMMAND verb="lock"   format="%s Obs"/>

    <COMMAND verb="unlock" format="%s Obs"/>

    <COMMAND verb="ren" format="%s Obs EvokeP"/>
    <COMMAND verb="ren"/>

    <COMMAND verb="ren_ip" format="%s Obs EvokeP"/>
    <COMMAND verb="ren_ip"/>

    <COMMAND verb="list" format="%s Obs EvokeP"/>
    <COMMAND verb="list"/>

    <COMMAND verb="erasescript" format="%s Obs EvokeP"/>
    <COMMAND verb="erasescript"/>

    <COMMAND verb="scripts" format="%s Obs"/>
    <COMMAND verb="scripts"/>

    <COMMAND verb="drag">
	<FORMAT value="%s 'nothing'" keywords="nothing"/>
	<FORMAT value="%s 'nobody'"  keywords="nobody"/>
	<FORMAT value="%s 'noone'"   keywords="noone"/>
	<FORMAT value="%s Obs"/>
	<FORMAT value="%s"/>
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

    <COMMAND verb="possess" format="%s Obs"/>

    <COMMAND verb="snoop"   format="%s Obs"/>

    <COMMAND verb="badname"  format="%s EvokeP"/>
    <COMMAND verb="badnames" format="%s EvokeP"/>
    <COMMAND verb="goodname" format="%s EvokeP"/>

    <COMMAND verb="info"     format="%s Word"/>

    <COMMAND verb="notes"    format="%s Word"/>
    <COMMAND verb="addnote"  format="%s Word EvokeP"/>
    <COMMAND verb="killnote" format="%s Word"/>

    <COMMAND verb="npc"      format="%s Words"/>
    <COMMAND verb="npc"/>

    <COMMAND verb="infoline" keywords="recall">
      <FORMAT value="%s 'on'"/>
      <FORMAT value="%s 'off'"/>
      <FORMAT value="%s 'recall'"/>
      <FORMAT value="%s"/>
    </COMMAND>

    <COMMAND verb="statistics" format="%s Word"/>
    <COMMAND verb="statistics" format="%s"/>

    <COMMAND verb="suspend" keywords="add,remove,status">
      <FORMAT value="%s"/>
      <FORMAT value="%s 'add' Word EvokeP"/>
      <FORMAT value="%s 'remove' Word"/>
      <FORMAT value="%s 'status' Word"/>
    </COMMAND>

    <COMMAND verb="roster">
      <FORMAT value="%s"
              function="return ({ \"roster_help\" })"/>
      <FORMAT value="%s Word"
              function="return ({ \"roster_check\", tree[1] })"/>
      <FORMAT value="%s Word cardinal"
              function="return ({ \"roster_change\", tree[1], tree[2] })"/>
    </COMMAND>

    <COMMAND verb="status">
      <FORMAT value="%s Word"/>
      <FORMAT value="%s"/>
    </COMMAND>

    <COMMAND verb="users"/>

    <COMMAND verb="objects">
      <FORMAT value="%s"/>
      <FORMAT value="%s cardinal" function="return ({ tree[0], (int)tree[1] })"/>
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

    <COMMAND verb="storypoints" keywords="help,info,take,log,give">
      <FORMAT value="%s"                            function="return ({ \"storypoints_help\" })"/>
      <FORMAT value="%s 'help'"                     function="return ({ \"storypoints_help\" })"/>
      <FORMAT value="%s 'info' Word"                function="return ({ \"storypoints_info\", tree[2] })"/>
      <FORMAT value="%s 'take' Word Evoke"          function="return ({ \"storypoints_take\", tree[2], 50,           tree[3] })"/>
      <FORMAT value="%s 'take' Word cardinal Evoke" function="return ({ \"storypoints_take\", tree[2], (int)tree[3], tree[4] })"/>
      <FORMAT value="%s 'give' Word Evoke"          function="return ({ \"storypoints_give\", tree[2], 50,           tree[3] })"/>
      <FORMAT value="%s 'give' Word cardinal Evoke" function="return ({ \"storypoints_give\", tree[2], (int)tree[3], tree[4] })"/>
      <FORMAT value="%s 'log'"                      function="return ({ \"storypoints_log\",  7            })"/>
      <FORMAT value="%s 'log'  cardinal"            function="return ({ \"storypoints_log\",  (int)tree[2] })"/>
      <FORMAT value="%s 'log'  Word"                function="return ({ \"storypoints_log_user\",  tree[2], 7     })"/>
      <FORMAT value="%s 'log'  Word cardinal"       function="return ({ \"storypoints_log_user\",  tree[2], (int)tree[3] })"/>
    </COMMAND>

    <COMMAND verb="ipnames" format="%s"/>

    <!--#additional -->

</COMMANDS>

_Word: _Art
_Word: _Ob
_Word: _PrepWords
_Word: __KEYWORDS__
_Word: __VERBS__
_Word: __FREE__

CardinalList:              cardinal {{ return ({ tree });                    }}
CardinalList: CardinalList cardinal {{ return ({ tree[0] + ({ tree[1] }) }); }}

# include "~/grammar/always.y"
# include "~/grammar/art.y"
# include "~/grammar/evoke.y"
# include "~/grammar/amount.y"
# include "~/grammar/obs.y"
# include "~/grammar/prep.y"
# include "~/grammar/numbers.y"
# include "~/grammar/social_ob.y"
# include "~/grammar/words.y"

GrammarEnd
