/*
**	/usr/DevSys/lib/formstate.c
**
**	We use the SID of objects to convert an object's internal
**	state to a form suitable for SAM handling and HTML forms,
**	and vice versa.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <SID.h>

private inherit "/lib/mapping";
private inherit "/usr/XML/lib/xmd";
private inherit "/usr/SID/lib/stateimpex";
private inherit "/usr/DTD/lib/dtd";

mapping *make_form_context(mapping amap, mapping tmap) {
   mapping *out;
   string  *ix, *aval;
   int i;

   /* output data that SAM files can easily use in an HTML FORM */

   ix = map_indices(amap);
   aval = map_values(amap);

   out = allocate(map_sizeof(amap));

   for (i = 0; i < sizeof(ix); i ++) {
      out[i] = ([ "var": ix[i], "val": aval[i], "type": tmap[ix[i]] ]);
   }
   return out;
}

mapping parse_form_context(mapping vars) {
   mapping amap, tmap, args;
   string *aix, type, arg;
   mixed val;
   int i;

   /* this is dependent on the suffices used in Form.sam */
   amap = prefixed_map(vars, "conval_");
   tmap = prefixed_map(vars, "contyp_");

   aix = map_indices(amap);
   args = ([ ]);

   for (i = 0; i < sizeof(aix); i ++) {
      if (sscanf(aix[i], "conval_%s", arg)) {
	 val = amap[aix[i]];
	 if (type = tmap["contyp_" + arg]) {
	    XDebug("type of " + arg + ": " + type);
	    val = ascii_to_typed(val, type);
	 }
	 args[arg] = val;
      }
   }
   return args;
}

static
mapping state_to_form(mixed state, varargs mapping args, mapping itamap,
		      mapping ittmap, string itfix) {
   mapping vmap, amap, hmap, tmap, cmap, romap, bump, nimap, dimap;
   object el;
   string var, pfix, type, *arr, *call;
   mixed *out, *itix, *attr, *query, res;
   int i, j;

   if (!args) {
      args = ([ ]);
   }
   if (!itfix) {
      itfix = "";
   }
   if (!itamap) {
      itamap = ([ ]);
   }
   if (!ittmap) {
      ittmap = ([ ]);
   }

   el = SID->get_node(xmd_element(state));

   vmap = ([ ]);
   amap = ([ ]);
   hmap = ([ ]);
   tmap = ([ ]);
   cmap = ([ ]);
   romap = ([ ]);
   nimap = ([ ]);
   dimap = ([ ]);

   attr = xmd_attributes(state);
   arr = ({ });

   for (i = 0; i < sizeof(attr); i += 2) {
      type = el->query_attribute_type(attr[i]);
      args[attr[i]] = attr[i+1];
      vmap[attr[i]] = attr[i+1];
      amap[attr[i]] = typed_to_ascii(attr[i+1], type);
      hmap[attr[i]] = typed_to_html(attr[i+1], type);
      tmap[attr[i]] = type;
      cmap[attr[i]] = el->query_attribute_comment(attr[i]);
      romap[attr[i]] = el->query_attribute_readonly(attr[i]) ? TRUE: nil;
      arr += attr[i..i];
   }
   if (call = el->query_newitem()) {
      nimap = ([ nil: TRUE ]);	/* basically a flag */

      for (j = 1; j < sizeof(call); j ++) {
	 if (call[j] && !sscanf(call[j], "#%*s")) {
	    nimap[call[j]] = args[call[j]];
	 }
      }
   }
   if (call = el->query_delitem()) {
      dimap = ([ nil: TRUE ]);	/* basically a flag */

      for (j = 1; j < sizeof(call); j ++) {
	 if (call[j] && !sscanf(call[j], "#%*s")) {
	    dimap[call[j]] = args[call[j]];
	 }
      }
   }
   bump = ([
      "asciimap":	amap,
      "htmlmap":	hmap,
      "valuemap":	vmap,
      "typemap":	tmap,
      "commentmap":	cmap,
      "readonlymap":	romap,
      "prefix":		itfix,
      "valuearr":	arr,
      "element":	el->query_name(),
      "comment":	el->query_comment(),
      "newitemmap":	nimap,
      "delitemmap":	dimap,
      "iterative":	!!el->query_iterator_variable(),
      ]);
   if (var = el->query_iterator_variable()) {
     itamap += ([ var : amap[var] ]);
     ittmap += ([ var : tmap[var] ]);
   }
   bump["itamap"] = itamap;
   bump["ittmap"] = ittmap;

   if (el->is_parent()) {
      if (el->query_recursion_point() && strlen(itfix)) {
	 bump["recpoint"] = TRUE;
      }
      res = ({ });
      state = query_colour_value(xmd_content(state));
      for (i = 0; i < sizeof(state); i ++) {
	 pfix = itfix + (i+1) + "_";
	 res += ({ state_to_form(state[i], args, itamap, ittmap, pfix) });
      }
      bump["children"] = res;
   } else if (el->is_leaf()) {
      bump["content"] = typed_to_ascii(xmd_content(state), el->query_type());
      bump["html"] = typed_to_html(xmd_content(state), el->query_type());
      bump["contentholder"] = TRUE;
   }
   return bump;
}

/* static TODO */
mapping form_to_state(mapping bump, varargs mapping args) {
   mapping children, local, map;
   string *ix, **callbacks, rest;
   object el;
   mixed *arr, *attr, content, **calls, *call;
   int i, j, num, di, ni;

   if (!bump["element"]) {
      error("no element");
   }
   el = SID->get_node(bump["element"]);

   calls = ({ });

   children = ([ ]);
   local = ([ ]);

   ix = map_indices(bump);

   ni = (typeof(bump["newitem.x"]) != T_NIL);
   di = (typeof(bump["delitem.x"]) != T_NIL);

   /* may HTML burn in hell for a thousand thousand years */
   bump["newitem.x"] = bump["newitem.y"] =
      bump["delitem.x"] = bump["delitem.y"] = nil;

   for (i = 0; i < sizeof(ix); i ++) {
      if (sscanf(ix[i], "%d_%s", num, rest)) {
	 if (!children[num]) {
	    children[num] = ([ ]);
	 }
	 children[num][rest] = bump[ix[i]];
      } else {
	 local[ix[i]] = bump[ix[i]];
      }
   }

   if (args) {
      args += local;
   } else {
      args  = local;
   }

   if (ni) {
      if (call = el->query_newitem()) {
	 XDebug("collecting newitem call " + dump_value(call));
	 calls += ({ ({ call[0] }) + translate_parameters(call[1 ..], args) });
      } else {
	 error("bump[NI] but no newitem");
      }
   }

   if (di) {
      if (call = el->query_delitem()) {
	 XDebug("collecting delitem call " + dump_value(call));
	 calls += ({ ({ call[0] }) + translate_parameters(call[1 ..], args) });
      } else {
	 error("bump[DI] but no delitem");
      }
   }


   arr = el->query_attributes();
   attr = ({ });

   for (i = 0; i < sizeof(arr); i ++) {
      mixed val;
      string type;

      val = local[arr[i]];
      if (typeof(val) == T_STRING && strlen(val)) {
	 string type;

	 type = el->query_attribute_type(arr[i]);
	 attr += ({ arr[i], ascii_to_typed(val, type) });
      }
   }

   XDebug("Element:" + dump_value(el));
   XDebug("Children: " + dump_value(children));
   XDebug("Local arguments: " + dump_value(local));

   if (el->is_parent()) {
      if (bump["content"]) {
	 error("unexpected content of parent element '" + el->query_name() + "'");
      }
      arr = map_indices(children);
      for (i = 0; i < sizeof(arr); i ++) {
	 map = form_to_state(children[arr[i]], args);
	 calls += map["calls"];
	 arr[i] = map["state"];
      }
      return ([
	 "calls": calls,
	 "state": xmd_elts(el->query_name(), attr, arr...)
	 ]);
   }
   if (map_sizeof(children)) {
      error("unexpected child nodes of content element '" + el->query_name() + "'");
   }
   if (bump["content"]) {
      content = ascii_to_typed(bump["content"], el->query_type());
   }
   return ([
      "calls": calls,
      /* TODO: later, xmd_text will be xmd_leaf or so */
      "state": xmd_text(el->query_name(), attr, content),
      ]);
}
