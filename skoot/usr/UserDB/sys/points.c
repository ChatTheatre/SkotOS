/*
 * ~UserDB/sys/points.c
 *
 * Keep track of the (most recently) handed out storypoints, for easy checking
 * and weekly summaries and such.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include <UserDB.h>

private inherit "/lib/string";
private inherit "/usr/SMTP/lib/smtp_api";

/*
 * A mapping from timestamps to an array of entries, each entry containing the
 * person that got the points, who gave them, how many, and why.
 */
private mapping history;

/*
 * A mapping from totals to a mapping from user objects to the timestamp when
 * they most recently got storypoints.
 */
private mapping toplist;

/*
 * A reverse mapping to easily find a player's total, avoids a loop over their
 * current data although that might not be that much effort, so if this becomes
 * too cumbersome it can be removed.
 */
private mapping reverse;

/* static */ void
create()
{
    history = ([ ]);
    toplist = ([ ]);
    reverse = ([ ]);
}

void
add_entry(int when, int points, string reason,
	  object from, string from_name, object to, string to_name)
{
    int     total;
    mapping map;
    mixed   *entry;

    DEBUG("POINTS:" + when + ":" + points + "/" + reason + "/" +
	  from_name + "/" + to_name);
    entry = history[when];
    if (!entry) {
	entry = ({ });
    }
    history[when] = entry + ({ ({ from_name, to_name, points, reason }) });

    map = reverse[to_name[..1]];
    if (map) {
	if (map[to_name[2..]]) {
	    total = map[to_name[2..]];
	    toplist[total][to] = nil;
	    if (!map_sizeof(toplist[total])) {
		toplist[total] = nil;
	    }
	} else {
	    total = 0;
	}
    } else {
	reverse[to_name[..1]] = map = ([ ]);
	total = 0;
    }
    total += points;
    map[to_name[2..]] = total;

    map = toplist[total];
    if (!map) {
	toplist[total] = map = ([ ]);
    }
    map[to] = when;
}

mapping
query_history_map()
{
    return history[..];
}

mapping
query_toplist_map()
{
    return toplist[..];
}

mapping
query_reverse_map()
{
    return reverse[..];
}

string
dump_toplist()
{
    int     i, sz, *scores, rank;
    string  result;
    mapping *users;

    result = "";
    sz     = map_sizeof(toplist);
    scores = map_indices(toplist);
    users  = map_values(toplist);

    result = "#    Total:  Name(s):\n";
    rank = 1;

    for (i = sz - 1; i >= 0; i--) {
	int j, sz_j;
	string *names;
	object *list;

	sz_j = map_sizeof(users[i]);
	list = map_indices(users[i]);
	names = allocate(sz_j);
	for (j = 0; j < sz_j; j++) {
	    names[j] = capitalize(list[j]->query_name());
	}
	result +=
	    pad_left((string)rank, 3) + "  " +
	    pad_left((string)scores[i], 6) + "  " +
	    implode(names, " ") + "\n";
	rank += sz_j;
    }
    return result;
}

string
dump_history(int from, int to)
{
    int     i, sz, *stamps;
    string  header;
    mapping submap;
    mixed   ***data;
    string  **result;

    if (to) {
	submap = history[from..to];
	header = "Story Points history from " + ctime(from) + " to " +
	         ctime(to) + ":\n";
    } else {
	submap = history[from..];
	header = "Story Points history from " + ctime(from) + " to present:\n";
    }
    header += "\n";
    result =  ({
	({ "Date:" }),
	({ "#" }),
	({ "For:" }),
	({ "From:" })
    });
    sz     = map_sizeof(submap);
    stamps = map_indices(submap);
    data   = map_values(submap);

    for (i = sz - 1; i >= 0; i--) {
	int j, sz_j;
	string prefix;

	prefix = ctime(stamps[i])[4..18];

	sz_j = sizeof(data[i]);
	for (j = 0; j < sz_j; j++) {
	    result[0] += ({ prefix });
	    result[1] += ({ (string)data[i][j][2] });
	    result[2] += ({ capitalize(data[i][j][1]) });
	    result[3] += ({ capitalize(data[i][j][0]) });
	    prefix = "";
	}
    }

    return header + tabulate(result...);
}

void
email_history_week()
{
    int t;
    string result;
    t = time();
    t -= t % 86400;

    result = dump_history(t - 7 * 86400, t);
    send_message("owner-monitor@skotos.net",
		 "UserDB", "monitor@skotos.net",
		 ([ "harte@staff.skotos.net": "Erwin Harte" ]), ([ ]),
		 "UserDB: Story Points weekly summary",
		 ({ }),
		 result +
		 "-- \n" +
		 "/usr/SMTP/sys/points.c -> email_history_week()\n" +
		 "Comments/questions to eng-cave@skotos.net.\n");
}

void
email_toplist()
{
    string result;

    result = dump_toplist();
    send_message("owner-monitor@skotos.net",
		 "UserDB", "monitor@skotos.net",
		 ([ "harte@staff.skotos.net": "Erwin Harte" ]), ([ ]),
		 "UserDB: Story Points Toplist",
		 ({ }),
		 result +
		 "-- \n" +
		 "/usr/SMTP/sys/points.c -> email_toplist()\n" +
		 "Comments/questions to eng-cave@skotos.net.\n");
}

/*
 * Only for maintenance.
 */

private int
add_user(object who)
{
    int   i, sz;
    mixed **log;

    log = who->query_property("sps:log");
    if (!log || !sizeof(log)) {
	return FALSE;
    }
    for (i = 0, sz = sizeof(log); i < sz; i++) {
	add_entry(log[i][0], log[i][2], log[i][1],
		  log[i][4], log[i][3], who, who->query_name());
    }
    return TRUE;
}

void
add_users()
{
    int     i, sz;
    mixed   *users;
    mapping map;

    create();

    map = USERDB->query_users();
    sz = map_sizeof(map);
    users = map_values(map);
    for (i = 0; i < sz; i++) {
        users[i] = map_values(users[i]);
	if (!sizeof(users[i])) {
	    users[i] = nil;
	}
    }
    call_out("delayed_add_users", 1, 0, users - ({ nil }), 0);
}

void
delayed_add_users(int total, object **users, int found)
{
    int count;

    count = 0;
    while (count < 100 && sizeof(users)) {
        string  hear;
        object  who;
	mapping map;

	who = users[0][0];
	users[0] = users[0][1..];
	if (!sizeof(users[0])) {
	    users = users[1..];
	}
	if (add_user(who)) {
	   found++;
	}
	count++;
    }
    total += count;

    if (total % 1000 == 0) {
        DEBUG("delayed_add_users: " + total + " done (" + found + " found).");
    }

    if (sizeof(users)) {
        call_out("delayed_add_users", 1, total, users, found);
	return;
    } else {
        DEBUG("delayed_add_users: DONE!  " + found + " found.");
    }
}
