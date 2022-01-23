/*
**	~UserAPI/obj/udat.c
**
**	Maintain data associated with a single user, such as their
**	memories, their user properties, the character(bodies) they
**	play, etc, etc.
**
**	Copyright Skotos Tech Inc 1999-2006
*/

# include <type.h>
# include <System.h>
# include <UserAPI.h>

# define LI_MAX 10

# define MARK_2002_11_01 1036137600

private inherit "/lib/string";
private inherit "/lib/array";

# define CLEAN()	{ if (member(nil, bodies)) bodies -= ({ nil }); }

inherit properties "/lib/properties";
inherit request    "/usr/UserAPI/lib/request";
inherit tip        "/usr/UserAPI/lib/tip";
inherit ctl        "/usr/UserAPI/lib/ctlapi";

string	name, email;
object *bodies;
mapping char_to_body;

int	creation_time;		/* when this user was created */
int	play_time;		/* when this user can first log on */
int	last_logon;		/* when this user last logged on */

int	last_day;		/* last 24-hour period logged on */
int	number_of_days;		/* # of 24-hour periods we've logged on */

string	last_client;
string	last_os;
string	last_browser;
mapping last_ip_numbers;        /* Keep track of last ipnrs, per category */

string  suspended;

mixed *notes;

mapping logged_in;		/* Keep track of counters and timestamps. */
mapping theatre_id;		/* Keep track of initial theatre-ids. */

string  account_type;  /* Legal values: 'regular','trial','free','developer','staff' */
mapping account_flags; /* Known flags: 'no-email', 'premium', 'grand', 'terms-of-service', 'deleted', 'banned' */

string query_state_root() { return "UserAPI:UDat"; }

static
void create(int clone) {
   if (clone) {
      properties::create();
      request::create();
      tip::create();
      ctl::create();

      bodies = ({ });

      last_client = last_os = last_browser = "unset";
      last_ip_numbers = ([ ]);

      logged_in = ([ ]);
      theatre_id = ([ ]);
       
      notes = ({ });
   }
}

/* calls from i.e. user object */

# define DAY	(3600 * 24)
# define WEEK	(DAY * 7)
# define MONTH	(DAY * 30)

static void
start_presence(string id, varargs int timer)
{
   if (!logged_in) {
      logged_in = ([ ]);
   }
   if (logged_in[id]) {
      logged_in[id][0]++;
   } else {
      logged_in[id] = ({ 1, time() });
   }
   if (timer > 0) {
      call_out("stop_presence", timer, id);
   }
}

static void
stop_presence(string id)
{
   int *data;

   if (!logged_in) {
      return;
   }
   data = logged_in[id];
   if (!data) {
      return;
   }
   data[0]--;
   if (data[0] > 0) {
      return;
   }
   start_cplayed("", name, data[1], time() - data[1], id ? id : "");
   logged_in[id] = nil;
}

private string
cached_theatre_id(object user)
{
    mixed id;

    if (theatre_id && theatre_id[user]) {
	return theatre_id[user];
    }

    id = user->query_body()->query_property("theatre:id");
    if (typeof(id) == T_STRING) {
       if (!theatre_id) {
	  theatre_id = ([ ]);
       }
       theatre_id[user] = id;

       return id;
    }
    return nil;
}

void logon(object user) {
    int    day;
    string id;

    last_logon = time();
    day = last_logon / DAY;
    if (day > last_day) {
	last_day = day;
	number_of_days ++;
    }

    start_presence(cached_theatre_id(user));
}

void logout(varargs object user) {
    stop_presence(cached_theatre_id(user));
}

void idle_stop(varargs object user) {
    start_presence(cached_theatre_id(user));
}

void idle_start(varargs object user) {
    stop_presence(cached_theatre_id(user));
}

void web_hit(string id) {
    DEBUG("WEBHIT:" + name + " @ " + id);
    /* 2.5 minutes, a guestimate at what is useful. */
    start_presence(id, 150);
}

void set_logon_count(string theatre, int i) {
    int count;

    count = logged_in[theatre] ? logged_in[theatre][0] : 0;
    if (i > count) {
	while (count != i) {
	    start_presence(theatre);
	    count = logged_in[theatre] ? logged_in[theatre][0] : 0;
	}
    } else if (i < count) {
	while (count != i) {
	    stop_presence(theatre);
	    count = logged_in[theatre] ? logged_in[theatre][0] : 0;
	}
    }
}

mapping *query_logon_data() {
    return ({ logged_in, theatre_id });
}

void patch_object_name()
{
   set_object_name("UserAPI:UDats:" + name[0 .. 0] + ":" + name);
}

atomic void set_name(string str) {
   if (previous_program() == UDATD) {
      name = str;
      patch_object_name();
   }
}

string query_name() {
   return name;
}


atomic
void set_email(string mail) {
   email = mail;
   UDATD->register_email(name, mail);
}

string query_email() {
   return email;
}



/* this is for the CHARACTER NAME and BODIES */

/* get rid of char_to_body; use bodies instead */
atomic void patch_bodies() {
   if (char_to_body) {
      string *chars;
      int i;

      chars =  map_indices(char_to_body);
      bodies = map_values(char_to_body);

      for (i = 0; i < sizeof(chars); i ++) {
	 UDATD->add_name_for_body(bodies[i], chars[i], TRUE);
      }
      char_to_body = nil;
   }
}


object *query_bodies() {
   patch_bodies();
   CLEAN();
   return bodies - ({ nil });
}

object query_body(string char) {
   string *names;
   int i;

   patch_bodies();

   char = lower_case(char);
   CLEAN();

   for (i = 0; i < sizeof(bodies); i ++) {
      names = UDATD->query_names_of_body(bodies[i]);
      if (names && member(char, names)) {
	 return bodies[i];
      }
   }
   return nil;
}

string *query_chars() {
   string *out, *names;
   int i;

   patch_bodies();

   bodies -= ({ nil });
   out = allocate(sizeof(bodies));
   for (i = 0; i < sizeof(bodies); i ++) {
      out[i] = UDATD->query_name_of_body(bodies[i]);
      if (!out[i]) {
	 out[i] = "???";
      }
   }
   return out;
}


# define MAX_CHARS	3

int query_basic_roster() {
   if (time() >= MARK_2002_11_01) {
      if (account_flags) {
	 if (account_flags["premium"]) {
	    return 5;
	 }
	 if (account_flags["grand"]) {
	    return 3;
	 }
      }
      return 2;
   }
   return MAX_CHARS;
}

int roster_is_full() {
   mixed extra;

   extra = query_property(ROSTER_EXTRA);
   if (!extra || typeof(extra) != T_INT) {
      extra = 0;
   }
   patch_bodies();
   CLEAN();
   return sizeof(bodies) >= (query_basic_roster() + extra) &&
      !DEV_USERD->query_wiztool(name);
}

int last_deletion;

int deletion_is_allowed() {
   patch_bodies();
   CLEAN();
   return sizeof(bodies) > query_basic_roster() || /* grandfather */
      time() - last_deletion > 24 * 3600 ||
      DEV_USERD->query_wiztool(name);
}

atomic
void add_body(object body, varargs int override) {
   if (map_sizeof(MODULED->query_modules() & ({ "Theatre" })) == 0 &&
       !override && roster_is_full()) {
      error("body roster is full");
   }
   CLEAN();

   if (!member(body, bodies)) {
      string id;

      /* Notify UserDB that this user created a character */
      id = body->query_property("theatre:id");
      start_ccreated("", name, time(), id ? id : "");

      bodies += ({ body });
   }
}

atomic
void del_body(object body) {
   patch_bodies();

   CLEAN();
   bodies -= ({ body });
   last_deletion = time();
}


/* timestamps */

void set_play_time(int time) {
   play_time = time;
}

int query_play_time() {
   return play_time;
}

void set_creation_time(int time) {
   creation_time = time;
}

int query_creation_time() {
   return creation_time;
}

void set_last_logon(int time) {
   last_logon = time;
}

int query_last_logon() {
   return last_logon;
}

int bump_number_of_days(int i) {
   number_of_days += i;
}

int query_number_of_days() {
   return number_of_days;
}

int query_total_days() {
   int time;

   if (number_of_days > 0) {
      time = 972334800;	/* when we started counting */

      if (creation_time > time) {
	 time = creation_time;
      }
      time -= (time % DAY);
      return (time() + DAY-1 - time) / DAY;
   }
   return 0;
}

float query_percent_of_days() {
   int tot;

   if (tot = query_total_days()) {
      return 100.0 * (float) query_number_of_days() / (float) query_total_days();
   }
   return -1.0;
}

void set_last_client(string str) {
   last_client = str;
}

string query_last_client() {
   return last_client;
}

void set_last_os(string str) {
   last_os = str;
}

string query_last_os() {
   return last_os;
}

void set_last_browser(string str) {
   last_browser = str;
}

string query_last_browser() {
   return last_browser;
}

void patch_ipdata()
{
   int i, sz_i;
   mixed ***data;

   if (!last_ip_numbers) {
      return;
   }
   sz_i = map_sizeof(last_ip_numbers);
   data = map_values(last_ip_numbers);
   for (i = 0; i < sz_i; i++) {
      int j, sz_j;

      sz_j = sizeof(data[i]);
      for (j = 0; j < sz_j; j++) {
	 SEARCHDB->add_reference("ipdata", data[i][j][0], this_object());
	 SEARCHDB->add_reference("ipdata", data[i][j][1], this_object());
      }
   }
}

void add_last_ipnumber(string category, string ipnr, string ipname)
{
    mixed **data;

    if (!last_ip_numbers) {
	last_ip_numbers = ([ ]);
    }
    data = last_ip_numbers[category];
    if (!data) {
	last_ip_numbers[category] = data = ({ ({ ipnr, ipname, time(), 1 }) });
	catch {
	   SEARCHDB->add_reference("ipdata", ipnr,   this_object());
	   SEARCHDB->add_reference("ipdata", ipname, this_object());
	}
    } else {
	if (ipnr == data[0][LI_IPNR]) {
	    data[0][LI_WHEN] = time();
	    data[0][LI_COUNT]++;
	} else {
	    data = ({ ({ ipnr, ipname, time(), 1 }) }) + data;
	    catch {
	       SEARCHDB->add_reference("ipdata", ipnr,   this_object());
	       SEARCHDB->add_reference("ipdata", ipname, this_object());
	    }
	    if (sizeof(data) > LI_MAX) {
	        catch {
		   int i, sz;
		   mixed **removed;

		   removed = data[LI_MAX ..];
		   sz = sizeof(removed);
		   for (i = 0; i < sz; i++) {
		      SEARCHDB->remove_reference("ipdata", removed[i][0], this_object());
		      SEARCHDB->remove_reference("ipdata", removed[i][1], this_object());
		   }
		}
		data = data[..LI_MAX - 1];
	    }
	    last_ip_numbers[category] = data;
	}
    }
}

mixed *
query_last_ipnumber(string category)
{
    mixed **data;

    if (!last_ip_numbers) {
	return nil;
    }
    data = last_ip_numbers[category];
    if (!data) {
	return nil;
    }
    return data[0][..];
}

mixed **
query_last_ip_numbers(string category)
{
   return last_ip_numbers[category];
}

string
dump_last_ipnumbers(string category)
{
   int    i, sz;
   string result;
   mixed  **data;

   data = last_ip_numbers[category];
   result = "";
   sz = sizeof(data);
   for (i = 0; i < sz; i++) {
      result +=
	 ctime(data[i][2]) + "  " + pad_left((string)data[i][3], 3) +
	 "  " + data[i][1] + "\n";
   }
   return result;
}

int
query_last_event(string ipentry)
{
   int    i, sz_i;
   mixed  ***data;

   sz_i = map_sizeof(last_ip_numbers);
   data = map_values(last_ip_numbers);
   for (i = 0; i < sz_i; i++) {
      int j, sz_j;

      sz_j = sizeof(data[i]);
      for (j = 0; j < sz_j; j++) {
	 if (data[i][j][0] == ipentry || data[i][j][1] == ipentry) {
	    return data[i][j][2];
	 }
      }
   }
   /* Should not get here, assuming the function was called appropriately. */
   return 0;
}

void
addnote(string note, string body, int timestamp) {
    notes += ({ ({ note, body, timestamp }) })
}

void
killnote() {
    notes = notes[.. sizeof(notes)-1];
}

mixed
*query_notes() {
    return notes;
}

void
set_suspended(string reason)
{
    suspended = reason;
}

string
query_suspended()
{
    return suspended;
}

string *query_property_indices(varargs int buggery) {
   return ::query_property_indices();
}

void update_account(string type, string *flags) {
   int i, sz;
   string code;

   account_type = type;
   account_flags = ([ ]);
   sz = sizeof(flags);
   for (i = 0; i < sz; i++) {
      account_flags[flags[i]] = TRUE;
   }

   /*
    * Update WOE/SH access:
    */
   if (code = SYS_INITD->query_access_code()) {
      if (account_flags[code]) {
	 if (!DEV_USERD->query_wiztool(name)) {
	    DEBUG("[udat::update_account] " + name + ": SH/WOE access added.");
	    DEV_USERD->set_developer(name);
	 }
      } else {
	 if (DEV_USERD->query_wiztool(name)) {
	    DEBUG("[udat::update_account] " + name + ": SH/WOE access removed.");
	    DEV_USERD->clear_developer(name);
	 }
      }
   }
}

string query_account_type() {
   return account_type ? account_type : "unknown";
}

int query_account_flag(string flag) {
   return account_flags ? !!account_flags[flag] : FALSE;
}

mapping query_account_flags() {
   return account_flags ? account_flags[..] : ([ ]);
}

void suicide() {
   call_out("die", 0);
}

void die() {
   destruct_object();
}

mixed query_downcased_property(string prop)
{
   string flag;

   if (prop == "skotos:name") {
      return name;
   }
   if (prop == "skotos:type") {
      return query_account_type();
   }
   if (sscanf(prop, "skotos:flags:%s", flag)) {
      return account_flags ? account_flags[flag] : nil;
   }
   return ::query_downcased_property(prop);
}

atomic void patch_cleanup() {
   string *ix;
   int i, cleaned;

   ix = query_property_indices();
   for (i = 0; i < sizeof(ix); i ++) {
      if (strlen(ix[i]) > 4 &&
	  ix[i][strlen(ix[i])-4 .. strlen(ix[i])-1] == "body") {
	 clear_downcased_property(ix[i]);
	 cleaned ++;
      }
   }
   if (cleaned > 0) {
      SysLog(name(this_object()) + ": properties cleaned: " + cleaned);
   }
}

void patch() {
   if (sscanf(ur_name(this_object()), "%*s#")) {
      if (name) {
         patch_object_name();
         UDATD->register_lost_udat(this_object());
      } else {
         SysLog("Destructing nameless udat " + ur_name(this_object()));
         destruct();
      }
   }
}
