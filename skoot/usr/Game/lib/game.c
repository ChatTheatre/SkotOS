/*
 *	/usr/Game/lib/game.c
 *
 *	Copyright Skotos Tech Inc 2001
 *
 *	This object should be inherited by one single object, preferable
 *	something like ~Yourgame/initd.c, which should then makes itself
 *	available for configuration through WOE as Yourgame:Init or so, and
 *	which will automatically handle the Theatre:* SAM-namespace and the
 *      Theatre:* properties.
 */

# include <base.h>
# include <SAM.h>
# include <HTTP.h>

inherit "~/lib/data";

private object male_body, female_body;

string query_state_root() { return "Game:Game"; }

static void
create()
{
   ::create();
   SAMD->register_root("Theatre");
   BASE_PROPERTIES->register_root_handler("Theatre", this_object());
}

object query_male_body()          { return male_body;    }
object query_female_body()        { return female_body;  }

void   set_male_body(object o)    { male_body = o;       }
void   set_female_body(object o)  { female_body = o;     }

/*
 * Overrule function to actually reclaim/yield ports if necessary.
 * Don't do this until the code is actually 'live'.
 */

# if 0
void
set_web_port(int new_port)
{
   int old_port;

   old_port = query_web_port();
   if (new_port == old_port) {
      return;
   }
   if (old_port) {
      HTTPD->yield_port(old_port);
   }
   HTTPD->claim_port(new_port);
   ::set_web_port(new_port);
}
# endif

/*
 * Support function for the SAM Theatre:* namespace.
 */

mixed
eval_sam_ref(string service, string ref, object context, mapping args)
{
   switch (ref) {
   case "list":
   case "list-all":
      return ({ ({
	 query_name(),
	 query_brief(),
	 query_description(),
	 query_announcement(),
	 query_web_port(),
	 query_root_url(),
	 query_realm_url()
      }) });
   case "id":
   case "name":
   case "gameid":
      return query_name();
   case "gamebrief":
      return query_brief();
   case "gamedesc":
      return query_description();
   case "port":
      return query_web_port();
   case "realmurl":
      return query_realm_url();
   case "bigmapurl":
      return query_bigmap_url();
   case "loginurl":
      return query_login_url();
   case "startroom":
      return query_start_room();
   case "home":
      return query_home();
   case "office":
      return query_office();
   case "jail":
      return query_jail();
   case "urmale":
      return query_male_body();
   case "urfemale":
      return query_female_body();
   case "chatmode":
      return FALSE;
   case "restricted":
      return FALSE;
   default:
      return nil;
   }
}

/*
 * Serve the Theatre:* property name-space for /base/obj/thing clones, for what
 * it's worth.
 */

mapping
query_properties(object who, string root, int derived)
{
   if (root != "theatre") {
      return nil;
   }
   if (!derived) {
      return ([ ]);
   }
   return ([
      "port":      query_web_port(),
      "realmurl":  query_realm_url(),
      "bigmapurl":  query_bigmap_url(),
      "loginurl":  query_login_url(),
      "startroom": query_start_room(),
      "home":      query_home(),
      "office":    query_office(),
      "jail":      query_jail(),
      "gameid":    query_name(),
      "gamebrief": query_brief(),
      "gamedesc":  query_description()
      ]);
}

mixed
query_property(object who, string root, string prop)
{
   if (root != "theatre") {
      return nil;
   }
   switch (prop) {
   case "port":      return query_web_port();
   case "bigmapurl":  return query_bigmap_url();
   case "loginurl":  return query_login_url();
   case "startroom": return query_start_room();
   case "home":      return query_home();
   case "office":    return query_office();
   case "jail":      return query_jail();
   case "gameid":    return query_name();
   case "gamebrief": return query_brief();
   case "gamedesc":  return query_description();
   default:          return nil;
   }
}

int
set_property(object who, string root, string prop, mixed value, mixed *retval)
{
   if (root != "theatre") {
      return FALSE;
   }
   switch (prop) {
   case "port":
   case "realmurl":
   case "bigmapurl":
   case "loginurl":
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

void
clear_all_properties(object who, string root)
{
   /* You're kidding, right? */
   return;
}
