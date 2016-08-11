# include <XML.h>

private inherit "/usr/XML/lib/xmd";

inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return TRUE;
   case "category":
      return "control";
   case "description":
      return
	 "Depending on the value of the val parameter, the correct <case> " +
	 "tag is picked from the contents of the tag and inserted into the " +
	 "output.";
   case "required":
      return ([ "val": "The value by which to select." ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

private int matches(string cond, mixed val);

int tag(mixed content, object context, mapping local, mapping args) {
   mixed val;
   int truth, i;

   args += local;

   /* TODO */
   if (query_colour(content) == COL_ELEMENT) {
      content = ({ query_colour_value(content) });
   } else {
      content = query_colour_value(content);
   }
   val = local["val"];

   if (typeof(content) == T_ARRAY) {
      /* looks like it's a sequence of tags/texts.. OK.. */
      for (i = 0; i < sizeof(content); i ++) {
	 mapping map;
	 mixed cond;

	 if (typeof(query_colour_value(content[i])) == T_ARRAY &&
	     xmd_element(content[i]) == "when") {
	    cond = attributes_to_mapping(xmd_attributes(content[i]))["match"];
	    if (matches(cond, val)) {
	       tf_elt(xmd_content(content[i]), context, args);
	       return TRUE;
	    }
	 }
      }
   }
   return TRUE;
}

private int matches(string cond, mixed val) {
   int n;

   if (cond == "*") {
      return TRUE;
   }
   switch(typeof(val)) {
   case T_STRING:
      return cond == val;
   case T_NIL:
      return cond == "nil";
   case T_INT: case T_FLOAT:
      return sscanf(cond, "%d", n) && n == val;
   }
   return FALSE;
}
