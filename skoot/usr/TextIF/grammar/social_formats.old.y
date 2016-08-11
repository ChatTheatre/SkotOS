# /* basic variations: */
# /*   VERB */
# /*   VERB DOB */
# /*   VERB DOB PREP1 IOB1 PREP2 IOB2 ... */
#
# /* Complications: */
# /*   ADV may appear at many places */
# /*   DOB and IOBs all need to be tested for junkiness */
#
# /*   We only need to be careful with junk when possibly near an ADV */


<FORMAT value="'totter' EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>
<FORMAT value="'totter' SocWords EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>
<FORMAT value="'totter' SocWords PWordPhrases EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>
<FORMAT value="'totter' PWordPhrases EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>

# /* wave slowly */
<FORMAT value="%s Adv EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>
# /* wave */
<FORMAT value="%s EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>

# /* first all variations that begin with a preposition (ignoring adverb) */

<FORMAT value="%s PPhrases Adv EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>
<FORMAT value="%s JunkyPPhrases EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>
<FORMAT value="%s Adv JunkyPPhrases EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>


/* then the ones that begin with a direct object */

<FORMAT value="%s SocOb Adv EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>
<FORMAT value="%s SocOb Adv JunkyPPhrases EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>

<FORMAT value="%s Adv SocOb JunkyPPhrases EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>

<FORMAT value="%s JunkySocOb EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>

<FORMAT value="%s JunkySocOb PPhrases Adv EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>
<FORMAT value="%s JunkySocOb PPhrases EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>

<FORMAT value="%s JunkySocOb JunkyPPhrases EvokeP" function="return ({ \"social\" }) + make_social(tree);"/>
