/*
**	~UserAPI/sys/ctl.c
**
**	Contact a UserDB somewhere for authentication purposes.
**
**
**	000202 Zell	Initial Revision
**
**	Copyright Skotos Tech 1999-2001
*/

# include <kernel/user.h>
# include <UserAPI.h>

private inherit "/lib/url";

inherit "/usr/System/lib/outboundapi";

private int local;		/* if we're not connected, or standalone */
private int open;		/* is the connection currently up? */
private int lost;		/* did we lose the connection? */
private mapping requests;	/* what objects have requested ctl? */
private int counter;		/* for indexing requests */

static void reconnect();

static
void create() {
   ::create();

   requests = ([ ]);
}

void boot(int block) {
   if (ur_name(previous_object()) == "/usr/UserAPI/initd") {
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
   string host;
   int port, standalone;

   host = SYS_INITD->query_userdb_hostname();
   port = SYS_INITD->query_userdb_portbase();
   standalone = SYS_INITD->query_standalone();

   if (host && port && !standalone) {
      local = LOCAL_USERDB;

#if 0
      INFO("UserAPI/CTLD: Connecting to " + host + " [" + (port+71) + "]");
      connect(host, port + 71);
#endif
      return;
   }
   if (standalone) {
      INFO("UserAPI/CTLD: No UserDB configured; standalone mode.");
      local = LOCAL_THEATRE;
   } else {
      INFO("UserAPI/CTLD: No UserDB configured; local mode.");
      local = LOCAL_LOCAL;
   }
}

int query_local() { return local; }

/* API */

int send_request(object ob, string command, string args) {
   XDebug("send_request()");
   if (previous_program() == "/usr/UserAPI/lib/ctlapi" || previous_object() == this_object()) {
      if (open) {
	 string str;

	 XDebug("send_request() open");

	 if (++counter == 0) counter ++;	/* skip 0 */

	 requests[counter] = ob;
	 if (args) {
	    XDebug("CTLD: sending [" + command + " " + counter + " " + args + "]");
	    str = command + " " + counter + " " + args + "\n";
	 } else {
	    XDebug("CTLD: sending [" + command + " " + counter + "]");
	    str = command + " " + counter + "\n";
	 }
	 "~UserAPI/sys/ctld_port"->message(str);
	 call_out("timeout", 60, counter);

	 return counter;
      }
      XDebug("send_request() closed");
      return 0;
   }
}

static
void timeout(int counter) {
   object ob;

   if (ob = requests[counter]) {
      ob->ctl_reply(counter, FALSE, "TIMEOUT");
      requests[counter] = nil;
   }
}

int receive_message(string line) {
   object ob;
   int ix;

   /* TODO check privilige */
   if (sscanf(line, "%d %s", ix, line) == 2) {
      if (ob = requests[ix]) {
	 if (sscanf(line, "OK %s", line)) {
	    ob->ctl_reply(ix, TRUE, line);
	 } else if (sscanf(line, "ERR %s", line)) {
	    ob->ctl_reply(ix, FALSE, line);
	 } else {
	    error("bad input for ix " + ix + ": " + line);
	 }
	 requests[ix] = nil;
      }
      return MODE_NOCHANGE;
   }
   error("BAD INPUT: " + dump_value(line));
}

/* network callbacks */

void close(int dest) {
   if (!open) {
      CRITICAL(name(this_object()) + " : Failed to connect to UserDB");
   } else {
      CRITICAL(name(this_object()) + " : Lost connection to UserDB");
      lost = TRUE;
   }
   open = FALSE;

   call_out("reconnect", 1);
}

void delayed_open() {
   call_out("open", 0);
}

int open() {
   open = TRUE;
   if (lost) {
      CRITICAL(name(this_object()) + " : Connection to UserDB re-established");
      lost = FALSE;
   } else {
      INFO(name(this_object()) + " : Connection to UserDB established");
   }
   this_object()->send_request(this_object(), "announce",
			       url_encode(SYS_INITD->query_hostname()) + " " +
			       url_encode((string)SYS_INITD->query_portbase()));
		
   return MODE_LINE;
}

void
test_open()
{
    disconnect();
}
