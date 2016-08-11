/*
**	/base/lib/ownership.c
**
**	This code handles most of the things related to ownership.
**
**	Copyright Skotos Tech Inc 1999
**
**	Initial Revision - Wes Connell
**	Essentially rewritten by Zell 020329 ...
**
*/

private inherit "/lib/array";

# include "/base/include/motion.h"

private mapping incoming_offers; /* offers to ME */
private mapping outgoing_offers; /* offers from ME */

private object offered_to;	/* who I am being offered to */
private object offered_by;	/* who is offering me */

static
void create() {
   incoming_offers = ([ ]);
   outgoing_offers = ([ ]);
}

nomask
mapping query_incoming_offers() {
   return incoming_offers;
}

nomask
mapping query_outgoing_offers() {
   return outgoing_offers;
}

nomask
object *query_all_incoming() {
   object *result, **parts;
   int i;

   result = ({ });
   parts = map_values(incoming_offers);

   for (i = 0; i < sizeof(parts); i ++) {
      result += parts[i];
   }
   return result;
}

nomask
object *query_all_outgoing() {
   object *result, **parts;
   int i;

   result = ({ });
   parts = map_values(outgoing_offers);

   for (i = 0; i < sizeof(parts); i ++) {
      result += parts[i];
   }
   return result;
}

nomask
object query_offered_to() {
   return offered_to;
}

nomask
object query_offered_by() {
   return offered_by;
}

int is_offered(object what) {
   return member(what, query_all_incoming());
}

atomic
void add_offer(object *what, object from_whom) {
   int i;

   if (incoming_offers[from_whom]) {
      incoming_offers[from_whom] |= what;
   } else {
      incoming_offers[from_whom] = what;
   }

   from_whom->add_outgoing_offer(what,this_object());

   for(i = 0; i < sizeof(what); i++) {
      if (what[i]->query_offered_to()) {
	 /* act layer fucked up: be merciless */
	 error("object already offered");
      }
      what[i]->set_offered_to(this_object());
      what[i]->set_offered_by(from_whom);
   }
}

atomic
void remove_offer(object from_whom) {
   object *what;
   int i;

   /* be lax about consistency, just in case: self-repair == good */
   from_whom->remove_outgoing_offer(this_object());

   if (what = incoming_offers[from_whom]) {
      incoming_offers[from_whom] = nil;

      for (i = 0; i < sizeof(what); i ++) {
	 what[i]->set_offered_to(nil);
	 what[i]->set_offered_by(nil);
      }
   }
}

atomic
void remove_from_offer(object from_whom, object ob) {
   if (!from_whom) {
      object *ix;
      int i;

      /* scan incoming offers to find who offered it -- backwards hack */
      SysLog("OWNERSHIP:: Performing backwards hack search for from_whom...");
      ix = map_indices(incoming_offers);
      while (i < sizeof(ix) && !member(ob, incoming_offers[ix[i]])) {
	 i ++;
      }
      if (i < sizeof(ix)) {
	 from_whom = ix[i];
      } else {
	 SysLog("OWNERSHIP:: FAILURE");
      }
   }
   if (from_whom) {
      from_whom->remove_from_outgoing_offer(this_object(), ob);
      if (incoming_offers[from_whom]) {
	 incoming_offers[from_whom] -= ({ ob });
	 if (!sizeof(incoming_offers[from_whom])) {
	    incoming_offers[from_whom] = nil;
	 }
      }
   }
   ob->set_offered_to(nil);
   ob->set_offered_by(nil);
}

atomic
void escape_offer() {
   object *arr;
   int i;

   if (offered_to) {
      Debug("ESCAPE: offer_to = " + dump_value(offered_to));
      if (!offered_by) {
	 /* backwards compat hack */
	 offered_by = query_holder();
	 if (!offered_by) {
	    SysLog("ESCAPING: offered_by in " + dump_value(this_object()) + " is NIL!");
	 }
	 Debug("ESCAPING: offered_by is " + name(offered_by));
      }
      offered_to->remove_from_offer(offered_by, this_object());
   }
   if (map_sizeof(outgoing_offers)) {
      Debug("ESCAPE: outgoing_offers = " + dump_value(outgoing_offers));
   }
   if (map_sizeof(incoming_offers)) {
      Debug("ESCAPE: incoming_offers = " + dump_value(incoming_offers));
   }
   arr = map_indices(outgoing_offers);
   for (i = 0; i < sizeof(arr); i ++) {
      arr[i]->remove_offer(this_object());
   }
   arr = map_indices(incoming_offers);
   for (i = 0; i < sizeof(arr); i ++) {
      remove_offer(arr[i]);
   }
}


/* internal API */
nomask
void set_offered_to(object who) {
   if (previous_program() == "/base/lib/ownership") {
      offered_to = who;
   } else {
      error("bad call");
   }
}

nomask
void set_offered_by(object who) {
   if (previous_program() == "/base/lib/ownership") {
      offered_by = who;
   } else {
      error("bad call");
   }
}

nomask
void add_outgoing_offer(object *what, object who) {
   if (previous_program() == "/base/lib/ownership") {
      if (outgoing_offers[who]) {
	 outgoing_offers[who] |= what;
      } else {
	 outgoing_offers[who] = what;
      }
   } else {
      error("bad call");
   }
}

nomask
void remove_outgoing_offer(object to_whom) {
   if (previous_program() == "/base/lib/ownership") {
      outgoing_offers[to_whom] = nil;
   } else {
      error("bad call");
   }
}

nomask
void remove_from_outgoing_offer(object to_whom, object ob) {
   if (previous_program() == "/base/lib/ownership") {
      if (outgoing_offers[to_whom]) {
	 outgoing_offers[to_whom] -= ({ ob });
	 if (!sizeof(outgoing_offers[to_whom])) {
	    outgoing_offers[to_whom] = nil;
	 }
      }
   } else {
      error("bad call");
   }
}
