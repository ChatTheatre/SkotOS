/*
 *	/lib/dp_registry.c
 *
 *	Maintain a registry of derived property root namespaces and
 *	which handlers implement them.
 *
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <type.h>

private inherit "/lib/string";

private mapping root_handlers;

void    register_root_handler(string root, object handler);
mixed  *query_root_handler(string prop);
mapping query_root_handler_map();

static
void create() {
    root_handlers = ([ ]);
}

void
register_root_handler(string root, object handler) {
    root_handlers[lower_case(root)] = handler;
}

mixed *query_root_handler(string prop) {
    string root, rest;

    root = prop;
    rest = nil;
    while (TRUE) {
	int    i, len;
	object handler;

	if (handler = root_handlers[root]) {
	    return ({ handler, root, rest ? rest : "" });
	}
	for (i = strlen(root) - 1; i >= 0; i--) {
	    if (root[i] == ':') {
		break;
	    }
	}
	if (i == -1) {
	    return nil;
	}
	if (rest) {
	    rest = ":" + rest;
	} else {
	    rest = "";
	}
	rest = root[i + 1..] + rest;
	root = root[..i - 1];
    }
    /* Should not reach here. */
    return nil;
}

mapping query_root_handler_map() {
    return root_handlers[..];
}
