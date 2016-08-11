/*
 * /usr/TextIF/grammar/social.y
 *
 * This grammar's only purpose is to enable the basic.y grammar to merely
 * collect a sequence of words instead of prematurely (and often incorrectly)
 * deciding that the last word is an adverb or, alternatively, leave it up to
 * the social.c code which is then not in a very good position to sort it out.
 *
 * By letting the social.c code parse all or a subsection of the collected
 * words itself and, knowing which combinations make more or less sense, then
 * deciding which one to go for or complain about the ambiguity, we avoid most
 * of the problems.
 */

# include <SkotOS.h>
# include "/usr/SkotOS/include/inglish.h"

inherit "/lib/string";
inherit "/usr/TextIF/lib/numbers";

# include "/usr/SkotOS/include/noun.h"

GrammarBegin

Line: Obs

# include "~/grammar/always.y"
# include "~/grammar/art.y"
# include "~/grammar/numbers.y"
# include "~/grammar/amount.y"
# include "~/grammar/obs.y"
# include "~/grammar/prep.y"
# include "~/grammar/words.y"

_Word: _Amount
_Word: _Art
#_Word: __VERBS__
#_Word: _Ob
_Word: _PrepWords

GrammarEnd
