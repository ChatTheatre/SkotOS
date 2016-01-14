/*
**	~System/sys/devuserd.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <status.h>
# include <kernel/user.h>
# include <kernel/access.h>
# include <System.h>
# include <System/log.h>
# include <base.h>

inherit user "/kernel/lib/user";
inherit rsrc "/kernel/lib/api/rsrc";
inherit access "/kernel/lib/api/access";
inherit "/lib/string";
inherit "/lib/date";

mapping user_to_hash;
mapping user_to_developer;
mapping user_to_wiztool;
object telnet, binary;

private void open_ports();

static
void create() {
   rsrc::create();
   access::create();

   /* set up the telnet logons */

   compile_object(DEV_PORT);
   telnet = clone_object(DEV_PORT);
   binary = clone_object(DEV_PORT);

   compile_object(DEV_USER);
   compile_object(SYS_OBJ_WIZTOOL);

   /* become managers for kernel functionality */
   USERD->set_telnet_manager(0, this_object());
   USERD->set_binary_manager(0, this_object());

   user_to_wiztool = ([ ]);

   user_to_hash = ([ ]);

   set_object_name("System:Developers");
}

string query_state_root() { return "System:Developers"; }

void patch() {
   user_to_hash = ([ ]);
}

void boot(int block) {
    if (ur_name(previous_object()) == SYS_BOOT) {
       open_ports();
    }
}

void reboot(int block) {
   boot(block);
}

private
void open_ports() {
   int port;

   port = SYS_INITD->query_portbase() + 98;

   if (telnet->open_port("telnet", port)) {
      INFO("DEV_USERD: accepting connections on telnet devport " + port);
   } else {
      CRITICAL("Panic: failed to open telnet devport " + port);
   }
   port ++;
   if (binary->open_port("tcp", port)) {
      INFO("DEV_USERD: accepting connections on binary devport " + port);
   } else {
      CRITICAL("Panic: failed to open binary devport " + port);
   }
}



/* calls from /kernel/sys/userd.c */
object select(string name) {
   if (!user_to_hash[name]) {
      return this_object();
   }
   return clone_object(DEV_USER);
}

void clear_developer(string name) {
   user_to_hash[name] = nil;
}

void set_developer(string name) {
   if (name && !user_to_hash[name]) {
      user_to_hash[name] = "foo";
   }
}

void set_password(string name, string pass) {
   if (name) {
      name = lower_case(name);
      user_to_hash[name] = to_hex(hash_md5(name + pass));
   }
}

void set_hash(string name, string hash) {
   if (name) {
      name = lower_case(name);
      user_to_hash[name] = hash;
   }
}

string *query_users() {
   return map_indices(user_to_hash);
}

string query_hash(string name) {
   return user_to_hash[name];
}

int test_password(string name, string pass) {
   return user_to_hash[name] == to_hex(hash_md5(name + pass));
}

object query_wiztool(string name) {
   if (user_to_hash[name]) {
      object tool;

      tool = user_to_wiztool[name];
      if (!tool) {
	 if (!sizeof(access::query_users() & ({ name }))) {
	    add_user(name);
	    set_access(name, "/", FULL_ACCESS);
	 }
	 if (!sizeof(query_owners() & ({ name }))) {
	    add_owner(name);
	 }
	 tool = user_to_wiztool[name] = clone_object(SYS_OBJ_WIZTOOL, name);
      }
      return tool;
   }
}

string query_banner(object connection) {
   if (connection && !connection->query_user()) {
      return "\nWelcome to SkotOS.\n\nWhat is your name: ";
   }
   return nil;
}

int query_timeout(object conn) {
    return 60;
}

int login(string str) {
   previous_object()->message("Unknown user.\n");
   return MODE_DISCONNECT;
}
