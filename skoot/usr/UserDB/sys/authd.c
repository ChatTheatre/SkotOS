/*
**	~UserDB/sys/authd.c
**
**	Allow external clients to log in and authenticate users.
**
**	Copyright Skotos Tech 2001
*/

inherit "/usr/System/lib/port";

static
void create() {
   ::create();

   find_or_load("/usr/UserDB/obj/auth");
}

void boot(int block) {
   int port;

   if (ur_name(previous_object()) == "/usr/UserDB/initd") {
      port = SYS_INITD->query_portbase() + 70;
      if (!open_port("tcp", port)) {
	 SysLog("Panic: failed to open Auth port " + port);
      }
   }
}


void reboot(int block) {
   boot(block);
}

static object open_connection(string ipaddr, int port) {
   if (ipaddr != "127.0.0.1") {
     SysLog("ILLEGAL CONNECTION ATTEMPT ON AUTH: " + ipaddr + "/" + port);
   }
   return clone_object("/usr/UserDB/obj/auth");
}
