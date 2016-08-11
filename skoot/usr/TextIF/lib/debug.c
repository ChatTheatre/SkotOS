/*
 *
 */

# include <SkotOS.h>
# include <UserAPI.h>

private inherit "/lib/string";
inherit "/usr/SkotOS/lib/noun";

# include "/usr/SkotOS/include/noun.h"

# define PARSER_DEBUG "/usr/TextIF/sys/parser/debug"

mixed
locate_objects(object user, object body, string arg)
{
    int    i, sz;
    string *elts;
    object current;

    elts = explode(arg, ".");
    switch (elts[0]) {
    case "me":
	current = body;
	break;
    case "here":
	current = body->query_environment();
	break;
    default:
	current = UDATD->query_body(elts[0]);
	if (!current) {
	    current = UDATD->query_body(lower_case(elts[0]));
	}
	/* XXX: Hackety hackety hack! */
	if (!current) {
	    current = find_object("Marrach:players:" +
				  capitalize(elts[0][..0]) + ":" +
				  lower_case(elts[0]));
	}
	if (!current) {
	    /* XXX: Backward compatibility, sheesh. */
	    current = find_object("chatters:" + lower_case(elts[0]) + ":" +
				  lower_case(elts[0]));
	}
	if (!current) {
	    current = find_object(elts[0]);
	}
	if (!current) {
	    mixed elt;
	    NRef elt_nref;

	    /* Fallback, look in inventory or environment. */
	    elt = PARSER_DEBUG->parse(elts[0]);
	    if (!elt) {
		return nil;
	    }
	    elt = elt[0];
	    switch (typeof(elt)) {
	    case T_INT:
		return nil;
	    case T_ARRAY:
		/* Look in inventory first, environment next. */
		elt_nref = locate_one(user, LF_HUSH_ALL | LF_NO_UPDATE,
				      ({ elt }), body, body,
				      body->query_environment());
		if (!elt_nref) {
		    return nil;
		}
		current = NRefOb(elt_nref);
		break;
	    default:
		return nil;
	    }
	}
	break;
    }

    for (i = 1, sz = sizeof(elts); i < sz; i++) {
	object *inv;
	mixed elt;
	NRef elt_nref;

	elt = PARSER_DEBUG->parse(elts[i]);
	if (!elt) {
	    return nil;
	}
	elt = elt[0];
	switch (typeof(elt)) {
	case T_INT:
	    inv = current->query_inventory();
	    if (elt < 1 || elt > sizeof(inv)) {
		return nil;
	    }
	    current = inv[elt - 1];
	    break;
	case T_ARRAY:
	    elt_nref = locate_one(user, LF_HUSH_ALL | LF_NO_UPDATE, ({ elt }),
				  body, current);
	    if (!elt_nref) {
		return nil;
	    }
	    current = NRefOb(elt_nref);
	    break;
	case T_NIL:
	default:
	    return nil;
	}
    }
    return current;
}
