# ---- begin ~TextIF/grammar/obs.y ----
Obs: Ob			{{ return ({ ({ tree[0] }) }) }}
Obs: Obs ',' Ob		{{ return ({ tree[0] + ({ tree[2] }) }) }}
Obs: Obs 'and' Ob	{{ return ({ tree[0] + ({ tree[2] }) }) }}

# ???
# Poss: Art CO AdjNounP {{ return ({ NewOb(nil, tree[0], tree[1], tree[2]) }); }}

Here: 'here'       {{ return ({ NewOb(nil, ART_IN_ENV, 0, ({ })) }); }}

Ob: 'myself'       {{ return ({ NewOb(nil, ART_ME,  0,       ({ }))   }); }}
Ob: 'my' 'self'    {{ return ({ NewOb(nil, ART_ME,  0,       ({ }))   }); }}
Ob: 'me'           {{ return ({ NewOb(nil, ART_ME,  0,       ({ }))   }); }}
Ob: 'him'          {{ return ({ NewOb(nil, ART_HIM, 0,       ({ }))   }); }}
Ob: 'her'          {{ return ({ NewOb(nil, ART_HER, 0,       ({ }))   }); }}
Ob: 'it'           {{ return ({ NewOb(nil, ART_IT,  0,       ({ }))   }); }}
Ob: 'all'          {{ return ({ NewOb(nil, ART_ALL, 0,       ({ }))   }); }}
Ob: 'here'         {{ return ({ NewOb(nil, ART_ENV, 0,       ({ }))   }); }}

# experimental
Ob: Art Cardinal AdjNoun 'in' Ob {{ return ({ NewOb(tree[4], tree[0], tree[1], tree[2]) }) }}
Ob: Art Ordinal  AdjNoun 'in' Ob {{ return ({ NewOb(tree[4], tree[0], -tree[1], tree[2]) }) }}
Ob: Art          AdjNoun 'in' Ob {{ return ({ NewOb(tree[3], tree[0], 0, tree[1]) }) }}
Ob:     Cardinal AdjNoun 'in' Ob {{ return ({ NewOb(tree[3], 0, tree[0], tree[1]) }) }}
Ob:     Ordinal  AdjNoun 'in' Ob {{ return ({ NewOb(tree[3], 0, -tree[0], tree[1]) }) }}
Ob:              AdjNoun 'in' Ob {{ return ({ NewOb(tree[2], 0, 0, tree[0]) }) }}

Ob: Art Poss Cardinal AdjNoun {{ return ({ NewOb(tree[1], tree[0],  tree[2], tree[3]) }); }}
Ob: Art Poss Ordinal  AdjNoun {{ return ({ NewOb(tree[1], tree[0], -tree[2], tree[3]) }); }}

# experimental
Ob: Art Poss Amount 'of' AdjNoun {{ return ({ NewOb(tree[1], tree[0], tree[2], tree[4]) }); }}

Ob: Art Poss          AdjNoun {{ return ({ NewOb(tree[1], tree[0],  0,       tree[2]) }); }}
Ob:     Poss Cardinal AdjNoun {{ return ({ NewOb(tree[0], 0,        tree[1], tree[2]) }); }}
Ob:     Poss Ordinal  AdjNoun {{ return ({ NewOb(tree[0], 0,       -tree[1], tree[2]) }); }}

# experimental
Ob:     Poss Amount 'of' AdjNoun {{ return ({ NewOb(tree[0], 0,        tree[1], tree[3]) }); }}

Ob:     Poss          AdjNoun {{ return ({ NewOb(tree[0], 0,        0,       tree[1]) }); }}

# experimental 'three torches here' support; Zell 021003

Ob: Cardinal AdjNoun Here {{ return ({ NewOb(tree[2], 0,            tree[0], tree[1]) }); }}
Ob: Ordinal  AdjNoun Here {{ return ({ NewOb(tree[2], 0,           -tree[0], tree[1]) }); }}
Ob:          AdjNoun Here {{ return ({ NewOb(tree[1], 0,            0,       tree[0]) }); }}

# experimental section ends

Ob: Art      Cardinal AdjNoun {{ return ({ NewOb(nil, tree[0],      tree[1], tree[2]) }); }}
Ob: Art      Ordinal  AdjNoun {{ return ({ NewOb(nil, tree[0],     -tree[1], tree[2]) }); }}

# experimental
Ob: Art   Amount 'of' AdjNoun {{ return ({ NewOb(nil, tree[0],      tree[1], tree[3]) }); }}

Ob: Art               AdjNoun {{ return ({ NewOb(nil, tree[0],      0,       tree[1]) }); }}
Ob:          Cardinal AdjNoun {{ return ({ NewOb(nil, 0,            tree[0], tree[1]) }); }}
Ob:          Ordinal  AdjNoun {{ return ({ NewOb(nil, 0,           -tree[0], tree[1]) }); }}

# experimental
Ob:       Amount 'of' AdjNoun {{ return ({ NewOb(nil, 0,            tree[0], tree[2]) }); }}

Ob:                   AdjNoun {{ return ({ NewOb(nil, 0,            0,       tree[0]) }); }}


# Keywords used in the Ob/Obs rules:
_Ob: 'of'
_Ob: 'and'
_Ob: 'myself'
_Ob: 'my'
_Ob: 'self'
_Ob: 'me'
_Ob: 'him'
_Ob: 'her'
_Ob: 'it'
_Ob: 'all'
_Ob: 'here'

Poss: 'my'                   {{ return ({ NewOb(nil,     ART_MINE,  0,       ({ })  ) }); }}
Poss: 'his'                  {{ return ({ NewOb(nil,     ART_HIS,   0,       ({ })  ) }); }}
Poss: 'her'                  {{ return ({ NewOb(nil,     ART_HERS,  0,       ({ })  ) }); }}
Poss: 'its'                  {{ return ({ NewOb(nil,     ART_ITS,   0,       ({ })  ) }); }}
Poss:      Cardinal AdjNounP {{ return ({ NewOb(nil,     0,         tree[0], tree[1]) }); }}
Poss:      Ordinal  AdjNounP {{ return ({ NewOb(nil,     0,        -tree[0], tree[1]) }); }}
Poss: Poss Cardinal AdjNounP {{ return ({ NewOb(tree[0], 0,         tree[1], tree[2]) }); }}
Poss: Poss Ordinal  AdjNounP {{ return ({ NewOb(tree[0], 0,        -tree[1], tree[2]) }); }}
Poss: Poss          AdjNounP {{ return ({ NewOb(tree[0], 0,         0,       tree[1]) }); }}
Poss:               AdjNounP {{ return ({ NewOb(nil,     0,         0,       tree[0]) }); }}




# Keywords used in the Poss rules:
_Poss: 'my'
_Poss: 'his'
_Poss: 'her'
_Poss: 'its'
_Poss: 'here'

AdjNoun: Words

AdjNounP: Words poss	{{ return ({ tree[0] + ({ lower_case(tree[1]) }) }) }}
AdjNounP: poss		{{ return ({ ({ tree[0] }) }) }}
# ---- begin ~TextIF/grammar/obs.y ----
