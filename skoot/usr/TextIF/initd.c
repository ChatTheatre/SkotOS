
/*
**	~TextIF/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <HTTP.h>
# include <SkotOS.h>
# include <System.h>
# include <TextIF.h>
# include <UserAPI.h>

# include <SAM.h>

inherit "/lib/womble";

private inherit "/lib/loader";

private inherit "/lib/string";
private inherit "/usr/SkotOS/lib/auth";

inherit sequencer "/lib/sequencer";
inherit port "/usr/System/lib/port";
inherit vault "~SID/lib/vaultnode";

int last_day;

SAM banner;

# define LOG_PAUSE	(5 * 60)

void womble_banner() {
   banner = womble(banner);
}

string query_state_root() { return "TextIF:Init"; }

static
void create() {
   port::create();
   vault::create("/usr/TextIF/data/vault");

   sequencer::create("~TextIF/main",
		     "~TextIF/sys/chatlines");

   set_object_name("TextIF:Init");

   compile_object("~TextIF/obj/user");
   compile_object("testmarkup");
   compile_object(ERRORDB);
   compile_object(OUTPUTDB);
   compile_object(INFOD);

   load_programs("/usr/TextIF/cmd");
   load_programs("/usr/TextIF/data");
   load_programs("/usr/TextIF/sys");
   load_programs("/usr/TextIF/sys/cmds");
   load_programs("/usr/TextIF/sys/parser");

   claim_node("TextIF");

   call_out("log_users", 10);
}

void patch() {
   banner = read_file(TEXT_BANNER);
}

mixed query_banner(varargs object connection) {
   if (connection) {
       /* Hack to work around current function overload. */
       return nil;
   }
   return banner;
}

void set_banner(SAM b) {
   banner = b;
}

private
void open_ports() {
   int port;

   port = SYS_INITD->query_real_textport();

   if (open_port("tcp", port)) {
      INFO("TextIF: accepting connections on game port " + port);
   } else {
      CRITICAL("Panic: failed to open game port " + port);
   }
}

int boot(int block) {
   if (ur_name(previous_object()) == MODULED) {
      HTTPD->register_root("TextIF");
      open_ports();
      call_daemons("boot", FALSE);
      spawn_now(0);
      return 20;
   }
}

void archives_parsed() {
   MODULED->daemon_done();
}

int reboot(int block) {
   if (ur_name(previous_object()) == MODULED) {
      open_ports();
      call_daemons("reboot", FALSE);
   }
}

static object open_connection(string ipaddr, int port) {
   /* create and return an object to deal with the new connection */
   return clone_object("~TextIF/obj/user");
}

/* we're guaranteed to be authenticated when we get here */
string remap_http_request(string base, string url, mapping args) {
   return "/usr/TextIF/data/www" + url;
}


# define DAY	(24 * 3600)

# define START	969519600	/* midnight sep 21st 2000 PST */

int query_last_day() { return last_day; }

void log_day() {
   string *created, *played;
   int day;

   day = (time() - START) / DAY;
   while (day > last_day) {
      created = UDATD->query_users_created_between(START + last_day * DAY,
						   START + (last_day + 1) * DAY);
      played = UDATD->query_users_who_last_played_between(START + last_day * DAY,
							  START + (last_day + 1) * DAY);
      write_file("/usr/TextIF/data/www/CreateLog.TXT",
		 ctime(START + last_day * DAY) +
		 pad_left("Created: " + sizeof(created), 20) +
		 pad_left("Lasted: " + (sizeof(created) - sizeof(created & played)), 20) +

		 "\n");
      last_day ++;
   }
}

static void log_users() {
   mapping map;
   object master, ob, *users;
   string name;
   int bodies, staff, i;

   if (master = find_object("~TextIF/obj/user")) {
      users = INFOD->query_current_user_objects();
      map = ([ ]);

      for(i = 0; i < sizeof(users); i++) {
	 if (users[i]->query_conn() && users[i]->query_body()) {
	    if (name = users[i]->query_name()) {
	       if (!map[name] && is_root(name)) {
		  staff ++;
	       }
	       map[name] = TRUE;
	       bodies ++;
	    }
	 }
      }
      write_file("/usr/TextIF/data/www/UserLog.TXT",
		 ctime(time()) +
		 pad_left("Customers: " + (map_sizeof(map) - staff), 20) +
		 pad_left("Staff: " + staff, 15) +
		 pad_left("Bodies: " + bodies, 15) +
		 "\n");
      call_out("log_users", LOG_PAUSE - (time() % LOG_PAUSE));
   }
   log_day();
}

