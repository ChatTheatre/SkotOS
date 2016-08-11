/*
**	~Skotos/sys/helpd.c
**
**	Maintain a small database of help-node objects (being clones
**	of ~SkotOS/obj/help) indexed by keywords.
**
**********************************************************************
**	991101 Zell	Initial Revision
**
**	Copyright Skotos Tech Inc 1999
*/

# include <SkotOS.h>
# include <HTTP.h>
# include <SAM.h>
# include <mapargs.h>

inherit "/lib/womble";

private inherit "/lib/string";
private inherit "/lib/mapargs";
private inherit "/lib/url";

mapping obs;
mapping key_to_obs;
mapping ob_to_oldkeys;

SAM	description;

void womble_description() {
   description = womble(description);
}

string query_state_root() { return "Help:Help"; }

static
void create() {
   set_object_name("SkotOS:Help");

   obs = ([ ]);
   key_to_obs = ([ ]);
   ob_to_oldkeys = ([ ]);

   compile_object(HELP_NODE);
}

/* public API */
object *query_help_nodes() {
   return map_indices(obs);
}

object *match_keyword(string key) {
   mixed tmp;

   if (tmp = key_to_obs[key]) {
      tmp = map_indices(key_to_obs[key]);
      if (sizeof(tmp)) {
	 return tmp;
      }
   }
}

atomic
void update_help(object ob) {
   string *keys;
   int i;

   if (previous_program() == HELP_NODE) {
      obs[ob] = ob;

      if (keys = ob_to_oldkeys[ob]) {
	 for (i = 0; i < sizeof(keys); i ++) {
	    key_to_obs[keys[i]][ob] = nil;
	 }
      }

      keys = ob->query_keywords();

      for (i = 0; i < sizeof(keys); i ++) {
	 if (!key_to_obs[keys[i]]) {
	    key_to_obs[keys[i]] = ([ ob: TRUE ]);
	 } else {
	    key_to_obs[keys[i]][ob] = TRUE;
	 }
      }
      ob_to_oldkeys[ob] = keys;
   }
}

SAM query_description() {
   return description;
}

void set_description(SAM m) {
   description = m;
}

mixed query_property(string prop) {
   if (prop && lower_case(prop) == "help:nodes") {
      return query_help_nodes();
   }
}
