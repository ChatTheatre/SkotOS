/*
**	/usr/TextIF/cmds/desc.c
**
**	This file will hold the desc_* functions. I suppose it will be a 
**	nice migration of desc_* functions to the OutputDB.
**
**	Copyright Skotos Tech Inc 2000
*/

# include <base.h>
# include <SAM.h>
# include <XML.h>
# include <base/living.h>
# include <base/bulk.h>
# include <base/preps.h>
# include <base/act.h>
# include <TextIF.h>

private inherit "/lib/string";
private inherit "/lib/date";
private inherit "/lib/array";
private inherit "/lib/type";

private inherit "/base/lib/urcalls";

inherit "/usr/SAM/lib/sam";
inherit "/usr/TextIF/lib/misc";
inherit "/usr/TextIF/lib/tell";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";

# include "/usr/SAM/include/sam.h"
# include "/usr/SkotOS/include/describe.h"
# include "/usr/TextIF/include/tell.h"
# include "/usr/SkotOS/include/noun.h"

void talkback(object user, object body, NRef ref, string tag, SAM def);

void desc_runtime_error(object *users, object body, string err) {
   tell_to(users, "ERROR [" + short_time(time()) + "]: " +
	   (err ? err : "unknown error") + "\n", body);
}

/* examine the error object and display an appropriate error */
void desc_error(object *users, object body, object error_ob) {
   string *error_names;
   int i;
   mapping error_args;
   mixed sam;

   error_names = error_ob->query_error_indices();

   for (i = 0; i < sizeof(error_names); i++) {
      error_args = error_ob->query_related(error_names[i]);
      sam = ERRORDB->query_error(error_names[i]);
      if (!sam) sam = "Undescribed error [" + error_names[i] + "]";

      general_sam(users, body, ({ body }), sam, error_args +
		  ([ "actor": body ]));
   }
}

/* examine the output object and display an appropriate message */
void desc_output(object *users, object body, object output_ob) {  
   mapping output_args;
   object node;
   string *output_names, bot, bit;
   mixed m1, m2, m3;
   int i, j, names_size;

   output_names = output_ob->query_output_indices();
   names_size = sizeof(output_names);

   /* onward to the SAM */
   for (i = 0; i < names_size; i++) {
      object *dobobs, *third;
      mixed *dobs;
      string foo;

      output_args = output_ob->query_output_related(output_names[i]);

      /* very temporary backwards compat hack */
      if (sscanf(output_names[i], "Desc:%s", foo) ||
	  sscanf(output_names[i], "desc:%s", foo)) {
	 if (output_args["args"]) {
	    MAIN->describe(foo, users, body, output_args["args"]...);
	 } else {
	    MAIN->describe(foo, users, body);
	 }
	 return;
      }

      dobs = output_ob->query_dobs(output_names[i]);

      if (body->query_environment()) {
	 if (dobs) {
	    dobobs = dobs[..];
	    for (j = 0; j < sizeof(dobs); j ++) {
	       if (IsNRef(dobs[j])) {
		  dobobs[j] = NRefOb(dobs[j]);
	       } else if (typeof(dobs[j]) == T_OBJECT) {
		  dobobs[j] = dobs[j];
	       }
	    }
	    dobobs -= ({ nil });
	    third = body->query_environment()->query_inventory() - dobobs;

	 } else {
	    third = body->query_environment()->query_inventory();
	 }
      }


      if (!output_args["obs"]) {
	 output_args["obs"] = dobs;
      }
      if (!output_args["what"]) {
	 output_args["what"] = dobs;
      }

      
	 m1 = OUTPUTDB->query_output_msg_first(output_names[i]);
	 m2 = OUTPUTDB->query_output_msg_second(output_names[i]);
	 m3 = OUTPUTDB->query_output_msg_third(output_names[i]);
    

      /* first person output */

      if (m1) {
	 general_sam(users, body, ({ body }), m1, output_args);
	 third -= ({ body });
	 dobobs -= ({ body });
      }

      /* second person output */ 
      if (m2 && dobobs && sizeof(dobobs)) {
	 general_sam(nil, body, dobobs, m2, output_args);
      }

      /* third person output */
      if (m3 && third && sizeof(third) &&
	  !output_ob->query_private(output_names[i])) {
	 general_sam(users, body, third, m3, output_args);
      }
   }
}

void desc_properties(object *users, object body,
		     string first, string second, string third,
		     varargs mapping args) {
   string msg_first, msg_second, msg_third;
   object first_targ, second_targ, target;

   XDebug("XX - desc_properties()...");

   first_targ = body;
   second_targ = body;
   target = body;

   /* if args then override */
   if (args) {
      if (args["first"])
	 first_targ = args["first"];
      if (args["second"])
	 second_targ = args["second"];
      if (args["target"])
	 target = args["target"];
   }
   if (first) {
      msg_first = target->query_property(first);
   }
   if (second) {
      msg_second = target->query_property(second);
   }
   if (third) {
      msg_third = target->query_property(third);
   }

   if (msg_first)
      XDebug("XX first: " + msg_first);
   if (msg_second)
      XDebug("XX second: " + msg_second);
   if (msg_third)
      XDebug("XX third: " + msg_third);

   XDebug("XX body: " + dump_value(body));
   XDebug("XX target: " + dump_value(target));
   XDebug("XX first_targ: " + dump_value(first_targ));
   XDebug("XX second_targ: " + dump_value(second_targ));

   if (msg_first) {
      tell_to(users, msg_first, first_targ);
   }

   if (msg_second) {
      tell_to(users, msg_second, second_targ);
   }

   if (msg_third) {
      tell_all_but(users, msg_third, first_targ, second_targ);
   }
}

/* Does the convicted have any last words before execution? */
void desc_die(object *users, object body, object env) {
   SAM msg_first, msg_third;

   msg_first = ur_die_message_first(body);
   msg_third = ur_die_message_third(body);

   if (body->query_holder()) {
      general_sam(users, body->query_holder(), ({ body->query_holder() }),
		  msg_first);
      general_sam(users, body->query_holder(), nil,
		  msg_third, ([ "who" : body->query_holder() ]), TRUE);
   } else {
      general_sam(users, body->query_holder(), nil, msg_third, nil, TRUE);
   }
}
