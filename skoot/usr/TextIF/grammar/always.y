# ---- begin ~TextIF/grammar/always.y ----

whitespace = /[ \t]+/

poss       = /[a-zA-Z_][a-zA-Z0-9_]*\'/
poss       = /[a-zA-Z_][a-zA-Z0-9_]*\'s/

cardinal   = /[0-9]+/

ordinal    = /[1-9][0-9]*0th/
ordinal    = /([1-9][0-9]*)?1[1-3]th/
ordinal    = /([1-9][0-9]*)?1st/
ordinal    = /([1-9][0-9]*)?2nd/
ordinal    = /([1-9][0-9]*)?3rd/
ordinal    = /([1-9][0-9]*)?[4-9]th/

word       = /[a-zA-Z0-9_][a-zA-Z0-9-_:]*/
remoteid   = /[a-zA-Z0-9_][a-zA-Z0-9_-]+@[a-zA-Z][a-zA-Z0-9:]+/

evoke      = /\".*/
evoke      = /\'.*/

other      = /[^\"a-zA-Z \t]+/

# ---- end ~TextIF/grammar/always.y ----
