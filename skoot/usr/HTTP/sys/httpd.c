/*
**	~HTTP/sys/httpd.c
**
**	This is the object associated with the HTTP port. Its primary
**	responsibility is to respond to HTTP requests by cloning and
**	returning the relevant user object.
**
**	It also serves as a relay; depending on the first part of the
**	URL in the request, it may pass it on to other objects that
**	have registered themselves as handlers for those URL's.
**
**	For example, the DevSys module handles all URL's on the form
**
**		/Dev/...
**
**	Copyright Skotos Tech Inc 1999
*/


# include <type.h>
# include <kernel/kernel.h>
# include <HTTP.h>
# include <UserAPI.h>

private inherit "/lib/string";
private inherit "/lib/file";
private inherit "/lib/array";
private inherit "/lib/url";

inherit auth "/usr/UserAPI/lib/authapi";

object	port_master;	/* outdated */

mapping port_nodes;

mapping root_to_handler;
mapping suffix_to_handler;

int locked, booted;

mapping hostnames;

static
void create() {
    auth :: create();

    /* By default we handle all URL's starting with /Open */
    /* MarrachGame is in case we also deal with port 80 for one of the theatres. */
    root_to_handler = ([
	"open":        this_object(),
	"user":        this_object(),
	"marrachgame": this_object(),
	]);
    suffix_to_handler = ([ ]);

    port_nodes = ([ ]);

    hostnames = ([ ]);

    compile_object(HTTPCONN);
    compile_object(HTTPPORT);
}

void patch() {
    if (!port_nodes) {
        port_nodes = ([ ]);
    }
    if (!root_to_handler["marrachgame"]) {
        root_to_handler["marrachgame"] = this_object();
    }
    if (!hostnames) {
       hostnames = ([ ]);
    }
}

void boot(int block) {
   if (sscanf(ur_name(previous_object()), "/usr/HTTP/%*s")) {
      object *nodes;
      int i;

      nodes = map_values(port_nodes);
      for (i = 0; i < sizeof(nodes); i ++) {
	 nodes[i]->boot();
      }
   }
   booted = TRUE;
}

void reboot(int block) {
    boot(block);
}

/* public API */

int register_root(string root) {
   if (root) {
      root = lower_case(root);
      if (!root_to_handler[root] && !sscanf(root, "%*s/")) {
	 /* register a handler for a specific root */
	 root_to_handler[root] = previous_object();
	 return 1;
      }
   }
   return 0;
}

int register_suffix(string suffix) {
   if (!suffix_to_handler[suffix] && !sscanf(suffix, "%*s.")) {
      /* register a handler for a specific suffix */
      suffix_to_handler[suffix] = previous_object();
      return 1;
   }
   return 0;
}

object query_root_handler(string root) {
   return root_to_handler[root];
}

object query_suffix_handler(string suffix) {
   return suffix_to_handler[suffix];
}

mapping query_root_to_handler() { return root_to_handler[..]; }
mapping query_suffix_to_handler() { return suffix_to_handler[..]; }

void claim_port(int port) {
   object node;

   if (port_nodes[port]) {
      if (port_nodes[port]->query_master()) {
	 if (port_nodes[port]->query_master() == previous_object()) {
	    INFO("HTTP port: " + port + " re-claimed by " + ur_name(previous_object()));
	    return;
	 }
	 error("the port is already claimed by " + name(port_nodes[port]->query_master()));
      } else {
	 error("the port is already claimed by " + name(port_nodes[port]));
      }
   }
   INFO("HTTP port: " + port + " claimed by: " + ur_name(previous_object()));
   port_nodes[port] = node = clone_object(HTTPPORT);
   node->set_master(previous_object());
   node->set_port(port);
   if (booted) {
      call_out("boot_node", 0, node);
   }
}

static
void boot_node(object node) {
   node->boot();
}


int query_booted()
{
   return booted;
}

void explicitly_clear_port(int port) {
   port_nodes[port] = nil;
}

void yield_port(int port) {
   if (port_nodes[port] &&
       port_nodes[port]->query_master() == previous_object()) {
      port_nodes[port]->die();
   }
}

object query_port_master(int port) {
   if (port_nodes[port]) {
      return port_nodes[port]->query_master();
   }
}

mapping query_port_nodes()
{
   return port_nodes[..];
}

mapping query_hostnames() {
   return hostnames;
}

int query_hostname(string str) {
   mapping map;

   map = hostnames[str];
   return map && map_sizeof(map);
}

object *query_hostname_owners(string str) {
   mapping map;

   map = hostnames[str];
   return map ? map_indices(map) : ({ });
}

void register_hostname(object obj, string str) {
   mapping map;

   if (str == "INSTANCE_HOSTNAME") {
      str = SYS_INITD->query_hostname();
   }

   map = hostnames[str];
   if (!map) {
      hostnames[str] = map = ([ ]);
   }
   map[obj] = TRUE;
}

void unregister_hostname(object obj, string str) {
   mapping map;

   if (str == "INSTANCE_HOSTNAME") {
      str = SYS_INITD->query_hostname();
   }

   map = hostnames[str];
   if (!map) {
      return;
   }
   map[obj] = nil;
   if (!map_sizeof(map)) {
      hostnames[str] = nil;
   }
}

void lock() {
   locked = TRUE;
}

void unlock() {
   locked = FALSE;
}

int query_locked() { return locked; }


string query_http_root_url_document() {
   return "/Open/Blank.sam";
}

string query_http_login_document() {
  return "https://" + SYS_INITD->query_login_hostname() + "/login.php";
}

string query_http_tos_document() {
  return "https://" + SYS_INITD->query_login_hostname() + "/view-tos.php";
}

string query_http_cc_document() {
  return "https://" + SYS_INITD->query_login_hostname() + "/overview.php";
}

string remap_url(string url, mapping args) {
   string root;
   object handler;

   if (url) {
      url = DRIVER->normalize_path(url, "");
      if (sscanf(url, "/%s/%s", root, url) == 2) {
	 url = "/" + url;
	 root = lower_case(root);
	 if (handler = root_to_handler[root]) {
	    return handler->remap_http_request(root, url, args);
	 }
      }
   }
}

/*
**	We export /open/www ourselves, so e.g. a request for
**
**		/Open/gfx/Mirage.jpeg
**
**	would be answered by sending the file
**
**		/usr/HTTP/data/www/gfx/Mirage.jpeg
*/

string remap_http_request(string root, string url, mapping args) {
   if (root == "open" || root == "marrachgame") {
      return "/usr/HTTP/data/www" + url;
   }
   error("unknown URL root " + root);
}

private
void set_stuff(object conn, object udat) {
   if (conn->query_activex()) {
      udat->set_last_client("activex");
   } else if (conn->query_java()) {
      udat->set_last_client("java");
   } else {
      udat->set_last_client("unknown");
   }

   if (conn->query_windows()) {
      udat->set_last_os("windows");
   } else if (conn->query_macintosh()) {
      udat->set_last_os("macintosh");
   } else {
      udat->set_last_os("unknown");
   }

   if (conn->query_netscape()) {
      udat->set_last_browser("netscape");
   } else if (conn->query_explorer()) {
      udat->set_last_browser("explorer");
   } else {
      udat->set_last_browser("unknown");
   }
}


mixed handle_http_request(string method, object conn, string base,
			  string url, mapping args) {
   return call_other(this_object(), "handle_" + lower_case(base),
		    conn, url, args);
}

static
int handle_open(object conn, string url, mapping args) {
   string user, pass;

   if (url == "/Error.sam") {
      /* Fall-through. */
      return FALSE;
   }
   if (lower_case(url) != "/login") {
     return FALSE;
   }

   user = args["user"];
   if (!user || !strlen(user)) {
      conn->redirect_to(conn->query_node()->query_document("login"),
			([ "complaint":
			 "You did not supply a user name." ]));
      return TRUE;
   }
   user = lower_case(strip(user));
   pass = args["pass"];
   if (!pass || !strlen(pass)) {
      conn->redirect_to(conn->query_node()->query_document("login"),
			([ "complaint":
			 "You did not supply a password." ]));
      return TRUE;
   }

   start_passlogin("create_cookie", user, pass,
		   conn, conn->query_node()->query_document("login"),
		   args["page"] ? args["page"] : "/",
		   user);

   return TRUE;
}

static
void create_cookie(int success, string reply, object conn, string source,
		   string page, string user) {
   if (success) {
      object udat;

      SysLog("HTTP: Logging " + user + " in.");

      udat = UDATD->get_udat(user);
      set_stuff(conn, udat);

      conn->redirect_to(page, ([ ]), ({
	 "Set-Cookie: user=" + url_encode(user) + "; path=/",
	 "Set-Cookie: pass=" + url_encode(reply) + "; path=/",
	    }));
      return;
   }
   switch(reply) {
   case "LOCAL BAD USER/PASS":
   case "BAD PASSWORD":
   case "NO SUCH USER":
      conn->redirect_to(source,
			([ "complaint":
			 "You entered an unknown user/password combination."
			   ]));
      return;
   case "BAD KEYCODE":
   case "BAD CODE":
      conn->redirect_to(source,
			([ "complaint":
			 "Your login has timed out. Please login again."
			   ]));
      return;
   default:
      conn->redirect_to(source,
			([ "complaint":
			 "USERDB ERROR: " + reply ]));
   }
}

void
dump_connections()
{
   object master, current;

   master = find_object("/usr/HTTP/obj/httpconn");
   current = master->query_next_clone();
   while (master && current && current != master) {
      if (current->query_conn()) {
	 DEBUG("HTTP connection (" + name(current) + "): " +
	       query_ip_name(current->query_conn()) + "[" +
	       query_ip_number(current->query_conn()) + "]");
      } else {
	 DEBUG("HTTP connection (" + name(current) + "): Not connected.");
      }
      current = current->query_next_clone();
   }
}
