/*
**	/base/lib/event.c
**
**	This is where we allow handling of events that happen. The data
**	stored for each handler is opaque; we do not interpret it. At the
**	time when I write this, the natural evaluation code would be SAM.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <base.h>

private inherit "/base/lib/urcalls";

inherit "/core/lib/core_object";

private object result;

# include "/base/include/thing.h"
# include "/base/include/motion.h"
# include "/base/include/environment.h"

static int new_run(string signal, string function, mapping react_map,
		   mapping witness_map, mapping args);
private mapping expand_witness_map(mapping map);


string query_script_registry() {
   return BASE_INITD;
}

int new_signal(string signal, mapping args, varargs mapping react_map,
	       mapping witness_map) {
   witness_map = expand_witness_map(witness_map);
   return new_run(signal, "new_signal", react_map, witness_map, args);
}

/* an alias: this is what it will be, later */
int prime_signal(string signal, mapping args, varargs mapping react_map,
	       mapping witness_map) {
   witness_map = expand_witness_map(witness_map);
   return new_run(signal, "new_signal", react_map, witness_map, args);
}

int pre_signal(string signal, mapping args, varargs mapping react_map,
	       mapping witness_map) {
   witness_map = expand_witness_map(witness_map);
   return new_run(signal, "pre_signal", react_map, witness_map, args);
}

int desc_signal(string signal, mapping args, varargs mapping react_map,
	       mapping witness_map) {
   witness_map = expand_witness_map(witness_map);
   return new_run(signal, "desc_signal", react_map, witness_map, args);
}

void post_signal(string signal, mapping args, varargs mapping react_map,
		 mapping witness_map) {
   witness_map = expand_witness_map(witness_map);
   call_out("new_run", 0, signal, "post_signal", react_map, witness_map, args);
}

static
void post_hook(string hook, string name, mapping args) {
   object runner;

   /* generalize later */
   if (runner = BASE_INITD->query_runner("merry")) {
      if (function_object("post_hook", runner)) {
	 call_other(runner, "post_hook", this_object(), hook, name, args);
      }
   }
}

static
int new_run(string signal, string function, mapping react_map,
	    mapping witness_map, mapping args) {
   return call_runners(function, signal,
		       react_map ? react_map : ([ ]),
		       witness_map, args);
}

private
mapping expand_witness_map(mapping map) {
   string *ix;
   object *arr, ob;
   int i;

   if (!map) {
      map = ([ nil: query_environment() ]);
   }
   ix = map_indices(map);
   for (i = 0; i < sizeof(ix); i ++) {
      map[ix[i]] = ({ map[ix[i]], map[ix[i]]->query_cached_inventory() });
   }
   return map;
}

/* backwards compat: remove these soon */
private
int do_run(string signal, string function, NRef target, mapping args) {
   object env;

   env = query_environment();

   return call_runners(function, signal, target, args,
		       env ? env->query_cached_environment() : ({ }));
}

int run_signal(string signal, varargs NRef target, mapping args) {
   return do_run(signal, "run_signal", target, args);
}


/* this is called by the act_ functions to signal a world event */
void broadcast(string event, mixed args...) {
   object env;

   if (env = query_environment()) {
      env->broadcast_event(event, this_object(), args...);
   }
}

/* something happened to us; tell ourselves */
void occur(string event, mixed args...) {
   mapping users_per_describer;
   mixed describer, *describers;
   object *users;
   int i;

#if 0
   if (sscanf(previous_program(), "/base/lib/%*s")) {  
#endif
      users = query_souls();
      users_per_describer = ([ ]);
      for (i = 0; i < sizeof(users); i ++) {	
	 describer = users[i]->query_describer();
	 if (!users_per_describer[describer]) {
	    users_per_describer[describer] = ({ });
	 }
	 users_per_describer[describer] += ({ users[i] });
      }
      describers = map_indices(users_per_describer) - ({ nil });
      for (i = 0; i < sizeof(describers); i ++) {
	 users = users_per_describer[describers[i]];

	 call_other(describers[i], "describe", event,
		    users, this_object(), args...);
      }
#if 0
   }
#endif
}

/* something happened in the world; broadcast it to interfaces */
void broadcast_locally(string event, object body, mixed args...) {
   mapping users_per_describer;
   mixed describer, *describers;
   object *inv, *users;
   int i, j;

   if (sscanf(previous_program(), "/base/lib/%*s")) {  
      users_per_describer = ([ ]);
      inv = query_inventory();
      for (i = 0; i < sizeof(inv); i ++) {
	 users = inv[i]->query_souls();
	 for (j = 0; j < sizeof(users); j ++) {	
	    describer = users[j]->query_describer();
	    if (!users_per_describer[describer]) {
	       users_per_describer[describer] = ({ });
	    }
	    users_per_describer[describer] += ({ users[j] });
	 }
      }
      describers = map_indices(users_per_describer) - ({ nil });
      for (i = 0; i < sizeof(describers); i ++) {
	 users = users_per_describer[describers[i]];

	 call_other(describers[i], "describe", event,
		    users, body, args...);
      }
   }
}


void broadcast_event(string event, object body, mixed args...) {
   object ob;

   if (sscanf(previous_program(), "/base/lib/%*s")) {  
      ob = this_object();
      do {
	 ob->broadcast_locally(event, body, args...);
	 ob = ob->query_environment();
      } while (ob);
   }
}

object get_result_object() {
   if (!result) {
      result = new_object("/base/data/result");
   }
   return result;
}

object query_result_object() {
   return result;
}

void flush_result_object() {
   if (result) {
      /* output the result of the action that just finished */
      if (sizeof(result->query_error_indices())) {
	 catch {
	    broadcast("error", result);
	 }
      }
      if (sizeof(result->query_output_indices())) {
	 catch {
	    broadcast("output", result);
	 }
      }
      result = nil;
   }
}

void clear_result_object() {
   result = nil;
}
