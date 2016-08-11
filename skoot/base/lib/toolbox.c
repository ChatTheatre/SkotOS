/*
**	Perform useful operations on things.
**
**	Copyright Skotos Tech 1999 - 2001
**
**	010307 Zell	Initial Revision
*/

private inherit "/base/lib/urcalls";

# include <base/preps.h>
# include <base.h>
# include <base/thing.h>
# include <base/toolbox.h>
# include <base/bulk.h>
# include <base/act.h>


static atomic object spawn_thing(object ur, varargs int skip_contents);


static atomic
void spawn_initial_contents(object new) {
   mapping total;
   object item, ur;
   string *groups;
   mixed **arr;
   int i, j, result;

   total = ([ ]);

   ur = new->query_ur_object();
   if (!ur) {
      error("object has no urparent");
   }
   groups = ur->pick_content_groups();

   Debug("Spawning from " + dump_value(ur) + "; OK, groups = " + dump_value(groups));

   for (i = 0; i < sizeof(groups); i ++) {
      arr = ur->pick_from_content_group(groups[i]);
      Debug("Picked group array: " + dump_value(arr));

      for (j = 0; j < sizeof(arr); j ++) {
	 item = arr[j][CONTENT_GROUP_OBJ];
	 if (!item) {
	    /* item was destructed and ur object not updated? */
	    continue;
	 }
	 item = spawn_thing(item);

	 item->set_stance(arr[j][CONTENT_GROUP_STANCE]);
	 item->set_preposition(arr[j][CONTENT_GROUP_PREP]);

	 if (arr[j][CONTENT_GROUP_PROX]) {
	    result = item->move(new, NewNRef(new, arr[j][CONTENT_GROUP_PROX]));
	 } else {
	    result = item->move(new);
	 }
	 if (result != GENERIC_SUCCESS) {
	    Debug("Move of " + dump_value(item) + " into " + dump_value(new) + " failed with code " + result);
	    error("failed to move content into spawned item");
	 } else {
	    Debug("successul move of " + dump_value(item) + " into " + dump_value(new));
	 }
	 if (arr[j][CONTENT_GROUP_FLAGS] & CONTENT_FLAG_WEAR) {
	    SysLog("trying to wear: " + dump_value(item));
	    new->action("wear", ([ "articles": ({ item }) ]),
			ACTION_SILENT | ACTION_FORCED);
	 }
	 item->set_property("base:initialspawner", new);
	 total[item] = TRUE;
      }
   }
   new->set_property("base:initialcontents", map_indices(total));
}


static atomic
object spawn_thing(object ur, varargs int skip_contents) {
   string clonable;
   object new;

   if (typeof(ur) != T_OBJECT ||
       function_object("is_thing", ur) != LIB_THING) {
      error("argument 1 to spawn_thing() is not a thing");
   }
   if (sscanf(ur_name(ur), "%s#", clonable)) {
      new = clone_object(clonable);
      new->set_ur_object(ur);

      if (!skip_contents) {
	 spawn_initial_contents(new);
      }
      return new;
   }
   error("argument 1 to spawn_thing() has bad name");
}


/* can 'who' take 'what' without anybody's permission? */

static
int check_ownership(mixed what, object who) {
   object env;

   if (!what) {
      error("bad argument 1 to check_ownership");
   }
   if (IsNRef(what)) {
      what = NRefOb(what);
   }
   if (ur_volition(what)) {
      /* we only own ourselves */
      return what == who;
   }
   /* else we can own an object whose container we can own */
   if (env = what->query_environment()) {
      return check_ownership(env, who);
   }
   /* else we can own rooms that we're inside */
   return who->contained_by(what);
}


/*
**	If we're in a room and we're trying to lick the tapestry
**	(a detail), our sword (held by ourselves), Nino's shield
**	(object held by another volitional) and a bucket down in
**	a well (object container by a non-volitional), this will
**	return a string for the tapestry (the detail id) and the
**	objects myself, Nino and the well for the others.
**
**	If the return value is a string, the peer is a detail in
**	the current room; if it's an object, the peer is always
**	in our current environment. If nil is returned, there is
**	no peer.
**
*/

static
mixed find_peer(NRef target, object looker, varargs int close) {
   object tob;

   tob = NRefOb(target);

   if (looker->contained_by(tob)) {
      /* tob is an environment of ours */
      if (tob == looker->query_environment()) {
	 /* it's our direct environment */
	 if (!IsPrime(target)) {
	    /* a detail there is a peer */
	    return target->query_detail(this_object());
	 }
      }
      return PEER_CONTAINER;
   }

   /* now bubble through environments until we reach our room (or nothing) */
   while (tob->query_environment() != looker->query_environment()) {
      tob = tob->query_environment();
      if (tob == nil) {
	 return PEER_NONE;
      }
      /* check if it's a closed container */

      if (close && sizeof(tob->query_doors(this_object())) &&
	  !sizeof(tob->query_open_doors(this_object()))) {
	 return PEER_CLOSED;
      }	  
   }
   return tob;
}


static
string prep_itoa(int num) {
   switch(num) {
   case PREP_CLOSE_TO:
      return "close to";
   case PREP_AGAINST:
      return "against";
   case PREP_UNDER:
      return "beneath";
   case PREP_ON:
      return "on";
   case PREP_INSIDE:
      return "in";
   case PREP_NEAR:
      return "near";
   case PREP_OVER:
      return "above";
   case PREP_BEHIND:
      return "behind";
   case PREP_BEFORE:
      return "in front of";
   case PREP_BESIDE:
      return "next to";
   }
   return nil;
}

static
int prep_atoi(string str) {
   switch(str) {
   case "close to": case "close by": case "close beside":
      return PREP_CLOSE_TO;
   case "against":
      return PREP_AGAINST;
   case "under": case "beneath": case "below":
      return PREP_UNDER;
   case "on": case "upon": case "on top of": case "atop":
      return PREP_ON;
   case "inside": case "in": case "within":
      return PREP_INSIDE;
   case "near": case "near to": case "by": case "nearby": case "at":
      return PREP_NEAR;
   case "over": case "above":
      return PREP_OVER;
   case "behind":
      return PREP_BEHIND;
   case "before": case "in front of": case "across from":
      return PREP_BEFORE;
   case "aside": case "beside": case "next to":
      return PREP_BESIDE;
   }
   return -1;
}

