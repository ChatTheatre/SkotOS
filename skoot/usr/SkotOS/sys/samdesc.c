/*
**
**
**	Copyright (C) Skotos Tech 2001
*/

# include <base.h>
# include <SAM.h>

private inherit "/lib/array";

inherit "/usr/SAM/lib/sam";
inherit "/base/lib/urcalls";

static
void create() {
   BASE_INITD->set_runner("samdesc", this_object());
}

int run_signal(object ob, string signal, NRef target, mapping args,
	       object *third) {
   SAM desc;

   if (target) {
      desc = ur_description(NRefOb(target), NRefDetail(target),
			    "react-" + signal);
      if (!desc) {
	 desc = ur_description(NRefOb(target), NRefDetail(target),
			       "react:" + signal);
	 if (desc) {
	    ERROR("OLD FORM SIGNAL: " + dump_value(target) +
		  "/react:" + signal);
	 }
      }
      if (desc) {
	 object *souls;
	 string str;
	 int j;

	 str = unsam(desc, args, nil, NRefOb(target));

	 souls = ob->query_souls();
	 for (j = 0; j < sizeof(souls); j ++) {
	    souls[j]->paragraph(str);
	 }
	 return FALSE;
      }
   }
   return TRUE;
}


void show_desc(SAM desc, object this, object *who, mapping args) {
   object *souls;
   string str;
   int j, k;

   str = unsam(desc, args, nil, this);

   for (k = 0; k < sizeof(who); k ++) {
      souls = who[k]->query_souls();
      for (j = 0; j < sizeof(souls); j ++) {
	 souls[j]->paragraph(str);
      }
   }
}


private
int doobie(object ob, string signal, string prefix,
	   mapping react_map, mapping witness_map,
	   mapping args) {
   string *reactors;
   object *inv;
   mixed arr;
   SAM desc;
   int i, j, seen;

   if (sizeof(ob->query_souls())) {
      desc = ur_description(ob, NRefDetail(ob),
			    (prefix ? "act-" + prefix : "act") +
			    ":" + signal);
      if (desc) {
	 show_desc(desc, ob, ({ ob }), args);
	 seen = TRUE;
      }
   }
   if (ob->query_environment()) {
      desc = ur_description(ob, NRefDetail(ob),
			    (prefix ? "act3-" + prefix : "act3") +
			    ":" + signal);
      if (desc) {
	 inv = ob->query_environment()->query_inventory() - ({ ob });
	 if (sizeof(inv) > 0) {
	    show_desc(desc, ob, inv, args);
	    seen = TRUE;
	 }
      }
   }
   desc = ur_description(ob, NRefDetail(ob),
			 (prefix ? "actin-" + prefix : "actin") +
			 ":" + signal);
   if (desc) {
      inv = ob->query_inventory();
      if (sizeof(inv) > 0) {
	 show_desc(desc, ob, inv, args);
	 seen = TRUE;
      }
   }
   if (seen) {
      return FALSE;
   }

   reactors = map_indices(react_map);
   for (i = 0; i < sizeof(reactors); i ++) {
      string reactor, name;
      mixed val;

      reactor = reactors[i];
      val = react_map[reactor];

      if (typeof(val) == T_STRING) {
	 /* used for e.g. nil -> dob aliasing */
	 reactor = val;
	 val = react_map[reactor];
      }

      switch(typeof(val)) {
      case T_OBJECT:
	 arr = ({ val });
	 break;
      case T_ARRAY:
	 arr = val;
	 break;
      case T_INT:
	 arr = args[reactor];
	 if (typeof(arr) == T_ARRAY) {
	    break;
	 }
	 /* else fall through to ignore */
      default:
	 /* shouldn't happen */
	 continue;
      }

      name = reactors[i] ? signal + "-" + reactors[i] : signal;

      for (j = 0; j < sizeof(arr); j ++) {
	 string str;
	 int j, k;

	 seen = FALSE;
	 args["target"] = arr[j];
	 args["this"] = NRefOb(arr[j]);

	 if (sizeof(ob->query_souls())) {
	    desc = ur_description(NRefOb(arr[j]), NRefDetail(arr[j]),
				  (prefix ? "react-" + prefix : "react") +
				  ":" + name);
	    if (desc) {
	       show_desc(desc, NRefOb(arr[j]), ({ ob }), args);
	       seen = TRUE;
	    }
	 }
	 if (ob->query_environment()) {
	    desc = ur_description(NRefOb(arr[j]), NRefDetail(arr[j]),
				  (prefix ? "react3-" + prefix : "react3") +
				  ":" + name);
	    if (desc) {
	       inv = ob->query_environment()->query_inventory() - ({ ob });
	       if (sizeof(inv) > 0) {
		  show_desc(desc, NRefOb(arr[j]), inv, args);
		  seen = TRUE;
	       }
	    }
	 }
	 desc = ur_description(NRefOb(arr[j]), NRefDetail(arr[j]),
			       (prefix ? "reactin-" + prefix : "reactin") +
			       ":" + name);
	 if (desc) {
	    inv = ob->query_inventory();
	    if (sizeof(inv) > 0) {
	       show_desc(desc, NRefOb(arr[j]), inv, args);
	       seen = TRUE;
	    }
	 }
	 /* legacy */
	 desc = ur_description(NRefOb(arr[j]), NRefDetail(arr[j]),
			       (prefix ? "show-" + prefix : "show") +
			       ":" + name);
	 if (desc) {
	    inv = ob->query_inventory();
	    if (sizeof(inv) > 0) {
	       show_desc(desc, NRefOb(arr[j]), inv, args);
	       seen = TRUE;
	    }
	 }
	 if (seen) {
	    return FALSE;
	 }
      }
   }
   return TRUE;
}

int pre_signal(object ob, string signal, mapping react_map,
	       mapping witness_map, mapping args) {
   return doobie(ob, signal, "pre", react_map, witness_map, args);
}

int new_signal(object ob, string signal, mapping react_map,
	       mapping witness_map, mapping args) {
   return doobie(ob, signal, nil, react_map, witness_map, args);
}

int desc_signal(object ob, string signal, mapping react_map,
	       mapping witness_map, mapping args) {
   return doobie(ob, signal, "desc", react_map, witness_map, args);
}

void post_signal(object ob, string signal, mapping react_map,
	       mapping witness_map, mapping args) {
   doobie(ob, signal, "post", react_map, witness_map, args);
}

