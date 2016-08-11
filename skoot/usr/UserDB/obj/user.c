/*
**	~UserDB/obj/user.c
**
**	Maintain data associated with a single user.
**
**	Copyright Skotos Tech Inc 1999-2002
*/

# include <type.h>
# include <UserDB.h>
# include <SkotOS.h>

# define CHALLENGE_LIFETIME		(3600 * 24)
# define KEYCODE_LIFETIME		(3600 * 24)

# define SERVICE_BASIC_MONTHLY		1
# define SERVICE_BASIC_QUARTERLY	2
# define SERVICE_BASIC_YEARLY           3

# define MAX_FAILURES			3

/* Marking the start of the new subscription prices. */
# define MARK_2002_10_01		1033455600
# define MARK_2002_11_01                1036137600

private inherit "/lib/url";
private inherit "/lib/date";
private inherit "/lib/string";

inherit properties "/lib/properties";
inherit billapi "~UserDB/lib/billapi";
inherit smtpapi "~SMTP/lib/smtp_api";

string	name;
string	password;
string	challenge;
string	keycode;
string	email;

int	keycode_stamp;
int	challenge_stamp;

int	pay_day;		/* from 1 to 28, billing day of month */
int	next_month;		/* when do we come due -- month [1 - 12] */
int	next_year;		/* when do we come due -- year [2001 - ] */
int	next_stamp;		/* which second do we come due? */

int	trial;			/* are we in the trial month? */
int     account_status;         /* What sort of account is this and what state
				 * is it in?
				 */

string	auth_order_id;          /* What OID did we get for the PREAUTH */
float   auth_order_amount;      /* How much was the amount of the PREAUTH */

string	cc_num;
string	cc_expm;
string	cc_expy;

int	service;

int	sale_failures;
string	sale_failure;

mapping interactive_failures;	/* How many PREAUTHs failed and/or upgrades to premium failed. */

mapping game_stats;             /* Track some game-statistics */
int     banned;                 /* Is this user (temporarily) banned? */

mixed   **billing_log;
mixed   *last_payment;          /* ({ timestamp, amount, expire }) */
mixed   *upgrade;               /* ({ service_type, timestamp }) */

int     *premium_sp_due;
float   account_credit;
mixed   *next_payment;          /* ({ account_type, account_period }) */
string  billing_flag;           /* TRUE if a SALE/PREAUTH or so is in progress, FALSE otherwise. */

string describe_card();
void   log_billing(string str, varargs int type, mixed extra...);
void   clear_card();

nomask string query_state_root() { return "UserDB:User"; }

static
void create(int clone) {
   if (clone) {
      properties::create();
      billapi::create();

      set_property("creation_time", time());
      set_property("b_same", TRUE);

      billing_log = ({ });
      service = SERVICE_BASIC_MONTHLY;

      game_stats = ([ ]);
   }
}

atomic
void set_name(string str) {
   name = str;
   USERDB->set_name(str, this_object());
   ::set_object_name("UserDB:" + name[.. 0] + ":" + name[.. 1] + ":" + name);
   log_billing("CREATED", BH_CREATED);
}

atomic
void rename_user(string str) {
	name = str;
	::set_object_name("UserDB:" + name[.. 0] + ":" + name[.. 1] + ":" + name);
}

int empty(mixed val) {
   switch(typeof(val)) {
   case T_NIL:
      return TRUE;
   case T_STRING:
      return !strlen(val);
   case T_ARRAY:
      return !sizeof(val);
   case T_MAPPING:
      return !map_sizeof(val);
   default:
      return !val;
   }
}

mixed query_billing_property(string ix) {
   if (!empty(query_property("b_same"))) {
      return query_property(ix);
   }
   return query_property("b_" + ix);
}

void set_object_name(string name) {
   error("userdb nodes cannot be renamed");
}

string query_name() {
   return name;
}

void set_password(string str) {
   password = str;
}

/* consider this one carefully */
string query_password() {
   return password;
}

int test_password(string pass) {
   return lower_case(pass) == lower_case(password);
}

int test_hash(string hash) {
   string h;

   /* TODO: Temporary Hack */
   h = to_hex(hash_md5(name + keycode + "NONE"));
   SysLog("hex of " + name + keycode + "NONE == [" + h + "]");
   SysLog("testing against [" + hash + "]");
   if (h == hash) {
      return TRUE;
   }
   h = to_hex(hash_md5(name + password + "NONE"));
   SysLog("FALLBACK: hex is [" + h + "]");
   return h == hash;

   /* this is what it really should do */
   if (challenge) {
      h = to_hex(hash_md5(name + challenge + password));
      SysLog("FALLBACK: hex is [" + h + "]");
      return h == hash;
   }
}

string get_keycode() {
   if (!keycode || time() - keycode_stamp > KEYCODE_LIFETIME) {
      keycode = (string) ((time() >> 1) + random(1<<29));
      keycode_stamp = time();
   }
   return keycode;
}

void invalidate_keycode() {
   keycode_stamp = 0;
}

string get_challenge() {
   if (!challenge || time() - challenge_stamp > CHALLENGE_LIFETIME) {
      challenge = (string) ((time() >> 1) + random(1<<29));
      challenge_stamp = time();
   }
   return challenge;
}

void invalidate_challenge() {
   challenge_stamp = 0;
}

/* For debugging only: */
atomic void clear_email() {
    USERDB->clear_email(email, this_object());
    catch {
	SEARCHDB->update_user(this_object(), "email", email, nil);
    }
    email = nil;
}

atomic void set_email(string mail) {
   if (!mail) {
       return;
   }
   if (email && strlen(email)) {
       USERDB->clear_email(email, this_object());
   }
   catch {
       SEARCHDB->update_user(this_object(), "email", email, mail);
   }
   email = mail;
   USERDB->set_email(email, this_object());
   if (email) {
      clear_property("email-ping:stamp");
      clear_property("email-ping:code");
      set_property("email-ping:ack", time());
   }
}

string query_email() {
   if (email && email != lower_case(email)) {
      set_email(lower_case(email));
   }
   return email;
}

void set_account_type(int t) {
    int orig_status;

    orig_status = account_status;
    account_status = (account_status & ~AS_TYPE_MASK) | (t & AS_TYPE_MASK);
    if (orig_status != account_status) {
	switch (orig_status & AS_TYPE_MASK) {
	case AS_REGULAR:
	    BILLINGD->remove_other_user(this_object());
	    break;
	case AS_TRIAL:
	    BILLINGD->remove_trial_user(this_object());
	    break;
	case AS_FREE:
	    BILLINGD->remove_free_user(this_object());
	    break;
	case AS_DEVELOPER:
	    BILLINGD->remove_developer_user(this_object());
	    break;
	case AS_STAFF:
	    BILLINGD->remove_staff_user(this_object());
	    break;
	}
    }
    switch (account_status & AS_TYPE_MASK) {
    case AS_REGULAR:
	BILLINGD->add_other_user(this_object());
	break;
    case AS_TRIAL:
	BILLINGD->add_trial_user(this_object());
	break;
    case AS_FREE:
	BILLINGD->add_free_user(this_object());
	break;
    case AS_DEVELOPER:
	BILLINGD->add_developer_user(this_object());
	break;
    case AS_STAFF:
	BILLINGD->add_staff_user(this_object());
	break;
    }
}

int query_account_type() {
    return account_status & AS_TYPE_MASK;
}

string describe_account_type() {
    switch (account_status & AS_TYPE_MASK) {
    case AS_REGULAR:
        return "regular";
    case AS_TRIAL:
        return "trial";
    case AS_FREE:
        return "free";
    case AS_DEVELOPER:
        return "developer";
    case AS_STAFF:
        return "staff";
    default:
        return "unknown(" + (account_status & AS_TYPE_MASK) + ")";
    }
}

void set_account_status(int t) {
    t &= ~AS_TYPE_MASK;
    account_status |= t;
    if (IS_PREMIUM(account_status)) {
	BILLINGD->add_premium_user(this_object());
    } else {
	BILLINGD->remove_premium_user(this_object());
    }
}

void clear_account_status(int t) {
    t &= ~AS_TYPE_MASK;
    account_status &= ~t;
    if (IS_PREMIUM(account_status)) {
	BILLINGD->add_premium_user(this_object());
    } else {
	BILLINGD->remove_premium_user(this_object());
    }
}

int query_account_status(varargs int t) {
    if (t) {
        t &= ~AS_TYPE_MASK;
    } else {
        t = ~AS_TYPE_MASK;
    }
    return account_status & t;
}

string *describe_account_status() {
    string *bits;

    bits = ({
      IS_BANNED(account_status)   ? "banned"   : nil,
      IS_DELETED(account_status)  ? "deleted"  : nil,
      IS_NO_EMAIL(account_status) ? "no-email" : nil,
      IS_PREMIUM(account_status)  ? "premium"  : nil,
      IS_GRAND(account_status)    ? "grand"    : nil,
    });
    return bits - ({ nil });
}

# define BUCKET_SIZE 86400

mapping convert_played_times(int *arr) {
    int i, sz;
    mapping map;

    map = ([ ]);
    for (i = 0, sz = sizeof(arr); i < sz; i += 2) {
        int bucket, *list;

	bucket = arr[i] / BUCKET_SIZE;
	list = map[bucket];
	if (!list) {
	    map[bucket] = list = ({ });
	}
	map[bucket] = list + arr[i .. i + 1];
    }
    return map;
}

int
calculate_playing_time(string game, int stamp_b, int stamp_e)
{
    int     *list, i, sz, seconds, b_i, b_sz, **b_lists;
    mapping map, buckets;

    if (!game_stats || !(map = game_stats[game])) {
	return 0;
    }
    switch (typeof(map["played-times"])) {
    case T_NIL:
        return 0;
    case T_ARRAY:
        /*
         * Conversion time!
         */
        map["played-times"] = convert_played_times(map["played-times"]);
        break;
    case T_MAPPING:
        break;
    }
    buckets = map["played-times"];
    b_sz = map_sizeof(buckets);
    b_lists = map_values(buckets);
    for (b_i = 0; b_i < b_sz; b_i++) {
	list = b_lists[b_i];
	sz = sizeof(list);
	for (i = 0; i < sz; i += 2) {
	    int time_b, time_e, current_day;

	    time_b = list[i];
	    time_e = list[i] + list[i + 1];

	    /*
	     * Slice out the relevant chunk, in case there is only a partial
	     * overlap with the request time-slice.
	     */
	    if (stamp_b <= time_b && time_b <= stamp_e) {
		if (time_e > stamp_e) {
		    time_e = stamp_e;
		}
	    } else if (stamp_b <= time_e && time_e <= stamp_e) {
		if (time_b < stamp_b) {
		    time_b = stamp_b;
		}
	    } else if (time_b < stamp_b && stamp_e < time_e) {
		time_b = stamp_b;
		time_e = stamp_e;
	    } else {
		continue;
	    }
	    seconds += time_e - time_b;
	}
    }
    return seconds;
}

/*
 * Assumption: stamp_b and stamp_e are both in the same year/month.
 * Return ({ time, frequency }) for this game.
 */
int *
summary_game_stats(string game, int stamp_b, int stamp_e)
{
    int b_i, b_sz, **b_lists;
    mapping map;
    mapping buckets;
    mixed   list;
    int i, sz, seconds;
    mapping days;

    if (!game_stats || !(map = game_stats[game])) {
	return ({ 0, 0  });
    }
    list = map["played-times"];
    switch (typeof(list)) {
    case T_NIL:
      return ({ 0, 0 });
    case T_ARRAY:
      /*
       * Conversion time!
       */
      map["played-times"] = convert_played_times(map["played-times"]);
      break;
    case T_MAPPING:
      break;
    }
    days = ([ ]);

    buckets = map["played-times"];
    b_sz = map_sizeof(buckets);
    b_lists = map_values(buckets);
    for (b_i = 0; b_i < b_sz; b_i++) {
        list = b_lists[b_i];
	sz = sizeof(list);
	for (i = 0; i < sz; i += 2) {
	    int time_b, time_e, current_day;

	    time_b = list[i];
	    time_e = list[i] + list[i + 1];

	    /*
	     * Slice out the relevant chunk, in case there is only a partial
	     * overlap with the request time-slice.
	     */
	    if (stamp_b <= time_b && time_b <= stamp_e) {
		if (time_e > stamp_e) {
		    time_e = stamp_e;
		}
	    } else if (stamp_b <= time_e && time_e <= stamp_e) {
		if (time_b < stamp_b) {
		    time_b = stamp_b;
		}
	    } else if (time_b < stamp_b && stamp_e < time_e) {
		/* Riiiight, someone online for over a month? */
		time_b = stamp_b;
		time_e = stamp_e;
	    } else {
		continue;
	    }
	    seconds += time_e - time_b;
	    current_day = get_day(time_b);
	    while (time_b < time_e) {
		days[current_day] = TRUE;
		time_b += 86400;
		current_day++;
	    }
	}
    }
    if (seconds) {
        return ({ (seconds + 3600 - 1) / 3600, map_sizeof(days) });
    }
    return ({ 0, 0 });
}

int *
query_game_history(string game, int year, int month)
{
    int when;

    when = year * 12 + month;
    if (game_stats) {
	mapping map;

	map = game_stats[game];
	if (map) {
	    map = map["history"];
	    if (map) {
		int *data;

		data = map[when];
		if (data) {
		    return data[..];
		}
	    }
	}
    }
    return ({ 0, 0 });
}

void clear_game_history(string game, int year, int month) {
    int when;

    when = year * 12 + month;
    if (game_stats && game_stats[game] && game_stats[game]["history"]) {
	game_stats[game]["history"][when] = nil;
    }
}

void store_game_history(string game, int year, int month, int *data) {
    int when;

    when = year * 12 + month;

    if (game_stats) {
	if (game_stats[game]) {
	    if (game_stats[game]["history"]) {
		game_stats[game]["history"][when] = data;
	    } else {
		game_stats[game]["history"] = ([ when: data ]);
	    }
	} else {
	    game_stats[game] = ([ "history": ([ when: data ]) ]);
	}
    } else {
	game_stats = ([ game: ([ "history": ([ when: data ]) ]) ]);
    }
}

void ccreated(string remote_host, string when) {
    mapping map;

    if (!game_stats) {
	game_stats = ([ ]);
    }
    if (!(map = game_stats[remote_host])) {
	game_stats[remote_host] = map = ([ ]);
    }
    if (map["created-total"] == nil) {
	map["created-total"] = 0;
    }
    if (map["created-times"] == nil) {
	map["created-times"] = ({ });
    }
    if (sizeof(map["created-times"]) > 0) {
        /* For back-patching data, ignore data older than already present. */
        if ((int)when <= map["created-times"][0]) {
	    return;
        }
    } else if ((int)when == 0) {
        when = query_property("creation_time");
    }
    map["created-total"]++;
    map["created-times"] += ({ (int)when });
    DEBUG(name + ": Created character on \"" + remote_host + "\" on " +
	  ctime((int)when));
}

void cplayed(string remote_host, string when, string duration) {
    int bucket;
    mapping map;

    if (!game_stats) {
	game_stats = ([ ]);
    }
    if (!(map = game_stats[remote_host])) {
	game_stats[remote_host] = map = ([ ]);
    }
    if (map["played-total"] == nil) {
	map["played-total"] = 0;
    }
    if (map["played-times"] == nil) {
	map["played-times"] = ([ ]);
    } else if (typeof(map["played-times"]) == T_ARRAY) {
        /*
	 * Conversion time!
	 */
        map["played-times"] = convert_played_times(map["played-times"]);
    }
    if (map_sizeof(map["played-times"]) > 0) {
        mixed *list;

	list = map_values(map["played-times"]);
	list = list[sizeof(list) - 1];
        /* For back-patching data, ignore data older than already present. */
	if ((int)when <= list[sizeof(list) - 2] + list[sizeof(list) - 1]) {
	    return;
	}
    }
    map["played-total"]++;
    bucket = (int)when / BUCKET_SIZE;
    if (map["played-times"][bucket]) {
        map["played-times"][bucket] += ({ (int)when, (int)duration });
    } else {
        map["played-times"][bucket] = ({ (int)when, (int)duration });
    }
    DEBUG(name + ": Played on \"" + remote_host + "\" starting on " +
	  ctime((int)when) + " for " + (int)duration + " seconds.");
}


mapping query_game_stats_map() {
    /*
     * Damn, gotta convert here too.
     */
    if (game_stats) {
        int i, sz;
	string *games;

	sz = map_sizeof(game_stats);
	games = map_indices(game_stats);
	for (i = 0; i < sz; i++) {
	    if (typeof(game_stats[games[i]]["played-times"]) == T_ARRAY) {
	        game_stats[games[i]]["played-times"] = convert_played_times(game_stats[games[i]]["played-times"]);
	    }
	}
    }
    return game_stats;
}

mapping query_game_stats_slice(int from, int to) {
    int     i, sz_i;
    string  *games;
    mapping map, result;
    mixed   *data;

    map = query_game_stats_map();
    result = ([ ]);
    sz_i = map_sizeof(map);
    games = map_indices(map);
    data = map_values(map);
    for (i = 0; i < sz_i; i++) {
	if (data[i]["played-times"]) {
	    mapping sub;

	    result[games[i]] = sub = data[i]["played-times"][from/86400..to/86400];
	    if (!map_sizeof(sub)) {
		result[games[i]] = nil;
	    }
	}
    }
    return result;
}

mixed query_game_stats(varargs string from, string field) {
    if (!from || !field) {
	return game_stats ? map_indices(game_stats) : ({ });
    }
    if (game_stats && game_stats[from] && game_stats[from][field]) {
	return game_stats[from][field];
    }
    return nil;
}

int query_trial_until() {
    return IS_TRIAL(account_status) ? next_stamp : 0;
}

int query_paid_until() {
    switch (query_account_type()) {
    case AS_TRIAL:
	return 0;
    case AS_FREE:
    case AS_DEVELOPER:
    case AS_STAFF:
	return time() + 30 * 86400;
    default:
	return next_stamp;
    }
}

string
query_billing_flag() {
    return billing_flag;
}

static void
set_billing_flag(string text) {
    if (billing_flag) {
	error("There is already a billing operation in progress");
    }
    DEBUG("set_billing_flag:" + name + ":Flag claimed. (" + text + ")");
    billing_flag = text;
}

static void
clear_billing_flag() {
    if (!billing_flag) {
	DEBUG("clear_billing_flag:" + name + ":Flag cleared twice? (" + billing_flag + ")");
    } else {
	DEBUG("clear_billing_flag:" + name + ":Flag properly cleared. (" + billing_flag + ")");
    }
    billing_flag = nil;
}

mixed **query_billing_log() {
    return billing_log;	/* should copy */
}

#if 1
void clear_billing_entry() {
    if (sizeof(billing_log) > 0) {
	billing_log = billing_log[..sizeof(billing_log) - 2];
    }
}
#endif

string describe_billing_log(varargs int flag) {
   string *lines;
   int i;

   lines = allocate(sizeof(billing_log));

   for (i = 0; i < sizeof(billing_log); i ++) {
      if (flag) {
	 string str;

	 if (sscanf(billing_log[i][1], "PREAUTH with OID %*s for card %s", str)) {
	    lines[i] = ctime(billing_log[i][0]) + " AUTHENTICATED CARD " + str;
	    continue;
	 }
	 if (sscanf(billing_log[i][1], "FAILED PREAUTH for %s", str)) {
	    lines[i] = ctime(billing_log[i][0]) + " CARD AUTHENTICATION FAILED for " + str;
	    continue;
	 }
      }
      lines[i] = ctime(billing_log[i][0]) + " " + billing_log[i][1];
   }
   return implode(lines, "\n") + "\n";
}

void log_billing(string str, varargs int type, mixed extra...) {
    if (extra) {
	billing_log += ({ ({ time(), str, type }) + extra });
    } else {
	billing_log += ({ ({ time(), str }) });
    }
}

mixed *query_last_payment() {
    return last_payment ? last_payment[..] : nil;
}

void last_payment() {
    int i, sz;

    if (!IS_REGULAR(account_status)) {
	DEBUG("last_payment:" + name + ":Not a paying account.");
	last_payment = nil;
	return;
    }
    if (!this_object()->query_card() && next_stamp < time()) {
	/* Backward compatibility irrelevant for inactive accounts. */
	DEBUG("last_payment:" + name + ":Not an active account.");
	last_payment = nil;
	return;
    }
    sz = sizeof(billing_log);
    for (i = sz - 1; i >= 0; i--) {
	switch (billing_log[i][2]) {
	case BH_CHARGE:
	    if (i < sz - 1 && billing_log[i + 1][2] == BH_BUMPED) {
		switch (billing_log[i + 1][3]) {
		case  1:
		    if (billing_log[i][0] >= MARK_2002_10_01) {
			last_payment = ({ billing_log[i][0],  12.95, next_stamp });
		    } else {
			last_payment = ({ billing_log[i][0],   9.95, next_stamp });
		    }
		    return;
		case  3:
		    if (billing_log[i][0] >= MARK_2002_10_01) {
			last_payment = ({ billing_log[i][0],  34.95, next_stamp });
		    } else {
			last_payment = ({ billing_log[i][0],  25.00, next_stamp });
		    }
		    return;
		case 12:
		    if (billing_log[i][0] >= MARK_2002_10_01) {
			last_payment = ({ billing_log[i][0], 129.95, next_stamp });
		    } else {
			last_payment = ({ billing_log[i][0], 100.00, next_stamp });
		    }
		    return;
		default:
		    DEBUG("last_payment:" + name + ":No appropriate 'bumped' event found.");
		    return;
		}
	    } else {
		DEBUG("last_payment:" + name + ":No 'bumped' event found.");
		return;
	    }
	    return;

	case BH_EXTERNAL:
	    last_payment = ({ billing_log[i][0], billing_log[i][3], next_stamp });
	    return;
	}
    }
    DEBUG("last_payment:" + name + ":No appropriate events found.");
}

mixed *query_upgrade() {
    return upgrade ? upgrade[..] : nil;
}

void set_upgrade(string service_type) {
    int i, sz;

    /*
     * If this user is currently a paying customer, check what the last 
     * payment was that was done and when, so that we can calculate a
     * reimbursement value when the nightly billing cycle comes along.
     */
    switch (service_type) {
    case "month":
	upgrade = ({ SERVICE_BASIC_MONTHLY, time() });
	break;
    case "quarter":
	upgrade = ({ SERVICE_BASIC_QUARTERLY, time() });
	break;
    case "year":
	upgrade = ({ SERVICE_BASIC_YEARLY, time() });
	break;
    default:
	error("set_upgrade() service type not recognized");
    }
}

void set_premium_sp_due(int months) {
    int i, y, m, d;

    if (!months) {
	premium_sp_due = nil;
	return;
    }
    y = next_year;
    m = next_month;
    d = pay_day;
    premium_sp_due = allocate(months);

    for (i = months - 1; i > 0; i--) {
	/* Go back a month. */
	m--;
	if (m == 0) {
	    y--;
	    m = 12;
	}
	premium_sp_due[i] = calculate_stamp(y, m, d, time());
    }
    /* And the first 50 SP immediately. */
    premium_sp_due[0] = 0;
    DEBUG("timestamps: " + dump_value(premium_sp_due));
}

atomic int check_premium_sp_due() {
    int   i, t;
    mixed **log;

    if (!premium_sp_due) {
	return FALSE;
    }
    if (!IS_PREMIUM(account_status)) {
	DEBUG("check_premium_sp_due:" + name + ":No longer a premium account, clearing premium_sp_due " + dump_value(premium_sp_due));
	premium_sp_due = nil;
	return FALSE;
    }
    if (!sizeof(premium_sp_due)) {
	premium_sp_due = nil;
	return FALSE;
    }
    if (time() < premium_sp_due[0]) {
	return FALSE;
    }
    if (sizeof(premium_sp_due) == 1) {
	premium_sp_due = nil;
    } else {
	premium_sp_due = premium_sp_due[1..];
    }
    /*
     * Alrighty, that means we need to hand out another 50SP for this
     * premium account.
     */
    log = query_property("sps:log");
    if (!log) {
	log = ({ });
    }
    log += ({ ({
	t = time(),
	"Premium Account",
	50,
	"Skotos Customer Experience",
	nil,
	TRUE,
	nil,
    }) });
    set_property("sps:log", log);
    POINTSDB->add_entry(t, 50, "Premium Account", nil,
			"Skotos Customer Experience", this_object(), name);
    DEBUG("check_premium_sp_due:" + name + ":Handed out 50 SP.");
    return TRUE;
}

int query_monthly() {
    return service == SERVICE_BASIC_MONTHLY;
}

int query_quarterly() {
    return service == SERVICE_BASIC_QUARTERLY;
}

int query_yearly() {
    return service == SERVICE_BASIC_YEARLY;
}

/*
 * Old cost (before MARK_2002_10_01) was:
 * Monthly:     9.95
 * Quarterly:  25.00
 * Yearly:    100.00
 *
 * New cost (after MARK_2002_10_01) is:
 *            Basic   Premium
 * Monthly:    12.95    29.95
 * Quarterly:  34.95    79.95
 * Yearly:    129.95   299.95
 */

float query_cost() {
   switch(service) {
   case SERVICE_BASIC_MONTHLY:
      return BILLINGD->query_billing_fee(IS_PREMIUM(account_status) ? "premium" : "basic", "month");
   case SERVICE_BASIC_QUARTERLY:
      return BILLINGD->query_billing_fee(IS_PREMIUM(account_status) ? "premium" : "basic", "quarter");
   case SERVICE_BASIC_YEARLY:
      return BILLINGD->query_billing_fee(IS_PREMIUM(account_status) ? "premium" : "basic", "year");
   }
   error("bad service");
}

float query_monthly_cost() {
    return BILLINGD->query_billing_fee(IS_PREMIUM(account_status) ? "premium" : "basic", "month");
}

float query_quarterly_cost() {
    return BILLINGD->query_billing_fee(IS_PREMIUM(account_status) ? "premium" : "basic", "quarter");
}

float query_yearly_cost() {
    return BILLINGD->query_billing_fee(IS_PREMIUM(account_status) ? "premium" : "basic", "year");
}

/* high level calls */

atomic void prod_due_date();


/*
 * Keep track of failures while trying to PREAUTH a card or upgrade an account
 * from basic to premium.
 */
void add_interactive_failure(string cc, string expm, string expy, string reason) {
    int     i, sz;
    string  text, *cards;
    mapping *maps;

    /*
     * Add the latest failure to the data-structure.
     */
    if (!interactive_failures) {
	interactive_failures = ([ ]);
    }
    if (!interactive_failures[cc]) {
	interactive_failures[cc] = ([ ]);
    }
    if (!interactive_failures[cc][reason]) {
	interactive_failures[cc][reason] = ([ ]);
    }
    interactive_failures[cc][reason][time()] = TRUE;

    DEBUG("add_interactive_failure:" + name + ":" + dump_value(interactive_failures));

    /* Now check if we've reached the limit. */
    if (map_sizeof(interactive_failures) < MAX_FAILURES) {
	return;
    }

    /* Ban account. */
    USERDB->add_user_ban(name, "UserDB", "Failed Preauths");

    /* Send email to CE about this. */
    text =
	"The user \"" + name + "\" has been banned after unsuccessfully\n" +
	"authenticating/upgrading with the following cards:\n\n";
    sz    = map_sizeof(interactive_failures);
    cards = map_indices(interactive_failures);
    maps  = map_values(interactive_failures);
    for (i = 0; i < sz; i++) {
	int     j, sz_j;
	string  *uses;
	mapping *times;

	if (sscanf(cards[i], "%s;%*s;%*s", cc) < 3) {
	    cc = cards[i];
	}
	while (strlen(cc) < 16) {
	    cc += "?";
	}
	cc = cc[.. 3] + "********" + cc[12 .. 15];
	text += cc + ":\n";
	sz_j = map_sizeof(maps[i]);
	uses = map_indices(maps[i]);
	times = map_values(maps[i]);
	for (j = 0; j < sz_j; j++) {
	    int k, sz_k, *stamps;

	    text += "    " + uses[j] + ":\n";
	    sz_k = map_sizeof(times[j]);
	    stamps = map_indices(times[j]);
	    for (k = 0; k < sz_k; k++) {
		text += "        " + ctime(stamps[k]) + "\n";
	    }
	}
    }
    text +=
	"-- \n" +
	"/usr/UserDB/obj/user.c -> add_interactive_failure()\n";

    send_message(/* Env  */ "monitor@skotos.net",
		 /* From */ "UserDB", "monitor@skotos.net",
		 /* To   */ ([ "ce@skotos.net": "Skotos Customer Experience" ]),
		 /* Cc   */ ([ "harte@skotos.net": "Erwin Harte" ]),
		 /* Subj */ "UserDB banned " + name + ": Failed Preauths",
		 /* Xtra */ nil,
		 text);

    DEBUG("add_interactive_failure:"+ name + ":Account banned, clearing data-structure now.");
    interactive_failures = nil;
}

void clear_interactive_failures() {
    DEBUG("clear_interactive_failures:" + name + ":Original value = " + dump_value(interactive_failures));
    interactive_failures = nil;
}

void successful_preauth(string oid, string ref, string code,
			string payserv, string avscode,
			string cardnumber,
			string expmonth, string expyear,
			string addr, string zip, varargs string amount) {
   string desc;

# if 0
   if (auth_order_id) {
      error("preauth oid already exists");
   }
   if (cc_num || cc_expm || cc_expy) {
      error("old credit card already exists");
   }
# endif	/* looks like we can just let the old one time out */
   SysLog("[" + name + "] Successful PREAUTH OID");
   auth_order_id = oid;
   auth_order_amount = amount ? (float)amount : 0.0;
   catch {
       SEARCHDB->update_user(this_object(), "creditcard", cc_num, cardnumber);
   }
   cc_num = cardnumber;
   cc_expm = expmonth;
   cc_expy = expyear;

   desc = describe_card();
   log_billing("PREAUTH with OID " + oid + " for card " + desc,
	       BH_AUTH, oid, desc);

   sale_failure = nil;	
   sale_failures = 0;

   clear_interactive_failures();

   if (time() > next_stamp) {
       /* It's overdue, when's the next time to start billing? */
       prod_due_date();
   }
   catch {
      BILLINGD->remove_other_user(this_object());
      BILLINGD->add_cc_user(this_object());
      BILLINGD->add_history(time(), this_object(), name, BH_AUTH,
			    "PREAUTH with OID " + oid + " for card " + desc);
   }
}

void unsuccessful_preauth(string cc, string expm, string expy, string err) {
    string orig_cc;

    orig_cc = cc;
    while (strlen(cc) < 16) {
        cc += "?";
    }
    cc = cc[.. 3] + "********" + cc[12 .. 15];
    log_billing("FAILED PREAUTH for " + cc + " [" + expm + "/" + expy +
		"]: " +	err,
		BH_AFAILURE, cc, expm, expy, err);
    BILLINGD->add_history(time(), this_object(), name, BH_AFAILURE,
			  "FAILED PREAUTH for " + cc + " [" + expm + "/" +
			  expy + "]: " + err);
    catch {
	if (err != "TIMEOUT") {
	    add_interactive_failure(orig_cc, expm, expy, "preauth");
	}
    }
}

private
set_pay_day(int day) {
   while (day > 28) {
      day -= 28;
   }
   pay_day = day;
}


atomic
void prod_due_date() {
   int day, month, year, hour, now;

   now = time();	/* better safe than sorry */

   day   = get_day(now);
   month = get_month(now);
   year  = get_year(now);
   hour  = get_hour(now);

   if (next_year < year ||
       (next_year == year && next_month < month) ||
       (next_year == year && next_month == month && pay_day < day)) {
      int delay;

      delay = BILLINGD->query_callout_delay();
      /*
       * We're delinquent; bump to tomorrow unless we're on days 1..28 and today's
       * billing cycle hasn't started yet.
       */
      if (day > 28 || hour >= 3 || delay < 0 || delay >= 3 * 3600) {
	 DEBUG("prod_due_date:" + name + ": Now = " + ctime(now) + "; delay = " + delay + "; bump one day.");
	 now += 24 * 3600;
      } else {
	 DEBUG("prod_due_date:" + name + ": Now = " + ctime(now) + "; delay = " + delay + "; sticking to it.");
      }

      set_pay_day(get_day(now));
      next_month = get_month(now);
      next_year = get_year(now);
      if (pay_day < get_day(now)) {
	  /* Shift forward a month, we're not billing on month-days 29..31 */
	  if (next_month < 12) {
	      next_month++;
	  } else {
	      next_month = 1;
	      next_year++;
	  }
      }

      next_stamp = calculate_stamp(next_year, next_month, pay_day, time());
   }
}

int query_next_stamp() {
    switch (query_account_type()) {
    case AS_FREE:
    case AS_DEVELOPER:
    case AS_STAFF:
	return time() + 30 * 86400;
    default:
	return next_stamp;
    }
}

void bump_months(int num) {
   prod_due_date();

   next_month += num;
   while (next_month > 12) {
      next_year ++;
      next_month -= 12;
   }
   while (next_month < 0) {
      /* just in case */
      next_year --;
      next_month += 12;
   }
   next_stamp = calculate_stamp(next_year, next_month, pay_day, time());

   log_billing("DUEDAY BUMPED to " + next_year + "/" + next_month + "/" + pay_day,
	       BH_BUMPED, num, next_month, next_year);

   catch {
     if (IS_REGULAR(account_status)) {
       BILLINGD->add_other_user(this_object(), TRUE);
     }
   }
}

int query_card();

static void
delay_sale()
{
    if (query_billing_flag()) {
	DEBUG("delay_sale:" + name + ":Delaying do_sale() while query_billing_flag() active.");
	call_out("delay_sale", 1);
    } else {
	set_billing_flag("Charging Fee");
	call_out("do_sale", 0);
    }
}

void charge_card() {
   if (!query_card()) {
      error("user does not have a registered card");
   }
   delay_sale();
}

/* grab the first five numbers from the address */
string calculate_cc_addr(string str) {
   string out;
   int i;

   if (!str) {
      return "";
   }

   out = "";
   for (i = 0; i < strlen(str); i ++) {
      if (str[i] >= '0' && str[i] <= '9') {
	 out += str[i..i];
      }
   }
   return out;
}

void
do_preauth(string ip, string acct, string expm, string expy, string account_type, string account_period,
	   object cb_obj, string cb_func, mixed args...)
{
    float amount;
    mapping data;

    if (query_billing_flag()) {
	error("BILLING OPERATION ALREADY IN PROGRESS");
    }
    set_billing_flag("CC Authentication");
    amount = BILLINGD->query_billing_fee(account_type, account_period);
#if 1
    /* Use a maximum of 79.95 for now */
    amount = amount < 79.95 ? amount : 79.95;
#endif
    data = ([
	     "cardnumber": acct,
	     "expmonth":   expm,
	     "expyear":    expy,
	     "zip":        query_billing_property("zip"),
	     "addr":       calculate_cc_addr(query_billing_property("street1")),
	     "ip":         ip,
	     "userid":     name,
	     "amount":     (string)amount,
	     "monthly":    account_period == "monthly"   ? "true" : nil,
	     "quarterly":  account_period == "quarterly" ? "true" : nil,
	     "yearly":     account_period == "yearly"    ? "true" : nil,
	     "premium":    account_type   == "premium"   ? "true" : nil,
	     "basic":      account_type   == "basic"     ? "true" : nil,
	     "b_name":     query_billing_property("name"),
	     "b_addr1":    query_billing_property("addr1"),
	     "b_city":     query_billing_property("city"),
	     "b_state":    query_billing_property("state"),
	     "b_zip":      query_billing_property("zip"),
	     "b_country":  query_billing_property("country"),
	     "s_name":     query_property("name"),
	     "s_addr1":    query_property("addr1"),
	     "s_city":     query_property("city"),
	     "s_state":    query_property("state"),
	     "s_zip":      query_property("zip"),
	     "s_country":  query_property("country"),
	     "phone":      query_property("phone"),
	     ]);
    bill_request("done_preauth", ({ data, account_type, account_period, cb_obj, cb_func, args }), "PREAUTH", data, 60);
}

static void
done_preauth(mapping reply, mapping data, string account_type, string account_period,
	     object cb_obj, string cb_func, mixed *args)
{
    clear_billing_flag();

    if (reply["approved"] == "APPROVED") {
	successful_preauth(reply["oid"],
			   reply["ref"],
			   reply["code"],
			   reply["payserv"],
			   reply["avscode"],
			   data["cardnumber"],
			   data["expmonth"],
			   data["expyear"],
			   data["addr"],
			   data["zip"],
			   data["amount"]);
	switch (account_period) {
	case "monthly":
	    service = SERVICE_BASIC_MONTHLY;
	    break;
	case "quarterly":
	    service = SERVICE_BASIC_QUARTERLY;
	    break;
	case "yearly":
	    service = SERVICE_BASIC_YEARLY;
	    break;
	}
	catch {
	    call_other(cb_obj, cb_func, TRUE, nil, args...);
	}
    } else {
	unsuccessful_preauth(data["cardnumber"],
			     data["expmonth"],
			     data["expyear"],
			     reply["error"] ? reply["error"] : "Unknown");
	catch {
	    call_other(cb_obj, cb_func, FALSE, reply["error"], args...);
	}
    }
}

void
practical_upgrade(string period)
{
    int stamp;

    next_stamp = 0;
    next_year  = 0;
    next_month = 0;
    pay_day    = 0;
    prod_due_date();
    stamp = next_stamp;
    switch (period) {
    case "monthly":
	    service = SERVICE_BASIC_MONTHLY;
	    bump_months(1);
	    set_premium_sp_due(1);
	    break;
    case "quarterly":
	    service = SERVICE_BASIC_QUARTERLY;
	    bump_months(3);
	    set_premium_sp_due(3);
	    break;
    case "yearly":
	    service = SERVICE_BASIC_YEARLY;
	    bump_months(12);
	    set_premium_sp_due(12);
	    break;
    }
    set_account_type(AS_REGULAR);
    set_account_status(AS_PREMIUM);
    last_payment = ({ stamp, BILLINGD->query_billing_fee("premium", period), next_stamp });
    check_premium_sp_due();
}

float
round_float(float f)
{
    return (float)((int)(f * 100.0)) / 100.0;
}

float
apply_credit(float cash)
{
    account_credit = round_float(account_credit + cash);

    return account_credit;
}

float
query_account_credit()
{
    return account_credit;
}

/*
 * This upgrade functionality is intended for the private CE interface where
 * someone can upgrade by (a) turning the remainder into cash, add some more
 * cash to the account, and then immediately starting with premium.
 */
void
do_cash_upgrade(float cash, string account_period, int do_waive, object cb_obj, string cb_func,
		mixed args...)
{
    float amount, credit;

    amount = BILLINGD->query_billing_fee("premium", account_period);
    credit = (float)query_property("account_credit_potential");
    if (credit + account_credit + cash < amount) {
	if (do_waive) {
	    float waive;

	    /* Log how much we waived: */
	    waive = round_float(amount - credit - account_credit - cash);
	    log_billing("Waived $" + waive + " difference", BH_TEXT);
	    BILLINGD->add_history(time(), this_object(), name, BH_TEXT,
				  "Waived $" + waive + " difference");
	    apply_credit(waive);
	} else {
	    /*
	     * The remainder turned into cash, any existing credit plus the amount
	     * that was being added, just didn't add up to enough money, sorry!
	     */
	    if (cb_obj && cb_func) {
		call_other(cb_obj, cb_func, FALSE, "Insufficient funds.", args...);
	    }
	    return;
	}
    }
    if (credit > 0.0) {
	log_billing("Converted remainder of current billing period into $" + credit + " non-refundable credit",
		    BH_TEXT);
	BILLINGD->add_history(time(), this_object(), name,
			      BH_TEXT,
			      "Converted remainder of current billing period into $" + credit + " non-refundable credit");
	apply_credit(credit);
    }

    if (cash > 0.0) {
	log_billing("CREDITED with $" + cash + " intended for " + account_period + " payment of premium account",
		BH_CREDIT, cash, "premium", account_period, nil);
	BILLINGD->add_history(time(), this_object(), name, BH_CREDIT,
			      "CREDITED with $" + cash + " intended for " + account_period + " payment of premium account");
	apply_credit(cash);
    }

    apply_credit(-amount);
    log_billing("UPGRADE successful using non-refundable credit (" + amount + ")", BH_UPGRADE, nil, amount);
    BILLINGD->add_history(time(), this_object(), name,
			  BH_UPGRADE, "UPGRADE successful using non-refundable credit (" + amount + ")");
    log_billing("FEE of $" + amount + " for " + account_period + " billing period processed",
		BH_FEE, amount, account_period);
    BILLINGD->add_history(time(), this_object(), name,
			  BH_FEE, "FEE of $" + amount + " for " + account_period + " billing period processed");
    practical_upgrade(account_period);
    sale_failure = nil;
    sale_failures = 0;
    catch {
	if (cb_obj && cb_func) {
	    call_other(cb_obj, cb_func, TRUE, nil, args...);
	}
    }
}

/*
 * This upgrade functionality is intended for the public user-interface where
 * someone with a CC can upgrade from basic to premium.
 */
void
do_upgrade(string ip, string acct, string expm, string expy, string period, object cb_obj, string cb_func, mixed args...)
{
    float amount, credit;
    mapping data;

    if (query_billing_flag()) {
	error("BILLING OPERATION ALREADY IN PROGRESS");
    }
    set_billing_flag("Account Upgrade");

    amount = BILLINGD->query_billing_fee("premium", period);
    credit = (float)query_property("account_credit_potential");
    if (credit + account_credit && credit + account_credit > amount) {
	/* Enough to not even have to pay for this? */

	if (acct == cc_num && expm == cc_expm && expy == cc_expy) {
	    /*
	     * This is the one that's already in there!
	     * I'll let it slide then.
	     */
	    apply_credit(credit - amount);
	    if (credit > 0.0) {
		log_billing("Converted remainder of current billing period into $" + credit + " non-refundable credit",
			    BH_TEXT);
		BILLINGD->add_history(time(), this_object(), name,
				      BH_TEXT,
				      "Converted remainder of current billing period into $" + credit + " non-refundable credit");
	    }
	    log_billing("UPGRADE successful using non-refundable credit (" + amount + ")", BH_UPGRADE, nil, amount);
	    BILLINGD->add_history(time(), this_object(), name,
				  BH_UPGRADE, "UPGRADE successful using non-refundable credit (" + amount + ")");
	    log_billing("FEE of $" + amount + " for " + period + " billing period processed",
			BH_FEE, amount, period);
	    BILLINGD->add_history(time(), this_object(), name,
				  BH_FEE, "FEE of $" + amount + " for " + period + " billing period processed");
	    practical_upgrade(period);
	    sale_failure = nil;
	    sale_failures = 0;
	    catch {
		if (cb_obj && cb_func) {
		    call_other(cb_obj, cb_func, TRUE, nil, args...);
		}
	    }
	    clear_billing_flag();
	    return;
	} else {
	    /* Do a minimal PREAUTH with the card just to verify it. */
	    data = ([
		     "cardnumber": acct,
		     "expmonth":   expm,
		     "expyear":    expy,
		     "zip":        query_billing_property("zip"),
		     "addr":       calculate_cc_addr(query_billing_property("street1")),
		     "ip":         ip,
		     "userid":     name,
		     "amount":     "1.00",
		     "monthly":    period == "monthly"   ? "true": nil,
		     "quarterly":  period == "quarterly" ? "true": nil,
		     "yearly":     period == "yearly"    ? "true": nil,
		     "premium":    "true",
		     "b_name":     query_billing_property("name"),
		     "b_addr1":    query_billing_property("addr1"),
		     "b_city":     query_billing_property("city"),
		     "b_state":    query_billing_property("state"),
		     "b_zip":      query_billing_property("zip"),
		     "b_country":  query_billing_property("country"),
		     "s_name":     query_property("name"),
		     "s_addr1":    query_property("addr1"),
		     "s_city":     query_property("city"),
		     "s_state":    query_property("state"),
		     "s_zip":      query_property("zip"),
		     "s_country":  query_property("country"),
		     "phone":      query_property("phone"),
		     ]);
	    bill_request("done_upgrade", ({ data, period, FALSE, credit, cb_obj, cb_func, args }), "PREAUTH", data, 60);
	    return;
	}
    }
    data = ([
	     "cardnumber": acct,
	     "expmonth":   expm,
	     "expyear":    expy,
	     "zip":        query_billing_property("zip"),
	     "addr":       calculate_cc_addr(query_billing_property("street1")),
	     "ip":         ip,
	     "userid":     name,
	     "amount":     (string) (amount - credit - account_credit),
	     "monthly":    period == "monthly"   ? "true": nil,
	     "quarterly":  period == "quarterly" ? "true": nil,
	     "yearly":     period == "yearly"    ? "true": nil,
	     "premium":    "true",
	     "b_name":     query_billing_property("name"),
	     "b_addr1":    query_billing_property("addr1"),
	     "b_city":     query_billing_property("city"),
	     "b_state":    query_billing_property("state"),
	     "b_zip":      query_billing_property("zip"),
	     "b_country":  query_billing_property("country"),
	     "s_name":     query_property("name"),
	     "s_addr1":    query_property("addr1"),
	     "s_city":     query_property("city"),
	     "s_state":    query_property("state"),
	     "s_zip":      query_property("zip"),
	     "s_country":  query_property("country"),
	     "phone":      query_property("phone"),
	     ]);
   bill_request("done_upgrade", ({ data, period, TRUE, credit, cb_obj, cb_func, args }), "SALE", data, 60);
}

void
done_upgrade(mapping reply, mapping data, string period, int is_upgrade, float credit, object cb_obj, string cb_func, mixed *args)
{
    string desc;

    clear_billing_flag();

    desc = data["cardnumber"];
    while (strlen(desc) < 16) {
	desc += "?";
    }
    desc = desc[.. 3] + "********" + desc[12 .. 15] + " [" +
	   data["expmonth"] + "/" + data["expyear"] + "]";

    if (reply["approved"] == "APPROVED") {
	float amount;

	catch {
	    if (query_account_type() == AS_TRIAL) {
		switch (period) {
		case "year":
		case "yearly":
		    send_message(/* Env  */ "monitor@skotos.net",
				 /* From */ "UserDB", "monitor@skotos.net",
				 /* To   */ ([ "ce@skotos.net": "Skotos Customer Experience" ]),
				 /* Cc   */ ([ "harte@skotos.net": "Erwin Harte" ]),
				 /* Subj */ "UserDB: Unusual upgrade: " + name,
				 /* Xtra */ nil,
				 "The trial account '" + name + "' upgraded to premium (" + period + ")\n" +
				 "-- \n" +
				 "/usr/UserDB/obj/user.c -> done_upgrade()\n");
		    break;
		default:
		    break;
		}
	    }
	}
	catch {
	    if (cb_obj && cb_func) {
		call_other(cb_obj, cb_func, TRUE, nil, args...);
	    }
	}
	practical_upgrade(period);

	amount = (float)data["amount"];
	/* Update the creditcard information. */
	catch {
	    SEARCHDB->update_user(this_object(), "creditcard", cc_num, data["cardnumber"]);
	}
	cc_num  = data["cardnumber"];
	cc_expm = data["expmonth"];
	cc_expy = data["expyear"];
	sale_failure = nil;
	sale_failures = 0;

	if (credit > 0.0) {
	    log_billing("Converted remainder of current billing period into $" + credit + " non-refundable credit",
			BH_TEXT);
	    BILLINGD->add_history(time(), this_object(), name,
				  BH_TEXT,
				  "Converted remainder of current billing period into $" + credit + " non-refundable credit");
	}
	if (is_upgrade) {
	    log_billing("UPGRADE successful using " + desc + " ($" + amount + ")",
			BH_UPGRADE, desc, credit, amount);
	    BILLINGD->add_history(time(), this_object(), name,
				  BH_UPGRADE, "UPGRADE successful using " + desc + " (" + amount + ")");
	    account_credit = 0.0;
	} else {
	    /*
	     * We did a PREAUTH b/c the converted remainder was more than
	     * was required for the first fee.
	     */
	    log_billing("UPGRADE successful using non-refundable credit (" + amount + ")", BH_UPGRADE, nil, amount, amount);
	    BILLINGD->add_history(time(), this_object(), name,
				  BH_UPGRADE, "UPGRADE successful using non-refundable credit (" + amount + ")");
	    apply_credit(credit - amount);
	}
	log_billing("FEE of $" + BILLINGD->query_billing_fee("premium", period) + " for " + period + " billing period processed",
		    BH_FEE, BILLINGD->query_billing_fee("premium", period), period);
	BILLINGD->add_history(time(), this_object(), name,
			      BH_FEE, "FEE of $" + BILLINGD->query_billing_fee("premium", period) + " for " + period + " billing period processed");

	BILLINGD->remove_other_user(this_object());
	BILLINGD->add_cc_user(this_object());

	clear_interactive_failures();
    } else {
	catch {
	    call_other(cb_obj, cb_func, FALSE, reply["error"], args...);
	}
	log_billing("FAILED UPGRADE using " + desc + ": "+ reply["error"],
		    BH_UFAILURE, desc, reply["error"]);
	BILLINGD->add_history(time(), this_object(), name,
			      BH_UFAILURE,
			      "FAILED UPGRADE using " + desc + ": "+ reply["error"]);
	if (reply["error"] != "TIMEOUT") {
	    catch {
		add_interactive_failure(data["cardnumber"], data["expmonth"], data["expyear"], "upgrade");
	    }
	}
    }
}

int do_downgrade(string period) {
    switch (period) {
    case "monthly":
	service = SERVICE_BASIC_MONTHLY;
	break;
    case "quarterly":
	service = SERVICE_BASIC_QUARTERLY;
	break;
    case "yearly":
	service = SERVICE_BASIC_YEARLY;
	break;
    }
    clear_account_status(AS_PREMIUM);
    log_billing("DOWNGRADED to basic account with " + period  + " billing", BH_TEXT);
    BILLINGD->add_history(time(), this_object(), name,
			  BH_TEXT, "DOWNGRADED to basic account with " + period + " billing");
    return TRUE;
}

static
void do_sale() {
    float   fee;
    string  next_type, next_period;
    mapping data;

    if (next_payment) {
	next_type = next_payment[0];
	next_period = next_payment[1];
	next_payment = nil;
    } else {
	next_type = IS_PREMIUM(account_status) ? "premium" : "basic";
	switch (service) {
	case SERVICE_BASIC_MONTHLY:
	    next_period = "monthly";
	    break;
	case SERVICE_BASIC_QUARTERLY:
	    next_period = "quarterly";
	    break;
	case SERVICE_BASIC_YEARLY:
	    next_period = "yearly";
	    break;
	}
    }
    fee = BILLINGD->query_billing_fee(next_type, next_period);
    if (account_credit > 0.0 && account_credit >= fee) {
	/* This should not happen! */
	clear_billing_flag();
	return;
    }
    data = ([
      "cardnumber": cc_num,
      "expmonth":   cc_expm,
      "expyear":    cc_expy,
      "zip":        query_billing_property("zip"),
      "addr":       calculate_cc_addr(query_billing_property("street1")),
      "userid":     name,
      "amount":     (string) (fee - account_credit),
      "monthly":    next_period == "monthly"   ? "true" : nil,
      "quarterly":  next_period == "quarterly" ? "true" : nil,
      "yearly":     next_period == "yearly"    ? "true" : nil,
      "premium":    next_type   == "premium"   ? "true" : nil,
      "basic":      next_type   == "basic"     ? "true" : nil,
      "b_name":     query_billing_property("name"),
      "b_addr1":    query_billing_property("addr1"),
      "b_city":     query_billing_property("city"),
      "b_state":    query_billing_property("state"),
      "b_zip":      query_billing_property("zip"),
      "b_country":  query_billing_property("country"),
      "s_name":     query_property("name"),
      "s_addr1":    query_property("addr1"),
      "s_city":     query_property("city"),
      "s_state":    query_property("state"),
      "s_zip":      query_property("zip"),
      "s_country":  query_property("country"),
      "phone":      query_property("phone"),
      ]);
   bill_request("done_sale", ({ data, next_type, next_period }), "SALE", data, 60);
}

static atomic
void check_referral_friend()
{
    int    t;
    object friend;
    mixed  val, **log;

    DEBUG("check_referral_friend()");
    val = query_property("referral:friend");
    if (!val) {
        DEBUG("No referral:friend\n");
        return;
    }
    if (query_property("payment_status")) {
        DEBUG("payment_status: " + ctime(query_property("payment_status")));
        return;
    }

    /*
     * This is going to be the first payment for this account _and_
     * this account was referred to Skotos by a friend or family member.
     */

    friend = val[2];
    if (!friend) {
	DEBUG("user object gone.");
	return;
    }

    /*
     * Good good, that same friend still exists, how convenient.
     */
    log = friend->query_property("sps:log");
    if (!log) {
	/*
	 * Eh, this shouldn't happen, but let's proceed.
	 */
	log = ({ });
    }
    log += ({ ({
        t = time(),
	"friend:paid",
	20,
	name,
	this_object(),
	val[3],
	val[4]
    }) });
    DEBUG("log is now " + dump_value(log));
    friend->set_property("sps:log", log);
    POINTSDB->add_entry(t, 20, "friend:paid", this_object(), name, friend,
			friend->query_name());
}

static
void done_sale(mapping reply, mapping data, string next_type, string next_period) {
   string failure, desc;

   clear_billing_flag();

   desc = describe_card();
   if (reply["approved"] && reply["approved"] == "APPROVED") {
      int orig_stamp;

      orig_stamp = next_stamp;
      catch {
	 check_referral_friend();
      }
      SysLog("[" + name + "] SALE successful using " + desc);
      log_billing("SALE successful using " + desc,
		  BH_CHARGE, desc);
      switch (next_period) {
      case "monthly":
	 bump_months(1);
	 service = SERVICE_BASIC_MONTHLY;
	 if (next_type == "premium") {
	     set_premium_sp_due(1);
	 }
	 break;
      case "quarterly":
	 bump_months(3);
	 service = SERVICE_BASIC_QUARTERLY;
	 if (next_type == "premium") {
	     set_premium_sp_due(3);
	 }
	 break;
      case "yearly":
	 bump_months(12);
	 service = SERVICE_BASIC_YEARLY;
	 if (IS_PREMIUM(account_status)) {
	     set_premium_sp_due(12);
	 }
	 break;
      }
      if (next_type == "basic") {
	  clear_account_status(AS_PREMIUM);
      } else {
	  set_account_status(AS_PREMIUM);
      }
      if (data && typeof(data) == T_MAPPING && typeof(data["amount"]) == T_STRING) {
	  last_payment = ({ time() > orig_stamp ? time() : orig_stamp, (float)data["amount"], next_stamp });
	  DEBUG("done_sale:" + name + ":Updated last_payment to " + dump_value(last_payment));
      }
      log_billing("FEE of $" + BILLINGD->query_billing_fee(next_type, next_period) + " for " + next_period + " billing period processed",
		  BH_FEE, BILLINGD->query_billing_fee(next_type, next_period), next_period);
      BILLINGD->add_history(time(), this_object(), name,
			    BH_FEE, "FEE of $" + BILLINGD->query_billing_fee(next_type, next_period) + " for " + next_period + " billing period processed");

      sale_failure = nil;
      sale_failures = 0;
      trial = FALSE;
      set_account_type(AS_REGULAR);
      catch {
	 BILLINGD->charge_users_callback(this_object(), 0, "");
	 BILLINGD->add_history(time(), this_object(), name, BH_CHARGE,
			       "[" + name + "] SALE successful using " + desc);
      }
      check_premium_sp_due();
      return;
   }
   /* the sale failed... this is not so good */
   failure = reply["error"] ? url_decode(reply["error"]) : "unknown error";
   if (failure != "TIMEOUT") {
      sale_failure = failure;
      sale_failures ++;
      log_billing("SALE failure #" + sale_failures + " [" + sale_failure +
		  "] using " + desc,
		  BH_SFAILURE, sale_failures, sale_failure, desc);
      SysLog("[" + name + "] SALE failure #" + sale_failures + " [" + sale_failure + "] using " + desc + "! Please investigate; reply = " + dump_value(reply));
      catch {
	 /*
	  * Do this before the card gets cleared so that the sale_* stuff still
	  * contains the relevant information.
	  */
	 BILLINGD->charge_users_callback(this_object(), sale_failures, failure);
      }
      if (sale_failures > MAX_FAILURES) {
	 /* Delay clearing the card until the logging has been done. */
	 catch {
	    BILLINGD->add_history(time(), this_object(), name, BH_EXPIRE);
	    log_billing("ACCOUNT EXPIRE", BH_EXPIRE);
	 }
	 clear_card();
      }
   } else {
      catch {
	 BILLINGD->charge_users_callback(this_object(), sale_failures, failure);
      }
   }
}

/*
 * Check if it's time to use some of the remaining credit yet.
 */
atomic void
check_credit()
{
    float fee;
    string next_type, next_period;

    if (time() < next_stamp) {
	/* It isn't time yet. */
	return;
    }
    if (!account_credit) {
	/* No credit to work with anyway. */
	return;
    }
    if (next_payment) {
	/* Check _preferred_ account type and billing period */
	next_type = next_payment[0];
	next_period = next_payment[1];
	next_payment = nil;
    } else {
	next_type = IS_PREMIUM(account_status) ? "premium" : "basic";
	switch (service) {
	case SERVICE_BASIC_MONTHLY:
	    next_period = "monthly";
	    break;
	case SERVICE_BASIC_QUARTERLY:
	    next_period = "quarterly";
	    break;
	case SERVICE_BASIC_YEARLY:
	    next_period = "yearly";
	    break;
	}
    }
    fee = BILLINGD->query_billing_fee(next_type, next_period);
    if (fee > account_credit) {
	switch (next_period) {
	case "yearly":
	    /* See if quarterly might work. */
	    fee = BILLINGD->query_billing_fee(next_type, "quarterly");
	    if (fee <= account_credit) {
		next_period = "quarterly";
		break;
	    }
	    /* Fall through */
	case "quarterly":
	    fee = BILLINGD->query_billing_fee(next_type, "monthly");
	    if (fee <= account_credit) {
		next_period = "monthly";
		break;
	    }
	    /* Fall through */
	default:
	    /* Not gonna work, then. */
	    return;
	}
    }
    log_billing("Used $" + fee + " from non-refundable credit for " + next_period + " fee",
		BH_TEXT);
    BILLINGD->add_history(time(), this_object(), name, BH_TEXT,
			  "Used $" + fee + " from non-refundable credit for " + next_period + " fee");
    apply_credit(-fee);
    set_account_type(AS_REGULAR);
    sale_failure = nil;
    sale_failures = 0;
    switch (next_period) {
    case "yearly":
	bump_months(12);
	service = SERVICE_BASIC_YEARLY;
	if (next_type == "premium") {
	    set_premium_sp_due(12);
	}
	break;
    case "quarterly":
	bump_months(3);
	service = SERVICE_BASIC_QUARTERLY;
	if (next_type == "premium") {
	    set_premium_sp_due(3);
	}
	break;
    case "monthly":
	bump_months(1);
	service = SERVICE_BASIC_MONTHLY;
	if (next_type == "premium") {
	    set_premium_sp_due(1);
	}
	break;
    }
    if (next_type == "basic") {
	clear_account_status(AS_PREMIUM);
    } else {
	set_account_status(AS_PREMIUM);
	check_premium_sp_due();
    }
    last_payment = ({ time(), fee, next_stamp });
}

atomic void
external_credit(float cash, string account_type, string account_period)
{
    /* This'll error out if the account type or period are invalid */
    BILLINGD->query_billing_fee(account_type, account_period);

    log_billing("CREDITED with $" + cash + " intended for " + account_period + " payment of " + account_type + " account",
		BH_CREDIT, cash, account_type, account_period, nil);
    BILLINGD->add_history(time(), this_object(), name, BH_CREDIT,
			  "CREDITED with $" + cash + " intended for " + account_period + " payment of " + account_type + " account");
    apply_credit(cash);
    next_payment = ({ account_type, account_period });
    check_credit();
    BILLINGD->add_other_user(this_object());
}

atomic void
external_debit(float cash, string reason)
{
    log_billing("DEBITED with $" + cash + ": " + reason,
		BH_DEBIT, cash, reason);
    BILLINGD->add_history(time(), this_object(), name, BH_DEBIT,
			  "DEBITED with $" + cash + ": " + reason);
    apply_credit(-cash);
}

/*
 * Upgrade the customers that were paying with yearly checks and sent in
 * an amount to upgrade the remaining X months of their account to premium.
 *
 * I'm going to (a) waive the difference and (b) waive the X days until their
 * billing day (day of the month, that is).
 */
atomic void
external_upgrade(float cash, int months) {
    float fee, credit, waive;

    /* First put the money in their account. */
    log_billing("CREDITED with $" + cash + " intended for " + months + " month(s) payment of premium account",
		BH_CREDIT, cash, nil, nil, nil);
    BILLINGD->add_history(time(), this_object(), name, BH_CREDIT,
			  "CREDITED with $" + cash + " intended for " + months + " month(s) payment of premium account");
    apply_credit(cash);

    /* This is how much we really need. */
    fee = BILLINGD->query_billing_fee("premium", "year") * (float)(next_stamp - time()) / (float)(365 * 86400);
    fee = round_float(fee);

    credit = floor(100.0 * (float)(next_stamp - time()) / (float)(365 * 86400));
    if (credit > 0.0) {
	log_billing("Converted remainder of current billing period into $" + credit + " non-refundable credit",
		    BH_TEXT);
	BILLINGD->add_history(time(), this_object(), name,
			      BH_TEXT,
			      "Converted remainder of current billing period into $" + credit + " non-refundable credit");
	apply_credit(credit);
    }
    if (fee > account_credit) {
	/* Nope, that's not enough, but we'll waive the difference. */
	waive = round_float(fee - account_credit);

	/* Log how much we used from the user: */
	log_billing("Used $" + account_credit + " from non-refundable credit for " + months + " month(s) fee",
		    BH_TEXT);
	BILLINGD->add_history(time(), this_object(), name, BH_TEXT,
			      "Used $" + account_credit + " from non-refundable credit for " + months + " month(s) fee");

	/* Log how much we waived: */
	log_billing("Waived $" + waive + " difference", BH_TEXT);
	BILLINGD->add_history(time(), this_object(), name, BH_TEXT,
			      "Waived $" + waive + " difference");

	/* Log that the upgrade was successful: */
	log_billing("UPGRADE of current billing period successful using non-refundable credit (" + account_credit + ")", BH_UPGRADE, account_credit, fee);
	BILLINGD->add_history(time(), this_object(), name,
			      BH_UPGRADE, "UPGRADE of current billing period successful using non-refundable credit (" + account_credit + ")");
	apply_credit(-account_credit);
    } else {
	/* Cool, this account actually has enough and maybe more than that. */

	/* Log how much we used from the user: */
	log_billing("Used $" + fee + " from non-refundable credit for yearly fee",
		    BH_TEXT);
	BILLINGD->add_history(time(), this_object(), name, BH_TEXT,
			      "Used $" + fee + " from non-refundable credit for yearly fee");

	/* Log that the upgrade was successful: */
	log_billing("UPGRADE of current billing period successful using non-refundable credit (" + fee + ")", BH_UPGRADE, fee, fee);
	BILLINGD->add_history(time(), this_object(), name,
			      BH_UPGRADE, "UPGRADE of current billing period successful using non-refundable credit (" + fee + ")");
	apply_credit(-fee);
    }
    set_account_status(AS_PREMIUM);
    set_premium_sp_due(months);
    set_account_type(AS_REGULAR);
    sale_failure = nil;
    sale_failures = 0;
    service = SERVICE_BASIC_YEARLY;
    check_premium_sp_due();
    /* Pretend they paid the full fee for the entire period. */
    last_payment = ({ next_stamp - 365 * 86400, fee, next_stamp });
}

atomic
void external_payment(float cash, int months) {
   int orig_stamp;

   orig_stamp = next_stamp;
   catch {
      /* Workaround to avoid giving away storypoints from free accounts. */
      if (months == 12) {
	  check_referral_friend();
      }
   }
   log_billing("EXTERNAL PAYMENT of $" + cash + " for " + months + " months.",
	       BH_EXTERNAL, cash, months);
   SysLog("[" + name + "] PAID $" + cash + " for " + months + " months");
   catch {
      BILLINGD->add_history(time(), this_object(), name, BH_EXTERNAL,
			    "EXTERNAL PAYMENT of $" + cash + " for " + months + " months.");
   }
   trial = FALSE;
   set_account_type(AS_REGULAR);
   sale_failure  = nil;
   sale_failures = 0;
   bump_months(months);
   if (cash > 0.0) {
       last_payment = ({ time() > orig_stamp ? time() : orig_stamp, cash, next_stamp });
       DEBUG("external_payment:" + name + ":Updated last_payment to " + dump_value(last_payment));
   }
}

void start_trial_month() {
   trial = TRUE;
   set_account_type(AS_TRIAL);
   prod_due_date();
   if (!query_property("no-trial-month")) {
       bump_months(1);
   }
}

string query_sale_failure() { return sale_failure; }

string
query_auth_order_id() {
    return auth_order_id;
}

float
query_auth_order_amount() {
    return auth_order_amount;
}

void clear_auth_order_id() {
   SysLog("[" + name + "] Clearing AUTH OID (" + auth_order_id + ")");
   log_billing("CLEAR AUTH OID " + auth_order_id,
	       BH_CAUTH, auth_order_id);
   auth_order_id = nil;
   auth_order_amount = 0.0;
}

string describe_card() {
   string dispnum;

   if (cc_num && cc_expm && cc_expy) {
      /* hack while we figure out why cc_num can be < 16 len */
      dispnum = cc_num;
      while (strlen(dispnum) < 16) {
	 dispnum += "?";
      }
      return dispnum[.. 3] + "********" + dispnum[12 .. 15] +
	 " [" + cc_expm + "/" + cc_expy + "]";
   }
   return "";
}

int query_card() {
    if (cc_num && cc_expm && cc_expy) {
	if (!BILLINGD->is_cc_user(this_object())) {
	    DEBUG("ALERT:" + name + " has CC data but isn't registered.");
	}
	return TRUE;
    }
    return FALSE;
}

string query_card_num() {
    return cc_num;
}

int expired_card() {
   if (cc_num && cc_expm && cc_expy) {
      int ts, ts_m, ts_y;

      ts = time();
      ts_m = get_month(ts);
      ts_y = get_year(ts) - 2000;
      /* XXX: Why don't we store the cc_expy as a 4 digit year? */
      return (int)cc_expy < ts_y || ((int)cc_expy == ts_y && (int)cc_expm < ts_m);
   }
   return FALSE;
}

void clear_card() {
   string desc;

   desc = describe_card();
   SysLog("[" + name + "] Clearing Credit Card (" + desc + ")");
   log_billing("CLEAR CARD " + desc,
	       BH_CANCEL, desc);
   catch {
       SEARCHDB->update_user(this_object(), "creditcard", cc_num, nil);
   }
   cc_num = cc_expm = cc_expy = nil;
   sale_failure = nil;
   sale_failures = 0;
   catch {
       BILLINGD->remove_cc_user(this_object());
       BILLINGD->add_other_user(this_object());
       BILLINGD->add_history(time(), this_object(), name, BH_CANCEL,
			     "CLEAR CARD " + desc);
   }
}


mixed query_property(string prop) {
   switch (prop) {
   case "salefailure":
      return sale_failure;
   case "salefailures":
      if (sale_failures > 0 && next_stamp < time()) {
	  DEBUG("User:" + name + ":Non-zero sale_failures queried.");
	  return sale_failures;
      }
      return 0;
   case "email":
      return email;
   case "card":
      return describe_card();
   case "card:expired":
      return expired_card();
   case "cost":
      return query_cost();
   case "monthlycost":
      return query_monthly_cost();
   case "quarterlycost":
      return query_quarterly_cost();
   case "yearlycost":
      return query_yearly_cost();
   case "nextstamp":
      return query_next_stamp();
   case "payday":
      return pay_day;
   case "trialdays":
      if (IS_TRIAL(account_status)) {
	 if (next_stamp > time()) {
	    return 1 + (query_trial_until() - time()) / (24*3600);
	 }
	 return 0;
      }
      return 0;
   case "paiddays":
       switch (query_account_type()) {
       case AS_FREE:
       case AS_DEVELOPER:
       case AS_STAFF:
	   return 30;
       case AS_TRIAL:
	   return 0;
       default:
	   if (next_stamp > time()) {
	       return 1 + (query_paid_until() - time()) / (24*3600);
	   }
	   return 0;
       }
       /* Unreachable statement. */
       return 0;
   case "dayth":
      if (pay_day) {
	 return pay_day + get_dayth(pay_day);
      }
      return "";
   case "month":
      if (next_month) {
	 return describe_month(next_month);
      }
      return "";
   case "year":
      if (next_year) {
	 return (string) next_year;
      }
      return "";
   case "monthlyservice":
      return service == SERVICE_BASIC_MONTHLY ? "true": "";
   case "quarterlyservice":
      return service == SERVICE_BASIC_QUARTERLY ? "true": "";
   case "yearlyservice":
      return service == SERVICE_BASIC_YEARLY ? "true": "";
   case "billinglog":
      return describe_billing_log(FALSE);
   case "billinglog:public":
      return describe_billing_log(TRUE);
   case "payment_status": {
      /* Has this user ever paid anything, ever? */
      int i, sz;

      sz = sizeof(billing_log);
      for (i = 0; i < sz; i++) {
	  switch (billing_log[i][2]) {
	  case BH_CHARGE:
	  case BH_EXTERNAL:
	  case BH_UPGRADE:
	  case BH_FEE:
	      return billing_log[i][0];
	  default:
	      break;
	  }
      }
      return FALSE;
   }
   case "account_status":
       return implode(describe_account_status(), ",");
   case "account_type":
       return describe_account_type();
   case "account_credit": {
       int ac_b, ac_a;

       if (!account_credit) {
	   return "0";
       }
       ac_b = (int)(account_credit * 100.0);
       ac_a = ac_b % 100;
       ac_b = ac_b / 100;
       switch (ac_a) {
       case 0..9:
	   return ac_b + ".0" + ac_a;
       default:
	   return ac_b + "." + ac_a;
       }
   }
   case "account_credit_potential": {
       int total;

       if (name == "hresh") {
	   total = 60;
       } else if (!IS_REGULAR(account_status)) {
	   total = 0;
       } else if (!last_payment) {
	   total = 0;
       } else if (time() > next_stamp) {
	   total = 0;
       } else {
	   total = next_stamp - time();
	   total = (int)(last_payment[1] * (float)(next_stamp - time()) / (float)(last_payment[2] - last_payment[0]));
       }
       return (string)total;
   }
   case "storypoints:total": {
       int   sum;
       mixed val;

       val = query_property("sps:log");
       if (val) {
	   int i, sz, sum;

	   sz = sizeof(val);
	   for (i = 0; i < sz; i++) {
	       if (val[i][2] > 0) {
		   sum += val[i][2];
	       }
	   }
	   return (string)sum;
       }
       return "0";
   }
   case "storypoints:used": {
       int sum;
       mixed val;

       val = query_property("sps:log");
       if (val) {
	   int i, sz;

	   sz = sizeof(val);
	   for (i = 0; i < sz; i++) {
	       if (val[i][2] < 0) {
		   sum -= val[i][2];
	       }
	   }
       }
       return (string)sum;
   }
   case "storypoints:available": {
       int sum;
       mixed val;

       val = query_property("sps:log");
       if (val) {
	   int i, sz;

	   sz = sizeof(val);
	   for (i = 0; i < sz; i++) {
	       sum += val[i][2];
	   }
       }
       return (string)sum;
   }
   case "storypoints:eligible": {
       mixed val;

       val = query_property("sps:log");
       if (val) {
	   int     i, sz, sum;
	   mapping eligible;

	   sz = sizeof(val);
	   eligible = ([ ]);
	   for (i = 0; i < sz; i++) {
	       switch (val[i][1]) {
	       case "friend":
		   /*
		    * Take into account that it would help if the user is still
		    * likely to ever become a paying customer.
		    */
		   if (val[i][4] && (val[i][4]->query_card() && val[i][4]->query_next_stamp() > time())) {
		       sum += 20;
		       eligible[val[i][4]] = TRUE;
		   }
		   break;
	       case "friend:paid":
		   if (eligible[val[i][4]]) {
		       sum -= 20;
		       eligible[val[i][4]] = nil;
		   }
		   break;
	       default:
		   break;
	       }
	   }
	   return (string)sum;
       }
       return "0";
   }
   case "royalties:available":
      return !!STATSDB->query_royalties_acclaim(this_object());
   case "royalties:position1": {
       int *toplist;

       toplist = STATSDB->query_royalties_acclaim(this_object());
       if (toplist) {
	   return toplist[0] + 1;
       }
       return nil;
   }
   case "royalties:name1": {
       int *toplist;

       toplist = STATSDB->query_royalties_acclaim(this_object());
       if (toplist) {
	   string game;

	   game = STATSDB->query_royalties_games()[toplist[0]];
	   return GAMEDB->query_game_by_name(game)[4];
       }
       return nil;
   }
   case "royalties:position2": {
       int *toplist;

       toplist = STATSDB->query_royalties_acclaim(this_object());
       if (toplist) {
	   return toplist[1] + 1;
       }
       return nil;
   }
   case "royalties:name2": {
       int *toplist;

       toplist = STATSDB->query_royalties_acclaim(this_object());
       if (toplist) {
	   string game;

	   game = STATSDB->query_royalties_games()[toplist[1]];
	   return GAMEDB->query_game_by_name(game)[4];
       }
       return nil;
   }
   case "royalties:position3": {
       int *toplist;

       toplist = STATSDB->query_royalties_acclaim(this_object());
       if (toplist) {
	   return toplist[2] + 1;
       }
       return nil;
   }
   case "royalties:name3": {
       int *toplist;

       toplist = STATSDB->query_royalties_acclaim(this_object());
       if (toplist) {
	   string game;

	   game = STATSDB->query_royalties_games()[toplist[2]];
	   return GAMEDB->query_game_by_name(game)[4];
       }
       return nil;
   }
   case "banned:when":
       if (IS_BANNED(account_status)) {
	   return USERDB->query_user_banned(name)[0];
       }
       return nil;
   case "banned:who":
       if (IS_BANNED(account_status)) {
	   return USERDB->query_user_banned(name)[1];
       }
       return nil;
   case "banned:reason":
       if (IS_BANNED(account_status)) {
	   return USERDB->query_user_banned(name)[2];
       }
       return nil;
   case "no-email":
       return IS_NO_EMAIL(account_status) ? "true" : nil;
   case "password:crypt":
       return crypt(password, password);
   case "password:md5":
       return to_hex(hash_md5(password));
   case "motd:global:message":
       return USERDB->query_motd_message();
   case "motd:global:timestamp":
       return USERDB->query_motd_timestamp();
   case "motd:global:expire":
       return USERDB->query_motd_expire();
   default:
       return ::query_property(prop);
   }
}

void clear_property(string prop) {
   switch (prop) {
   case "no-email":
      ::clear_property(prop);
      clear_account_status(AS_NO_EMAIL);
      break;
   case "motd:global:message":
      USERDB->set_motd_message(nil);
      break;
   case "motd:global:timestamp":
      USERDB->set_motd_timestamp(0);
      break;
   case "motd:global:expire":
      USERDB->set_motd_expire(0);
      break;
   default:
      ::clear_property(prop);
      break;
   }
}

void set_property(string prop, mixed val) {
   switch(prop) {
   case "monthlyservice":
      if (typeof(val) == T_STRING && strlen(val)) {
	 service = SERVICE_BASIC_MONTHLY;	/* for now */
      }
      break;
   case "quarterlyservice":
      if (typeof(val) == T_STRING && strlen(val)) {
	 service = SERVICE_BASIC_QUARTERLY;	/* for now */
      }
      break;
   case "yearlyservice":
      if (typeof(val) == T_STRING && strlen(val)) {
	 service = SERVICE_BASIC_YEARLY;	/* for now */
      }
      break;
   case "reminder":
      if (typeof(val) == T_STRING) {
	 SysLog("[" + name + "] reminder moved to " + ctime((int) (float) val));
	 ::set_property(prop, (int) (float) val);
      } else {
	 ::set_property(prop, val);
      }
      break;
   case "no-email":
      ::set_property(prop, val);
      if (query_property(prop)) {
	 set_account_status(AS_NO_EMAIL);
      } else {
	 clear_account_status(AS_NO_EMAIL);
      }
      break;
   case "motd:global:message":
      USERDB->set_motd_message(val);
      break;
   case "motd:global:timestamp":
      USERDB->set_motd_timestamp((int)val);
      break;
   case "motd:global:expire":
      USERDB->set_motd_expire((int)val);
      break;
   default: {
      mixed old;

      old = query_property(prop);
      ::set_property(prop, val);
      catch {
	  SEARCHDB->update_user(this_object(), prop, old, val);
      }
      }
   }
}

int query_trial() {
    return IS_TRIAL(account_status);
}

int query_premium() {
    return IS_PREMIUM(account_status);
}

void
set_user_ban()
{
    if (previous_program() == USERDB) {
	banned = TRUE;
	set_account_status(AS_BANNED);
	if (query_card()) {
	    /*
	     * To avoid people ending up still paying when their account is
	     * blocked by _us_.
	     */
	    clear_card();
	}
    }
}

void
clear_user_ban()
{
    if (previous_program() == USERDB) {
	banned = FALSE;
	clear_account_status(AS_BANNED);
    }
}

int
query_user_ban()
{
    return IS_BANNED(account_status);
}

void hack() {
   sale_failure = "Your tummy is empty. Eat food!";
}

void die() { destruct_object(); }

void
patch_billing_log()
{
    int i, sz;
    mixed **log;

    sz = sizeof(billing_log);
    log = allocate(sz);
    for (i = 0; i < sz; i++) {
	string str, str1, str2, str3, str4;

	str = billing_log[i][1];

	if (sizeof(billing_log[i]) > 2) {
	    log[i] = billing_log[i][..];
	    continue;
	}
	DEBUG("patch_billing_log:" + name + ":" + dump_value(billing_log[i]));
	if (sscanf(str, "PREAUTH with OID %s for card %s", str1, str2) == 2) {
	    log[i] = billing_log[i] + ({ BH_AUTH, str1, str2 });
	    continue;
	}
	if (sscanf(str, "SALE successful using %s", str1) == 1) {
	    log[i] = billing_log[i] + ({ BH_CHARGE, str1 });
	    continue;
	}
	if (sscanf(str, "EXTERNAL PAYMENT of $%s for %s months.", str1, str2) == 2) {
	    log[i] = billing_log[i] + ({ BH_EXTERNAL, (float)str1, (int)str2 });
	    continue;
	}
	if (sscanf(str, "CLEAR AUTH OID %s", str1) == 1) {
	    log[i] = billing_log[i] + ({ BH_CAUTH, str1 });
	    continue;
	}
	if (sscanf(str, "CLEAR CARD %s", str1) == 1) {
	    log[i] = billing_log[i] + ({ BH_CANCEL, str1 });
	    continue;
	}
	if (str == "CREATED") {
	    log[i] = billing_log[i] + ({ BH_CREATED });
	    continue;
	}
	if (sscanf(str, "DUEDAY BUMPED %s to %s/%s", str1, str2, str3) == 3) {
	    log[i] = billing_log[i] + ({ BH_BUMPED, (int)str1, (int)str2, (int) str3 });
	    continue;
	}
	if (sscanf(str, "SALE failure #%s [%s] using %s", str1, str2, str3) == 3) {
	    log[i] = billing_log[i] + ({ BH_SFAILURE, (int)str1, str2, str3 });
	    continue;
	}
	DEBUG("patch_billing_log: " + name + ": Unrecognized entry: " +
	      billing_log[i][1]);
	log[i] = billing_log[i][..];
    }
    billing_log = log;
}

atomic void
patch()
{
}
