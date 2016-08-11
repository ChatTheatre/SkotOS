# include <base/living.h>
# include <UserAPI.h>

private inherit "/lib/string";
private inherit "/base/lib/toolbox";
private inherit "/usr/SkotOS/lib/auth";

inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

atomic
int tag(mixed content, object context, mapping local, mapping args) {
   object theatre;
   object body, clone, *bodies;
   string uname, cname;
   int gender, i;

   if (!context) {
      error("internal error: no context");
   }
   uname = local["uname"];
   cname = local["cname"];
   if (!uname || !cname) {
      error("expecting uname and cname attributes");
   }

   if (!local["theatre"]) {
      error("expecting theatre attribute");
   }
   theatre = find_object("Theatre:Theatres:" + local["theatre"]);
   if (!theatre) {
      error("cannot find theatre");
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

   body->set_object_name("DetachedBodies:" +
			 theatre->query_id() + ":" +
			 cname[0 .. 1] + ":" +
			 cname);
   body->set_property("theatre:id", theatre->query_id());

   body->add_alias("default", cname);
   body->add_alias("default", capitalize(cname) );

   body->set_gender(gender);

   /* save the user's choice no matter what happens to body's gender later */
   body->set_property("theatre:gender",
		      lower_case(local["gender"] ? local["gender"] : "male"));

   UDATD->register_body(body, uname, cname);

   body->set_disconnect_room(theatre->query_start_room());
   body->set_property("skotos:creation_time", time());

   return TRUE;
}
