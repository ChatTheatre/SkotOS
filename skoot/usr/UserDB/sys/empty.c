/*
 * ~UserDB/sys/empty.c
 *
 * Copyright Skotos Tech Inc 2002
 */

# include <SMTP.h>
# include <UserDB.h>

# define PURGE_EXPIRE_AGE (365 + 182)
# define PURGE_NO_PLAY    (365)
# define NO_PURGE         "skotos:no_purge"

inherit SMTP_API;
inherit "/lib/string";

mapping search_results;

static void
create()
{
}

void
find_udats()
{
    int     i, sz;
    mixed   *users;
    mapping map;

    map = USERDB->query_users();
    sz = map_sizeof(map);
    users = map_values(map);
    for (i = 0; i < sz; i++) {
	users[i] = map_values(users[i]);
	if (!sizeof(users[i])) {
	    users[i] = nil;
	}
    }
    call_out("delayed_find_udats", 1, users - ({ nil }), ([
        "started_at":    time(),
	"finished_at":   0,
	"checked":       0,
	"skipped":       0,
	"found_skipped": ([ ]),
	"found_banned":  ([ ]),
	"found_old":     ([ ]),
	"found_noplay":  ([ ]),
	"total_skipped": 0,
	"total_banned":  0,
	"total_old":     0,
	"total_noplay":  0,
    ]));
}

private void
add_to_map(mapping map, object udat, mixed data)
{
    string  name;
    mapping sub;

    name = udat->query_name();
    sub  = map[name[.. 1]];
    if (!sub) {
	map[name[.. 1]] = sub = ([ ]);
    }
    sub[name[2 ..]] = data;
}

private string
dump_udat_map(mapping map, varargs int width)
{
    int     i, sz_i;
    string  text, line;
    mapping names, *list;

    width = width ? width : 72;
    text = "";
    line = "";
    sz_i = map_sizeof(map);
    list = map_values(map);
    for (i = 0; i < sz_i; i++) {
	int j, sz_j;
	string name;
	mixed **data;

	sz_j = map_sizeof(list[i]);
	data = map_values(list[i]);
	for (j = 0; j < sz_j; j++) {
	    name = data[j][1];
	    if (data[j][2] > 0) {
		name += "[" + ((time() - data[j][2]) / 86400) + "]";
	    }
	    if (j < sz_j - 1 || i < sz_i - 1) {
		name += ",";
	    }
	    if (!strlen(line)) {
		line = "  " + name;
	    } else if (strlen(line) + strlen(name) + 1 <= width) {
		line += " " + name;
	    } else {
		text += line + "\n";
		line = "  " + name;
	    }
	}
    }
    if (strlen(line)) {
	text += line + "\n";
    }
    return text;
}

void
report_found_udats(mapping results)
{
    string text;

    if (!results) {
	results = search_results;
	if (!results) {
	    return;
	}
    }
    text =
	"Started at:   " + ctime(results["started_at"]) + "\n" +
	"Finished at:  " + ctime(results["finished_at"]) + "\n" +
	"Execution at: " + ctime(results["execution_at"]) + "\n" +
	"Checked:      " + results["checked"] + "\n";

    if (results["total_skipped"] > 0) {
	text +=
	    "\n" +
	    "No-purge accounts skipped (" + results["total_skipped"] + "):\n" +
	    "\n" +
	    dump_udat_map(results["found_skipped"]);
    }
    if (results["total_banned"] > 0) {
	text +=
	    "\n" +
	    "Banned accounts skipped (" + results["total_banned"] + "):\n" +
	    "\n" +
	    dump_udat_map(results["found_banned"]);
    }
    if (results["total_old"] > 0) {
	text +=
	    "\n" +
	    "Old accounts found (" + results["total_old"] + "):\n" +
	    "\n" +
	    dump_udat_map(results["found_old"]);
    }
    send_message(/* env  */ "harte@skotos.net",
                 /* from */ "Erwin Harte", "harte@skotos.net",
# if 0
                 /* to   */ ([ "eng-fire@skotos.net": "Fire!" ]),
                 /* cc   */ ([ "harte@skotos.net": "Erwin Harte" ]),
# else
                 /* to   */ ([ "harte@skotos.net": "Erwin Harte" ]),
                 /* cc   */ ([ ]),
# endif
                 /* subj */ "[Automated] UserDB purge alert",
                 /* xtra */ ({ }),
                 text +
                 "-- \n" +
                 "/usr/UserDB/sys/empty.c -> find_udats()\n");
}

private void
find_udat(object node, mapping results)
{
    int stamp;

    results["checked"]++;
    if (node->query_property(NO_PURGE)) {
	add_to_map(results["found_skipped"], node, ({ node, node->query_name(), 0 }));
	results["total_skipped"]++;
	return;
    }
    if (node->query_user_ban()) {
	/* Don't purge banned users, we'd lose the ban info and such. */
	add_to_map(results["found_banned"], node, ({ node, node->query_name(), 0 }));
	results["total_banned"]++;
	return;
    }
    switch (node->query_account_type()) {
    case AS_REGULAR:
    case AS_TRIAL:
	break;
    default:
	/* AS_{FREE,DEVELOPER,STAFF} */
	return;
    }
    if (node->query_card()) {
	/* Still paying, one would assume. */
	return;
    }
    if (node->query_property("payment_status") > 0) {
	/* Has at one point paid, let's keep this one. */
	return;
    }
    stamp = node->query_next_stamp();
    if (stamp < time() - PURGE_EXPIRE_AGE * 86400) {
	add_to_map(results["found_old"], node, ({ node, node->query_name(), stamp }));
	results["total_old"]++;
    }
}

static void
delayed_find_udats(object **nodes, mapping results)
{
    object node;

    while (sizeof(nodes) && !sizeof(nodes[0])) {
	nodes = nodes[1..];
    }
    if (!sizeof(nodes)) {
	results["finished_at"] = time();
	results["execution_at"] = time() + 7 * 86400;
	search_results = results;
	report_found_udats(results);
	return;
    }
    node = nodes[0][0];
    nodes[0] = nodes[0][1..];

    find_udat(node, results);

    if (results["checked"] % 100 == 0) {
	DEBUG("delayed_find_udats:" + results["checked"] + " checked so far; " +
	      results["total_skipped"] + " skipped; " +
	      results["total_banned"]  + " banned; " +
	      results["total_old"]     + " found.");
    }

    /*
     * Up to 10 per second, that means it will still take around 2 hours to
     * finish.
     */
    call_out("delayed_find_udats", 0.2, nodes, results);
}
