/*
**	/usr/SkotOS/lib/describe.c
**
**	Perform most of the description tasks related to the physical
**	objects in /base.
**
**	Copyright Skotos Tech Inc 2000
*/

# include <status.h>
# include <type.h>
# include <SkotOS.h>

# include <base/thing.h>
# include <base/light.h>

# include <SAM.h>

inherit "/usr/SAM/lib/sam";

private inherit "/lib/string";
private inherit "/lib/type";
private inherit "/lib/array";
private inherit "/lib/mapping";

private inherit "/base/lib/urcalls";

# define NOCLUSTER FALSE

SAM get_lit_description(object ob, string id, object looker,
			string types...) {
   object ur;
   string tmp;
   SAM desc;
   int i;

   switch(ob->query_light_category()) {
   case LIGHT_DARK:
      tmp = "dark";
      break;
   case LIGHT_DIM:
      tmp = "dim";
      break;
   case LIGHT_BRIGHT:
   default:
      tmp = "bright";
      break;
   case LIGHT_SUPERBRIGHT:
      tmp = "superbright";
      break;
   }
   ur = ob;
   while (ur && !desc) {
      desc = ur->query_description(id, types[i] + "-" + tmp);
      if (!desc && tmp == "superbright") {
	 desc = ur->query_description(id, types[i] + "-bright");
      }
      if (!desc) {
	 desc = ur->query_description(id, types[i] + "-full");
      }
      if (!desc) {
	 desc = ur->query_description(id, types[i]);
      }
      ur = ur->query_ur_object();
   }
   return desc;
}


/*
**	This is largely stolen from ...
**	   http://www.csse.monash.edu.au/~damian/papers/HTML/Plurals.html
**	... which is a generally nice paper for these matters.
*/

static
string pluralize_word(string noun) {
   int l;

   l = strlen(noun);

   if (l > 2) {
      switch(noun[l-1]) {
      case 'e':
	 if (noun[l-2] == 'f') {
	    /* life -> lives */
	    return noun[.. l-3] + "ves";
	 }
	 break;
      case 'f':
	 /* this needs more work */
	 if (noun[l-2] == 'l') {
	    /* wolf -> wolves */
	    return noun[.. l-2] + "ves";
	 }
      case 'h':
	 if (noun[l-2] == 'c' || noun[l-2] == 's') {
	    /* church -> churches, wish -> wishes, but not zyph->zyphs */
	    return noun + "es";
	 }
	 break;
      case 'm':
	 if (noun[l-2] == 'u') {
	    /* bacterium -> bacteria */
	    return noun[.. l-3] + "a";
	 }
	 break;
      case 'n':
	 if (noun[l-2] == 'a') {
	    /* woman -> women */
	    return noun[.. l-3] + "en";
	 }
	 break;
      case 's':
	 switch(noun[l-2]) {
	 case 'i':
	    /* basis -> bases */
	    return noun[.. l-3] + "es";
	 case 'u': case 's':
	    /* radius -> radiuses, class -> classes */
	    return noun + "es";
	 }
	 /* series -> series */
	 return noun;
      case 'x':
	 if (noun[l-2] == 'e' || noun[l-2] == 'i') {
	    /* index -> indices, matrix -> matrices */
	    return noun[.. l-3] + "ices";
	 }
	 /* box -> boxes */
	 return noun + "es";
      case 'y':
	 switch(noun[l-2]) {
	 case 'a': case 'e': case 'i': case 'o': case 'u':
	    /* boy -> boys */
	    return noun + "s";
	 }
	 /* ferry -> ferries */
	 return noun[.. l-2] + "ies";
      }
   }
   return noun + "s";
}

static
string pluralize_phrase(string str) {
   string *words;
   int i;

   words = explode(str, " ");
   if (words[0] == "a" || words[0] == "an") {
      /* plural phrases don't need indefinite article */
      words = words[1 ..];
   }
   for (i = 1; i < sizeof(words); i ++) {
      switch(words[i]) {
      case "above":
      case "about":
      case "against":
      case "around":
      case "at":
      case "atop":
      case "before":
      case "behind":
      case "beneath":
      case "below":
      case "beside":
      case "by":
      case "concerning":
      case "down":
      case "for":
      case "from":
      case "in":
      case "inside":
      case "into":
      case "near":
      case "nearby":
      case "of":
      case "on":
      case "onto":
      case "over":
      case "regarding":
      case "to":
      case "toward":
      case "towards":
      case "under":
      case "upon":
      case "up":
      case "using":
      case "with":
      case "within":
	 words[i-1] = pluralize_word(words[i-1]);
	 return implode(words, " ");
      }
   }
   return pluralize_word(str);
}


# define build_sam_args(this, looker, actor, target) \
  ([ "this": this, "looker": looker, "actor": actor, "target": target ])

static string describe_many(NRef *nrefs, varargs mixed doer,
			    object looker, int style);


private
mixed *describe_by_unit(float amount, mixed *units) {
   int i;

   while (i < sizeof(units) && amount > units[i]) {
      i += 4;
   }
   if (i > 0) {
      i -= 4;
   }

   if (i < sizeof(units) - 4 &&
       amount/units[i] >= 2.0 && amount/units[i+4] > 0.5) {
      /* about half a bar, almost a bar */
      i += 4;
   }
   amount /= units[i];

   /*
   **	about half a foo	0.5 - 0.7
   **	almost a foo		0.7 - 1.0
   **	about a foo		1.0 - 1.3
   **	more than a foo		1.3 - 1.5
   **	about a foo and a half	1.5 - 2.0
   **	about N foo		  N - N+1
   */

   if (amount < 0.5) {
      return ({ "less than half " + units[i+1] + " " + units[i+2], FALSE });
   }
   if (amount < 0.7) {
      return ({ "half " + units[i+1] + " " + units[i+2], FALSE });
   }
   if (amount < 1.0) {
      return ({ "almost " + units[i+1] + " " + units[i+2], FALSE });
   }
   if (amount < 1.3) {
      return ({ units[i+1] + " " + units[i+2], FALSE });
   }
   if (amount < 1.5) {
      return ({ "more than " + units[i+1] + " " + units[i+2], FALSE });
   }
   if (amount < 2.0) {
      return ({ units[i+1] + " " + units[i+2] + " and a half", FALSE });
   }
   return ({ desc_cardinal((int) amount) + " " + units[i+2], TRUE });
}

static
mixed *describe_weight(float kg) {
   return describe_by_unit(kg * 1e3, ({
      28.0,	"an", "ounce", "ounces",
	 454.0, "a", "pound", "pounds",
	 907000.0, "a", "ton", "tons"
	 }));
}

static
mixed *describe_volume(float m3) {
    return describe_by_unit(m3 * 1e6, ({
	 1.0, "a", "dash", "dashes",
	 5.0, "a", "teaspoon", "teaspoons",
	15.0, "a", "tablespoon", "tablespoons",
       225.0, "a", "cup", "cups",
       473.0, "a", "pint", "pints",
       950.0, "a", "quart", "quarts",
      3785.0, "a", "gallon", "gallons",
	    /* 7570.0, "a", "peck", "pecks", */
	    /* 30280.0, "a", "bushel", "bushels", */
    119240.0, /* 31.5 gallons */ "a", "barrel", "barrels",
    }));
}

static
string determine_prep(object *what) {
   string prime_id, *snames;
   int plural, i;

   if (sizeof(what) > 1) {
      return "are";
   }
   /* personally I think this is a cheesy hack to determine 
   ** if an object is considered a plural object.
   ** such as: boots, socks, and pants. -- wes
   */

   prime_id = ur_prime_id(what[0], "outside");

   /* New approach. */
   if (ur_plural(what[0], prime_id)) {
      return "are";
   }

   /* Leaving original heuristic in place for now: */

   /* For lack of something better.  --Erwin. */
   if (typeof(what[0]->query_property("trait:isare")) == T_STRING) {
      return what[0]->query_property("trait:isare");
   }

   snames = ur_snames(what[0], prime_id);

   if (snames) {
      /* basically all snames and pnames need to be the same */
      for (i = 0; i < sizeof(snames); i++) {
	 if (snames[i] == ur_pname_of_sname(what[0], snames[i]))
	    plural = TRUE;
      }
   }
   return plural ? "are" : "is";
}

/* I don't remember why this isn't just describe_one() + bits? -- Zell */

string possessive_description(NRef holder, object looker, object actor,
			      varargs int style) {
   string tmp;

   if (actor && IsPrime(actor) && holder == NRefOb(actor)) {
      return holder->query_possessive();
   }


   tmp = strip(unsam(get_lit_description(holder, NRefDetail(holder),
					 looker, "brief"),
		     build_sam_args(holder, looker, looker, holder) +
		     ([ "detail": NRefDetail(holder) ]),
		     nil, holder));

   if (!tmp || !strlen(tmp)) {
      return nil;
   }

   /*
    * Optionally adjust by prepending a/an/the to the 'brief' of the
    * holder.
    */
   if (!ur_def(holder, NRefDetail(holder))) {
      if (style & STYLE_DEFINITE) {
	 tmp = "the " + tmp;
      } else {
	 switch (tmp[0]) {
	 case 'a': case 'A':
	 case 'e': case 'E':
	 case 'i': case 'I':
	 case 'o': case 'O':
	 case 'u': case 'U':
	    tmp = "an " + tmp;
	    break;
	 default:
	    tmp = "a " + tmp;
	    break;
	 }
      }
   }
   switch(tmp[strlen(tmp)-1]) {
   case 's':
   case 'z':
   case 'x':
      return tmp + "'";
   default:
      return tmp + "'s";
   }
}

static
string describe_one(NRef ref, varargs mixed actor, object looker, int style,
		    int cardinal) {
   mapping args;
   object ob, holder, actor_ob;
   string id, desc, art, dtype;
   mixed worn_desc;
   int ticks;
   int ref_prime;

   if (!ref) {
      return "";
   }

   ticks = status()[ST_TICKS];

   ob = NRefOb(ref);
   id = NRefDetail(ref);
   ref_prime = IsPrime(ref);

   args = build_sam_args(ob, looker, looker, ref);
   args["detail"] = id;

   if (looker && !(style & STYLE_IGNORE_VISIBILITY) &&
       !looker->can_identify(ref)) {
      if (style & STYLE_NO_SHOW_INVIS) {
	 return "";
      }
      if (worn_desc = ur_description(ob, id, "brief-unseen")) {
	 return strip(unsam(worn_desc, args, nil, ob));
      }
      if (ref_prime && ur_volition(ob)) {
	 return "someone unseen";
      }
      return "something unseen";
   }

   if (ref_prime && ob == looker && !(style & STYLE_WEAK_LOOKER)) {
      /* looker is eyeing himself */
      if (NRefsEqual(ref, actor)) {
	 return "yourself";
      }
      return "you";
   }
   if (NRefsEqual(ref, actor)) {
      return (style & STYLE_PRONOUN) ?
	 ob->query_objective() + "self" :
	 ob->query_objective();
   }

   /* if the object is worn check for a 'worn' desc and replace desc */
   if (ob->query_worn_by()) {
      if (worn_desc = ur_description(ob, id, "worn")) {
	 desc = strip(unsam(worn_desc, args, nil, ob));
      }
   }

   if (ref_prime) {
      holder = ob->query_holder();

      if (ur_combinable(ob)) {
	 float amount;
	 mixed *ret;

	 amount = ref->query_amount();

	 if (ur_discrete(ob)) {
	    if ((int) amount > 1) {
	       desc = strip(unsam(get_lit_description(ob, id, looker, "pbrief"), nil, ob));
	       if (!desc) {
		  return "[missing pbrief]";
	       }
	       desc = ob->query_combinable_noun() + " of " +
		  desc_cardinal((int) amount) + " " + desc;
	       cardinal = TRUE;
	    }
	 } else {
	    if (ur_by_weight(ob)) {
	       ret = describe_weight(amount);
	    } else {
	       ret = describe_volume(amount);
	    }
	    desc = strip(unsam(get_lit_description(ob, id, looker, "brief"), args, nil, ob));
	    if (!desc) {
	       return "[missing brief]";
	    }
	    desc = ret[0] + " of " + desc;
	    cardinal = TRUE;
	    if (!ret[1]) {
	       /* half a teaspoon of meat */
	       style |= STYLE_NONPOSS;
	    }
	 }
      }
   } else if (ur_volition(ob)) {
      holder = ob;
   }

   if (!desc) {
      if (cardinal) {
	 mixed tmp;

	 if (tmp = get_lit_description(ob, id, looker, "pbrief")) {
	    desc = desc_cardinal(cardinal) + " " + strip(unsam(tmp, args, nil, ob));
	 }
      }
      if (!desc) {
	 desc = strip(unsam(get_lit_description(ob, id, looker, "brief"), args, nil, ob));
	 if (cardinal) {
	    desc = desc_cardinal(cardinal) + " " + pluralize_phrase(desc);
	 }
      }
      if (!strlen(desc)) {
	 return "";
      }
   }

   if (ur_def(ob, id)) {
      /* do not prepend any sort of article */
      return desc;
   }

   if (holder && !(style & STYLE_NONPOSS)) {
      if ((looker && looker == holder) || (style & STYLE_FIRST_PERSON)) {
	 return "your " + desc;
      }
      return possessive_description(holder, looker, actor, style) + " " + desc;
   }
   if (cardinal) {
       /* a cardinal has been prepended; another article would be Bad */
      return desc;
   }
   if (style & STYLE_DEFINITE) {
      return "the " + desc;
   }
   switch(desc[0]) {
   case 'a': case 'A':
   case 'e': case 'E':
   case 'i': case 'I':
   case 'o': case 'O':
   case 'u': case 'U':
      art = "an"; break;
   default:
      art = "a"; break;
   }
   return art + " " + desc;
}

static
string *cluster_briefs(NRef *nrefs, object actor, object looker, int style) {
   /*
   **	There's no large enough subset of objects sharing a
   **	name. For moderately sized inventories, this branch
   **	is more likely to get used than real clustering one
   **	below. This is where we require plural briefs.
   */

   mapping pmap, cmap;
   string *res, *descs;
   int i, sz;

   pmap = ([ ]);
   cmap = ([ ]);

   sz = sizeof(nrefs);
   descs = allocate(sz);
   
   for (i = 0; i < sz; i ++) {
      string desc;
      NRef   nref;

      nref = nrefs[i];
      desc = describe_one(nref, actor, looker, style);

      if (!desc || !strlen(desc)) {
	 /* invisible: stop here */
	 continue;
      }

      descs[i] = desc;

      if (pmap[desc] == nil || pmap[desc] < 0) {
	 mixed p;

	 p = ur_description(NRefOb(nref), NRefDetail(nref), "pbrief");
	 if (p) {
	    pmap[desc] = (i+1);
	 } else {
	    pmap[desc] = -(i+1);
	 }
      }
      if (cmap[desc]) {
	 cmap[desc] ++;
      } else {
	 cmap[desc] = 1;
      }
   }
   descs -= ({ nil });
   sz = sizeof(descs);
   for (i = 0; i < sz; i ++) {
      string desc;

      desc = descs[i];

      if (pmap[desc] < 0) {
	 pmap[desc] = -pmap[desc];
      }

      if (pmap[desc]) {
	 if (cmap[desc] > 1) {
	    /* we've not dealt with this description yet */
	    descs[i] = describe_one(nrefs[pmap[desc]-1], actor, looker, style,
				    cmap[desc]);
	    cmap[desc] = 0;
	    continue;
	 }
	 if (cmap[desc] == 0) {
	    /* we've already handled this one */
	    descs[i]= nil;
	    continue;
	 }
      }
      /* else we leave it alone */
   }
   return descs - ({ nil });
}

static
string describe_many(NRef *nrefs, varargs mixed actor,
		     object looker, int style) {
   mapping name_to_nrefs, name_count, useless, hstrings, pnames;
   string name, *names, *res, top_name, hstring;
   mixed *arr;
   NRef top_ref, looker_ref;
   int top_count;
   int i, j, sz_i;
   int size;

   nrefs = nrefs[..];
   size = sizeof(nrefs);

   /*
   **	This is a proof of concept implementation of about half of what
   **	we eventually want the clustering system to do. It's O(N^2) which
   **	is probably unacceptable in the long run, but tolerable for now.
   */

   if (size == 1) {
       return describe_one(nrefs[0], actor, looker, style);
   }

   res = ({ });
   useless = ([ ]);

   if (NOCLUSTER || (style & STYLE_NOCLUSTER)) {
       res += cluster_briefs(nrefs, actor, looker, style);
   } else {
       NRef *excluded;
       int spotted, spotted_exclude;

       sz_i = sizeof(nrefs);

       spotted_exclude = FALSE;
       excluded = allocate(sz_i);
       hstrings = ([ ]);
       pnames = ([ ]);

       for (i = 0; i < sz_i; i++) {
	   object holder;
	   NRef   nref;
	   object nref_ob;
	   int sz_j;
	   mapping seen;

	   nref    = nrefs[i];
	   nref_ob = NRefOb(nref);
	   if (IsPrime(nref) && ur_volition(nref_ob)) {
	       excluded[i] = nref;
	       nrefs[i] = nil;
	       spotted_exclude = TRUE;
	       continue;
	   }
	   if (holder = nref_ob->query_holder()) {
	       if (holder == looker) {
		   hstring = "your";
	       } else {
		   hstring = hstrings[holder];
		   if (!hstring) {
		       hstring = possessive_description(holder, looker, actor);
		       hstrings[holder] = hstring;
		   }
	       }
	   } else {
	       hstring = nil;
	   }
	   names = ur_pnames(nref_ob, NRefDetail(nref));
	   sz_j = sizeof(names);
	   spotted = FALSE;
	   seen = ([ ]);
	   if (hstring) {
	       for (j = 0; j < sz_j; j++) {
		   string name;

		   name = names[j];
		   if (!name || !strlen(name)) {
		     spotted = TRUE;
		     continue;
		   }
		   if (seen[name]) {
		       names[j] = nil;
		       spotted = TRUE;
		   } else {
		       seen[name] = TRUE;
		       names[j] = hstring + "/" + name;
		   }
	       }
	   } else {
	       for (j = 0; j < sz_j; j++) {
		   string name;

		   name = names[j];
		   if (!name || !strlen(name)) {
		       spotted = TRUE;
		       continue;
		   }
		   if (seen[name]) {
		       names[j] = nil;
		       spotted = TRUE;
		   } else {
		       seen[name] = TRUE;
		   }
	       }
	   }
	   pnames[nref] = spotted ? names - ({ nil }) : names;
       }
       if (spotted_exclude) {
	   nrefs -= ({ nil });
	   excluded -= ({ nil });
       } else {
	   excluded = ({ });
       }

       while (sz_i = sizeof(nrefs)) {
	   name_to_nrefs = ([ ]);
	   top_name  = nil;
	   top_count = -1;

	   for (i = 0; i < sz_i; i ++) {
	      object holder;
	      NRef   nref;
	      int    sz_j;

	      nref = nrefs[i];
	      names = pnames[nref];
	      sz_j = sizeof(names);
	      for (j = 0; j < sz_j; j ++) {
		 name = names[j];

		 if (useless[name]) {
		    continue;
		 }

		 arr = name_to_nrefs[name]; 
		 if (!arr) {
		    arr = ({ });
		 }
		 arr += ({ nref });
		 if (sizeof(arr) > top_count) {
		    top_count = sizeof(arr);
		    top_name  = name;
		    top_ref   = nref;
		 }
		 name_to_nrefs[name] = arr;
	      }
	   }
	   if (top_count < 7) {
	      res += cluster_briefs(nrefs, actor, looker, style);
	      nrefs = ({ });
	   } else if (top_count < sz_i || !(style & STYLE_REDUCE_CHUNK)) {
	      if (top_count == 1) {
		 res += ({ describe_one(top_ref, actor, looker, style) });
		 nrefs -= ({ top_ref });
	      } else {
		 if (sscanf(top_name, "%s/%s", hstring, name) == 2) {
		    res += ({
		       hstring + " " + desc_cardinal(top_count) + " " + name
		    });
		 } else {
		    res += ({ desc_cardinal(top_count) + " " + top_name });
		 }
		 nrefs -= name_to_nrefs[top_name];
	      }
	   } else {
	      useless[top_name] = TRUE;
	   }
       }
       if (sizeof(excluded) > 0) {
	   res += cluster_briefs(excluded, actor, looker, style);
       }
   }
   res -= ({ "" });

   if (sizeof(res) == 0) {
      return "";
   }
   if (sizeof(res) == 1) {
      return res[0];
   }
   return implode(res[.. sizeof(res)-2], ", ") + " and " + res[sizeof(res)-1];
}

mixed **describe_multi(NRef *nrefs, object actor, varargs int style)
{
   mapping name_to_nrefs, name_count, useless, seen, hstrings;
   string name, *names, *res_1st, *res_2nd, top_name, hstring;
   mixed *arr, *results;
   object **res_nrefs;
   object *lookers;

   NRef top_ref, looker_ref;
   int top_count;
   int i, sz_i, j;
   int size;

   size = sizeof(nrefs);

   if (size == 1) {
       NRef   nref;
       object looker;

       nref = nrefs[0];
       looker = NRefOb(nref);
       return ({ ({ looker, describe_one(nref, actor, looker, style) }) });
   }

   res_1st   = ({ });
   res_2nd   = ({ });
   res_nrefs = ({ });

   useless = ([ ]);
   nrefs = nrefs - ({ nil }); /* copy */

   sz_i = sizeof(nrefs);

   seen = ([ ]);

   for (i = 0; i < sz_i; i ++) {
      seen[NRefOb(nrefs[i])] = TRUE;
   }
   lookers = map_indices(seen);

   hstrings = ([ ]);

   if (NOCLUSTER || (style & STYLE_NOCLUSTER)) {
      for (i = 0; i < sizeof(nrefs); i ++) {
	  res_nrefs += ({ ({ NRefOb(nrefs[i]) }) });
	  res_1st   += ({ describe_one(nrefs[i], actor, NRefOb(nrefs[i]), style) });
      }
      res_2nd += cluster_briefs(nrefs, actor, nil, style);
   } else while (sizeof(nrefs)) {
      name_to_nrefs = ([ ]);
      top_name  = nil;
      top_count = -1;

      for (i = 0; i < sizeof(nrefs); i ++) {
	 NRef nref;
	 object nref_ob;

	 nref = nrefs[i];
	 nref_ob = NRefOb(nref);
	 if (IsPrime(nref) && ur_volition(nref_ob)) {
	    continue;
	 }

	 hstring = nil;
	 if (TRUE || (this_user() && this_user()->query_name() == "zell")) {
	    object holder;

	    if (holder = nref_ob->query_holder()) {
	       if (!hstrings[holder]) {
		  hstrings[holder] = possessive_description(holder, actor,
							    lookers[0]);
	       }
	       hstring = hstrings[holder];
	    }
	 }

	 names = ur_snames(nref_ob, NRefDetail(nref));
	 if (!names) {
	    continue;
	 }
	 seen = ([ ]);
	 for (j = 0; j < sizeof(names); j ++) {
	    name = ur_pname_of_sname(nref_ob, names[j]);

	    if (!name || !strlen(name)) {
	       continue;
	    }
	    if (hstring) {
	       name = hstring + "/" + name;
	    }
	    if (seen[name] || useless[name]) {
	       continue;
	    }

	    seen[name] = TRUE;

	    arr = name_to_nrefs[name]; 
	    if (!arr) {
	       arr = ({ });
	    }
	    arr += ({ nref });
	    if (sizeof(arr) > top_count) {
	       top_count = sizeof(arr);
	       top_name  = name;
	       top_ref   = nref;
	    }
	    name_to_nrefs[name] = arr;
	 }
      }
      if (top_count < 7) {
	 /* too few names remain with which to differentiate... */
	 for (i = 0; i < sizeof(nrefs); i ++) {
	     NRef nref;
	     mixed value;
	     object nref_ob;

	     nref = nrefs[i];
	     nref_ob = NRefOb(nref);
	     res_nrefs += ({ ({ nref_ob }) });
	     res_1st   += ({ describe_one(nref, actor, nref_ob, style) });
	     res_2nd   += ({ describe_one(nref, actor, nil,     style) });
	 }
	 nrefs = ({ });
      } else if (top_count < sizeof(nrefs) || !(style & STYLE_REDUCE_CHUNK)) {
	 if (top_count == 1) {
	     res_nrefs += ({ ({ NRefOb(top_ref) }) });
	     res_1st   += ({ describe_one(top_ref, actor, NRefOb(top_ref), style) });
	     res_2nd   += ({ describe_one(top_ref, actor, nil, style) });
	    nrefs -= ({ top_ref });
	 } else {
	     int   k, sz_k;
	     mixed *list;

	     list = name_to_nrefs[top_name];
	     nrefs -= list;
	     sz_k = sizeof(list);
	     for (k = 0; k < sz_k; k++) {
		 list[k] = NRefOb(list[k]);
	     }
	     res_nrefs += ({ list });
	     if (sscanf(top_name, "%s/%s", hstring, name) == 2) {
		res_1st   += ({ "your " + desc_cardinal(top_count) + " " + top_name });
		res_2nd   += ({ hstring + " " + desc_cardinal(top_count) + " " + top_name });
	     } else {
		res_1st   += ({ desc_cardinal(top_count) + " " + top_name });
		res_2nd   += ({ desc_cardinal(top_count) + " " + top_name });
	     }
	 }
      } else {
	 useless[top_name] = TRUE;
      }
   }

   sz_i = sizeof(lookers);
   results = allocate(sz_i);
   for (i = 0; i < sz_i; i++) {
       int sz_j;
       string *list;

       if (sizeof(lookers[i]->query_souls())) {
	   sz_j = sizeof(res_nrefs);
	   list = allocate(sz_j);;
	   for (j = 0; j < sz_j; j++) {
	       if (sizeof(lookers[i..i] & res_nrefs[j]) > 0) {
		   list[j] = res_1st[j];
	       } else {
		   list[j] = res_2nd[j];
	       }
	   }
	   if (sz_j == 1) {
	       results[i] = ({ lookers[i], list[0] });
	   } else {
	       results[i] = ({ lookers[i], implode(list[..sz_j - 2], ", ") + " and " + list[sz_j - 1] });
	   }
       } else {
	   results[i] = ({ lookers[i], nil });
       }
   }

   return results;
}

mapping map_inventory(object *inv, object looker) {
   mapping map, per_ob, per_detail, per_stance;
   string prep, stance, desc, foo;
   int i, j;

   map = ([ nil: ([ nil: ([ nil: ([ nil: ([ ]) ]) ]) ]) ]);

   for (i = 0; i < sizeof(inv); i ++) {
      NRef target;
      object ob;
      string detail;

      if (target = inv[i]->query_proximity()) {
	 /* we want to index a mapping by the proximity, but we can't use */
	 /* the prox in its raw form; it's an array, and DGD maps arrays */
	 /* by pointer, not by content (unlike for strings)..  */

	 /* instead of just storing half the NRef we will add another */
	 /* level to the nestedness and store both parts of the NRef */
/*	 if (IsPrime(prox)) {
	    prox = NRefOb(prox);
	 } else {
	    prox = NRefDetail(prox);
	 }
*/
	 ob = NRefOb(target);
	 detail = NRefDetail(target);
      } else {
	ob = nil;
	detail = nil;
      }

      per_ob = map[ob];
      if (!per_ob) {
	 map[ob] = per_ob = ([ ]);
      }

      per_detail = per_ob[detail];
      if (!per_detail) {
	 per_ob[detail] = per_detail = ([ ]);
      }

      stance = inv[i]->describe_stance();
      per_stance = per_detail[stance];
      if (!per_stance) {
	 per_detail[stance] = per_stance = ([ ]);
      }

      prep = inv[i]->describe_preposition();
      if (per_stance[prep]) {
	 per_stance[prep][inv[i]] = TRUE;
      } else {
	 per_stance[prep] = ([ inv[i]: TRUE]);
      }
   }
   return map;
}

static
string describe_draggers ( object *inv ) {
   int i;
   string tmp;
   mixed dragger;

   tmp = "";
   for (i = 0; i < sizeof(inv); i ++) {
      dragger = inv[i]->query_property("dragger");
      if (dragger &&
	  dragger->query_environment() == inv[i]->query_environment() ) {
         tmp += describe_one(inv[i]) + " is being restrained by " +
	    describe_one(dragger) + ". ";
      }
   }

   return tmp;
}

private
NRef *strip_integrals(NRef *nrefs) {
   mapping props;
   object ob, env;
   string prop;
   int i;

   nrefs = nrefs[..];

   for (i = 0; i < sizeof(nrefs); i ++) {
      if (IsPrime(nrefs[i])) {
	 ob = NRefOb(nrefs[i]);
	 if (prop = ur_integration_property(ob)) {
	    if (env = ob->query_environment()) {
	       props = env->query_properties();
	       props = prefixed_map(props, prop);
	       if (map_sizeof(props) > 0) {
		  nrefs[i] = nil;
	       }
	    }
	 }
      }
   }
   return nrefs -= ({ nil });
}

static
string describe_inventory(object *inv, object looker, varargs int style) {
   mapping map, per_ob, per_prox, per_stance, per_prep, per_detail;
   string *stances, *out, *preps, prox_desc, foo;
   object room, nref_ob;
   mixed *proxes, *refs, *details;
   int i, j, k, l;

   /* sort inventory by prox and stance */

   inv = strip_integrals(inv);
   inv = filter(inv, "can_identify", looker);

   if (!sizeof(inv)) {
      return "";
   }

   room = inv[0]->query_environment();
   map = map_inventory(inv, looker);

   out = ({ });

   proxes = map_indices(map);

   for (i = 0; i < sizeof(proxes); i ++) {
      per_ob = map[proxes[i]];
      nref_ob = proxes[i];
      details = map_indices(map[nref_ob]);

      for (j = 0; j < sizeof(details); j++) {
	 if (typeof(nref_ob) == T_NIL) {
	    prox_desc = nil;
	 } else {
	    if (nref_ob == looker) {
	       prox_desc = "you";
	    } else {
	       prox_desc = describe_one(NewNRef(nref_ob, details[j]),
					nil, looker);
	       if (prox_desc && !strlen(prox_desc)) {
		  prox_desc = nil;
	       }
	    }
	 }

	 per_detail = per_ob[details[j]];

	 stances = map_indices(per_detail);
	 for (k = 0; k < sizeof(stances); k ++) {
	    per_stance = per_detail[stances[k]];
	    preps = map_indices(per_stance);
	    for (l = 0; l < sizeof(preps); l ++) {
	       refs = map_indices(per_stance[preps[l]]);

	       foo = "";
	       if (stances[k] && strlen(stances[k])) {
		  foo += " " + stances[k];
	       }
	       if (preps[l] && strlen(preps[l])) {
		  foo += " " + preps[l];
	       } else if (prox_desc) {
		  foo += " near";
	       }
	       if (prox_desc) {
		  foo += " " + prox_desc;
	       }
	       if (i == 0) {
		  foo += " here";
	       }
	       if (sizeof(refs) > 0) {
		  string who, verb;

		  if (sizeof(refs) == 1) {
		     if (typeof(refs[0]) == T_OBJECT && refs[0] == looker) {
			who = "you"; verb = "are";
		     } else {
			who = describe_many(refs, nil, looker,
					    style | STYLE_NO_SHOW_INVIS); 
			catch {
			   verb = determine_prep(refs);
			} : {
			   verb = "is";
			}
		     }
		  } else {
		     who = describe_many(refs, nil, looker, 
					 style | STYLE_NO_SHOW_INVIS); 
		     verb = "are";
		  }
		  if (strlen(who))
		     out += ({ capitalize(who) + " " + verb + foo + "." });
	       }
	    }
	 }
      }
   }
   return implode(out, " ");
}

static
string describe_clothes(mapping clothes_worn, object looker,
			int style, int clothing) {
   int i, j;
   int index_size, covered_size, generic_size;
   string *index_list, *covered, *res;
   object *tmp_list, *tmp, *generic, *clothes, *weapons;
   mapping tmp_map;

   index_list = map_indices(clothes_worn);
   index_size = sizeof(index_list);
   res = ({ });
   weapons = ({ });
   clothes = ({ });

   if (style == STYLE_REL_LOC) {
      tmp_map = ([ ]);

      /* copy the tops of the stacks */
      for(i = 0; i < index_size; i++) {
	 tmp = clothes_worn[index_list[i]];

	 if (sizeof(tmp) == 1) {
	    tmp_map[tmp[0]] = TRUE;
	    continue;
	 }

	 /* work our way down from the tops of the stacks until we */
	 /* find the first non transparent article */
	 for (j = sizeof(tmp) - 1; j >= 0; j--) {
	       tmp_map[tmp[j]] = TRUE;

	    if (!ur_transparency(tmp[j])) {
	       break;
	    }

	 }
      }    

      tmp_list = map_indices(tmp_map);

      for(i = 0; i < sizeof(tmp_list); i++) {
	 if (ur_weapon(tmp_list[i]))
	    weapons += ({ tmp_list[i] });
	 else
	    clothes += ({ tmp_list[i] });
      }

      if (clothing) {
	 if (sizeof(clothes))
	    return describe_many(clothes, nil, looker, STYLE_NONPOSS | STYLE_NOCLUSTER);
	 else
	    return nil;
      }
      else {
	 if (sizeof(weapons)) 
	    return describe_many(weapons, nil, looker, STYLE_NONPOSS | STYLE_NOCLUSTER);
	 else
	    return nil;
      }
   }

   /* this is me looking at me */
   if (style == STYLE_FIRST_PERSON) {  
      /* this code is a copy of clothing::list_clothes */
      covered = map_indices(clothes_worn);
      covered_size = sizeof(covered);
      tmp_map = ([ ]);

      for(i = 0; i < covered_size; i++) {
	 if(clothes_worn[covered[i]] != nil) {
	    generic = clothes_worn[covered[i]];
	    generic_size = sizeof(generic);

	    for(j = 0; j < generic_size; j++) {
	       if (generic[j] != nil)
		  tmp_map[generic[j]] = generic[j];
	    }
	 }
      }
      tmp_list = map_indices(tmp_map);

      for(i = 0; i < sizeof(tmp_list); i++) {
	 if (ur_weapon(tmp_list[i]))
	    weapons += ({ tmp_list[i] });
	 else
	    clothes += ({ tmp_list[i] });
      }

      if (clothing) {
	 if (sizeof(clothes)) {
	    return describe_many(clothes, nil, looker, STYLE_NONPOSS | STYLE_NOCLUSTER);
	 }
	 return nil;
      }
      if (sizeof(weapons)) {
	 return describe_many(weapons, nil, looker, STYLE_NONPOSS | STYLE_NOCLUSTER);
      }
      return nil;
   }

   /* this is used for debug or stat */
   if (style == STYLE_NONPOSS) {
      for(i = 0; i < index_size; i++) {
	 res += ({ "Worn on " + index_list[i] + ": " });
	 res += ({ describe_many(clothes_worn[index_list[i]]) + "." });
	 res += ({ "\n" });
      }

      return implode(res, "");
   }

   return "nothing";
}

static
string describe_exits(NRef *exits) {
   string *res, *exit_dirs, desc, *foo;
   NRef exit;
   int i, exits_size;

   exits_size = sizeof(exits);
   res = ({ });

   for(i = 0; i < exits_size; i++) {
      exit = exits[i];

      desc = describe_one(exit);

      exit_dirs = ur_exit_dirs(NRefOb(exit), NRefDetail(exit));
      if (sizeof(exit_dirs)) {
	 res += ({ LINK(desc, "go " + exit_dirs[0], "command") +
		      "(" + implode(exit_dirs,", ") + ")" });
      } else {
	 foo = ur_snames(NRefOb(exit), NRefDetail(exit));
	 res += ({ LINK(desc, "enter " + foo[0], "command") + "(enter)" });
      }

      if ( i == (exits_size - 2))
	 res += ({ " and " });
      else if ( i == (exits_size - 1))
	 continue;
      else
	 res += ({ ", " });
   }

   return implode(res,"");
}

/* describe_doors()
** This functions shows the current state of an objects doors.
*/
static
string describe_doors(object ob, object looker) {
   NRef *closed, *open;
   mixed desc;

   closed = ob->create_nrefs(ob->query_closed_doors(looker, TRUE));
   open = ob->create_nrefs(ob->query_open_doors(looker, TRUE));
   desc = "";

   if (sizeof(closed) > 1) {
      desc += " " + capitalize(describe_many(closed, nil, looker)) +
	 " are closed.";
   } else if (sizeof(closed)) {
      desc += " " + capitalize(describe_many(closed, nil, looker)) +
	 " is closed.";
   }

   if (sizeof(open) > 1) {
      desc += " " + capitalize(describe_many(open, nil, looker)) +
	 " are open.";
   } else if (sizeof(open)) {
      desc += " " + capitalize(describe_many(open, nil, looker)) +
	 " is open.";
   }

   return desc;
}

/* 
**	describe_covered()
**
**	Return a string describing the clothes covering this object. 
**	Sanity checks are in this function.
*/
static
string describe_covered(object ob, object looker) {
   object worn_by, *covered;
   mixed desc;

   desc = "";

   /* if the object is a piece of clothing show what is covering it */
   if (worn_by = ob->query_worn_by()) {
      covered = worn_by->query_covered_by(ob);
	 
      if (sizeof(covered)) {
	 desc += capitalize(ob->query_pronoun()) + " is covered by " + 
	    describe_many(covered, looker, looker, STYLE_NONPOSS) +
	    ". ";
      }
   }

   return desc;
}



/*
**	describe_item()
**
**	This function describes the details of 'what', describes the
**	clothes/weapons, and finally describes the rest of the inventory.
*/

static
string describe_item(NRef what, object looker, string types...) {
   mapping clothes_worn;
   object ob, *proximate, *inv;
   string id, clothes, weapons, verb;
   mixed tmp;
   SAM desc, pdesc;
   int i, space, style;

   id = what->query_detail(looker);
   ob = what->query_object();

   /* Assign the Ur descriptions to 'desc' */
   i = 0;
   while (i < sizeof(types) && !desc) {
      object ur;

      if (types[i] == nil) {
	 i++;
	 continue;
      }
      if (looker->query_property("base:infravision")) {
	 tmp = "bright";
      } else {
	 switch(ob->query_light_category()) {
	 case LIGHT_DARK:
	    tmp = "dark";
	    break;
	 case LIGHT_DIM:
	    tmp = "dim";
	    break;
	 case LIGHT_BRIGHT:
	 default:
	    tmp = "bright";
	    break;
	 case LIGHT_SUPERBRIGHT:
	    tmp = "superbright";
	    break;
	 }
      }
      ur = ob;
      while (ur && !desc) {
	 desc = ur->query_description(id, types[i] + "-" + tmp);
	 if (!desc && tmp == "superbright") {
	    desc = ur->query_description(id, types[i] + "-bright");
	 }
	 if (!desc) {
	    desc = ur->query_description(id, types[i] + "-full");
	 }
	 if (!desc && tmp != "dark") {
	    desc = ur->query_description(id, types[i]);
	 }
	 ur = ur->query_ur_object();
      }
      if (!desc &&
	  !looker->query_property("base:infravision") &&
	  ob->query_light_category() == LIGHT_DARK) {
	 return "It's dark.";
      }
      pdesc = nil;
      while (ur && !pdesc) {
	 pdesc = ur->query_description(id, types[i] + "-" + tmp);
	 if (!pdesc && tmp == "superbright") {
	    pdesc = ur->query_description(id, types[i] + "-bright");
	 }
	 if (!pdesc) {
	    pdesc = ur->query_description(id, types[i] + "-full");
	 }
	 if (!pdesc && tmp != "dark") {
	    pdesc = ur->query_description(id, types[i]);
	 }
	 ur = ur->query_ur_object();
      }
      i ++;
   }
   if (sizeof(types & ({ "examine" }))) {
      style = STYLE_NOCLUSTER;
   }
   /* If 'desc' then translate the SAM code into text */
   if (desc) {
      desc = unsam(desc,
		   build_sam_args(ob, looker, looker, what) +
		   ([ "parent-desc": pdesc ]),
		   nil, ob);
   } else {
      desc = "You see emptiness.";
   }

   if (!IsPrime(what)) {
      /* TODO: kill this redundancy */
      /* if 'what' is NOT a prime then show its guts with less detail */
      inv = ob->query_inventory();

      for (i = 0; i < sizeof(inv); i ++) {
	 if (inv[i] != ob) {
	    if (!inv[i]->query_proximity() ||
		!NRefsEqual(inv[i]->query_proximity(), what)) {
	       inv[i] = nil;
	    }
	 }
      }
      proximate = inv - ({ nil });

      desc += " " + describe_inventory(proximate, looker, style);

      if (ur_is_door(ob, id)) {
	 desc +=
	    " " + capitalize(describe_one(what, nil, looker)) + " is " +
	    (ob->query_closed(id) ? "closed." : "open.");
      }
# if 0
      if (ur_luminosity(ob, id) > 0.0) {
	 desc +=
	    " " + capitalize(describe_one(what, nil, looker)) +
	    " is emanating light.";
      }
# endif

      return desc + "\n";
   }
      
   /* If 'looker' is looking at a real object and not just a detail */
   /* then show the guts of the object */

   /* Make sure 'ob' is not being held by someone, if so then the */
   /* looker is ok to proceed. Everyone else gets no inventory */
   /* description. */
   if (ur_public_container(ob) ||
       !ob->query_holder() || ob->query_holder() == looker) {
      /* If a container verb is NOT assigned then display the guts with */
      /* minor detail. */
      verb = ur_container_verb_third(ob);
      if (!verb || !strlen(verb)) {
	 inv = ob->query_inventory();
	 inv = strip_integrals(inv);
	 if (sizeof(inv)) {
	    desc += "\n\n";
	    if (strlen(tmp = describe_inventory(inv, looker, style))) {
	       desc += tmp + "\n";
	    }
	 }

	 desc += describe_covered(ob, looker);

	 if (strlen(tmp = describe_draggers(inv)))
	    desc += "\n" + tmp;

	 space = FALSE;
      } else {

	 /* If there are no open doors show that they are all closed. */
	 if (!ur_transparent_container(ob) &&
	     sizeof(ob->query_doors(looker)) > 0 &&
	     sizeof(ob->query_open_doors(looker)) == 0) {
	    desc += describe_doors(ob, looker);
	 } else {
	    /* We have an open door so lets show some guts! */

	    /* Display the state of the doors */
	    desc += describe_doors(ob, looker);

	    /* A container verb is set so the display of the guts will be */
	    /* done in a pretty manner. */
	    inv = ob->query_inventory() - ob->list_clothes();
	    inv -= ({ looker });

	    inv = strip_integrals(inv);

	    /* This will show what 'ob' is carrying. */
	    if (sizeof(inv)) {
	       if (looker == ob) {
		  /* special case: look at ourselves from third person */
		  desc += " " + capitalize(ob->query_pronoun()) + " " +
		     verb + " " + describe_many(inv, ob, nil,
						style | STYLE_NONPOSS) + ".";
	       } else {
		  desc += " " + capitalize(ob->query_pronoun()) + " " +
		     verb + " " + describe_many(inv, ob, looker,
						style | STYLE_NONPOSS) + ".";
	       }
	    }
	 }

	 /* This will show what 'ob' is wearing and wielding. */
	 clothes_worn = ob->query_clothes_worn();
	 if (ob == looker)
	    style = STYLE_FIRST_PERSON;
	 else
	    style = STYLE_REL_LOC;
	 clothes = describe_clothes(clothes_worn, looker, 
				    style, TRUE);
	 weapons = describe_clothes(clothes_worn, looker, 
				    style, FALSE);

	 if (weapons) {
	    desc += " " + capitalize(ob->query_pronoun()) + 
	       " is wielding " + weapons + ".";
	 }

	 if (clothes) {
	    desc += " " + capitalize(ob->query_pronoun()) + " is wearing " +
	       clothes + ".";
	 } else {
	    if (ur_clothes_expected(ob)) {
	       desc += " " + capitalize(ob->query_pronoun()) + 
		  " is nude.";
	    }
	 }

	 space = TRUE;
      }
   }


   /* If 'looker' is looking at its environment then this segment will */
   /* be executed. This segment describes the guts of the environemnt */
   /* with prox relations. */
   if (ob->query_environment() &&
       looker->contained_by(ob->query_environment())) {

      inv = ob->query_environment()->query_inventory() - ({ looker, ob });

      for (i = 0; i < sizeof(inv); i ++) {
	 if (!inv[i]->query_proximity() ||
	     !NRefsEqual(inv[i]->query_proximity(), what)) {
	    inv[i] = nil;
	 }
      }
      proximate = inv - ({ nil });
      if (strlen(tmp = describe_inventory(proximate, looker, style))) {
	 if (space) {
	    desc += " ";
	 }
	 desc += tmp;
      }
   }
# if 0
   if (ur_luminosity(ob, id) > 0.0) {
      desc +=
	 " " + capitalize(describe_one(what, nil, looker)) +
	 " is emanating light.";
   }
# endif
   return desc + "\n";
}
