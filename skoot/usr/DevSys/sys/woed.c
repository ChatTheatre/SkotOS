/*
**	~DevSys/sys/woed.c
**
**	Implement the World Tree of Woe; serve requests
**
**	Copyright Skotos Tech Inc 1999
*/

# include <System.h>

inherit "/usr/System/lib/port";

static
void create() {
   ::create();

   find_or_load("/usr/DevSys/obj/woe");
}

void die() {
   destruct_object();
}


void boot(int block) {
   int port;

   if (ur_name(previous_object()) == "/usr/DevSys/initd") {
      port = SYS_INITD->query_portbase() + 90;
      if (!open_port("tcp", port)) {
	 CRITICAL("Panic: failed to open Woe port " + port);
      }
      INFO("WOED: opened port " + port + " successfully.");
   }
}

void patch() {
   int port;
      port = SYS_INITD->query_portbase() + 90;
      if (!open_port("tcp", port)) {
	 CRITICAL("Panic: failed to open Woe port " + port);
      }
}


void reboot(int block) {
   boot(block);
}

static object open_connection(string ipaddr, int port) {
    return clone_object("/usr/DevSys/obj/woe");
}
