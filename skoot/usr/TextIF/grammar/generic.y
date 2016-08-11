# include <System.h>

inherit "/lib/string";

GrammarBegin

whitespace   = /[ \t]+/
poss         = /[a-zA-Z_][a-zA-Z0-9_]*\'/
poss         = /[a-zA-Z_][a-zA-Z0-9_]*\'s/
atword       = /@[a-zA-Z0-9_]+/
word         = /[a-zA-Z0-9_][a-zA-Z0-9-_:]*/
remoteid     = /[a-zA-Z0-9_][a-zA-Z0-9_-]+@[a-zA-Z][a-zA-Z0-9:]+/
evoke        = /\".*/
evoke        = /\'.*/
other        = /./

Cmd:    Words EvokeP        {{ return ({ tree[0] + tree[1] });              }}
Cmd:    AtWord Words EvokeP {{ return ({ tree[..0] + tree[1] + tree[2] });  }}
Cmd:    AtWord EvokeP       {{ return ({ tree[..0] + tree[1] });            }}

AtWord:	atword          {{ return ({ lower_case(tree[0]) }); }}

Words:  Word            {{ return ({ tree                    }); }}
Words:  Words     Word  {{ return ({ tree[0] + ({ tree[1] }) }); }}
Words:  Words ',' Word  {{ return ({ tree[0] + tree[1..2]    }); }}
Words:  Words '-' Word  {{ return ({ tree[0] + tree[1..2]    }); }}

Word:   _Word        {{ return ({ lower_case(tree[0]) });     }}

_Word:  poss
_Word:  word
_Word:  remoteid

EvokeP:              {{ return ({ ({ }) });                   }}
EvokeP: ',' evoke    {{ return ({ tree[1..] });               }}
EvokeP: evoke        {{ return ({ tree });                    }}

GrammarEnd
