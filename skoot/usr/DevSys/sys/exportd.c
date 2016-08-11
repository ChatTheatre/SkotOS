/*
**	~DevSys/sys/exportd.c
**
**	Accept connections from slave development systems who
**	wish to view and/or modify our data.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <System.h>

inherit "/usr/System/lib/port";

static
void create() {
   ::create();

   find_or_load("/usr/DevSys/obj/export");
}

void boot(int block) {
   int port;

   if (ur_name(previous_object()) == "/usr/DevSys/initd") {
      port = SYS_INITD->query_portbase() + 91;
      if (!open_port("tcp", port)) {
	 CRITICAL("Panic: failed to open Woe port " + port);
      }
   }
}

void patch() {
   int port;
   port = SYS_INITD->query_portbase() + 91;
   if (!open_port("tcp", port)) {
      CRITICAL("Panic: failed to open Woe port " + port);
   }
}


void reboot(int block) {
   boot(block);
}

static object open_connection(string ipaddr, int port) {
   if (ipaddr == "127.0.0.1") {
      return clone_object("/usr/DevSys/obj/export");
   }
}
