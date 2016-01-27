/*
**	~SkootOnInn/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <kernel/access.h>

# include <SAM.h>
# include <HTTP.h>
# include <SkotOS.h>
# include <base.h>

private inherit "/lib/string";
private inherit "/lib/loader";

inherit vault "/usr/SID/lib/vaultnode";
inherit module "/lib/module";

mapping theatres;

static
void create() {
   module::create("HTTP");

   theatres = ([ ]);

   compile_object("/usr/Theatre/obj/theatre");
   compile_object("/usr/Theatre/sys/portal");

   set_object_name("Theatre:Init");
   load_programs("/usr/Theatre/samtags");
   load_programs("/usr/Theatre/sys");
   claim_node("Theatre");

   SAMD->register_root("Theatre");
   BASE_PROPERTIES->register_root_handler("Theatre", this_object());
}

void patch() {
   SAMD->register_root("Theatre");
   BASE_PROPERTIES->register_root_handler("Theatre", this_object());

   compile_object("/usr/Theatre/sys/portal");
}


static
void HTTP_loaded() {
   HTTPD->register_root("Theatre");
}

string remap_http_request(string root, string url, mapping args) {
   if (root == "theatre") {
      return "/usr/Theatre/data/www" + url;
   }
   error("unknown URL root " + root);
}

string *sort_theatres(string *theatres) {
   int i, sz, *rankings;
   mapping groups;
   mapping o2n;
   mixed *list;

   groups = ([ ]);
   o2n = ([ ]);
   sz = sizeof(theatres);
   for (i = 0; i < sz; i++) {
      int ranking;
      object theatre;

      theatre = find_object("Theatre:Theatres:" + theatres[i]);
      o2n[theatre] = theatres[i];
      if (!theatre) {
	 continue;
      }
      ranking = theatre->query_ranking();
      if (groups[ranking]) {
	 groups[ranking][theatre] = TRUE;
      } else {
	 groups[ranking] = ([ theatre: TRUE ]);
      }
   }
   sz = map_sizeof(groups);
   rankings = map_indices(groups);
   list = ({ });
   for (i = 0; i < sz; i++) {
      list += map_indices(groups[rankings[i]]);
   }
   sz = sizeof(list);
   for (i = 0; i < sz; i++) {
      list[i] = o2n[list[i]];
   }
   return list;
}

object get_chatter(object udat, string sought) {
   object *bodies;
   string found;
   int i;

   bodies = udat->query_bodies();

   for (i = 0; i < sizeof(bodies); i ++) {
      found = bodies[i]->query_property("theatre:id");
      if (found && lower_case(found) == lower_case(sought)) {
	 return bodies[i];
      }
   }
   return nil;
}

mixed eval_sam_ref(string service, string ref, object context, mapping args) {
   object udat, theatre;

   ref = lower_case(ref);
   switch (ref) {
   case "list":
   case "list-all": {
       int    i, sz, restricted;
       string *theatres;
       mixed  *list;

       restricted = ref == "list";

       theatres = sort_theatres(IDD->query_objects("Theatre:Theatres"));
       sz = sizeof(theatres);
       list = allocate(sz);
       for (i = 0; i < sz; i++) {
	   object theatre;

	   theatre = find_object("Theatre:Theatres:" + theatres[i]);
	   if (!theatre) {
	       continue;
	   }
	   if (!restricted || (!theatre->query_restricted() &&
			       !theatre->query_hidden())) {
	       list[i] = ({
		   theatre->query_name(),
		   theatre->query_brief(),
		   theatre->query_description(),
		   theatre->query_announcement(),
		   theatre->query_web_port(),
		   theatre->query_http_root_url_document(),
		   theatre->query_realm_url(),
		   theatre->query_hostname()
	       });
	   }
       }
       return list - ({ nil });
   }
   default:
       break;
       
   }

   if (!context) {
      error("internal error: no context");
   }
   udat = context->query_udat();
   if (!udat) {
      error("internal error: no udat");
   }
   if (!context->query_user()) {
      error("internal error: no context connection");
   }
   if (args["theatre"]) {
      theatre = find_object("Theatre:Theatres:" + args["theatre"]);
   }
   if (!theatre) {
      if (!context->query_user()->query_node()) {
	string theatre_name;
	object body;

	body = context->query_user()->query_body();
	if (!body) {
	  error("internal error: no http port node and no body");
	}
	if (body->query_property("Theatre:ID")) {
	   theatre_name = body->query_property("Theatre:ID");
	} else {
	   /* Older hack. -EH */
	   sscanf(name(body), "Chatters:%s:", theatre_name);
	}
	theatre = find_object("Theatre:Theatres:" + theatre_name);
	if (!theatre) {
	  error("internal error: Unknown theatre");
	}
      } else {
	string host;
	object *list;

	host = context->query_user()->query_header("host");
	sscanf(host, "%s:%*d", host);
	list = HTTPD->query_hostname_owners(host);
	if (list && sizeof(list) == 1) {
	   theatre = list[0];
	} else {
	   theatre = context->query_user()->query_node()->query_master();
	   if (!theatre) {
	      error("internal error: no theatre connected to port node");
	   }
	}
      }
   }
   if (!theatre) {
      error("cannot find a valid theatre");
   }
   switch(ref) {
   case "startstory": {
      object chatter;
      mapping *arr;
      int ix;

      chatter = get_chatter(udat, theatre->query_id());

      arr = theatre->query_nugget_mappings(chatter);

      ix = sizeof(arr)/2;

      return ([
	 "left": arr[.. ix-1],
	 "right": arr[ix ..],
	 ]);
   }
   case "id":
      return theatre->query_id();
   case "name":
      return theatre->query_name();
   case "port":
      return theatre->query_web_port();
   case "hostname":
      return theatre->query_hostname();
   case "realmurl":
     return theatre->query_realm_url();
   case "loginurl":
     return theatre->query_login_url();
   case "default":
      return theatre->query_http_root_url_document();
   case "startroom":
      return theatre->query_start_room();
   case "home":
      return theatre->query_home();
   case "office":
      return theatre->query_office();
   case "jail":
      return theatre->query_jail();
   case "urmale":
      return theatre->query_male_body();
   case "urfemale":
      return theatre->query_female_body();
   case "chatter":
      return get_chatter(udat, theatre->query_id());
   case "chatname":
      return theatre->query_id() + ":*";
   case "chatmode":
      return CONFIGD->query("TextIF:ChatMode");
   case "gameid":
      return theatre->query_name();
   case "gamebrief":
      return theatre->query_brief();
   case "gamedesc":
      return theatre->query_description();
   case "restricted":
      return theatre->query_restricted();
   }
   return nil;
}

/*
 * Play root-handler for /base/{lib,sys}/properties.c as well:
 */

mapping
r_query_properties(object who, string root, int derived)
{
    if (root == "theatre") {
      string id;
      object theatre;

      if (!derived) {
	  return nil;
      }

      id = who->query_property("Theatre:ID");
      if (!id) {
	  return nil;
      }

      theatre = find_object("Theatre:Theatres:" + id);
      if (!theatre) {
	  return nil;
      }

      return ([
	  "port":      theatre->query_web_port(),
	  "hostname":  theatre->query_hostname(),
	  "realmurl":  theatre->query_realm_url(),
	  "loginurl":  theatre->query_login_url(),
	  "default":   theatre->query_http_root_url_document(),
	  "startroom": theatre->query_start_room(),
	  "home":      theatre->query_home(),
	  "office":    theatre->query_office(),
	  "jail":      theatre->query_jail(),
	  "gameid":    theatre->query_name(),
	  "gamebrief": theatre->query_brief(),
	  "gamedesc":  theatre->query_description()
      ]);
    }
}

mixed
r_query_property(object who, string root, string prop)
{
    if (root == "theatre") {
        string id;
	object theatre;

        switch (prop) {
	case "whobody":
	   return TRUE;
	case "port":
	case "hostname":
	case "realmurl":
	case "loginurl":
	case "default":
	case "startroom":
	case "home":
	case "office":
	case "jail":
	case "gameid":
	case "gamebrief":
	case "gamedesc":
	    break;
	default:
	    return nil;
	}
	/* Fun, this _may_ cause a recursive call which then aborts with nil */
	id = who->query_property("Theatre:ID");
	if (!id) {
	    return nil;
	}

	theatre = find_object("Theatre:Theatres:" + id);
	if (!theatre) {
	  return nil;
	}

	switch (prop) {
	case "port":      return theatre->query_web_port();
	case "hostname":  return theatre->query_hostname();
	case "realmurl":  return theatre->query_realm_url();
	case "loginurl":  return theatre->query_login_url();
	case "default":   return theatre->query_http_root_url_document();
	case "startroom": return theatre->query_start_room();
	case "home":      return theatre->query_home();
	case "office":    return theatre->query_office();
	case "jail":      return theatre->query_jail();
	case "gameid":    return theatre->query_name();
	case "gamebrief": return theatre->query_brief();
	case "gamedesc":  return theatre->query_description();
	default:          return nil;
	}
    }
}

int
r_set_property(object who, string root, string prop, mixed value, mixed *retval)
{
    DEBUG("[Theatre] r_set_property(" + dump_value(who) + ", " + dump_value(root) + ", " +
	  dump_value(prop) + ", " + dump_value(value) + ", " + dump_value(retval) + ")");

    if (root == "theatre") {
        switch (prop) {
	case "port":
	case "hostname":
	case "realmurl":
	case "loginurl":
	case "default":
	case "startroom":
	case "home":
	case "office":
	case "jail":
	case "gameid":
	case "gamebrief":
	case "gamedesc":
	    return TRUE;
	default:
	    return FALSE;
	}
    }
}

void
r_clear_all_properties(object who, string root)
{
    if (root == "theatre") {
        /* You're kidding, right? */
        return;
    }
}
