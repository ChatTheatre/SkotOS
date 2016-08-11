/*
**	~HTTP/obj/httpport.c
**
**	Copyright Skotos Tech Inc 2001
*/

# include <HTTP.h>

inherit port "/usr/System/lib/port";


object master;
int port;

void set_master(object m) { master = m; }
object query_master() { return master; }

void set_port(int p) { port = p; }
int query_port() { return port; }

static
void create(int clone) {
   if (clone) {
      port::create();
   }
}


int suicide;

int plan_suicide() { suicide = TRUE; }

void die() {
   call_out("unatomic_destruct_object", 0);
}   

void boot() {
   if (previous_program() == HTTPD) {
      if (suicide) {
	 INFO("HTTPD: suiciding on port: " + port);
	 die();
	 return;
      }
      if (open_port("tcp", port)) {
	 INFO("HTTPD: accepting connections on http port: " + port);
	 return;
      }
      CRITICAL("Panic: failed to open HTTP port: " + port);
   }
}

static object open_connection(string ipaddr, int port) {
   object conn;

   /* create and return an object to deal with the new connection */
   conn = clone_object(HTTPCONN);
   conn->set_origin(ipaddr);
   conn->set_node(this_object());

   return conn;
}

string query_document(string name) {
   string doc;

   if (master) {
      doc = call_other(master, "query_http_" + name + "_document");
   }
   if (!doc) {
      doc = call_other(HTTPD, "query_http_" + name + "_document");
   }
   if (!doc) {
      error("failed to find document '" + name + "'");
   }
   return doc;
}
