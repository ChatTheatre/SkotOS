/*
 * ~TextIF/grammar/basic.y
 *
 * Parser for the main set of commands, social verbs, etc.
 * StoryGuide and StoryHost commands are parsed elsewhere.
 *
 * TODO:
 *   - handle ambiguities like 'help the old woman' (?)
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

# include "~/grammar/always.y"


<COMMANDS>

    <!-- A set of commands: Soul! -->
    <SOCIAL>
        <!-- These are not in social_formats.y so we do not have to deal with
             e.g. +foo and !foo commands starting with adverbs -->
        <FORMAT value="SocA %s EvokeP"               function="return ({ \"social\" }) + make_social(tree);"/>
        <FORMAT value="SocA %s SocO EvokeP"          function="return ({ \"social\" }) + make_social(tree);"/>
        <FORMAT value="SocA %s PPhrases EvokeP"      function="return ({ \"social\" }) + make_social(tree);"/>
        <FORMAT value="SocA %s SocO PPhrases EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>

        <!--#include file="/usr/TextIF/grammar/social_formats.y" -->
        <!-- Soul verbs are inserted automatically by ~TextIF/sys/parser/basic.c -->
        <!--SOUL_VERBS-->
    </SOCIAL>

    <!-- A set of commands: 'stances' -->
    <COMMAND>
	<VERB value="crouch"/>
	<VERB value="kneel"/>
	<VERB value="lie"/>
	<VERB value="sit"/>
	<VERB value="stand"/>
	<FORMAT value="%s Dir Prep_Ob"  function="return ({ \"stance\", tree[0], tree[1], tree[2], tree[3] });"/>
	<FORMAT value="%s Prep_Ob"      function="return ({ \"stance\", tree[0], nil,     tree[1], tree[2] });"/>
	<FORMAT value="%s Dir"          function="return ({ \"stance\", tree[0], tree[1], nil,     nil     });"/>
	<FORMAT value="%s Ob"           function="return ({ \"stance\", tree[0], nil,     nil,     tree[1] });"/>
	<FORMAT value="%s"              function="return ({ \"stance\", tree[0], nil,     nil,     nil     });"/>
    </COMMAND>

    <!-- Freeform commands. -->

    <COMMAND verb="help" format="*"/>

    <!-- Regular commands: -->

    <COMMAND verb="who"/>
    <COMMAND verb="who" format="%s Word"/>

    <COMMAND verb="exits"/>

    <COMMAND verb="profile" keywords="theme,themes,none">
        <FORMAT value="%s"/>
        <FORMAT value="%s 'theme'"/>
        <FORMAT value="%s 'theme' 'none'"/>
        <FORMAT value="%s 'theme' Words"/>
        <FORMAT value="%s 'themes'"/>
    </COMMAND>

    <COMMAND verb="page" keywords="busy,accept,block,unblock,allow,unallow,deny,dismiss,status,recall">
	<FORMAT value="%s 'busy'"             function="return ({ \"page_consent\", \"busy\"                   });"/>
	<FORMAT value="%s 'busy' 'on'"        function="return ({ \"page_consent\", \"busy\",     ({ \"on\" }) });"/>
	<FORMAT value="%s 'busy' Words"       function="return ({ \"page_consent\", \"busy\",          tree[2] });"/>

	<FORMAT value="%s 'block'"            function="return ({ \"page_consent\", \"block\"                  });"/>
	<FORMAT value="%s 'block' Words"      function="return ({ \"page_consent\", \"block\",         tree[2] });"/>
	<FORMAT value="%s 'block' remoteid"
                function="return ({ \"page_consent\", \"block_remote\", ({ tree[2] }) });"/>
	<FORMAT value="%s 'block' Words remoteid"
                function="return ({ \"page_consent\", \"block_remote\", tree[2] + ({ tree[3] }) });"/>

	<FORMAT value="%s 'unblock'"          function="return ({ \"page_consent\", \"unblock\"                });"/>
	<FORMAT value="%s 'unblock' Words"    function="return ({ \"page_consent\", \"unblock\",       tree[2] });"/>
	<FORMAT value="%s 'unblock' remoteid"
                function="return ({ \"page_consent\", \"unblock_remote\", ({ tree[2] }) });"/>
	<FORMAT value="%s 'unblock' Words remoteid"
                function="return ({ \"page_consent\", \"unblock_remote\", tree[2] + ({ tree[3] }) });"/>

	<FORMAT value="%s 'allow'"            function="return ({ \"page_consent\", \"allow\"                  });"/>
	<FORMAT value="%s 'allow' Words"      function="return ({ \"page_consent\", \"allow\",         tree[2] });"/>
	<FORMAT value="%s 'allow' remoteid"
                function="return ({ \"page_consent\", \"allow_remote\", ({ tree[2] }) });"/>
	<FORMAT value="%s 'allow' Words remoteid"
                function="return ({ \"page_consent\", \"allow_remote\", tree[2] + ({ tree[3] }) });"/>

	<FORMAT value="%s 'unallow'"          function="return ({ \"page_consent\", \"unallow\"                });"/>
	<FORMAT value="%s 'unallow' Words"    function="return ({ \"page_consent\", \"unallow\",       tree[2] });"/>
	<FORMAT value="%s 'unallow' remoteid"
                function="return ({ \"page_consent\", \"unallow_remote\", ({ tree[2] }) });"/>
	<FORMAT value="%s 'unallow' Words remoteid"
                function="return ({ \"page_consent\", \"unallow_remote\", tree[2] + ({ tree[3] }) });"/>

	<FORMAT value="%s 'accept'"           function="return ({ \"page_consent\", \"accept\"                 });"/>
	<FORMAT value="%s 'accept' Words"     function="return ({ \"page_consent\", \"accept\",        tree[2] });"/>
	<FORMAT value="%s 'accept' remoteid"
                function="return ({ \"page_consent\", \"accept_remote\", ({ tree[2] }) });"/>
	<FORMAT value="%s 'accept' Words remoteid"
                function="return ({ \"page_consent\", \"accept_remote\", tree[2] + ({ tree[3] }) });"/>

	<FORMAT value="%s 'deny'"
                function="return ({ \"page_consent\", \"deny\" });"/>
	<FORMAT value="%s 'deny' Words"
                function="return ({ \"page_consent\", \"deny\", tree[2] });"/>
	<FORMAT value="%s 'deny' remoteid"
                function="return ({ \"page_consent\", \"deny_remote\", ({ tree[2] }) });"/>
	<FORMAT value="%s 'deny' Words remoteid"
                function="return ({ \"page_consent\", \"deny_remote\", tree[2] + ({ tree[3] }) });"/>

	<FORMAT value="%s 'dismiss'"          function="return ({ \"page_consent\", \"deny\"                   });"/>
	<FORMAT value="%s 'dismiss' Words"    function="return ({ \"page_consent\", \"deny\",          tree[2] });"/>
	<FORMAT value="%s 'dismiss' remoteid"
                function="return ({ \"page_consent\", \"deny_remote\", ({ tree[2] }) });"/>
	<FORMAT value="%s 'dismiss' Words remoteid"
                function="return ({ \"page_consent\", \"deny_remote\", tree[2] + ({ tree[3] }) });"/>

        <FORMAT value="%s 'status'"           function="return ({ \"page_consent\" })"/>
        <FORMAT value="%s 'recall' Words"     function="return ({ \"page_recall\", tree[2] })"/>
        <FORMAT value="%s 'recall'"           function="return ({ \"page_recall\" })"/>
	<FORMAT value="%s Words EvokeP"/>
	<FORMAT value="%s remoteid EvokeP"
                function="return ({ \"page_remote\", ({ tree[1] }), tree[2] });"/>
	<FORMAT value="%s Words remoteid EvokeP"
                function="return ({ \"page_remote\", tree[1] + ({ tree[2] }), tree[3] });"/>
        <FORMAT value="%s"/>
    </COMMAND>

    <COMMAND verb="wield"    format="%s Obs"/>

    <COMMAND verb="unwield"  format="%s Obs"/>

    <COMMAND verb="remove">
        <FORMAT value="%s Obs 'from' Obs" function="return ({ tree[0], tree[1], tree[3] });"/>
        <FORMAT value="%s Obs"            function="return ({ tree[0], tree[1], nil     });"/>
    </COMMAND>

    <COMMAND verb="unwear"   format="%s Obs"/>

    <COMMAND verb="wear"     format="%s Obs"/>

    <COMMAND verb="go"       format="%s Obs"/>

    <COMMAND verb="enter"    format="%s Obs"/>

    <COMMAND verb="leave"    format="%s Obs"/>
    <COMMAND verb="leave"/>

    <COMMAND verb="approach" format="%s Obs"/>
    <COMMAND verb="approach"/>

    <COMMAND verb="take"     format="%s Obs Prep Obs"      function="return ({ tree[0], tree[1], tree[3] });"/>
    <COMMAND verb="take"     format="%s Obs"               function="return ({ tree[0], tree[1], nil });"/>
    <COMMAND verb="get"      format="%s Obs Prep Obs"      function="return ({ \"take\", tree[1], tree[3] });"/>
    <COMMAND verb="get"      format="%s Obs"               function="return ({ \"take\", tree[1], nil });"/>

    <COMMAND verb="address"  format="%s Obs 'to' EvokeP"/>

    <COMMAND verb="give"     format="%s Obs Prep Obs"/>
    <COMMAND verb="give"/>

    <COMMAND verb="offer"    format="%s Obs Prep Obs"/>
    <COMMAND verb="offer"    format="%s Ob Obs" function="return ({ tree[0], tree[2], \"to\", ({ tree[1] }) });"/>
    <COMMAND verb="offer"    function="return ({ tree[0], nil, nil, nil });"/>

    <COMMAND verb="accept"   format="%s 'from' Ob"     function="return ({ tree[0], nil,     tree[2] });"/>
    <COMMAND verb="accept"   format="%s Obs 'from' Ob" function="return ({ tree[0], tree[1], tree[3] });"/>
    <COMMAND verb="accept"   format="%s Obs"           function="return ({ tree[0], tree[1], nil });"/>
    <COMMAND verb="accept"                             function="return ({ tree[0], nil, nil });"/>

    <COMMAND verb="tip"/>
    <COMMAND verb="tip"      format="%s Words"/>

    <COMMAND verb="follow" keywords="nobody,nothing,noone">
        <FORMAT value="%s"/>
        <FORMAT value="%s 'nothing'" keywords="nothing"/>
        <FORMAT value="%s 'nobody'"  keywords="nobody"/>
        <FORMAT value="%s 'noone'"   keywords="noone"/>
        <FORMAT value="%s Obs"/>
    </COMMAND>

    <COMMAND verb="more"/>
    <COMMAND verb="more"     format="%s Words"/>

    <!--#include configd="Base:Skills:Grammar" -->
    <!--#additional -->

</COMMANDS>

Dir: 'up'
Dir: 'down'

Prep_Ob_PrepWords: PrepWords {{ return ({ tree }); }}
Prep_Ob: Prep_Ob_PrepWords SA_Words {{ return sanitize_prep_ob(tree); }}

# Extend the _Word rule so that these keywords can be used as regular
# words outside their regular context.

# include "~/grammar/amount.y"
# include "~/grammar/art.y"
# include "~/grammar/evoke.y"
# include "~/grammar/numbers.y"
# include "~/grammar/obs.y"
# include "~/grammar/prep.y"
# include "~/grammar/social_ob.y"
# include "~/grammar/words.y"

_Word: __VERBS__
_Word: __KEYWORDS__
_Word: Dir
_Word: _Art
_Word: _PrepWords
_Word: _Amount
_Word: 'nobody'
_Word: 'noone'
_Word: 'nobody'

GrammarEnd
