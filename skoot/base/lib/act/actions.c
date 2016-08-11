/*
**      /base/lib/act/actions.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <SkotOS.h>
# include <base.h>
# include <base/act.h>
# include <status.h>

private inherit "/lib/string";
private inherit "/lib/array";

# include "/base/include/act.h"
# include "/base/include/events.h"

mixed	*queue;		/* action strings and args arrays interleaved */
mixed	*head;		/* to be inserted at the head of the queue */
string	repeat;		/* perform next state of current action */
int	handle;		/* callout handle */

int query_busy() { return !!handle; }

atomic
void stop_busy() {
   queue = head = nil;
   if (handle != 0) {
      remove_call_out(handle);
      handle = 0;
   }
}

atomic
void cancel_actions() {
   repeat = nil;
   queue = head = nil;
   if (handle > 0) {
      remove_call_out(handle);
      handle = 0;
   }
}

static void do_next_action();

atomic
void action(string action, varargs mapping args, int flags) {
   if (action == "repeat") {
      /* just an alias */
      repeat_action(args ? args["state"] : nil);
      return;
   }
   if (handle == -1) {
      /* we're mid-action, assume insert */
      this_object()->insert_action(action, args, flags);
      return;
   }
   if (!queue) {
      queue = ({ });
   } else if (sizeof(queue) == status()[ST_ARRAYSIZE]) {
      /* callout chain has died or somebody is being bad -- clear */
      queue = ({ });
      if (handle > 0) {
	 remove_call_out(handle);
	 handle = 0;
      }
   }

   queue += ({
      action,
	 args ? copy(args) : ([ ]),
	 flags,
	 "virgin",
	 });

   if (handle == 0) {
      do_next_action();
   }
}

static atomic
void insert_action(string action, varargs mapping args, int flags) {
   if (!queue || !sizeof(queue)) {
      error("insert_action must be called during an action");
   }
   if (!head) {
      head = ({ });
   }

   head += ({
      action,
	 args ? copy(args) : ([ ]),
	 flags,
	 "virgin",
 	 });
}

static atomic
void repeat_action(varargs string str) {
   /* the current action should occur again soon */
   repeat = str ? str : "virgin";
}

static atomic
void atomic_next_action() {
   do_next_action();
}

static
float execute_action(string action, mapping args, varargs int flags,
		     string repeat) {
   string *roles;
   object handler;
   object verb;
   int i;

   if (!repeat) {
      repeat = "virgin";
   }

   args["actor"] = this_object();

   if (function_object("action_" + action, this_object())) {
      float delay;

      if (handler = ACTIONS->query_handler(action + "/head")) {
	 if (!handler->handle_action("prime", args)) {
	    delay = -1.0;
	 }
      }
      if (delay != -1.0) {
	 if (action != "social") {
	    args["current-action"] = action;
	 }
	 delay = call_limited("action_" + action, get_result_object(),
			      repeat, flags, args);

	 if (delay >= 0.0) {
	    if (handler = ACTIONS->query_handler(action + "/tail")) {
	       handler->handle_action("prime", args);
	    }
	 }
      }
      return delay;
   }

   if (handler = ACTIONS->query_handler(action)) {
      if (verb = handler->query_verb()) {
	 roles = handler->query_roles();
	 for (i = 0; i < sizeof(roles); i ++) {
	    if (!args[roles[i]]) {
	       roles[i] = nil;
	    }
	 }
	 roles -= ({ nil });
	 if (sizeof(verb->query_required_roles() - roles)) {
	    SysLog("ACTION: Missing required roles: " +
		   implode(verb->query_required_roles() - roles,
			   ", "));
	    SysLog("ACTION: Queue contains ... " +
		   dump_value(queue));
	 }
      }
   }
   if (!roles) {
      roles = ({ });
   }

   args["vob"] = handler ? handler->query_verb() : nil;
   args["override-action"] = action;
   args["roles"] = roles;
   
   return call_limited("action_social", get_result_object(),
		       repeat, flags, args);
}


/* until we figure something better out */
atomic
void immediate_action(string action, mapping args) {
   if (execute_action(action, args) > 0.0) {
      error("immediate action may not take time");
   }
}


static
void do_next_action() {
   object result;
   string action;
   mixed *args;
   float delay;

   handle = -1;

   if (!queue) {
      handle = 0;
      return;
   }
   if (!sizeof(queue)) {
      /* we're no longer busy */
      handle = 0;
      queue = nil;
      return;
   }

   repeat = nil;

   action = queue[0];

   catch {
      delay = execute_action(action, queue[1], queue[2], queue[3]);
   } : {
      SysLog("Error caught: clearing action schedule, returning...");
      Debug("Queue = " + dump_value(queue));
      handle = 0;
      broadcast("runtime_error", SYSLOGD->query_last_error());
      repeat = nil;
      queue = head = nil;
      return;
   }

   if (!queue) {
      DEBUG("[do_next_action] queue disappeared from under us, aborting.");
      return;
   }

   if ((queue[2] & ACTION_SILENT) == 0) {
      flush_result_object();
   } else {
      clear_result_object();
   }

   if (delay < 0.0) {
      /* negative return value means serious error: abort rest of queue */
      handle = 0;
      queue = head = nil;
      repeat = nil;
      return;
   }
   if (repeat) {
      /* execute the next stage of the same action */
      queue[3] = repeat;
      repeat = nil;
   } else {
      /* or the action is finished: pop the queue */
      queue = queue[4 ..];
   }
   if (head) {
      /* then perform any insertions requested during the action */
      queue = head + queue; 
      head = nil;
   }

   if (delay >= 0.0 && sizeof(queue)) {
      /* reschedule unless queue is empty and no delay should be imposed */
      handle = call_out("atomic_next_action", delay);
   } else {
      handle = 0;
   }
}
