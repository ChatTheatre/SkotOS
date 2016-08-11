/*
**	/usr/SAM/sys/sugar.c
**
**	Assist the XML parser in converting syntactically sweetened
**	constructs to raw XML and vice versa.
**
**	Copyright Skotos Tech Inc 1999 - 2002
**
*/

# include <type.h>
# include <XML.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/usr/XML/lib/xmd";
private inherit "/usr/XML/lib/xmlgen";
private inherit "/usr/DTD/lib/dtd";

private
string * get_parameter_names(string type) {
   switch(type) {
   case "not":
      return ({ "val" });
   case "equal":
   case "inequal":
      return ({ "left", "right" });
   case "greaterthan":
   case "lessthan":
   case "greaterthanorequal":
   case "lessthanorequal":
      return ({ "lhs", "rhs" });
   case "range":
      return ({ "val", "start", "finish" });
   }
   return nil;
}

mixed parse_specials(mixed *nuggets) {
   string type;
   int i;
   mixed nuggets0;

   nuggets0 = query_colour_value(nuggets[0]);
   /* this is truly repulsive */
   if (typeof(nuggets0) == T_STRING) {
      if (strlen(nuggets0) && nuggets0[0] == '?') {
	 type = strip(nuggets0[1 ..]);
      }
   } else {
      int ok;

      /* it might be e.g. ({ " ", "?", " ", "foo", " ", "bar" }) */

      ok = TRUE;
      for (i = 0; i < sizeof(nuggets0); i ++) {
	 ok &= (typeof(nuggets0[i]) == T_STRING);
      }
      if (ok) {
	 type = strip_left(implode(nuggets0, ""));
	 if (type[0] == '?') {
	    type = strip(type[1 ..]);
	 } else {
	    type = nil;
	 }
      }
   }
   if (type) {
      string *parnames;
      mixed predicate, *attributes;

      if (type == "when") {
	 mixed *out;

	 predicate = nuggets[1];
	 if (query_colour(predicate) == COL_PCDATA) {
	    predicate = xmd_optimize(xmd_strip_pcdata(predicate));
	 }
	 nuggets = nuggets[2 ..];
	 out = allocate(sizeof(nuggets)/2);
	 for (i = 0; i < sizeof(nuggets); i += 2) {
	    if (i+1 == sizeof(nuggets)) {
	       error("uneven number of arguments to {?when ...");
	    }
	    out[i/2] = xmd_text("when",
				({ "match", 
				      xmd_optimize(xmd_strip_pcdata(nuggets[i]))
				      }),
				nuggets[i+1]);
	 }
	 return xmd_text("case", ({ "val", predicate }), NewXMLPCData(out));
      }
      /* else it's 'if' sugar */

      if (parnames = get_parameter_names(type)) {
	 attributes = allocate(sizeof(parnames)*2);
	 for (i = 0; i < sizeof(parnames); i ++) {
	    attributes[i*2+0] = parnames[i];
	    attributes[i*2+1] = xmd_optimize(xmd_strip_pcdata(nuggets[i+1]));
	 }
	 predicate = xmd_ref(type, attributes);
	 nuggets = nuggets[i+1 ..];
      } else {
	 predicate = nuggets[1];
	 if (query_colour(predicate) == COL_PCDATA) {
	    predicate = xmd_optimize(xmd_strip_pcdata(predicate));
	 }
	 nuggets = nuggets[2 ..];
      }
      if (sizeof(nuggets) == 1) {
	 return xmd_text("if", ({ "val", predicate }),
			 xmd_text("true", ({ }), nuggets[0]));
      }
      if (sizeof(nuggets) == 2) {
	 return xmd_text("if", ({ "val", predicate }),
			 NewXMLPCData(({
			   xmd_text("true",  ({ }), nuggets[0]),
			   xmd_text("false", ({ }), nuggets[1])
			 })));
      }
      return "{? construct with wrong number of arguments (" + sizeof(nuggets) + "); Possibly \"" + type + "\" is a typo";
   }
   /* otherwise it's a straight oneof */
   for (i = 0; i < sizeof(nuggets); i ++) {
      nuggets[i] = xmd_text("z", ({ }), nuggets[i]);
   }
   return xmd_text("oneof", ({ }), nuggets);
}

string generate_specials(string tag, mixed attr, mixed content) {
   mixed tmp;
   int i;

   switch(tag) {
   case "ref":
      if (sizeof(attr) != 2 || attr[0] == "var") {
	 return nil;
      }
      return "$(" + attr[1] + ")";
   case "case": {
      string head;
      mixed *nuggets;

      if (sizeof(attr) != 2 || attr[0] != "val" ||
	  typeof(query_colour_value(content)) != T_ARRAY) {
	 return nil;
      }
      content = query_colour_value(content);
      head = "{?when| " + generate_pcdata(attr[1]);
      nuggets = allocate(sizeof(content)*2);
      for (i = 0; i < sizeof(content); i ++) {
	 if (typeof(query_colour_value(content[i])) != T_ARRAY) {
	    return nil;
	 }
	 attr = xmd_attributes(content[i]);
	 if (sizeof(attr) != 2 || attr[0] != "match") {
	    return nil;
	 }
	 nuggets[i*2+0] = generate_pcdata(attr[1]);
	 nuggets[i*2+1] = generate_pcdata(xmd_content(content[i]));
      }
      return head + " | " + implode(nuggets, " | ") + "}";
   }
   case "if": {
      mixed true, false;
      string head;

      if (sizeof(attr) != 2 || attr[0] != "val") {
	 return nil;
      }

      if (query_colour(attr[1]) == COL_SAMREF) {
	 mapping amap;
	 string type, *parnames, *nuggets;

	 type = xmd_ref_ref(attr[1]);
	 amap = attributes_to_mapping(xmd_ref_attributes(attr[1]));
	 if (parnames = get_parameter_names(type)) {
	    nuggets = allocate(sizeof(parnames));
	    for (i = 0; i < sizeof(parnames); i ++) {
	       nuggets[i] = generate_pcdata(amap[parnames[i]]);
	    }
	    head = type + " | " + implode(nuggets, " | ");
	 }
      }
      if (!head) {
	 head = "| " + generate_pcdata(attr[1]);
      }

      if (typeof(query_colour_value(content)) != T_ARRAY) {
	 /* default is truth */
	 true = content;
      } else {
	 /* look deeper */
	 content = query_colour_value(xmd_force_to_data(content));
	 if (sizeof(content) == 0 || sizeof(content) > 2) {
	    /* we don't handle this */
	    return nil;
	 }
	 if (query_colour(content[0]) != COL_ELEMENT) {
	    if (sizeof(content) > 1) {
	       return nil;
	    }
	    /* a single non-element: assume truth */
	    true = content[0];
	 } else {
	    if (xmd_element(content[0]) == "true") {
	       true = xmd_content(content[0]);
	    } else if (xmd_element(content[0]) == "false") {
	       false = xmd_content(content[0]);
	    } else {
	       return nil;
	    }
	    if (sizeof(content) > 1) {
	       if (xmd_element(content[1]) == "true") {
		  true = xmd_content(content[1]);
	       } else if (xmd_element(content[1]) == "false") {
		  false = xmd_content(content[1]);
	       } else {
		  return nil;
	       }
	    }
	 }
      }
      if (true) {
	 if (false) {
	    return "{? " + head + " |" +
	       generate_pcdata(true) + "|" +
	       generate_pcdata(false) + "}";
	 }
	 return "{? " + head + " |" + generate_pcdata(true) + "}";
      }
      if (false) {
	 return "{? " + head + " ||" + generate_pcdata(false) + "}";
      }
      return nil;
   }
   case "oneof":
      if (typeof(content) == T_ARRAY) {
	 content = query_colour_value(xmd_force_to_data(content));
	 for (i = 0; i < sizeof(content); i ++) {
	    if (typeof(query_colour_value(content[i])) == T_ARRAY &&
		xmd_element(content[i]) == "z") {
	       if (xmd_content(content[i])) {
		  content[i] = generate_pcdata(xmd_content(content[i]));
	       } else {
		  content[i] = "";
	       }
	    } else {
	       content[i] = generate_pcdata(content[i..i]);
	    }
	 }
      } else {
	 content = ({ untyped_to_ascii(content) });
      }
      return "{" + implode(content, "|") + "}";
   }
}


string generate_reference(string ref, mixed *attr) {
   if (lower_case(ref) == "merry.eval") {
      if (attr[0] == "code") {
	 return "$[" + attr[1] + "]";
      }
      if (attr[0] == "node") {
	 string lpc;

	 lpc = attr[1]->query_source();
	 if (sscanf(lpc, "return %s", lpc)) {
	    /* will end with a semicolon */
	    lpc = lpc[.. strlen(lpc)-2];
	 }
	 return "$[" + lpc + "]";
      }
   }
}

