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

    <COMMAND verb="obstat"    format="*"/>
    <COMMAND verb="clearprop" format="*"/>
    <COMMAND verb="inventory" format="*"/>

    <!-- Regular commands -->

    <COMMAND verb="stat" format="%s Obs EvokeP"/>

    <COMMAND verb="sh" keywords="all,off,who,recall,secure">
        <FORMAT value="%s EvokeP"       function="return ({ \"sh_chat\", FALSE, tree[1] });"/>
        <FORMAT value="%s 'all' EvokeP" function="return ({ \"sh_chat\", TRUE,  tree[2] });"/>
        <FORMAT value="%s 'on'"         function="return ({ \"sh_set\", TRUE            });"/>
        <FORMAT value="%s 'off'"        function="return ({ \"sh_set\", FALSE           });"/>
        <FORMAT value="%s 'who'"        function="return ({ \"sh_who\"                  });"/>
        <FORMAT value="%s 'recall'"     function="return ({ \"sh_recall\"               });"/>
        <FORMAT value="%s 'secure'"     function="return ({ \"sh_secure\"               });"/>
    </COMMAND>

    <COMMAND verb="force" format="%s Obs EvokeP"/>
    <COMMAND verb="force"/>

    <COMMAND verb="slay" format="%s 'override' Obs"/>
    <COMMAND verb="slay" format="%s Obs" function="return ({ tree[0], nil, tree[1] });"/>

    <COMMAND verb="transfer" format="%s Words 'from' Words 'to' Words"/>
    <COMMAND verb="transfer"/>

    <COMMAND verb="page" format="%s Words EvokeP"/>
    <COMMAND verb="page" format="%s remoteid EvokeP"
                         function="return ({ \"page_remote\", ({ tree[1] }), tree[2] })"/>
    <COMMAND verb="page" format="%s Words remoteid EvokeP"
                         function="return ({ \"page_remote\", tree[1] + ({ tree[2] }), tree[3] })"/>
    <COMMAND verb="page"/>

    <COMMAND verb="possess" format="%s Obs"/>

    <COMMAND verb="notes"    format="%s Word"/>
    <COMMAND verb="addnote"  format="%s Word EvokeP"/>
    <COMMAND verb="killnote" format="%s Word"/>

    <COMMAND verb="infoline" keywords="recall">
      <FORMAT value="%s 'on'"/>
      <FORMAT value="%s 'off'"/>
      <FORMAT value="%s 'recall'"/>
      <FORMAT value="%s"/>
    </COMMAND>

    <COMMAND verb="suspend" keywords="add,remove,status">
      <FORMAT value="%s"/>
      <FORMAT value="%s 'add' Word EvokeP"/>
      <FORMAT value="%s 'remove' Word"/>
      <FORMAT value="%s 'status' Word"/>
    </COMMAND>

    <COMMAND verb="status">
      <FORMAT value="%s Word"/>
      <FORMAT value="%s"/>
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
