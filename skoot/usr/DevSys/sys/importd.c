/*
**	~DevSys/sys/importd.c
**
**	Contact a DevSys master running somewhere.
**
**	Copyright Skotos Tech 1999-2001
*/

# include <kernel/user.h>

inherit "~System/lib/outboundapi";

private int open;		/* is the connection currently up? */
private int lost;		/* did we lose the connection? */

private string	host;
private int	port;

static void reconnect();

static
void create() {
   ::create();

   host = "127.0.0.1";
   port = 8091;
}

void boot(int block) {
   if (ur_name(previous_object()) == "/usr/DevSys/initd") {
      reconnect();
   }
}

void patch() {
   reconnect();
}

void reboot(int block) {
   boot(block);
}


static
void reconnect() {
   INFO("DevSys/Importer: Connecting to " + host + " [" + port + "]");
   connect(host, port);
}

/* API */

int receive_message(string line) {
   error("BAD INPUT: " + dump_value(line));
}

/* network callbacks */

void close(int dest) {
   if (!open) {
      CRITICAL(name(this_object()) + " : Failed to connect to DevSys Master");
   } else {
      CRITICAL(name(this_object()) + " : Lost connection to DevSys Master");
      lost = TRUE;
   }
   open = FALSE;

   call_out("reconnect", 1);
}

int open() {
   open = TRUE;
   if (lost) {
      CRITICAL(name(this_object()) + " : Connection to DevSys Master re-established");
      lost = FALSE;
   } else {
      INFO(name(this_object()) + " : Connection to DevSys Master established");
   }
   return MODE_LINE;
}
