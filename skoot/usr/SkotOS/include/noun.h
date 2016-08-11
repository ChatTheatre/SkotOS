# include <nref.h>

/* Constructing and interpreting locate() results */
# define LOC_PAIR(type,data) ({ type, data })
# define LOC_RESULT      0
# define LOC_DATA        1

# define LOC_ONE         0
# define LOC_MULTI       1
# define LOC_ERROR       2
# define LOC_ERROR_MULTI 3
# define LOC_ERROR_POSS  4
# define LOC_ERROR_GONE  5

/* Pick the right HUSH_* value for the error */
# define LOC_HUSH(err) (1 << ((err) - LOC_ERROR))

/*
 * Possible values for the 'int flags' parameters of the locate_* and
 * search_* functions, to be or'd when combined.
 *
 * If more LF_HUSH_* flags are added, try to keep it in the first 8 bits so
 * that LOC_HUSH() can be kept fairly simple still.
 */

# define LF_HUSH_FAIL     1  /* Things not being found or understood     */
# define LF_HUSH_MANY     2  /* Multiple objects found, pick one damnit  */
# define LF_HUSH_POSS     4  /* Something going wrong on a lower level   */
# define LF_HUSH_GONE     8  /* Understand what you're saying, but gone  */
# define LF_NO_UPDATE   256  /* Don't update the 'him/her/it' references */

# define LF_HUSH_ALL      (LF_HUSH_FAIL|LF_HUSH_MANY|LF_HUSH_POSS|LF_HUSH_GONE)

/*
 * Backward compatibility, gotta love it.
 */
# define HUSH_FAIL	  LF_HUSH_FAIL
# define HUSH_MANY	  LF_HUSH_MANY
# define HUSH_POSS        LF_HUSH_POSS
# define HUSH_ALL         LF_HUSH_ALL

# define Ob		mixed *

# define NewOb(poss, art, oc, words) \
    (({ poss, art, oc }) + words)

# define ObPoss(n)	((n)[0])
# define ObArt(n)	((n)[1])
# define ObOC(n)	((n)[2])
# define ObWords(n)	((n)[3..])


static mixed  locate_one(object user, int flags, Ob *ref, object looker,
			object loc...);
static NRef *locate_many(object user, int flags, Ob *refs, object looker,
			 object loc...);
static mixed  locate_first(object user, int flags, Ob *refs, object filter,
			  object looker, object loc...);
static NRef *locate_best(object user, int flags, Ob *refs, object filter,
			 object looker, object loc...);

static mixed  search_one(object user, int flags, Ob *ref, object looker,
			mixed among...);
static NRef *search_many(object user, int flags, Ob *refs, object looker,
			 mixed among...);
static mixed  search_first(object user, int flags, Ob *refs, object filter,
			  object looker, mixed loc...);
static NRef *search_best(object user, int flags, Ob *refs, object filter,
			 object looker, mixed loc...);

static void locate_update(object user, NRef *res);
