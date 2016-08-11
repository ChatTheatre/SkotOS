/*
 * ~UserDB/sys/stats.c - Process statistical information.
 *
 * Copyright Skotos Tech Inc 2001 - 2002.
 */

/*
 * TODO: Store the most recent 'category' that a player is in, so that
 *       other bits of code can easily reuse that information.
 */

# include <type.h>
# include <UserDB.h>

# define DATA_TEC       "~/data/tec-conversion/current.text"
# define DATA_ROYALTIES "~/data/royalties/"
# define DATA_STATS     "~/data/statistics/recent"
# define DATA_REFERRALS "~/data/statistics/referrals"
# define DATA_EMAIL     "~/data/email/"

# define NO_EMAIL       "no-email"

# if 1
#   define STATS_INTERVAL (12 * 3600) /* Every three hours */
# else
#   define STATS_INTERVAL (3600) /* Every hour, for now */
# endif

# define B_NONE     0
# define B_CLEAR    1
# define B_PREAUTH  2
# define B_SALE     3
# define B_EXTERNAL 4

/*
 * The 6 categories I'm using to indicate what sort of user we're dealing
 * with, updated each time the run_stats() are done.
 */

# define S_PROPERTY "billing_status"
                            /* The property to store the billing status in */

# define S_FREE     "free"  /* Freebies, could be customers, could be
			     * storyplotters, etc. */
# define S_PAID     "paid"  /* Customers that have actually paid once (or
			     * more) */
# define S_AUTH     "auth"  /* Customers that have authenticated but are still
			     * in their trial month. */
# define S_TRIAL    "trial" /* Other customers that are still in their trial
			     * month. */
# define S_GONE     "gone"  /* Other customers that have decided not to play,
			     * but may be emailed. */
# define S_LOST     "lost"  /* The remaining lot, gone and don't want to be
			     * contacted. */

/*
 * When do we start counting.
 */
# define R_YEAR  2001
# define R_MONTH    6

/*
 * When to start 'trusting' the cplayed data (Sat Jun  9 00:00:00 2001 PDT)
 */
# define H_TIMESTAMP 992070000

inherit "/lib/date";
inherit "/lib/string";

int     stats_handle, run_stats_handle;

int     final_timestamp;
mapping final_result;

int     tec_timestamp;
mapping tec_result;

int     royalties_timestamp;
mapping royalties_result;

private mapping referral_conversions;

/*
 * Prototypes:
 */

        int **calculate_royalties(object who, string *games, int d_year, int d_month);
        void  calculate_royalties_summary(mapping r_result, int year, int month, string title, string suffix);
        void  dump_royalties_acclaim(int year, int month);
private void  read_referral_conversions();

/*
 * The actual code
 */

static void
create()
{
    stats_handle = call_out("perform_run_stats",
			    STATS_INTERVAL - (time() % STATS_INTERVAL) +
			    5 * 60);
    read_referral_conversions();
}

void patch()
{
    read_referral_conversions();
}

void
reboot()
{
    if (stats_handle) {
        remove_call_out(stats_handle);
    }
    stats_handle = call_out("perform_run_stats",
			    STATS_INTERVAL - (time() % STATS_INTERVAL) +
			    5 * 60);
}

mapping query_referral_conversions() { return referral_conversions[..]; }

private void
read_referral_conversions()
{
    int i, sz;
    string text, *lines, heard, match, *others;
    mapping reverse, *matches;

    DEBUG("Loading referral conversions...");
    reverse = ([ ]);
    referral_conversions = ([ ]);
    text = read_file("~/data/referral.conversions");
    if (!text) {
	return;
    }
    lines = explode(text, "\n");
    sz = sizeof(lines);
    for (i = 0; i < sz; i++) {
	string line;

	line = lines[i];
	if (!line) {
	    continue;
	}
	line = strip(line);
	sscanf(line, "%s#", line);
	if (!strlen(line)) {
	    continue;
	}
	if (sscanf(line, "HEARD:%s", heard)) {
	    heard = strip(heard);
	    continue;
	}
	if (sscanf(line, "MATCH:%s", match)) {
	    match = lower_case(strip(match));
	    if (!referral_conversions[match]) {
		referral_conversions[match] = heard;
		if (!reverse[heard]) {
		    reverse[heard] = ([ ]);
		}
		reverse[heard][match] = TRUE;
	    }
	}
    }
    DEBUG("Done (" + map_sizeof(referral_conversions) + " total)");
    sz = map_sizeof(reverse);
    others = map_indices(reverse);
    matches = map_values(reverse);
    for (i = 0; i < sz; i++) {
	DEBUG("Matches for \"" + others[i] + "\": " +
	      map_sizeof(matches[i]));
    }

    remove_file("~/data/referral.conversions.new");
    write_file("~/data/referral.conversions.new",
	       "# --------------------------------------\n" +
	       "# File re-generated on " + ctime(time()) + "\n" +
	       "# --------------------------------------\n" +
	       "\n");
    for (i = 0; i < sz; i++) {
	int j, sz_j;
	string *str;

	write_file("~/data/referral.conversions.new",
		   "# --------------------------------------\n" +
		   "HEARD: " + others[i] + "\n" +
		   "# --------------------------------------\n" +
		   "\n");
	sz_j = map_sizeof(matches[i]);
	str = map_indices(matches[i]);
	for (j = 0; j < sz_j; j++) {
	    write_file("~/data/referral.conversions.new",
		       "MATCH: " + str[j] + "\n");
	}
	write_file("~/data/referral.conversions.new",
		   "\n");
    }
}

static void
perform_run_stats()
{
    this_object()->run_stats();
    stats_handle = call_out("perform_run_stats",
			    STATS_INTERVAL - (time() % STATS_INTERVAL) +
			    5 * 60);
}

void
run_stats()
{
    int     i, sz, total, t, stamp_b, stamp_e, month, year;
    string  filename, *games;
    mapping map, dist;
    mixed   *users;

    if (run_stats_handle) {
	DEBUG("Removing already running delayed_run_stats() call_out sequence.");
	remove_call_out(run_stats_handle);
	run_stats_handle = 0;
    }
    /* map = ([ "ab": ([ "cd": <object>, ... ]), ... ]) */
    map = USERDB->query_users();
    sz = map_sizeof(map);
    users = map_values(map);
    for (i = 0; i < sz; i++) {

	users[i] = map_values(users[i]);
	if (!sizeof(users[i])) {
	    users[i] = nil;
	} else {
	    total += sizeof(users[i]);
	}
    }

    games = GAMEDB->query_games(1);

    /*
     * Figure out if today the new royalty stats need to be calculated.
     */
    t = time();
    if (get_day(t) >= 2) {
        int    c_year, c_month;
        string text;

	month = c_month = get_month(t);
	year  = c_year  = get_year(t);
	if (month == 1) {
	    month = 12;
	    year--;
	} else {
	    month--;
	}

	filename = DATA_ROYALTIES + year + "-" +
	           (month > 9 ? (string)month : "0" + month) + ".csv";
	DEBUG("filename = " + filename);
	text = read_file(filename);
	if (text) {
	    /* This month's data already exists. */
	    filename = nil;
	    if (get_day(t) >= 9) {
		string final;

		final = DATA_ROYALTIES + year + "-" +
		        (month > 9 ? (string)month : "0" + month) + ".final";
		text = read_file(final);
		if (!text) {
		    /* First run that  the final royalties need to be calculated in. */
		    calculate_royalties_summary(royalties_result, year, month, "Final Royalties Report", "final");
		    dump_royalties_acclaim(year, month);
		}
	    }
	}
	if (filename) {
	    int i, sz;

	    stamp_b = calculate_stamp(year,   month,   1, time());
	    stamp_e = calculate_stamp(c_year, c_month, 1, time()) - 1;

	    /* Make sure we stay within the range of the trusted statistics. */
	    if (stamp_b < H_TIMESTAMP) {
	        stamp_b = H_TIMESTAMP;
	    }
	    /* Not likely, but check the end-stamp too: */
	    if (stamp_e < H_TIMESTAMP) {
	        stamp_e = H_TIMESTAMP;
	    }

	    remove_file(filename);
	    write_file(filename, "Name,Billing Type,Billing Monthly");
	    for (i = 0, sz = sizeof(games); i < sz; i++) {
	        write_file(filename,
			   ",Percentage " + games[i] +
			   ",Time " + games[i] +
			   ",Frequency " + games[i] +
			   ",Longevity " + games[i]);
	    }
	    write_file(filename, "\n");
	}
    }

    remove_file(DATA_EMAIL + "work/any.all");

    remove_file(DATA_EMAIL + "work/any.free");
    remove_file(DATA_EMAIL + "work/any.paid");
    remove_file(DATA_EMAIL + "work/any.auth");
    remove_file(DATA_EMAIL + "work/any.trial");
    remove_file(DATA_EMAIL + "work/any.gone");
    remove_file(DATA_EMAIL + "work/any.lost");

    for (i = 0, sz = sizeof(games); i < sz; i++) {
	remove_file(DATA_EMAIL + "work/" + games[i] + ".all");

	remove_file(DATA_EMAIL + "work/" + games[i] + ".free");
	remove_file(DATA_EMAIL + "work/" + games[i] + ".paid");
	remove_file(DATA_EMAIL + "work/" + games[i] + ".auth");
	remove_file(DATA_EMAIL + "work/" + games[i] + ".trial");
	remove_file(DATA_EMAIL + "work/" + games[i] + ".gone");
	remove_file(DATA_EMAIL + "work/" + games[i] + ".lost");
    }

    remove_file("~/data/misc/tec_user_stats");

    /* Go for it! */
    run_stats_handle = call_out("delayed_run_stats", 1, this_user(), users - ({ nil }),
	     games,
	     ([
		 "missing": total,
		 "total":        0,
		 "all":          ([ ]),
		 "1":            ([ ]),
		 "7":            ([ ]),
		 "14":           ([ ]),
		 "30":           ([ ]),
		 "pay":          ([ ]),
		 "all-toplist":  ([ ]),
		 "paid-toplist": ([ ])
		 ]),
	     ([
		 "conversion_trial":   0,
		 "conversion_pending": 0,
		 "conversion_actual":  0,
		 "conversion_noplay":  0,
		 "conversion_lost":    0,
		 "conversion_noshow":  0,
		 "users_trial":        0,
		 "users_pending":      0,
		 "users_actual":       0
		 ]),
	     ([ "games": games, "users": ([ ]) ]),
	     filename, year, month, stamp_b, stamp_e);
}

private int
first_payment(mixed **log)
{
    int i,sz;

    /*
     * Look for the first sign of a card or other sort of payment.
     */
    for (i = 0, sz = sizeof(log); i < sz; i++) {
	if (sizeof(log[i]) == 2) {
	    DEBUG("first_payment: undersized log entry: " + dump_value(log[i]));
	    continue;
	}
	switch (log[i][2]) {
	case BH_AUTH:
	case BH_CHARGE:
	case BH_EXTERNAL:
	case BH_UPGRADE:
	case BH_FEE:
	    return log[i][0];
	default:
	    break;
	}
    }
    return 0;
}

private int
last_payment(mixed **log, varargs int ignore)
{
    int i;
    int preauth, sale, external;

    /* Trial or paid, look at billing-log to figure out which */
    for (i = sizeof(log) - 1; i >= 0; i--) {
	if (ignore && log[i][0] >= ignore) {
	    continue;
	}
	if (sizeof(log[i]) == 2) {
	    DEBUG("last_payment: undersized log entry: " + dump_value(log[i]));
	    continue;
	}
	switch (log[i][2]) {
	case BH_CANCEL:   return B_CLEAR;
	case BH_AUTH:     return B_PREAUTH;
	case BH_CHARGE:   return B_SALE;
	case BH_EXTERNAL: return B_EXTERNAL;
	case BH_UPGRADE:  return B_SALE;
	case BH_CREDIT:   return B_EXTERNAL;
	default:          break;
	}
    }
    return B_NONE;
}

private int
last_payment_time(mixed **log)
{
    int i;

    for (i = sizeof(log) - 1; i >= 0; i--) {
	if (sizeof(log[i]) == 2) {
	    DEBUG("last_payment_time: undersized log entry: " + dump_value(log[i]));
	    continue;
	}
	switch (log[i][2]) {
	case BH_CHARGE:
	case BH_EXTERNAL:
	case BH_UPGRADE:
	case BH_FEE:
	    return log[i][0];
	default:
	    break;
	}
    }
    return 0;
}

private int
has_paid(mixed **log)
{
    int i, total;

    total = 0;
    for (i = sizeof(log) - 1; i >= 0; i--) {
	if (sizeof(log[i]) == 2) {
	    DEBUG("has_paid: undersized log entry: " + dump_value(log[i]));
	    continue;
	}
	switch (log[i][2]) {
	case BH_CHARGE:
	case BH_EXTERNAL:
	case BH_UPGRADE:
	    total++;
	    break;
	default:
	    break;
	}
    }
    return total;
}

private void
mark_entry(object who, mapping result, string section, string entry, varargs int flag)
{
    if (result[section][entry] == nil) {
	result[section][entry] = 0;
    }
    result[section][entry]++;
    if (flag) {
	if (result[section][entry + "-names"] == nil) {
	    result[section][entry + "-names"] = ([ ]);
	}
	result[section][entry + "-names"][who->query_name()] = TRUE;
    }
}

private void
mark_timed(mapping result, string entry, int when)
{
    mark_entry(nil, result, "all", entry);
    if (when >= time() -  1 * 86400) { mark_entry(nil, result, "1",  entry); }
    if (when >= time() -  7 * 86400) { mark_entry(nil, result, "7",  entry); }
    if (when >= time() - 14 * 86400) { mark_entry(nil, result, "14", entry); }
    if (when >= time() - 30 * 86400) { mark_entry(nil, result, "30", entry); }
}

private void
mark_score(mapping result, object who, int when)
{
    string  hear;
    mapping map;

    hear = who->query_property("hear");
    if (hear == nil) {
	return;
    } else if (hear == "other") {
	hear = who->query_property("other");
	if (hear == nil) {
	    hear = "other";
	} else {
	    string conv;
	    /* On the fly fixing: */
	    if (conv = referral_conversions[lower_case(strip(hear))]) {

		if (sscanf(conv, "other:%s", conv)) {
		    DEBUG("Referral:" + who->query_name() + ": other(" +
			  hear + ") => other(" + conv + ")");
		    who->set_property("other", conv);
		    hear = "other (" + conv + ")";
		} else {
		    DEBUG("Referral:" + who->query_name() + ": other(" +
			  hear + ") => " + conv);
		    who->set_property("hear", conv);
		    who->set_property("other", nil);
		    hear = conv;
		}
	    } else {
		hear = "other (" + lower_case(strip(hear)) + ")";
	    }
	}
    }
    map = result["all"]["hear"];
    if (!map) {
	map = result["all"]["hear"] = ([ ]);
    }
    if (map[hear] == nil) {
	map[hear] = 0;
    }
    map[hear]++;

    if (when >= time() - 1 * 86400) {
	map = result["1"]["hear"];
	if (!map) {
	    map = result["1"]["hear"] = ([ ]);
	}
	if (map[hear] == nil) {
	    map[hear] = 0;
	}
	map[hear]++;
    }
    if (when >= time() - 7 * 86400) {
	map = result["7"]["hear"];
	if (!map) {
	    map = result["7"]["hear"] = ([ ]);
	}
	if (map[hear] == nil) {
	    map[hear] = 0;
	}
	map[hear]++;
    }
    if (when >= time() - 30 * 86400) {
	map = result["30"]["hear"];
	if (!map) {
	    map = result["30"]["hear"] = ([ ]);
	}
	if (map[hear] == nil) {
	    map[hear] = 0;
	}
	map[hear]++;
    }
    {
	mixed log;

	log = who->query_billing_log();
	if (!log) {
	  DEBUG("BILLING LOG EMPTY: " + who->query_name() + "!");
	  log = ({ });
	}
	if (log && first_payment(log) > 0) {
	    map = result["pay"]["hear"];
	    if (!map) {
	        map = result["pay"]["hear"] = ([ ]);
	    }
	    if (map[hear] == nil) {
	        map[hear] = 0;
	    }
	    map[hear]++;
	}
    }
}

private void
run_game_stats(object who, int c, mapping result, string prefix)
{
    int    i, sz, did_play, did_create, did_return, did_playtime, duration;
    string *games;
    mapping map;

    map = who->query_game_stats_map();
    if (!map) {
	map = ([ ]);
    }
    games = GAMEDB->query_games(1);
    for (i = 0, sz = sizeof(games); i < sz; i++) {
	int g_create, g_play, g_playtime;
	mixed t_create, t_play;
	mapping sub;

	g_create = 0;
	g_play = 0;
	g_playtime = 0;
	t_create = nil;
	t_play = nil;
	sub = nil;

	sub = map[games[i]];
	if (!sub) {
	    sub = ([ ]);
	}
	if (t_create = sub["created-total"]) {
	    g_create = 1;
	}
	if (t_play = sub["played-total"]) {
	    int l, l_sz;
	    int **lists;

	    lists = map_values(sub["played-times"]);
	    l_sz = sizeof(lists);
	    for (l = 0; l < l_sz; l++) {
		int j, sz_j;
		mixed *stamps;

		stamps = lists[l];
		sz_j = sizeof(stamps);
#if 1
		for (j = 0; j < sz_j; j++) {
		  stamps[j] = (int)stamps[j];
		}
#endif
		g_playtime = stamps[sizeof(stamps) - 2] +
			     stamps[sizeof(stamps) - 1];
		for (j = 0; j < sz_j; j += 2) {
		  duration += stamps[j + 1];
		}
	    }
	    g_create = 1;
	    g_play   = t_play > 0;
	}
	if (g_create) {
	    mark_timed(result, prefix + "game-" + games[i] + "-char", c);
	    if (g_play) {
		mark_timed(result, prefix + "game-" + games[i] + "-played", g_playtime);
		if (c < time() -  1 * 86400 && g_playtime >= time() -  1 * 86400) {
		    mark_entry(nil, result, "1", prefix + "game-" + games[i] + "-returned");
		}
		if (c < time() -  7 * 86400 && g_playtime >= time() -  7 * 86400) {
		    mark_entry(nil, result, "7", prefix + "game-" + games[i] + "-returned");
		}
		if (c < time() - 14 * 86400 && g_playtime >= time() - 14 * 86400) {
		    mark_entry(nil, result, "14", prefix + "game-" + games[i] + "-returned");
		}
		if (c < time() - 30 * 86400 && g_playtime >= time() - 30 * 86400) {
		    mark_entry(nil, result, "30", prefix + "game-" + games[i] + "-returned");
		}
	    } else {
		mark_timed(result, prefix + "game-" + games[i] + "-noplay", c);
	    }
	} else {
	    mark_timed(result, prefix + "game-" + games[i] + "-nochar", c);
	}
	did_create |= g_create;
	did_play   |= g_play;
	if (g_playtime > did_playtime) {
	    did_playtime = g_playtime;
	}
    }
#if 0
    if (duration > 0) {
	result[prefix + "toplist"][who->query_name()] = duration;
    }
#endif
    if (did_create) {
	if (did_play) {
	    mark_timed(result, prefix + "played", did_playtime);
	    if (c < time() -  1 * 86400 && did_playtime >= time() -  1 * 86400) {
		mark_entry(nil, result, "1", prefix + "returned");
	    }
	    if (c < time() -  7 * 86400 && did_playtime >= time() -  7 * 86400) {
		mark_entry(nil, result, "7", prefix + "returned");
	    }
	    if (c < time() - 14 * 86400 && did_playtime >= time() - 14 * 86400) {
		mark_entry(nil, result, "14", prefix + "returned");
	    }
	    if (c < time() - 30 * 86400 && did_playtime >= time() - 30 * 86400) {
		mark_entry(nil, result, "30", prefix + "returned");
	    }
	} else {
	    mark_timed(result, prefix + "noplay", c);
	}
    } else {
	mark_timed(result, prefix + "nochar", c);
    }
}

private void
log_status_change(object who, string old, string new)
{
    if (old == new) {
	return;
    }
    write_file("/usr/UserDB/data/log/changes",
	       time() + " [" + ctime(time()) + "]: " +
	       who->query_name() + " = " +
	       (new ? "\"" + new + "\"" : "<no status>") + " (" +
	       (old ? "\"" + old + "\"" : "<no status>") + ")\n");
}

static void
run_stats_user(object who, mapping result)
{
    int    c, t, i, paid;
    string hear, old_status, type;
    mixed  log;

    result["missing"]--;

    old_status = who->query_property(S_PROPERTY);
    who->clear_property(S_PROPERTY);

    if (who->query_property("creation_time") != nil) {
	c = who->query_property("creation_time");
    } else {
	c = 0;
    }
    mark_timed(result, "total", c);

    /* Create a toplist of where people heard of us */
    mark_score(result, who, c);

    /* NoPlay, NoChar, etc... */
    run_game_stats(who, c, result, "all-");

    /* Billing!  Who pays, or not, and how, when, why, where */
    t = who->query_next_stamp();

    switch (who->query_account_type()) {
    case AS_FREE:
    case AS_DEVELOPER:
    case AS_STAFF:
#if 0
	write_file("~/data/free.list", who->query_name() + "\n");
#endif
	mark_timed(result, "free-total", c);
	mark_timed(result, "unpaid", c);
	who->set_property(S_PROPERTY, S_FREE);
	log_status_change(who, old_status, S_FREE);
	return;
    default:
	break;
    }

    log = who->query_billing_log();
    if (!log) {
        DEBUG("BILLING LOG EMPTY: " + who->query_name() + "!");
	log = ({ });
    }

    if (who->query_trial() && t >= time()) {
	/*
	 * The current trial users.  By checking this before payment status we
	 * also find the returning trial users, a new phenomenon.
	 */
	if (who->query_card()) {
	    string period;

	    if (who->query_monthly()) {
		period = "month";
	    } else if (who->query_quarterly()) {
		period = "quart";
	    } else {
		period = "year";
	    }
	    mark_timed(result, "new-" + period + "-total", c);
	    mark_timed(result, "unpaid", c);
	    who->set_property(S_PROPERTY, S_AUTH);
	    log_status_change(who, old_status, S_AUTH);
	} else {
	    mark_timed(result, "trial-total", c);
	    mark_timed(result, "unpaid", c);
	    who->set_property(S_PROPERTY, S_TRIAL);
	    log_status_change(who, old_status, S_TRIAL);
	}
	return;
    }

    type = who->query_account_status(AS_PREMIUM) ? "premium" : "basic";

    if (who->query_card()) {
	/*
	 * Anyone with a card we attempt to charge from, so we consider
	 * these paying customers no matter what.
	 */
	int w;
	string period;

	w = first_payment(log);
	if (who->query_monthly()) {
	    period = "month";
	} else if (who->query_quarterly()) {
	    period = "quart";
	} else {
	    period = "year";
	}
	switch (has_paid(log)) {
	case 0:
	    mark_timed(result, "new-" + period + "-total", w);
	    mark_timed(result, "unpaid", w);
	    who->set_property(S_PROPERTY, S_AUTH);
	    log_status_change(who, old_status, S_AUTH);
	    break;
	case 1:
	    mark_timed(result, period + "-" + type + "-total", w);
	    mark_timed(result, period + "-total", w);
	    mark_timed(result, "paid", w);
	    run_game_stats(who, c, result, "paid-");
	    who->set_property(S_PROPERTY, S_PAID);
	    log_status_change(who, old_status, S_PAID);
	    break;
	default:
	    w = last_payment_time(log);
	    mark_timed(result, "renew-" + period + "-" + type + "-total", w);
	    mark_timed(result, "renew-" + period + "-total", w);
	    mark_timed(result, "paid", w);
	    run_game_stats(who, c, result, "paid-");
	    who->set_property(S_PROPERTY, S_PAID);
	    log_status_change(who, old_status, S_PAID);
	    break;
	}
	return;
    }
    if (t >= time()) {
	int w;
	string period;

	w = first_payment(log);
	paid = last_payment(log);

	switch (paid) {
	case B_NONE:
	    mark_timed(result, "trial-total", c);
	    mark_timed(result, "unpaid", c);
	    who->set_property(S_PROPERTY, S_TRIAL);
	    log_status_change(who, old_status, S_TRIAL);
	    break;
	case B_CLEAR:
	    if (!has_paid(log)) {
		/* DEBUG("[stats] :: " + who->query_name() + " canceled card before trial is over."); */
		mark_timed(result, "cancel-trial", c);
		mark_timed(result, "unpaid", c);
		who->set_property(S_PROPERTY, S_TRIAL);
		log_status_change(who, old_status, S_TRIAL);
		break;
	    }
	    /*
	     * If they did pay and are apparently still paid up, fall-through
	     * and deal with it asif the last thing was a sale or so.
	     */
	case B_PREAUTH:
	case B_SALE:
	    if (paid == B_PREAUTH) {
		DEBUG("run_stats_user:" + who->query_name() + ": No card but still marked as pre-auth?");
	    }
	    
	    if (who->query_monthly()) {
		period = "month";
	    } else if (who->query_quarterly()) {
		period = "quart";
	    } else {
		period = "year";
	    }
	    switch (has_paid(log)) {
	    case 0:
		mark_timed(result, "new-" + period + "-total", c);
		mark_timed(result, "unpaid", c);
		who->set_property(S_PROPERTY, S_TRIAL);
		log_status_change(who, old_status, S_TRIAL);
		break;
	    case 1:
		mark_timed(result, period + "-" + type + "-total", w);
		mark_timed(result, period + "-total", w);
		mark_timed(result, "paid", w);
		run_game_stats(who, c, result, "paid-");
		who->set_property(S_PROPERTY, S_PAID);
		log_status_change(who, old_status, S_PAID);
		break;
	    default:
		w = last_payment_time(log);
		mark_timed(result, "renew-" + period + "-" + type + "-total", w);
		mark_timed(result, "renew-" + period + "-total", w);
		mark_timed(result, "paid", w);
		run_game_stats(who, c, result, "paid-");
		who->set_property(S_PROPERTY, S_PAID);
		log_status_change(who, old_status, S_PAID);
		break;
	    }
	    break;
	case B_EXTERNAL:
	    if (has_paid(log) == 1) {
		mark_timed(result, "cheque-" + type + "-total", w);
		mark_timed(result, "cheque-total", w);
	    } else {
		w = last_payment_time(log);
		mark_timed(result, "renew-cheque-" + type + "-total", w);
		mark_timed(result, "renew-cheque-total", w);
	    }
	    run_game_stats(who, c, result, "paid-");
	    mark_timed(result, "paid", w);
	    who->set_property(S_PROPERTY, S_PAID);
	    log_status_change(who, old_status, S_PAID);
	    break;
	}
	return;
    }

    mark_timed(result, "gone", t);
    paid = last_payment(log);
    switch (paid) {
    case B_NONE:
	/* Trial users, never paid or even intended to. */
	if (!who->query_email() || who->query_property(NO_EMAIL)) {
	    mark_timed(result, "trial-lost", t);
	} else {
	    mark_timed(result, "trial-market", t);
	}
	break;
    case B_CLEAR:
        /* DEBUG("[stats] :: " + who->query_name() + " canceled card and trial is over."); */
        if (!has_paid(log)) {
	    /* Trial users who preauthenticated but then canceled. */
	    if (!who->query_email() || who->query_property(NO_EMAIL)) {
		mark_timed(result, "cancel-lost", t);
	    } else {
		mark_timed(result, "cancel-market", t);
	    }
	    break;
	}
	/* FALLING THROUGH! */
    case B_PREAUTH:
	/* Is this possible? */
    case B_SALE:
    case B_EXTERNAL:
	/* These people actually paid at some point, are they still
	 * marketable?
	 */
	if (!who->query_email() || who->query_property(NO_EMAIL)) {
	    mark_timed(result, "paid-lost", t);
	} else {
	    mark_timed(result, "paid-market", t);
	}
	break;
    }
    if (!who->query_email() || who->query_property(NO_EMAIL)) {
	who->set_property(S_PROPERTY, S_LOST);
	log_status_change(who, old_status, S_LOST);
    } else {
	who->set_property(S_PROPERTY, S_GONE);
	log_status_change(who, old_status, S_GONE);
    }
}

static void
run_tec_stats_user(object who, string *games, mapping result)
{
    int   t;
    mixed log;

    switch (who->query_account_type()) {
    case AS_FREE:
    case AS_DEVELOPER:
    case AS_STAFF:
	return;
    default:
	break;
    }

    log = who->query_billing_log();
    if (!log) {
        DEBUG("BILLING LOG EMPTY: " + who->query_name() + "!");
	log = ({ });
    }

    /* What's this user's "due date". */
    t = who->query_next_stamp();

    if (who->query_property("tec-user-name")) {
	/*
	 * Relevant user for the conversion statistics.
	 */
	if (has_paid(log) > 0) {
	    result["conversion_actual"]++;
	} else if (who->query_card()) {
	    result["conversion_pending"]++;
	} else {
	    if (!t) {
		/*
		 * Never showed up, so never a timestamp set.
		 */
		result["conversion_noshow"]++;
	    } else {
		if (t >= time()) {
		    mapping map;

		    map = who->query_game_stats_map();
		    if (map && map["tec"] && map["tec"]["played-total"]) {
			result["conversion_trial"]++;
		    } else {
			result["conversion_noplay"]++;
		    }
		} else {
		    result["conversion_lost"]++;
		}
	    }
	}
    }

    if (who->query_card() || t >= time()) {
	/*
	 * Figure out how much they've been playing and if it's significantly
	 * or not.  Significant meaning 40% TEC and more TEC than any other
	 * game.  So 40% TEC and 50% Marrach does not count.
	 */
	int i, sz, index, *seconds, total, seconds_max;

	sz = sizeof(games);
	seconds = allocate(sz);
	seconds_max = 0;
	for (i = 0; i < sz; i++) {
	    int stamp_b;

	    stamp_b = time() - 30 * 86400;
	    if (stamp_b < H_TIMESTAMP) {
		stamp_b = H_TIMESTAMP;
	    }
	    seconds[i] = who->calculate_playing_time(games[i], stamp_b, time());
	    if (seconds[i] > seconds_max) {
		seconds_max = seconds[i];
		index = i;
	    }
	    total += seconds[i];
	}
	if (seconds_max == 0 || games[index] != "tec" ||
	    seconds_max < (total * 2 / 5)) {
	    /* TEC is not the biggest game, or doesn't have at least 40% */
	    if (has_paid(log)) {
	        /*
		 * This is a paying customer, so where -are- they spending their
		 * time, then?
		 */
	        /* DEBUG("[TEC] " + who->query_name() + ":" + dump_value(seconds)); */
	    }
	    return;
	}
	if (has_paid(log) > 0) {
	    result["users_actual"]++;
	    write_file("~/data/misc/tec_user_stats", who->query_name() + "\n");
	} else if (who->query_card()) {
	    result["users_pending"]++;
	} else {
	    result["users_trial"]++;
	} 
    }
}

private string
percentage(float p)
{
    int i;

    i = (int)(p * 1000.0);
    return (i / 10)  + "." + (i % 10);
}

private string
dollars(float p)
{
    int i, r;

    i = (int)(p * 100.0);
    r = i % 100;
    return (i / 100) + "." + (r > 9 ? r : "0" + r);
}

int **
compose_game_toplist(float *avgs)
{
    int     i, sz, current, *result, *reverse;
    mapping *avgs_maps;
    mapping map;

    map = ([ ]);
    sz = sizeof(avgs);
    result  = allocate(sz);
    reverse = allocate(sz);
    for (i = 0; i < sz; i++) {
	if (map[avgs[i]]) {
	    map[avgs[i]][i] = TRUE;
	} else {
	    map[avgs[i]] = ([ i: TRUE ]);
	}
    }

    sz        = map_sizeof(map);
    avgs_maps = map_values(map);
    for (i = sz - 1; i >= 0; i--) {
	int j, sz_j;
	int *avgs_list;

	avgs_list = map_indices(avgs_maps[i]);
	sz_j = sizeof(avgs_list);
	for (j = 0; j < sz_j; j++) {
	    result[current] = avgs_list[j];
	    reverse[avgs_list[j]] = current;
	    current++;
	}
    }
    return ({ result, reverse });
}

static void
run_royalty_user(object who, string *games, int year, int month,
		 string filename, mapping result)
{
    int    t, i, sz, **data, *sums, **results;
    float  *avgs, pay_amount;
    string pay_type, *output;
    mixed  log, *last_payment;

    log = who->query_billing_log();
    if (!log) {
        DEBUG("BILLING LOG EMPTY: " + who->query_name() + "!");
	log = ({ });
    }
    /* Exclude free/trial accounts. */
    switch (who->query_account_type()) {
    case AS_FREE:
    case AS_DEVELOPER:
    case AS_STAFF:
    case AS_TRIAL:
        return;
    default:
        break;
    }
    /* Check if otherwise a paying customer. */
    if (first_payment(log) == 0) {
	return;
    }
    if (!has_paid(log)) {
        /* Interesting, could this actually happen? */
        return;
    }
    last_payment = who->query_last_payment();
    t = who->query_next_stamp();
    if (who->query_card()) {
	if (who->query_monthly()) {
	    pay_type = "monthly";
	    pay_amount = who->query_monthly_cost();
	} else if (who->query_quarterly()) {
	    pay_type = "quarterly";
	    pay_amount = who->query_quarterly_cost() / 3.0;
	} else {
	    pay_type = "yearly";
	    pay_amount = who->query_yearly_cost() / 12.0;
	}
    } else {
	if (t >= time()) {
	    switch (last_payment(log)) {
	    case B_NONE:
		/* Can this actually happen? */
		return;
	    case B_CLEAR:
	        /* Assume that they are/were CC users. */
		if (who->query_monthly()) {
		    pay_type = "monthly";
		    pay_amount = who->query_monthly_cost();
		} else if (who->query_quarterly()) {
		    pay_type = "quarterly";
		    pay_amount = who->query_quarterly_cost() / 3.0;
		} else {
		    pay_type = "yearly";
		    pay_amount = who->query_yearly_cost() / 12.0;
		}
		break;
	    default:
		pay_type = "yearly";
		pay_amount = who->query_yearly_cost() / 12.0;
		break;
	    }
	} else {
	    /*
	     * Person paid, but must've cancelled that or something,
	     * either way, no card set anymore, and time expired, so.. bye.
	     */
	    return;
	}
    }
    sz = sizeof(games);
    data = calculate_royalties(who, games, year, month);
    sums = allocate_int(3);
    avgs = allocate_float(sz);
    for (i = 0; i < sz; i++) {
	sums[0] += data[0][i];
	sums[1] += data[1][i];
	sums[2] += data[2][i];
    }
    if (sums[0] == 0) {
	/* Never played any of the games, amazing. :-) */
	for (i = 0; i < sz; i++) {
	    avgs[i] = 1.0 / (float)sz;
	}
    } else {
	for (i = 0; i < sz; i++) {
	    int j;

	    avgs[i] = (float)data[0][i] / (float)sums[0] +
		      (float)data[1][i] / (float)sums[1];
	    if (sums[2] == 0) {
		avgs[i] /= 2.0;
	    } else {
		avgs[i] = (avgs[i] + (float)data[2][i] / (float)sums[2]) / 3.0;
	    }
	}
    }
    results = compose_game_toplist(avgs);
    result["users"][who] = ([
	"data":     data,
	"sums":     sums,
	"average":  avgs,
	"toplist":  results[0],
	"reverse":  results[1],
	"modified": FALSE,
	"acclaim":  results[0][..2],
	"amount":   pay_amount,
	"premium":  !!who->query_account_status(AS_PREMIUM)
	]);
    output = allocate(sz * 4);
    for (i = 0; i < sz; i++) {
	output[0 + i * 4] = percentage(avgs[i]);
	output[1 + i * 4] = (string)data[0][i];
	output[2 + i * 4] = (string)data[1][i];
	output[3 + i * 4] = (string)data[2][i];
    }
    write_file(filename,
	       who->query_name() + "," + pay_type + "," + pay_amount + "," +
	       !!who->query_account_status(AS_PREMIUM) + "," +
	       implode(output, ",") + "\n");

}

static void
run_email_stats_user(object who, string *games)
{
    int     i, sz;
    string  email, st;
    mapping map;

    if (who->query_property(NO_EMAIL)) {
	return;
    }

    email = who->query_property("email");
    if (!email || email == "" || email == "[unknown]") {
	return;
    }

    write_file(DATA_EMAIL + "work/any.all",
	       who->query_name() + ":" + email + "\n");
    switch (st = who->query_property(S_PROPERTY)) {
    case S_FREE:
    case S_PAID:
    case S_AUTH:
    case S_TRIAL:
    case S_GONE:
	write_file(DATA_EMAIL + "work/any." + st,
		   who->query_name() + ":" + email + "\n");
	break;
    case S_LOST:
	/* Not possible. */
	break;
    default:
	DEBUG(who->query_name() + ": No billing status set?");
	break;
    }

    map = who->query_game_stats_map();
    if (!map || !map_sizeof(map)) {
	return;
    }

    for (i = 0, sz = sizeof(games); i < sz; i++) {
	mapping sub;

	sub = map[games[i]];
	if (!sub || !map_sizeof(sub)) {
	    continue;
	}
	if (!sub["played-total"]) {
	    continue;
	}
	write_file(DATA_EMAIL + "work/" + games[i] + ".all",
		   who->query_name() + ":" + email + "\n");
	switch (st) {
	case S_FREE:
	case S_PAID:
	case S_AUTH:
	case S_TRIAL:
	case S_GONE:
	    write_file(DATA_EMAIL + "work/" + games[i] + "." + st,
		       who->query_name() + ":" + email + "\n");
	    break;
	case S_LOST:
	    /* Not possible. */
	    break;
	default:
	    break;
	}
    }
}

private string
to_string(mixed v)
{
    if (v == nil) {
	return (string)0;
    }
    return (string)v;
}

string
dump_row(mapping result, string entry)
{
    return
	pad_left(to_string(result["all"][entry]), 7) + "  " +
	pad_left(to_string(result["30"][entry]),  7) + "  " +
	pad_left(to_string(result["7"][entry]),   7) + "  " +
	pad_left(to_string(result["1"][entry]),   7) + "\n";
}

private mixed **
compose_toplist(mapping map)
{
    int i, sz, *totals, current;
    string *names;
    mapping reverse;
    mixed **toplist;

    if (!map || !map_sizeof(map)) {
	return ({ });
    }

    sz = map_sizeof(map);
    names = map_indices(map);
    totals = map_values(map);
    reverse = ([ ]);
    for (i = 0; i < sz; i++) {
	if (reverse[totals[i]]) {
	    reverse[totals[i]][names[i]] = TRUE;
	} else {
	    reverse[totals[i]] = ([ names[i]: TRUE ]);
	}
    }
    toplist = allocate(sz);

    sz = map_sizeof(reverse);
    totals = map_indices(reverse);
    for (i = sz - 1, current = 0; i >= 0; i--) {
	int j, sz_j;

	names = map_indices(reverse[totals[i]]);
	sz_j = sizeof(names);
	for (j = 0; j < sz_j; j++) {
	    toplist[current++] = ({ totals[i], names[j] });
	}
    }
    return toplist;
}

private string
dump_score_column(mixed **toplist, int position, varargs int width)
{
    string title, total;

    if (position > sizeof(toplist)) {
	return pad_right("-", 30);
    }
    title = toplist[position - 1][1];
    total = (string)toplist[position - 1][0];
    if (!width) {
        width = 25;
    }
    if (strlen(title) > width) {
        title = title[..width - 1];
    } else {
        title = pad_right(title, width);
    }
    return title + " " + pad_left(total, 4);
}

private string
dump_score_time(mixed **toplist, int position)
{
    int d, h, m, s;
    string name;
    if (position > sizeof(toplist)) {
	return "-";
    }
    name = toplist[position - 1][1];
    s    = toplist[position - 1][0];

    d  = s / 86400;
    s -= d * 86400;
    h  = s / 3600;
    s -= h * 3600;
    m  = s / 60;
    s -= m * 60;

    return
	pad_right(name, 20) + " " +
	pad_left((string)d, 2) + "d " +
	pad_left((string)h, 2) + "h " +
	pad_left((string)m, 2) + "m " +
	pad_left((string)s, 2) + "s";
}

static void
delayed_run_stats(object user, object **users, string *games, mapping result1,
		  mapping result2, mapping result3, string filename,
		  int year, int month, int stamp_b, int stamp_e)
{
    int count;

#if 0
    return;
#endif

    run_stats_handle = 0;

    count = 0;
    while (count < 10 && sizeof(users)) {
	object who;

	who = users[0][0];
	users[0] = users[0][1..];
	if (!sizeof(users[0])) {
	    users = users[1..];
	}
	count++;

	/* Run regular stats. */
	run_stats_user(who, result1);

	/* Run TEC specific stats. */
	run_tec_stats_user(who, games, result2);

	/* Collect email addresses. */
	run_email_stats_user(who, games);

	if (filename) {
	    /* It's that time of the month again. */
	    int i, sz;

	    /*
	     * First calculate the history of the previous month, assuming
	     * that by now all data has arrived and can be processed.
	     */
	    for (i = 0, sz = sizeof(games); i < sz; i++) {
	        int *data;

		data = who->summary_game_stats(games[i], stamp_b, stamp_e);
		who->store_game_history(games[i], year, month, data);
	    }

	    run_royalty_user(who, games, year, month, filename, result3);
	}

#if 1
	/*
	 * Test account_status variable.
	 */
	catch {
	    who->verify_account_type();
	    who->verify_account_status();
	}
#endif
    }
    if (sizeof(users)) {
	run_stats_handle = call_out("delayed_run_stats", 1, user, users, games,
				    result1, result2, result3, filename,
				    year, month, stamp_b, stamp_e);
	return;
    }

    final_result    = result1;
    final_timestamp = time();

    tec_result    = result2;
    tec_timestamp = time();

    if (filename) {
	string initial;

        royalties_result    = result3;
        royalties_timestamp = time();

	/*
	 * Post-process the royalty statistics to calculate the summaries in
	 * one big sweep.
	 */
	calculate_royalties_summary(royalties_result, year, month, "Initial Royalties Report", "initial");
    }

    DEBUG("delayed_run_stats() finished.");

    {
	int    i, sz_i;
	string *ext;

	ext = ({ "all", "free", "paid", "auth", "trial", "gone", "lost" });
	for (i = 0, sz_i = sizeof(ext); i < sz_i; i++) {
	    int j, sz_j;

	    remove_file(DATA_EMAIL + "current/any." + ext[i]);
	    rename_file(DATA_EMAIL + "work/any." + ext[i],
			DATA_EMAIL + "current/any." + ext[i]);
	    for (j = 0, sz_j = sizeof(games); j < sz_j; j++) {
		remove_file(DATA_EMAIL + "current/" + games[j] + "." + ext[i]);
		rename_file(DATA_EMAIL + "work/"    + games[j] + "." + ext[i],
			    DATA_EMAIL + "current/" + games[j] + "." + ext[i]);
		
	    }
	}
    }

    /* Dump most recent versions to file, for emailing purposes. */
    remove_file(DATA_TEC);
    write_file(DATA_TEC,
	       this_object()->dump_statistics("tec-conversion") +
	       "\n" +
	       this_object()->dump_statistics("tec-users"));

    /* Dump most recent versions to file, for emailing purposes. */
    remove_file(DATA_REFERRALS);
    write_file(DATA_REFERRALS,
	       this_object()->dump_statistics("referrals-full"));

    /* Dump most recent version to file, for emailing purposes. */
    remove_file(DATA_STATS);
    write_file(DATA_STATS,
	       "> +statistics subscriptions\n" +
	       this_object()->dump_statistics("subscriptions") +
	       "\n" +
	       "> +statistics referrals\n" +
	       this_object()->dump_statistics("referrals") +
	       "\n" +
	       "> +statistics games-all\n" +
	       this_object()->dump_statistics("games-all") +
	       "\n" +
	       "> +statistics games-paid\n" +
	       this_object()->dump_statistics("games-paid") +
	       "\n" +
	       "> +statistics players-all\n" +
	       this_object()->dump_statistics("players-all") +
	       "\n" +
	       "> +statistics players-paid\n" +
	       this_object()->dump_statistics("players-paid") +
	       "\n" +
	       "> +statistics tec-conversion\n" +
	       this_object()->dump_statistics("tec-conversion") +
	       "\n" +
	       "> +statistics tec-users\n" +
	       this_object()->dump_statistics("tec-users"));
}

string
dump_statistics(string type)
{
    int    i, sz;
    string output, *games, **list, prefix;

    switch (type) {
    case "subscriptions":
	return
	    "[Data collected on " + ctime(final_timestamp) + "]\n" +
	    "\n" +
	    "SUBSCRIPTIONS\n" +
	    "=============            Total    30d      7d       1d\n" +
	    "                         -------  -------  -------  -------\n" +
	    "All Users:               " + dump_row(final_result, "total") +
	    "----\n" +
	    "Trial-Users-Lost:        " + dump_row(final_result, "trial-lost") +
	    "Trial-Users-Marketable:  " + dump_row(final_result, "trial-market") +
	    "Cancelled-Lost:          " + dump_row(final_result, "cancel-lost") +
	    "Cancelled-Marketable:    " + dump_row(final_result, "cancel-market") +
	    "Ex-Customers-Lost        " + dump_row(final_result, "paid-lost") +
	    "Ex-Customers-Marketable: " + dump_row(final_result, "paid-market") +
	    "                         -------  -------  -------  -------\n" +
	    "Total Non-Customers:     " + dump_row(final_result, "gone") +
	    "----\n" +
	    "Trial-Users:             " + dump_row(final_result, "trial-total") +
	    "Free-Accounts:           " + dump_row(final_result, "free-total") +
	    "Cancelled:               " + dump_row(final_result, "cancel-trial") +
	    "Authenticated-Monthly:   " + dump_row(final_result, "new-month-total") +
	    "Authenticated-Quarterly: " + dump_row(final_result, "new-quart-total") +
	    "Authenticated-Yearly:    " + dump_row(final_result, "new-year-total") +
	    "                         -------  -------  -------  -------\n" +
	    "Total Unpaid Users:      " + dump_row(final_result, "unpaid") +
	    "----\n" +
	    "Basic:\n" +
	    "  Paid-Monthly:          " + dump_row(final_result, "month-basic-total") +
	    "  Paid-Quarterly:        " + dump_row(final_result, "quart-basic-total") +
	    "  Paid-Yearly:           " + dump_row(final_result, "year-basic-total") +
	    "  Paid-External:         " + dump_row(final_result, "cheque-basic-total") +
	    "  Renew-Paid-Monthly:    " + dump_row(final_result, "renew-month-basic-total") +
	    "  Renew-Paid-Quarterly:  " + dump_row(final_result, "renew-quart-basic-total") +
	    "  Renew-Paid-Yearly:     " + dump_row(final_result, "renew-year-basic-total") +
	    "  Renew-Paid-External:   " + dump_row(final_result, "renew-cheque-basic-total") +
	    "Premium:\n" +
	    "  Paid-Monthly:          " + dump_row(final_result, "month-premium-total") +
	    "  Paid-Quarterly:        " + dump_row(final_result, "quart-premium-total") +
	    "  Paid-Yearly:           " + dump_row(final_result, "year-premium-total") +
	    "  Paid-External:         " + dump_row(final_result, "cheque-premium-total") +
	    "  Renew-Paid-Monthly:    " + dump_row(final_result, "renew-month-premium-total") +
	    "  Renew-Paid-Quarterly:  " + dump_row(final_result, "renew-quart-premium-total") +
	    "  Renew-Paid-Yearly:     " + dump_row(final_result, "renew-year-premium-total") +
	    "  Renew-Paid-External:   " + dump_row(final_result, "renew-cheque-premium-total") +
	    "                         -------  -------  -------  -------\n" +
	    "Total-Paid-Customers:    " + dump_row(final_result, "paid");
    case "referals":
    case "referrals":
	if (!final_result["all"]["hear_toplist"]) {
	    final_result["all"]["hear_toplist"] = compose_toplist(final_result["all"]["hear"]);
	}
	if (!final_result["1"]["hear_toplist"]) {
	    final_result["1"]["hear_toplist"] = compose_toplist(final_result["1"]["hear"]);
	}
	if (!final_result["7"]["hear_toplist"]) {
	    final_result["7"]["hear_toplist"] = compose_toplist(final_result["7"]["hear"]);
	}
	if (!final_result["30"]["hear_toplist"]) {
	    final_result["30"]["hear_toplist"] = compose_toplist(final_result["30"]["hear"]);
	}
	if (!final_result["pay"]["hear_toplist"]) {
	    final_result["pay"]["hear_toplist"] = compose_toplist(final_result["pay"]["hear"]);
	}
	output =
	    "[Data collected on " + ctime(final_timestamp) + "]\n" +
	    "\n" +
	    "HEARD ABOUT\n" +
	    "===========\n" +
	    "    Total                           30d\n" +
	    "    ------------------------------  ------------------------------\n";
	for (i = 1; i <= 20; i++) {
	    output +=
		pad_left((string)i, 2) + ". " +
		dump_score_column(final_result["all"]["hear_toplist"], i) + "  " +
		dump_score_column(final_result["30"]["hear_toplist"], i) + "\n";
	}
	output +=
	    "\n" +
	    "    7d                              1d\n" +
	    "    ------------------------------  ------------------------------\n";
	for (i = 1; i <= 20; i++) {
	    output +=
		pad_left((string)i, 2) + ". " +
		dump_score_column(final_result["7"]["hear_toplist"], i) + "  " +
		dump_score_column(final_result["1"]["hear_toplist"], i) + "\n";
	}
	output +=
	    "\n" +
	    "    Paying Customers\n" +
	    "    -------------------------------     -------------------------------\n";
	for (i = 1; i <= 20; i++) {
	    output +=
	      pad_left((string)i, 2) + ". " +
	      dump_score_column(final_result["pay"]["hear_toplist"],  i) +
	      "  " +
	      pad_left((string)(i + 20), 2) + ". " +
	      dump_score_column(final_result["pay"]["hear_toplist"], i + 20) +
	      "\n";
	}
	return output;
    case "referrals-full":
        output =
	    "[Data collected on " + ctime(final_timestamp) + "]\n" +
	    "\n" +
	    "HEARD ABOUT\n" +
	    "===========\n" +
	    "    Paying Customers:\n" +
	    "    ------------------------------------------------------------\n";
	if (!final_result["pay"]["hear_toplist"]) {
	    final_result["pay"]["hear_toplist"] =
	        compose_toplist(final_result["pay"]["hear"]);
	  
	}
	if (!final_result["all"]["hear_toplist"]) {
	    final_result["all"]["hear_toplist"] =
	        compose_toplist(final_result["all"]["hear"]);
	}
	for (i = 1; i <= sizeof(final_result["pay"]["hear_toplist"]); i++) {
	    output +=
		pad_left((string)i, 2) + ". " +
		dump_score_column(final_result["pay"]["hear_toplist"], i, 60) +
	        "\n";
	}
	output +=
	    "    All Accounts:\n" +
	    "    ------------------------------------------------------------\n";
	for (i = 1; i <= sizeof(final_result["all"]["hear_toplist"]); i++) {
	    output +=
		pad_left((string)i, 2) + ". " +
		dump_score_column(final_result["all"]["hear_toplist"], i, 60) +
	        "\n";
	}
	return output;
    case "games-paid":
    case "games-all":
    case "games":
	if (type == "games-paid") {
	    prefix = "paid-";
	} else {
	    prefix = "all-";
	}
	output =
	    "[Data collected on " + ctime(final_timestamp) + "]\n" +
	    "\n" +
	    "GAMES\n" +
	    "=====     Created   NoChar    NoPlay    Played    Returned\n" +
	    "          --------  --------  --------  --------  --------\n";
	list = ({
	    ({ "Total",    "all" }),
	    ({ "Last 30d",  "30" }),
	    ({ "Last 14d",  "14" }),
	    ({ "Last 7d",    "7" }),
	    ({ "Last 24h",   "1" })
	});
	for (i = 0, sz = sizeof(list); i < sz; i++) {
	    output +=
		pad_right(list[i][0], 10) +
		pad_left(to_string(final_result[list[i][1]]["total"]), 8) + "  " +
		pad_left(to_string(final_result[list[i][1]][prefix + "nochar"]), 8) + "  " +
		pad_left(to_string(final_result[list[i][1]][prefix + "noplay"]), 8) + "  " +
		pad_left(to_string(final_result[list[i][1]][prefix + "played"]), 8);
	    if (i != 0) {
		output += "  " + pad_left(to_string(final_result[list[i][1]][prefix + "returned"]), 8);
	    }
	    output += "\n";
	}
	games = GAMEDB->query_games(1);
	for (i = 0, sz = sizeof(games); i < sz; i++) {
	    int j, sz_j;
	    string game;

	    game = prefix + "game-" + games[i] + "-";
	    output +=
		"\n" +
		games[i] + ":\n" +
		"========= Char      NoPlay    Played    Returned\n" +
		"          --------  --------  --------  --------\n";
	    for (j = 0, sz_j = sizeof(list); j < sz_j; j++) {
		output +=
		    pad_right(list[j][0], 10) + 
		    pad_left(to_string(final_result[list[j][1]][game + "char"]), 8) + "  " +
		    pad_left(to_string(final_result[list[j][1]][game + "noplay"]), 8) + "  " +
		    pad_left(to_string(final_result[list[j][1]][game + "played"]), 8);
		if (j != 0) {
		    output += "  " + pad_left(to_string(final_result[list[j][1]][game + "returned"]), 8);
		}
		output += "\n";
	    }
	}
	return output;
    case "players-paid":
    case "players-all":
    case "players":
	if (type == "players-paid") {
	    prefix = "paid-";
	} else {
	    prefix = "all-";
	}
	if (!final_result[prefix + "toplist_toplist"]) {
	    final_result[prefix + "toplist_toplist"] = compose_toplist(final_result[prefix + "toplist"]);
	}
	output =
	    "[Data collected on " + ctime(final_timestamp) + "]\n" +
	    "\n" +
	    "TOPLIST PLAYERS\n" +
	    "===============\n";
	for (i = 1; i <= 25; i++) {
	    output += 
		pad_left((string)i, 2) + ". " + dump_score_time(final_result[prefix + "toplist_toplist"],  i) + "\n";
	}
	return output;
    case "tec-conversion":
	output =
	    "[Data collected on " + ctime(tec_timestamp) + "]\n" +
	    "\n" +
	    "TEC Conversion Statistics\n" +
	    "=========================\n" +
	    "  Actual TEC Convertees: " + pad_left((string)tec_result["conversion_actual"],  5) + "\n" +
	    " Pending TEC Convertees: " + pad_left((string)tec_result["conversion_pending"], 5) + "\n" +
	    "   Trial TEC Convertees: " + pad_left((string)tec_result["conversion_trial"],   5) + "\n" +
	    "NP Trial TEC Convertees: " + pad_left((string)tec_result["conversion_noplay"],  5) + "\n" +
	    "    Lost TEC Convertees: " + pad_left((string)tec_result["conversion_lost"],    5) + "\n" +
	    " No Show TEC Convertees: " + pad_left((string)tec_result["conversion_noshow"],  5) + "\n";
	return output;
    case "tec-users":
	output =
	    "[Data collected on " + ctime(tec_timestamp) + "]\n" +
	    "\n" +
	    "TEC User Statistics\n" +
	    "===================\n" +
	    "  Active TEC Users: " + pad_left((string)tec_result["users_actual"],  5) + "\n" +
	    " Pending TEC Users: " + pad_left((string)tec_result["users_pending"], 5) + "\n" +
	    "   Trial TEC Users: " + pad_left((string)tec_result["users_trial"],   5) + "\n";
	return output;
    case "distribution":
        return GAMEDB->dump_distribution();
    }
}

/* ------------------------------------------------------------------------- *\
	     Created  NoPlay  NoChar  Played  Returned  % Days
   total     5434
   last 30d   908     356     99      809     257       26
   last 14d   405     155     48      471     221       39
   last 7d    184      78     27      295     189       52
   last 24h    31      14      5      130     113       71
% days = #individual days logged on since creation of the character, divided
by the number of days that have passed since then, but for our purpose we will
use the first registered login-time in place of the creation-time.
\* ------------------------------------------------------------------------- */

mapping
query_final_result()
{
    return final_result;
}

/*
 * For each month, we need to know:
 * - The number of hours played.
 * - The number of individual days played.
 * - The consecutive months played _before_ this month.
 *
 * What I'm really tempted to do is to only store the hours and days, and to
 * calculate the consecutive months by checking the history as we've stored it
 * so far.
 *
 * But, there's one catch.  If someone hasn't played at all, not on any game,
 * the 3 values are "extended" to the current month, for each game.  Which can
 * work recursively if someone doesn't play for several months.
 *
 * Crazy, but true.
 *
 * So what I need to have in order to make this work is:
 * - A function that calculates for each game the #hours and #days played.
 * - A function that, starts at the beginning of history as it has been
 *   previously calculated and stored, and figures out which time, frequency
 *   and longevity values to keep.
 * - So the trick is to not try to deal with longevity at all but to just focus
 *   on calculating the other bits, and if the month is over and the new values
 *   are definite, we can deducde the real longevity and other values and make
 *   our calculations with those.
 */

int **
calculate_royalties(object who, string *games, int d_year, int d_month)
{
    int c_when, d_when;
    int *time_arr, *freq_arr, *long_arr;
    int i, sz;
    int *prev_arr;

    c_when  = R_YEAR * 12 + R_MONTH;
    d_when  = d_year * 12 + d_month;

    prev_arr = allocate_int(sizeof(games));

    sz = sizeof(games);
    time_arr = allocate_int(sz);
    freq_arr = allocate_int(sz);
    long_arr = allocate_int(sz);

    while (c_when <= d_when) {
	int c_year, c_month;
	int **data, non_zero;

	prev_arr = long_arr[..];

	c_year  = c_when / 12;
	c_month = c_when % 12;

	data = allocate(sz);
	non_zero = FALSE;
	for (i = 0; i < sz; i++) {
	    data[i] = who->query_game_history(games[i], c_year, c_month);
	    if (data[i][0] != 0) {
		non_zero = TRUE;
	    }
	}
	if (non_zero) {
	    /*
	     * The user played at least one of the games, so only check if
	     * this means extending the longevity of the played games.
	     */
	    for (i = 0; i < sz; i++) {
		if (data[i][0]) {
		    long_arr[i]++;
		    time_arr[i] = data[i][0];
		    freq_arr[i] = data[i][1];
		} else {
		    long_arr[i] = 0;
		    time_arr[i] = 0;
		    freq_arr[i] = 0;
		}
	    }
	} else {
	    /*
	     * The user didn't play any of the games, keep the time and
	     * frequency values of the previous month and extend the longevity
	     * values for those games that he/she had played last month.
	     */
	    for (i = 0; i < sz; i++) {
		if (time_arr[i]) {
		    long_arr[i]++;
		}
	    }
	}
	c_when++;
    }
    return ({ time_arr, freq_arr, prev_arr });
}

string *
query_royalties_games()
{
    return royalties_result ? royalties_result["games"] : ({ });
}

void
clear_royalties_result()
{
    royalties_result = nil;
}

/*
 * This returns nil if you're either not a paying customer, it's not within
 * 7 days of the calculation of last month's royalties, or you've already
 * reordered your top 3.
 */
int *
query_royalties_acclaim(object who)
{
    int t;

    if (!royalties_timestamp || !royalties_result) {
        return nil;
    }
    if (!royalties_result["users"][who]) {
        return nil;
    }
    if (royalties_result["users"][who]["modified"]) {
        return nil;
    }
    t = time();
    if (t >= royalties_timestamp + 7 * 86400) {
        /* You only get 7 days to adjust your top 3. */
        return nil;
    }
    return royalties_result["users"][who]["acclaim"];
}

atomic string
set_royalties_acclaim(object who, int *acclaim)
{
    int i, sz, t, *old;

    if (!royalties_timestamp || !royalties_result) {
	return "There is no top 3 data available for you to modify";
    }
    if (!royalties_result["users"][who]) {
	return "There is no top 3 data available for you to modify";
    }
#if 1
    if (royalties_result["users"][who]["modified"]) {
	return "You have already modified the top 3 of this month";
    }
#endif
    t = time();
    if (t >= royalties_timestamp + 7 * 86400) {
	/* You only get 7 days to adjust your top 3. */
	return "You are too late to modify the top 3 of this month";
    }
    old = royalties_result["users"][who]["acclaim"][..];
    if (sizeof(old) != sizeof(acclaim)) {
	return "Internal error: Notify CE!";
    }
    for (i = 0, sz = sizeof(acclaim); i < sz; i++) {
	if (!sizeof(old & acclaim[i..i])) {
	    return "You seem to have constructed an invalid top 3";
	}
	old -= acclaim[i..i];
    }
    royalties_result["users"][who]["acclaim"] = acclaim;
    royalties_result["users"][who]["modified"] = TRUE;
    return nil;
}

mapping
query_royalties_user(object who)
{
    return royalties_result["users"][who];
}

float *
calculate_user_royalties(string name, int *acclaim)
{
    int     i, sz;
    string  *games;
    float   *result;
    object  who;
    mapping map;

    who = USERDB->query_user(name);
    if (!who) {
	return nil;
    }
    map = royalties_result["users"][who];
    if (!map) {
	return nil;
    }
    games = royalties_result["games"];
    sz = sizeof(games);
    result = allocate_float(sz);
    for (i = 0; i < sz; i++) {
	int acc_j;

	if (map["reverse"][i] >= 3) {
	    acc_j = i;
	} else {
	    acc_j = acclaim[map["reverse"][i]];
	}
	result[i] = map["average"][acc_j];
    }
    return result;
}

string
dump_csv_line(mixed data...)
{
    string *bits;
    int i, sz;

    sz = sizeof(data);
    bits = allocate(sz);
    for (i = 0; i < sz; i++) {
        switch (typeof(data[i])) {
	case T_INT:
	case T_FLOAT:
	    bits[i] = (string)data[i];
	    break;
	case T_STRING:
	    bits[i] = data[i];
	    break;
	default:
	    bits[i] = "";
	    break;
        }
	if (sscanf(bits[i], "%*s,")) {
	    bits[i] =
	      "\"" +
	      implode(explode("\"" + bits[i] + "\"", "\""), "\"\"") +
	      "\"";
	}
    }
    return implode(bits, ",");
}

void
dump_royalties_acclaim(int year, int month)
{
    int i, sz_i;
    string filename, result, *games;
    object *users;
    mixed  *data;

    filename = year + "-" + (month > 9 ? (string)month : "0" + month) + ".acclaim";
    remove_file(DATA_ROYALTIES + filename);
    result = "Name,Initial 1,Initial 2,Initial 3,Final 1,Final 2,Final 3\n";

    users = map_indices(royalties_result["users"]);
    data = map_values(royalties_result["users"]);
    sz_i = sizeof(users);

    games = royalties_result["games"];

    for (i = 0; i < sz_i; i++) {
        string stat;

	if (data[i]["toplist"][0] == data[i]["acclaim"][0] &&
	    data[i]["toplist"][1] == data[i]["acclaim"][1] &&
	    data[i]["toplist"][2] == data[i]["acclaim"][2]) {
	    stat = "identical";
	} else {
	    stat = "modified";
	}
        result +=
	  dump_csv_line(users[i]->query_name(),
			games[data[i]["toplist"][0]],
			games[data[i]["toplist"][1]],
			games[data[i]["toplist"][2]],
			games[data[i]["acclaim"][0]],
			games[data[i]["acclaim"][1]],
			games[data[i]["acclaim"][2]],
			stat) + "\n";
    }
    write_file(DATA_ROYALTIES + filename, result);
}

void
calculate_royalties_summary(mapping r_result, int year, int month, string title, string suffix)
{
    /*
     * Setup:
     *
     * Initialize the totals for each to 0.0 (float).
     * Loop over all users, take their payment value and averages into
     * account, and add the relevant bits to the relevant games.
     */
    int     i, sz_i;
    string  *games, result, filename, *targets;
    float   *t_totals;
    object  *users;
    mixed   *data;
    mapping *summary, payments, *g_royalties, *totals;

    games = r_result["games"];

    sz_i = sizeof(games);
    g_royalties = allocate(sz_i);
    totals      = allocate(sz_i);
    summary     = allocate(sz_i);
    for (i = 0; i < sz_i; i++) {
	int    j, sz_j;
	string *cats;
	float  r;

	g_royalties[i] = ([ "basic": 0.0, "premium": 0.0 ]);
	totals[i]      = ([ "basic": 0.0, "premium": 0.0 ]);
	cats = GAMEDB->query_distr_cats(games[i], "basic");
	sz_j = sizeof(cats);
	for (j = 0; j < sz_j; j++) {
	    g_royalties[i]["basic"] += (float)GAMEDB->query_distr_cat_perc(games[i], "basic", cats[j]) / 100.0;
	}
	cats = GAMEDB->query_distr_cats(games[i], "premium");
	sz_j = sizeof(cats);
	for (j = 0; j < sz_j; j++) {
	    g_royalties[i]["premium"] += (float)GAMEDB->query_distr_cat_perc(games[i], "premium", cats[j]) / 100.0;
	}

	summary[i] = ([
	    "basic": ([
	        "users":        0,
		"users-all":    0,
		"ftes":         0.0,
		"rating":       0.0,
		"royalties":    0.0,
		"subscription": 0.0,
		"long perc":    0.0,
		"long avg":     0.0,
		"time perc":    0.0,
		"time avg":     0.0,
		"freq perc":    0.0,
		"freq avg":     0.0,
		"accl perc":    0.0,
		"accl rate":    0.0,
		]),
	    "premium": ([
		"users":        0,
		"users-all":    0,
		"ftes":         0.0,
		"rating":       0.0,
		"royalties":    0.0,
		"subscription": 0.0,
		"long perc":    0.0,
		"long avg":     0.0,
		"time perc":    0.0,
		"time avg":     0.0,
		"freq perc":    0.0,
		"freq avg":     0.0,
		"accl perc":    0.0,
		"accl rate":    0.0,
		]),
	    "total": ([
		"users":        0,
		"users-all":    0,
		"ftes":         0.0,
		"rating":       0.0,
		"royalties":    0.0,
		"subscription": 0.0,
		"long perc":    0.0,
		"long avg":     0.0,
		"time perc":    0.0,
		"time avg":     0.0,
		"freq perc":    0.0,
		"freq avg":     0.0,
		"accl perc":    0.0,
		"accl rate":    0.0,
		])
	    ]);
    }

    users = map_indices(r_result["users"]);
    data = map_values(r_result["users"]);
    sz_i = sizeof(users);
    for (i = 0; i < sz_i; i++) {
	int    j, sz_j, game;
	string type;
	float  *avgs, amount;

	type = data[i]["premium"] ? "premium" : "basic";

	/*
	 * Calculate royalty for each game.
	 */
	amount = data[i]["amount"];
	avgs = data[i]["average"];
	for (j = 0, sz_j = sizeof(games); j < sz_j; j++) {
	    int acc_j;

	    if (data[i]["reverse"][j] >= 3) {
		acc_j = j;
	    } else {
		acc_j = data[i]["acclaim"][data[i]["reverse"][j]];
	    }
	    totals[j][type] += amount * avgs[acc_j];
	}

	/*
	 * Calculate generic statistics for the #1 game of this user,
	 * assuming it is played at least 40% of the user's time.
	 */
	game = data[i]["toplist"][0];
	if (data[i]["average"][game] >= 0.4) {
	    /*
	     * This is a user of the game 'game', whichever one that is.
	     */
	    summary[game][type]["users"]++;
	} else {
	    /*
	     * This user doesn't play any particular game enough to be called
	     * a user of that game.
	     */
	  DEBUG(users[i]->query_name() + "\t" +
		to_string(users[i]->query_property("name")) + "\t" +
		( users[i]->query_card() ?
		  ( users[i]->query_monthly() ? "monthly" :
		    users[i]->query_quarterly() ? "quarterly" : "yearly" ) :
		  "check") + "\t" +
		(users[i]->query_property("creation_time") ? 
		 ctime(users[i]->query_property("creation_time")) :
		 "<no creation time>") + "\t" +
		(users[i]->query_next_stamp() ?
		 ctime(users[i]->query_next_stamp()) : "<no next stamp>"));
	}

	for (j = 0, sz_j = sizeof(games); j < sz_j; j++) {
	    int acc_j;

	    if (data[i]["average"][j] <= 0.0) {
		continue;
	    }
	    if (data[i]["reverse"][j] >= 3) {
		/*
		 * This game wasn't one of the first 3.
		 */
		acc_j = j;
		summary[j][type]["accl rate"] += (float) (data[i]["reverse"][j] + 1);
	    } else {
		int k;

		acc_j = data[i]["acclaim"][data[i]["reverse"][j]];
		for (k = 0; k < 3; k++) {
		    if (data[i]["acclaim"][k] == j) {
			summary[j][type]["accl rate"] += (float) (k + 1);
			break;
		    }
		}
		if (k == 3) {
		    DEBUG(users[i]->query_name() + ": " + dump_value(data[i]));
		}
	    }
	    summary[j][type]["accl perc"] += data[i]["average"][acc_j];
	    summary[j][type]["users-all"]++;

	    summary[j][type]["ftes"]         += data[i]["average"][j];
	    summary[j][type]["rating"]       += data[i]["average"][j];
	    summary[j][type]["royalties"] += g_royalties[j][type] * data[i]["amount"] * data[i]["average"][acc_j];
	    summary[j][type]["subscription"] += data[i]["amount"];

	    summary[j][type]["time avg"] += (float)data[i]["data"][0][j];
	    if (data[i]["sums"][0] == 0) {
		summary[j][type]["time perc"] += (float)1 / (float)3;
	    } else {
		summary[j][type]["time perc"] += (float)data[i]["data"][0][j] / (float)data[i]["sums"][0];
	    }

	    summary[j][type]["freq avg"] += (float)data[i]["data"][1][j];
	    if (data[i]["sums"][1] == 0) {
		summary[j][type]["freq perc"] += (float)1 / (float)3;
	    } else {
		summary[j][type]["freq perc"] += (float)data[i]["data"][1][j] / (float)data[i]["sums"][1];
	    }

	    summary[j][type]["long avg"] += (float)data[i]["data"][2][j];
	    if (data[i]["sums"][2] == 0) {
		summary[j][type]["long perc"] += (float)1 / (float)3;
	    } else {
		summary[j][type]["long perc"] += (float)data[i]["data"][2][j] / (float)data[i]["sums"][2];
	    }
	}
    }

    for (i = 0, sz_i = sizeof(games); i < sz_i; i++) {
	int    j, sz_j;
	string *labels;

	labels = map_indices(summary[i]["total"]);
	sz_j = sizeof(labels);
	for (j = 0; j < sz_j; j++) {
	    summary[i]["total"][labels[j]] = summary[i]["basic"][labels[j]] + summary[i]["premium"][labels[j]];
	}
    }

    result = "[Data collected on " + ctime(time()) + "] " + title + ":\n";
    for (i = 0, sz_i = sizeof(games); i < sz_i; i++) {
	int    j, sz_j;
	float  total;
	string desc, *types;

	desc = GAMEDB->query_game_by_name(games[i])[4];
	DEBUG("games[i] = " + games[i] + " desc = " + dump_value(desc));
	result +=
	    "\n" + desc + "\n" +
	    "----------------------------------------"[..strlen(desc) - 1] + "\n";

	types = ({ "basic", "premium", "total" });
	sz_j = sizeof(types);
	for (j = 0; j < sz_j; j++) {
	    string type;

	    type = types[j];
	    total = (float)summary[i][type]["users-all"];
	    result +=
		"Category '" + types[j] + "':\n" +
		"   Total Users:               " + summary[i][type]["users"] + "\n" +
		"   FTEs:                      " + dollars(summary[i][type]["ftes"]) + "\n" +
		"   Average Rating:            " + percentage(summary[i][type]["rating"] / total) + "%\n" +
		"   Total Royalties:           $" + dollars(summary[i][type]["royalties"]) + "\n" +
		"   Average Subscription Rate: $" + dollars(summary[i][type]["subscription"] / total) + "\n" +
		"   Average Longevity:         " + percentage(summary[i][type]["long perc"] / total) + "%, " +
		                                   dollars(summary[i][type]["long avg"] / total) + " months\n" +
		"   Average Time:              " + percentage(summary[i][type]["time perc"] / total) + "%, " +
		                                   dollars(summary[i][type]["time avg"] / total) + " hours\n" +
		"   Average Frequency:         " + percentage(summary[i][type]["freq perc"] / total) + "%, " +
		                                   dollars(summary[i][type]["freq avg"] / total) + " days\n" +
		"   Average Acclaim:           " + percentage(summary[i][type]["accl perc"] / total) + "%, " +
		                                   "rated #" + dollars(summary[i][type]["accl rate"] / total) + "\n";
	}
    }

    result += "\n";

    payments = ([ ]);
    for (i = 0, sz_i = sizeof(games); i < sz_i; i++) {
	int    t, sz_t;
	string *types, desc;

	desc = GAMEDB->query_game_by_name(games[i])[4];
	result +=
	    "\n" + desc + "\n" +
	    "----------------------------------------"[..strlen(desc) - 1] + "\n";

	types = ({ "basic", "premium" });
	sz_t = sizeof(types);
	for (t = 0; t < sz_t; t++) {
	    int j, sz_j;
	    float total;
	    string *cats, type;

	    type = types[t];
	    total = totals[i][type];
	    result += "Category '" + type + "':\n";
	    cats = GAMEDB->query_distr_cats(games[i], type);
	    for (j = 0, sz_j = sizeof(cats); j < sz_j; j++) {
		int k, sz_k;
		int cat_perc;
		string *subcats;

		cat_perc = GAMEDB->query_distr_cat_perc(games[i], type, cats[j]);
		result += "  " + cats[j] + " (" + percentage((float)cat_perc / 100.0) + "%): $" + dollars(total * (float)cat_perc / 100.0) + "\n";
		subcats = GAMEDB->query_distr_subcats(games[i], type, cats[j]);
		for (k = 0, sz_k = sizeof(subcats); k < sz_k; k++) {
		    int perc;
		    string amount;

		    perc = GAMEDB->query_distr_subcat_perc(games[i], type, cats[j], subcats[k]);
		    amount = dollars(total * (float)perc / 100.0);
		    if (!payments[subcats[k]]) {
			payments[subcats[k]] = 0.0;
		    }
		    payments[subcats[k]] += (float)amount;
		    result += "    " + subcats[k] + " (" + percentage((float)perc / 100.0) + "%): $" + amount + "\n";
		}
	    }
	}
    }
    sz_i     = map_sizeof(payments);
    targets  = map_indices(payments);
    t_totals = map_values(payments);
    result += "\nSummary:\n";
    for (i = 0; i < sz_i; i++) {
	result += targets[i] + ": $" + dollars(t_totals[i]) + "\n";
    }
    filename = year + "-" + (month > 9 ? (string)month : "0" + month) + "." + suffix;
    remove_file(DATA_ROYALTIES + filename);
    write_file(DATA_ROYALTIES + filename, result);
}

int
get_this_month()
{
    return get_month(time());
}

/*
 * CUSTOM: Temporary stuff for requests from Shannon and others.
 */

void
dump_paying_referrals()
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

    remove_file("~/data/misc/referral-paid");

    call_out("delayed_paying_referrals", 1, 0, users - ({ nil }), ([ ]), ([ ]));
}

void
delayed_paying_referrals(int total, object **users, mapping found, mapping sums)
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
	count++;

	/* ---------------------------------------- */

	if (who->query_property("billing_status") == "paid") {
	    hear = who->query_property("hear");
	    if (hear == nil) {
		hear = "<unknown>";
	    } else if (hear == "other") {
		hear = who->query_property("other");
		if (hear == nil) {
		    hear = "other";
		} else {
		    hear = "other (" + hear + ")";
		}
	    }
	    map = found[hear];
	    if (!map) {
		found[hear] = map = ([ ]);
		sums[hear] = 0;
	    }
	    map[who->query_name()] = TRUE;
	    sums[hear]++;
	}
    }
    total += count;

    DEBUG("delayed_paying_referrals(): " + total + " done.");

    if (sizeof(users)) {
        call_out("delayed_paying_referrals", 1, total, users, found, sums);
	return;
    } else {
        int     i, j, sz, *totals;
        string  *what;
	mapping map, *maps;

	map = ([ ]);
	sz     = map_sizeof(sums);
	what   = map_indices(sums);
	totals = map_values(sums);

	for (i = 0; i < sz; i++) {
	    mapping sub;

	    sub = map[totals[i]];
	    if (!sub) {
	        sub = map[totals[i]] = ([ ]);
	    }
	    sub[what[i]] = found[what[i]];
	}

	sz     = map_sizeof(map);
	totals = map_indices(map);
	maps   = map_values(map);
	for (i = sz - 1, j = 1; i >= 0; i--) {
	    int k, sz_k;
	    mapping *names;

	    sz_k  = map_sizeof(maps[i]);
	    what  = map_indices(maps[i]);
	    names = map_values(maps[i]);
	    for (k = 0; k < sz_k; k++, j++) {
		write_file("~/data/misc/referral-paid",
			   "[" + pad_left((string)j, 3) + "] " +
			   what[k]  + " (" + totals[i] + " )\n" +
			   "      " +
			   implode(map_indices(names[k]), ", ") + "\n");
	    }
	}
    }
}


void
dump_paid_tec()
{
    int     i, sz;
    string  *games;
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

    games = GAMEDB->query_games(1);

    call_out("delayed_paid_tec", 1, 0, users - ({ nil }), games, ([ ]));
}

void
delayed_paid_tec(int total, object **users, string *games, mapping found)
{
    int count;

    count = 0;
    while (count < 200 && sizeof(users)) {
        string  hear;
        object  who;
	mapping map;

	who = users[0][0];
	users[0] = users[0][1..];
	if (!sizeof(users[0])) {
	    users = users[1..];
	}
	count++;

	/* ---------------------------------------- */

	if (who->query_property("billing_status") == "paid") {
	    /* Sort out who's TEC and who's not. */

	    int i, sz, seconds_max, index, total, *seconds;

	    sz = sizeof(games);
	    seconds = allocate(sz);
	    seconds_max = 0;
	    index = 0;
	    total = 0;
	    for (i = 0; i < sz; i++) {
		int stamp_b;

		stamp_b = time() - 30 * 86400;
		if (stamp_b < H_TIMESTAMP) {
		    stamp_b = H_TIMESTAMP;
		}
		seconds[i] = who->calculate_playing_time(games[i], stamp_b, time());
		if (seconds[i] > seconds_max) {
		    seconds_max = seconds[i];
		    index = i;
		}
		total += seconds[i];
	    }
	    if (seconds_max == 0 || games[index] != "tec" || seconds_max < (total * 2 / 5)) {
	        DEBUG("Skipped " + who->query_name() + ": " + dump_value(seconds) + " (max: " + seconds_max + ", index = " + index + ", games[index] = " + games[index] + ", total = " + total + ")");
	    } else {
	        found[who] = ({ seconds_max, total });
	    }
	}
    }
    total += count;

    DEBUG("delayed_paid_tec(): " + total + " done.");

    if (sizeof(users)) {
        call_out("delayed_paid_tec", 1, total, users, games, found);
	return;
    } else {
        int i, sz, **seconds;
	object *list;
	string *props;

	sz   = map_sizeof(found);
	list = map_indices(found);
	seconds = map_values(found);

	props = ({
	  "name",
	  "street1",
	  "street2",
	  "city",
	  "state",
	  "zip",
	  "country",
	});

	remove_file("~/data/misc/paid_tec");

	/* CSV labels: */
	write_file("~/data/misc/paid_tec",
		   "Account,Played,Total,Percentage,Name,Street1,Street2,City,State,Zip,Country\n");

	for (i = 0; i < sz; i++) {
	    int j, sz_j;
	    mixed *vals;
	    object who;
	    int empty;

	    who = list[i];
	    sz_j = sizeof(props);
	    vals = allocate(sz_j);
	    empty = TRUE;

	    for (j = 0; j < sz_j; j++) {
		vals[j] = who->query_property(props[j]);
		vals[j] = vals[j] ? vals[j] : "";
		if (props[j] != "name" && props[j] != "country") {
		    empty = empty && vals[j] == "";
		}
		vals[j] = "\"" +
		          implode(explode("\"" + vals[j] + "\"", "\""),
				  "\"\"") +
		          "\"";
	    }
	    if (empty) {
	        for (j = 0; j < sz_j; j++) {
		    vals[j] = who->query_property("b_" + props[j]);
		    vals[j] = vals[j] ? vals[j] : "";
		    vals[j] = "\"" +
			      implode(explode("\"" + vals[j] + "\"", "\""),
				      "\"\"") +
			      "\"";
		}
	    }
	    write_file("~/data/misc/paid_tec",
		       who->query_name() + "," +
		       seconds[i][0] + "," + seconds[i][1] + "," + (100 * seconds[i][0] / seconds[i][1]) + "," +
		       implode(vals, ",") + "\n");
	}
	DEBUG("delayed_paid_tec(): DONE!");
    }
}

void
check_royalties_data()
{
  int i, sz;
  object *list;

  list = map_indices(royalties_result["users"]);
  sz = sizeof(list);
  for (i = 0; i < sz; i++) {
    switch (list[i]->describe_account_type()) {
    case "free":
    case "developer":
      DEBUG("check_royalties_data:" + list[i]->query_name() + ":" +
	    list[i]->describe_account_type() + ":" +
	    list[i]->query_card() + ":" + 
	    list[i]->query_property("billing_status") + ":" +
	    list[i]->query_property("payment_status") + ":" +
	    last_payment(list[i]->query_billing_log(), 1014971758));
      royalties_result["users"][list[i]] = nil;
      DEBUG("Removed from list.");
      break;
    default:
      break;
    }
  }
}

int
total_royalties_users()
{
  return map_sizeof(royalties_result["users"]);
}

int
test_royalties_user(object who)
{
    mixed log, last_payment;

    switch (who->query_account_type()) {
    case AS_FREE:
    case AS_DEVELOPER:
    case AS_STAFF:
    case AS_TRIAL:
	return 0;
    default:
	log = who->query_billing_log();
	if (first_payment(log) == 0) {
	    return 0;
	}
	if (!has_paid(log)) {
	    return 0;
	}
	last_payment = who->query_last_payment();
	if (!last_payment) {
	    DEBUG("test_royalties_user:" + who->query_name() + ":No last payment data.");
	    return 2;
	}
	if (who->query_card()) {
	    if (who->query_cost() == last_payment[1]) {
		return 1;
	    }
	    DEBUG("test_royalties_user:" + who->query_name() + ":Last payment = " + last_payment[1] + "; query_cost() = " + who->query_cost());
	    return 2;
	} else {
	    if (who->query_yearly_cost() == last_payment[1]) {
		return 1;
	    }
	    DEBUG("test_royalties_user:" + who->query_name() + ":Last payment = " + last_payment[1] + "; query_yearly_cost() = " + who->query_yearly_cost());
	    return 2;
	}
	return 1;
    }
}

void
premium_royalties()
{
    int     m, sz_m;
    string  *games;
    mapping *maps, result;

    games = GAMEDB->query_games(1);

    maps = ({
	BILLINGD->query_cc_users(),
	BILLINGD->query_other_users()
    });
    sz_m = sizeof(maps);
    for (m = 0; m < sz_m; m++) {
	int     i, sz_i;
	mapping *names, map;

	map = maps[m];
	sz_i  = map_sizeof(map);
	names = map_values(map);
	for (i = 0; i < sz_i; i++) {
	    int    j, sz_j;
	    object *list;

	    sz_j = map_sizeof(names[i]);
	    list = map_values(names[i]);
	    for (j = 0; j < sz_j; j++) {
		run_royalty_user(list[j], games, 2002, 10, "/usr/UserDB/data/test/2002-10.csv", result);
	    }
	}
    }
    
}

void
update_royalties_data()
{
    int     i, sz, total;
    mapping map;
    mixed   *users;

    map = USERDB->query_users();
    sz = map_sizeof(map);
    users = map_values(map);
    for (i = 0; i < sz; i++) {
        users[i] = map_values(users[i]);
	total += sizeof(users[i]);
	if (!sizeof(users[i])) {
	    users[i] = nil;
	}
    }
    call_out("update_royalties_data_delayed", 1, users - ({ nil }), total, 0, 0);
}

static void
update_royalties_data_delayed(object **users, int total, int checked, int changed)
{
    int count;

    while (count < 50 && sizeof(users)) {
        object current;

	current = users[0][0];
	users[0] = users[0][1..];
	if (!sizeof(users[0])) {
	    users = users[1..];
	}
	count++;
	if (!royalties_result["users"][current]) {
	    mapping result;

	    result = ([ "games": royalties_result["games"],
			"users": ([ ])
			]);
	    run_royalty_user(current,
			     royalties_result["games"], 2002, 2,
			     DATA_ROYALTIES + "update.csv", result);
	    if (result["users"][current]) {
	      DEBUG("update_royalties_data_delayed: " + current->query_name() +
		    " added.");
	      royalties_result["users"][current] = result["users"][current];
	      changed++;
	    }
	}
    }
    checked += count;
    if (checked % 1000 == 0) {
      DEBUG("update_royalties_data_delayed: " + checked + "/" + total + " checked; " + changed + " changed.");
    }
    if (sizeof(users)) {
	call_out("update_royalties_data_delayed", 1, users, total, checked, changed);
    } else {
	DEBUG("update_royalties_data_delayed: DONE! Checked = " + checked + "; changed: " + changed);
    }
}
