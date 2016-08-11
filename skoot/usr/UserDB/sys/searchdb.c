/*
 * ~UserDB/sys/searchdb.c
 *
 * A search-database, keep track of a number of fields in user objects, making
 * it easier/faster to search them than to loop over all 50k (and growing) of
 * them.
 *
 * Copyright 2001 Skotos Tech Inc.
 */

string  *fields;
mapping database;

static void
create()
{
    int i, sz;

    fields = ({
	"email",
	"name",
	"street1",
	"street2",
	"city",
	"state",
	"zip"
    });
    database = ([ ]);
    for (i = 0, sz = sizeof(fields); i < sz; i++) {
	database[fields[i]] = clone_object("~UserDB/obj/searchnode");
	database[fields[i]]->set_field(fields[i]);
	DEBUG("searchdb: Cloned " + ur_name(database[fields[i]]) +
	      " for field \"" + fields[i] + "\"");
    }
}

void
update_user(object user, string field, mixed old_value, mixed new_value)
{
    if (database[field]) {
	database[field]->update_user(user, old_value, new_value);
    }
}

object *
search(string field, string type, string pattern)
{
    if (database[field]) {
	if (field == "creditcard") {
	    return database[field]->search("exact", pattern);
	} else {
	    return database[field]->search(type, pattern);
	}
    } else {
	return ({ });
    }
}

void
add_user(object user)
{
    int i, sz;

    for (i = 0, sz = sizeof(fields); i < sz; i++) {
	switch (fields[i]) {
	case "creditcard":
	    if (user->query_card()) {
		update_user(user, fields[i], nil, user->query_card_num());
	    }
	    break;
	default:
	    update_user(user, fields[i], nil, user->query_property(fields[i]));
	    break;
	}
    }
}

void
patch()
{
    int i, sz;

    fields = ({
	"creditcard",
	"email",
	"name",
	"street1",
	"street2",
	"city",
	"state",
	"zip"
    });
    for (i = 0, sz = sizeof(fields); i < sz; i++) {
	if (database[fields[i]]) {
	    continue;
	}
	database[fields[i]] = clone_object("~UserDB/obj/searchnode");
	database[fields[i]]->set_field(fields[i]);
	DEBUG("searchdb: Cloned " + ur_name(database[fields[i]]) +
	      " for field \"" + fields[i] + "\"");
    }
}

void
fill_database()
{
    object master;

    master = find_object("~UserDB/obj/user");
    call_out("delayed_fill_database", 1, 0, master, master->query_next_clone());
}

static void
delayed_fill_database(int total, object master, object current)
{
    int count;

    while (count < 50 && current && current != master) {
	add_user(current);

	count++;
	total++;
	current = current->query_next_clone();
    }
    DEBUG("delayed_fill_database(): " + total + " done.");
    if (current && current != master) {
	call_out("delayed_fill_database", 1, total, master, current);
    } else {
	DEBUG("delayed_fill_database() finished.");
    }
}

void
fill_field(string field)
{
    object master;

    master = find_object("~UserDB/obj/user");
    call_out("delayed_fill_field", 1, 0, field,
	     master, master->query_next_clone());
}

static void
delayed_fill_field(int total, string field, object master, object current)
{
    int count;

    while (count < 50 && current && current != master) {
	switch (field) {
	case "creditcard":
	    if (current->query_card()) {
		update_user(current, field, nil, current->query_card_num());
	    }
	    break;
	default:
	    update_user(current, field, nil, current->query_property(field));
	    break;
	}

	count++;
	total++;
	current = current->query_next_clone();
    }
    DEBUG("delayed_fill_field(): " + total + " done.");
    if (current && current != master) {
	call_out("delayed_fill_field", 1, total, field, master, current);
    } else {
	DEBUG("delayed_fill_field() finished.");
    }
}

void
dump_db_status()
{
    int i, sz;

    for (i = 0, sz = sizeof(fields); i < sz; i++) {
	database[fields[i]]->dump_db_status();
    }
}
