/*
**	~TextIF/obj/user.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <config.h>
# include <kernel/kernel.h>
# include <kernel/user.h>
# include <kernel/rsrc.h>
# ifndef COMPAT_CODE
#   include <kernel/net.h>
# endif
# include <System.h>
# include <SkotOS.h>
# include <UserAPI.h>
# include <TextIF.h>
# include <base.h>

# include <SAM.h>

# define IDLE_TIMEOUT (60 * 60)

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/url";

inherit "/usr/System/lib/user";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/auth";
inherit "/usr/TextIF/lib/misc";
inherit "/usr/SAM/lib/sam";

inherit authapi "/usr/UserAPI/lib/authapi";
inherit tip_timer "/usr/SkotOS/lib/tip_timer"; 
inherit more      "/usr/TextIF/lib/more";

inherit sam "/usr/SAM/lib/sam";

inherit "/usr/TextIF/lib/markup";

int debug;		/* TODO debugging mode */
int newline;            /* Last output was a newline. */
int skotos_client;	/* can we use special protocol features? */
int html_client;	/* should we send HTML? */
int skoot_client;	/* can we tolerate SKOOT messages? */
string client_id;	/* how did the connecting client identify itself? */

string name;		/* user name */
string secret;		/* secret we send: used for hash */
object body;
int possessed;		/* did we possess the chosen body yet? */
NRef male, female, neuter;	/* last references, used in parser */
object udat;

object *bodies;		/* bodies to select from during logon */

private int need_banner;    /* Only used once. */

int idle_timestamp;
int idle_callout;

object input_object;    /* Where input should go instead of the regular */
string input_function;  /* command-parsing. */

private string freemote; /* Copy value from ~System/initd for efficiency */
private string helpverb; /* Copy value from ~System/initd for efficiency */
private string message_buffer;

string avchat_channel; /* Current Jitsi channel name for audio/video chat. */

void message(string str);
void html_message(string str);
void raw_line(string str);

void initialize_theme();

private void do_prompt();

string query_client_id() { return client_id; }

int query_html_client() { return html_client; }
int query_skoot_client() { return skoot_client; }

int query_skotos_client() { return skotos_client; }

object query_body() { return body; }
string query_describer() {
   return MAIN;
}

int query_host() { return name && is_root(name); }

string query_name() { return name; }
object query_udat() { return udat; }

void set_last_male(NRef ob) {
   male = ob;
}

void set_last_female(NRef ob) {
   female = ob;
}

void set_last_neuter(NRef ob) {
   neuter = ob;
}

NRef query_last_male() {
   return (male && NRefOb(male)) ? male : nil;
}

NRef query_last_female() {
   return (female && NRefOb(female)) ? female : nil;
}

NRef query_last_neuter() {
   return (neuter && NRefOb(neuter)) ? neuter : nil;
}

static
void create( int clone ) {
   if (clone) {
      authapi::create();
      tip_timer::create();
      more::create();

      need_banner = TRUE;
      freemote = SYS_INITD->query_freemote();
      helpverb = SYS_INITD->query_helpverb();
   }
}

int query_idle_timestamp() {
   return idle_timestamp;
}

int query_idled_out() {
   return idle_timestamp && !idle_callout;
}


string get_colour(string id) {
   string tid;
   object tdat;
   mixed res;

   if (!id) {
      return nil;
   }
   if (tid = body->query_property("client:theme")) {
      tdat = udat->query_property("themes:" + tid);
      if (typeof(tdat) != T_OBJECT) {
	 tdat = "/usr/TextIF/sys/themes"->query_property("themes:" + tid);
      }
      if (typeof(tdat) != T_OBJECT) {
	 return nil;
      }
      /* LWO containing display properties */
      return tdat->query_colour(lower_case(tid), lower_case(id));
   }
   return nil;
}

static
void idle_timeout() {
    idle_callout = 0;
    udat->idle_start(this_object());
}

void
set_input_to(object obj, string func)
{
   if (input_object || input_function) {
      error("set_input_to: Already set.");
   }
   input_object   = obj;
   input_function = func;
}

static void
input_relay(object body, object i_obj, string i_fun, string str)
{
   call_other(i_obj, "input_" + i_fun, this_object(), body, str);
}

void patch() {
   if (skotos_client) {
      skoot_client = TRUE;
   }
}

atomic
void possess_body(object b, varargs object l) {
   object *souls;
   SAM banner;
   int i;

   if (body) {
      body->depossess(this_object());
   }
   body = b;

   /* do this after body link made, but before we move */

   initialize_theme();

   if (need_banner) {
     need_banner = FALSE;
     if (banner = TEXT_INITD->query_banner()) {
	html_message(unsam(banner, nil, this_object()) + "\n");
     }
   }


   /* throw any other instances of ourselves out of this body */
   souls = body->query_souls();
   for (i = 0; i < sizeof(souls); i ++) {
      if (souls[i]->query_name() == name) {
	 catch {
	    souls[i]->disconnect();
	 }
      }
   }

   message("You inhabit " + describe_one(body) + ".\n");

   body->possess(this_object());
   body->stop_busy();	/* just in case */
   if (!body->query_environment()) {
      body->action("login", ([ "default": l ]));
   }

   body->clear_property("StoryHostLineOff");

   possessed = TRUE;

   call_out("display_pages", 1);
}

static
void display_pages() {
   mapping pending_map;

   if (body) {
      int output;

      output = FALSE;
      pending_map = body->query_property("Page:Pending");
      if (typeof(pending_map) == T_MAPPING && map_sizeof(pending_map)) {
	 message(TAG("You have pending pages from " +
		     describe_many(map_indices(pending_map)) + ".\n",
		     "Page"));
	 output = TRUE;
      }
      pending_map = body->query_property("Page:PendingRemote");
      if (typeof(pending_map) == T_MAPPING && map_sizeof(pending_map)) {
	 message(TAG("You have pending remote pages from " +
		     implode(map_indices(pending_map), ", ") + ".\n",
		     "Page"));
	 output = TRUE;
      }
      if (output) {
	 message(TAG("To receive a page, use \"@page accept <name>\"; To refuse, use \"@page deny <name>\".\n", 
		     "Page"));
      }
   }
}

atomic
void depossess_body() {
   if (body) {
      body->depossess(this_object());
      body = nil;
   }
   possessed = FALSE;
}

int query_debug() { return debug; }

void disconnect() {
   ::disconnect();
}

int login(string str) {
   if (previous_program() == LIB_CONN) {
      skotos_client = FALSE;
      html_client = FALSE;
      client_id = nil;

      connection(previous_object());

      call_out("login_prompt", 10, previous_object());
      message_buffer = "";
      return MODE_RAW;
   }
}

static
void raw_message(string str)
{
   if (query_conn()->query_mode() < MODE_RAW) {
      ::message(str);
   } else {
      ::message(implode(explode("\n" + str + "\n", "\n"), "\r\n"));
   }
}

static
void login_prompt(object conn) {
   if (conn && !name && !udat && !secret) {
      raw_message("\nUser: ");
   }
}

private
void udat_logout() {
    /*
     * If necessary, 'unidle' for a split second.
     */
    if (idle_timestamp && !idle_callout) {
	/*
	 * Yes, this user had been marked as 'idled out'.
	 */
	udat->idle_stop(this_object());
    }
    udat->logout(this_object());
}

void body_destructing() {
   if (body && previous_object() == body) {
      udat_logout();	/* let udat know */

      XDebug("Logging out");

      body = nil;

      raw_message("Your body disintegrates beneath you!\n");
      ::disconnect();
      destruct_object();
   }
}

static
void smorgasbord() {
   mixed *arr;
   int i;

   arr = ({
      allocate(sizeof(bodies)+1),
      allocate(sizeof(bodies)+1),
      allocate(sizeof(bodies)+1),
      allocate(sizeof(bodies)+1),
      allocate(sizeof(bodies)+1),
   });
   arr[0][0] = "    #";
   arr[1][0] = "character";
   arr[2][0] = "location";
   arr[3][0] = "played by";
   arr[4][0] = "object name";

   for (i = 1; i <= sizeof(bodies); i ++) {
      arr[0][i] = "    " + i + ".";
      arr[1][i] = describe_one(bodies[i-1]);
      if (bodies[i-1]->query_environment()) {
	 arr[2][i] = describe_one(bodies[i-1]->query_environment());
      } else {
	 arr[2][i] = "<void>";
      }
      if (sizeof(bodies[i-1]->query_souls()) > 0) {
	 arr[3][i] = bodies[i-1]->query_souls()[0]->query_name();
	 if (arr[3][i] == nil) {
	    arr[3][i] = "<somebody>";
	 }
      } else {
	 arr[3][i] = "<nobody>";
      }
      arr[4][i] = name(bodies[i-1]);
   }
   raw_message(tabulate(arr...));
}

void logout(int dest) {
   if (body) {
      catch {
	 object b;

	 udat_logout();

	 b = body;
	 body->act_linkdie();
	 depossess_body();
	 INFOLINE->info_linkdie(this_object(), b);
	 INFOD->logout(this_object());
      }
   }
   destruct_object();
}

int receive_line(string str);

int receive_message(string str) {
   if (previous_program() != LIB_CONN) {
      error("illegal call");
   }

   if (message_buffer) {
      string line;

      str = message_buffer + str;
      message_buffer = "";
      if (!udat && sscanf(str, "<policy-file-request/>\000%*s")) {
	 /*
	  * Heuristic.  Only check for this if we haven't authenticated yet
	  * with the UserDB.
	  */
	 DEBUG("crossdomain.xml sent for " + query_ip_name(query_conn()) +
	       "[" + query_ip_number(query_conn()) + "]");
	 raw_message(read_file("/usr/TextIF/data/www/crossdomain.xml") +
		     "\000");
	 return MODE_DISCONNECT;
      }
      while (strlen(str) &&
	     (sscanf(str, "%s\r\n%s", line, str) == 2 ||
	      sscanf(str, "%s\n%s", line, str) == 2)) {
	 /* sscanf(line, "%s\n", line); */
	 if (receive_line(line) == MODE_DISCONNECT) {
	    return MODE_DISCONNECT;
	 }
      }
      if (strlen(str) > 0) {
	 message_buffer = str;
      } else if (udat) {
	 message_buffer = nil;
	 return MODE_ECHO;
      }
   } else {
      int    i;
      string *lines;

      lines = explode(str, "\n");
      for (i = 0; i < sizeof(lines); i ++) {
	 if (receive_line(lines[i]) == MODE_DISCONNECT) {
	    return MODE_DISCONNECT;
	 }
      }
   }
   /* Strip possible newline at end of line for SKOOT Alice 1,0,0,41\n */
   return MODE_NOCHANGE;
}

int receive_line(string str) {
   string tmp, orig_str;
   int num, i, stamp;

   orig_str = str;
   for (i = 0; i < strlen(str); i++) {
      if (str[i] < 32) {
	 /* turn incoming control characters into spaces */
	 str[i] = ' ';
      }
   }
   if (!udat) {
      if (!str || !strlen(str)) {
	 raw_message("\nFarewell!\n");
	 return MODE_DISCONNECT;
      }
      if (sscanf(str, "WHO%s", str)) {
	 mapping out;
	 object *bods;
	 mixed *list;
	 int j;

	 str = strip(str);

	 list = build_who_list();
	 out = ([ ]);

	 for (i = 0; i < sizeof(list); i ++) {
	    if (bods = list[i]["body"]) {
	       for (j = 0; j < sizeof(bods); j ++) {
		  if (!strlen(str) ||
		      str == bods[j]->query_property("Theatre:ID")) {
		     if (bods[j]->query_property("Theatre:WhoBody") &&
			 !bods[j]->query_property("Theatre:NotWhoBody")) {
			out[url_encode(describe_one(bods[j]))] = TRUE;
		     }
		  }
	       }
	    }
	 }
	 raw_message(map_sizeof(out) + " " +
		     implode(map_indices(out), " ") + "\n");
	 return MODE_ECHO;
      }
      if (sscanf(str, "HELP%s", str)) {
	 mixed *nodes, desc, *descs;

	 str = strip(str);
	 if (strlen(str)) {
	    nodes = HELPD->match_keyword(str);
	    if (!nodes) {
	       raw_message("No such help node.\n");
	       return MODE_ECHO;
	    }
	 } else {
	    nodes = ({ HELPD });
	 }
	 descs = ({ });
	 for (i = 0; i < sizeof(nodes); i ++) {
	    if (desc = nodes[i]->query_description()) {
	       descs += ({
		  url_encode(unsam(desc, ([ "keyword": str ]), this_object()))
		     });
	    }
	 }
	 raw_message(sizeof(descs) + " " + implode(descs, " ") + "\n");
	 return MODE_ECHO;
      }
      if (str == "SKOTOS" || sscanf(str, "SKOTOS %s", client_id)) {
	 html_client = TRUE;
	 skoot_client = TRUE;

	 if (str == "SKOTOS") {
	    client_id = nil;
	 } else {
	    client_id = lower_case(client_id);
	    if (sscanf(client_id, "java %*s")) {
	       html_client = FALSE;
	       if (sscanf(client_id, "%*s-compat")) {
		  skoot_client = FALSE;
	       }
	    }
	 }
	 secret = "NONE";
	 raw_message("SECRET NONE\n");
	 return MODE_ECHO;
      }
      if (secret) {
	 if (sscanf(str, "USER %s", name)) {
	    XDebug("Got USER: " + dump_value(name));
	    name = lower_case(name);
	    return MODE_ECHO;
	 }
	 if (sscanf(str, "SECRET %s", tmp)) {
	    if (tmp != secret) {
	       XDebug("BAD SECRET: X" + tmp + "X != X" + secret + "X");
	       return MODE_DISCONNECT;
	    }
	    XDebug("Got valid SECRET: " + tmp);
	    return MODE_ECHO;
	 }
	 if (sscanf(str, "KEYCODE %s", tmp)) {
	    XDebug("Got KEYCODE: " + tmp);
	    start_keycodeauth("await_char", name, tmp, name);
	    return MODE_ECHO;
	 }
	 if (sscanf(str, "HASH %s", tmp)) {
	    XDebug("Got HASH: " + tmp);
            start_md5login("finish_md5login", name, tmp, name);
	    return MODE_ECHO;
	 }
	 if (sscanf(str, "CHAR %s", str)) {
	    /* groan */
	    XDebug("Holding off...");
	    call_out("receive_line", 0.05, "CHAR " + str);
	    return MODE_ECHO;
	 }
	 XDebug("EEK, got line: " + dump_value(str));
	 return MODE_DISCONNECT;
      }
      /* no secret; a telnet logon */
      if (!name) {
	 name = lower_case(str);
	 raw_message("Password: ");
	 return MODE_NOECHO;
      }
      start_passlogin("finish_passlogin", name, str, name);
      return MODE_ECHO;
   }
   if (!body) {
      string suspended;

      suspended = udat->query_suspended();
      if (suspended) {
	 raw_message("Your account has been suspended.\n" +
		   "If you have any questions about why your account has " +
		   "been suspended, please check the e-mail address you " +
		   "registered to Skotos with, or e-mail ce@skotos.com.\n" +
		   "Comments from Skotos CE: " +
		   (strlen(suspended) ? replace_html(suspended) : "None") +
		   "\n");
	 return MODE_DISCONNECT;
      }
      if (skotos_client) {
	 sscanf(str, "CHAR %s", str);
      }
      if (!bodies) {
	 raw_message("Authentication not finished yet. Try again: ");
	 return MODE_ECHO;
      }
      str = lower_case(str);
      if (sscanf(str, "%d", num)) {
	 if (num < 1 || num > sizeof(bodies)) {
	    raw_message("Please enter a number [1 - " + sizeof(bodies) + "]: ");
	    return MODE_ECHO;
	 }
	 body = bodies[num-1];
	 if (!body) {
	    raw_message("Eep! Somebody just nuked that body! Try again: ");
	    return MODE_ECHO;
	 }
      } else {
	 string theatre;

	 if (!sscanf(str, "%s:%s", theatre, str)) {
	    /* just for clarity */
	    theatre = nil;
	 }
	 for (i = 0; i < sizeof(bodies) && !body; i ++) {
	    string *names, id;

	    id = bodies[i]->query_property("theatre:id");

	    if (theatre && (!id || theatre != lower_case(id))) {
	       continue;
	    }
	    if (str == "*") {
	       body = bodies[i];
	    } else {
	       names = UDATD->query_names_of_body(bodies[i]);
	       if (names && member(str, names)) {
		  body = bodies[i];
	       }
	    }
	 }
	 if (!body) {
	    raw_message("You don't seem to have a body named '" + str + "'. Try again: ");
	    return MODE_ECHO;
	 }
      }

      if (CONFIGD->query("TextIF:ChatMode") &&
	  udat->query_property("TextIF:ChatMode") == nil) {
	udat->set_property("TextIF:ChatMode", TRUE);
	raw_message("\n\nChatMode is ON.\n");
      }

      raw_message("\n\n");
   }
   if (!possessed) {
      INFOD->login(this_object());
      INFOLINE->info_login(this_object(), body);
      udat->logon(this_object());
      catch {
	  udat->add_last_ipnumber("text",
				  query_ip_number(query_conn()),
				  query_ip_name(query_conn()));
      }
      idle_timestamp = time();
      idle_callout = call_out("idle_timeout", IDLE_TIMEOUT);

      /* technically allow future chain possession */
      while (body->query_possessee()) {
	 body = body->query_possessee();
      }
      if (body->query_environment()) {
	 possess_body(body);
      } else {
	 possess_body(body, body->query_property("theatre:startroom"));
      }
      start_tip_timer( 5, "intro" );

      if (skotos_client) {
	 if (udat->query_property("TextIF:ChatMode")) {
	    raw_line("SKOOT " + CHATMODE_ON + " dummy\n");
	 } else {
	    raw_line("SKOOT " + CHATMODE_OFF + " dummy\n");
	 }
      }
      if (html_client) {
         string jitsi_host;
         /* If Jitsi is configured, send a Jitsi SKOOT message - but first we need a JWT token for this user, channel and time. */
         jitsi_host = "/usr/System/initd"->query_jitsi_host();
         if(jitsi_host) {
            /* TODO: MERRY field on player for what Jitsi channel name? Would be nice if it were persistent across logins. */
            avchat_channel = "jitsi_chat_default";
            "/usr/DevSys/sys/avchat"->token_for_channel_and_name(name, avchat_channel, this_object(), "avchat_reply");
         }
      }

      return MODE_ECHO;
   }
   if (idle_timestamp) {
       if (idle_callout) {
	   /* Remove old callout */
	   remove_call_out(idle_callout);
	   idle_callout = 0;
       } else {
	   /* Idled out, wake up again. */
	   udat->idle_stop(this_object());
       }
       idle_timestamp = time();
       idle_callout = call_out("idle_timeout", IDLE_TIMEOUT);
   }
   if (str) {
      str = strip(str);
   } else {
      str = "";
   }
   if (body) {
      int slash;

      if (input_object && input_function) {
	 string i_fun;
	 object i_obj;

	 i_fun = input_function; input_function = nil;
	 i_obj = input_object;   input_object   = nil;

	 call_limited("input_relay", body, i_obj, i_fun, orig_str);
      } else if (str && strlen(str)) {
	 /* basic alias substitution */
	 if (str[0] == '\'' || str[0] == '\"') {
	    if (strlen(str) > 1) {
	      if (udat->query_property("TextIF:ChatMode")) {
		str = str[1 ..];
	      } else {
		str = "say \"" + str[1 ..];
	      }
	    } else {
	       str = "say";
	    }
	 } else if (str[0] == ':' && freemote) {
	   if (strlen(str) > 1) {
	     str = freemote + " " + strip(str[1..]);
	   } else {
	     str = freemote;
	   }
	 } else if (str[0] == '?' && helpverb) {
	   if (strlen(str) > 1) {
	     str = helpverb + " " + strip(str[1..]);
	   } else {
	     str = helpverb;
	   }
	 } else {
	    string verb, arg;

	    str = strip(str);
	    if (!sscanf(str, "%s %s", verb, arg)) {
	       verb = str;
	       arg = nil;
	    }
	    if (verb[0] == '/') {
	      verb = verb[1 ..];
	      slash = TRUE;
	    }
	    if (verb == "quit" || verb == "logout" || verb == "logoff") {
	      object b;

	      udat_logout();

	      b = body;
 	      body->act_quit();
	      depossess_body();
	      INFOLINE->info_quit(this_object(), b);
	      INFOD->logout(this_object());
	      return MODE_DISCONNECT;
	    }
	    if (str == "debug") {
	      if (debug = !debug) {
		message("Debug flag is now ON.\n");
	      } else {
		message("Debug flag is now OFF.\n");
	      }
	      do_prompt();
	      return MODE_ECHO;
	    }
	    if (verb == "chatmode") {
	      if (udat->query_property("TextIF:ChatMode")) {
		udat->set_property("TextIF:ChatMode", FALSE);
		message("ChatMode is now OFF.\n");
		if (skotos_client) {
		  raw_line("SKOOT " + CHATMODE_OFF + " dummy\n");
		}
	      } else {
		udat->set_property("TextIF:ChatMode", TRUE);
		message("ChatMode is now ON.\n");
		if (skotos_client) {
		  raw_line("SKOOT " + CHATMODE_ON + " dummy\n");
		}
	      }
	      return MODE_ECHO;
	    }
	    if (slash || !udat->query_property("TextIF:ChatMode")) {
	      switch(lower_case(verb)) {
	      case "l":	verb = "look"; break;
	      case "exa":	verb = "examine"; break;
	      }
	      if (arg) {
		str = verb + " " + arg;
	      } else {
		str = verb;
	      }
	      switch(lower_case(str)) {
	      case "out": case "exit":	   str = "go out";	break;
	      case "n":	 case "north":	   str = "go north";	break;
	      case "s":	 case "south":	   str = "go south";	break;
	      case "w":	 case "west":	   str = "go west";	break;
	      case "e":	 case "east":	   str = "go east";	break;
	      case "ne": case "northeast": str = "go northeast";	break;
	      case "nw": case "northwest": str = "go northwest";	break;
	      case "se": case "southeast": str = "go southeast";	break;
	      case "sw": case "southwest": str = "go southwest";	break;
	      case "u":	 case "up":	   str = "go up";		break;
	      case "d":	 case "down":	   str = "go down";	break;
	      case "i":			   str = "inventory"; break;
	      case "l":	 		   str = "look"; break;
	      }
	    }
	    if (slash) {
	      str = "/" + str;
	    }
	 }
	 call_limited("parse_command", body, str);
      }
   }
   do_prompt();
   if (!body) {
      if (this_object()) {
	 destruct_object();
      }
      return MODE_DISCONNECT;
   }
   return MODE_ECHO;
}

static
void send_secret(int success, string reply) {
   if (!success) {
      raw_message("Internal UserDB error: " + reply + "\n");
      disconnect();
      return;
   }
   secret = reply;
   raw_message("SECRET " + secret + "\n");
}

static
void finish_md5login(int success, string reply, string lname) {
   XDebug("Finished md5login: " + reply);
   if (!success) {
      raw_message("Authentication error: " + reply + "\n");
      SysLog("Disconnecting " + dump_value(name) + "; reply=" + dump_value(reply));
      disconnect();
      return;
   }
   start_keycodeauth("await_char", lname, reply, lname);
}

static
void await_char(int success, string reply, string lname) {
   string account_type, account_flags;

   if (!success) {
      raw_message("Authentication error: " + reply + "\n");
      SysLog("Disconnecting " + dump_value(name) + "; reply=" + dump_value(reply));
      disconnect();
      return;
   }
   secret = nil;
   udat = UDATD->get_udat(lname);
   if (sscanf(reply, "%*s(%s;%s)", account_type, account_flags) == 3) {
      udat->update_account(account_type, explode(account_flags, " "));
   }
   skotos_client = TRUE;
   XDebug("Awaiting CHAR line...");
   query_conn()->set_mode(MODE_UNBLOCK);
   bodies = udat->query_bodies();
}

static
void finish_passlogin(int success, string reply, string lname) {
   if (!success) {
      raw_message("Authentication error: " + reply + "\n");
      SysLog("Disconnecting " + dump_value(name) + "; reply=" + dump_value(reply));
      disconnect();
      return;
   }
   udat = UDATD->get_udat(lname);
   if (!udat) {
      raw_message("Augh! Can't find you an udat!\n");
      disconnect();
      return;
   }
   start_keycodeauth("welcome_telnet", lname, reply, lname);
}

static
void welcome_telnet(int success, string reply, string lname) {
   string account_type, account_flags;

   if (!success) {
      raw_message("Authentication error: " + reply + "\n");
      SysLog("Disconnecting " + dump_value(name) + "; reply=" + dump_value(reply));
      disconnect();
      return;
   }
   if (sscanf(reply, "%*s(%s;%s)", account_type, account_flags) == 3) {
      udat->update_account(account_type, explode(account_flags, " "));
   }
   bodies = udat->query_bodies();
   raw_message("\n\nWelcome, " + capitalize(lname) + "! These are your characters:\n\n");
   call_limited("smorgasbord");
   /* sizeof(bodies) should be > 0, no character creation code here. */
   raw_message("Please enter a name or a number [1 - " + sizeof(bodies) + "] ");
}

void parse_command(object body, string str) {
   MAIN->command(this_object(), body, str);
}

void initialize_theme() {
   string bgcolour, fgcolour, link, vlink, alink, str;

   if (!html_client) {
      return;
   }

   bgcolour = get_colour("background");
   fgcolour = get_colour("text");
   link = get_colour("link");
   vlink = get_colour("vlink");
   alink = get_colour("alink");

   if (fgcolour || bgcolour || link) {
      str = "<body";
      if (bgcolour) {
	 str += " bgcolor='" + bgcolour + "'";
      }
      if (fgcolour) {
	 str += " text='" + fgcolour + "'";
      }
      if (link) {
	 str += " link='" + link + "' plink='" + link + "'";
      }
      if (vlink) {
	 " vlink='" + vlink + "'";
      }
      if (alink) {
	 " alink='" + alink + "'";
      }
      str += ">\n";
      html_message(str);
   }
}

/* Values for muted are muted (1), unmuted (0), and "no change" (-1) */
/* Value for room must not contain any single- or double-quotes. */
void send_jitsi_message(string room, string jwt_token, int muted) {
   string jitsi_host, muted_json;

   /* Jitsi is only supported by the html client */
   if (!html_client) {
      return;
   }

   jitsi_host = "/usr/System/initd"->query_jitsi_host();

   /* If no jitsi_host is set in the instance file, don't send Jitsi SKOOT messages. */
   if(!jitsi_host) {
      error("Got a send_jitsi_message but no jitsi_host is set in instancefile for account " + name);
      return;
   }

   /* This is a security problem. Even if the server returned a token, don't send it to the user. */
   if(room == "*") {
      error("Jitsi room should never be sent as asterisk or they have access to all rooms! Account " + name);
      return;
   }

   if(muted == 0) {
      muted_json = "'muted': 'unmuted', ";
   } else if(muted == 1) {
      muted_json = "'muted': 'muted', ";
   } else {
      /* Any other value means "don't change the muted status" */
      muted_json = "";
   }

   raw_line("SKOOT 80 { "
      /* 'prefix' is client-supported but we never supply it. */
      + "\"domain\": \"" + jitsi_host + "\", "
      + "\"name\": \"" + name + "\", "
      + muted_json
      + "\"room\": \"" + room + "\", "
      + "\"jwt\": \"" + jwt_token + "\" " /* No comma */
      + "}");
}

void avchat_reply(int success, string msg, string token) {
   if(success == 0) {
      Debug("AVChat daemon returned failure for user " + name + " with reason: " + msg);
      /* Usually this is a timeout. For now, just ignore in all cases. */
   } else if(success == 1) {
      /* TODO: Merry field for whether a player should be server-muted? */
      /* For muted, 1 == muted, 0 == unmuted, -1 == don't change the setting. */
      send_jitsi_message(avchat_channel, token, -1);
   } else {
      /* Malformed reply from the AVChat daemon... */
      error("Success field for avchat_reply should always be 0 or 1!");
   }
}

/*
 * Variable to handle the output trickling.
 */
private int trickle_handle;
private mixed *trickle_data;
private int prompt_handle;

/*
 * NAME:        add_trickle()
 * DESCRIPTION: Add the data to the storage and start a call_out if necessary.
 */
private void add_trickle(string type, string str) {
   if (!trickle_data) {
      trickle_data = ({ });
   }
   trickle_data += ({ ({ type, str }) });
   if (trickle_handle == 0) {
      /* First trickle, unprompt then. */
      trickle_handle = call_out("trickle", 0);
   }
}

private void do_prompt() {
   if (trickle_handle > 0) {
      /* Output pending.  Force it for the command-giver. */
      newline = TRUE;
      remove_call_out(trickle_handle);
      this_object()->trickle(TRUE);
   } else {
      if (!input_function && !input_object) {
	 raw_message("> ");
	 newline = FALSE;
      }
   }
}

/*
 * NAME:        trickle()
 * DESCRIPTION: Trickle some output and if necessary start a call_out for some
 *              more.
 */
static void trickle(varargs int flag) {
   int i, sz, sum, sum_text;
   string type, text, prefix;

   if (prompt_handle > 0) {
      remove_call_out(prompt_handle);
      prompt_handle = 0;
   }

   if (name == "nino") {
      /* DEBUG("[trickle] " + ur_name(this_object()) + ":" + name + "; newline = " + newline); */
   }
   trickle_handle = 0;

   sz = sizeof(trickle_data);

   prefix = newline ? "" : (skoot_client ? "\b\b" : "");
   if (!flag && udat && udat->query_property("skotos:linebreaks")) {
      prefix += "\n";
   }

   for (i = 0; i < sz; i++) {
      if (trickle_data[i][0] == "raw_line") {
	 if (name == "nino") {
	    /* DEBUG("[trickle] type = raw_line; text = " + dump_value(trickle_data[i][1])); */
	 }
	 if (!newline) {
	    if (!strlen(prefix) || prefix[strlen(prefix) - 1] != '\n') {
	       raw_message(prefix + "\n");
	    } else {
	       raw_message("\n");
	    }
	    sum++;
	    newline = TRUE;
	    prefix = "";
	 }

	 text = trickle_data[i][1];

	 catch {
	    text = mutate_markup(text);
	    text = expand_markup(text);
	 } : {
	    text = "[ERROR: Unable to process output.  Alert game staff to what caused this error]";
	 }
	 if (strlen(text)) {
	    raw_message(text);
	    sum += strlen(text);
	    newline = text[strlen(text) - 1] == '\n';
	 }
      }
   }

   for (i = 0; i < sz; i++) {
      string type, text;

      type = trickle_data[i][0];
      text = trickle_data[i][1];

      if (type != "raw_line") {
	 if (name == "nino") {
	    /* DEBUG("[trickle] type = " + type + "; text = " + dump_value(text)); */
	 }
	 switch (type) {
	 case "html_message":
	    catch {
	       text = mutate_markup(text);
	       text = expand_markup(text);
	    } : {
	       text = "[ERROR: Unable to process output.  Alert game staff to what caused this error]";
	    }
	    if (strlen(text)) {
	       raw_message(prefix + text);
	       sum_text += strlen(prefix) + strlen(text);
	       newline = text[strlen(text) - 1] == '\n';
	    }
	    break;

	 case "message":
	    catch {
	       text = mutate_markup(text);
	       if (skotos_client) {
		  text = replace_html(text);
	       }
	       text = expand_markup(text);
	    } : {
	       text = "[ERROR: Unable to process output.  Alert game staff to what caused this error]";
	    }
	    if (strlen(text)) {
	       raw_message(prefix + text);
	       sum_text += strlen(prefix) + strlen(text);
	       newline = text[strlen(text) - 1] == '\n';
	    }
	    break;

	 case "paragraph":
	    catch {
	       text = mutate_markup(text);
	       if (skotos_client) {
		  text = replace_html(text);
	       }
	       text = expand_markup(text);
	       text = strip_right(text, TRUE);
	    } : {
	       text = "[ERROR: Unable to process output.  Alert game staff to what caused this error]";
	    }
	    if (!strlen(text) || text[strlen(text) - 1] != '\n') {
	       text += "\n";
	    }
	    raw_message(prefix + capitalize(text));
	    sum_text += strlen(prefix) + strlen(text);
	    newline = TRUE;
	    break;

	 default:
	    error("Internal error, unknown trickle data type: " + type);
	 }
	 prefix = "";
      }
   }

   trickle_data = ({ });

   if (!input_function && !input_object) {
      if (!newline) {
	 raw_message("\n");
	 sum_text++;
	 newline = TRUE;
      }
      prompt_handle = call_out("delayed_prompt", 0.1);
   }
#if 0
   if (sum + sum_text > 1000) {
      DEBUG("[trickle] " + name + " :: " + sum + " raw bytes and " + sum_text + " bytes of text sent.\n");
   }
#endif
}

static void delayed_prompt() {
   raw_message("> ");
   prompt_handle = 0;
   newline = FALSE;
}

void raw_line(string str) {
   add_trickle("raw_line", str);
}

void html_message(string str) {
   if (str && strlen(str)) {
      add_trickle("html_message", str);
   }
}

void message(string str) {
   if (str && strlen(str)) {
      add_trickle("message", str);
   }
}

void paragraph(string str) {
   if (str && strlen(str)) {
      add_trickle("paragraph", str);
   }
}


/* TODO */
void append_string(string str) {
   message(str);
}

/* DEBUG */
void slay() {
   if (body) {
      udat_logout();
   }
   destruct_object();
}
