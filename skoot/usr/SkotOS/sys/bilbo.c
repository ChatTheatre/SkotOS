/*
**	/usr/SkotOS/sys/bilbo.c
**
**	Copyright Skotos Tech Inc 2000
*/

# include <base.h>
# include <nref.h>

private inherit "/lib/array";

inherit module "/lib/module";
inherit bilbo "/usr/SkotOS/lib/bilbo";

inherit "/usr/SkotOS/lib/describe";


static
void create() {
   module::create("BaseWorld");
   bilbo::create();
}

static
void BaseWorld_loaded() {
   BASE_INITD->set_runner("bilbo", this_object());
}

void patch_runner() {
   BASE_INITD->set_runner("bilbo", this_object());
}

int run_signal(object ob, string signal, NRef target, mapping args,
	       object *third) {
   string tdet;
   object runner;
   mixed code;

   if (!args) {
      args = ([ ]);
   }

   /* this to be removed some glorious day */
   { 
      string *sar, s1;
      int i;

      args +=
	 ([ "adverb": args["adverb"] ? args["adverb"] : "",
	    "prep": args["prep"] ? args["prep"] : "" ]);

      if (s1 = args["evoke"]) {
	 sscanf(s1, "\"%s\"", s1);

	 args["evokefull"] = s1;

	 sar = explode(s1, " ");
	 for(i = 0; i < sizeof(sar); i ++) {
	    args["evoke"+(i+1)] = sar[i];
	 }
	 args["evoke0"] = sizeof(sar);
      } else {
	 args["evoke0"] = 0;
      }
   }
   if (!target) {
      /* if there is no target, run any solo script */
      if (!run_bilbo(ob, ob, signal, "solo", nil, args)) {
	 return FALSE;
      }
      return TRUE;
   }
   /* backwards compatibility for Generic system */
   if (signal == "start" || signal == "stop" ||
       signal == "enter_room" || signal == "leave_room") {
      if (!run_bilbo(ob, NRefOb(target), signal, "auto",
		     NRefDetail(target), args)) {
	 return FALSE;
      }
      return TRUE;
   }
   if (signal == "renamed") {
      if (!run_bilbo(ob, NRefOb(target), signal, "solo",
		     NRefDetail(target), args)) {
	 return FALSE;
      }
      return TRUE;
   }
   if (signal == "move") {
      object *inv;
      int i;

      inv = ob->query_environment()->query_inventory();

      for (i = 0; i < sizeof(inv); i ++) {
	 if (!run_bilbo(inv[i], ob, "move", "auto", nil, args)) {
	    return FALSE;
	 }
      }
      return TRUE;
   }

   /* else run any act/react scripts */
   if (!run_bilbo(ob, NRefOb(target), signal, "act",
		  NRefDetail(target), args)) {
      return FALSE;
   }
   if (!run_bilbo(NRefOb(target), ob, signal, "react",
		  NRefDetail(target), args)) {
      return FALSE;
   }
   return TRUE;
}

int new_signal(object ob, string signal, mapping react_map,
	       mapping witness_map, mapping args) {
   string *reactors;
   mixed arr;
   int i, j;

   if (!args) {
      args = ([ ]);
   } else {
      args = copy(args);
   }
   args["actor"] = ob;

   /* this to be removed some glorious day */
   { 
      string *sar;
      mixed s1;
      int i;

      args +=
	 ([ "adverb": args["adverb"] ? args["adverb"] : "",
	    "prep": args["prep"] ? args["prep"] : "" ]);

      s1 = args["evoke"];
      if (typeof(s1) == T_STRING) {
	 sscanf(s1, "\"%s\"", s1);

	 args["evokefull"] = s1;

	 sar = explode(s1, " ");
	 for(i = 0; i < sizeof(sar); i ++) {
	    args["evoke"+(i+1)] = sar[i];
	 }
	 args["evoke0"] = sizeof(sar);
      } else {
	 args["evoke0"] = 0;
      }
   }
   if (!react_map || !map_sizeof(react_map)) {
      /* run act */
      if (!run_bilbo(ob, ob, signal, "act", nil, args)) {
	 return FALSE;
      }
      /* backwards compat */
      if (!run_bilbo(ob, ob, signal, "solo", nil, args)) {
	 return FALSE;
      }
      return TRUE;
   }
   /* run reacts */


   reactors = map_indices(react_map);
   for (i = 0; i < sizeof(reactors); i ++) {
      mixed val;

      val = react_map[reactors[i]];

      switch(typeof(val)) {
      case T_OBJECT:
	 arr = ({ val });
	 break;
      case T_ARRAY:
	 arr = val;
	 break;
      case T_INT:
	 arr = args[reactors[i]];
	 if (typeof(arr) == T_ARRAY) {
	    break;
	 }
	 /* else fall through to ignore */
      default:
	 /* shouldn't happen */
	 continue;
      }

      for (j = 0; j < sizeof(arr); j ++) {
	 args["target"] = arr[j];
	 if (!run_bilbo(ob, NRefOb(arr[j]), signal, "act",
			NRefDetail(arr[j]), args)) {
	    arr[j] = nil;
	    continue;
	 }
	 if (!run_bilbo(NRefOb(arr[j]), ob, signal, "react",
			NRefDetail(arr[j]), args)) {
	    arr[j] = nil;
	    continue;
	 }
      }
      arr -= ({ nil });
      if (typeof(val) == T_INT) {
	 args[reactors[i]] = arr;
      }
      if (!sizeof(arr)) {
	 return FALSE;
      }
   }
   return TRUE;
}
