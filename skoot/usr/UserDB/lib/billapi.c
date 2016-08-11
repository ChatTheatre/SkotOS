/*
**	~SkotOS/lib/billapi.c
**
**	Create and talk to a Billing connection.
**
**
**	000202 Zell	Initial Revision
**
**	Copyright Skotos Tech 1999-2001
*/

private mapping ob_to_callback;
private mapping ob_to_args;
private mapping ob_to_originator;

static
void create() {
   ob_to_callback = ([ ]);
   ob_to_args = ([ ]);
   ob_to_originator = ([ ]);
}

void patch() {
   ob_to_originator = ([ ]);
}

static /* atomic buggery */
void bill_request(string callback, mixed *args, string cmd,
		  mapping data, int retries, varargs object ob) {
   /* we trust the clone entirely */

   ob = clone_object("/usr/UserDB/obj/bill");

   ob_to_callback[ob] = callback;
   ob_to_originator[ob] = query_originator();
   ob_to_args[ob] = args;

   ob->configure(this_object(), cmd, data);
}

atomic
void bill_reply(mapping reply) {
   object ob;

   SysLog("OK, in bill_reply() ... ");

   if (previous_program() == "/usr/UserDB/obj/bill") {
      string fun;

      ob = previous_object();

      if (fun = ob_to_callback[ob]) {
	 set_originator(ob_to_originator[ob]);

	 SysLog("BILL: reply [" + ur_name(ob) + ": " + dump_value(ob_to_args[ob])+ "]");
	 call_out(fun, 0, reply, ob_to_args[ob]...);
	 ob_to_callback[ob] = nil;
      }
      ob_to_args[ob] = nil;
   }
}
