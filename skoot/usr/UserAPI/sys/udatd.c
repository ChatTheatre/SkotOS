/*
**	~UserAPI/sys/udatd.c
**
**	Keep user information per group, such as bodies owned,
**	request ID object, ... allow lookups of body per name.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>

private inherit "/lib/type";
private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/mapargs";

inherit authapi "/usr/UserAPI/lib/authapi";
inherit module "/lib/module";

# include <SAM.h>
# include <UserAPI.h>
# include <mapargs.h>

mapping *udats_arr;	/* map username to udat */
mapping *email_arr;	/* map email to username */
mapping char_to_body;	/* OLD: map charname to body */

int     server_locked;  /* Server is locked, staff allowed in, at own risk */

object  namedb;		/* LWO keeping track of body-names */

void setup_db();

mapping query_c2b() { return char_to_body; }

static
void create() {
    int i;

    set_object_name("UserAPI:UDatD");

    udats_arr = allocate(256);
    email_arr = allocate(256);
    for (i = 0; i < 256; i++) {
	udats_arr[i] = ([ ]);
	email_arr[i] = ([ ]);
    }

    setup_db();

    compile_object(UDAT);

    authapi::create();
    module::create("SAM");
}

void setup_db() {
   object body_to_names;
   object name_to_bodies;

   /* create two bigmap LWO's with the proper indices */
   body_to_names  = new_object("/data/ob_bigmap");
   name_to_bodies = new_object("/data/str_bigmap");

   /* create the N-N LWO to maintain consistency */
   namedb = new_object("/data/n2n", body_to_names, name_to_bodies);
}

static
void SAM_loaded() {
   SAMD->register_root("UDat");
}


void register_lost_udat(object udat) {
   string name;

   if (name = udat->query_name()) {
      if (strlen(name) > 0) {
	 udats_arr[name[0]][name] = udat;
      }
   }
}

/* user/udat stuff */

atomic
object get_udat(string uname, varargs string password) {
   object udat;

   if (udat = udats_arr[uname[0]][uname]) {
      return udat;
   }
   udats_arr[uname[0]][uname] = udat = clone_object(UDAT);
   udats_arr[uname[0]][uname]->set_name(uname);

   return udat;
}

mapping query_char_to_body() {
   return char_to_body;		/* NOTE: private pointer returned */
}

object query_namedb() {
   return namedb;
}

mapping *query_udats_arr() {
   return udats_arr;		/* NOTE: private pointer returned */
}

object query_udat(string uname) {
  return udats_arr[uname[0]][uname];
}

string query_user_by_email(string email) {
   if (email && strlen(email) > 0) {
      email = lower_case(email);
      return email_arr[email[0]][email];
   }
}

atomic
void register_email(string user, string email) {
   if (previous_program() == UDAT) {
      email = lower_case(email);
      email_arr[email[0]][email] = user;
   }
}

mixed query_udats() {
    int    i;
    object **list;

    list = allocate(256);
    for (i = 0; i < 256; i++) {
	if (map_sizeof(udats_arr[i]) > 0) {
	    list[i] = map_values(udats_arr[i]);
	}
    }
    return list - ({ nil });
}

/* body/name stuff */

void addnote(string user, string note, string body, int timestamp, string head, SAM desc) {
    object udat;
    
    udat = udats_arr[user[0]][user];
    if(!udat) {
        error("unknown user");
    }
    
    udat->addnote(note, body, timestamp);
    
    LOGD->add_entry("Notes Log", head, desc);
}

void killnote(string user, string head, SAM desc) {
    object udat;
    
    udat = udats_arr[user[0]][user];
    if(!udat) {
        error("unknown user");
    }
    
    LOGD->add_entry("Notes Log", head, desc);
}

void notes(string user) {
    object udat;
    
    udat = udats_arr[user[0]][user];
    if(!udat) {
        error("unknown user");
    }
    
    return udat->query_notes();
}

void set_suspended(string reason, string user, string head, SAM desc) {
   object udat;

   udat = udats_arr[user[0]][user];
   if (!udat) {
      error("unknown user");
   }
   
   udat->set_suspended(reason);
   
   LOGD->add_entry("Suspend Log", head, desc);
}

string query_suspended(string user) {
   object udat;

   udat = udats_arr[user[0]][user];
   if (!udat) {
      error("unknown user");
   }
   
   return udat->query_suspended();
}

atomic
void add_body_to_roster(object body, string user) {
   object udat;

   udat = udats_arr[user[0]][user];
   if (!udat) {
      error("unknown user");
   }
   udat->add_body(body, FALSE);
}

atomic
void remove_body_from_roster(object body, string user) {
   object udat;

   udat = udats_arr[user[0]][user];
   if (!udat) {
      error("unknown user");
   }
   udat->del_body(body);
}

atomic
object *query_bodies_in_roster(string user) {
   object udat;

   udat = udats_arr[user[0]][user];
   if (!udat) {
      error("unknown user");
   }
   return udat->query_bodies();
}

atomic
void remove_name_for_body(object body, string name) {
   namedb->remove_association(body, lower_case(name));
}

atomic
void add_name_for_body(object body, string name, varargs int primary) {
   namedb->add_association(body, lower_case(name), primary);
}

object query_body(string char) {
   object *bodies;

   if (bodies = namedb->get_associators(lower_case(char))) {
      bodies -= ({ nil });
      if (sizeof(bodies) > 0) {
	 return bodies[0];
      }
   }
   return nil;
}

object *query_bodies(string char) {
   object *bodies;

   if (bodies = namedb->get_associators(lower_case(char))) {
      return bodies - ({ nil });
   }
   return nil;
}

/* this returns main name & aliases */
string *query_names_of_body(object body) {
   return namedb->get_associates(body);
}

/* this returns main name only, or nil */
string query_name_of_body(object body) {
   string *names;

   names = namedb->get_associates(body);
   if (names && sizeof(names) > 0) {
      return names[0];
   }
   return nil;
}

/* patch function */


atomic
void patch_data() {
   mapping map;
   object body;
   mixed *outer, *inner;
   int i, j;

   setup_db();

   outer = map_indices(char_to_body);
   for (i = 0; i < sizeof(outer); i ++) {
      map = char_to_body[outer[i]];
      inner = map_indices(map);
      for (j = 0; j < sizeof(inner); j ++) {
	 string name;

	 body = map[inner[j]];
	 name = outer[i] + inner[j];

	 sscanf(name, "%s:%*s", name);

	 namedb->add_association(body, name);
      }
   }
   /* when you're certain this works right, uncomment the next line */
   /* char_to_body = nil; */
}

string *query_users_created_between(int from, int to) {
    mapping names_map;
    int i;

    names_map = ([ ]);
    for (i = 0; i < 256; i++) {
	string *names;
	object *dats;
	int j;

	names = map_indices(udats_arr[i]);
	dats = map_values(udats_arr[i]);

	for (j = 0; j < sizeof(dats); j ++) {
	    if (dats[j]->query_creation_time() >= from &&
		dats[j]->query_creation_time() < to) {
		names_map[names[j]] = 1;
	    }
	}
    }
    return map_indices(names_map);
}

string *query_users_who_last_played_between(int from, int to) {
    mapping names_map;
    int i;

    names_map = ([ ]);
    for (i = 0; i < 256; i++) {
	string *names;
	object *dats;
	int j;

	names = map_indices(udats_arr[i]);
	dats = map_values(udats_arr[i]);
	for (j = 0; j < sizeof(dats); j++) {
	    if (dats[j]->query_last_logon() >= from &&
		dats[j]->query_last_logon() < to) {
		names_map[names[j]] = 1;
	    }
	}
    }
    return map_indices(names_map);
}

# define DAY	(3600 * 24)
# define WEEK	(DAY * 7)
# define MONTH	(DAY * 30)

/* SAM service */

mixed eval_sam_ref(string service, string ref, object context, mapping args) {
   switch(lower_case(ref)) {
   case "dat":
      return context->query_udat();
   case "name":
      return context->query_user()->query_name();
   case "chatmode":
      return context->query_udat() &&
	 context->query_udat()->query_property("TextIF:ChatMode");
   case "chatchar":
      if (context->query_udat()) {
	 if (context->query_udat()->query_property("TextIF:ChatMode")) {
	    return "/";
	 }
	 return "";
      }
      return "";
   case "findbody": {
      string char;
      object body;

      char = StrArg("char", "UDat.FindBody");
      body = query_body(char);
      if (body) {
	 return body;
      }
      /*
       * This is to work around the fact that I cleaned up the main
       * char_to_body list but left the local udat mappings as they were so
       * that people wouldn't have to change their config files around.
       */
      return context->query_udat()->query_body(char);
   }
   case "bodies":
      return context->query_udat()->query_bodies();
   case "chars":
      return context->query_udat()->query_chars();
   case "rosterisfull":
      return context->query_udat()->roster_is_full();
   case "deletionisallowed":
      return context->query_udat()->deletion_is_allowed();
   case "lastlogon":
      return context->query_udat()->query_last_logon();
   case "userclient":
      return query_udat(StrArg("user", "$UDat.UserClient"))->query_last_client();
   case "useros":
      return query_udat(StrArg("user", "$UDat.UserOs"))->query_last_os();
   case "userbrowser":
      return query_udat(StrArg("user", "$UDat.UserBrowser"))->query_last_browser();
   case "useremail":
      return query_udat(StrArg("user", "$UDat.UserEMail"))->query_email();
   case "userlastlogon":
      return query_udat(StrArg("user", "$UDat.UserPlayTime"))->query_last_logon();
   case "userplaytime":
      return query_udat(StrArg("user", "$UDat.UserPlayTime"))->query_playtime();
   case "usercreationtime":
      return query_udat(StrArg("user", "$UDat.UserCreationTime"))->query_creation_time();
   case "userdayson":
      return query_udat(StrArg("user", "$UDat.UserDaysON"))->query_number_of_days();
   case "userdaystotal":
      return query_udat(StrArg("user", "$UDat.UserDaysTotal"))->query_total_days();
   case "userdayspercent":
      return query_udat(StrArg("user", "$UDat.UserDaysPercent"))->query_percent_of_days();
   case "setprop":
   {
      context->query_udat()->set_property(StrArg("name", "$UDat.SetProp"), 
					  StrArg("val", "$UDat.SetProp"));
   }
   case "queryprop":
   {
      return context->query_udat()->query_property(StrArg("name", "$UDat.QueryProp"));
   }
   case "activex":
      return context->query_user()->query_activex();
   case "java":
      return context->query_user()->query_java();
   case "win":
      return context->query_user()->query_windows();
   case "mac":
      return context->query_user()->query_macintosh();
   case "moz":
      return context->query_user()->query_mozilla();
   case "premium":
      return !!context->query_udat()->query_account_flag("premium");
   case "host":
      return !!DEV_USERD->query_wiztool(context->query_udat()->query_name());
   case "guide":
      return sizeof(rfilter(context->query_udat()->query_bodies(),
			    "query_property", "guide"));
   case "hostguide":
      return !!DEV_USERD->query_wiztool(context->query_udat()->query_name()) ||
	     sizeof(rfilter(context->query_udat()->query_bodies(),
			    "query_property", "guide"));
   }
   return nil;
}

mixed
query_property(string prop) {
   if (prop) {
      if (sscanf(lower_case(prop), "bodies:%s", prop)) {
	 return query_body(prop);
      }
      if (sscanf(lower_case(prop), "all-bodies:%s", prop)) {
	 return query_bodies(prop);
      }
      if (sscanf(lower_case(prop), "udats:%s", prop)) {
	 return query_udat(prop);
      }
   }
}

void set_property(string prop, mixed val, varargs int opaque) {
}

mapping query_properties(varargs int derived) {
   return ([ ]);
}

int query_server_locked() {
   return server_locked;
}

int query_method(string method) {
   switch(lower_case(method)) {
   case "add_name_for_body":
   case "remove_name_for_body":
   case "query_names_of_body":
   case "query_name_of_body":
   case "add_body_to_roster":
   case "remove_body_from_roster":
   case "query_bodies_in_roster":
   case "query_udat":
   case "search_udat":
   case "udat_ipdata":
   case "server_lock":
   case "udat_list":
   case "set_suspended":
   case "query_suspended":
   case "addnote":
   case "killnote":
   case "notes":
      return TRUE;
   default:
   return FALSE;
   }
}

mixed
call_method(string method, mapping args) {
   switch(lower_case(method)) {
   case "add_name_for_body":
      if (typeof(args["body"]) != T_OBJECT) {
	 error("set_body_name expects $body");
      }
      if (typeof(args["name"]) != T_STRING) {
	 error("set_body_name expects $name");
      }
      add_name_for_body(args["body"], args["name"], !!args["primary"]);
      return nil;
   case "remove_name_for_body":
      if (typeof(args["body"]) != T_OBJECT) {
	 error("remove_body_name expects $body");
      }
      if (typeof(args["name"]) != T_STRING) {
	 error("remove_body_name expects $name");
      }
      remove_name_for_body(args["body"], args["name"]);
      return nil;
   case "query_names_of_body":
      if (typeof(args["body"]) != T_OBJECT) {
	 error("query_names_of_body expects $body");
      }
      /* do not return the private pointer! */
      return copy(query_names_of_body(args["body"]));
   case "query_name_of_body":
      if (typeof(args["body"]) != T_OBJECT) {
	 error("query_name_of_body expects $body");
      }
      return query_name_of_body(args["body"]);
   case "add_body_to_roster":
      if (typeof(args["body"]) != T_OBJECT) {
	 error("add_body_to_roster expects $body");
      }
      if (typeof(args["user"]) != T_STRING) {
	 error("add_body_to_roster expects $user");
      }
      add_body_to_roster(args["body"], args["user"]);
      return nil;
   case "remove_body_from_roster":
      if (typeof(args["body"]) != T_OBJECT) {
	 error("remove_body_from_roster expects $body");
      }
      if (typeof(args["user"]) != T_STRING) {
	 error("remove_body_from_roster expects $user");
      }
      remove_body_from_roster(args["body"], args["user"]);
      return nil;
   case "query_bodies_in_roster":
      if (typeof(args["user"]) != T_STRING) {
	 error("query_bodies_in_roster expects $user");
      }
      return query_bodies_in_roster(args["user"]);
   case "query_udat":
      if (typeof(args["name"]) != T_STRING) {
	 error("query_udat expects $name");
      }
      return query_udat(args["name"]);
   case "search_udat":
      if (typeof(args["field"]) != T_STRING) {
	 error("search_udat expects $field");
      }
      if (typeof(args["pattern"]) != T_STRING) {
	 error("search_udat expects $pattern");
      }
      return "~UserAPI/sys/searchdb"->query_matches(args["field"], args["pattern"]);
   case "udat_ipdata":
      if (typeof(args["name"]) != T_STRING) {
	 error("udat_ipdata expects $name");
      }
      if (typeof(args["category"]) != T_STRING) {
	 error("udat_ipdata expects $category");
      }
      if (!query_udat(args["name"])) {
	 return nil;
      }
      return copy(query_udat(args["name"])->query_last_ip_numbers(args["category"]));
   case "server_lock":
      if (typeof(args["locked"]) != T_INT) {
	 return server_locked;
      }
      server_locked = !!args["locked"];
      return server_locked;
   case "udat_list":
     return query_udats();
   case "set_suspended":
     set_suspended(args["reason"], args["user"], args["head"], args["desc"]);
     return nil;
   case "query_suspended":
     return query_suspended(args["user"]);
   case "addnote":
     addnotes(args["user"], args["note"], args["body"], args["timestamp"], args["head"], args["desc"]);
     return nil;
   case "killnote":
     return killnote(args["user"], args["head"], args["desc"]);
   case "notes":
     return notes(args["user"]);
   default:
      return nil;
   }
}

void patch()
{
    int sz;

    for (sz = 0; sz < 256; ++sz) {
	mapping map;

	map = udats_arr[sz];

	if (map) {
	    object *udats;
	    int sz;

	    udats = map_values(map);
	    sz = sizeof(udats);

	    for (sz = sizeof(udats); --sz >= 0; ) {
	        udats[sz]->patch_object_name();
	    }
	}
    }
}
