/*
**	/usr/TextIF/cmds/ownership.c
**
**	This file is used for trading, offering, and accepting goods.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <base.h>
# include <SkotOS.h>
# include <base/living.h>
# include <base/bulk.h>
# include <base/act.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/type";

private inherit "/base/lib/urcalls";

inherit "/usr/SAM/lib/sam";
inherit "/usr/TextIF/lib/misc";
inherit "/usr/TextIF/lib/tell";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";

# include "/usr/SkotOS/include/describe.h"
# include "/usr/TextIF/include/tell.h"
# include "/usr/SkotOS/include/noun.h"

void cmd_offer(object user, object body, Ob *what, string prep, Ob *who) {
   NRef *what_refs, who_ref;
   object *obarr, *indices;
   int i, indices_size, found;
   mapping tmp_offers;

   if (who && sizeof(what)) {
      who_ref = locate_first(user, FALSE, who,
			     new_object("~TextIF/data/filterfarm",
					({ ">carried" })),
			     body,
			     body->query_environment(), body);
      what_refs = locate_best(user, FALSE, what,
			      new_object("~TextIF/data/filterfarm",
					 ({ "<carried", ">worn" })),
			      body,
			      body->query_environment(), body);

      if (!who_ref || !what_refs) {
	 return;
      }

      if (NRefOb(who_ref) == body) {
	 user->message("You cannot offer things to yourself.\n");
	 return;
      }

      if (NRefOb(who_ref)->query_brain("name")) {
	 obarr = allocate(sizeof(what_refs));
	 for(i = 0; i < sizeof(what_refs); i++) {
	    if (IsPrime(what_refs[i])) {
	       obarr[i] = NRefOb(what_refs[i]);
	       what_refs[i] = nil;
	    }
	 }
	 obarr -= ({ nil });
	 what_refs -= ({ nil });
	 if (sizeof(what_refs)) {
	    user->message("You cannot offer details such as " +
			  describe_many(what_refs, nil, body) + ".\n");
	 }
	 if (sizeof(obarr)) {
	    NPCBRAINS->npc_give( user, body, obarr, NRefOb(who_ref) );
	 }
	 return;
      }
      body->action("offer", ([ "what" : what_refs, "who" : NRefOb(who_ref) ]));
      return;
   }
   tmp_offers = body->query_outgoing_offers();

   indices = map_indices(tmp_offers);
   indices_size = sizeof(indices);

   for (i = 0; i < indices_size; i++) {
      if (sizeof(tmp_offers[indices[i]])) {
	 found = TRUE;
	 user->message("You are offering " +
		       describe_many(tmp_offers[indices[i]], who_ref, who_ref) +
		       " to " +
		       describe_one(indices[i], who_ref, who_ref) + ". ");
      }
   }
   if (found) {
      user->message("\n");
   } else {
      user->message("You currently have no pending offers.\n");
   }
}

void cmd_give(object user, object body, mixed args...) {
   if (!sizeof(args)) {
      user->paragraph("Give what to whom?");
      return;
   }
   cmd_offer(user, body, args...);
}

void cmd_accept(object user, object body, Ob *what, mixed who) {
   mapping incoming;
   mapping tmp_offers;
   object *indices, *what_list;
   NRef who_ref, *what_refs;
   int i;

   incoming = body->query_incoming_offers();

   if (!what && !who) {
      int found;

      /* dump the incoming offers mapping */
      tmp_offers = incoming;
      indices = map_indices(tmp_offers);

      for (i = 0; i < sizeof(indices); i++) {
	 if (sizeof(tmp_offers[indices[i]])) {
	    found = TRUE;
	    user->message(describe_one(indices[i], nil, body) +
			  " is offering you " +
			  describe_many(tmp_offers[indices[i]], 
					indices[i], body) + ". ");
	 }
      }
      if (found) {
	 user->message("\n");
      } else {
	 user->message("There are no offers for you.\n");
      }
      return;
   }

   if (what && who) {
      who_ref = locate_one(user, FALSE, ({ who }), body,
			   body->query_environment(), body);
      if (!who_ref)
	 return;

      if (!incoming[NRefOb(who_ref)]) {
	 user->paragraph("You have no offers from " +
			 describe_one(who_ref));
	 return;
      }

      what_refs = search_best(user, FALSE, what,
			      new_object("~TextIF/data/filterfarm",
					 ({ })),
			      body,
			      incoming[NRefOb(who_ref)]...);
      if (!what_refs)
	 return;
   }

   if (!what) {
      who_ref = locate_one(user, FALSE, ({ who }), body,
			   body->query_environment(), body);
      if (!who_ref)
	 return;

      if (!incoming[NRefOb(who_ref)]) {
	 user->paragraph("You have no offers from " +
			 describe_one(who_ref));
	 return;
      }

      what_list = incoming[NRefOb(who_ref)];
      what_refs = allocate(sizeof(what_list));

      for (i = 0; i < sizeof(what_list); i++) {
	 what_refs[i] = NewNRef(what_list[i],"NIL");
      }
   }

   if (!who) {
      /* there should only be 1 ref */
      what_refs = search_best(user, FALSE, what,
			      new_object("~TextIF/data/filterfarm",
					 ({ })),
			      body,
			      body->query_all_incoming()...);

      if (!what_refs)
	 return;
      
      /* this is hackish but what the heck */
      who_ref = NewNRef(NRefOb(what_refs[0])->query_environment(),"NIL");
   }

   if (NRefOb(who_ref)->query_environment() != body->query_environment()) {
      user->message(describe_one(NRefOb(who_ref)) + " is not here.\n");
      return;
   }

   what_list = allocate(sizeof(what_refs));

   for (i = 0; i < sizeof(what_list); i ++) {
      what_list[i] = NRefOb(what_refs[i]);
   }

   /* finally! call act_accept() */
   body->action("accept",
		([ "who" : NRefOb(who_ref), 
		   "what" : what_list ]) );
}
