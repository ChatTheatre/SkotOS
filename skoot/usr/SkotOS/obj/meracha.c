/*
**	/usr/SkotOS/obj/meracha.c
**
**	MerAcHa, Merry Action Handler, a plugin object conforming to
**	the interface required by /base/sys/actions.c for objects to
**	register there as action handlers (i.e. handle_action).
*/

# include <base.h>
# include <core.h>

inherit "/usr/SkotOS/lib/merryapi";
inherit "/usr/SkotOS/lib/ursocials";

inherit "/core/lib/core_object";

string action;
object verb;

string query_state_root() { return "SkotOS:MerAcHa"; }

string query_script_registry() {
   return BASE_INITD;
}

string query_dp_registry() {
   return CORE_DP_REGISTRY;
}

static
void create(int clone) {
   if (!clone) {
      set_object_name("SkotOS:MerAcHa");
   }
   :: create();
}

string query_action() {
   return action;
}

atomic
void set_action(string s) {
   if (action) {
      "/base/sys/actions"->deregister_handler(action);
   }
   action = s;
   while ("/base/sys/actions"->query_handler(action)) {
      action += "_backup";
   }
   "/base/sys/actions"->register_handler(action, this_object());
}

int handle_action(string phase, mapping args) {
   return !!run_merry(this_object(), "action",
		      "global" + (phase != "prime" ? "-" + phase : ""),
		      args);
}

atomic
void set_verb(object v) {
   if (verb) {
      verb->remove_handler_dependency(this_object());
   }
   verb = v;
   if (verb) {
      verb->add_handler_dependency(this_object());
   }
}

object query_verb() { return verb; }

string *query_roles() {
   if (verb) {
      return ur_roles(verb);
   }
   return ({ });
}

void die() {
    destruct_object();
}

void suicide() {
    call_out("die", 0);
}
