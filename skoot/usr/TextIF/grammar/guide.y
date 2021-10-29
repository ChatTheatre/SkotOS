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

    <!-- SG aliases of SH command(s) -->
    <COMMAND verb="hotspots"  format="*"/>

    <!-- Regular commands -->
    <COMMAND verb="page"    format="%s Words EvokeP"/>
    <COMMAND verb="page"/>

    <!-- SG aliases of SH command(s) -->
    <COMMAND verb="customers"/>
    <COMMAND verb="people"/>
    <COMMAND verb="notes"     format="%s Word"/>
    <COMMAND verb="addnote"   format="%s Word EvokeP"/>
    <COMMAND verb="killnote"  format="%s Word"/>
    <COMMAND verb="infoline" keywords="recall">
      <FORMAT value="%s 'on'"/>
      <FORMAT value="%s 'off'"/>
      <FORMAT value="%s 'recall'"/>
      <FORMAT value="%s"/>
    </COMMAND>
    <COMMAND verb="disconnect" format="%s Obs"/>
    <COMMAND verb="suspend" keywords="add,remove,status">
      <FORMAT value="%s"/>
      <FORMAT value="%s 'add' Word EvokeP"/>
      <FORMAT value="%s 'remove' Word"/>
      <FORMAT value="%s 'status' Word"/>
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
