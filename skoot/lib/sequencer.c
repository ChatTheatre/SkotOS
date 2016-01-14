/*
**	/lib/sequencer.c
**
**	This program aids in bootup sequences, allowing various
**	daemons to flag their initialization as blocking.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <System/log.h>

private inherit "/lib/array";

private string *daemons;

private int	boot_ix, blocking, timeout;
private string	boot_fun;

static
void create(string d...) {
   int i;

   for (i = 0; i < sizeof(d); i ++) {
      find_or_load(d[i]);
   }
   daemons = d;
}

static
void add_daemon(string d) {
   find_or_load(d);
   daemons |= ({ d });
}

string *query_daemons() {
   return daemons;
}

static
void call_daemons(string fun, int block) {
   boot_ix = 0;
   boot_fun = fun;
   blocking = block;
   if (block) {
      call_out("boot_next_daemon", 0);
   } else {
      this_object()->boot_next_daemon();
   }
}

/* this can be masked */
static
mixed call_daemon(object obj, string fun, int blocking) {
   return call_other(obj, fun, blocking);
}

static
void boot_next_daemon() {
   object obj;
   mixed wait;

   while (boot_ix < sizeof(daemons)) {
      catch {
	 obj = find_object(daemons[boot_ix]);
	 if (!obj) {
	    obj = compile_object(daemons[boot_ix]);
	 }
	 INFO(" -- " + daemons[boot_ix] + "...");

	 find_or_load(daemons[boot_ix]);

	 /* if the daemon wants to do this, let it */
	 if (function_object(boot_fun, obj)) {
	    if ((wait = call_daemon(obj, boot_fun, blocking)) && blocking) {
	       timeout = call_out("timeout", wait);
	       return;
	    }
	 }
      }
      /* else try next one */
      boot_ix ++;
   }
   /* all done */
   if (function_object(boot_fun + "_sequence_done", this_object())) {
      call_other(this_object(), boot_fun + "_sequence_done");
   }
   boot_fun = nil;
   blocking = 0;
}

static
void timeout() {
   WARNING(" -- Warning: TIMEOUT for " + daemons[boot_ix]);
   timeout = 0;
   boot_ix ++;
   boot_next_daemon();
}

void daemon_done() {
   if (timeout && member(ur_name(previous_object()), daemons)) {
      remove_call_out(timeout); timeout = 0;
      boot_ix ++;
      call_out("boot_next_daemon", 0);
   }
}
