# include <SAM.h>

# define P(s)		(s)
# define IP(h, s)	(h + s)	/* indented header; trickier later */

static void tell_to(object *users, string msg, object bodies...);
static void tell_each( object *users, object who, string msg, object body);
static void tell_all_but(object *users, string msg, object except...);
static void tell(object *users, string first, string third, object bodies...);

static void general_sam(object *users, object actor, object *lookers,
			SAM what, varargs mapping args, int public);






