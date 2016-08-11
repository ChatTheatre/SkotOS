/*
 * /usr/SkotOS/lib/noun.c
 *
 * Copyright Skotos Tech Inc 2000
 */

# include <type.h>
# include <SkotOS.h>
# include "/usr/SkotOS/include/inglish.h"

private inherit "/lib/string";
private inherit "/base/lib/urcalls";

# include "/usr/SkotOS/include/noun.h"
# include "/usr/SkotOS/include/describe.h"

static mixed *locate(object user, Ob *refs, object filter, object looker,
		     mapping among_set, object *locations);

# define Fail(user, flags, str) \
	if ((flags & LF_HUSH_ALL) == 0) { \
	    user->paragraph(str); \
	} \
	return nil;

/*
 * Utility function prototypes.
 */
private string  refdescs(NRef *refs, object looker);
private mapping make_among(mixed *among, object looker);
private object *make_locations(mixed *among);

static  void   locate_update(object user, NRef *res);

/* public */

static mixed
locate_one(object user, int flags, Ob *refs, object looker,
	   object locations...) {
   mixed *res;

   res = locate(user, refs, nil, looker, nil, locations);
   switch (res[LOC_RESULT]) {
   case LOC_ONE:
       if (!(flags & LF_NO_UPDATE)) locate_update(user, res[LOC_DATA]);
       return res[LOC_DATA][0];
   case LOC_MULTI:
       Fail(user, flags, refdescs(res[LOC_DATA], looker));
   default:
       /* Errors */
       Fail(user, flags, res[LOC_DATA]);
   }
}

static mixed
locate_first(object user, int flags, Ob *ref, object filter, object looker,
	     object locations...) {
   mixed *res;

   res = locate(user, ref, filter, looker, nil, locations);
   switch (res[LOC_RESULT]) {
   case LOC_ONE:
   case LOC_MULTI:
       if (!(flags & LF_NO_UPDATE)) locate_update(user, res[LOC_DATA]);
       return res[LOC_DATA][0];
   default:
       /* Errors */
       Fail(user, flags, res[LOC_DATA]);
   }
}

static NRef *
locate_best(object user, int flags, Ob *ref, object filter, object looker,
	    object locations...) {
   mixed *res;

   res = locate(user, ref, filter, looker, nil, locations);
   switch (res[LOC_RESULT]) {
   case LOC_ONE:
   case LOC_MULTI:
       if (!(flags & LF_NO_UPDATE)) locate_update(user, res[LOC_DATA]);
       return res[LOC_DATA];
   default:
       /* Errors */
       Fail(user, flags, res[LOC_DATA]);
   }
}

static NRef *
locate_many(object user, int flags, Ob *refs, object looker,
	    object locations...) {
   mixed *res;

   res = locate(user, refs, nil, looker, nil, locations);
   switch (res[LOC_RESULT]) {
   case LOC_ONE:
   case LOC_MULTI:
       if (!(flags & LF_NO_UPDATE)) locate_update(user, res[LOC_DATA]);
       return res[LOC_DATA];
   default:
       /* Errors */
       Fail(user, flags, res[LOC_DATA]);
   }
}

static mixed
search_one(object user, int flags, Ob *ref, object looker, mixed among...) {
   mixed *res;

   res = locate(user, ref, nil, looker, make_among(among, looker),
		make_locations(among));
   switch (res[LOC_RESULT]) {
   case LOC_ONE:
       if (!(flags & LF_NO_UPDATE)) locate_update(user, res[LOC_DATA]);
       return res[LOC_DATA][0];
   case LOC_MULTI:
       /* this can happen if one hit is from singular, but only if ref[0] */
       /* is both singular and plural (as in e.g. 'sheep') so this works */
       Fail(user, flags, refdescs(res[LOC_DATA], looker));
   default:
       /* Errors */
       Fail(user, flags, res[LOC_DATA]);
   }
}

static mixed
search_first(object user, int flags, Ob *ref, object filter, object looker,
	     mixed among...) {
   mixed *res;

   res = locate(user, ref, filter, looker, make_among(among, looker),
		make_locations(among));
   switch (res[LOC_RESULT]) {
   case LOC_ONE:
   case LOC_MULTI:
       if (!(flags & LF_NO_UPDATE)) locate_update(user, res[LOC_DATA]);
       return res[LOC_DATA][0];
   default:
       /* Errors */
       Fail(user, flags, res[LOC_DATA]);
   }
}

static NRef *
search_best(object user, int flags, Ob *refs, object filter, object looker,
	    mixed among...) {
    mixed *res;

    res = locate(user, refs, filter, looker, make_among(among, looker),
		 make_locations(among));
    switch (res[LOC_RESULT]) {
    case LOC_ONE:
    case LOC_MULTI:
	if (!(flags & LF_NO_UPDATE)) locate_update(user, res[LOC_DATA]);
	return res[LOC_DATA];
    default:
	/* Errors */
	Fail(user, flags, res[LOC_DATA]);
    }
}

static NRef *
search_many(object user, int flags, Ob *refs, object looker, mixed among...) {
    mixed *res;

    res = locate(user, refs, nil, looker, make_among(among, looker),
		 make_locations(among));
    switch (res[LOC_RESULT]) {
    case LOC_ONE:
    case LOC_MULTI:
	if (!(flags & LF_NO_UPDATE)) locate_update(user, res[LOC_DATA]);
        return res[LOC_DATA];
    default:
	/* Errors */
	Fail(user, flags, res[LOC_DATA]);
    }
}

/* internal */
private NRef *
unique_refs(NRef *res, varargs mapping among) {
   mapping seen, map;
   NRef *out;
   int i, j;

   seen = ([ ]);
   out = allocate(sizeof(res));

   for (i = 0; i < sizeof(res); i ++) {
      /* if an 'among' filter is supplied, only accept nrefs in it */
      if (among) {
	 map = among[NRefOb(res[i])];
	 if (!map || !map[NRefDetail(res[i])]) {
	    continue;
	 }
      }
      /* otherwise just check if it's already been seen in this array */
      if (map = seen[NRefOb(res[i])]) {
	 if (map[NRefDetail(res[i])]) {
	    continue;
	 }
	 map[NRefDetail(res[i])] = TRUE;
      } else {
	 seen[NRefOb(res[i])] = ([ NRefDetail(res[i]) : TRUE ]);
      }
      out[j ++] = res[i];
   }
   return out[.. j-1];
}

private mapping
make_among(mixed *among, object looker) {
   int     i, sz;
   mapping out;

   out = ([ ]);

   sz = sizeof(among);
   for (i = 0; i < sz; i ++) {
       mixed item;
       mapping map;

       item = among[i];

       if (map = out[NRefOb(item)]) {
	   map[NRefDetail(item)] = TRUE;
       } else {
	   out[NRefOb(item)] = ([ NRefDetail(item) : TRUE ]);
       }
   }
   return out;
}

private object *
make_locations(mixed *among) {
   int     i, sz;
   mapping out;

   out = ([ ]);

   sz = sizeof(among);
   for (i = 0; i < sz; i++) {
       object env, obj;
       mixed  item;

       item = among[i];

       if (IsNRef(item)) {
	  if (IsPrime(item)) {
	     env = NRefOb(item)->query_environment();
	     obj = NRefOb(item);
	  } else {
	     obj = env = NRefOb(item);
	  }
       } else {
	  env = item->query_environment();
	  obj = item;
       }
       if (env) {
	   out[env] = TRUE;
       } else {
	   /* things in the NIL are their own locations */
	   out[obj] = TRUE;
       }
   }
   return map_indices(out);
}

private string
refdescs(NRef *refs, object looker) {
    string *arr, str;

    arr = cluster_briefs(refs, looker, looker, 0);

    if (sizeof(arr) == 1) {
       str = arr[0];
    } else {
       str = implode(arr[.. sizeof(arr) - 2], ", ") + " or " +
	  arr[sizeof(arr) - 1];
    }
    return "You must choose one of " + str + ".";
}

static int
share_environment(object body, mixed target)
{
    object target_ob, body_env, target_env;

    if (!body) {
	return FALSE;
    }
    target_ob = NRefOb(target);
    if (!target_ob) {
	return FALSE;
    }
    body_env = body->query_environment();
    target_env = target_ob;
    while (target_env) {
	if (body_env == target_env) {
	    return TRUE;
	}
	target_env = target_env->query_environment();
    }
    return FALSE;
}

static void
locate_update(object user, NRef *res) {
    int i, sz;

    if (!user) {
	return;
    }

    sz = sizeof(res);
    for (i = 0; i < sz; i ++) {
	mixed item;

	item = res[i];
	if (IsPrime(item)) {
	    object ob;

	    ob = NRefOb(item);
	    if (ob == user->query_body()) {
		/* Don't update reference when referring to one-self */
		continue;
	    }
	    if (ob->query_male()) {
		user->set_last_male(item);
	    } else if (ob->query_female()) {
		user->set_last_female(item);
	    } else {
		user->set_last_neuter(item);
	    }
	} else {
	    user->set_last_neuter(item);
	}
    }
}

/*
 * The actual working part of all the visible functions:
 */
static mixed *locate(object user, Ob *refs, object filter,
		     object looker, mapping among_set,
		     object *orig_locations) {
   object *locations;
   string word, *adj, phrase;
   mixed *ref;
   NRef *stmp, *ptmp, *res, tmp;
   int i, sz_i, j, sz_j;

   res = ({ });
   for (i = 0, sz_i = sizeof(refs); i < sz_i; i ++) {
      int       details_only;
      string	*words;
      Ob	poss;
      NRef	possref;
      NRef	**tmp_holder;

      locations = orig_locations[..];
      ref = refs[i];
      details_only = FALSE;

      if (ObArt(ref)) {
	 NRef	who;
	 int	set;
	 object *inv;

	 who = nil;

	 switch(ObArt(ref)) {
	 case ART_ALL:
	    if (!sizeof(ObWords(ref))) {
	        return LOC_PAIR(LOC_ERROR,
				"You have to be more specific than 'all'.");
	    }
	    break;
	 case ART_ENV:
	    who = looker->query_environment();
	    set = TRUE;
	    break;
	 case ART_ME:
	    who = looker;
	    set = TRUE;
	    break;
	 case ART_HIM:
	     who = user->query_last_male();
	     if (who && !share_environment(looker, who)) {
		 return LOC_PAIR(LOC_ERROR_GONE,
				 capitalize(describe_one(who)) +
				 " is not here.");
	     }
	     set = TRUE;
	     break;
	 case ART_HER:
	     who = user->query_last_female();
	     if (who && !share_environment(looker, who)) {
		 return LOC_PAIR(LOC_ERROR_GONE,
				 capitalize(describe_one(who)) +
				 " is not here.");
	     }
	     set = TRUE;
	     break;
	 case ART_IT:
	     who = user->query_last_neuter();
	     if (who && !share_environment(looker, who)) {
		 return LOC_PAIR(LOC_ERROR_GONE,
				 capitalize(describe_one(who)) +
				 " is not here.");
	     }
	     set = TRUE;
	     break;
	 }	 
	 if (set) {
	    if (!who || NRefOb(who)->is_hidden(NRefDetail(who))) {
	       return LOC_PAIR(LOC_ERROR,
			       "I don't know what the pronoun refers to.");
	    }
	    res += ({ who });
	    continue;
	 }
      }

      words = ObWords(ref);

      if (sizeof(words)) {
	 word = words[sizeof(words)-1];
	 adj  = words[.. sizeof(words)-2];
      } else {
	 word = nil;
	 adj = ({ });
      }

      phrase = implode(words, " ");

      if (poss = ObPoss(ref)) {
	 string lastword;
	 mixed *res;
	 int set;

	 possref = nil;

	 set = FALSE;

	 if (ObArt(poss)) {
	    switch(ObArt(poss)) {	
	    case ART_IN_ENV:
	       possref = looker->query_environment();
	       set = TRUE;
	       break;
	    case ART_ENV:
	       possref = looker->query_environment();
	       set = TRUE;
	       break;
	    case ART_MINE:
	       possref = looker;
	       set = TRUE;
	       break;
	    case ART_HIS:
	       possref = user->query_last_male();
	       if (possref && !share_environment(looker, possref)) {
		   return LOC_PAIR(LOC_ERROR_GONE,
				   capitalize(describe_one(possref)) +
				   " is not here.");
	       }
	       set = TRUE;
	       break;
	    case ART_HERS:
	       possref = user->query_last_female();
	       if (possref && !share_environment(looker, possref)) {
		   return LOC_PAIR(LOC_ERROR_GONE,
				   capitalize(describe_one(possref)) +
				   " is not here.");
	       }
	       set = TRUE;
	       break;
	    case ART_ITS:
	       possref = user->query_last_neuter();
	       if (possref && !share_environment(looker, possref)) {
		   return LOC_PAIR(LOC_ERROR_GONE,
				   capitalize(describe_one(possref)) +
				   " is not here.");
	       }
	       set = TRUE;
	       break;
	    }
	 }
	 if (!possref) {
	    if (set) {
	       return LOC_PAIR(LOC_ERROR,
			       "I don't know what the pronoun refers to.");
	    }

	    if (typeof(poss[sizeof(poss) - 1]) != T_STRING) {
	       DEBUG("ref = " + dump_value(ref)  + "; poss = " + dump_value(poss));
	    }
	    if (!sscanf(poss[sizeof(poss)-1], "%s'", lastword)) {
	       lastword = poss[sizeof(poss)-1];
	    }

	    /* Don't modify the -original- data. */
	    poss = poss[..];
	    poss[sizeof(poss)-1] = lastword;

	    res = locate(user, ({ poss }), nil, looker, nil, locations);
	    switch (res[LOC_RESULT]) {
	    case LOC_ONE:
	       /* Excellent! */
	       possref = res[LOC_DATA][0];
	       break;
	    case LOC_MULTI:
	       return LOC_PAIR(LOC_ERROR_MULTI,
			       refdescs(res[LOC_DATA], looker));
	    default:
	       /* 'Promote' the error when sending it up in the call-chain. */
	       return LOC_PAIR(LOC_ERROR_POSS, res[LOC_DATA]);
	    }
	    if (!IsPrime(possref)) {
	       return LOC_PAIR(LOC_ERROR,
			       describe_one(possref, nil, nil, STYLE_DEFINITE) +
			       (ur_volition(NRefOb(possref)) ?
				" has no " : " holds no ") +
			       phrase + ".");
	    }
	 }
	 if (possref) {
	    if (!ur_transparent_container(NRefOb(possref)) &&
		sizeof(NRefOb(possref)->query_doors(looker)) > 0 &&
		sizeof(NRefOb(possref)->query_open_doors(looker)) == 0 &&
		!looker->contained_by(NRefOb(possref))) {
	       if (IsPrime(possref)) {
		   details_only = TRUE;
	       } else {
		   return LOC_PAIR(LOC_ERROR,
				   describe_one(possref, nil, looker, STYLE_DEFINITE) +
				   " is closed!");
	       }
	    }
	    locations = ({ NRefOb(possref) });
	 }
      }

      stmp = ({ });
      for (j = 0, sz_j = sizeof(locations); j < sz_j; j ++) {
 	 stmp += locations[j]->single_match(word, looker, adj, details_only);
      }

      ptmp = ({ });
      for (j = 0, sz_j = sizeof(locations); j < sz_j; j ++) {
	 ptmp += locations[j]->plural_match(word, looker, adj, details_only);
      }
      stmp = unique_refs(stmp, among_set);
      ptmp = unique_refs(ptmp, among_set);

      if (filter) {
	 tmp_holder = call_other(filter, "filter_refs", looker,
				 stmp, ptmp, ref);
	 stmp = tmp_holder[0];
	 ptmp = tmp_holder[1];
      }

      for (j = 0; j < sizeof(stmp); j++) {
	 if (!looker->can_identify(stmp[j])) {
	    stmp[j] = nil;
	 }
      }
      stmp -= ({ nil });

      for (j = 0; j < sizeof(ptmp); j++) {
	 if (!looker->can_identify(ptmp[j])) {
	    ptmp[j] = nil;
	 }
      }
      ptmp -= ({ nil });

      if (!sizeof(stmp) && !sizeof(ptmp)) {
	 if (possref) {
	    if (details_only) {
	       return LOC_PAIR(LOC_ERROR,
			       describe_one(possref, nil, looker, STYLE_DEFINITE) +
			       " is closed!");
	    } else {
		return LOC_PAIR(LOC_ERROR,
				describe_one(possref, nil, nil, STYLE_DEFINITE) +
				(ur_volition(NRefOb(possref)) ?
				 " has no " : " holds no ") +
				phrase + ".");
	    }
	 }
	 /* if you change this message, change ~TextIF/lib/cmds/social.c too */
	 return LOC_PAIR(LOC_ERROR, "You see no " + phrase + " here.");
      }

      /* we need the stmps before we can test for ART_LAST */
      if (ObArt(ref) == ART_LAST) {
	 res += ({ stmp[sizeof(stmp) - 1] });
	 continue;
      }

      /* process Ordinal and Cardinal stuff */
      if (typeof(ObOC(ref)) == T_ARRAY) {
	 object ob;
	 float amount;

	 /* it's a volume or weight reference */
	 if (sizeof(stmp) > 1) {
	   if (sizeof(stmp) > 1) {
	       /* there should only be one single_match for each of the refs */
	       return LOC_PAIR(LOC_ERROR_MULTI, refdescs(stmp, looker));
	    }
	 }
	 ob = NRefOb(stmp[0]);
	 if (ObOC(ref)[0] == "V" && ur_by_weight(ob)) {
	    amount = ObOC(ref)[1] * ob->query_actual_density() * 1E3;
	 } else if (ObOC(ref)[0] == "W" && !ur_by_weight(ob)) {
	    amount = ObOC(ref)[1] / ob->query_actual_density() * 1E-3;
	 } else {
	    amount = ObOC(ref)[1];
	 }
	 res += ({ MutatedNRef(stmp[0], amount) });
      } else if (ObOC(ref) < 0) {
	 int ix;

	 if (sizeof(stmp) > 0) {
	    ix = -1 - ObOC(ref);
	    if (ix >= sizeof(stmp)) {
	       return LOC_PAIR(LOC_ERROR,
			       "There isn't a " + desc_ordinal(-ObOC(ref)) +
			       " " + phrase + ", there " +
			       (sizeof(stmp) == 1 ? "is": "are") + " only " +
			       desc_cardinal(sizeof(stmp)) + ".");
	    }
	    res += ({ stmp[ix] });
	 } else {
	    /* the fourth humans */
	    return LOC_PAIR(LOC_ERROR,
			    "Perhaps you mean '" + desc_cardinal(-ObOC(ref)) +
			    " " + phrase + "'?");
	 }
      } else if (ObOC(ref) == 1) {
	 if (sizeof(stmp) == 0) {
	    /* one humans */
	    return LOC_PAIR(LOC_ERROR,
			    "Perhaps you mean 'the " + phrase + "'?");
	 }
	 /* one human */
	 res += ({ stmp[random(sizeof(stmp))] });
      }	else if (ObOC(ref) > 0) {
	 if (sizeof(ptmp) > 0) {
	    if (ObOC(ref) > sizeof(ptmp)) {
	       return LOC_PAIR(LOC_ERROR,
			       "There aren't " + desc_cardinal(ObOC(ref)) +
			       " " + phrase + ", there " +
			       (sizeof(ptmp) == 1 ? "is": "are") + " only " +
			       desc_cardinal(sizeof(ptmp)) + ".");
	    }
	    res += ptmp[.. ObOC(ref)-1];
	 } else {
	    /* the four human */
	    return LOC_PAIR(LOC_ERROR,
			    "Perhaps you mean '" + desc_ordinal(ObOC(ref)) +
			    " " + phrase + "'?");
	 }
      } else {
	 switch(ObArt(ref)) {
	 case ART_UNDEF:
	    if (sizeof(stmp) == 0) {
	       /* a humans */
	       return LOC_PAIR(LOC_ERROR,
			       "Perhaps you mean 'the " + phrase + "'?");
	    }
	    /* a human */
	    res += ({ stmp[random(sizeof(stmp))] });
	    break;
	 case ART_ALL:
	    res += stmp + ptmp;
	    break;
	 default:
	   if (sizeof(stmp) > 1) {
	       /* there should only be one single_match for each of the refs */
	       return LOC_PAIR(LOC_ERROR_MULTI, refdescs(stmp, looker));
	    }
	    /* if there are any single matches, give them precedent */
	    if (sizeof(stmp) > 0) {
	       res += stmp;
	    } else {
	       res += ptmp;
	    }
	 }
      }
   }

   res = unique_refs(res);
   if (sizeof(res) > 20) {
      if (sizeof(res) > 40) {
	 if (sizeof(res) > 80) {
	    SysLog("Somebody fiddling with > 80 things...");
	 } else {
	    SysLog("Somebody fiddling with > 40 things...");
	 }
      } else {
	 SysLog("Somebody fiddling with > 20 things...");
      }
   }
   return LOC_PAIR((sizeof(res) == 1) ? LOC_ONE : LOC_MULTI, res);
}
