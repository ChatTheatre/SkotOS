/*
**	/usr/SkotOS/obj/startstory.c
**
**	One start-story node. This is unrelated to ~SkotOS/sys/startstory.c
**
**	Copyright Skotos Tech Inc 1999
*/

# define SSNUGGET	"/usr/SkotOS/data/ssnugget"

private inherit "/lib/string";

private mapping nuggetnodes;

static
void create() {
   nuggetnodes = ([ ]);
}

void clear_nuggets() {
   nuggetnodes = ([ ]);
}


void configure_ss_nugget(string nugget, string description) {
   nugget = lower_case(nugget);

   if (!nuggetnodes[nugget]) {
      nuggetnodes[nugget] = new_object(SSNUGGET, nugget);
   }
   nuggetnodes[nugget]->set_description(description);
}

void set_ss_choice(string nugget, string choice_id, string choice_name) {
   nugget = lower_case(nugget);

   if (!nuggetnodes[nugget]) {
      error("set_choice to non-existant nugget " + nugget);
   }
   nuggetnodes[nugget]->set_choice(choice_id, choice_name);
}

void clear_ss_choices(string nugget) {
   nugget = lower_case(nugget);
   if (nuggetnodes[nugget]) {
      nuggetnodes[nugget]->clear_choices();
   }
}
   

string *query_ss_nuggets() {
   return map_indices(nuggetnodes);
}

string query_ss_description(string id) {
   return nuggetnodes[lower_case(id)]->query_description();
}

string *query_ss_choices(string id) {
   return nuggetnodes[lower_case(id)]->query_choices();
}

string query_ss_choice_name(string id, string choice) {
   return nuggetnodes[lower_case(id)]->query_choice_name(choice);
}

mapping *query_nugget_mappings(varargs object chatter) {
   mapping *nuggets;
   string *ix;
   int i;

   nuggets = allocate(map_sizeof(nuggetnodes));
   ix = map_indices(nuggetnodes);

   for (i = 0; i < sizeof(ix); i ++) {
      nuggets[i] = nuggetnodes[ix[i]]->query_nugget_mapping();
      if (chatter) {
	 nuggets[i]["currentvalue"] =
	    chatter->query_property(nuggetnodes[ix[i]]->query_property());
      }
   }
   return nuggets;
}

