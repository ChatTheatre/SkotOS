/*
 * ~UserDB/obj/searchnode.c
 *
 * A node of the search-database, concentrating on one single field, to avoid
 * having one huge object needlessly keeping track of all these bits.
 *
 * Copyright 2001 Skotos Tech Inc.
 */

# include <status.h>
# include <UserDB.h>

inherit "/lib/string";

private string  field;
private mapping data;
private mapping heur;

static void
create(int clone)
{
    if (clone) {
	/* ... */
	field = nil;
	data = ([ ]);
    }
}

void
set_field(string f)
{
    if (previous_program() == SEARCHDB) {
	field = f;
    }
}

private string
prepare_value(string value)
{
    if (value) {
	/* Get rid of whitespace and lowercase the search-value */
	value = lower_case(strip(value));
	if (strlen(value)) {
	    return value;
	}
    }
    return nil;
}

atomic void
update_user(object user, string old_value, string new_value)
{
    old_value = prepare_value(old_value);
    new_value = prepare_value(new_value);

    if (old_value && new_value && old_value == new_value) {
	/* No change! */
	return;
    }
    if (old_value) {
	string pre, rest;

	/* Remove old one. */
	if (strlen(old_value) < 2) {
	    pre = old_value;
	    rest = "";
	} else {
	    pre = old_value[..1];
	    rest = old_value[2..];
	}
	if (data[pre] && data[pre][rest] && data[pre][rest][user]) {
	    data[pre][rest][user] = nil;
	    if (map_sizeof(data[pre][rest]) == 0) {
		data[pre][rest] = nil;
		if (map_sizeof(data[pre]) == 0) {
		    data[pre] = nil;
		}
	    }
	}
    }
    if (new_value) {
	string pre, rest;
	mapping m, n;

	/* Insert new one. */
	if (strlen(new_value) < 2) {
	    pre  = new_value;
	    rest = "";
	} else {
	    pre  = new_value[..1];
	    rest = new_value[2..];
	}
	m = data[pre];
	if (!m) {
	    m = data[pre] = ([ ]);
	}
	n = m[rest];
	if (!n) {
	    n = m[rest] = ([ ]);
	}
	n[user] = TRUE;
    }
}

object *
search(string type, string pattern)
{
    /* DEBUG("searchnode: " + type + " / " + pattern); */
    pattern = prepare_value(pattern);
    switch (type) {
    case "exact": {
	string  pre, rest;
	mapping m, n;

	if (strlen(pattern) < 2) {
	    pre  = pattern;
	    rest = "";
	} else {
	    pre  = pattern[..1];
	    rest = pattern[2..];
	}
	m = data[pre];
	if (m) {
	    n = m[rest];
	    if (n) {
		return map_indices(n);
	    }
	}
	return ({ });
    }
    case "infix": {
	int     i, sz_i;
	string  *pre;
	mapping *map_i;
	mapping results;

	results = ([ ]);

	sz_i  = map_sizeof(data);
	pre   = map_indices(data);
	map_i = map_values(data);

	for (i = 0; i < sz_i; i++) {
	    int j, sz_j;
	    string *rest;
	    mapping *map_j;

	    sz_j  = map_sizeof(map_i[i]);
	    rest  = map_indices(map_i[i]);
	    map_j = map_values(map_i[i]);
	    for (j = 0; j < sz_j; j++) {
		string value;

		value = pre[i] + rest[j];
		if (sscanf(value, "%*s" + pattern + "%*s") == 2) {
		    results += map_j[j];
		}
	    }
	}
	return map_indices(results);
    }
    case "prefix":
    case "suffix":
	/* Not yet implemented. */
	return ({ });
    }
}

void
clear_data()
{
    data = ([ ]);
}

void
clear_heur_data()
{
    heur = ([ ]);
}

void
dump_db_status()
{
    int     i, sz_i, lengths, total;
    string  *pre;
    mapping *map_i;

    DEBUG("*** " + ur_name(this_object()) + ": Field \"" + field + "\"");

    sz_i  = map_sizeof(data);
    pre   = map_indices(data);
    map_i = map_values(data);

    DEBUG("# prefixes: " + sz_i);

    for (i = 0; i < sz_i; i++) {
	int j, sz_j;
	string *rest;
	mapping *map_j;

	total += sz_j;

	sz_j  = map_sizeof(map_i[i]);
	rest  = map_indices(map_i[i]);
	map_j = map_values(map_i[i]);
	for (j = 0; j < sz_j; j++) {
	    string value;

	    value = pre[i] + rest[j];
	    lengths += strlen(value);
	}
    }
    DEBUG("Average #entries per prefix: " + (total / sz_i));
    DEBUG("Average entry length: " + (lengths / total));
    DEBUG("Total #entries: " + total);
    DEBUG("Swap sectors used: " + status(this_object())[O_NSECTORS]);
}
