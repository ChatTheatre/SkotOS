# ---- begin ~TextIF/grammar/numbers.y ----

# Delay conversion as long as possible, to allow for reuse of the Card*X* rules:

Cardinal: Card99
Cardinal: Card999
Cardinal: cardinal {{ return ({ cardinal(tree[0]) }) }}

Card99: CardX  {{ return ({ cardinal(tree[0]) }); }}
Card99: Card1X {{ return ({ cardinal(tree[0]) }); }}
Card99: CardX0 {{ return ({ cardinal(tree[0]) }); }}
Card99: CardXX {{ return ({ cardinal(tree[0]) }); }}
Card99: CardX0 '-' CardX {{ return ({ cardinal(tree[0]) + cardinal(tree[2]) }); }}

Card999: CardX Card100 {{ return ({ cardinal(tree[0]) * 100 }); }}
Card999: CardX Card100 'and' Card99 {{ return ({ cardinal(tree[0]) * 100 + tree[3] }); }}

# The same, but without the conversion to an integer value:

Cardinal_Words: Card99_Words
Cardinal_Words: Card999_Words
Cardinal_Words: cardinal

Card99_Words: CardX
Card99_Words: Card1X
Card99_Words: CardX0
Card99_Words: CardXX
Card99_Words: CardX0 '-' CardX

Card999_Words: CardX Card100
Card999_Words: CardX Card100 'and' Card99_Words

CardX:		'one'
CardX:		'two'
CardX:		'three'
CardX:		'four'
CardX:		'five'
CardX:		'six'
CardX:		'seven'
CardX:		'eight'
CardX:		'nine'

Card1X:		'ten'
Card1X:		'eleven'
Card1X:		'twelve'
Card1X:		'thirteen'
Card1X:		'fourteen'
Card1X:		'fifteen'
Card1X:		'sixteen'
Card1X:		'seventeen'
Card1X:		'eighteen'
Card1X:		'nineteen'

CardX0:		'twenty'
CardXX:		'twentyone'
CardXX:		'twenty-one'
CardXX:		'twentytwo'
CardXX:		'twenty-two'
CardXX:		'twentythree'
CardXX:		'twenty-three'
CardXX:		'twentyfour'
CardXX:		'twenty-four'
CardXX:		'twentyfive'
CardXX:		'twenty-five'
CardXX:		'twentysix'
CardXX:		'twenty-six'
CardXX:		'twentyseven'
CardXX:		'twenty-seven'
CardXX:		'twentyeight'
CardXX:		'twenty-eight'
CardXX:		'twentynine'
CardXX:		'twenty-nine'

CardX0:		'thirty'
CardXX:		'thirtyone'
CardXX:		'thirty-one'
CardXX:		'thirtytwo'
CardXX:		'thirty-two'
CardXX:		'thirtythree'
CardXX:		'thirty-three'
CardXX:		'thirtyfour'
CardXX:		'thirty-four'
CardXX:		'thirtyfive'
CardXX:		'thirty-five'
CardXX:		'thirtysix'
CardXX:		'thirty-six'
CardXX:		'thirtyseven'
CardXX:		'thirty-seven'
CardXX:		'thirtyeight'
CardXX:		'thirty-eight'
CardXX:		'thirtynine'
CardXX:		'thirty-nine'

CardX0:		'forty'
CardXX:		'fortyone'
CardXX:		'forty-one'
CardXX:		'fortytwo'
CardXX:		'forty-two'
CardXX:		'fortythree'
CardXX:		'forty-three'
CardXX:		'fortyfour'
CardXX:		'forty-four'
CardXX:		'fortyfive'
CardXX:		'forty-five'
CardXX:		'fortysix'
CardXX:		'forty-six'
CardXX:		'fortyseven'
CardXX:		'forty-seven'
CardXX:		'fortyeight'
CardXX:		'forty-eight'
CardXX:		'fortynine'
CardXX:		'forty-nine'

CardX0:		'fifty'
CardXX:		'fiftyone'
CardXX:		'fifty-one'
CardXX:		'fiftytwo'
CardXX:		'fifty-two'
CardXX:		'fiftythree'
CardXX:		'fifty-three'
CardXX:		'fiftyfour'
CardXX:		'fifty-four'
CardXX:		'fiftyfive'
CardXX:		'fifty-five'
CardXX:		'fiftysix'
CardXX:		'fifty-six'
CardXX:		'fiftyseven'
CardXX:		'fifty-seven'
CardXX:		'fiftyeight'
CardXX:		'fifty-eight'
CardXX:		'fiftynine'
CardXX:		'fifty-nine'

CardX0:		'sixty'
CardXX:		'sixtyone'
CardXX:		'sixty-one'
CardXX:		'sixtytwo'
CardXX:		'sixty-two'
CardXX:		'sixtythree'
CardXX:		'sixty-three'
CardXX:		'sixtyfour'
CardXX:		'sixty-four'
CardXX:		'sixtyfive'
CardXX:		'sixty-five'
CardXX:		'sixtysix'
CardXX:		'sixty-six'
CardXX:		'sixtyseven'
CardXX:		'sixty-seven'
CardXX:		'sixtyeight'
CardXX:		'sixty-eight'
CardXX:		'sixtynine'
CardXX:		'sixty-nine'

CardX0:		'seventy'
CardXX:		'seventyone'
CardXX:		'seventy-one'
CardXX:		'seventytwo'
CardXX:		'seventy-two'
CardXX:		'seventythree'
CardXX:		'seventy-three'
CardXX:		'seventyfour'
CardXX:		'seventy-four'
CardXX:		'seventyfive'
CardXX:		'seventy-five'
CardXX:		'seventysix'
CardXX:		'seventy-six'
CardXX:		'seventyseven'
CardXX:		'seventy-seven'
CardXX:		'seventyeight'
CardXX:		'seventy-eight'
CardXX:		'seventynine'
CardXX:		'seventy-nine'

CardX0:		'eighty'
CardXX:		'eightyone'
CardXX:		'eighty-one'
CardXX:		'eightytwo'
CardXX:		'eighty-two'
CardXX:		'eightythree'
CardXX:		'eighty-three'
CardXX:		'eightyfour'
CardXX:		'eighty-four'
CardXX:		'eightyfive'
CardXX:		'eighty-five'
CardXX:		'eightysix'
CardXX:		'eighty-six'
CardXX:		'eightyseven'
CardXX:		'eighty-seven'
CardXX:		'eightyeight'
CardXX:		'eighty-eight'
CardXX:		'eightynine'
CardXX:		'eighty-nine'

CardX0:		'ninety'
CardXX:		'ninetyone'
CardXX:		'ninety-one'
CardXX:		'ninetytwo'
CardXX:		'ninety-two'
CardXX:		'ninetythree'
CardXX:		'ninety-three'
CardXX:		'ninetyfour'
CardXX:		'ninety-four'
CardXX:		'ninetyfive'
CardXX:		'ninety-five'
CardXX:		'ninetysix'
CardXX:		'ninety-six'
CardXX:		'ninetyseven'
CardXX:		'ninety-seven'
CardXX:		'ninetyeight'
CardXX:		'ninety-eight'
CardXX:		'ninetynine'
CardXX:		'ninety-nine'

Card100:	'hundred'

# ----------------------------------------------------------------------

# Delay conversion as long as possible, to allow for reuse of the Ord*X* rules:
Ordinal: Ord99
Ordinal: Ord999
Ordinal: ordinal {{ return ({ ordinal(tree[0]) }); }}

Ord99: OrdX  {{ return ({ ordinal(tree[0]) }); }}
Ord99: Ord1X {{ return ({ ordinal(tree[0]) }); }}
Ord99: OrdX0 {{ return ({ ordinal(tree[0]) }); }}
Ord99: OrdXX {{ return ({ ordinal(tree[0]) }); }}
Ord99: CardX0 '-' OrdX {{ return ({ cardinal(tree[0]) + ordinal(tree[2]) }); }}

Ord999: CardX Ord100 {{	return ({ cardinal(tree[0]) * 100 }) }}
Ord999: CardX Card100 'and' Ord99 {{ return ({ cardinal(tree[0]) * 100 + tree[3] }); }}

# The same, but without the conversion to an integer value:

Ordinal_Words: Ord99_Words
Ordinal_Words: Ord999_Words
Ordinal_Words: ordinal

Ord99_Words: OrdX
Ord99_Words: Ord1X
Ord99_Words: OrdX0
Ord99_Words: OrdXX
Ord99_Words: CardX0 '-' OrdX

Ord999_Words: CardX Ord100
Ord999_Words: CardX Card100 'and' Ord99_Words

OrdX:		'first'
OrdX:		'second'
OrdX:		'third'
OrdX:		'fourth'
OrdX:		'fifth'
OrdX:		'sixth'
OrdX:		'seventh'
OrdX:		'eighth'
OrdX:		'ninth'

Ord1X:		'tenth'
Ord1X:		'eleventh'
Ord1X:		'twelfth'
Ord1X:		'thirteenth'
Ord1X:		'fourteenth'
Ord1X:		'fifteenth'
Ord1X:		'sixteenth'
Ord1X:		'seventeenth'
Ord1X:		'eighteenth'
Ord1X:		'nineteenth'

OrdX0:		'twentieth'
OrdXX:		'twentyfirst'
OrdXX:		'twenty-first'
OrdXX:		'twentysecond'
OrdXX:		'twenty-second'
OrdXX:		'twentythird'
OrdXX:		'twenty-third'
OrdXX:		'twentyfourth'
OrdXX:		'twenty-fourth'
OrdXX:		'twentyfifth'
OrdXX:		'twenty-fifth'
OrdXX:		'twentysixth'
OrdXX:		'twenty-sixth'
OrdXX:		'twentyseventh'
OrdXX:		'twenty-seventh'
OrdXX:		'twentyeighth'
OrdXX:		'twenty-eighth'
OrdXX:		'twentyninth'
OrdXX:		'twenty-ninth'

OrdX0:		'thirtieth'
OrdXX:		'thirtyfirst'
OrdXX:		'thirty-first'
OrdXX:		'thirtysecond'
OrdXX:		'thirty-second'
OrdXX:		'thirtythird'
OrdXX:		'thirty-third'
OrdXX:		'thirtyfourth'
OrdXX:		'thirty-fourth'
OrdXX:		'thirtyfifth'
OrdXX:		'thirty-fifth'
OrdXX:		'thirtysixth'
OrdXX:		'thirty-sixth'
OrdXX:		'thirtyseventh'
OrdXX:		'thirty-seventh'
OrdXX:		'thirtyeighth'
OrdXX:		'thirty-eighth'
OrdXX:		'thirtyninth'
OrdXX:		'thirty-ninth'

OrdX0:		'fortieth'
OrdXX:		'fortyfirst'
OrdXX:		'forty-first'
OrdXX:		'fortysecond'
OrdXX:		'forty-second'
OrdXX:		'fortythird'
OrdXX:		'forty-third'
OrdXX:		'fortyfourth'
OrdXX:		'forty-fourth'
OrdXX:		'fortyfifth'
OrdXX:		'forty-fifth'
OrdXX:		'fortysixth'
OrdXX:		'forty-sixth'
OrdXX:		'fortyseventh'
OrdXX:		'forty-seventh'
OrdXX:		'fortyeighth'
OrdXX:		'forty-eighth'
OrdXX:		'fortyninth'
OrdXX:		'forty-ninth'

OrdX0:		'fiftieth'
OrdXX:		'fiftyfirst'
OrdXX:		'fifty-first'
OrdXX:		'fiftysecond'
OrdXX:		'fifty-second'
OrdXX:		'fiftythird'
OrdXX:		'fifty-third'
OrdXX:		'fiftyfourth'
OrdXX:		'fifty-fourth'
OrdXX:		'fiftyfifth'
OrdXX:		'fifty-fifth'
OrdXX:		'fiftysixth'
OrdXX:		'fifty-sixth'
OrdXX:		'fiftyseventh'
OrdXX:		'fifty-seventh'
OrdXX:		'fiftyeighth'
OrdXX:		'fifty-eighth'
OrdXX:		'fiftyninth'
OrdXX:		'fifty-ninth'

OrdX0:		'sixtieth'
OrdXX:		'sixtyfirst'
OrdXX:		'sixty-first'
OrdXX:		'sixtysecond'
OrdXX:		'sixty-second'
OrdXX:		'sixtythird'
OrdXX:		'sixty-third'
OrdXX:		'sixtyfourth'
OrdXX:		'sixty-fourth'
OrdXX:		'sixtyfifth'
OrdXX:		'sixty-fifth'
OrdXX:		'sixtysixth'
OrdXX:		'sixty-sixth'
OrdXX:		'sixtyseventh'
OrdXX:		'sixty-seventh'
OrdXX:		'sixtyeighth'
OrdXX:		'sixty-eighth'
OrdXX:		'sixtyninth'
OrdXX:		'sixty-ninth'

OrdX0:		'seventieth'
OrdXX:		'seventyfirst'
OrdXX:		'seventy-first'
OrdXX:		'seventysecond'
OrdXX:		'seventy-second'
OrdXX:		'seventythird'
OrdXX:		'seventy-third'
OrdXX:		'seventyfourth'
OrdXX:		'seventy-fourth'
OrdXX:		'seventyfifth'
OrdXX:		'seventy-fifth'
OrdXX:		'seventysixth'
OrdXX:		'seventy-sixth'
OrdXX:		'seventyseventh'
OrdXX:		'seventy-seventh'
OrdXX:		'seventyeighth'
OrdXX:		'seventy-eighth'
OrdXX:		'seventyninth'
OrdXX:		'seventy-ninth'

OrdX0:		'eightieth'
OrdXX:		'eightyfirst'
OrdXX:		'eighty-first'
OrdXX:		'eightysecond'
OrdXX:		'eighty-second'
OrdXX:		'eightythird'
OrdXX:		'eighty-third'
OrdXX:		'eightyfourth'
OrdXX:		'eighty-fourth'
OrdXX:		'eightyfifth'
OrdXX:		'eighty-fifth'
OrdXX:		'eightysixth'
OrdXX:		'eighty-sixth'
OrdXX:		'eightyseventh'
OrdXX:		'eighty-seventh'
OrdXX:		'eightyeighth'
OrdXX:		'eighty-eighth'
OrdXX:		'eightyninth'
OrdXX:		'eighty-ninth'

OrdX0:		'ninetieth'
OrdXX:		'ninetyfirst'
OrdXX:		'ninety-first'
OrdXX:		'ninetysecond'
OrdXX:		'ninety-second'
OrdXX:		'ninetythird'
OrdXX:		'ninety-third'
OrdXX:		'ninetyfourth'
OrdXX:		'ninety-fourth'
OrdXX:		'ninetyfifth'
OrdXX:		'ninety-fifth'
OrdXX:		'ninetysixth'
OrdXX:		'ninety-sixth'
OrdXX:		'ninetyseventh'
OrdXX:		'ninety-seventh'
OrdXX:		'ninetyeighth'
OrdXX:		'ninety-eighth'
OrdXX:		'ninetyninth'
OrdXX:		'ninety-ninth'

Ord100:	'hundredth'

# ---- end ~TextIF/grammar/numbers.y ----
