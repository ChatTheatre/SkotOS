/*
**	~UserDB/sys/userd.c
**
**	Go through the user database at regular intervals (nightly)
**	and see who needs to be charged; then charge them.
**
**	Copyright Skotos Tech 2001-2002
*/

# include <type.h>
# include <status.h>
# include <SAM.h>
# include <SkotOS.h>
# include <mapargs.h>
# include <UserDB.h>

private inherit "/lib/string";
private inherit "/lib/date";
private inherit "/usr/SMTP/lib/smtp_api";

int	handle;
mapping cc_users;
mapping cc_stats;
mapping other_users;
mapping other_stats;
mapping history;
mapping free_users;
mapping developer_users;
mapping staff_users;
mapping premium_users;
mapping cc_used;
mapping trial_users;

# define PING_AGE (86400 * 128)

static
void create() {
   ::create();

   cc_users        = ([ ]);
   other_users     = ([ ]);
   history         = ([ ]);
   free_users      = ([ ]);
   developer_users = ([ ]);
   staff_users     = ([ ]);
   premium_users   = ([ ]);
   cc_used         = ([ ]);
   trial_users     = ([ ]);
}

int
query_callout_delay()
{
    if (handle) {
	int i, sz;
	mixed **co;

	co = status(this_object())[O_CALLOUTS];
	sz = sizeof(co);
	for (i = 0; i < sz; i++) {
	    if (co[i][CO_HANDLE] == handle) {
		return co[i][CO_DELAY];
	    }
	}
    }
    return -1;
}

static
void schedule_callout() {
   int now, h, seconds;

   now = time();

   h = get_hour(now);

   if (handle) {
      remove_call_out(handle);
   }

   /* figure out how many seconds 'til 3 AM rolls around again */

   seconds = 3600 * 24 - (
      3600 * (h - 3) +
      60 * get_minute(now) +
      get_second(now));

   if (seconds < 12 * 3600) {
      /* something weird happened, maybe daylight savings; bump */
      seconds += 3600 * 24;
   }

   SysLog("scheduling next billing callout in " + seconds + " seconds.");
   handle = call_out("filter_users", seconds);
}

void boot(int block) {
   if (ur_name(previous_object()) == "/usr/UserDB/initd") {
      schedule_callout();
   }
}

atomic int
add_cc_user(object user, varargs int silently)
{
   string  uname;
   mapping map;

#if 0
    if (user->query_next_stamp() <= time()) {
	return FALSE;
    }
#endif
    if (!user->query_card()) {
	return FALSE;
    }
    uname = user->query_name();
    map = cc_users[uname[.. 1]];
    if (!map) {
	map = cc_users[uname[.. 1]] = ([ ]);
    }
   if (!map[uname[2 ..]]) {
     map[uname[2 ..]] = user;
     DEBUG("BILLING -> add_cc_user: " + uname + " added to list.");
   } else {
     if (!silently) {
       DEBUG("BILLING -> add_cc_user: " + uname + " already in list.");
     }
   }
}

atomic void
remove_cc_user(object user, varargs int silent)
{
   string  uname;
   mapping map;

   uname = user->query_name();
   map = cc_users[uname[.. 1]];
   if (map) {
      if (map[uname[2 ..]]) {
	 map[uname[2 ..]] = nil;
	 DEBUG("BILLING -> remove_cc_user: " + uname + " removed from list.");
	 if (!map_sizeof(map)) {
	   cc_users[uname[.. 1]] = nil;
	 }
	 return;
      }
   }
   if (!silent) {
       DEBUG("BILLING -> remove_cc_user: " + uname + " was not in list.");
   }
}

int
is_cc_user(object user)
{
    string  uname;
    mapping map;

    uname = user->query_name();
    map = cc_users[uname[.. 1]];
    if (!map) {
	return FALSE;
    }
    return !!map[uname[2 ..]];
}

atomic int
add_other_user(object user, varargs int silent)
{
   string  uname;
   mapping map;

   if (user->describe_account_type() != "regular") {
       return FALSE;
   }
   if (user->query_card()) {
       return FALSE;
   }
   if (user->query_next_stamp() <= time() && !user->query_account_credit()) {
       return FALSE;
   }
   uname = user->query_name();
   map = other_users[uname[.. 1]];
   if (!map) {
      map = other_users[uname[.. 1]] = ([ ]);
   }
   if (!map[uname[2 ..]]) {
     map[uname[2 ..]] = user;
     DEBUG("BILLING -> add_other_user: " + uname + " added to list.");
   } else {
     DEBUG("BILLING -> add_other_user: " + uname + " already in list.");
   }
   return TRUE;
}

atomic void
remove_other_user(object user)
{
   string  uname;
   mapping map;

   uname = user->query_name();
   map = other_users[uname[.. 1]];
   if (map) {
      if (map[uname[2 ..]]) {
	 map[uname[2 ..]] = nil;
	 DEBUG("BILLING -> remove_other_user: " + uname + " removed from list.");
	 if (!map_sizeof(map)) {
	   other_users[uname[.. 1]] = nil;
	 }
	 return;
      }
   }
   DEBUG("BILLING -> remove_other_user: " + uname + " was not in list.");
}

atomic int
add_trial_user(object user)
{
    string  uname;
    mapping map;

    uname = user->query_name();
    map = trial_users[uname[.. 1]];
    if (!map) {
	map = trial_users[uname[.. 1]] = ([ ]);
    }
    if (!map[uname[2 ..]]) {
	map[uname[2 ..]] = user;
	DEBUG("BILLING -> add_trial_user: " + uname + " added to list.");
    } else {
	DEBUG("BILLING -> add_trial_user: " + uname + " already in list.");
    }
    return TRUE;
}

atomic int
remove_trial_user(object user)
{
    string uname;
    mapping map;

    uname = user->query_name();
    map = trial_users[uname[.. 1]];
    if (map) {
	if (map[uname[2 ..]]) {
	    map[uname[2 ..]] = nil;
	    DEBUG("BILLING -> remove_trial_user: " + uname + " removed from list.");
	    if (!map_sizeof(map)) {
		trial_users[uname[.. 1]] = nil;
	    }
	    return TRUE;
	}
    }
    DEBUG("BILLING -> remove_trial_user: " + uname + " was not in list.");
    return FALSE;
}

atomic int
add_free_user(object user)
{
    string  uname;
    mapping map;

    uname = user->query_name();
    map = free_users[uname[.. 1]];
    if (!map) {
	map = free_users[uname[.. 1]] = ([ ]);
    }
    if (!map[uname[2 ..]]) {
	map[uname[2 ..]] = user;
	DEBUG("BILLING -> add_free_user: " + uname + " added to list.");
    } else {
	DEBUG("BILLING -> add_free_user: " + uname + " already in list.");
    }
    return TRUE;
}

atomic int
remove_free_user(object user)
{
    string uname;
    mapping map;

    uname = user->query_name();
    map = free_users[uname[.. 1]];
    if (map) {
	if (map[uname[2 ..]]) {
	    map[uname[2 ..]] = nil;
	    DEBUG("BILLING -> remove_free_user: " + uname + " removed from list.");
	    if (!map_sizeof(map)) {
		free_users[uname[.. 1]] = nil;
	    }
	    return TRUE;
	}
    }
    DEBUG("BILLING -> remove_free_user: " + uname + " was not in list.");
    return FALSE;
}

atomic int
add_developer_user(object user)
{
    string  uname;
    mapping map;

    uname = user->query_name();
    map = developer_users[uname[.. 1]];
    if (!map) {
	map = developer_users[uname[.. 1]] = ([ ]);
    }
    if (!map[uname[2 ..]]) {
	map[uname[2 ..]] = user;
	DEBUG("BILLING -> add_developer_user: " + uname + " added to list.");
    } else {
	DEBUG("BILLING -> add_developer_user: " + uname + " already in list.");
    }
    return TRUE;
}

atomic int
remove_developer_user(object user)
{
    string uname;
    mapping map;

    uname = user->query_name();
    map = developer_users[uname[.. 1]];
    if (map) {
	if (map[uname[2 ..]]) {
	    map[uname[2 ..]] = nil;
	    DEBUG("BILLING -> remove_developer_user: " + uname + " removed from list.");
	    if (!map_sizeof(map)) {
		developer_users[uname[.. 1]] = nil;
	    }
	    return TRUE;
	}
    }
    DEBUG("BILLING -> remove_developer_user: " + uname + " was not in list.");
    return FALSE;
}

atomic int
add_staff_user(object user)
{
    string  uname;
    mapping map;

    uname = user->query_name();
    map = staff_users[uname[.. 1]];
    if (!map) {
	map = staff_users[uname[.. 1]] = ([ ]);
    }
    if (!map[uname[2 ..]]) {
	map[uname[2 ..]] = user;
	DEBUG("BILLING -> add_staff_user: " + uname + " added to list.");
    } else {
	DEBUG("BILLING -> add_staff_user: " + uname + " already in list.");
    }
    return TRUE;
}

atomic int
remove_staff_user(object user)
{
    string uname;
    mapping map;

    uname = user->query_name();
    map = staff_users[uname[.. 1]];
    if (map) {
	if (map[uname[2 ..]]) {
	    map[uname[2 ..]] = nil;
	    DEBUG("BILLING -> remove_staff_user: " + uname + " removed from list.");
	    if (!map_sizeof(map)) {
		staff_users[uname[.. 1]] = nil;
	    }
	    return TRUE;
	}
    }
    DEBUG("BILLING -> remove_staff_user: " + uname + " was not in list.");
    return FALSE;
}

atomic int
add_premium_user(object user, varargs int silently)
{
    string  uname;
    mapping map;

    uname = user->query_name();
    map = premium_users[uname[.. 1]];
    if (!map) {
	map = premium_users[uname[.. 1]] = ([ ]);
    }
    if (!map[uname[2 ..]]) {
	map[uname[2 ..]] = user;
	DEBUG("BILLING -> add_premium_user: " + uname + " added to list.");
    } else {
        if (!silently) {
	    DEBUG("BILLING -> add_premium_user: " + uname + " already in list.");
	}
    }
    return TRUE;
}

atomic int
remove_premium_user(object user, varargs int silent)
{
    string uname;
    mapping map;

    uname = user->query_name();
    map = premium_users[uname[.. 1]];
    if (map) {
	if (map[uname[2 ..]]) {
	    map[uname[2 ..]] = nil;
	    DEBUG("BILLING -> remove_premium_user: " + uname + " removed from list.");
	    if (!map_sizeof(map)) {
		premium_users[uname[.. 1]] = nil;
	    }
	    return TRUE;
	}
    }
    if (!silent) {
        DEBUG("BILLING -> remove_premium_user: " + uname + " was not in list.");
    }
    return FALSE;
}

void filter_users() {
   mapping *bits;
   object **nodes;
   int i, tot;

   schedule_callout();

   /* create an array of arrays of all relevant user nodes */
   bits = map_values(cc_users);
   nodes = allocate(sizeof(bits));

   for (i = 0; i < sizeof(bits); i ++) {
      nodes[i] = map_values(bits[i]);
      tot += map_sizeof(bits[i]);
   }

   cc_stats = ([
      "total":             0,     /* How many requests have been sent.       */
      "done":              0,     /* How many have been dealt with so far.   */
      "users":             tot,   /* How many (credit card) users there are. */
      "check":             FALSE, /* Whether charge_users() is done.         */
      "success_monthly":   0,     /* How many monthly users had success.     */
      "success_quarterly": 0,     /* How many quarterly users had success.   */
      "success_yearly":    0,     /* How many yearly users had success.      */
      "failed_monthly":    0,     /* How many monthly users failed.          */
      "failed_quarterly":  0,     /* How many quarterly users failed.        */
      "failed_yearly":     0,     /* How many yearly users failed.           */
      "failed_counts":     ([ ]), /* How many times did this happen now?     */
      "failed_reasons":    ([ ]), /* What are the reasons for failure?       */
   ]);

   SysLog("BILLING: Starting charge_users; " + tot + " users...");
   call_out("charge_users", 0, nodes);
}

void patch() {
    if (!staff_users) {
	staff_users = ([ ]);
    }
    if (!premium_users) {
        premium_users = ([ ]);
    }
    if (!cc_used) {
	cc_used = ([ ]);
    }
    if (!trial_users) {
	trial_users = ([ ]);
    }
}

void
add_history(int stamp, object user, string name, int event,
	    varargs mixed extra...)
{
    int   day;
    mixed **list;

    day = stamp / 86400;

    if (!history) {
	history = ([ ]);
    }
    if (!history[day]) {
	history[day] = ([ ]);
    }
    list = history[day][stamp];
    if (!list) {
	list = ({ });
    }
    history[day][stamp] = list + ({ ({ stamp, name, user, event }) + extra });
}

mixed ***
query_history(int from, int to)
{
    int     i, sz, from_day, to_day, *days;
    mapping *maps, events, map;

    if (!history || !map_sizeof(history)) {
	return ({ });
    }
    from_day = from / 86400;
    to_day   = to   / 86400;

    map = history[from_day..to_day];
    sz = map_sizeof(map);
    if (!sz) {
	return ({ });
    }
    days = map_indices(map);
    maps = map_values(map);
    if (days[0] == from_day) {
	maps[0] = maps[0][from..];
    }
    if (days[sz - 1] == to_day) {
	maps[sz - 1] = maps[sz - 1][..to];
    }
    events = ([ ]);
    for (i = 0; i < sz; i++) {
	events += maps[i];
    }
    return map_values(events);
}

string
describe_history_event(int e)
{
    switch (e) {
    case BH_AUTH:
	return "Card Authenticated";
    case BH_CANCEL:
	return "Card Canceled";
    case BH_CHARGE:
	return "Card Charged";
    case BH_EXTERNAL:
	return "External Payment";
    case BH_EXPIRE:
	return "Account Expired";
    case BH_CREATED:
	return "Account Created";
    case BH_BUMPED:
	return "Dueday bumped";
    case BH_AFAILURE:
	return "Card Authentication failure";
    case BH_SFAILURE:
	return "Card Charge failure";
    case BH_UPGRADE:
        return "Account Upgrade";
    case BH_UFAILURE:
        return "Account Upgrade failure";
    case BH_TEXT:
        return "Informational Message";
    case BH_CREDIT:
        return "Account Credited (externally)";
    case BH_DEBIT:
        return "Account Debited";
    case BH_FEE:
        return "Fee Processed";
    case BH_CAUTH:
    default:
	return "Unknown (" + e + ")";
    }
}

string
dump_history(int from, int to)
{
    int    i, sz, last_stamp;
    string result;
    mixed  ***data;

    last_stamp = -1;
    data = query_history(from, to);
    sz = sizeof(data);
    result = "Billing history from " + ctime(from) + " to " +
	     ctime(to) + ":\n";
    for (i = 0; i < sz; i++) {
	int   j, sz_j;
	mixed **entries;

	entries = data[i];
	sz_j = sizeof(entries);
	for (j = 0; j < sz_j; j++) {
	    result +=
		(last_stamp == entries[j][0] ?
		 "                          " :
		 ctime(entries[j][0]) + ": ") +
		pad_right(entries[j][1], 30) + " - " +
		describe_history_event(entries[j][3]) + "\n";
	    last_stamp = entries[j][0];
	}
    }
    return result;
}

void
mark_cc_used(string cc, string expm, string expy)
{
    cc = cc + ";" + expm + ";" + expy;
    cc_used[cc] = time();
}

int
query_cc_used(string cc, string expm, string expy)
{
    cc = cc + ";" + expm + ";" + expy;
    return cc_used[cc] ? cc_used[cc] : 0;
}

void
dump_stats()
{
   int sz;
   string result;

   /* Adding cc_stats results: */
   result =
       "CC charging run:\n"+
       "\n" +
       "Total users:          " + cc_stats["users"] + "\n" +
       "CC charges attempted: " + cc_stats["total"] + "\n" +
       "Successful monthly:   " + cc_stats["success_monthly"] + "\n" +
       "Successful quarterly: " + cc_stats["success_quarterly"] + "\n" +
       "Successful yearly:    " + cc_stats["success_yearly"] + "\n" +
       "Failed monthly:       " + cc_stats["failed_monthly"] + "\n" +
       "Failed quarterly:     " + cc_stats["failed_quarterly"] + "\n" +
       "Failed yearly:        " + cc_stats["failed_yearly"] + "\n";
   sz = map_sizeof(cc_stats["failed_counts"]);
   if (sz) {
      int i, *failures, *totals;

      failures = map_indices(cc_stats["failed_counts"]);
      totals = map_values(cc_stats["failed_counts"]);
      result += "Failure counts:\n";
      for (i = 0; i < sz; i++) {
	  result += "Failure #" + failures[i] + ": " + totals[i] + "\n";
      }
   }
   sz = map_sizeof(cc_stats["failed_reasons"]);
   if (sz) {
      int i, *totals;
      string *reasons;

      reasons = map_indices(cc_stats["failed_reasons"]);
      totals = map_values(cc_stats["failed_reasons"]);
      result += "Failure reasons:\n";
      for (i = 0; i < sz; i++) {
	  result += "Reason " + dump_value(reasons[i]) + ": " + totals[i] + "\n";
      }
   }

   /* Now add other_stats results: */
   result +=
       "\n" +
       "Email notification run:\n" +
       "\n" +
       "Total users:        " + other_stats["users"] + "\n" +
       "Notifications sent: " + other_stats["total"] + "\n" +
       "Users expired:      " + other_stats["expired"] + "\n" +
       "Former CC users:    " + other_stats["last_cc"] + "\n" +
       "Already notified:   " + other_stats["sent"] + "\n" +
       "Not yet due:        " + other_stats["skipped"] + "\n" +
       "No email address:   " + other_stats["impossible"] + "\n" +
       "No-email flag set:  " + other_stats["no-email"] + "\n" +
       "";
   /* Log and email results. */
   DEBUG(result);
   send_message(nil, "UserDB", "monitor@skotos.net",
		([ "harte@skotos.net": "Erwin Harte" ]), ([ ]),
		"UserDB Nightly Billing", nil,
		result +
		"-- \n" +
		"/usr/UserDB/sys/billing.c\n");
}

static
void charge_users(object **nodes, varargs int skipped, int done) {
   object node;

   while (sizeof(nodes) > 0 && sizeof(nodes[0]) == 0) {
       nodes = nodes[1 ..];
   }
   if (sizeof(nodes) == 0) {
       DEBUG("charge_users: Finished; " + done + " done, " + skipped + " done");
       cc_stats["check"] = TRUE;
       if (cc_stats["total"] == cc_stats["done"]) {
	   call_out("start_notify_users", 0);
       }
       return;
   }
   node = nodes[0][0];
   nodes[0] = nodes[0][1 ..];

   /* Check for the next 50 SP if it's due. */
   catch {
       node->check_premium_sp_due();
   }
   catch {
       node->check_credit();
   }
   if (!node->query_next_stamp() || node->query_next_stamp() > time() ||
       !node->query_card()) {
       call_out("charge_users", 0, nodes, skipped + 1, done + 1);
       return;
   }
   call_out("charge_users", 0, nodes, skipped, done + 1);
   cc_stats["total"]++;
   node->charge_card();
}

void
charge_users_callback(object node, int failures, string reason)
{
   cc_stats["done"]++;
   DEBUG("charge_users_callback(" + name(node) + ", " + failures + ", " +
	 dump_value(reason) + ")");
   DEBUG("cc_stats[\"done\"] = " + cc_stats["done"] + "; " +
	 "cc_stats[\"total\"] = " + cc_stats["total"] + "; " +
	 "cc_stats[\"check\"] = " + cc_stats["check"]);

   if (failures) {
      if (node->query_monthly()) {
	 cc_stats["failed_monthly"]++;
      } else if (node->query_quarterly()) {
	 cc_stats["failed_quarterly"]++;
      } else {
	 cc_stats["failed_yearly"]++;
      }
      if (!cc_stats["failed_counts"][failures]) {
	 cc_stats["failed_counts"][failures] = 0;
      }
      cc_stats["failed_counts"][failures]++;
      if (!cc_stats["failed_reasons"][reason]) {
	 cc_stats["failed_reasons"][reason] = 0;
      }
      cc_stats["failed_reasons"][reason]++;
   } else {
      if (node->query_monthly()) {
	 cc_stats["success_monthly"]++;
      } else if (node->query_quarterly()) {
	 cc_stats["success_quarterly"]++;
      } else {
	 cc_stats["success_yearly"]++;
      }
   }
   if (cc_stats["check"] && cc_stats["total"] == cc_stats["done"]) {
      call_out("start_notify_users", 0);
   }
}

void
start_notify_users()
{
   int     i, tot;
   object  **nodes;
   mapping *bits;

   /* create an array of arrays of all relevant user nodes */
   bits = map_values(other_users);
   nodes = allocate(sizeof(bits));

   for (i = 0; i < sizeof(bits); i ++) {
      nodes[i] = map_values(bits[i]);
      tot += map_sizeof(bits[i]);
   }

   other_stats = ([
       "total":      0,   /* How many requests have been sent. */
       "done":       0,   /* How many have been dealt with so far. */
       "users":      tot, /* How many regular (non-cc) users there are. */
       "expired":    0,   /* How many users expired. */
       "last_cc":    0,   /* How many are probably users with cleared cc's */
       "sent":       0,   /* How many have already been sent an email. */
       "skipped":    0,   /* How many are not due for a notification. */
       "impossible": 0,   /* How many didn't have a valid email address. */
       "no-email":   0,   /* How many had a no-email property set. */
   ]);

   SysLog("BILLING: Starting notify_users; " + tot + " users...");
   call_out("notify_users", 0, nodes);
}

static void
notify_users(object **nodes, varargs int skipped, int done)
{
    int    i, mark;
    object node;
    mixed  **log;

    while (sizeof(nodes) > 0 && sizeof(nodes[0]) == 0) {
	nodes = nodes[1 ..];
    }
    if (sizeof(nodes) == 0) {
	DEBUG("BILLING: Finished; " + done + " done, " + skipped + " done");
	other_stats["check"] = TRUE;
	if (other_stats["total"] == other_stats["done"]) {
	    dump_stats();
	    call_out("start_email_ping", 1);
	}
	return;
    }
   node = nodes[0][0];
   nodes[0] = nodes[0][1 ..];

   call_out("notify_users", 0, nodes, skipped, done + 1);

   /* Check for the next 50 SP if it's due. */
   catch {
       node->check_premium_sp_due();
   }

   /* Check if it's time to use some of the remaining credit yet. */
   catch {
       node->check_credit();
   }

   if (node->query_next_stamp() < time()) {
       other_stats["expired"]++;
       remove_other_user(node);
       add_history(time(), node, node->query_name(), BH_EXPIRE);
       node->log_billing("ACCOUNT EXPIRED", BH_EXPIRE);
       return;
   }
   mark = node->query_next_stamp() - 30 * 86400;
   log = node->query_billing_log();
   for (i = sizeof(log) - 1; i >= 0; i--) {
       /*
	* Was the most recent payment a credit card payment, or an external
	* payment (for 12 months, preferably).
	*/
       if (sscanf(log[i][1], "EXTERNAL PAYMENT of $%*d for %*d months") == 2) {
	   /*
	    * Assume that, in some way or another, this means they are
	    * paying annually now, no matter what they did before.
	    */
	   break;
       }
       if (sscanf(log[i][1], "SALE successful%*s using %*s") == 2) {
	   /*
	    * Not interesting, card presumably removed voluntarily or
	    * because it couldn't be charged repeatedly.
	    */
	   other_stats["last_cc"]++;
	   return;
       }
   }
   if (time() > mark) {
       mixed last_email;

       last_email = node->query_property("billing:email");
       if (typeof(last_email) == T_INT && last_email > mark) {
	   /* We already sent this person an email. */
	   other_stats["sent"]++;
	   return;
       }
       if (!node->query_email() || node->query_email() == "email@blanked") {
	   other_stats["impossible"]++;
	   return;
       }
       if (node->query_property("no-email")) {
	   other_stats["no-email"]++;
	   return;
       }
       other_stats["total"]++;
       call_out("notify_user", 0,
		node, node->query_next_stamp(), node->query_email());
       return;
   }
   other_stats["skipped"]++;
}

static void
notify_user(object node, int stamp, string email)
{
    string wday, month, mday, year;
    string message;

    wday = ([ "Sun": "Sunday",
	      "Mon": "Monday",
	      "Tue": "Tuesday",
	      "Wed": "Wednesday",
	      "Thu": "Thursday",
	      "Fri": "Friday",
	      "Sat": "Saturday" ])[ctime(stamp)[..2]];
    month = ([ "Jan": "January",
	       "Feb": "February",
	       "Mar": "March",
	       "Apr": "April",
	       "May": "May",
	       "Jun": "June",
	       "Jul": "July",
	       "Aug": "August",
	       "Sep": "September",
	       "Oct": "October",
	       "Nov": "November",
	       "Dec": "December" ])[ctime(stamp)[4..6]];
    mday = (string)(int)ctime(stamp)[8..9];
    year = ctime(stamp)[20..];
    other_stats["done"]++;

    message = read_file("/usr/UserDB/data/expire.template");
    message = implode(explode("[date]" + message + "[date]", "[date]"),
		      wday + ", " + month + " " + mday + " " + year);
    send_message("ce@skotos.net", /* Envelope sender */
		 "Skotos Customer Experience Team", "ce@skotos.net", /* From */
		 ([ email: "Skotos Customer" ]), /* To */
		 ([ ]), /* Cc */
		 "Your account expires soon!", /* Subject */
		 nil, /* Xtra headers */
		 message /* Actual message body */
		 );
    node->set_property("billing:email", time());
    DEBUG("notify_user(" + dump_value(node) + ") [" + node->query_name() +
	  "; " + ((node->query_next_stamp() - time()) / 86400) + "]");
    if (other_stats["check"] && other_stats["total"] == other_stats["done"]) {
	dump_stats();
	call_out("start_email_ping", 1);
    }
}

void
start_email_ping()
{
    int     i, sz, tot;
    string  *prefs;
    object  **nodes;
    mapping *bits, total;

    total = ([ ]);

    sz = map_sizeof(free_users);
    prefs = map_indices(free_users);
    bits = map_values(free_users);
    for (i = 0; i < sz; i++) {
	total[prefs[i]] = bits[i][..];
    }

    sz = map_sizeof(developer_users);
    prefs = map_indices(developer_users);
    bits = map_values(developer_users);
    for (i = 0; i < sz; i++) {
	if (total[prefs[i]]) {
	    total[prefs[i]] += bits[i][..];
	} else {
	    total[prefs[i]] = bits[i][..];
	}
    }

    sz = map_sizeof(cc_users);
    prefs = map_indices(cc_users);
    bits = map_values(cc_users);
    for (i = 0; i < sz; i++) {
	if (total[prefs[i]]) {
	    total[prefs[i]] += bits[i][..];
	} else {
	    total[prefs[i]] = bits[i][..];
	}
    }

    sz = map_sizeof(other_users);
    prefs = map_indices(other_users);
    bits = map_values(other_users);
    for (i = 0; i < sz; i++) {
	if (total[prefs[i]]) {
	    total[prefs[i]] += bits[i][..];
	} else {
	    total[prefs[i]] = bits[i][..];
	}
    }

    sz = map_sizeof(total);
    bits = map_values(total);
    nodes = allocate(sz);

    for (i = 0; i < sz; i++) {
	nodes[i] = map_values(bits[i]);
	tot += map_sizeof(bits[i]);
    }

    SysLog("BILLING: Starting email_ping: " + tot + " users...");
    call_out("delayed_email_ping", 0, nodes, 0, 0);
}

static void
delayed_email_ping(object **nodes, int skipped, int done)
{
    object node;
    mixed  creation, ack, stamp;

    while (sizeof(nodes) > 0 && sizeof(nodes[0]) == 0) {
	nodes = nodes[1 ..];
    }
    if (sizeof(nodes) == 0 || done - skipped == 25) {
	DEBUG("BILLING: delayed_email_ping finished; " + done + " done, " + skipped + " skipped");
	call_out("start_expire_trials", 1);
	return;
    }
    node = nodes[0][0];
    nodes[0] = nodes[0][1 ..];

    creation = node->query_property("creation_time");
    stamp    = node->query_property("email-ping:stamp");
    ack      = node->query_property("email-ping:ack");

    if (ack && time() < (int)ack + PING_AGE) {
	DEBUG("delayed_email_ping: " + node->query_name() + ": Ack recent.\n");
	skipped++;
    } else if (stamp) {
	DEBUG("delayed_email_ping: " + node->query_name() + ": Awaiting ack.\n");
	skipped++;
    } else if (creation && time() < (int)creation + PING_AGE) {
	DEBUG("delayed_email_ping: " + node->query_name() + ": Created recently.\n");
	skipped++;
    } else if (!node->query_email() || !strlen(node->query_email()) ||
	       lower_case(node->query_email()) == "[unknown]") {
        DEBUG("delayed_email_ping: " + node->query_name() + ": No useful email address available.\n");
	skipped++;
    } else {
	DEBUG("delayed_email_ping: " + node->query_name() + ": Should send ping.\n");
	this_object()->send_email_ping(node);
    }
    call_out("delayed_email_ping", 0, nodes, skipped, done + 1);
}

int
send_email_ping(object user)
{
    string code, message;
    mixed  creation, ack, stamp;

    creation = user->query_property("creation_time");
    stamp    = user->query_property("email-ping:stamp");
    ack      = user->query_property("email-ping:ack");
    if (ack && time() < (int)ack + PING_AGE) {
	/* Has recently enough sent an acknowledgement. */
	return FALSE;
    }
    if (stamp) {
	/* Still awaiting confirmation or email change. */
	return FALSE;
    }
    if (creation && time() < (int)creation + PING_AGE) {
	/*
	 * Was created recently enough that we assume the email confirmation
	 * during creation is still good enough.
	 */
	return FALSE;
    }
    stamp = time();
    code = to_hex(hash_md5(user->query_name() + "#ping#" + stamp));

    user->set_property("email-ping:stamp", stamp);
    user->set_property("email-ping:code",  code);
    user->set_property("email-ping:ack",   nil);

    message = read_file("/usr/UserDB/data/ping.template");
    message = implode(explode(message, "[code]"), code);
    message = implode(explode(message, "[account]"), user->query_name());
    message = implode(explode(message, "[email]"), user->query_email());

    send_message(/* Env  */ "ce@skotos.net",
		 /* From */ "Skotos Customer Experience Team", "ce@skotos.net",
		 /* To   */ ([ user->query_email(): "Skotos Customer" ]),
		 /* Cc   */ ([ ]),
		 /* Subj */ "Email Verification",
		 /* Xtra */ nil,
		 message);

    return TRUE;
}

void
start_expire_trials()
{
    int     i, tot;
    object  **nodes;
    mapping *bits;

    bits = map_values(trial_users);
    nodes = allocate(sizeof(bits));

    for (i = 0; i < sizeof(bits); i++) {
	nodes[i] = map_values(bits[i]);
	tot += map_sizeof(bits[i]);
    }

    DEBUG("BILLING: starting delayed_expire_trials; " + tot + " users...");
    call_out("delayed_expire_trials", 0, nodes, 0, 0);
}

static void
delayed_expire_trials(object **nodes, int expired, int checked)
{
    object node;

    while (sizeof(nodes) > 0 && sizeof(nodes[0]) == 0) {
	nodes = nodes[1 ..];
    }
    if (sizeof(nodes) == 0) {
	DEBUG("delayed_expire_trials: Finished; " + checked + " checked; " + expired + " expired.");
	return;
    }
    node = nodes[0][0];
    nodes[0] = nodes[0][1 ..];

    checked++;
    if (!node->query_card() && node->query_next_stamp() < time()) {
	DEBUG("delayed_expire_trials:" + node->query_name() + ":Expired.");
	remove_trial_user(node);
	expired++;
    }
    call_out("delayed_expire_trials", 0, nodes, expired, checked);
}

void filter_cc_users() {
   mapping *bits;
   object **nodes;
   int i, tot;

   /* create an array of arrays of all user nodes */
   bits = map_values(USERDB->query_users());
   nodes = allocate(sizeof(bits));

   for (i = 0; i < sizeof(bits); i ++) {
      nodes[i] = map_values(bits[i]);
      tot += map_sizeof(bits[i]);
   }
   call_out("delayed_cc_users", 0, nodes);
}

static
void delayed_cc_users(object **nodes, varargs int skipped, int done) {
   object node;

   while (sizeof(nodes) > 0 && sizeof(nodes[0]) == 0) {
       nodes = nodes[1 ..];
   }
   if (sizeof(nodes) == 0) {
       DEBUG("delayed_cc_users: Finished; " + done + " done, " + skipped + " skipped");
       return;
   }
   node = nodes[0][0];
   nodes[0] = nodes[0][1 ..];

   if (node->query_card()) {
      /* DEBUG("delayed_cc_users: Adding " + node->query_name() + "."); */
      add_cc_user(node, TRUE);
   } else {
      remove_cc_user(node, TRUE);
      skipped++;
   }
   call_out("delayed_cc_users", 0, nodes, skipped, done + 1);
}

# if 1
/* DEBUG ONLY */
mapping
query_cc_users()
{
  return cc_users[..];
}

int
query_cc_total()
{
    int i, sz, sum;
    mapping *names;

    sz = map_sizeof(cc_users);
    names = map_values(cc_users);
    sum = 0;
    for (i = 0; i < sz; i++) {
	sum += map_sizeof(names[i]);
    }
    return sum;
}

mapping
query_other_users()
{
    return other_users[..];
}

int
query_other_total()
{
    int i, sz, sum;
    mapping *names;

    sz = map_sizeof(other_users);
    names = map_values(other_users);
    sum = 0;
    for (i = 0; i < sz; i++) {
	sum += map_sizeof(names[i]);
    }
    return sum;
}

void
verify_other_users()
{
  int i, sz_i;
  mapping *maps;

  sz_i = map_sizeof(other_users);
  maps = map_values(other_users);
  for (i = 0; i < sz_i; i++) {
    int j, sz_j;
    object *list;

    sz_j = map_sizeof(maps[i]);
    list = map_values(maps[i]);
    for (j = 0; j < sz_j; j++) {
      if (list[j]->query_card()) {
	DEBUG("verify_other_users:" + list[j]->query_name() + ": Has CC but was in other_users!");
	add_cc_user(list[j]);
	remove_other_user(list[j]);
      }
    }
  }
}

void
verify_cc_users()
{
    int i, sz_i;
    mapping *maps;
    
    sz_i = map_sizeof(cc_users);
    maps = map_values(cc_users);
    for (i = 0; i < sz_i; i++) {
	int j, sz_j;
	object *list;

	sz_j = map_sizeof(maps[i]);
	list = map_values(maps[i]);
	for (j = 0; j < sz_j; j++) {
	    if (!list[j]->query_card()) {
		DEBUG("verify_cc_users:" + list[j]->query_name() + ": Has no CC but is in other_users!");
	    }
	    switch (list[j]->query_account_type()) {
	    case AS_FREE:
	    case AS_DEVELOPER:
	    case AS_STAFF:
		DEBUG("verify_cc_users:" + list[j]->query_name() + ": Free/developer/staff user with CC.");
		break;
	    case AS_TRIAL:
		DEBUG("verify_cc_users:" + list[j]->query_name() + ": Trial user with CC.");
		break;
	    default:
		break;
	    }
	}
    }
}

mapping
query_trial_users()
{
    return trial_users[..];
}

int
query_trial_total()
{
    int i, sz, sum;
    mapping *names;

    sz = map_sizeof(trial_users);
    names = map_values(trial_users);
    sum = 0;
    for (i = 0; i < sz; i++) {
	sum += map_sizeof(names[i]);
    }
    return sum;
}

mapping
query_free_users()
{
    return free_users[..];
}

int
query_free_total()
{
    int i, sz, sum;
    mapping *names;

    sz = map_sizeof(free_users);
    names = map_values(free_users);
    sum = 0;
    for (i = 0; i < sz; i++) {
	sum += map_sizeof(names[i]);
    }
    return sum;
}

mapping
query_developer_users()
{
  return developer_users[..];
}

int
query_developer_total()
{
    int i, sz, sum;
    mapping *names;

    sz = map_sizeof(developer_users);
    names = map_values(developer_users);
    sum = 0;
    for (i = 0; i < sz; i++) {
	sum += map_sizeof(names[i]);
    }
    return sum;
}

mapping
query_staff_users()
{
  return staff_users[..];
}

int
query_staff_total()
{
    int i, sz, sum;
    mapping *names;

    sz = map_sizeof(staff_users);
    names = map_values(staff_users);
    sum = 0;
    for (i = 0; i < sz; i++) {
	sum += map_sizeof(names[i]);
    }
    return sum;
}

mapping
query_premium_users()
{
  return premium_users[..];
}

int
query_premium_total()
{
    int i, sz, sum;
    mapping *names;

    sz = map_sizeof(premium_users);
    names = map_values(premium_users);
    sum = 0;
    for (i = 0; i < sz; i++) {
	sum += map_sizeof(names[i]);
    }
    return sum;
}

# endif

void convert_history() {
   mapping *bits;
   object **nodes;
   int i, tot;

   /* create an array of arrays of all user nodes */
   bits = map_values(USERDB->query_users());
   nodes = allocate(sizeof(bits));

   for (i = 0; i < sizeof(bits); i ++) {
      nodes[i] = map_values(bits[i]);
      tot += map_sizeof(bits[i]);
   }
   call_out("delayed_conversion", 0, nodes, 0);
}

static
void delayed_conversion(object **nodes, int done) {
   int    sz;
   object node;
   mixed  **log;

   while (sizeof(nodes) > 0 && sizeof(nodes[0]) == 0) {
       nodes = nodes[1 ..];
   }
   if (sizeof(nodes) == 0) {
       DEBUG("delayed_conversion: Finished; " + done + " done.");
       return;
   }

   node = nodes[0][0];
   nodes[0] = nodes[0][1 ..];

   node->patch_billing_log();

   done++;
   if (done % 1000 == 0) {
       DEBUG("delayed_conversion: " + done + " done so far.");
   }

   call_out("delayed_conversion", 0, nodes, done);
}

# define MARK_2002_AUG_01 1028185200
# define MARK_2002_OCT_01 1033455600
# define MARK_2002_NOV_01 1036137600

void
start_emailout()
{
    int     i, sz;
    object  **nodes;
    mapping *bits;

    bits = map_values(USERDB->query_users());
    sz = sizeof(bits);
    nodes = allocate(sz);
    for (i = 0; i < sz; i++) {
	nodes[i] = map_values(bits[i]);
    }
    call_out("delayed_emailout", 0, nodes, 0, 0);
}

static void
do_emailout(object node)
{
    string text;

    text = read_file("~/data/templates/20020929-active.tmpl");
    send_message(/* Env  */ "ce@skotos.net",
		 /* From */ "Skotos Customer Experience", "ce@skotos.net",
		 /* To   */ ([ node->query_email(): "Skotos Customer" ]),
		 /* Cc   */ ([ ]),
		 /* Subj */ "Skotos: Reminder",
		 /* Xtra */ nil,
		 text);
}

static void
delayed_emailout(object **nodes, int done, int found)
{
    object node;

    while (sizeof(nodes) > 0 && sizeof(nodes[0]) == 0) {
	nodes = nodes[1 ..];
    }
    if (sizeof(nodes) == 0) {
	DEBUG("delayed_emailout: DONE!  " + done + " checked; " + found + " found.");
	return;
    }
    node = nodes[0][0];
    nodes[0] = nodes[0][1 ..];

    if ((node->query_next_stamp() >= time() || node->query_card()) &&
	node->query_property("creation_time") &&
	node->query_property("creation_time") >= MARK_2002_AUG_01) {
	DEBUG("delayed_emailout: Found " + node->query_name());
	do_emailout(node);
	found++;
    }
    done++;

    if (done % 1000 == 0) {
	DEBUG("delayed_emailout: " + done + " checked so far; " + found + " found so far.");
    }
    call_out("delayed_emailout", 0.1, nodes, done, found);
}

void
schedule_grandfather_event()
{
    call_out("grandfather_event", MARK_2002_NOV_01 - time());
}

static void
grandfather_event()
{
    int     i, sz_i;
    mapping *bits;
    string  result;

    result = "";

    result += "Staff accounts:\n";
    /* Select the group with credit cards. */
    bits = map_values(staff_users);
    sz_i = map_sizeof(staff_users);
    for (i = 0; i < sz_i; i++) {
	int     j, sz_j;
	object  *nodes;
	mapping map;

	map   = bits[i];
	sz_j  = map_sizeof(map);
	nodes = map_values(map);
	for (j = 0; j < sz_j; j++) {
	    if (nodes[j]->query_account_status(AS_GRAND) == AS_GRAND) {
		continue;
	    }
	    result += nodes[j]->query_name() + "\n";
	    nodes[j]->set_account_status(AS_GRAND);
	}
    }

    result += "\nFree accounts:\n";
    /* Select the group with free accounts. */
    bits = map_values(free_users);
    sz_i = map_sizeof(free_users);
    for (i = 0; i < sz_i; i++) {
	int     j, sz_j;
	object  *nodes;
	mapping map;

	map   = bits[i];
	sz_j  = map_sizeof(map);
	nodes = map_values(map);
	for (j = 0; j < sz_j; j++) {
	    if (nodes[j]->query_account_status(AS_GRAND) == AS_GRAND) {
		continue;
	    }
	    result += nodes[j]->query_name() + "\n";
	    nodes[j]->set_account_status(AS_GRAND);
	}
    }

    result += "\nDeveloper accounts:\n";
    /* Select the group with developer accounts. */
    bits = map_values(developer_users);
    sz_i = map_sizeof(developer_users);
    for (i = 0; i < sz_i; i++) {
	int     j, sz_j;
	object  *nodes;
	mapping map;

	map   = bits[i];
	sz_j  = map_sizeof(map);
	nodes = map_values(map);
	for (j = 0; j < sz_j; j++) {
	    if (nodes[j]->query_account_status(AS_GRAND) == AS_GRAND) {
		continue;
	    }
	    result += nodes[j]->query_name() + "\n";
	    nodes[j]->set_account_status(AS_GRAND);
	}
    }

    result += "\nUsers with authenticated credit cards:\n";
    /* Select the group with credit cards. */
    bits = map_values(cc_users);
    sz_i = map_sizeof(cc_users);
    for (i = 0; i < sz_i; i++) {
	int     j, sz_j;
	object  *nodes;
	mapping map;

	map   = bits[i];
	sz_j  = map_sizeof(map);
	nodes = map_values(map);
	for (j = 0; j < sz_j; j++) {
	    if (nodes[j]->query_account_status(AS_GRAND) == AS_GRAND) {
		continue;
	    }
	    result += nodes[j]->query_name() + "\n";
	    nodes[j]->set_account_status(AS_GRAND);
	}
    }

    result += "\nUsers that are otherwise paid up:\n";
    /* Select the group that is otherwise paid up. */
    bits = map_values(other_users);
    sz_i = map_sizeof(other_users);
    for (i = 0; i < sz_i; i++) {
	int     j, sz_j;
	object  *nodes;
	mapping map;

	map   = bits[i];
	sz_j  = map_sizeof(map);
	nodes = map_values(map);
	for (j = 0; j < sz_j; j++) {
	    if (nodes[j]->query_account_status(AS_GRAND) == AS_GRAND) {
		continue;
	    }
	    result += nodes[j]->query_name() + "\n";
	    nodes[j]->set_account_status(AS_GRAND);
	}
    }
    send_message(/* Env  */ nil,
		 /* From */ "UserDB", "monitor@skotos.net",
		 /* To   */ ([ "harte@skotos.net": "Erwin Harte" ]),
		 /* Cc   */ ([ ]),
		 /* Subj */ "UserDB: Grandfathering accounts",
		 /* Xtra */ nil,
		 result +
		 "-- \n" +
		 "/usr/UserDB/sys/billing.c -> grandfather_event()\n");
}

float
query_billing_fee(string type, string period)
{
    switch (type) {
    case "basic":
	switch (period) {
	case "month":
	case "monthly":
	    return 12.95;
	case "quarter":
	case "quarterly":
	    return 34.95;
	case "year":
	case "yearly":
	    return 129.95;
	default:
	    error("Invalid billing period");
	}
    case "premium":
	switch (period) {
	case "month":
	case "monthly":
	    return 29.95;
	case "quarter":
	case "quarterly":
	    return 79.95;
	case "year":
	case "yearly":
	    return 299.95;
	default:
	    error("Invalid billing period");
	}
    default:
	error("Invalid account type");
    }
}

void
find_trials()
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
    call_out("delayed_find_trials", 1, users - ({ nil }), 0, 0);
}

static void
delayed_find_trials(object **nodes, int total, int found)
{
    object node;

    while (sizeof(nodes) > 0 && sizeof(nodes[0]) == 0) {
	nodes = nodes[1 ..];
    }
    if (sizeof(nodes) == 0) {
	DEBUG("delayed_find_trials: DONE!  " + total + " checked; " + found + " found.");
	return;
    }
    node = nodes[0][0];
    nodes[0] = nodes[0][1 ..];

    if (node->query_account_type() == AS_TRIAL &&
	(node->query_next_stamp() >= time() || node->query_card())) {
	add_trial_user(node);
	found++;
    }
    total++;

    if (total % 1000 == 0) {
	DEBUG("delayed_find_trials: " + total + " checked so far; " + found + " found so far.");
    }
    call_out("delayed_find_trials", 0.1, nodes, total, found);
}
