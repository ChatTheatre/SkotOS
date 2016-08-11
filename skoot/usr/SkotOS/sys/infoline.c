/*
 * /usr/SkotOS/sys/infoline.c
 *
 * Redistribute logon/logout/linkdead information to those who want to hear it.
 * Since this 'want' is stored in a body-property, it's character-specific and
 * not per-user.
 *
 * Copyright Skotos Tech Inc 2001-2006.
 */

# define MAX_RECALL 40

# include <type.h>

inherit "~SAM/lib/sam";
inherit "~/lib/describe";

private mapping listeners; /* Permanent mapping of listeners, whether they are
			    * logged in or not. */
private mapping listening; /* Mapping from listeners to relevant user-clones so
			    * that we know who to send the message to.
			    * Updated as the signals come in. */
private mixed **recall;    /* Central memory, what happened recently? */

static void
create()
{
    listeners = ([ ]);
    listening = ([ ]);
    recall    = ({ });
}

void
patch()
{
   recall = ({ });
}

mixed **
query_recall()
{
   return recall[..];
}

void remove_listener(object user, object body);

private void
tell_all_listeners(object user, object body, string text)
{
    catch {
	int    i, sz_i;
	object *bodies;
	mixed  *users;

	text = describe_one(body) + "[" + user->query_name() + "]" + text;
	while (sizeof(recall) > MAX_RECALL) {
	   recall = recall[1..];
	}
	recall += ({ ({ time(), text }) });
	text = ctime(time())[11..18] + " [INFO] " + text + "\n";

	sz_i   = map_sizeof(listening);
	bodies = map_indices(listening);
	users  = map_values(listening);
	for (i = 0; i < sz_i; i++) {
	    int j, sz_j;
	    object *list;

	    if (typeof(users[i]) == T_OBJECT) {
	       list = ({ users[i] });
	    } else {
	       list = map_indices(users[i]);
	    }
	    sz_j = sizeof(list);
	    for (j = 0; j < sz_j; j++) {
	        object user;

		user = list[j];
		if (user && bodies[i]) {
		   /* DEBUG("user = " + dump_value(user) + "; bodies[" + i + "] = " + dump_value(bodies[i])); */
		   if (!user->query_host()) {
		      remove_listener(user, bodies[i]);
		   } else if (user->query_body() == bodies[i]) {
		      /* Good, user hasn't gone and possessed another body */
		      user->message(text);
		   }
		}
	    }
	}
    }
}

void
info_login(object user, object body)
{
    /* Show information to current listeners. */
    tell_all_listeners(user, body, " logs in.");

    /* Check if newcomer is one of the listeners. */
    if (listeners[body]) {
        switch (typeof(listening[body])) {
	case T_NIL:
	   listening[body] = ([ user: TRUE ]);
	   break;
	case T_OBJECT:
	   listening[body] = ([ user: TRUE, listening[body]: TRUE ]);
	   break;
	case T_MAPPING:
	   listening[body][user] = TRUE;
	   break;
	}
    }
}

void
info_linkdie(object user, object body)
{
    /* Remove badly connected person from listeners list if necessary. */
    switch (typeof(listening[body])) {
    case T_NIL:
       break;
    case T_OBJECT:
       listening[body] = nil;
       break;
    case T_MAPPING:
       listening[body][user] = nil;
       break;
    }

    /* Show information to remaining listeners. */
    tell_all_listeners(user, body,
		       " loses " + body->query_possessive() + " link.");
}

void
info_quit(object user, object body)
{
    /* Remove quitter from listeners list if necessary. */
    switch (typeof(listening[body])) {
    case T_NIL:
       break;
    case T_OBJECT:
       listening[body] = nil;
       break;
    case T_MAPPING:
       listening[body][user] = nil;
       break;
    }

    /* Show information to the remaining tough ones. */
    tell_all_listeners(user, body, " logs out.");
}

void
add_listener(object user, object body)
{
    listeners[body] = TRUE;
    listening[body] = ([ user: TRUE ]);
}

void
remove_listener(object user, object body)
{
    listeners[body] = nil;
    listening[body] = nil;
}

int
query_listener(object user, object body)
{
    return !!listeners[body];
}
