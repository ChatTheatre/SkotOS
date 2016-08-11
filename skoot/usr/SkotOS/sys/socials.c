/*
**	/usr/SkotOS/sys/socials.c
**
**	Maintain the database of socials.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <mapargs.h>
# include <SkotOS.h>
# include <HTTP.h>
# include <SAM.h>

private inherit "/lib/mapping";
private inherit "/lib/array";
private inherit "/lib/string";
private inherit "/lib/mapargs";
private inherit "/lib/url";

inherit "/usr/XML/lib/xmlparse";

private int updating;

# define Updated() { if (updating) { remove_call_out(updating); } updating = call_out("signal_updated", 1); }

/* verbs */
mapping verb_obs;

/* adverbs */
mapping adverbs;
mapping node_to_advarr;

private void clear_socials();

string *query_verbs();
string *query_all_adverbs();
SAM query_default_definition(string form);

string query_state_root() { return "Socials:Socials"; }

/* creator */

static void signal_updated() { event("updated"); }

static
void create() {
   set_object_name("SkotOS:Socials:Socials");

   compile_object(VERB);
   compile_object(ADVCAT);

   clear_socials();

   add_event("updated");
   subscribe_event(this_object(), "updated");
}

/* allow people to listen to our signal */
int allow_subscribe(object ob, string ev) {
   return TRUE;
}

private
void clear_socials() {
   verb_obs = ([ ]);
   adverbs = ([ ]);
   node_to_advarr = ([ ]);
}


/* public API: verbs */


/* basically just return ({ "a", "b", ... }) */

string *query_letters() {
   string *verbs;
   mapping out;
   int i;

   out = ([ ]);
   verbs = map_indices(verb_obs);

   for (i = 0; i < sizeof(verbs); i ++) {
      if (strlen(verbs[i])) {
	 out[verbs[i][0 .. 0]] = TRUE;
      }
   }
   return map_indices(out);
}

string *query_verbs() {
   verb_obs[nil] = nil;
   return map_indices(verb_obs);
}

string *query_prefixed_verbs(string prefix) {
   return map_indices(prefixed_map(verb_obs, prefix));
}

object *query_verb_obs(string imp) {
   if (verb_obs[imp]) {
      return verb_obs[imp] -= ({ nil });
   }
   return nil;
}

mapping query_verb_obs_mapping() { return verb_obs[..]; }


/* public API: adverbs */

void remove_advcat(object obj) {
   int i, sz;
   string *str;
   object **list;

   sz = map_sizeof(adverbs);
   str = map_indices(adverbs);
   list = map_values(adverbs);
   for (i = 0; i < sz; i++) {
      list[i] -= ({ obj });
      if (sizeof(list[i]) > 0) {
	 adverbs[str[i]] = list[i];
      } else {
	 adverbs[str[i]] = nil;
      }
   }
}

string *query_all_adverbs() {
   return map_indices(adverbs);
}

string *expand_adverb_prefix(string prefix) {
   return map_indices(prefixed_map(adverbs, prefix));
}

int query_adverb(string adv) {
   return !!adverbs[adv];
}

/* internals */

void deregister_verb(string imp, object ob) {
   if (verb_obs[imp]) {
      verb_obs[imp] -= ({ ob });
      if (!sizeof(verb_obs[imp])) {
	 verb_obs[imp] = nil;
      }
   }
   Updated();
}

void register_verb(string imp, object ob) {
   if (verb_obs[imp]) {
      verb_obs[imp] |= ({ ob });
   } else {
      verb_obs[imp]  = ({ ob });
   }
   Updated();
}

void update_verb(object ob) {
}


atomic
void update_advcat(object ob) {
   string *ix;
   int i;

   if (!node_to_advarr) {
      /* this is a new data structure -- Zell 020820 */
      node_to_advarr = ([ ]);
   }

   /* Workaround to clear up 'lost' adverbs that might otherwise never disappear -EH */
   ix = map_indices(adverbs) - ob->query_adverbs();
   for (i = 0; i < sizeof(ix); i++) {
      adverbs[ix[i]] -= ({ ob, nil });
      if (!sizeof(adverbs[ix[i]])) {
	 adverbs[ix[i]] = nil;
      }
   }

   ix = node_to_advarr[ob];
   if (ix) {
      ix -= ob->query_adverbs();
      for (i = 0; i < sizeof(ix); i ++) {
	 if (adverbs[ix[i]]) {
	    adverbs[ix[i]] -= ({ ob });
	    if (!sizeof(adverbs[ix[i]])) {
	       adverbs[ix[i]] = nil;
	    }
	 }
      }
   }

   ix = ob->query_adverbs();
   if (node_to_advarr[ob]) {
      ix -= node_to_advarr[ob];
   }
   for (i = 0; i < sizeof(ix); i ++) {
      if (!adverbs[ix[i]]) {
	 adverbs[ix[i]] = ({ });
      }
      adverbs[ix[i]] |= ({ ob });
   }
   node_to_advarr[ob] = ob->query_adverbs();
   Updated();
}

void evt_updated(object source) {
   updating = FALSE;
}

void patch() {
   adverbs = ([ ]);
   node_to_advarr = ([ ]);
}

mixed query_property(string prop) {
   string str;

   if (prop) {
      if (sscanf(lower_case(prop), "verbs:%s", prop)) {
	 if (sscanf(prop, "objects:%s", str)) {
	    return verb_obs[str];
	 }
	 if (prop == "verb-to-objects-mapping") {
	    return copy(verb_obs);
	 }
	 error("unknown verbs: property");
      }
      if (sscanf(lower_case(prop), "adverbs:%s", prop)) {
	 if (sscanf(prop, "objects:%s", str)) {
	    return adverbs[str];
	 }
	 if (prop == "adverb-to-objects-mapping") {
	    return copy(adverbs);
	 }
	 error("unknown adverbs: property");
      }
   }
   error("only verbs: and adverbs: property spaces served here");
}

/* debug */

mixed query_node_to_advarr() {
  return node_to_advarr;
}
