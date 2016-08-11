/*
**	~TextIF/sys/infod.c
**
**	Returns information about the current state of the game.
**	The information is limited to stuff the TextIF layer would know.
**
**	Copyright Skotos Tech Inc 2000
*/

# include <type.h>

private inherit "/lib/string";
private inherit "/lib/mapargs";
private inherit "/usr/TextIF/lib/misc";
private inherit "/usr/SkotOS/lib/auth";

private mapping current_users;
private mapping current_names;

# include <SAM.h>
# include <SkotOS.h>
# include <mapargs.h>

# include "/usr/SAM/include/sam.h"

atomic
void patch_currents() {
   object master, ob;

   current_users = ([ ]);
   current_names = ([ ]);

   if (master = find_object("~TextIF/obj/user")) {
      for (ob = master->query_next_clone(); ob != master;
	   ob = ob->query_next_clone()) {
	 if (ob->query_name()) {
	    current_users[ob] = TRUE;
	    current_names[ob->query_name()] = TRUE;
	 }
      }
   }
}

void test_currents() {
   mapping test;
   object master, ob;

   test = current_users[..];

   if (master = find_object("~TextIF/obj/user")) {
      for (ob = master->query_next_clone(); ob != master;
	   ob = ob->query_next_clone()) {
	 if (ob->query_name()) {
	    if (!test[ob]) {
	       SysLog("Missing in current_users: " + ob->query_name());
	    }
	    test[ob] = nil;
	 }
      }
      if (map_sizeof(test)) {
	 SysLog("Extra in current_users: " + dump_value(map_indices(test)));
      }
   }
}

static
void create() {
   SAMD->register_root("Info");
   set_object_name("TextIF:Info");
   current_users = ([ ]);
   current_names = ([ ]);
}

void patch()
{
   set_object_name("TextIF:Info");
}

atomic
void logout(object user) {
   current_users[user] = nil;
   current_names[user->query_name()] = nil;
}

atomic
void login(object user) {
   current_users[user] = TRUE;
   current_names[user->query_name()] = TRUE;
}

object *query_current_user_objects() {
   return map_indices(current_users);
}

object *query_current_user_names() {
   return map_indices(current_names);
}


/* SAM service */

mixed eval_sam_ref(string service, string ref, object context, mapping args) {
   switch(lower_case(ref)) {
   case "users":
      return query_current_user_objects();
   case "username":
      AssertArg("val", "Info Daemon - Username");
      return info_username(args["val"]);
   case "ip":
      AssertArg("val", "Info Daemon - IP");
      return info_ip(args["val"]);
   case "body":
      AssertArg("val", "Info Daemon - Bodies");
      return info_body(args["val"]);
   case "location":
      AssertArg("val", "Info Daemon - Location");
      return info_location(args["val"]);
   case "staff":
      AssertArg("val", "Info Daemon - Staff");
      return info_staff(args["val"]);
   case "usercount":
      return info_usercount();
   case "staffcount":
      return info_staffcount();
   case "customercount":
      return info_customercount();
   }
   
   return nil;
}

# if 0
int is_combat_verb(string verb) {
   string *nodes;
   int i;

   nodes = COMBATDB->query_node_names();

   for(i = 0; i < sizeof(nodes); i++) {
      if (lower_case(nodes[i]) == lower_case(verb)) {
	 return TRUE;
      }
   }

   return FALSE;
}
# endif

void
verify_currents()
{
   int i, sz;
   object *list;

   list = query_current_user_objects();
   sz = sizeof(list);
   for (i = 0; i < sz; i++) {
      if (list[i]->query_conn()) {
	 continue;
      }
      DEBUG("list[" + i + "] = " +
	    dump_value(list[i]) + "; name = " +
	    dump_value(list[i]->query_name()) +
	    "; conn = " +
	    dump_value(list[i]->query_conn()) + ".");
      /* list[i]->logout(1); */
   }
}

int query_method(string method)
{
   switch (method) {
   case "users":
   case "bodies":
   case "username":
   case "ip":
   case "ipnr":
   case "body":
   case "location":
   case "staff":
   case "usercount":
   case "staffcount":
   case "customercount":
      return TRUE;
   default:
      return FALSE;
   }
}

mixed call_method(string method, mapping args)
{
   switch (method) {
   case "users":
      return query_current_user_objects();
   case "bodies": {
      int i, sz;
      object *list;

      list = query_current_user_objects();
      sz = sizeof(list);
      for (i = 0; i < sz; i++) {
	 list[i] = list[i]->query_body();
      }
      return list - ({ nil });
   }
   case "username": {
      mixed user;

      user = args["user"];
      if (typeof(user) == T_OBJECT) {
	 return user->query_name();
      }
      return nil;
   }
   case "ip": {
      mixed user;

      user = args["user"];
      if (typeof(user) == T_OBJECT) {
	 return query_ip_name(user->query_conn());
      }
      return nil;
   }
   case "ipnr": {
      mixed user;

      user = args["user"];
      if (typeof(user) == T_OBJECT) {
	 return query_ip_number(user->query_conn());
      }
      return nil;
   }
   case "body": {
      mixed user;

      user = args["user"];
      if (typeof(user) == T_OBJECT) {
	 return user->query_body();
      }
      return nil;
   }
   case "location": {
      mixed user;

      user = args["user"];
      if (typeof(user) == T_OBJECT && user->query_body() && user->query_body()->query_environment()) {
	 return name(user->query_body()->query_environment());
      }
      return nil;
   }
   case "staff": {
      mixed user;

      user = args["user"];
      if (typeof(user) == T_OBJECT) {
	 return is_root(user->query_name());
      }
      return FALSE;
   }
   case "usercount":
      return sizeof(query_current_user_objects());
   case "staffcount": {
      int i, sz, count;
      object *users;

      users = query_current_user_objects();
      sz = sizeof(users);
      for (i = 0; i < sz; i++) {
	 if (is_root(users[i]->query_name())) {
	    count++;
	 }
      }
      return count;
   }
   case "customercount": {
      int i, sz, count;
      object *users;

      users = query_current_user_objects();
      sz = sizeof(users);
      for (i = 0; i < sz; i++) {
	 if (!is_root(users[i]->query_name())) {
	    count++;
	 }
      }
      return count;
   }
   default:
      error("Unsupported method");
   }
}
