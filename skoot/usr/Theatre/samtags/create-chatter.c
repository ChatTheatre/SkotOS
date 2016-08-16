# include <base/living.h>
# include <HTTP.h>
# include <UserAPI.h>

private inherit "/lib/string";
private inherit "/base/lib/toolbox";
private inherit "/usr/SkotOS/lib/auth";

inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

atomic
int tag(mixed content, object context, mapping local, mapping args) {
   object udat, theatre;
   object body, clone, *bodies;
   string name, char;
   int gender, i;

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
   if (local["theatre"]) {
      theatre = find_object("Theatre:Theatres:" + local["theatre"]);
      SysLog("Success: " + dump_value(theatre));
   }
   if (!theatre) {
      string host;
      object *list;

      if (!context->query_user()->query_node()) {
	 error("internal error: no http port node");
      }
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
   if (!theatre) {
      error("cannot find a valid theatre");
   }
   if (theatre->query_restricted()) {
      if (!is_root(udat->query_name())) {
	 /*
	  * This is a restricted Theatre and you are not a SH/SP type person
	  * so you cannot create a chatter here.
	  *
	  * However we just decided that some guests might be allowed!
	  */
	 if (sizeof(theatre->query_guests() & ({ udat->query_name() })) == 0) {
	    error("Attempting to enter a restricted Theatre");
	 }
      }
   }
   SysLog("I -do- have a theatre node: " + dump_value(theatre));
   if (!(theatre <- "~/lib/theatre")) {
      error("internal error: theatre node " + name(theatre) +
      " (lpc " + ur_name(theatre) + ") does not inherit theatre library");
   }
   if (!local["gender"] || local["gender"] == "male") {
      gender = GENDER_MALE;
      bodies = theatre->query_male_bodies();
   } else {
      gender = GENDER_FEMALE;
      bodies = theatre->query_female_bodies();
   }
   if (!bodies) {
      error("internal error: no urbodies");
   }
   if (!sizeof(bodies)) {
      error("no bodies to choose from");
   }
   body = spawn_thing(bodies[random(sizeof(bodies))]);

   name = lower_case(context->query_user()->query_name());

   char = local["char"];
   if (char == nil) {
      char = name;
   }

   body->set_object_name("Chatters:" +
			 theatre->query_id() + ":" +
			 char[0 .. 1] + ":" +
			 char);
   body->set_property("theatre:id", theatre->query_id());

   /* this was previously in UserAPI, where it does NOT belong */
   body->set_property("SkotOS:CharName", char);
   body->set_property("SkotOS:Creator", name);

   body->add_alias("default", char);
   body->add_alias("default", capitalize(char) );

   body->set_gender(gender);

   /* save the user's choice no matter what happens to body's gender later */
   body->set_property("theatre:gender",
		      lower_case(local["gender"] ? local["gender"] : "male"));

   UDATD->add_name_for_body(body, char);
   UDATD->add_name_for_body(body, char + ":" + theatre->query_id(), TRUE);
   UDATD->add_body_to_roster(body, name);

   body->set_disconnect_room(theatre->query_start_room());
   body->set_property("skotos:creation_time", time());

   return TRUE;
}
