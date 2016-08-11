# include "/usr/SkotOS/include/inglish.h"

inherit "/lib/string";
inherit "/usr/TextIF/lib/numbers";

# include "/usr/SkotOS/include/noun.h"

GrammarBegin

whitespace = /[ \t]+/

cardinal   = /[0-9]+/

ordinal    = /[1-9][0-9]*0th/
ordinal    = /([1-9][0-9]*)?1[1-3]th/
ordinal    = /([1-9][0-9]*)?1st/
ordinal    = /([1-9][0-9]*)?2nd/
ordinal    = /([1-9][0-9]*)?3rd/
ordinal    = /([1-9][0-9]*)?[4-9]th/

word       = /[a-zA-Z][a-zA-Z0-9-]*/
word       = /[0-9]+[a-zA-Z][a-zA-Z0-9-]*/

Entry: Digits           {{ return ({ tree[0] });                          }}
Entry: Ordinal AdjNoun  {{ return ({ NewOb(nil, 0, -tree[0], tree[1]) }); }}
Entry: AdjNoun Cardinal {{ return ({ NewOb(nil, 0, -tree[1], tree[0]) }); }}
Entry: AdjNoun          {{ return ({ NewOb(nil, 0,  0,       tree[0]) }); }}

Digits: cardinal        {{ return ({ (int)tree[0] });                     }}

AdjNoun: Words

Words: word             {{ return ({ tree });                             }}
Words: Words word       {{ return ({ tree[0] + ({ tree[1] }) });          }}

# include "~/grammar/numbers.y"

GrammarEnd
