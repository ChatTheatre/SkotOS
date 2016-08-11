PPhrases: PPhrases PPhrase
PPhrases: PPhrase

PPhrase: SocPrep SocOb

SocOb: Obs  {{
   NRef *obs;

   if (obs = try_locate(tree[0])) {
      if (sizeof(obs)) {
	 return ({ ([ "form": "O", "O": obs ]) });
      }
      return ({ ([ "form": "X", "X": tree[0] ]) });
   }
   return nil;
}}

JunkyPPhrases: JunkyPPhrases JunkyPPhrase
JunkyPPhrases: JunkyPPhrase

JunkyPPhrase: SocPrep JunkySocOb

PWordPhrases: PWordPhrases PWordPhrase
PWordPhrases: PWordPhrase

PWordPhrase: SocPrep SocWords

JunkySocOb: SocOb
JunkySocOb: SA_Words {{
   return ({ ([ "form": "W", "W": tree ]) });
}}

SocWords: SA_Words {{
   return ({ ([ "form": "Z", "Z": tree ]) });
}}

SA_Words: SA_Word
SA_Words: SA_Words SA_Word

SA_Word: word
SA_Word: __KEYWORDS__
SA_Word: _Art
SA_Word: _Amount
SA_Word: _Ob
SA_Word: _Poss
SA_Word: poss
SA_Word: Cardinal_Words
SA_Word: Ordinal_Words
_Word: 'nobody'
_Word: 'noone'
_Word: 'nobody'

Adv: Word {{
    if (sizeof(SOCIALS->expand_adverb_prefix(tree[0])) > 0) {
	return ({ ([ "form": "A", "adverb": tree[0] ]) });
    }
    return nil;
}}
