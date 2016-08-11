# ---- begin ~TextIF/grammar/words.y ----
Words:  Word	   {{ return ({ tree })                    }}
Words:  Words Word {{ return ({ tree[0] + ({ tree[1] }) }) }}
Word:   _Word      {{ return ({ lower_case(tree[0]) });    }}

# It's up to the toplevel to define what else _Word may be.
_Word: word
# ---- end ~TextIF/grammar/words.y ----
