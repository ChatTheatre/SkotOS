/*
**	/usr/TextIF/data/filterfarm.c
**
**	This object (or one sharing its interface) is sent to the
**	locate() function as a filtering tool. It is instantiated
**	with some relevant contextual information, e.g. the body
**	performing the locate(). It also accepts a sequence of
**	named filters to apply to the stmp/ptmp stream.
**
**	Copyright Skotos Tech Inc 2000
*/

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/type";

private inherit "/base/lib/urcalls";

# include "/usr/SkotOS/include/noun.h"
# include "/usr/SkotOS/include/inglish.h"

private string	*funs;
private int	*flags;
private object	looker;

# define FLAG_NOT	1
# define FLAG_SORT	2

void add_filter(string filter);

static
void configure(string *arr) {
   int i;

   funs = ({ });
   flags = ({ });

   for (i = 0; i < sizeof(arr); i ++) {
      add_filter(arr[i]);
   }
}

void add_filter(string filter) {
   int flag, i, sz, found;

   switch(filter[0]) {
   case '<':
      flag = FLAG_SORT;
      break;
   case '>':
      flag = FLAG_NOT | FLAG_SORT;
      break;
   default:
      flag = 0;
      break;
   case '!':
      flag = FLAG_NOT;
      break;
   }
   if (flag) {
      filter = "i_" + filter[1 ..];
   } else {
      filter = "i_" + filter;
   }
   found = FALSE;
   for (i = 0, sz = sizeof(funs); !found && i < sz; i++) {
      if (funs[i] == filter && flags[i] == flag) {
	 found = TRUE;
      }
   }
   if (!found) {
      funs  += ({ filter });
      flags += ({ flag });
   }
}

void delete_filter(string filter) {
   int flag, i, sz;

   DEBUG("delete_filter(" + dump_value(filter) + ")");
   switch(filter[0]) {
   case '<':
      flag = FLAG_SORT;
      break;
   case '>':
      flag = FLAG_NOT | FLAG_SORT;
      break;
   default:
      flag = 0;
      break;
   case '!':
      flag = FLAG_NOT;
      break;
   }
   if (flag) {
      filter = "i_" + filter[1 ..];
   } else {
      filter = "i_" + filter;
   }
   for (i = 0, sz = sizeof(funs); i < sz; i++) {
      if (funs[i] == filter && flags[i] == flag) {
	 funs  = funs[..i - 1]  + funs[i + 1..];
	 flags = flags[..i - 1] + flags[i + 1..];
	 return;
      }
   }
}

string *query_filters() {
   string *out;
   int i;

   out = allocate(sizeof(funs));

   for (i = 0; i < sizeof(out); i ++) {
      switch(flags[i]) {
      case 0:
	 out[i] = funs[i][2 ..];
	 break;
      case FLAG_NOT:
	 out[i] = "!" + funs[i][2 ..];
	 break;
      case FLAG_SORT:
	 out[i] = "<" + funs[i][2 ..];
	 break;
      case FLAG_SORT | FLAG_NOT :
	 out[i] = ">" + funs[i][2 ..];
	 break;
      default:
	 error("unknown flag: " + flags[i]);
      }
   }
   return out;
}

	 
NRef **filter_refs(object body, NRef *srefs, NRef *prefs, Ob ref) {
   NRef **sstreams, **pstreams;
   NRef **snew, **pnew;

   int i, j;

   looker = body;

   if (ObOC(ref) != 0) {
      return ({ srefs, prefs });
   }
   switch(ObArt(ref)) {
   case ART_UNDEF:
   case ART_ALL:
   case ART_LAST:
      return ({ srefs, prefs });
   }

   sstreams = ({ srefs });
   pstreams = ({ prefs });

   for (i = 0; i < sizeof(funs); i ++) {
      snew = allocate(sizeof(sstreams) * 2);
      pnew = allocate(sizeof(sstreams) * 2);
      for (j = 0; j < sizeof(sstreams); j ++) {
	 snew[2*j] = filter(sstreams[j], funs[i], this_object(),
			    flags[i] & FLAG_NOT);
	 snew[2*j+1] = sstreams[j] - snew[2*j];
	 pnew[2*j] = filter(pstreams[j], funs[i], this_object(),
			    flags[i] & FLAG_NOT);
	 pnew[2*j+1] = pstreams[j] - pnew[2*j];
      }
      XDebug("After " + (flags[i] & FLAG_NOT ? "(NOT)":"") + "'" + funs[i] +
	     "'...");
      XDebug("sstreams: " + dump_value(sstreams) + " has become...");
      XDebug("snew: " + dump_value(snew));
      sstreams = snew;
      pstreams = pnew;
   }
   for (i = 0; i < sizeof(sstreams); i ++) {
      if (sizeof(sstreams[i])) {
	 srefs = sstreams[i][0 .. 0];
	 break;
      }
   }
   for (i = 0; i < sizeof(pstreams); i ++) {
      if (sizeof(pstreams[i])) {
	 prefs = pstreams[i];
	 break;
      }
   }
   return ({ srefs, prefs });
}

static
int i_carried(NRef ref, int not) {
   return NRefOb(ref)->contained_by(looker) ^ not;
}

static
int i_worn(NRef ref, int not) {
   return (NRefOb(ref)->query_worn_by() == looker) ^ not;
}

static
int i_open(NRef ref, int not) {
   return (ur_is_door(NRefOb(ref), NRefDetail(ref)) &&
	   !NRefOb(ref)->query_closed(NRefDetail(ref))) ^ not;
}

static
int i_unlocked(NRef ref, int not) {
   return (ur_is_door(NRefOb(ref), NRefDetail(ref)) &&
	   !NRefOb(ref)->query_locked(NRefDetail(ref))) ^ not;
}

static
int i_readable(NRef ref, int not) {
   return
      (!!ur_description(NRefOb(ref), NRefDetail(ref), "writing") ||
       !!ur_description(NRefOb(ref), NRefDetail(ref), "delivery-from") ||
       !!ur_description(NRefOb(ref), NRefDetail(ref), "delivery-end") ||
       !!ur_description(NRefOb(ref), NRefDetail(ref), "delivery-to") ||
       !!ur_property(NRefOb(ref), "writable")) ^ not;
}
