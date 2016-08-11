PPhrases: SocPrep SocO PPhrases
PPhrases: SocPrep SocOA

SocO: SA_Words {{
   return ({ ([ "form": "O", "W": tree ]) });
}}
SocOA: SA_Words {{
   return ({ ([ "form": "OA", "W": tree ]) });
}}
SocAOA: SA_Words {{
   return ({ ([ "form": "AOA", "W": tree ]) });
}}

SocA: Word {{
    string *list;

    list = SOCIALS->expand_adverb_prefix(tree[0]);
    if (sizeof(list) > 0) {
	if (sizeof(list & ({ tree[0] })) == 0) {
	    object *obs;

	    obs = SOCIALS->query_verb_obs(tree[0]);
	    if (obs && sizeof(obs) > 0) {
		return nil;
	    }
	}
	return ({ ([ "form": "A", "adverb": tree[0] ]) });
    }
    return nil;
}}

SA_Words: SA_Word
SA_Words: SA_Words SA_Word
SA_Words: SA_Words ',' SA_Word

SA_Word: 'of'
SA_Word: word
SA_Word: __VERBS__
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
