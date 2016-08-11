# include <limits.h>
# include <SkotOS.h>
# include <HTTP.h>
# include <SAM.h> 
# include <XML.h>
# include <UserAPI.h>

# define ASSIST_LOCAL

# define SEPARATOR "-------------------------------------------------------------------\n"

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/url";
private inherit "/lib/mapargs";

inherit "/usr/SkotOS/lib/auth";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/merryapi";

inherit "/usr/SMTP/lib/smtp_api";
inherit "/usr/SAM/lib/sam";
inherit ctlapi "/usr/UserAPI/lib/ctlapi";

mapping work_q;		/* Who are available to assist */
mapping wait_q;		/* Who are waiting to be assisted */
mapping wait_time;	/* List the waiting players ordered by time */
mapping user_notes;	/* ... */

object *alerts;		/* List of staff receiving assist alerts */
object tas_relay;	/* Where to send Merry calls for the 'tas' channel. */

void tell_alert_staff(string s, varargs int flag);

static 
void create() {
   ctlapi::create();
   set_object_name("SkotOS:Assistd");

   wait_q = ([ ]);
   work_q = ([ ]);
   wait_time = ([ ]);
   user_notes = ([ ]);
   alerts = ({ });

# ifndef ASSIST_LOCAL
   BROADCAST->add_listener("assist", this_object());
#endif
   BROADCAST->add_listener("tas", this_object());
}

/*** Begin interface to Merry ***/

int
query_method(string name)
{
    return !!function_object("method_" + name, this_object());
}

mixed
call_method(string name, mapping args)
{
    return call_other(this_object(), "method_" + name, args);
}

static int
method_list(mapping args)
{
    string callback;
    object this;

    callback = args["callback"];
    if (!callback || !strlen(callback)) {
	return FALSE;
    }
    this = args["this"];
    if (!this) {
	return FALSE;
    }
    start_assists_list("callback_method_list", callback, this, args);
    return TRUE;
}

static void
callback_method_list(int success, string answer, string callback, object this, mapping args)
{
    if (success) {
	int i, sz;
	string *list;

	list = explode(answer, " ");
	sz = sizeof(list);
	for (i = 0; i < sz; i++) {
	    list[i] = url_decode(list[i]);
	}
	run_merry(this, "relay_" + callback, "lib", args + ([ "success": TRUE, "list": list ]));
    } else {
	run_merry(this, "relay_" + callback, "lib", args + ([ "success": FALSE, "list": ({ }) ]));
    }
}

static int
method_query(mapping args)
{
    string callback, group;
    object this;

    callback = args["callback"];
    if (!callback || !strlen(callback)) {
	return FALSE;
    }
    this = args["this"];
    if (!this) {
	return FALSE;
    }
    group = args["group"];
    if (!group || !strlen(group)) {
	return FALSE;
    }
    start_assists_query("callback_method_query", group, group, callback, this, args);
    return TRUE;
}

static void
callback_method_query(int success, string answer, string group, string callback, object this, mapping args)
{
    if (success) {
	int     i, sz;
	string  *list;

	list = explode(answer, " ");
	sz = sizeof(list);
	for (i = 0; i < sz; i++) {
	    list[i] = url_decode(list[i]);
	}
	run_merry(this, "relay_" + callback, "lib",
		  args +
		  ([ "success": TRUE,
		     "email":   list[0],
		     "prefix":  list[1],
		     "from":    list[2] ]));
    } else {
	run_merry(this, "relay_" + callback, "lib",
		  args +
		  ([ "success": FALSE ]));
    }
}

static int
method_send(mapping args)
{
    string callback, group, subject, body;
    object this;

    callback = args["callback"];
    if (!callback || !strlen(callback)) {
	return FALSE;
    }
    this = args["this"];
    if (!this) {
	return FALSE;
    }
    group = args["group"];
    if (!group || !strlen(group)) {
	return FALSE;
    }
    subject = args["subject"];
    body = args["body"];
    /*
     * TODO: If the $accounts parameter is present, check the email addresses for
     * everyone involved and insert the relevant information before the actual
     * body, so that it can be used by whoever wants to reply.
     */
    start_assists_query("callback_method_send", group, group, subject, body, callback, this, args);
    return TRUE;
}

static void
callback_method_send(int success, string answer, string group, string subject, string body, string callback, object this, mapping args)
{
    if (success) {
	int i, sz;
	string *list, id, *addresses;
	mapping to;

	list = explode(answer, " ");
	sz = sizeof(list);
	for (i = 0; i < sz; i++) {
	    list[i] = url_decode(list[i]);
	}
	/* Turn whitespace characters into regular spaces. */
	list[0] = implode(explode(list[0], "\r"), " ");
	list[0] = implode(explode(list[0], "\n"), " ");
	list[0] = implode(explode(list[0], "\t"), " ");
	list[0] = implode(explode(list[0], "\b"), " ");

	/* Replace consecutive spaces into commas. */
	list[0] = implode(explode(list[0], " ") - ({ "" }), ",");

	addresses = explode(list[0], ",") - ({ "" });
	to = ([ ]);
	sz = sizeof(addresses);
	for (i = 0; i < sz; i++) {
	    to[addresses[i]] = "";
	}
	DEBUG("to = " + dump_value(to));
	id = send_message(/* Env     */ list[2],
			  /* Name    */ "",
			  /* From    */ list[2],
			  /* To      */ to,
			  /* Cc      */ ([ ]),
			  /* Subject */ ((list[1] && strlen(list[1])) ? list[1] : "[ASSIST]") + " " + subject,
			  /* Headers */ nil,
			  /* Body    */ body);
	DEBUG("Queue ID: " + id);
	run_merry(this, "relay_" + callback, "lib",
		  args + ([ "success": TRUE, "id": id ]));
    } else {
	run_merry(this, "relay_" + callback, "lib",
		  args + ([ "success": FALSE ]));
    }
}

static string **
method_query_notes(mapping args)
{
   int i, sz;
   string user;
   mixed *notes;

   if (!(user = args["user"])) {
      return nil;
   }
   if (!(notes = user_notes[user])) {
      return nil;
   }
   notes = notes[..];
   sz = sizeof(notes);
   for (i = 0; i < sz; i++) {
      string from, stamp, message;

      sscanf(notes[i], "%s|%s|%s", from, stamp, message);
      if (strlen(message) >= 2 && message[0] == '"' && message[strlen(message) - 1] == '"') {
	 message = message[1..strlen(message) - 2];
      }
      notes[i] = ({ from, stamp, message });
   }
   return notes[..];
}

static int
method_add_note(mapping args)
{
   string user, message, from;

   if (!(user = args["user"])) {
      return FALSE;
   }
   if (!(message = args["message"])) {
      return FALSE;
   }
   if (!(from = args["from"])) {
      return FALSE;
   }
   if (!user_notes[user]) {
      user_notes[user] = ({ });
   }
   user_notes[user] += ({ from + "|" + ctime(time()) + "|\"" + message + "\"" });
   return TRUE;
}

static int
method_remove_last_note(mapping args)
{
   string user, *notes;

   if (!(user = args["user"])) {
      return FALSE;
   }
   if (!(notes = user_notes[user]) || !sizeof(notes)) {
      return FALSE;
   }
   user_notes[user] = notes[.. sizeof(notes) - 2];
   return TRUE;
}

int
method_transfer_relay(mapping args)
{
   if (typeof(args["relay"]) != T_OBJECT) {
      error("transfer_relay: $relay should be an object value");
   }
   tas_relay = args["relay"];
   return TRUE;
}

object
query_tas_relay()
{
   return tas_relay;
}

int
method_transfer_data(mapping args)
{
   string server, id;
   mixed  data;

   if (!tas_relay) {
      error("transfer_data: Set a relay object using transfer_relay");
   }

   if (typeof(args["server"]) != T_STRING) {
      error("transfer_data: $server should be a string value");
   }
   server = args["server"];

   if (typeof(args["id"]) != T_STRING) {
      error("transfer_data: $id should be a string value");
   }
   id = args["id"];

   data = args["transfer_data"];

   BROADCAST->broadcast_one(server, "tas", "transfer_request", ({ id, mixed_to_ascii(data), mixed_to_ascii(args) }));
}

object
resolve_unknown(string ref)
{
   return nil;
}

void
channel_tas_transfer_request(string game, int self, string id, string data_str, string args_str)
{
   mapping args;

   if (!tas_relay) {
      BROADCAST->broadcast_one(game, "tas", "transfer_reply", ({ id, FALSE, "No relay object set.", args_str }));
      return;
   }
   args = ([ "server": game, "id": id, "transfer_data": ascii_to_mixed(data_str, "resolve_unknown") ]);
   run_merry(tas_relay, "transfer_request", "lib", args);
   BROADCAST->broadcast_one(game, "tas", "transfer_reply", ({ id, mixed_to_ascii(args["result"]), mixed_to_ascii(args["message"]), args_str }));
}

void
channel_tas_transfer_reply(string game, int self, string id, string result_str, string message, string args_str)
{
   mapping args;

   if (!tas_relay) {
      return;
   }
   args = ascii_to_mixed(args_str, "resolve_unknown");
   args["server"]  = game;
   args["id"]      = id;
   args["result"]  = ascii_to_mixed(result_str, "resolve_unknown");
   args["message"] = ascii_to_mixed(message, "resolve_unknown");
   run_merry(tas_relay, "transfer_reply", "lib", args);
}

/*** End of interface to Merry ***/

void
patch()
{
# if 0
   ctlapi::create();
   BROADCAST->add_listener("assist", this_object());
# endif
# ifdef ASSIST_LOCAL
   BROADCAST->add_listener("assist", this_object());
# else   
   BROADCAST->remove_listener("assist", this_object());
# endif
   BROADCAST->add_listener("tas", this_object());
}

string clear() {
   wait_q = ([ ]);
   work_q = ([ ]);
   wait_time = ([ ]);
   user_notes = ([ ]);
   alerts = ({ });
}

mapping query_wait_queue() { return wait_q; }
mapping query_work_queue() { return work_q; }


void clear_assists ( object user ) {
   work_q = ([ ]);
   user->message("All assists cleared from the 'current' queue.\n");
}

string list_top ( void )
{
   return "  Time       User        Character";
}

void list_assists (object user, varargs int ugly) {
   int    i;
   string num, *times, playername, str, verb, result;

   if (is_root(user->query_name())) {
       verb = "+assist";
   } else {
       verb = "!assist";
   }

   result =
       "<PRE>\nCommon " + verb + " options:  (also status, goto)\n"+
       "  " + verb + " 1          To help user #1 in the list below \n"+
       "  " + verb + " done       When you are done helping someone \n"+
       "  " + verb + " cur        To see people currently being helped \n"+
       "  " + verb + " alert      To receive assist alerts (and the assist line) \n"+
       "  " + verb + " ugly       To see an ugly list of assists\n"+
       "  " + verb + " curugly    To see an ugly list of open assists\n" +
       "\n" +
       "The Assist line:\n" +
       "  " + verb + " \"message\"  To talk on the line.\n" +
       "  " + verb + " who        To see who's listening.\n" +
       "  " + verb + " recall     To check the last few chats.\n";

   times = map_indices( wait_time );
   result += "\n  " + list_top() + "              Message\n" + SEPARATOR;
   for (i = 1; i <= sizeof(times); i++ ) {
      num = " "+i+" ";
      if ( i<10 )
         num += " ";
      playername = wait_time[ times[i-1] ];
      str = wait_q[ playername ];
      if (!ugly && strlen(str) > 62)
         str = str[0..62]; 

      if (!is_root(user->query_name())) {
	  int j;

	  /* Hide the 'username'. */
	  for (j = 8; j < 19; j ++) {
	      str[j] = '*';
	  }
      }
      result += num + replace_html(str) + "\n";
   }
   if ( sizeof(times)==0 ) {
       result += "          We rock.  Nobody is waiting to be helped.\n";
   }
   user->html_message( result + SEPARATOR + "</PRE>\n" );
}

void list_open (object user, varargs int ugly) {
   int i;
   string *helpers, str, result;

   helpers = map_indices( work_q );
   
   result =  "<PRE>\n   CE Contact  " + list_top() + "\n" + SEPARATOR;
   
   for( i=0; i<sizeof(helpers); i++ ) {
      str = work_q[ helpers[i] ];
      if (!ugly && strlen(str) > 62)
         str = str[0..62];
      	  
      if (!is_root(user->query_name())) {
	 int j;

	 for (j = 23; j < 34; j ++) {
	    str[j] = '*';
	 }
      }
      result += "  " + replace_html(str) + "\n" ;
   }
   if ( sizeof(helpers)==0 ) {
      result += "             Nobody is currently being assisted.\n";
   }
   user->html_message(result + SEPARATOR + "</PRE>" );
}

void assist_done ( object user, varargs string comment ) {
   if ( work_q[ user->query_name() ] ) {
      string text;

      user->message( "Assist removed from open queue.\n");
      text = work_q[user->query_name()];
      /* nino         | 02:00 | nino        | Hresh       | Help! */
      /* 01234567890123456789012345678901234567890123456789012345 */
      /* 0         1         2         3         4         5      */
      LOGD->add_entry("Assist Log",
		      describe_one(user->query_body()) + "[" +
		      strip(text[0..12]) + "] assisted " +
		      strip(text[36..48]) + "["+ strip(text[22..34]) + "]",
		      "Original request: \"" + strip(text[50..]) + "\"\n\n" +
		      "Comment: " + (comment ? comment : "None.") + "\n");
      work_q[ user->query_name() ] = nil;
   } else {
      user->message( "You don't have any assists open.\n");
   }
}

void do_assist ( object user, int num ) {
   string str, *ppl, *times, playername, verb;
   int i;
   
   if ( work_q[ user->query_name() ] ) {
      if (is_root(user->query_name())) {
	 verb = "+assist";
      } else {
	 verb = "!assist";
      }
      user->message("You already have an assist open.  Type '" + verb + " done' when finished.\n");
      return;
   }

   ppl = map_indices( wait_q );
   times = map_indices( wait_time );
   if ( num < 1 || num > sizeof(ppl) ) {
      user->message( "Invalid number.  Please enter one of the following numbers:\n");
      list_assists( user );
	  return;
   }

   playername = wait_time[ times[num-1] ];

   str = wait_q[ playername ];
   work_q[ user->query_name() ] = pad_right( user->query_name(), 12 ) + " | " + str;

   user->message( "\nAssist message: " + wait_q[ playername ][35..] + "\n" );

   wait_q[ playername ] = nil;
   wait_time[ times[num-1] ] = nil;

   user->message( "Assist moved to open queue.  "+capitalize(playername)+" is awaiting your page.\n" );

   tell_alert_staff( "<br/>[ASSIST] " + 
		     "<if val=\"$(Body.Guide)\"><true>" +
		     describe_one(user->query_body()) +
		     "</true><false>" +
		     capitalize(user->query_name()) + 
		     "</false></if>" +
		     " is now assisting " + capitalize(playername) + "<br/>" );
# ifdef ASSIST_LOCAL
   "~TextIF/sys/chatlines"->channel_assist_answer("marrach", TRUE,
						  user->query_name(),
						  playername);
# else
   start_broadcast_all("", "assist", "answer",
		       ({ user->query_name(), playername }));
# endif
}

void
channel_assist_answer(string game, int self, string name, string playername)
{
    if (self) {
	return;
    }
    tell_alert_staff("<br/>[" + game + ":ASSIST] " + capitalize(name) + 
		     " is now assisting " + capitalize(playername) + "<br/>",
		     FALSE);
}

# define HOUR	3600
# define DAY	(HOUR * 24)

void add_assist ( object user, object body, string helptext ) {
   string username, playername, str;
   int day, hour, i;
   object uobj;

   playername = describe_one( body );
   username = user->query_name();

   str = pad_right( ctime(time())[11..15], 6 ) + "| ";
   str += pad_right( username, 12 ) + "| ";
   str += pad_right( playername, 12 ) + "| ";
   str += helptext;

   if ( wait_q[playername] ) {
      wait_q[ playername ] = str;
   } else {
      wait_q[ playername ] = str;
      wait_time[ " "+time()+playername ] = playername;
   }

   day =  (time() / DAY + 4) % 7;
   hour = (time() / HOUR + 17) % 24;

   if (day == 0 || day == 1 || hour < 12 || hour > 21) {
       user->message("Your assist has been sent. Marrach is normally staffed "+
		     "by StoryHosts 12pm  to 9pm Pacific time, Tuesday "+
		     "through Saturday. You will be contacted by a StoryHost "+
		     "or StoryGuide with a 'page'. Please use the 'page' "+
		     "command to respond to whomever contacts you. A proper "+
		     "example of the usage of page is 'page storyhost (name) "+
		     "\"message', or 'page storyguide (name) \"message'.");
   } else {
       user->message("Your assist message has been sent, you will be "+
		     "contacted shortly by a StoryHost or StoryGuide with a "+
		     "'page'. Please use the 'page' command to respond to "+
		     "whomever contacts you. A proper example of the usage of "+
		     "page is 'page storyhost (name) \"message', or "+
		     "'page storyguide (name) \"message'.");
   }
   tell_alert_staff("<br/>[ASSIST] "+playername+": " +
		    replace_html(helptext) + "<br/>");
# ifdef ASSIST_LOCAL
   "~TextIF/sys/chatlines"->channel_assist_request("marrach", TRUE,
						   username, playername,
						   helptext);
   
# else
   start_broadcast_all("", "assist", "request",
		       ({ username, playername, helptext }));
# endif
}

void
channel_assist_request(string game, int self, string username,
		       string playername, string helptext)
{
    if (self) {
	return;
    }
    tell_alert_staff("<br/>[" + game + ":ASSIST] " + playername + ": " +
		     replace_html(helptext) + "<br/>", FALSE);
}

object *query_alerts() {
   alerts -= ({ nil });
   return alerts;
}

void add_to_alerts(object body) {
   alerts += ({ body });
}  

void remove_from_alerts(object body) {
   alerts -= ({ body });
}

void tell_alert_staff (string s, varargs int flag) {
   int i;
   object uobj;   
   string str;
   SAM parsed;
   
   parsed = XML->parse(s);

   /* Obviously, the 'unsam()' call is not very efficient. */
   for ( i=0; i<sizeof(alerts); i++ ) {
      if (flag && alerts[i] && alerts[i]->query_property("GUIDE") == "true") {
	  /* Exclude StoryGuides -- used for remote assist requests. */
	  continue;
      }
      if (alerts[i] && sizeof(alerts[i]->query_souls()) > 0 ) {
         uobj = alerts[i]->query_souls()[0];
	 str = unsam(parsed, ([ "body" : alerts[i], "user" : uobj ]));
         uobj->message(TAG(str, "assist-alert"));
      }
   }
}


string query_currently_helping ( object user ) {
   string str;
   if ( work_q[ user->query_name() ] == nil )
      return nil;

   sscanf( work_q[ user->query_name() ], "%*s|%*s|%*s| %s|%*s", str );
   return str;
}



/* notes */

int kill_last_note(string user) {
   string *notes;

   if (notes = user_notes[user]) {
      if (sizeof(notes)) {
	 user_notes[user] = notes[.. sizeof(notes)-2];
	 return TRUE;
      }
   }
   return FALSE;
}

void add_user_note(string user, string msg) {
   string *notes;

   notes = user_notes[user];
   if (!notes) {
      notes = ({ });
   }
   user_notes[user] = notes + ({ msg });
}

string *query_user_notes(string user) {
   return user_notes[user];
}
