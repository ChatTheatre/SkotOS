/*
**	~UserDB/obj/bill.c
**
**	Communicate with the little Java servlet.	
**
**
**	000308 Zell	Initial Revision
**
**	Copyright Skotos Tech 1999-2001
*/

# include <kernel/user.h>

private inherit "/lib/url";

inherit "/usr/System/lib/outboundapi";

private object callback;	/* where to call back */
private string message;		/* what to send */

static
void create(int clone) {
   ::create();
}

/* API */

void configure(object ob, string command, mapping data) {
   if (ob && previous_program() == "/usr/UserDB/lib/billapi") {
      callback = ob;
      SysLog("Data: " + dump_value(data));
      message = command + " " + form_encode(data) + "\n";
      connect("127.0.0.1", SYS_INITD->query_userdb_portbase(TRUE) + 79);
      call_out("timeout", 180);
   }
}

static
void timeout() {
   if (callback) {
      callback->bill_reply(([ "error": "TIMEOUT" ]));
   }
   call_out("destroy", 0.1);
}

int receive_message(string line) {
   INFO("received:: " + dump_value(line));
   if (callback) {
      INFO("calling " + dump_value(callback));
      callback->bill_reply(form_decode(line));
   }
   call_out("destroy", 0.1);
   return MODE_LINE;
}

/* network callbacks */

void close(int dest) {
   if (!dest) {
      if (callback) {
	 callback->bill_reply(([ "error": "NOCONNECT" ]));
      }
      CRITICAL("Failed to connect to CreditCardServlet");
      call_out("destroy", 0.1);
   }
}

int open() {
   INFO("CreditCardServlet reached... sending " + dump_value(message));
   message(message);
   return MODE_LINE;
}

static void destroy() {
   INFO("DESTROYING");
   destruct_object();
}
