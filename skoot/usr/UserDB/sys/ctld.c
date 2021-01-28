/*
**	~UserDB/sys/ctld.c
**
**	Accept complex queries and modification requests.
**
**	This port is to be kept much more secure than authd.c
**
**	Copyright Skotos Tech 2001
*/

inherit "/usr/System/lib/port";

static
void create() {
   ::create();

   find_or_load("/usr/UserDB/obj/ctl");
}

void boot(int block) {
   int port;

   if (ur_name(previous_object()) == "/usr/UserDB/initd") {
      port = SYS_INITD->query_portbase() + 71;
      if (!open_port("tcp", port)) {
	 SysLog("Panic: failed to open Control port " + port);
      }
   }
}


void reboot(int block) {
   boot(block);
}

static object open_connection(string ipaddr, int port) {
   object ctl;

   if (ipaddr != "127.0.0.1" && ipaddr != "0.0.0.0") {
      error("illegal connection from " + ipaddr);
   }
   ctl = clone_object("/usr/UserDB/obj/ctl");
   ctl->set_remote(ipaddr, port);
   return ctl;
}
