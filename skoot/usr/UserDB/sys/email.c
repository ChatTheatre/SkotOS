/*
 * ~UserDB/sys/email.c
 *
 * Keep track of email addresses of (former) customers and whether or not they
 * still want to receive email from us, or what games they've played.
 *
 * TODO: Try to avoid sending out things to banned accounts.
 * NOTE: The banned status should probably be remembered when an account is purged and we
 *       store the email address with additional data in here.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include <type.h>
# include <UserDB.h>

private inherit "/lib/file";
private inherit "/lib/string";
private inherit "/lib/url";

private inherit "/usr/SMTP/lib/smtp_api";

mapping data;
mapping collection;
mapping exclude_account;
mapping exclude_email;

static void
create()
{
    data = ([ ]);
}

/*
 * Add another email address to the collection, return 0 if this could not be
 * done, for instance because it already existed.  1 if it has been added
 * successfully.
 */
atomic int
add_email(string email, varargs mapping initial)
{
    mapping map;

    if (strlen(email) < 2) {
	/* Too short, can't be right. */
	return FALSE;
    }
    map = data[email[.. 1]];
    if (!map) {
	data[email[..1]] = map = ([ ]);
    } else if (map[email[2 ..]] != nil) {
	/* Already exists. */
	return FALSE;
    }
    if (!initial) {
	initial = ([ ]);
    }
    map[email[2 ..]] = initial[..];
    return TRUE;
}

int
process_purged_data(string line)
{
    int    creation, no_email;
    string addr;

    if (sscanf(line, "\"%*s\" \"%s\"%s", addr, line) == 3) {
	if (sscanf(line, "%*s\"creation_time\":\%d,%*s", creation) < 3) {
	    creation = 0;
	}
	if (sscanf(line, "%*s\"no-email\":\"true\"%*s") == 2) {
	    no_email = TRUE;
	}
	DEBUG("parse_purged_data: " + addr + " / " + no_email + " / " +
	      creation);
	add_email(addr, ([ "no-email": (string)no_email,
			   "creation": creation ]));
	return 1;
    }
    DEBUG("parse_purged_data: Unrecognized line: " + line);
    return 0;
}

# define CHUNK (128 * 1024)
# define PURGED "~/data/purged.data"

void
parse_purged_data()
{
    int    i, sz, size, offset, total;
    string *lines, *chunks;

#if 1
    create();
#endif

    size = file_size(PURGED);
    chunks = ({ });
    while (size > CHUNK) {
	chunks += ({ read_file(PURGED, offset, CHUNK) });
	offset += CHUNK;
	size -= CHUNK;
    }
    chunks += ({ read_file(PURGED, offset, size) });

    while (sizeof(chunks) > 0) {
	string line;

	while (sscanf(chunks[0], "%s\n%s", line, chunks[0]) == 2) {
	    /* Process line. */
	    total += process_purged_data(line);
	}
	if (sizeof(chunks) > 1) {
	    chunks[1] = chunks[0] + chunks[1];
	    chunks = chunks[1..];
	} else {
	    /* Deal with last line. */
	    total += process_purged_data(chunks[0]);
	    chunks = ({ });
	}
    }
    DEBUG("Total of " + total + " addresses found?");
}

/*
 * Check whether an email address is in our list at all.
 */
int
query_email(string email)
{
    mapping map;

    if (strlen(email) < 2) {
	return FALSE;
    }
    map = data[email[.. 1]];
    return map && map[email[2 ..]];
}

mixed
query_email_field(string email, string field)
{
    mapping map;

    if (strlen(email) < 2) {
	return nil;
    }
    map = data[email[.. 1]];
    return (map && map[email[2 ..]]) ? map[email[2 ..]][field] : nil;
}

/*
 * Return a list of lists of email addresses with the given field value.
 */
string **
query_list(string field, mixed value)
{
    int i, sz_i;
    string *pre, **lists;
    mapping *maps;

    sz_i = map_sizeof(data);
    pre = map_indices(data);
    maps = map_values(data);
    lists = allocate(sz_i);
    for (i = 0; i < sz_i; i++) {
	int j, sz_j;
	string *list;
	mapping *flags;

	sz_j = map_sizeof(maps[i]);
	list = map_indices(maps[i]);
	flags = map_values(maps[i]);
	lists[i] = allocate(sz_j);
	for (j = 0; j < sz_j; j++) {
	    if (value == nil || flags[j][field] == value) {
		lists[i][j] = pre[i] + list[j];
	    }
	}
	lists[i] -= ({ nil });
	if (!sizeof(lists[i])) {
	    lists[i] = nil;
	}
    }
    return lists - ({ nil });
}

/*
 * Change the field value for the email address, return 0 if this could not be
 * done (for instance because it was not in our list), 1 if the change was
 * made successfully.
 */
atomic int
update_email(string email, string field, mixed value)
{
    mapping map;

    if (strlen(email) < 2) {
	return FALSE;
    }
    map = data[email[.. 1]];
    if (!map || !map[email[2 ..]]) {
	return FALSE;
    }
    map[email[2 ..]][field] = value;
    return TRUE;
}

/*
 * Remove the email address from our list, return 0 if the address wasn't in
 * our list to begin with, 1 if it has been removed successfully.
 */
atomic int
remove_email(string email)
{
    mapping map;

    if (strlen(email) < 2) {
	return FALSE;
    }
    map = data[email[.. 1]];
    if (!map || map[email[2 ..]] == nil) {
	return FALSE;
    }
    map[email[2 ..]] = nil;
    if (map_sizeof(data[email[.. 1]]) == 0) {
	data[email[.. 1]] = nil;
    }
    return TRUE;
}

void
collect_email(string *games, int comp, int current, int trial)
{
    int     i, sz;
    string  **nodes_2, *pre;
    object  **nodes_1;
    mapping *bits;

    /* Prepare nodes_1 */
    bits = map_values(USERDB->query_users());
    sz = sizeof(bits);
    nodes_1 = allocate(sz);
    for (i = 0; i < sz; i++) {
	nodes_1[i] = map_values(bits[i]);
    }

    sz = map_sizeof(data);
    pre = map_indices(data);
    bits = map_values(data);
    nodes_2 = allocate(sz);
    for (i = 0; i < sz; i++) {
	int    j, sz_j;
	string *list;

	sz_j = map_sizeof(bits[i]);
	list = map_indices(bits[i]);
	nodes_2[i] = allocate(sz_j);
	for (j = 0; j < sz_j; j++) {
	    nodes_2[i][j] = pre[i] + list[j];
	}
    }

#if 1
    call_out("delayed_collect_email_1", 0, nodes_1, nodes_2, games, comp,
	     current, trial, ([ ]), 0, 0);
#else
    call_out("delayed_collect_email_2", 0, nodes_2, games, comp, current,
	     trial, ([ ]), 0, 0);
#endif
}

static int
merge_collection(string email, int creation, int no_email, int deleted,
		 int trial, int gone, string *played, string account,
		 mapping collected)
{
    mapping map, data;

    data = ([ 
      "email":    email,
      "no-email": no_email,
      "deleted":  deleted,
      "trial":    trial,
      "gone":     gone,
      "played":   played,
      "account":  account,
      "creation": creation
    ]);
    map = collected[email[.. 1]];
    if (map) {
	mapping old_data;

	if (old_data = map[email[2 ..]]) {
	    string replace;

	    /*
	     * Replace-logic: Pick whichever one of the two is not deleted,
	     * or not 'gone', or not trial, or was created more recently.
	     */
	    replace = nil;
	    if (old_data["deleted"] == data["deleted"]) {
		if (old_data["gone"] == data["gone"]) {
		    if (data["trial"] == data["trial"]) {
			if (old_data["creation"] < data["creation"]) {
			    replace = "Creation time";
			}
		    } else if (old_data["trial"]) {
			replace = "Trial account";
		    }
		} else if (old_data["gone"]) {
		    replace = "Existing account";
		}
	    } else if (old_data["deleted"]) {
		replace = "Deleted account";
	    }
	    if (replace) {
#if 0
		DEBUG("merge_collection: " + email +
		      " - Keeping new data (based on: " + replace + ").\n" +
		      "Old: " + dump_value(map[email[2 ..]]) + "\n" +
		      "New:  " + dump_value(data));
#endif
		map[email[2 ..]] = data;
	    } else {
#if 0
		DEBUG("merge_collection: " + email +
		      " - Keeping old data.\n" +
		      "Old: " + dump_value(map[email[2 ..]]) + "\n" +
		      "New:  " + dump_value(data));
#endif
	    }
	    return 0;
	}
    } else {
	collected[email[.. 1]] = map = ([ ]);
    }
    map[email[2 ..]] = data;
    return 1;
}

static int
collect_node_data(object node, string *games, mapping collected)
{
    int    i, sz, creation, no_email, deleted, trial, gone;
    string *played, email;

    email = node->query_email();
    if (!email || strlen(email) < 2 || email == "[unknown]" ||
	sscanf(email, "%*s@%*s.%*s") < 3) {
	/* No valid email, what can we do, hm? */
	return 0;
    }
    creation = node->query_property("creation_time") ?
	       node->query_property("creation_time") : 0;
    no_email = !!node->query_property("no-email");
    if (node->query_account_status(AS_DELETED)) {
	deleted = TRUE;
	trial   = FALSE;
	gone    = TRUE;
    } else {
	deleted = FALSE;
	if (node->query_next_stamp() >= time() || node->query_card()) {
	    gone  = FALSE;
	    trial = node->query_account_type() == AS_TRIAL;
	} else {
	    gone  = TRUE;
	    trial = FALSE;
	}
    }

    if (games) {
	sz = sizeof(games);
	played = allocate(sz);
	for (i = 0; i < sz; i++) {
	    mixed total;

	    total = node->query_game_stats(games[i], "played-total");
	    if (typeof(total) == T_INT && total > 0) {
		played[i] = games[i];
	    }
	}
	played -= ({ nil });
    }

    return merge_collection(email, creation, no_email, deleted, trial, gone,
			    played, node->query_name(), collected);
}

static int
collect_email_data(string email, string *games, mapping collected)
{
    int    i, sz, creation, no_email, deleted, trial, gone;
    string *played;

    if (!email || strlen(email) < 2 || email == "[unknown]" ||
	sscanf(email, "%*s@%*s.%*s") < 3) {
	/* No valid email, what can we do, hm? */
	DEBUG("collect_email_data: Invalid email address? " + dump_value(email));
	return 0;
    }
    creation = query_email_field(email, "creation") ?
	       query_email_field(email, "creation") : 0;
    no_email = query_email_field(email, "no-email") == "1";

    deleted = TRUE;
    trial   = FALSE;
    gone    = TRUE;

    if (games) {
	sz = sizeof(games);
	played = allocate(sz);
	for (i = 0; i < sz; i++) {
	    if (query_email_field(email, "game_" + games[i])) {
		played[i] = games[i];
	    }
	}
	played -= ({ nil });
    }

    email = strip(lower_case(email));
    return merge_collection(email, creation, no_email, deleted, trial, gone,
			    played, nil, collected);
}

static void
delayed_collect_email_1(object **nodes_1, string **nodes_2, string *games,
			int comp, int current, int trial, mapping collected,
			int done, int found)
{
    object node;

    while (sizeof(nodes_1) > 0 && sizeof(nodes_1[0]) == 0) {
	nodes_1 = nodes_1[1 ..];
    }
    if (sizeof(nodes_1) == 0) {
	call_out("delayed_collect_email_2", 0, nodes_2, games, comp, current,
		 trial, collected, done, found);
	return;
    }

    node = nodes_1[0][0];
    nodes_1[0] = nodes_1[0][1..];

    found += collect_node_data(node, games, collected);
    done++;

    if (done % 1000 == 0) {
	DEBUG("delayed_collect_email_1: " + done + " done so far, " + found + " unique email addresses.");
    }
    call_out("delayed_collect_email_1", 0, nodes_1, nodes_2, games, comp,
	     current, trial, collected, done, found);
}

# define DUMP "~/data/collection.dump"

static int
match_entry(mapping data, string *games, int comp, int current, int trial)
{
    if (data["no-email"]) {
	return 0;
    }
    if (games && sizeof(games)) {
	int found;

	found = data["played"] && sizeof(data["played"]);
	if (found == comp) {
	    /*
	     * Meaning: Either the account didn't play any of the accounts we
	     *          are looking for (!found && !comp) or the account did
	     *          play one or more of the games we do not want to notify
	     *          (found && comp) because we already sent that group a
	     *          (different) message, for instance.
	     */
	    return 0;
	}
    }
    if (current) {
	if (data["gone"]) {
	    return 0;
	}
	if (trial) {
	    if (!data["trial"]) {
		return 0;
	    }
	}
    } else {
	if (!data["gone"]) {
	    return 0;
	}
    }
    return 1;
}

static void
dump_collection(mapping collected, string *games, int comp, int current,
		int trial)
{
    int i, sz, deleted, purged, no_email;
    string *pre;
    mapping *maps;
    int matches;

    collection = ([ ]);

    remove_file(DUMP);
    sz = map_sizeof(collected);
    pre = map_indices(collected);
    maps = map_values(collected);
    for (i = 0; i < sz; i++) {
	int j, sz_j;
	string *rest;
	mapping *data;

	sz_j = map_sizeof(maps[i]);
	rest = map_indices(maps[i]);
	data = map_values(maps[i]);
	for (j = 0; j < sz_j; j++) {
	    if (!data[j]["account"]) {
		purged++;
	    } else if (data[j]["deleted"]) {
		data[j]["account"] = nil;
		deleted++;
	    }
	    if (data[j]["no-email"] == 1) {
		no_email++;
	    }
	    if (match_entry(data[j], games, comp, current, trial)) {
		mapping map;

		map = collection[pre[i]];
		if (!map) {
		    collection[pre[i]] = map = ([ ]);
		}
		collection[pre[i]][rest[j]] = data[j];
		write_file(DUMP, dump_value(data[j]) + "\n");
		matches++;
	    }
	}
    }
    DEBUG("dump_collection: " + purged + " purged and " + deleted + " deleted accounts, " + no_email + " don't want email, " + matches + " matches found.");
}

static void
delayed_collect_email_2(string **nodes_2, string *games, int comp, int current,
			int trial, mapping collected, int done, int found)
{
    string node;

    while (sizeof(nodes_2) > 0 && sizeof(nodes_2[0]) == 0) {
	nodes_2 = nodes_2[1 ..];
    }
    if (sizeof(nodes_2) == 0) {
	/* call_out("select_email", 0, collected, done, found); */
	dump_collection(collected, games, comp, current, trial);
	DEBUG("delayed_collect_email_2: DONE! " + done + " in total, " + found + " unique email addresses.");
	return;
    }
    node = nodes_2[0][0];
    nodes_2[0] = nodes_2[0][1..];

    found += collect_email_data(node, games, collected);
    done++;

    if (done % 1000 == 0) {
	DEBUG("delayed_collect_email_2: " + done + " done so far, " + found + " unique email addresses.");
    }
    call_out("delayed_collect_email_2", 0, nodes_2, games, comp, current,
	     trial, collected, done, found);
}

void
test_collection()
{
    collection = ([ "ha": ([ "rte@skotos.net": ([
	 "email":   "harte@skotos.net",
	 "account": "nino"
    ]) ]) ]);
}

mapping
query_collection()
{
    return collection;
}

void
purge_exclude_lists()
{
    exclude_account = ([ ]);
    exclude_email = ([ ]);
}

mapping
query_exclude_account()
{
    return exclude_account;
}

mapping
query_exclude_email()
{
    return exclude_email;
}

static void
process_account_template(mapping data, string env, string subject,
			 string template)
{
    int    code;
    string email, account, check;

    email   = data["email"];
    account = data["account"];
    code    = random(900000) + 100000;
    check   = to_hex(hash_md5(code + ":dommelsch:" + email));

    DEBUG("process_account_template:" + email + " / " + account);

    template = implode(explode(template, "[email]"),       email);
    template = implode(explode(template, "[email_enc]"),   url_encode(email));
    template = implode(explode(template, "[account]"),     account);
    template = implode(explode(template, "[account_enc]"), url_encode(account));
    template = implode(explode(template, "[code]"),        (string)code);
    template = implode(explode(template, "[code_enc]"),    url_encode((string)code));
    template = implode(explode(template, "[check]"),       check);
    template = implode(explode(template, "[check_enc]"),   url_encode(check));

    send_message(env,
		 "Skotos Customer Experience Team", "ce@skotos.net",
#if 0
		 ([ "harte@skotos.net": email ]), 
#else
		 ([ email: "Skotos Member" ]),
#endif
		 ([ ]),
		 subject,
		 nil,
		 template);
}

static void
process_email_template(mapping data, string env, string subject,
		       string template)
{
    int    code;
    string check, email;

    email = data["email"];
    code  = random(900000) + 100000;
    check = to_hex(hash_md5(code + ":dommelsch:" + email));

    DEBUG("process_email_template:" + email + " / " + code + " / " + check);

    template = implode(explode(template, "[email]"),     email);
    template = implode(explode(template, "[email_enc]"), url_encode(email));
    template = implode(explode(template, "[code]"),      (string)code);
    template = implode(explode(template, "[code_enc]"),  url_encode((string)code));
    template = implode(explode(template, "[check]"),     check);
    template = implode(explode(template, "[check_enc]"), url_encode(check));

    send_message(env,
		 "Skotos Customer Experience Team", "ce@skotos.net",
#if 0
		 ([ "harte@skotos.net": email ]), 
#else
		 ([ email: "Former Skotos Member" ]),
#endif
		 ([ ]),
		 subject,
		 nil,
		 template);

}

/*
 * Use the collection created earlier and send messages out to the addresses,
 * using the account_file template for people that have an existing account
 * (not purged and not marked as deleted) and email_file for the others.
 *
 * For account_file the following things will be replaced in the template:
 *
 *   [email]
 *   [email_enc]
 *   [account]
 *   [account_enc]
 *
 * For email_file the following things will be replaced in the template:
 *
 *   [email]
 *   [email_enc]
 *   [code]
 *   [code_enc]
 *   [check]
 *   [check_enc]
 */
void
email_run(string env, string subject, string account_file, string email_file,
	  int offer_trial)
{
    int     i, sz;
    string  account_tmpl, email_tmpl;
    mapping *maps, **data;

    if (!exclude_email) {
	exclude_email = ([ ]);
    }
    if (!exclude_account) {
	exclude_account = ([ ]);
    }

    if (!env) {
	env = "email@bounce.skotos.net";
    }
    if (!subject) {
#if 1
	subject = "[Skotos] Free Trial for New Space Conquest Game";
#else
	subject = "[Skotos] Free Trial for Two New Games";
#endif
    }

    account_tmpl = read_file(account_file);
    email_tmpl = read_file(email_file);

    sz = map_sizeof(collection);
    maps = map_values(collection);
    data = allocate(sz);
    for (i = 0; i < sz; i++) {
	data[i] = map_values(maps[i]);
    }

    call_out("delayed_email_run", 1, data, env, subject, account_tmpl,
	     email_tmpl, offer_trial, 0, 0, 0);
}

int
in_exclude_lists(mapping node)
{
    string account, email;

    account = node["account"];
    if (account) {
	mapping map;

	if ((map = exclude_account[account[.. 1]]) && map[account[2 ..]]) {
	    return TRUE;
	}
    }
    email = node["email"];
    if (email) {
	mapping map;

	if ((map = exclude_email[email[.. 1]]) && map[email[2 ..]]) {
	    return TRUE;
	}
    }
    return FALSE;
}

void
add_exclude_lists(mapping node)
{
    string account, email;

    account = node["account"];
    if (account) {
	mapping map;

	map = exclude_account[account[.. 1]];
	if (!map) {
	    exclude_account[account[.. 1]] = map = ([ ]);
	}
	map[account[2 ..]] = TRUE;
    }
    email = node["email"];
    if (email) {
	mapping map;

	map = exclude_email[email[.. 1]];
	if (!map) {
	    exclude_email[email[.. 1]] = map = ([ ]);
	}
	map[email[2 ..]] = TRUE;
    }
}

static void
delayed_email_run(mapping **data, string env, string subject,
		  string account_tmpl, string email_tmpl, int offer_trial,
		  int done_account, int done_email, int skipped)
{
    string account;
    mapping node;

    while (sizeof(data) > 0 && sizeof(data[0]) == 0) {
	data = data[1 ..];
    }
    if (sizeof(data) == 0) {
	DEBUG("delayed_email_run: DONE!  " + done_account + " accounts, " +
	      done_email + " email addresses, " + skipped + " skipped.");
	return;
    }
    node = data[0][0];
    data[0] = data[0][1..];

    if (in_exclude_lists(node)) {
	skipped++;
    } else {
	if (account = node["account"]) {
	    if (offer_trial) {
		USERDB->query_user(account)->set_property("trial_offer", time());
	    }
	    process_account_template(node, env, subject, account_tmpl);
	    done_account++;
	} else {
	    process_email_template(node, env, subject, email_tmpl);
	    done_email++;
	}
	add_exclude_lists(node);
    }

#if 0
    if (skipped + done_account + done_email % 100 == 0) {
#endif
	DEBUG("delayed_email_run: " + done_account + " accounts, " +
	      done_email + " email addresses, " + skipped + " skipped.");
#if 0
    }
#endif
    call_out("delayed_email_run", 4, data, env, subject, account_tmpl,
	     email_tmpl, offer_trial, done_account, done_email, skipped);
}

private int
add_active_user(object node, mapping data)
{
    string  email;
    mapping map;

    if (node->query_property("no-email")) {
	DEBUG("add_active_user: no-email for " + node->query_name());
	return FALSE;
    }
    email =node->query_email();
    if (!email || strlen(email) < 2 || lower_case(email) == "[unknown]") {
	DEBUG("add_active_user: no valid email for " + node->query_name());
	return FALSE;
    }
    email = lower_case(email);
    map = data[email[..1]];
    if (!map) {
	data[email[.. 1]] = map = ([ ]);
    } else if (map[email[2..]]) {
	DEBUG("add_active_user: duplicate email (" + email + ") for " +
	      node->query_name() + " and " + map[email[2..]]->query_name());
	return FALSE;
    }
    map[email[2..]] = node;
    return TRUE;
}

private int
add_active_users(mapping nodes, mapping email)
{
    int     i, sz_i, total;
    mapping *data;

    sz_i = map_sizeof(nodes);
    data = map_values(nodes);
    for (i = 0; i < sz_i; i++) {
	int    j, sz_j;
	object *users;

	sz_j  = map_sizeof(data[i]);
	users = map_values(data[i]);
	for (j = 0; j < sz_j; j++) {
	    if (add_active_user(users[j], email)) {
		total++;
	    }
	}
    }
    return total;
}

void
mailout_active()
{
    /*
     * Combine cc, other, free and developer users, so just not the 'staff'
     * accounts.  That should be the collection of all 'active' users.
     */
    int     i, sz_i, total;
    string  template;
    mapping email, *data;

    email = ([ ]);
    total = 0;

    total += add_active_users(BILLINGD->query_cc_users(),        email);
    total += add_active_users(BILLINGD->query_other_users(),     email);
    total += add_active_users(BILLINGD->query_free_users(),      email);
    total += add_active_users(BILLINGD->query_developer_users(), email);

    DEBUG("mailout_active: Collected " + total + " addresses.");

#if 0
    email = ([ "ha": ([ "rte@skotos.net": find_object("UserDB:n:ni:nino") ]) ]);
#endif

    template = read_file("/usr/UserDB/data/templates/20020804-all.tmpl");
    sz_i = map_sizeof(email);
    data = map_values(email);
    for (i = 0; i < sz_i; i++) {
	int j, sz_j;
	object *nodes;

	sz_j  = map_sizeof(data[i]);
	nodes = map_values(data[i]);
	for (j = 0; j < sz_j; j++) {
	    send_message("ce@skotos.net",
			 "Skotos Customer Experience Team", "ce@skotos.net",
			 ([ nodes[j]->query_email(): "Skotos Member" ]),
			 ([ ]),
			 "An Open Letter to Skotos Players",
			 nil,
			 template);
	}
    }
    DEBUG("mailout_active: Scheduled the lot of 'em.");
}
