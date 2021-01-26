/*
**	~UserDB/obj/ctl.c
**
**	Handle one actual control connection; see ctld.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <kernel/user.h>
# include <System.h>
# include <UserDB.h>

private inherit "/lib/string";
private inherit "/lib/url";

inherit "/usr/UserDB/lib/billapi";
inherit "/usr/System/lib/output_buffer";

/* This is what we know: */
private string remote_host;
private int    remote_port;
/* This is the name of the game, if we know it: */
private string remote_game;
/* This is the portbase of the game, if we know it: */
private int    remote_base;

static
void create(int clone) {
   ::create();
}

int
query_remote_base()
{
  return remote_base;
}

void set_remote(string host, int port) {
    if (previous_program() == "/usr/UserDB/sys/ctld") {
	remote_host = host;
	remote_port = port;
    }
}

int login(string name) {
   if (previous_program() == LIB_CONN) {
      connection(previous_object());
      return MODE_LINE;
   }
}

void logout(int dest) {
   if (previous_program() == LIB_CONN) {
      destruct_object();
   }
}

# define ERR(str)	{ send_message(ix + " ERR " + (str) + "\n"); return; }
# define OK(str)	{ send_message(ix + " OK " + (str) + "\n"); return; }

int receive_message(string line) {
   if (previous_program() == LIB_CONN) {
      return call_limited("receive_line", line);
   }
}

static
int receive_line(string line) {
   string cmd, err, *args;
   int ix, i;

   if (sscanf(line, "%s %d %s", cmd, ix, line) < 3) {
       if (sscanf(line, "%s %d%s", cmd, ix, line) < 3 || strlen(line)) {
	   error("CtlD bad input: " + line);
       }
       args = ({ });
       line = "";
   } else {
       args = full_explode(line, " ");
   }

   for (i = 0; i < sizeof(args); i ++) {
      args[i] = url_decode(args[i]);
   }
   if (!function_object("cmd_" + cmd, this_object())) {
      send_message(ix + " ERR UNKNOWN CMD '" + cmd + "'\n");
      return MODE_NOCHANGE;
   }
   err = catch(call_other(this_object(), "cmd_" + cmd, ix, args...));
   if (err) {
      DEBUG("line was: " + dump_value(line));
      send_message(ix + " ERR " + err + "\n");
   }
   return MODE_NOCHANGE;
}

static
void cmd_create(int ix, string user, string password, string email) {
   object node;

   if (USERDB->query_user(user)) {
      ERR("user exists");
   }
   if (USERDB->query_purged_user(user)) {
      /* We'll pretend the user still exists. */
      ERR("user exists");
   }
   SysLog("OK, making user " + dump_value(user));

   node = USERDB->make_user(user, password, email);
   node->start_trial_month();
   OK(node->get_keycode());
}

static
void cmd_setpwd(int ix, string user, string old, string newpw) {
   object node;

   node = USERDB->query_user(user);
   if (!node) {
      ERR("no such user");
   }
   if (!node->test_password(old)) {
      ERR("wrong old password");
   }
   SysLog("Changing password for " + user);
   node->set_password(newpw);
   OK("SET");
}

/*
 * Used by the 'send me my email' web-page, be very careful with this!
 */
static
void cmd_getpwd(int ix, string user) {
    object node;

    node = USERDB->query_user(user);
    if (!node) {
        ERR("no such user");
    }
    OK(url_encode(node->query_password()));
}

static
void cmd_getprop(int ix, string user, string prop) {
   object node;
   mixed val;

   if (node = USERDB->query_user(user)) {
      if (val = node->query_property(prop)) {
	 OK(url_encode((string)val));
      }
      OK("");
   }
   ERR("USER UNKNOWN");
}

static
void cmd_setprop(int ix, string user, string prop, string val) {
   object node;

   if (node = USERDB->query_user(user)) {
      if (!strlen(val)) {
	 node->clear_property(prop);
      } else {
	 node->set_property(prop, val);
      }
      OK("SET");
   }
   ERR("USER UNKNOWN");
}

static
void cmd_clearprop(int ix, string user, string prop) {
   object node;

   if (node = USERDB->query_user(user)) {
      node->clear_property(prop);
      OK("CLEARED");
   }
   ERR("USER UNKNOWN");
}

static
void cmd_announce(int ix, string hostname, string portbase) {
    if (remote_base) {
	/* Already have that information, what's happening? */
	OK("IGNORED");
	return;
    }
    /*
     * Can't know which exact game this is yet since we need to hear what
     * theatre it is before deciding, now.
     */
    remote_base = (int)portbase;
    OK("OK");
}

/*
 * To announce that someone has created a character on a game.
 * Required for the NoChar statistics.
 */
static
void cmd_ccreated(int ix, string user, string when, varargs string theatre) {
    string game;

    if (theatre && !strlen(theatre)) {
      theatre = nil;
    }
    game = GAMEDB->query_game_by_host(remote_host, remote_base, theatre);
    if (game) {
	object node;

	if (node = USERDB->query_user(user)) {
	    node->ccreated(game, when);
	    OK("NOTED");
	}
	ERR("USER UNKNOWN");
    }
    DEBUG("[" + ur_name(this_object()) + " - " + (remote_host ? remote_host : "?") + "] :: " +
	  "ccreated(" + ix + ", " + user + ", " + when + ")");
    OK("IGNORED");
}

/*
 * To announce that someone has played a certain character for a given
 * amount of time on a game.
 * Required for NoPlay statistics and royalty calculations.
 */
static void
cmd_cplayed(int ix, string user, string when, string duration,
	    varargs string theatre, string character)
{
    string game;

    if (theatre == "" || theatre == "nil") {
      theatre = nil;
    }
    game = GAMEDB->query_game_by_host(remote_host, remote_base, theatre);
    if (game) {
	object node;

	if (node = USERDB->query_user(user)) {
	    node->cplayed(game, when, duration);
	    OK("NOTED");
	}
	ERR("USER UNKNOWN");
    }
    DEBUG("[" + ur_name(this_object()) + " - " + (remote_host ? remote_host : "?") + ":" + (remote_base ? (string)remote_base : "?") + (theatre ? ":" + theatre : "") + "] :: " +
	  "cplayed(" + ix + ", " + user + ", " + when + ", " + duration +
	  (theatre ? ", " + theatre : "") + ")");
    OK("IGNORED");
}

/*
 * Query certain game/usage/billing statistics, only available to users that
 * have a wiztool on _HERE_, to avoid handing that information too easily.
 */
static
void cmd_statistics(int ix, string user, string type) {
    if (DEV_USERD->query_wiztool(user)) {
        OK(url_encode(STATSDB->dump_statistics(type)));
    }
    ERR("No access.");
}

/*
 * Set the user's requested 'acclaim' of his/her top 3o of games.
 */
static
void cmd_acclaim(int ix, string user, string accl1, string accl2, string accl3)
{
    object node;

    if (node = USERDB->query_user(user)) {
	string err;

	DEBUG("[acclaim] " + user  + ": " +
	      accl1 + " / " + accl2 + " / " + accl3);
	if (err = STATSDB->set_royalties_acclaim(node, ({ (int)accl1, (int)accl2, (int)accl3 }))) {
	    ERR(url_encode(err));
	}
	OK("DONE");
    }
    ERR("USER UNKNOWN");
}

static void
cmd_search(int ix, string field, string type, varargs string pattern)
{
    object *list;

    if (!pattern) {
	pattern = type;
	type    = "exact";
    }
    switch (field) {
    case "creditcard":
    case "email":
    case "name":
    case "street1":
    case "street2":
    case "city":
    case "state":
    case "zip":
	switch (type) {
	case "infix":
	case "exact":
	    list = SEARCHDB->search(field, type, pattern);
	    if (list && sizeof(list)) {
		int i, sz;
		string *result;

		sz = sizeof(list);
		result = allocate(sz);
		for (i = 0; i < sz; i++) {
		    result[i] = url_encode(list[i]->query_name());
		}
		OK(implode(result, " "));
	    } else {
		OK("");
	    }
	default:
	    break;
	}
    default:
	break;
    }
    ERR("NOT YET IMPLEMENTED");
}

static void
cmd_billexternal(int ix, string user, string months, string amount, string who)
{
    object node;

    if (node = USERDB->query_user(user)) {
	node->external_payment((float)amount, (int)months);
	DEBUG("billexternal:" + who + ":" + user + ":" + months + ":" + amount);
	OK("DONE");
    }
    ERR("USER UNKNOWN");
}

static void
cmd_billcredit(int ix, string user, string months, string who)
{
    object node;

    if (node = USERDB->query_user(user)) {
	node->bump_months((int)months);
	DEBUG("billcredit:" + who + ":" + user + ":" + months);
	OK("DONE");
    }
    ERR("USER UNKNOWN");
}

static void
cmd_ce_creditexternal(int ix, string user, string account_type, string account_period, string amount, string who)
{
    object node;

    if (node = USERDB->query_user(user)) {
	node->external_credit((float)amount, account_type, account_period);
	DEBUG("ce_creditexternal:" + who + ":" + user + ":" + account_type + ":" + account_period + ":" + amount);
	OK("DONE");
    }
    ERR("USER UNKNOWN");
}

static void
cmd_ce_debitexternal(int ix, string user, string amount, string reason, string who)
{
    object node;

    if (node = USERDB->query_user(user)) {
	node->external_debit((float)amount, reason);
	DEBUG("ce_debitexternal:" + who + ":" + user + ":" + amount + ":" + reason);
	OK("DONE");
    }
    ERR("USER UNKNOWN");
}

static void
cmd_ce_upgradeexternal(int ix, string user, string amount, string who)
{
    object node;

    if (node = USERDB->query_user(user)) {
	node->external_upgrade((float)amount);
	DEBUG("ce_upgradeexternal:" + who + ":" + user + ":" + amount);
	OK("DONE");
    }
    ERR("USER UNKNOWN");
}

static void
cmd_ce_billing(int ix, string cmd, varargs string args...)
{
    switch (cmd) {
    case "history": {
	int     i, sz_i;
	string  *result;
	mixed   ***data;

	switch (sizeof(args)) {
	case 0:
	    ERR("MISSING PARAMETERS");
	case 1:
	    data = BILLINGD->query_history((int)args[0], time() + 1);
	    break;
	case 2:
	default:
	    data = BILLINGD->query_history((int)args[0], (int)args[1]);
	    break;
	}
	result = ({ });
	sz_i = sizeof(data);
	for (i = 0; i < sz_i; i++) {
	    int   j, sz_j;
	    mixed **entries;

	    entries = data[i];
	    sz_j = sizeof(entries);
	    for (j = 0; j < sz_j; j++) {
		result += ({ url_encode(entries[j][0] + ":" +
					entries[j][1] + ":" +
					BILLINGD->describe_history_event(entries[j][3])) });
	    }
	}
	OK(implode(result, " "));
    }
    }
}


static void
cmd_ce_storypoints(int ix, string cmd, varargs string args...)
{
    switch (cmd) {
    case "toplist": {
	int     i, sz, *scores, rank;
	string  *result;
	mapping *users, map;

	map    = POINTSDB->query_toplist_map();
	result = ({ });
	sz     = map_sizeof(map);
	scores = map_indices(map);
	users  = map_values(map);

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
	    result += ({ url_encode(rank + ":" +
				    scores[i] + ":" +
				    implode(names, " ")) });
	    rank += sz_j;
	}
	OK(implode(result, " "));
    }
    case "history": {
	int     i, sz, *stamps;
	string  *result;
	mapping map;
	mixed   **data;

	map = POINTSDB->query_history_map();
	switch (sizeof(args)) {
	case 0:
	    break;
	case 1:
	    map = map[(int)args[0]..];
	    break;
	case 2:
	default:
	    map = map[(int)args[0]..(int)args[1]];
	    break;
	}
	sz     = map_sizeof(map);
	stamps = map_indices(map);
	data   = map_values(map);

	result = ({ });
	for (i = sz - 1; i >= 0; i--) {
	    int j, sz_j;

	    sz_j = sizeof(data[i]);
	    for (j = 0; j < sz_j; j++) {
		result += ({ url_encode((string)stamps[i]) + ":" +
			     url_encode((string)data[i][j][2]) + ":" +
			     url_encode(capitalize(data[i][j][1])) + ":" +
			     url_encode(capitalize(data[i][j][0])) });
	    }
	}
	OK(implode(result, " "));
    }
    }
}

static void
cmd_broadcast_all(int ix, string channel, string sub, string args...)
{
    int result;

    catch {
        result = BROADCAST->broadcast_all(remote_host, remote_base,
					  channel, sub, args...);
    }
    OK(result + " sent.");
}

static void
cmd_broadcast_one(int ix, string to_game, string channel, string sub,
		  string args...)
{
    int result;

    catch {
      DEBUG(dump_value(({ "broadcast_one", to_game, channel, sub }) + args));
        result = BROADCAST->broadcast_one(remote_host, remote_base,
					  to_game, channel, sub, args...);
    }
    OK(result + " sent.");
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


static
void cmd_ccauth(int ix, string user, string code, string ip,
		string acct, string expm, string expy, mixed amt) {
   mapping data;
   object node;

   amt = (float) amt;

   node = USERDB->query_user(user);
   if (!node || code != node->get_keycode()) {
      ERR("bad user/password");
   }
# if 0
   if (node->query_auth_order_id()) {
      ERR("There is already a credit card on record");
   }
# endif
   data = ([
      "cardnumber": acct,
      "expmonth":   expm,
      "expyear":    expy,
      "zip":        node->query_billing_property("zip"),
      "addr":       calculate_cc_addr(node->query_billing_property("street1")),
      /* "email": node->query_email(), */
      "ip":         ip,
      "userid":     user,
      "amount":     (string) node->query_cost(),
      "monthly":    node->query_monthly()   ? "true": nil,
      "quarterly":  node->query_quarterly() ? "true": nil,
      "yearly":     node->query_yearly()    ? "true": nil,
      "b_name":     node->query_billing_property("name"),
      "b_addr1":    node->query_billing_property("addr1"),
      "b_city":     node->query_billing_property("city"),
      "b_state":    node->query_billing_property("state"),
      "b_zip":      node->query_billing_property("zip"),
      "b_country":  node->query_billing_property("country"),
      "s_name":     node->query_property("name"),
      "s_addr1":    node->query_property("addr1"),
      "s_city":     node->query_property("city"),
      "s_state":    node->query_property("state"),
      "s_zip":      node->query_property("zip"),
      "s_country":  node->query_property("country"),
      "phone":      node->query_property("phone"),
      ]);
   bill_request("do_ccauth", ({ ix, node, data }), "PREAUTH", data, 60);
}

static
void do_ccauth(mapping reply, int ix, object user, mapping data) {
   string avs;

   XDebug("do_ccauth called! user = " + dump_value(user));
   XDebug("do_ccauth called! reply = " + dump_value(reply));
   XDebug("do_ccauth called! data = " + dump_value(data));

   catch {
      if (reply["approved"] && reply["approved"] == "APPROVED") {
	 /* check AVS */
	 if (avs = reply["avscode"]) {
	 }
	 user->successful_preauth(reply["oid"],
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
	 OK("PREAUTH");
      }
      user->unsuccessful_preauth(data["cardnumber"],
				 data["expmonth"], data["expyear"],
				 reply["error"] ? reply["error"] : "Unknown");
      if (reply["error"]) {
	 ERR(reply["error"]);
      }
   }
   ERR(dump_value(reply - ({ "approved" })));
}

static
void cmd_ccvoid(int ix, string user, string password) {
   mapping data;
   object node;

   if (node = USERDB->query_user(user)) {
# if 0
      if (!node->test_password(password)) {
	 ERR("bad user/password");
      }
# endif
      if (!node->query_auth_order_id()) {
	 ERR("there is no order to void");
      }
      data = ([ "oid": node->query_auth_order_id() ]);
      bill_request("do_ccvoid", ({ ix, node, data }), "VOIDSALE", data, 60);
      return;
   }
   ERR("unknown user");
}

static
void do_ccvoid(mapping reply, int ix, object user, mapping data) {
   string avs;

   XDebug("do_ccvoid called! user = " + dump_value(user));
   XDebug("do_ccvoid called! reply = " + dump_value(reply));
   XDebug("do_ccvoid called! data = " + dump_value(data));

   catch {
      if (reply["approved"] && reply["approved"] == "APPROVED") {
	 /* check AVS */

	 user->successful_voidsale(reply["oid"],
				   reply["ref"],
				   reply["code"],
				   reply["payserv"],
				   data["oid"]);
	 OK("VOIDSALE");
      }
      if (reply["error"]) {
	 ERR(reply["error"]);
      }
   }
   ERR(dump_value(reply - ({ "approved" })));
}

static
void cmd_ccclear(int ix, string user, string code, string ip) {
   object node;

   node = USERDB->query_user(user);
   if (!node || code != node->get_keycode()) {
      ERR("bad user/password");
   }
   if (!node->query_card()) {
      ERR("there is no creditcard to clear");
   }
   node->clear_card();
   OK("CLEARED");
}

static void
cmd_preauth(int ix, string user, string code, string ip,
	    string acct, string expm, string expy, string account_type, string account_period)
{
    object node;

    node = USERDB->query_user(user);
    if (!node) {
	ERR("UNKNOWN USER");
    }
    if (code != node->get_keycode()) {
	ERR("INCORRECT PASSWORD");
    }
    if (node->query_billing_flag()) {
	ERR("There's already a billing operation (" + node->query_billing_flag() + ") in progress");
    }
    /* Alright, let the user object do its magic now. */
    node->do_preauth(ip, acct, expm, expy, account_type, account_period,
		     this_object(), "done_preauth", ix);
}

void
done_preauth(int success, string extra, int ix)
{
    /* Did the magic work? */
    if (success) {
	OK("PREAUTH");
    }
    ERR(extra ? extra : "Unknown");
}

static void
cmd_upgrade(int ix, string user, string code, string ip,
	    string acct, string expm, string expy, string account_period)
{
    object  node;

    node = USERDB->query_user(user);
    if (!node) {
	ERR("UNKNOWN USER");
    }
    if (code != node->get_keycode()) {
	ERR("INVALID PASSWORD");
    }
    if (node->query_account_type() != AS_REGULAR && node->query_account_type() != AS_TRIAL) {
	ERR("NOT A PAYING/TRIAL ACCOUNT");
    }
    if (node->query_billing_flag()) {
	ERR("There's already a billing operation (" + node->query_billing_flag() + ") in progress");
    }
    /* Alright, let the user object do its magic now. */
    node->do_upgrade(ip, acct, expm, expy, account_period,
		     this_object(), "done_upgrade", ix);
}

void
done_upgrade(int success, string extra, int ix)
{
    /* Did the magic work? */
    if (success) {
	OK("UPGRADED");
    }
    ERR(extra ? extra : "Unknown");
}

static void
cmd_downgrade(int ix, string user, string code,
	      string account_period)
{
    object node;

    node = USERDB->query_user(user);
    if (!node) {
	ERR("UNKNOWN USER");
    }
    if (code != node->get_keycode()) {
	ERR("INVALID PASSWORD");
    }
    if (node->query_account_type() != AS_REGULAR && node->query_account_type() != AS_TRIAL) {
	ERR("NOT A PAYING/TRIAL ACCOUNT");
    }
    if (!node->do_downgrade(account_period)) {
	ERR("FAILED");
    }
    OK("DOWNGRADED");
}

/*
 * -----------------------------------------------------------------
 * Stuff that should only ever be used from troll.skotos.net, really
 * -----------------------------------------------------------------
 */

static void
cmd_ce_banned(int ix) {
    int    i, sz_i;
    string result;
    mixed  *names;

    names = USERDB->query_users_banned();
    sz_i = sizeof(names);
    if (!sz_i) {
	OK("");
    }
    result = "";
    for (i = 0; i < sz_i; i++) {
	int j, sz_j;

	for (j = 0, sz_j = sizeof(names[i]); j < sz_j; j++) {
	    result += " " + url_encode(names[i][j]);
	}
    }
    OK(result[1..]);
}

static void
cmd_ce_ccclear(int ix, string user, string who)
{
    object node;

    if (node = USERDB->query_user(user)) {
	if (!node->query_card()) {
	    ERR("NO CREDITCARD");
	}
	node->clear_card();
	DEBUG("ce_ccclear:" + who + ":" + user);
	OK("CLEARED");
    }
    ERR("USER UNKNOWN");
}

static void
cmd_ce_ban(int ix, string user, string who, string reason)
{
    object node;

    if (node = USERDB->query_user(user)) {
	if (USERDB->query_user_banned(user)) {
	    ERR("USER ALREADY BANNED");
	}
	USERDB->add_user_ban(user, who, reason);
	DEBUG("ce_ban:" + who + ":" + user);
	OK("BANNED");
    }
    ERR("USER UNKNOWN");
}

static void
cmd_ce_ban_clear(int ix, string user, string who)
{
    object node;

    DEBUG("user = " + dump_value(user));
    DEBUG("who = " + dump_value(who));
    if (node = USERDB->query_user(user)) {
	if (!USERDB->query_user_banned(user)) {
	    ERR("USER NOT BANNED");
	}
	USERDB->remove_user_ban(user);
	DEBUG("ce_ban_clear:" + who + ":" + user);
	OK("BAN CLEARED");
    }
    ERR("USER UNKNOWN");
}

static void
cmd_ce_setemail(int ix, string user, string who, string email)
{
    object node;

    DEBUG("ce_setemail(" + dump_value(({ ix, user, who, email })) + ")");
    if (node = USERDB->query_user(user)) {
	if (node->query_email() == email) {
	    OK("IDENTICAL");
	}
	DEBUG("ce_setemail:" + who + ":" + user + ":" +
	      (node->query_email() ? node->query_email() : "") + ":" +
	      email);
	if (email && strlen(email)) {
	    node->set_email(email);
	} else {
	    node->clear_email();
	}
	OK("SET");
    }
    ERR("USER UNKNOWN");
}

# define SCRATCHD "/usr/UserDB/sys/scratch"

static void
cmd_scratch(int ix, string cmd, varargs string args...)
{
    switch (cmd) {
    case "create":
	OK(SCRATCHD->create_entry((int)args[0]));
    case "purge":
	if (SCRATCHD->purge_entry(args[0])) {
	    OK("PURGED");
	}
	ERR("UNKNOWN ENTRY");
    case "store":
	if (SCRATCHD->store_data(args[0], args[1], args[2])) {
	    OK("STORED");
	}
	ERR("UNKNOWN ENTRY");
    case "clear":
	if (SCRATCHD->store_data(args[0], args[1], nil)) {
	    OK("CLEARED");
	}
	ERR("UNKNOWN ENTRY");
    case "fetch": {
	mixed result;

	result = SCRATCHD->fetch_data(args[0], args[1]);
	if (result == 0) {
	    ERR("UNKNOWN ENTRY");
	}
	OK(result ? url_encode(result) : "");
    }
    default:
	ERR("UNKNOWN SUBCMD " + cmd);
    }
}

/*
 * We keep track of stuff using the following properties:
 *
 *  "referral:friend" : ({ <stamp>, <name>, <object>, <public>, <text> })
 *  "referral:mentor" : ({ <stamp>, <name>, <object>, <public>, <text> })
 *  "referral:reminder:friend": "<int>"
 *  "referral:reminder:mentor": "<int>"
 *
 *  "sps:log": ({
 *    ({ <stamp>, <type>, <points>, ...<extra>... }),
 *    ...
 *  })
 *  "sps:used"   : <integer>
 *
 * <type> can be, for now:
 *
 *   "friend"        - A friend referral. [1 point]
 *   "friend:paid"   - The friend made their first payment. [20 points]
 *   "mentor"        - A mentor referral. [50 points]
 *
 * For those three the ...<extra>... bit will be:
 *
 *   <name>, <object>, <public>, <text>
 */

static atomic void
cmd_storypoints(int ix, string user, string cmd, varargs string args...)
{
    object node;

    if (node = USERDB->query_user(user)) {
        mixed val;

	if (cmd != "query") {
	    DEBUG("[STORYPOINTS] " + dump_value(({ ix, user, cmd }) + args));
	}
	switch (cmd) {
	case "query":
	    if (sizeof(args) < 1) {
	        ERR("MISSING PARAMETER");
	    }
	    switch (args[0]) {
	    case "log":
	    case "log:full": {
		int stamp;

		if (sizeof(args) != 2) {
		    ERR("MISSING PARAMETER");
		}

		stamp = time() - (int)args[1];
		val = node->query_property("sps:log");
		if (val) {
		    int    i, sz;
		    string *output;

		    sz     = sizeof(val);
		    output = ({ });

		    for (i = 0; i < sz; i++) {
			string s_year, s_mon, s_day, s_ctime;

			if (val[i][0] < stamp) {
			    /* Too old to show for this request. */
			    continue;
			}
			s_ctime = ctime(val[i][0]);
			s_day  = s_ctime[8..9];
			s_mon  = s_ctime[4..6];
			s_year = s_ctime[20..];

			switch (val[i][1]) {
			case "friend":
			    output += ({
			      s_mon + "/" + s_day + "/" + s_year,
				(string)val[i][2],
				(val[i][5] || args[0] == "log:full") ?
				capitalize(val[i][3]) : "Anonymous",
				"Referral of New User",
				val[i][6] ? val[i][6] : ""
				});;
			    break;
			case "friend:paid":
			    output += ({
			      s_mon + "/" + s_day + "/" + s_year,
				(string)val[i][2],
				(val[i][5] || args[0] == "log:full") ?
				capitalize(val[i][3]) : "Anonymous",
				"Referral of Paying User",
				val[i][6] ? val[i][6] : ""
				});;
			    break;
			case "mentor":
			    output += ({
			      s_mon + "/" + s_day + "/" + s_year,
				(string)val[i][2],
				(val[i][5] || args[0] == "log:full") ?
				capitalize(val[i][3]) : "Anonymous",
				"Mentorship of New User",
				val[i][6] ? val[i][6] : ""
				});;
			    break;
			default:
			    output += ({
			      s_mon + "/" + s_day + "/" + s_year,
				(string)val[i][2],
				(val[i][5] || args[0] == "log:full") ?
				capitalize(val[i][3]) : "Anonymous",
				"Other: " + val[i][1],
				val[i][6] ? val[i][6] : ""
				});;
			    break;
			    
			}
		    }
		    sz = sizeof(output);
		    for (i = 0; i < sz; i++) {
		        output[i] = url_encode(output[i]);
		    }
		    OK(implode(output, " "));
		} else {
		    OK("");
		}
	    }
	    case "total":
		OK(node->query_property("storypoints:total"));
	    case "used":
		OK(node->query_property("storypoints:used"));
	    case "available":
		OK(node->query_property("storypoints:available"));
	    case "eligible":
		OK(node->query_property("storypoints:eligible"));
	    default:
		ERR("QUE?");
	    }
	case "add": {
	    /*
	     * storypoints <user> add <points> <reason> <from> <anonymous>
	     *                        <extra>
	     */
	    int    t, points, anonymous;
	    string reason, from, extra;
	    mixed  **log;

	    if (sizeof(args) != 5) {
		ERR("INCORRECT NUMBER OF PARAMETERS");
	    }
	    points    = (int)args[0];
	    reason    = args[1];
	    from      = args[2];
	    anonymous = !!(int)args[3];
	    extra     = args[4];

	    log = node->query_property("sps:log");
	    if (!log) {
		log = ({ });
	    }
	    log += ({ ({
		t = time(),
		reason,
		points,
		from,
		nil,
		anonymous,
		(extra && strlen(extra)) ? extra : nil
	    }) });
	    node->set_property("sps:log", log);
	    POINTSDB->add_entry(t, points, reason, nil, from,
				node, node->query_name());
	    OK("ADDED");
	}
	default:
	    ERR("QUE?");
	}
    } else {
	ERR("UNKNOWN USER");
    }
}

static atomic void
cmd_referral(int ix, string user, string cmd, varargs string args...)
{
    object node;

    if (node = USERDB->query_user(user)) {
	if (cmd != "query") {
	    DEBUG("[REFERRAL] " + dump_value(({ ix, user, cmd }) + args));
	}
	switch (cmd) {
	case "query": {
	    mixed val;

	    switch (args[0]) {
	    case "reminder:friend":
	    case "reminder:mentor":
	        val = node->query_property("referral:" + args[0]);
		if (val) {
		    OK(val);
		}
		OK(0);
	    case "friend":
	    case "mentor":
	        val = node->query_property("referral:" + args[0]);
		if (val) {
		    OK(val[1]);
		}
		ERR("UNSET");
	    case "origin":
	        val = node->query_property("referral:" + args[0]);
		if (val) {
		  OK(val);
		}
		ERR("UNSET");
	    default:
	        ERR("UNKNOWN QUERY PARAMETER");
	    }
	    break;
	}
	case "clear":
	    if (sizeof(args) != 1) {
	        ERR("MISSING PARAMETER");
	    }
	    switch (args[0]) {
	    case "reminder:friend":
	    case "reminder:mentor":
		if (node->query_property("referral:" + args[0])) {
		    node->set_property("referral:" + args[0], nil);
		    OK("CLEARED");
		}
		ERR("UNSET");
	    default:
	        ERR("UNKNOWN CLEAR PARAMETER");
	    }
	    break;
	case "set":
	    switch (sizeof(args)) {
	    case 2:
		if (args[0] == "mentor" || args[0] == "friend") {
		    ERR("INSUFFICIENT SET PARAMETER(S)");
		}
		break;
	    case 4:
	        if (args[0] != "mentor" && args[0] != "friend") {
		    ERR("TOO MANY SET PARAMETERS");
		}
		break;
	    default:
	        ERR("INCORRECT NUMBER OF SET PARAMETERS");
	    }
	    switch (args[0]) {
	    case "origin":
	        if (node->query_property("referral:origin")) {
		    ERR("ALREADY SET");
		}
	        node->set_property("referral:origin", args[1]);
		OK("SET");
	    case "reminder:friend":
	    case "reminder:mentor":
		if (node->query_property("referral:" + args[0]) &&
		    node->query_property("referral:" + args[0]) != "-1") {
		    ERR("ALREADY SET");
		}
		node->set_property("referral:" + args[0], args[1]);
		OK("SET");
	    case "friend":
	    case "mentor": {
	        object node_other;

		node_other = USERDB->query_user(args[1]);
		if (node_other) {
		    int   t;
		    mixed **log;

		    if (node->query_property("referral:" + args[0])) {
		        ERR("ALREADY SET");
		    }

		    /*
		     * Store both object and name, to notice if
		     * {mentor|friend disappears has disappeared.
		     */
		    node->set_property("referral:" + args[0],
				       ({ time(),
					  node_other->query_name(),
					  node_other,
					  args[2] == "1",
					  args[3] == "" ? nil : args[3] }));

		    /* Clear a possible reminder. */
		    node->set_property("referral:reminder:" + args[0], nil);

		    /* Update the StoryPoint log. */
		    log = node_other->query_property("sps:log");
		    if (!log) {
		        log = ({ });
		    }
		    log += ({ ({
			t = time(),
			args[0],
			args[0] == "friend" ? 1 : 50,
			node->query_name(),
			node,
			args[2] == "1",
			args[3] == "" ? nil : args[3]
		    }) });
		    node_other->set_property("sps:log", log);
		    catch {
		      POINTSDB->add_entry(t, args[0] == "friend" ? 1 : 50,
					  args[0], node, node->query_name(),
					  node_other,
					  node_other->query_name());
		    }
		    OK("SET");
		}
	        ERR("UNKNOWN FRIEND");
	    }
	    default:
	        ERR("UNKNOWN SET PARAMETER");
	    }
	    break;
	default:
	    ERR("UNKNOWN SUBCMD " + cmd);
	}
    } else {
        ERR("USER UNKNOWN");
    }
}

static
void cmd_validate(int ix, string username) {
    int i;
    username = lower_case(username);
    if (USERDB->query_user(username)) {
	ERR("EXISTS");
    }
    if (USERDB->query_purged_user(username)) {
        ERR("PURGED");
    }
    if (strlen(username) < 4) {
        ERR("TOO SHORT");
    }
    if (strlen(username) > 30) {
        ERR("TOO LONG");
    }
    switch (username[0]) {
    case 'a'..'z':
        /* Ok, fine. */
        break;
    default:
        ERR("INITIAL");
    }
    for (i = 1; i < strlen(username); i++) {
        switch (username[i]) {
	case 'a'..'z':
	case '_':
	case '0'..'9':
	    break;
	default:
	    ERR("INVALID:" + (i + 1));
	}
    }
    OK("OK");
}

static
void cmd_account(int ix, string user, string cmd, varargs string args...)
{
    object node;

    DEBUG("account " + dump_value(({ ix, user, cmd }) + args));
    node = USERDB->query_user(user);
    if (!node) {
        ERR("NO SUCH USER");
    }
    switch (cmd) {
    case "get":
        if (sizeof(args) != 1) {
	    ERR("MISSING GET ARGUMENT");
	}
	switch (args[0]) {
	case "type":
	    OK(node->describe_account_type());
	case "status":
	    OK(implode(node->describe_account_status(), " "));
	default:
	    ERR("UNKNOWN GET PARAMETER");
	}
    case "set":
	if (sizeof(args) != 2) {
	    ERR("MISSING SET ARGUMENT(S)");
	}
	switch (args[0]) {
	case "type":
	    switch (args[1]) {
	    case "free":
		node->set_account_type(AS_FREE);
		OK("SET");
	    case "developer":
		node->set_account_type(AS_DEVELOPER);
		OK("SET");
	    case "regular":
		node->set_account_type(AS_REGULAR);
		OK("SET");
	    case "trial":
		node->set_account_type(AS_TRIAL);
		OK("SET");
	    case "staff":
		node->set_account_type(AS_STAFF);
		OK("SET");
	    default:
		ERR("UNKNOWN SET VALUE");
	    }
	case "status":
	    switch (args[1]) {
	    case "banned":
	        node->set_account_status(AS_BANNED);
		OK("SET");
	    case "deleted":
	        node->set_account_status(AS_DELETED);
		OK("SET");
	    case "no-email":
	        node->set_account_status(AS_NO_EMAIL);
		OK("SET");
	    case "premium":
	        node->set_account_status(AS_PREMIUM);
		OK("SET");
	    default:
		ERR("UNKNOWN SET VALUE");
	    }
	default:
	    ERR("UNKNOWN SET PARAMETER");
	}
    case "clear":
	if (sizeof(args) != 2) {
	    ERR("MISSING CLEAR ARGUMENT(S)");
	}
	switch (args[0]) {
	case "status":
	    switch (args[1]) {
	    case "banned":
		node->clear_account_status(AS_BANNED);
		OK("CLEARED");
	    case "deleted":
		node->clear_account_status(AS_DELETED);
		OK("CLEARED");
	    case "no-email":
		node->clear_account_status(AS_NO_EMAIL);
		OK("CLEARED");
	    case "premium":
		node->clear_account_status(AS_PREMIUM);
		OK("CLEARED");
	    default:
	        ERR("UNKNOWN CLEAR VALUE");
	    }
	default:
	    ERR("UNKNOWN CLEAR PARAMETER");
	}
    case "trial":
	if (!node->query_property("trial_offer")) {
	    ERR("There was no free trial offer for this account or the offer has expired.");
	}
	DEBUG(node->query_name() + ":Claimed free trial month.");
	node->set_property("trial_offer", nil);
	node->set_account_type(AS_TRIAL);
	node->bump_months(1);
	/*
	 * If they got here that means they received an email from us and as a
	 * result their email account should be considered 'alive'.
	 */
	node->clear_property("email-ping:stamp");
	node->clear_property("email-ping:code");
	node->set_property("email-ping:ack", time());
	OK("DONE");
    case "stats-mixed": {
	/*
	 * Return playing statistics for the account, but unlike "stats" it'll
	 * mix the games and give the results purely in time-order so that you
	 * get a better idea of when someone spends time where.
	 */
	int i, sz_i;
	string *games;
	mapping map, *data;

	if (sizeof(args) == 0) {
	    map = node->query_game_stats_slice(time() - 30 * 86400, time());
	} else {
	    map = node->query_game_stats_slice(time() - (int)args[0], time());
	}
	/* ... */
	ERR("Not Yet Implemented");
    }
    case "stats": {
	/*
	 * Return playing statistics for the account.  Looks a bit out of place
	 * but I didn't feel like adding yet another cmd_* function just for
	 * this.
	 */
	int     i, sz_i;
	string  *games;
	mapping map, *data;

	if (sizeof(args) == 0) {
	    /*
	     * Figure out the data for the past 30 days by default.
	     */
	    map = node->query_game_stats_slice(time() - 30 * 86400, time());
	} else {
	    map = node->query_game_stats_slice(time() - (int)args[0], time());
	}
	sz_i  = map_sizeof(map);
	games = map_indices(map);
	data  = map_values(map);
	for (i = 0; i < sz_i; i++) {
	    int j, sz_j;
	    mixed *val;

	    sz_j = map_sizeof(data[i]);
	    val  = map_values(data[i]);
	    for (j = 0; j < sz_j; j++) {
		int k, sz_k;
		mixed *list;

		sz_k = sizeof(val[j]);
		list = allocate(sz_k);
		if (sz_k) {
		    for (k = 0; k < sz_k; k++) {
			list[k] = (string)val[j][k];
		    }
		    val[j] = implode(list, ",");
		} else {
		    val[j] = nil;
		}
	    }
	    val -= ({ nil });
	    if (sizeof(val)) {
		games[i] += "," + implode(val, ",");
	    } else {
		games[i] = nil;
	    }
	}
	OK(implode(games - ({ nil }), " "));
    }
    default:
        ERR("UNKNOWN SUBCOMMAND");
    }
}


static
void cmd_staff(int ix, string user)
{
    object node;

    node = USERDB->query_user(user);
    if (!node) {
        ERR("NO SUCH USER");
    }
    OK((string) USERDB->query_staff_user(node->query_name()));
}

static
void cmd_emailping(int ix, string user, string cmd)
{
    object node;

    node = USERDB->query_user(user);
    if (!node) {
	ERR("NO SUCH USER");
    }
    switch (cmd) {
    case "send":
	/* Clear all (just in case) and try it (again). */
	node->clear_property("email-ping:stamp");
	node->clear_property("email-ping:code");
	node->clear_property("email-ping:ack");
	if (BILLINGD->send_email_ping(node)) {
	    OK("SENT");
	}
	ERR("NOT SENT");
    case "ack":
	node->clear_property("email-ping:stamp");
	node->clear_property("email-ping:code");
	node->set_property("email-ping:ack", time());
	OK("ACKNOWLEDGED");
    default:
	ERR("UNRECOGNIZED SUBCOMMAND");
    }
}

/*
 * Possible subcommands:
 *
 *   email add <address> ...
 *   email remove <address> ...
 *   email list <field> <value>
 *   email set <address> <field> <value>
 *   email clear <address> <field>
 *   email query <address>
 *   email no-email <address>
 */
static
void cmd_email(int ix, string cmd, varargs string args...)
{
    int i, sz, total;
    mapping initial;
    mixed *lists;

    switch (cmd) {
    case "add":
	sz = sizeof(args);
	initial =  ([ ]);
	for (i = 0, total = 0; i < sz; i++) {
	    total += EMAILD->add_email(args[i], initial);
	}
	OK(total + " ADDED");
    case "remove":
	sz = sizeof(args);
	for (i = 0, total = 0; i < sz; i++) {
	    total += EMAILD->remove_email(args[i]);
	}
	OK(total + " REMOVED");
    case "list":
	lists = EMAILD->query_list(args[0], args[1]);
	sz = sizeof(lists);
	for (i = 0; i < sz; i++) {
	    int j, sz_j;

	    lists[i] = lists[i][..];
	    sz_j = sizeof(lists[i]);
	    for (j = 0; j < sz_j; j++) {
		lists[i][j] = url_encode(lists[i][j]);
	    }
	    lists[i] = implode(lists[i], " ");
	}
	OK(implode(lists, " "));
    case "set":
	if (!EMAILD->update_email(args[0], args[1], args[2])) {
	    ERR("NO SUCH ADDRESS");
	}
	OK("FLAGGED");
    case "clear":
	if (!EMAILD->update_email(args[0], args[1], nil)) {
	    ERR("NO SUCH ADDRESS");
	}
	OK("FLAGGED");
    case "query":
	OK(EMAILD->query_email(args[0]));
    case "no-email":
	sz = sizeof(args);
	for (i = 0; i < sz; i++) {
	    /* Check if any accounts exist with this email address. */
	    int    j, sz_j;
	    object *list;

	    list = USERDB->query_users_by_email(args[i]);
	    sz_j = sizeof(list);
	    for (j = 0; j < sz_j; j++) {
		DEBUG("no-email:Marking " + list[j]->query_name() + ".");
		list[j]->set_account_status(AS_NO_EMAIL);
	    }
	    if (EMAILD->query_email(args[i])) {
		DEBUG("no-email:Marking " + args[i] + ".");
		EMAILD->update_email(args[i], "no-email", "1");
	    }
	}
	OK("DONE");
    }
}

static
void cmd_promo(int ix, string cmd, varargs string args...)
{
    switch (cmd) {
    case "list": {
	int i, sz;
	string *list;

	list = PROMOTIONS->query_promotions();
	sz = sizeof(list);
	for (i = 0; i < sz; i++) {
	    list[i] = url_encode(list[i]);
	}
	OK(implode(list, " "));
    }
    case "get": {
	int     i, sz;
	mixed   *ind, *val, value;
	mapping data;

	if (sizeof(args) != 2) {
	    ERR("Missing parameter");
	}
	data = PROMOTIONS->query_promotion(args[0]);
	if (!data) {
	    ERR("No such promotion");
	}
	value = data[args[1]];
	switch (typeof(value)) {
	case T_NIL:
	    ERR("Not Found");
	case T_INT:
	case T_FLOAT:
	    OK((string)value);
	case T_STRING:
	    OK(url_encode(value));
	case T_OBJECT:
	    ERR("Invalid value");
	case T_ARRAY:
	    value = value[..];
	    sz = sizeof(value);
	    for (i = 0; i < sz; i++) {
		value[i] = url_encode(value[i]);
	    }
	    OK(implode(value, " "));
	case T_MAPPING:
	    sz  = map_sizeof(value);
	    ind = map_indices(value);
	    val = map_values(value);
	    value = allocate(sz * 2);
	    for (i = 0; i < sz; i++) {
		switch (typeof(ind[i])) {
		case T_INT:
		case T_FLOAT:
		    value[i * 2] = (string)ind[i];
		    break;
		case T_STRING:
		    value[i * 2] = url_encode(ind[i]);
		    break;
		default:
		    value[i * 2] = "";
		    break;
		}
		switch (typeof(val[i])) {
		case T_INT:
		case T_FLOAT:
		    value[i * 2 + 1] = (string)val[i];
		    break;
		case T_STRING:
		    value[i * 2 + 1] = url_encode(val[i]);
		    break;
		default:
		    value[i * 2 + 1] = "";
		    break;
		}
	    }
	    OK(implode(value, " "));
	}
    }
    case "batch":
	if (sizeof(args) < 1) {
	    ERR("Missing parameter(s)");
	}
	switch (args[0]) {
	case "list": {
	    int     i, sz, *ids;
	    string  *list, *titles;
	    mapping map;

	    map = PROMOTIONS->query_batches();
	    sz     = map_sizeof(map);
	    ids    = map_indices(map);
	    titles = map_values(map);

	    list = allocate(sz * 2);

	    for (i = 0; i < sz; i++) {
		list[i * 2]     = (string)ids[i];
		list[i * 2 + 1] = url_encode(titles[i]);
	    }
	    OK(implode(list, " "));
	}
	case "get": {
	    string text;

	    if (sizeof(args) != 2) {
		ERR("Missing parameter");
	    }
	    text = PROMOTIONS->query_batch((int)args[1]);
	    if (!text) {
		ERR("No such batch");
	    }
	    OK(url_encode(text));
	}
	default:
	    ERR("Command not understood");
	}
    default:
	ERR("Command not understood");
    }
}

#if 0
static
void cmd_usernotes(int ix, string user, string cmd, varargs string args...)
{
    int     i, sz, *list;
    object  node;
    mapping notes, *data;

    node = USERDB->query_user(user);
    if (!node) {
	ERR("NO SUCH USER");
    }
    switch (cmd) {
    case "list":
	if (sizeof(args) > 0) {
	    ERR("TOO MANY PARAMETERS");
	}
	notes = node->query_property("usernotes");
	if (!nodes) {
	    OK("");
	}
	sz = map_sizeof(notes);
	list = map_indices(notes);
	data = map_values(notes);
	for (i = 0; i < sz; i++) {
	}
	OK(implode(numbers, ""));
    case "get":
    case "set":
    case "remove":
    default:
	ERR("UNKNOWN SUBCOMMAND");
    }
}
#endif

static void
cmd_accountlist(int ix, string type)
{
    int     i, sz_i;
    mapping data, *maps;
    string  *names;

    switch (type) {
    case "staff":
    case "developer":
    case "free":
    case "cc":
	data = call_other(BILLINGD, "query_" + type + "_users");
	sz_i = map_sizeof(data);
	maps = map_values(data);
	names = allocate(sz_i);

	for (i = 0; i < sz_i; i++) {
	    int j, sz_j;
	    string *str;
	    object *list;

	    sz_j = map_sizeof(maps[i]);
	    list = map_values(maps[i]);
	    str = allocate(sz_j);
	    for (j = 0; j < sz_j; j++) {
		if (list[j]->query_user_ban()) {
		    continue;
		}
		str[j] = url_encode(list[j]->query_name());
	    }
	    if (sizeof(str) > 0) {
		names[i] = implode(str, " ");
	    }
	}
	names -= ({ nil });
	OK(implode(names, " "));
    case "regular":
	ERR("NOT YET IMPLEMENTED");
    case "premium":
    case "premium/regular": {
	data = BILLINGD->query_premium_users();

	sz_i = map_sizeof(data);
	maps = map_values(data);
	names = allocate(sz_i);

	for (i = 0; i < sz_i; i++) {
	    int j, sz_j;
	    string *str;
	    object *list;

	    sz_j = map_sizeof(maps[i]);
	    list = map_values(maps[i]);
	    str = allocate(sz_j);
	    for (j = 0; j < sz_j; j++) {
		if (list[j]->query_user_ban()) {
		    continue;
		}
		if (list[j]->query_card() || list[j]->query_next_stamp() > time()) {
		    if (type == "premium" || list[j]->query_account_type() == AS_REGULAR) {
			str[j] = url_encode(list[j]->query_name());
		    }
		}
	    }
	    str -= ({ nil });
	    if (sizeof(str) > 0) {
		names[i] = implode(str, " ");
	    }
	}
	names -= ({ nil });
	OK(implode(names, " "));
    }
    default:
	ERR("TYPE NOT SUPPORTED/RECOGNIZED");
    }
}

/*
 * The slightly more restricted interface for external use.
 */
static atomic void
cmd_sps(int ix, string user, string cmd, varargs string total, string reason, string comment, string theatre)
{
    object node;

    node = USERDB->query_user(user);
    if (!node) {
	ERR("UNKNOWN USER");
    }
    switch (cmd) {
    case "total":
	OK(node->query_property("storypoints:total"));
    case "used":
	OK(node->query_property("storypoints:used"));
    case "eligible":
	OK(node->query_property("storypoints:eligible"));
    case "available":
	OK(node->query_property("storypoints:available"));
    case "add": {
	int    num, t;
	string game;
	mixed  **log;

	if (!total || !reason) {
	    ERR("INSUFFICIENT PARAMETERS");
	}
	num = (int)total;
	if (num <= 0) {
	    ERR("POINTS SHOULD BE POSITIVE");
	}
	if (!reason || !strlen(reason)) {
	    ERR("REASON IS NEEDED");
	}
	game = GAMEDB->query_game_by_host(remote_host, remote_base, theatre);
	if (game) {
	    game = GAMEDB->query_game_by_name(game)[4];
	} else {
	    game = "Unknown";
	}
	game = "Game: " + game;
	log = node->query_property("sps:log");
	if (!log) {
	    log = ({ });
	}
	log += ({ ({
	    t = time(),
	    reason,
	    num,
	    game,
	    nil,
	    TRUE,
	    (comment && strlen(comment)) ? comment : nil
	}) });
	node->set_property("sps:log", log);
	POINTSDB->add_entry(t, num, reason, nil, game, node, node->query_name());
	OK("NOTED");
    }
    case "use": {
	int    num, t, available;
	string game;
	mixed  **log;

	if (!total || !reason) {
	    ERR("INSUFFICIENT PARAMETERS");
	}
	num = (int)total;
	if (num <= 0) {
	    ERR("POINTS SHOULD BE POSITIVE");
	}
	available = (int)node->query_property("storypoints:available");
	if (num > available) {
	    ERR("INSUFFICIENT POINTS");
	}
	num = -num;
	if (!reason || !strlen(reason)) {
	    ERR("REASON IS NEEDED");
	}
	game = GAMEDB->query_game_by_host(remote_host, remote_base, theatre);
	if (game) {
	    game = GAMEDB->query_game_by_name(game)[4];
	} else {
	    game = "Unknown";
	}
	game = "Game: " + game;
	log = node->query_property("sps:log");
	if (!log) {
	    log = ({ });
	}
	log += ({ ({
	    t = time(),
	    reason,
	    num,
	    game,
	    nil,
	    TRUE,
	    (comment && strlen(comment)) ? comment : nil
	}) });
	node->set_property("sps:log", log);
	POINTSDB->add_entry(t, num, reason, nil, game, node, node->query_name());
	OK("NOTED");
    }
    default:
	ERR("QUE");
    }
}
