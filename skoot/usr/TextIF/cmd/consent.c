/*
**	/usr/TextIF/cmds/consent.c
**
**	Copyright Skotos Tech Inc 2000
**
**	Initial revision: Wes Connell
*/

# include <base.h>
# include <base/living.h>
# include <base/act.h>
# include <base/preps.h>

private inherit "/lib/string";
private inherit "/lib/array";

private inherit "/base/lib/urcalls";

inherit "/usr/SAM/lib/sam";
inherit "/usr/TextIF/lib/misc";
inherit "/usr/TextIF/lib/tell";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";

# include "/usr/SkotOS/include/describe.h"
# include "/usr/TextIF/include/tell.h"
# include "/usr/SkotOS/include/noun.h"
# include "/usr/SkotOS/include/inglish.h"


void cmd_always(object user, object body, string type, Ob *who) {
   NRef who_ref;

   who_ref = locate_one(user, FALSE, who, body,
			body->query_environment());

   if (!who_ref)
      return;

   if (type == "allow" || type == "deny") {
      user->message("Consent list for " + describe_one(body) + 
		    " has been modified.\n");
      body->action("consent",
		   ([ "what" : NRefOb(who_ref), 
		      "type" : type ]) );
      return;
   }

   user->message("usage: always <allow | deny> <person>\n");
   return;
}


void cmd_never(object user, object body, string type, Ob *who) {
   NRef who_ref;

   who_ref = locate_one(user, FALSE, who, body,
			body->query_environment());

   if (!who_ref)
      return;

   if (type == "allow") {
      user->message("Consent list for " + describe_one(body) + 
		    " has been modified.\n");
      body->action("consent",
		   ([ "what" : NRefOb(who_ref), 
		      "type" : "deny" ]) );
      return;
   }

   if (type == "deny") {
      user->message("Consent list for " + describe_one(body) + 
		    " has been modified.\n");
      body->action("consent",
		   ([ "what" : NRefOb(who_ref),
		      "type" : "allow" ]) );
      return;
   }

   user->message("usage: never <allow | deny> <person>\n");
   return;
}


void cmd_consent(object user, object body, varargs string type, mixed who) {
   NRef who_ref;
   mapping consent;
   mixed *indices;
   int i, indices_size;

   if (type == "list") {
      consent = body->query_consent();
      indices = map_indices(consent);
      indices_size = sizeof(indices);

      if (!sizeof(indices)) {
	 user->message("You have no entries in your consent list.\n");
	 return;
      }

      for (i = 0; i < indices_size; i++) {
	 if (indices[i] == "all") {
	    user->message("You are " + consent[indices[i]] + "ing everybody.\n");
	    continue;
	 }

	 user->message("You are " + consent[indices[i]] + "ing " + 
		       describe_one(indices[i], body, body) + ".\n");
      }
      return;
   }
   if (type == "allow" || type == "deny" || type == "remove") {
      if (typeof(who) == T_STRING) {
	 body->action("consent",
		      ([ "what" : "all",
		       "type" : type ]) );
      } else {
	 if (type == "remove") {
	    consent = body->query_consent();
	    if (consent["all"]) {
		user->message("Did you perhaps means '@consent remove all'?\n");
		return;
	    }
	    if (!map_indices(consent)) {
		user->message("There is noone in your consent list that can be removed.\n");
		return;
	    }
	    who_ref = search_one(user, LF_HUSH_ALL, who, body,
				 map_indices(consent)...);
	    if (!who_ref) {
		user->message("That name cannot be found in your consent-list.\n");
		return;
	    }
	 } else {
	    who_ref = locate_one(user, FALSE, who, body,
				 body->query_environment());
	    if (!who_ref)
	       return;
	 }
	 body->action("consent",
		      ([ "what" : NRefOb(who_ref),
		       "type" : type ]) );

      }
      user->message("Consent list for " + describe_one(body) + 
		    " has been modified.\n");
      return;
   }
   user->message("usage:\n" +
		 "  @consent <allow | deny | remove> <person>\n" +
		 "  @consent list\n");
   return;
}

void list_approach_requests(object user, object body) {
   mapping requests;
   object *indices;
   int i, indices_size;

   requests = body->query_requests();
   indices = map_indices(requests);
   indices_size = sizeof(indices);

   for (i = 0; i < indices_size; i++)
      user->message(describe_one(indices[i]) + " is trying to " +
		    body->query_request_info(indices[i]) + " you.\n");

   return;
}


void cmd_allow(object user, object body, Ob *what) {
   NRef what_ref;

   if (!what) {
      user->message("usage: @allow <person>\n");
      list_approach_requests(user, body);
      return;
   }

   if (what_ref = locate_one(user, FALSE, what, body, 
			     body->query_environment())) {
      body->action("allow", 
		   ([ "who" : NRefOb(what_ref) ]) );
   }
}


void cmd_deny(object user, object body, Ob *what) {
   NRef what_ref;

   if (!what) {
      user->message("usage: @deny <person>\n");
      list_approach_requests(user, body);
      return;
   }

   if (what_ref = locate_one(user, LF_HUSH_ALL, what, body,
			     body->query_environment())) {
      body->action("deny", ([ "who" : NRefOb(what_ref) ]));
      return;
   }
   if (what_ref = search_one(user, FALSE, what, body, 
			     map_indices(body->query_requests())...)) {
      body->action("deny", ([ "who" : NRefOb(what_ref) ]));
   }
}
