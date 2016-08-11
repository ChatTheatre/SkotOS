# ---- begin ~TextIF/grammar/art.y ----

Art: 'last' 	 {{ return ({ ART_LAST }) }}
Art: 'all'	 {{ return ({ ART_ALL }) }}
Art: 'all' 'the' {{ return ({ ART_ALL }) }}
Art: 'the'	 {{ return ({ ART_DEF }) }}
Art: 'a'	 {{ return ({ ART_UNDEF }) }}
Art: 'an'	 {{ return ({ ART_UNDEF }) }}

# Keywords used in the Ob rules:
_Art: 'last'
_Art: 'all'
_Art: 'all'
_Art: 'the'
_Art: 'a'
_Art: 'an'

# ---- end ~TextIF/grammar/art.y ----
