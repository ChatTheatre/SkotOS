/*
**
**	/usr/SkotOS/sys/merry.c
**
**	Merry, short for Meriadoc, short for Mediated Execution
**	Registry Involving A Dialect Of C. This daemon maintains
**	a cache registry of compiled LPC snippets in the form of
**	of /merry/ objects.
**
**	Initial Revision by Zell 010731
**
**	Copyright (C) Skotos Tech 2001
*/

# include <status.h>
# include <SkotOS.h>
# include <SAM.h>
# include <nref.h>
# include <base.h>


private inherit "/lib/array";
private inherit "/lib/string";
inherit parse "/lib/fileparse";

inherit "/usr/SkotOS/lib/merryapi";

# define NODE_COUNT	256

# define HALFLIFE	30

mapping tick_usage;
mapping last_update;
mapping recent_ticks;
mapping node_map;

mapping script_spaces;
int     cleanup_stamp;

/* merry code begins 5 lines into the generated LPC file */
int query_line_offset() { return 5; }

static
void create() {
   recent_ticks = ([ ]);
   last_update = ([ ]);
   tick_usage = ([ ]);

   script_spaces = ([ "udat": find_object("/usr/UserAPI/sys/udatd") ]);

   node_map = ([ ]);
   compile_object("/usr/SkotOS/data/merry");
   compile_object("/usr/SkotOS/data/mcontext");

   BASE_INITD->set_runner("merry", this_object());

   SAMD->register_root("Merry");

   set_object_name("SkotOS:Merry");

   parse :: create("/usr/SkotOS/tmp/merry", "/usr/SkotOS/grammar/merry.y");
}

void register_script_space(string space, object ob) {
   space = lower_case(space);
   if (space == "merry") {
      error("cannot register the merry script space");
   }
   if (!script_spaces) {
      script_spaces = ([ ]);
   }
   script_spaces[space] = ob;
}

void unregister_script_space(string space) {
   space = lower_case(space);
   if (space == "merry") {
      error("cannot unregister the merry script space");
   }
   if (!script_spaces) {
      script_spaces = ([ ]);
   }
   if (script_spaces[space] == nil) {
      error("no such script space registered");
   }
   script_spaces[space] = nil;
}

object query_script_space(string space) {
   space = lower_case(space);
   if (space == "merry") {
      return this_object();
   }
   if (!script_spaces) {   
      script_spaces = ([ ]);
   }
   return script_spaces[space];
}

mapping query_script_space_mapping() {
   return script_spaces;
}

void patch_sam() {
   SAMD->register_root("Merry");
   BASE_INITD->set_runner("merry", this_object());
}

void patch_parser() {
   parse :: create("/usr/SkotOS/tmp/merry", "/usr/SkotOS/grammar/merry.y");
}

void patch_runner() {
   BASE_INITD->set_runner("merry", this_object());
}

mixed eval_sam_ref(string service, string ref, object context, mapping args) {
   if (service == "merry") {
      switch(lower_case(ref)) {
      case "call":
	 if (!find_merry(args["obj"], args["script"], "lib")) {
	    return "(ERROR: no script " + dump_value(args["script"]) +
	       " on " + dump_value(args["obj"]) + ")";
	 }
	 return run_merry(args["obj"], args["script"], "lib", args);
      case "eval": {
	 int ticks;
	 object merry;
	 string lpc;
	 mixed result;

	 merry = args["node"];
	 if (!merry) {
	    lpc = args["code"];
	    if (lpc == nil) {
	       error("no code to execute in $Merry.Eval");
	    }
	    lpc = strip(lpc);
	    if (lpc[strlen(lpc)-1] != ';' && lpc[strlen(lpc)-1] != '}') {
	       /* heuristic to guess that we need to construct a 'return' */
	       lpc = "return " + lpc + ";";
	    }
	    SysLog("LPC being compiled for EVAL: " + lpc);
	    merry = new_object("/usr/SkotOS/data/merry", lpc);
	 }
	 ticks = status()[ST_TICKS];
	 result = merry->evaluate(context->query_this(), nil, nil, args, nil,
				  context->query_this() ?
				  "SAM[" + name(context->query_this()) + "]" :
				  "SAM[<nil>]");
	 catch {
	    MPROFILER->add_tick_cost(context->query_this(), "SAM", nil, ticks - status()[ST_TICKS]);
	 }
	 return result;
      }
      }
   }
   error("only $(Merry.Call) and $(Merry.Eval) served here");
}

int run_signal(object ob, string signal, NRef target, mapping args,
	       varargs object *third) {
   int i;

   if (!args) {
      args = ([ ]);
   }

   if (!target) {
      /* if there is no target, run any solo script */
      if (!run_merry(ob, signal, "solo", args + ([ "actor": ob ]))) {
	 return FALSE;
      }
      return TRUE;
   }
   /* else run any act/react scripts */
   if (!run_merry(ob, signal, "act", args +
		  ([ "actor": ob, "target": target ]))) {
      return FALSE;
   }
   if (!run_merry(NRefOb(target), signal, "react", args +
		  ([ "actor": ob, "target": target ]))) {
      return FALSE;
   }

   if (third) {
      /* finally run third party scripts */
      for (i = 0; i < sizeof(third); i ++) {

	 if (!run_merry(third[i], signal, "auto", args)) {
	    return FALSE;
	 }
      }
   }
   return TRUE;
}

int do_new_signal(object ob, string signal, string prefix,
		  mapping react_map, mapping witness_map, 
		  mapping args) {
   mapping seen;
   string *reactors, *witnesses, name;
   mixed arr;
   int i, j;

   if (!args) {
      args = ([ ]);
   }
   if (react_map) {
      /* args += react_map; */
      args["targets"] = react_map[nil];
   }
   if (witness_map) {
      /* args += witness_map; */
      args["witnesses"] = witness_map[nil];
   }
   /* args[nil] = nil; */
   args["actor"] = ob;

   /* run act */
   if (!run_merries(ob, signal,
		    prefix ? "act-" + prefix : "act",
		    args)) {
      return FALSE;
   }


   /* run reacts */

   seen = ([ ob: TRUE ]);

   if (react_map) {
      mixed val;

      reactors = map_indices(react_map);
      for (i = 0; i < sizeof(reactors); i ++) {
	 string reactor;

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
	    /* but does, with NIL -- investigate sometime */
	    continue;
	 }
	 name = reactors[i] ? signal + "-" + reactors[i] : signal;
	 for (j = 0; j < sizeof(arr); j ++) {
	    if (typeof(arr[j]) != T_OBJECT) {
	       SysLog("ERROR: args = " + dump_value(args) + "; arr = " + dump_value(arr));
	    }
	    args["target"] = arr[j];
	    seen[NRefOb(arr[j])] = TRUE;
	    if (!run_merries(NRefOb(arr[j]), name,
			     prefix ? "react-" + prefix : "react",
			     args)) {
	       arr[j] = nil;
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
   }

   /* run witnesses */
   if (witness_map) {
      witnesses = map_indices(witness_map);
      for (i = 0; i < sizeof(witnesses); i ++) {
	 arr = witness_map[witnesses[i]][1];
	 arr -= map_indices(seen);

	 name = witnesses[i] ? signal + "-" + witnesses[i] : signal;
	 for (j = 0; j < sizeof(arr); j ++) {
	    if (IsNRef(arr[j]) &&
		!run_merries(NRefOb(arr[j]), name,
			     prefix ? "witness-" + prefix : "witness",
			     args)) {
	       return FALSE;
	    }
	 }
      }
   }
   return TRUE;
}

int pre_signal(object ob, string signal, mapping react_map,
	       mapping witness_map, mapping args) {
   return do_new_signal(ob, signal, "pre", react_map, witness_map, args);
}

int new_signal(object ob, string signal, mapping react_map,
	       mapping witness_map, mapping args) {
   return do_new_signal(ob, signal, nil, react_map, witness_map, args);

}

int desc_signal(object ob, string signal, mapping react_map,
	       mapping witness_map, mapping args) {
   return do_new_signal(ob, signal, "desc", react_map, witness_map,
			copy(args));
}

int post_signal(object ob, string signal, mapping react_map,
	       mapping witness_map, mapping args) {
   return do_new_signal(ob, signal, "post", react_map, witness_map,
			copy(args));
}

int special_signal(object ob, string namespace, string signal, mapping args) {
   if (!run_merry(ob, signal, namespace, args)) {
      return FALSE;
   }
   return TRUE;
}


int post_hook(object ob, string hook, string name, mapping args) {
   run_merry(ob, name, hook + "-post", args);
}

void clear_merry_cache() {
   string *files, file, name;
   object o;
   int i;

   files = get_dir("/usr/SkotOS/merry/*.c")[0];
   for (i = 0; i < sizeof(files); i ++) {
      file = "/usr/SkotOS/merry/" + files[i];
      sscanf(file, "%s.c", name);
      if (o = find_object(name)) {
	 o->suicide();
      }
      remove_file(file);
   }
}

atomic
void new_merry_node(object program) {
   if (previous_program() == "/usr/SkotOS/data/merry") {
      node_map[program] = TRUE;
      if (map_sizeof(node_map) > 4 * NODE_COUNT) {
	 mixed *callouts;

	 callouts = status(this_object())[O_CALLOUTS];

	 if (!sizeof(callouts)) {
	    call_out("clean_nodes", 0);
	 }
      }
   }
}

atomic
void clean_nodes(varargs int all) {
   mapping sort;
   object *nodes;
   int ix, i;

   SysLog("MERRY: clearing " + (all ? "all" :
				(map_sizeof(node_map) - 3 * NODE_COUNT)) +
	  " nodes...");

   sort = ([ ]);

   nodes = map_indices(node_map) - ({ nil });

   if (!all) {
      for (i = 0; i < sizeof(nodes); i ++) {
	 ix = -nodes[i]->query_mru_stamp();
	 while (sort[ix]) {
	    ix --;
	 }
	 sort[ix] = nodes[i];
      }
      nodes = map_values(sort);
      nodes = nodes[NODE_COUNT * 3..];
   }
   for (i = 0; i < sizeof(nodes); i ++) {
      nodes[i]->suicide();

      /*
       * If clean_nodes() gets called repeatedly, don't let it start a
       * call_out() on the same object repeatedly.  We hit the callout roof
       * real fast that way.  So clean up the object in the node_map even
       * though it hasn't self-destructed yet.  We just assume it will, soon.
       */
      node_map[nodes[i]] = nil;
   }
   cleanup_stamp = time();
}

mixed **parse_merry(string str) {
   int    i;
   string *lines;
   mixed  *res, *tree;

   lines = explode(str, "\n");
   for (i = 0; i < sizeof(lines); i ++) {
      if (strlen(lines[i]) && lines[i][0] == '#') {
	 error("you can't begin a line with # in merry");
      }
   }
   res = ::parse_string(str);
   if (!res) {
      error("oops... internal merry grammar error... mail Nino the exact code you're trying to commit, please");
   }
   tree = res[0];
   if (sizeof(res) == 1) {
      /* there was a compilation error */
      int line, i;

      line = 1;
      for (i = 0; i < sizeof(tree); i ++) {
	 if (typeof(tree[i]) == T_STRING) {
	    line += (sizeof(explode("\n" + tree[i] + "\n", "\n"))-1);
	 }
      }
      error("merry parse error around line: " + line);
   }
   return res;
}

mixed *raw_parse(string str) {
   return ::parse_string(str);
}

void patch_ticks() {
   recent_ticks = ([ ]);
   last_update = ([ ]);
   tick_usage = ([ ]);
}

int Int(mixed val) {
   switch(typeof(val)) {
   case T_NIL:
      return 0;
   case T_INT:
      return val;
   case T_FLOAT:
      return (int) val;
   case T_STRING:
      if (sscanf(val, "%d", val)) {
	 return val;
      }
      break;
   }
   error("cannot convert value to integer");
}

float Flt(mixed val) {
   switch(typeof(val)) {
   case T_NIL:
      return 0.0;
   case T_INT:
      return (float) val;
   case T_FLOAT:
      return val;
   case T_STRING:
      if (sscanf(val, "%f", val)) {
	 return val;
      }
      break;
   }
   error("cannot convert value to float");
}


private
void shuffle(string id) {
   int stamp, now;

   now = time();

   stamp = Int(last_update[id]);
   if (stamp > 0 && stamp < now) {
      float dT;

      dT = (float) (now - stamp);
      tick_usage[id] = (int)
	 (pow(0.5, dT / (float) HALFLIFE) * Flt(tick_usage[id]) +
	  pow(0.5, dT / (float) HALFLIFE) * Flt(recent_ticks[id]));
      recent_ticks[id] = nil;
      if (tick_usage[id] == 0) {
	 tick_usage[id] = nil;
      }
   }
   last_update[id] = now;
}   

void update_resource(object home, string signal, string mode, string label,
		     int ticks) {
   string id;
   int stamp, now;

   if (!tick_usage) {
      patch_ticks();
   }

   id = name(home) + "/" + mode + ":" + signal + "[" + label + "]";

   shuffle(id);

   recent_ticks[id] = Int(recent_ticks[id]) + ticks/1000;
}

mapping *query_mappings() {
   return ({ tick_usage, recent_ticks, last_update });
}

mapping query_tick_usage() {
   string *ix;
   int i;

   ix = map_indices(tick_usage);
   for (i = 0; i < sizeof(ix); i ++) {
      shuffle(ix[i]);
   }
   return tick_usage;
}

mixed query_property(string prop) {
   if (prop) {
      string space;

      prop = lower_case(prop);

      if (sscanf(prop, "script-spaces:%s:handler", space) ||
	  sscanf(prop, "script-space:%s:handler", space)) {
	 return query_script_space(space);
      }
      switch(prop) {
      case "script-spaces":
	 return map_indices(query_script_space_mapping());
      case "tick-usage":
	 return query_tick_usage();
      }
   }
}



int query_method(string method) {
   switch(lower_case(method)) {
   case "register_script_space":
   case "unregister_script_space":
      return TRUE;
   }
   return FALSE;
}

mixed
call_method(string method, mapping args) {
   switch(lower_case(method)) {
   case "register_script_space":
      if (typeof(args["space"]) != T_STRING) {
	 error("register_script_space expects $space");
      }
      if (typeof(args["handler"]) != T_OBJECT) {
	 error("register_script_space expects $handler");
      }
      register_script_space(args["space"], args["handler"]);
      break;
   case "unregister_script_space":
      if (typeof(args["space"]) != T_STRING) {
	 error("unregister_script_space expects $space");
      }
      unregister_script_space(args["space"]);
      break;
   }
}

int *query_node_map_stats()
{
   return ({ map_sizeof(node_map), 4 * NODE_COUNT, cleanup_stamp });
}
