/*
**	/usr/SID/lib/statefuns.c
**
**	Some helper functions for state import and export.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <mapargs.h>
# include <type.h>
# include <SID.h>

private inherit "/usr/DTD/lib/dtd";
private inherit "/usr/XML/lib/xmd";

private mixed *export_element(object ob, object el, mapping args,
			       mapping context, int vaultflag);
private void import_element(object ob, mixed state, mapping args);

static
mixed export_state(object ob, varargs object el, mapping args,
		    mapping context, int vaultflag) {
   mixed *res;

   if (!el) {
      el = SID->get_root_node(ob);
   }
   if (!args) {
      args = ([ ]);
   }
   if (!context) {
      context = ([ ]);
   }
   args = context + args;
   res = export_element(ob, el, args, context, vaultflag);
   res = query_colour_value(res);
   if (sizeof(res) > 1) {
      error("multiple results from export_state");
   }
   if (sizeof(res) == 1) {
      return res[0];
   }
   error("no results from export_state");
}

static atomic
void import_state(object ob, mixed state, varargs mapping args) {
   import_element(ob, state, args ? args : ([ ]));
}

static
object duplicate_clone(object ob) {
   string clonable;
   mixed state;

   if (!sscanf(ur_name(ob), "%s#", clonable)) {
      error("object is not a clone");
   }
   state = export_state(ob);
   ob = clone_object(clonable);
   catch {
      import_state(ob, state);
   } : {
      ob->destruct();
      error("*");
   }
   return ob;
}

static
mixed *translate_parameters(mixed *params, mapping args) {
   int i;

   params = params[..];

   for (i = 0; i < sizeof(params); i ++) {
      if (sscanf(params[i], "#%s", params[i])) {
	 /* replace with a number if it is indeed a number */
	 if (sscanf(params[i], "%*d.%*d") == 2) {
	    sscanf(params[i], "%f", params[i]);
	 } else {
	    sscanf(params[i], "%d", params[i]);
	 }
      } else if (params[i] == "&user") {
	 params[i] = query_originator()->query_name();
      } else {
	 params[i] = args[params[i]];
      }
   }
   return params;
}

static
mixed execute_call(mixed query, object ob, mapping args) {
   return call_other(ob, query[0],
		     translate_parameters(query[1 ..], args)...);
}


static
mixed make_xml_context(mapping amap, mapping tmap) {
   string *vars;
   mixed *elts;
   int i;

   vars = map_indices(amap);

   elts = ({ });
   for (i = 0; i < sizeof(vars); i ++) {
      elts += ({
       xmd_text("iterator",
                ({ "var", vars[i],
                   "val", amap[vars[i]],
                   "type", tmap[vars[i]] }),
                nil)
      });
   }
   return xmd_text("context", ({ }), elts);
}

static
mapping parse_context_mappings(mapping amap, mapping tmap) {
   mapping res;
   string *vars;
   int i;

   res = ([ ]);

   vars = map_indices(amap);
   for (i = 0; i < sizeof(vars); i ++) {
      res[vars[i]] = ascii_to_typed(amap[vars[i]], tmap[vars[i]]);
   }
   return res;
}

static
mapping parse_xml_context(mixed state) {
   mapping args, map;
   mixed states;
   int i;

   if (xmd_element(state) != "context") {
      error("expected context");
   }
   states = query_colour_value(xmd_force_to_data(xmd_content(state)));

   args = ([ ]);

   if (states) {
      for (i = 0; i < sizeof(states); i ++) {
       if (xmd_element(states[i]) != "iterator") {
          error("expected iterator");
       }
       map = attributes_to_mapping(xmd_attributes(states[i]));
       args[map["var"]] = ascii_to_typed(map["val"], map["type"]);
      }      
   }
   return args;
}



/* internal workhorse functions */

private
mixed raw_value(string type, mapping args, object ob, mixed query...) {
   string aval;
   mixed val;

   if (sizeof(query) == 1 &&
       sscanf(query[0], "<%s>", aval)) {
      return ascii_to_typed(aval, type);
   }

   val = execute_call(query, ob, args);

   if (!test_raw_data(val, type)) {
      error("SID:: query " + query[0] + dump_value(translate_parameters(query[1 ..], args)) + " does not return " + type);
   }
   return val;
}

private
mixed *export_element(object ob, object el, mapping args,
		       mapping context, int vaultflag) {
    mapping nargs;
    string itvar, *query, *attr;
    mixed *out, *aout, *arr, *res;
    mixed tmp, itcall, itix;
    int i, j;

    /* iterative element? */

    if (itvar = el->query_iterator_variable()) {
	if (context[itvar]) {
	    args[itvar] = context[itvar];
	} else if (itcall = el->query_iterator_call()) {
	    itix = execute_call(itcall, ob, args);
	    if (typeof(itix) == T_NIL) {
		itix = ({ });
	    } else if (typeof(itix) != T_ARRAY) {
		itix = ({ itix });
	    }
	} else {
	    error("iterator variable but no iterator call");
	}
    }
    if (!itix) {
	itix = ({ nil });
    }

    attr = el->query_attributes();

    out = allocate(sizeof(itix));

    for (j = 0; j < sizeof(itix); j ++) {
	if (itvar) {
	    nargs = args + ([ itvar : itix[j] ]);
	} else {
	    nargs = args[..];
	}

	aout = allocate(sizeof(attr) * 2);
	for (i = 0; i < sizeof(attr); i ++) {
	    if (query = el->query_attribute_query(attr[i])) {
		nargs[attr[i]] =
		    raw_value(el->query_attribute_type(attr[i]),
			      nargs, ob, query...);
	    }
	    aout[i*2] = attr[i];
	    aout[i*2+1] = nargs[attr[i]];
	}
	if (el->is_parent()) {
	    res = ({ });
	    arr = el->query_children();
	    if (sizeof(arr & ({ nil }) )) {
	        SysLog("Warning: SID node " + name(el) + " is missing children.");
	        arr -= ({ nil });
	    }
	    for (i = 0; i < sizeof(arr); i ++) {
		/* if vaultflag is on we do not save transient information
		   such as e.g. UrChildren or VerbNodes etc */

		if (!vaultflag || !arr[i]->query_transient()) {
		    context = context[..];
		    res += export_element(ob, arr[i], nargs,
					  context + ([ itvar : nil ]),
					  vaultflag);
		}
	    }
	    out[j] = xmd_elts(el->query_name(), aout, res...);
	} else if (el->query_leaf()) {
	    tmp = raw_value(el->query_type(), nargs, ob,
			    el->query_leaf()...);
	    out[j] = xmd_text(el->query_name(), aout, tmp);
	} else {
	    out[j] = xmd_text(el->query_name(), aout, nil);
	}
    }
    return out;
}

private
void import_element(object ob, mixed state, mapping args) {
   mixed *def, arr, content;
   string **callbacks;
   object el;
   int i, j;

   el = SID->query_node(xmd_element(state));

   if (!el) {
      SysLog("Warning:: SID node " + xmd_element(state) + " not found!");
      return;
   }

   if (arr = xmd_attributes(state)) {
      for (i = 0; i < sizeof(arr); i += 2) {
	 args[arr[i]] = arr[i+1];
      }
   }

   callbacks = el->query_callbacks();

   content = xmd_content(state);

   for (i = 0; i < sizeof(callbacks); i ++) {
      string *nugget;

      nugget = callbacks[i];
      arr = allocate(sizeof(nugget)-1);
      for (j = 1; j < sizeof(nugget); j ++) {
	 if (nugget[j] == "CONTENT") {
	    arr[j-1] = content;
	 } else if (sscanf(nugget[j], "#%s", arr[j-1])) {
	    /* replace with a number if it is indeed a number */
	    if (sscanf(arr[j-1], "%*d.%*d") == 2) {
	       sscanf(arr[j-1], "%f", arr[j-1]);
	    } else {
	       sscanf(arr[j-1], "%d", arr[j-1]);
	    }
	 } else if (args[nugget[j]] != nil) {
	    arr[j-1] = args[nugget[j]];
	 } else {
	    arr[j-1] = el->query_default_value(nugget[j]);
	 }
      }
      call_other(ob, nugget[0], arr...);
   }
   if (el->is_parent()) {
      if (arr = query_colour_value(xmd_force_to_data(xmd_content(state)))) {
	 for (i = 0; i < sizeof(arr); i ++) {
	    import_element(ob, arr[i], args[..]);
	 }
      }
   }
}
