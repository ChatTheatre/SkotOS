PrepWords:	'above'
PrepWords:	'about'
PrepWords:	'across' 'from'
PrepWords:	'against'
PrepWords:      'along'
PrepWords:	'around'
PrepWords:	'at'
PrepWords:	'atop'

PrepWords:	'before'
PrepWords:	'behind'
PrepWords:	'beneath'
PrepWords:	'below'
PrepWords:	'beside'
PrepWords:      'between'
PrepWords:	'by'

PrepWords:	'close' 'beside'
PrepWords:	'close' 'by'
PrepWords:	'close' 'to'
PrepWords:	'concerning'

PrepWords:	'down'

PrepWords:	'for'
PrepWords:	'from'

PrepWords:	'in'
PrepWords:	'in' 'front' 'of'
PrepWords:	'inside'
PrepWords:	'into'

PrepWords:	'near'
PrepWords:	'nearby'
PrepWords:	'near' 'to'
PrepWords:	'next' 'to'

PrepWords:	'of'

PrepWords:	'on'
PrepWords:	'on' 'top' 'of'
PrepWords:	'onto'
PrepWords:	'over'

PrepWords:	'regarding'

PrepWords:	'to'
PrepWords:	'toward'
PrepWords:	'towards'
PrepWords:	'through'

PrepWords:	'under'
PrepWords:	'upon'
PrepWords:	'up'

PrepWords:	'using'
PrepWords:	'with'
PrepWords:	'within'

Prep:  PrepWords {{ return ({ implode(tree, " ") }); }}
SocPrep: Prep      {{ return ({ ([ "prep": tree[0] ]) }); }}

_PrepWords:     'front'
_PrepWords:     'top'
