/*
**	Helpful Merry functions.
**
*/

# include <status.h>
# include <type.h>

private inherit "/lib/string";
private inherit "/base/lib/urcalls";


static
object find_merry(object ob, string signal, string mode) {
   string dprop, iprop;

   signal = lower_case(signal);
   mode = lower_case(mode);

   dprop = "merry:" + mode + ":" + signal;
   iprop = "merry:inherit:" + mode + ":" + signal;

   while (ob) {
      mixed code, new;

      code = ob->query_raw_property(dprop);
      if (typeof(code) == T_OBJECT) {
	 return code;
      }
      code = nil;
      new = ob->query_raw_property(iprop);
      while (typeof(new) == T_OBJECT) {
	 code = new;
	 new = code->query_raw_property(iprop);
      }
      if (typeof(code) == T_OBJECT) {
	 new = code->query_raw_property(dprop);
	 if (typeof(new) == T_OBJECT) {
	    return new;
	 }
      }
      ob = ob->query_ur_object();
   }
   return nil;
}

/*
**	This function is used by code that wants to deal with the
**	location of a specific merry script, rather than the script
**	itself.
*/

static
object find_merry_location(object ob, string signal, string mode) {
   string dprop, iprop;

   signal = lower_case(signal);
   mode = lower_case(mode);

   dprop = "merry:" + mode + ":" + signal;
   iprop = "merry:inherit:" + mode + ":" + signal;

   while (ob) {
      mixed code, new;

      code = ob->query_raw_property(dprop);
      if (typeof(code) == T_OBJECT) {
	 return ob;
      }
      code = nil;
      new = ob->query_raw_property(iprop);
      while (typeof(new) == T_OBJECT) {
	 code = new;
	 new = code->query_raw_property(iprop);
      }
      if (typeof(code) == T_OBJECT) {
	 new = code->query_raw_property(dprop);
	 if (typeof(new) == T_OBJECT) {
	    return code;
	 }
      }
      ob = ob->query_ur_object();
   }
   return nil;
}

static
mixed run_merry(object ob, string signal, string mode, mapping args,
		varargs string label) {
   object code;

   signal = lower_case(signal);
   mode = lower_case(mode);

   if (code = find_merry(ob, signal, mode)) {
      int ticks;
      mixed result;

      ticks = status()[ST_TICKS];
      result = code->evaluate(ob, signal, mode, args, label, name(ob) + "/" + mode + ":" + signal);
      "~SkotOS/sys/profiler"->add_tick_cost(ob, "merry:" + mode + ":" + signal, label, ticks - status()[ST_TICKS]);
      return result;
   }
   return TRUE;
}


static
mapping find_merries(object ob, string signal, string mode) {
   mapping imap;
   string prop, *ix;
   string dprop, iprop;
   mixed code, new;
   object *ancestry;
   mapping out;
   int i, j;

   signal = lower_case(signal);
   mode = lower_case(mode);

   dprop = "merry:" + mode + ":" + signal;
   iprop = "merry:inherit:" + mode + ":" + signal;

   out = ([ ]);

   ancestry = ({ });
   for (new = ob; new; new = new->query_ur_object()) {
      ancestry = ({ new }) + ancestry;
   }

   for (i = 0; i < sizeof(ancestry); i ++) {
      mixed val;

      ob = ancestry[i];

      val = ob->query_raw_property(dprop);
      if (typeof(val) == T_OBJECT) {
	 out[dprop] = val;
      }
      out += ob->query_prefixed_properties(dprop + "%");

      imap = ob->query_prefixed_properties(iprop + "%");
      imap[iprop] = ob->query_raw_property(iprop);

      ix = map_indices(imap);

      for (j = 0; j < sizeof(ix); j ++) {
	 code = nil;
	 new = imap[ix[j]];
	 while (typeof(new) == T_OBJECT) {
	    code = new;
	    new = code->query_raw_property(ix[j]);
	 }
	 if (typeof(code) == T_OBJECT) {
	    /* turn merry:inherit:foo into merry:foo */
	    prop = "merry:" + ix[j][14 ..];
	    new = code->query_raw_property(prop);
	    if (typeof(new) == T_OBJECT) {
	       out[prop] = new;
	    }
	 }
      }
   }
   return out;
}

static
mixed run_merries(object ob, string signal, string mode, mapping args,
		  varargs string label) {
   object *codes;
   int ret, i, sz;
   string *scripts;
   mapping map;

   signal = lower_case(signal);
   mode = lower_case(mode);

   map = find_merries(ob, signal, mode);
   sz = map_sizeof(map);
   codes = map_values(map);
   scripts = map_indices(map);

   ret = TRUE;
   for (i = 0; i < sz; i ++) {
      sscanf(scripts[i], "merry:%s:%s", mode, signal);
      ret &= !!codes[i]->evaluate(ob, signal, mode, args, label,
				  name(ob) + "/" + mode + ":" + signal);
   }
   return ret;
}


static
string categorize_merry_word(string word) {
   switch(lower_case(word)) {
   case "float":
   case "int":
   case "mapping":
   case "mixed":
   case "object":
   case "string":
   case "void":
      return "type";
   case "atomic":
   case "break":
   case "case":
   case "catch":
   case "continue":
   case "default":
   case "do":
   case "else":
   case "for":
   case "if":
   case "inherit":
   case "nil":
   case "nomask":
   case "private":
   case "return":
   case "rlimits":
   case "static":
   case "switch":
   case "varargs":
   case "while":
      return "keyword";
   case "Act":
   case "Call":
   case "Describe":
   case "EmitIn":
   case "EmitTo":
   case "Error":
   case "Every":
   case "Get":
   case "In":
   case "Set":
   case "SAM":
   case "Slay":
   case "Spawn":
   case "Stop":
      return "merryfun";
   case "acos":
   case "allocate":
   case "allocate_float":
   case "allocate_int":
   case "asin":
   case "atan":
   case "atan2":
   case "call_trace":
   case "ceil":
   case "cos":
   case "cosh":
   case "crypt":
   case "ctime":
   case "error":
   case "exp":
   case "explode":
   case "fabs":
   case "find_object":
   case "floor":
   case "fmod":
   case "frexp":
   case "function_object":
   case "implode":
   case "ldexp":
   case "log":
   case "log10":
   case "map_indices":
   case "map_sizeof":
   case "map_values":
   case "millitime":
   case "modf":
   case "object_name":
   case "paint_value":
   case "parse_string":
   case "pow":
   case "previous_object":
   case "previous_program":
   case "query_colour":
   case "query_editor":
   case "query_ip_name":
   case "query_ip_number":
   case "random":
   case "sin":
   case "sinh":
   case "sizeof":
   case "sqrt":
   case "sscanf":
   case "status":
   case "strlen":
   case "tan":
   case "tanh":
   case "this_object":
   case "time":
   case "typeof":
      return "kfun";
   }
   return nil;
}
