/*
**	/core/lib/scripts.c
**
**	Collect the functionality required for this object to trigger
**	events and thus execute scripts.
**
**	Copyright Skotos Tech Inc 1999
*/

inherit "/lib/properties";

string query_script_registry();

static
int call_runners(string function, mixed args...) {
   object *runners;
   int i;

   runners = map_values(query_script_registry()->query_runners());

   for (i = 0; i < sizeof(runners); i ++) {
      if (function_object(function, runners[i]) &&
	  !call_other(runners[i], function, this_object(), args...)) {
	 return FALSE;
      }
   }
   return TRUE;
}

static
int special_signal(string namespace, string signal, varargs mapping args) {
   return call_runners("special_signal", namespace, signal, args);
}

static
void post_hook(string hook, string name, mapping args) {
   call_runners("post_hook", hook, name, args);
}


/* delays */



/* this is an open relay -- currently called from merrynode */

void delayed_call(object ob, string fun, mixed delay, mixed args...) {
   call_out("perform_delayed_call", delay, ob, fun, args);
}

static
void perform_delayed_call(object ob, string fun, mixed *args) {
   call_other(ob, fun, args...);
}

# include <status.h>

string *query_delayed_calls() {
   mixed *calls;
   int i;

   calls = status(this_object())[O_CALLOUTS];
   for (i = 0; i < sizeof(calls); i ++) {
      if (calls[i][CO_FUNCTION] != "_F_skotos_callout") {
	 SysLog("Confusing callout: " + dump_value(calls[i]));
	 calls[i] = nil;
      } else if (calls[i][CO_FIRSTXARG] != "perform_delayed_call") {
	 /* more likely */
	 calls[i] = nil;
      } else {
	 mixed *args;

	 args = calls[i][CO_FIRSTXARG + 2];
	 calls[i] = args[0]->describe_delayed_call(args[1], args[2],
						   calls[i][CO_DELAY]);
      }
   }
   return calls - ({ nil });
}
