# include <config.h>
# include "nref.h"

# define UDat()			(context->query_udat())

# define SKOTDIR		(USR + "/SkotOS")

# define STARTUP_DIR		(USR + "/SkotOS/data/startup")

# define SKOT_INITD		(SKOTDIR + "/initd")

# define SOCIALS		(SKOTDIR + "/sys/socials")
# define VERB			(SKOTDIR + "/obj/verb")
# define ADVCAT			(SKOTDIR + "/obj/advcat")

# define LOGD			(SKOTDIR + "/sys/logd")
# define LOG_NODE		(SKOTDIR + "/obj/lognode")

# define LANG			(SKOTDIR + "/sys/lang")
# define TIP			(SKOTDIR + "/sys/tip")
# define CMEM			(SKOTDIR + "/sys/cmemories")
# define NPCBRAINS		(SKOTDIR + "/sys/npc-brains")
# define NPCBODIES		(SKOTDIR + "/sys/npc-bodies")
# define ASSISTD		(SKOTDIR + "/sys/assistd")
# define STARTSTORY		(SKOTDIR + "/sys/startstory")
# define BILBO			(SKOTDIR + "/sys/bilbo")
# define SYS_MERRY		(SKOTDIR + "/sys/merry")
# define SAMDESC		(SKOTDIR + "/sys/samdesc")
# define EDD			(SKOTDIR + "/sys/edd")

# define HELP_NODE		(SKOTDIR + "/obj/help")
# define HELPD			(SKOTDIR + "/sys/helpd")

# define TIP_NODE               (SKOTDIR + "/obj/tip")
# define TIPD                   (SKOTDIR + "/sys/tipd")

# define INFOLINE		(SKOTDIR + "/sys/infoline")
# define MPROFILER              (SKOTDIR + "/sys/profiler")

/* these are DTD definitions -- really should be renamed */

# define THING_PREP		"thing_prep"
# define THING_DESC_TYPE	"thing_desc_type"
# define THING_FACE		"thing_face"
# define THING_GENDER		"thing_gender"

# define THING_STANCE		"thing_stance"

# define LOG_TAG		"log_tag"

# define NOTE_STATUS		"note_status"

# define SOCIAL_APPROACH	"social_approach"
# define SOCIAL_REQUIREMENT	"social_requirement"
# define SOCIAL_ROLE		"social_role"

# define MERRY			"merry"

# define BITMAP			"bitmap"

# define CA_COND		"ca_cond"


/* these two kind of conflict */
# define STYLE_NONPOSS		1	/* don't do the 'his frog' thing */
# define STYLE_DEFINITE		2	/* the frog */

/* bit 2 */
# define STYLE_REL_LOC		4	/* the frog in his bag */

/* bit 3 */
# define STYLE_FIRST_PERSON	8	/* the frog in your bag */

# define STYLE_REDUCE_CHUNK	16	/* avoid look frogs->four frogs */
# define STYLE_PRONOUN		32	/* allow i.e. 'yourself' vs 'you' */

# define STYLE_NOCLUSTER	64
# define STYLE_NO_SHOW_INVIS	128	/* dont show unseen objects */

# define STYLE_WEAK_LOOKER	256	/* don't replace Zell with You */

# define STYLE_PLURALIZE	512	/* actually build the plural version */

# define STYLE_IGNORE_VISIBILITY	1024

# define STYLE_COMMAND		2048	/* 'my first frog' instead of 'your frog' */

# define VERB_FORBIDDEN	1
# define VERB_OPTIONAL	2
# define VERB_REQUIRED	3

# define TAG(s, t)		("\000\003" + (t) + "\001" + (s) + "\002")
# define LINK(s, u, t)		("\000\004" + (t) + "|" + (u) + "\001" + (s) + "\002")
# define INDENT(h, s)		("\000\005" + (h) + "\001" + (s) + "\002")

# define INLANG(l, p, s)	("\000\206" + (l) + "|" + (p) + "\001" + (s) + "\002")
# define PRE(s)			("\000\007\001" + (s) + "\002")
# define PROP(p, t, e)		("\000\210" + (p) + "|" + (t) + "\001" + (e) + "\002")

# define ADVPOS_PRE_SUBJECT	"ps"
# define ADVPOS_PRE_VERB	"pv"
# define ADVPOS_PRE_ROLE	"pr"
# define ADVPOS_PRE_PREP	"pp"
# define ADVPOS_PRE_EVOKE	"pe"
