/*
**	~UserAPI/sys/auth.c
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

inherit "/usr/System/lib/outboundapi";

private int open;		/* is the connection currently up? */
private int lost;		/* did we lose the connection? */
private mapping requests;	/* what objects have requested auth? */
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
   INFO("UserAPI/AUTHD: UserDB configured, waiting for incoming connection.");
}

/* API */

int send_request(object ob, string command, string args) {
   XDebug("send_request()");
   if (previous_program() == "/usr/UserAPI/lib/authapi") {
      if (open) {
	 string str;

	 XDebug("send_request() open");

	 if (++counter == 0) counter ++;	/* skip 0 */

	 requests[counter] = ob;
	 if (args) {
	    XDebug("AUTHD: sending [" + command + " " + counter + " " + args + "]");
	    str = command + " " + counter + " " + args + "\n";
	 } else {
	    XDebug("AUTHD: sending [" + command + " " + counter + "]");
	    str = command + " " + counter + "\n";
	 }
	 "~UserAPI/sys/authd_port"->message(str);
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
      ob->auth_reply(counter, FALSE, "TIMEOUT");
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
	    ob->auth_reply(ix, TRUE, line);
	 } else if (sscanf(line, "ERR %s", line)) {
	    ob->auth_reply(ix, FALSE, line);
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
   return MODE_LINE;
}
