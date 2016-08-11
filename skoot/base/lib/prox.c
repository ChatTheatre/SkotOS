/*
**	/base/lib/prox.c
**
**	This code defines the behaviour of a 'prox'; something other
**	objects can be near/under/over/whatnot. It works somewhat like
**	an environment, except that our 'contents' do not follow when
**	we move; rather, they are informed of the move and free to do
**	what they like (though right now, it always means they lose us
**	as their prox).
**
**	Copyright Skotos Tech Inc 1999
*/

# include <base.h>
# include "/base/include/prox.h"
# include "/base/include/environment.h"

private mapping proximate;

/* called by thing.c when the object has just been created */
static
void create() {
   proximate = ([ ]);
}

/* called by thing.c when the object is about to be destroyed */
static atomic
void destructing() {
   /* we're about to die; let go of objects whose prox we are */
   leave_proximates();
}

/* public API */
nomask
object *query_proximate() {
   return map_indices(proximate);
}

nomask atomic
void leave_proximates(varargs mixed what) {
   object *obarr;
   int i;

   if (what) {
      obarr = query_near(what);
   } else {
      obarr = map_indices(proximate);
   }
   for (i = 0; i < sizeof(obarr); i ++) {
      if (obarr[i]) {
	 obarr[i]->prox_leaving();
      }
   }
   proximate = ([ ]);
}


/* base-internal API */


/* another object wants to be our prox; called from move() */
nomask
void enter_proximity(object which) {
   if (sscanf(previous_program(), "/base/lib/%*s") == 0) {
      error("bad caller");
   }
   if (proximate[which]) {
      ERROR("ERROR:: Object " + dump_value(which) + " already proximate in " + dump_value(this_object()));
      /* error("object already present"); */
   }
   proximate[which] = which;
}

/* another object wants to leave our prox; called from move() */
nomask
void leave_proximity(object which) {
   if (sscanf(previous_program(), "/base/lib/%*s") == 0) {
      error("bad caller");
   }
   if (!proximate[which]) {
      ERROR("ERROR:: Object " + dump_value(which) + " not proximate in " + dump_value(this_object()));
      /* error("object not present"); */
   }
   proximate[which] = nil;
}
