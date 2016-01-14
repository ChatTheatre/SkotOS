/*
**	~UserDB/obj/auth.c
**
**	Handle one authentication request...
**
**	Copyright Skotos Tech Inc 1999
*/

# include <kernel/user.h>
# include <UserDB.h>

private inherit "/lib/string";
private inherit "/lib/url";

inherit "/usr/System/lib/output_buffer";

string challenge;

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

# define ERR(str) { send_message(ix + " ERR " + (str) + "\n"); return; }
# define NERR(str) { send_message(ix + " ERR " + url_encode((string) str) + "\n"); return FALSE; }

# define OK(str) { send_message(ix + " OK " + (str) + "\n"); return; }
# define NOK(str) { send_message(ix + " OK " + url_encode((string) str) + "\n"); return FALSE; }


int receive_message(string line) {
   if (previous_program() == LIB_CONN) {
      return call_limited("receive_line", line);
   }
}

static
int receive_line(string line) {
   string cmd, err, rest, *args;
   string user, code;
   object node;
   int ix, i, ret, newcode;

   if (!strlen(line)) {
      return MODE_NOCHANGE;
   }
   if (line[0] == ':') {
      line = line[1 ..];
      if (sscanf(line, "%s %s %s %d %s", user, code, cmd, ix, rest) != 5) {
	 if (sscanf(line, "%s %s %s %d", user, code, cmd, ix) != 4) {
	    send_message("0 ERR BAD+INPUT\n");
	    return MODE_NOCHANGE;
	 }
      }
      user = url_decode(user);
      node = USERDB->query_user(user);
      newcode = TRUE;
   } else if (sscanf(line, "%s %d %s", cmd, ix, rest) != 3) {
      if (sscanf(line, "%s %d", cmd, ix) != 2) {
	 send_message("0 ERR BAD+INPUT\n");
	 return MODE_NOCHANGE;
      }
   }

   if (rest) {
      args = full_explode(rest, " ");
      for (i = 0; i < sizeof(args); i ++) {
	 args[i] = url_decode(args[i]);
      }
   } else {
      args = ({ });
   }

   if (newcode) {
      if (!function_object("newcmd_" + cmd, this_object())) {
	 send_message(ix + " ERR " +
		      url_encode("UNKNOWN CMD '" + cmd + "'") +
		      "\n");
	 return MODE_NOCHANGE;
      }
      /* new code */
      err = catch(ret = call_other(this_object(), "newcmd_" + cmd, ix, node,
				   node ? (code == node->get_keycode()):FALSE,
				   args...));
      if (err) {
	 DEBUG("line was: " + dump_value(line));
	 send_message(ix + " ERR " + err + "\n");
      } else if (ret) {
	 send_message(ix + " ERR " + url_encode("BAD KEYCODE") + "\n");
      }
      return MODE_NOCHANGE;
   }

   if (cmd != "keycodeauth" && cmd != "md5login" && cmd != "getprop") {
     SysLog("FILTERED OLD STYLE LINE: " + dump_value(line));
   }

   if (!function_object("cmd_" + cmd, this_object())) {
      send_message(ix + " ERR " +
		   url_encode("UNKNOWN CMD '" + cmd + "'") +
		   "\n");
      return MODE_NOCHANGE;
   }
   err = catch(call_other(this_object(), "cmd_" + cmd, ix, args...));
   if (err) {
      DEBUG("line was: " + dump_value(line));
      send_message(ix + " ERR " + url_encode(err) + "\n");
   }
   return MODE_NOCHANGE;
}

static
void cmd_md5login(int ix, string user, string hash) {
   object node;

   node = USERDB->query_user(user);
   if (!node) {
      ERR("NO SUCH USER");
   }
   if (node->query_account_status(AS_DELETED)) {
      ERR("NO SUCH USER");
   }
   if (node->test_hash(hash)) {
      OK(node->get_keycode());
   }
   ERR("BAD HASH");
}

static
void cmd_passlogin(int ix, string user, string pass) {
   object node;

   node = USERDB->query_user(user);
   if (!node) {
      ERR("NO SUCH USER");
   }
   if (node->query_account_status(AS_DELETED)) {
      ERR("NO SUCH USER");
   }
   if (node->test_password(pass)) {
      OK(node->get_keycode());
   }
   ERR("BAD PASSWORD");
}

string describe_account(object node)
{
    string type, *flags;

    type = node->describe_account_type();
    flags = node->describe_account_status();
    return " (" + type + ";" + implode(flags, " ") + ")";
}

static
void cmd_keycodeauth(int ix, string user, string code) {
   object node;

   node = USERDB->query_user(user);
   if (!node) {
      ERR("NO SUCH USER");
   }
   if (node->query_account_status(AS_DELETED)) {
      ERR("NO SUCH USER");
   }
   if (code != node->get_keycode()) {
     /* DEBUG(dump_value(({ ix, user, code, node->get_keycode() }))); */
      ERR("BAD KEYCODE");
   }
   if (USERDB->query_user_banned(node->query_name())) {
      DEBUG("BLOCKED: " + node->query_name());
      ERR("ACCOUNT BLOCKED");
   }
   if (!node->query_property("eighteen") &&
       !node->query_property("sixteen-waiver")) {
      ERR("YOUTH");
   }
   if (!node->query_property("terms-of-service")) {
      ERR("TOS");
   }
   if (!node->query_property("address-submit")) {
      ERR("ADDRESS");
   }
   if (!node->query_next_stamp()) {
      node->start_trial_month();
   }
   if (IS_FREE(node->query_account_type()) ||
       IS_STAFF(node->query_account_type()) ||
       IS_DEVELOPER(node->query_account_type())) {
      OK("PAID 0" + describe_account(node));
   }
   catch {
       node->check_credit();
   }
   if (node->query_paid_until() >= time()) {
      if (node->query_card()) {
	 OK("CCPAID " + node->query_paid_until() + describe_account(node));
      }
      OK("PAID " + node->query_paid_until() + describe_account(node));
   }
   if (node->query_trial_until() >= time()) {
      if (node->query_card()) {
	 /* we're in trial, but we've given a credit card */
	 OK("CCTRIAL " + node->query_trial_until() + describe_account(node));
      }
      OK("TRIAL " + node->query_trial_until() + describe_account(node));
   }
   if (node->query_card()) {
      OK("UNPAID" + describe_account(node));
   }
   ERR("UNPAID");
}

static
void cmd_challenge(int ix, string user) {
   object node;

   node = USERDB->query_user(user);
   if (!node) {
      ERR("NO SUCH USER");
   }
   if (node->query_account_status(AS_DELETED)) {
      ERR("NO SUCH USER");
   }
   OK(node->get_challenge());
}

static
void cmd_getprop(int ix, string user, string prop) {
   object node;
   mixed val;

   if (node = USERDB->query_user(user)) {
      if (val = node->query_property(prop)) {
	 OK(val);
      }
      OK("");
   }
   ERR("USER UNKNOWN");
}

static
void cmd_getbprop(int ix, string user, string prop) {
   object node;
   mixed val;

   if (node = USERDB->query_user(user)) {
      if (val = node->query_billing_property(prop)) {
	 OK(val);
      }
      OK("");
   }
   ERR("USER UNKNOWN");
}


static
void cmd_find_invalid_users(int ix, string user, string users...) {
   int i;

   for (i = 0; i < sizeof(users); i ++) {
      if (USERDB->query_user(users[i])) {
	 SysLog("Checking user: " + users[i] + ": EXISTS");
	 users[i] = nil;
      } else {
	 SysLog("Checking user: " + users[i] + ": DEAD");
	 users[i] = url_encode(users[i]);
      }
   }
   OK(implode(users - ({ nil }), " "));
}



static
int newcmd_md5login(int ix, object node, int auth, string hash) {
   if (!node) {
      return TRUE;
   }
   if (node->query_account_status(AS_DELETED)) {
      return TRUE;
   }
   if (node->test_hash(hash)) {
      NOK(node->get_keycode());
   }
   NERR("BAD HASH");
}

static
int newcmd_passlogin(int ix, object node, int auth, string pass) {
   if (!node) {
      return TRUE;
   }
   if (node->query_account_status(AS_DELETED)) {
      return TRUE;
   }
   if (node->test_password(pass)) {
      NOK(node->get_keycode());
   }
   NERR("BAD PASSWORD");
}


static
int do_auth(int ix, object node) {
   if (!node->query_property("eighteen") &&
       !node->query_property("sixteen-waiver")) {
      NERR("YOUTH");
   }
   if (!node->query_property("address-submit")) {
      NERR("ADDRESS");
   }
   if (!node->query_property("terms-of-service")) {
      NERR("TOS");
   }
   if (!node->query_next_stamp()) {
      node->start_trial_month();
   }
   if (IS_FREE(node->query_account_type()) ||
       IS_STAFF(node->query_account_type()) ||
       IS_DEVELOPER(node->query_account_type())) {
      NOK("PAID 0" + describe_account(node));
   }
   catch {
       node->check_credit();
   }
   if (node->query_paid_until() >= time()) {
      if (node->query_card()) {
	 NOK("CCPAID " + node->query_paid_until() + describe_account(node));
      }
      NOK("PAID " + node->query_paid_until() + describe_account(node));
   }
   if (node->query_trial_until() >= time()) {
      if (node->query_card()) {
	 /* we're in trial, but we've given a credit card */
	 NOK("CCTRIAL " + node->query_trial_until() + describe_account(node));
      }
      NOK("TRIAL " + node->query_trial_until() + describe_account(node));
   }
   if (node->query_card()) {
      SysLog("[" + node->query_name() + "] return OK UNPAID");
      NOK("UNPAID" + describe_account(node));
   }
   SysLog("[" + node->query_name() + "] return ERR UNPAID");
   NERR("UNPAID");
}

static
int newcmd_keycodeauth(int ix, object node, int auth) {
   if (!auth || !node) {
      return TRUE;
   }
   if (node->query_account_status(AS_DELETED)) {
      return TRUE;
   }
   if (USERDB->query_user_banned(node->query_name())) {
      DEBUG("BLOCKED: " + node->query_name());
      NERR("ACCOUNT BLOCKED");
   }
   return do_auth(ix, node);
}


static
int newcmd_passwordauth(int ix, object node, int auth, string pass) {
   if (!node) {
      return TRUE;
   }
   if (node->query_account_status(AS_DELETED)) {
      return TRUE;
   }
   if (node->test_password(pass)) {
      if (USERDB->query_user_banned(node->query_name())) {
	 DEBUG("BLOCKED: " + node->query_name());
	 NERR("ACCOUNT BLOCKED");
      }
      return do_auth(ix, node);
   }
   NERR("BAD PASSWORD");
}

static
int newcmd_md5auth(int ix, object node, int auth, string hash) {
   if (!node) {
      return TRUE;
   }
   if (node->query_account_status(AS_DELETED)) {
      return TRUE;
   }
   if (node->test_hash(hash)) {
      if (USERDB->query_user_banned(node->query_name())) {
	 DEBUG("BLOCKED: " + node->query_name());
	 NERR("ACCOUNT BLOCKED");
      }
      return do_auth(ix, node);
   }
   NERR("BAD HASH");
}

static
int newcmd_status(int ix, object node, int auth) {
   if (!node) {
      return TRUE;
   }
   if (node->query_account_status(AS_DELETED)) {
      return TRUE;
   }
   if (USERDB->query_user_banned(node->query_name())) {
     DEBUG("BLOCKED: " + node->query_name());
     NERR("ACCOUNT BLOCKED");
   }
   return do_auth(ix, node);
}

static
int newcmd_emailused(int ix, object node, int auth, string email) {
   /* since this is not protected, only return yes or no */
   if (USERDB->query_users_by_email(email)) {
      NOK("YES");
   }
   NOK("NO");
}


static
int newcmd_challenge(int ix, object node, int auth) {
   if (!node || !auth) {
      return TRUE;
   }
   NOK(node->get_challenge());
}

static
int newcmd_getprop(int ix, object node, int auth, string user, string prop) {
   object newnode;
   mixed val;

   if (!node || !auth) {
      return TRUE;
   }

   if (newnode = USERDB->query_user(user)) {
      if (node != newnode) {
	 error("security infraction");
      }
      if (val = newnode->query_property(prop)) {
	 NOK(val);
      }
      NOK("");
   }
   NERR("USER UNKNOWN");
}

static
int newcmd_getbprop(int ix, object node, int auth, string user, string prop) {
   object newnode;
   mixed val;

   if (!node || !auth) {
      return TRUE;
   }

   if (newnode = USERDB->query_user(user)) {
      if (node != newnode) {
	 error("security infraction");
      }
      if (val = newnode->query_billing_property(prop)) {
	 NOK(val);
      }
      NOK("");
   }
   NERR("USER UNKNOWN");
}

static
int newcmd_setemail(int ix, object node, int auth, string email) {
    object newnode;

    if (!node || !auth) {
	return TRUE;
    }
    DEBUG(node->query_name() + ": (Re)setting e-mail address to " + email);
    node->set_email(email);
    NOK("OK");
}
