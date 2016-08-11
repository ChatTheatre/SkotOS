/*
**	/base/lib/motion.c
**
**	This program gives physical objects the ability to move;
**	it maintains pointers to its environment and to its prox,
**	as well as what volitional, if any, is currently holding
**	it. The key function here is move(), which asks the object
**	to move to a new environment and/or proximity.
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/base/lib/urcalls";

# include <base.h>
# include <base/bulk.h>
# include "/base/include/living.h"
# include "/base/include/clothing.h"
# include "/base/include/prox.h"
# include "/base/include/events.h"
# include "/base/include/ownership.h"

private object	env;		/* our current environment */
private NRef	proximity;	/* our current proximity */

private int	cached_motion;	/* find this object fast in an environment? */

private int	immobile;	/* if the object is immovable */


/* called by thing.c when the object has just been created */
static
void create() {
   add_event("moved");
}

/* called by thing.c when we're just about to be destructed */
static atomic
void destructing() {
   if (env) {
      /* leave our current environment */
      env->_F_leaving(this_object());
      env = nil;
   }
   if (proximity && IsPrime(proximity)) {
      /* leave our current proximity */
      NRefOb(proximity)->leave_proximity(this_object());
   }
   catch {
      this_object()->post_move_effects();
   }
}

/* public API: inventory/environment */

int query_immobile() {
   return immobile;
}

void set_immobile(int n) {
   immobile = n;
}

nomask object query_environment() {
   return env;
}

mixed foo() { return proximity; }

nomask NRef query_proximity() {
   if (proximity) {
      return CopyNRef(proximity);
   }
   return nil;
}

nomask object query_holder() {
   object ob;

   ob = query_environment();
   while (ob) {
      if (ur_volition(ob)) {
	 return ob;
      }
      ob = ob->query_environment();
   }
   return nil;
}

nomask int contained_by(object container) {
   object env;

   env = query_environment();

   while (env) {
      if (env == container) {
	 return TRUE;
      }
      env = env->query_environment();
   }
   return FALSE;
}

nomask int query_cached_motion() { return cached_motion; }

atomic
void set_cached_motion(int val) {
   if (cached_motion != val) {
      if (env) {
	 if (val) {
	    env->enter_cache(this_object());
	 } else {
	    env->leave_cache(this_object());
	 }
      }
      cached_motion = val;
      CRAWLERS->start_crawler(this_object());
   }
}


/* base-internal API */

/* this is called if our prox suddenly leaves */
nomask atomic
void prox_leaving() {
   if (sscanf(previous_program(), "/base/lib/%*s") == 0) {
      error("bad caller");
   }
   if (proximity) {
      if (IsPrime(proximity)) {
	 do {
	    proximity = NRefOb(proximity)->query_proximity();
	 } while (proximity && IsPrime(proximity) && ur_volition(NRefOb(proximity)) &&
		  !NRefOb(proximity)->check_auto_allow(this_object()));
	 if (proximity && IsPrime(proximity)) {
	    NRefOb(proximity)->enter_proximity(this_object());
	 }
      } else {
	 proximity = nil;
      }
      clear_preposition();
   }
}

private
void enter_prox(NRef target) {
   NRefOb(target)->enter_proximity(this_object());
}


/* this is the main move function; return values in /include/base/bulk.h */

nomask atomic
int move(object dest, varargs NRef dprox) {
   object ob, me;
   string prog;
   NRef ref;

# if 0
   if (sscanf(previous_program(), "/base/lib/%*s") == 0) {
      error("bad caller");
   }
# endif

   me = this_object();

   if (dest) {
      prog = function_object("is_environment", dest);
      if (!prog || !sscanf(prog, "/base/%*s")) {
	 error("bad destination");
      }
      /* now follow the environment chain upwards */
      ob = dest;
      while (ob) {
	 /* make sure there are no cycles */
	 if (ob == me) {
	    return RES_CYCLIC_CONT;
	 }
	 ob = ob->query_environment();
      }
   }
   if (dprox) {
      if (!IsNRef(dprox)) {
	 error("bad second argument to move");
      }
      /* follow the (prime) prox link */
      ref = dprox;
      while (ref && IsPrime(ref)) {
	 /* make sure the prox chain is not cyclic */
	 if (NRefOb(dprox) == me) {
	    return RES_CYCLIC_PROX;
	 }
	 ref = NRefOb(ref)->query_proximity();
      }
   }


   /* XDebug("XX - move() dest: " + dump_value(dest)); */

   if (env && ur_immobile(me)) {
      return FAIL_IMMOBILE;
   }
   if (query_worn_by()) {
      return FAIL_WORN_CHECK;
   }

   /* bulk system checks */
   if (dest) {
      if (dest->check_max_weight(me)) {
	 return FAIL_MASS_CHECK;
      }
      if (dest->check_capacity(me)) {
	 return FAIL_CAP_CHECK;
      }
      if (ur_combinable(me) && !ur_discrete(me) && !ur_tight(dest)) {
	 return FAIL_WOULD_SPILL;
      }
   }
   /* before we move, invalidate the caches */

   leave_proximates();

   ob = env;
   XDebug("move() prox: " + dump_value(proximity));

   /* before the move, flush the result object */
   flush_result_object();

   escape_offer();

   if (proximity && NRefOb(proximity) && NRefOb(proximity) != env) {
      NRefOb(proximity)->leave_proximity(this_object());
   }
   proximity = nil;
   if (env) {
      env->_F_leaving(this_object(), dest);
      env = nil;
   }
   clear_stance();
   if (dest) {
      env = dest;
      dest->_F_entered(this_object(), ob);
   }

   if (dprox && NRefOb(dprox) != env) {
      enter_prox(dprox);
   }
   proximity = dprox;
   event("moved", ob, env);	/* TODO use? proxes? */

   call_out("post_move_effects", 0);
}
