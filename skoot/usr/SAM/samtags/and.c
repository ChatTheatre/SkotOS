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
      return "Used as sub-tag for the <if> tag, to be able to build more complex conditions.";
   case "required":
      return ([ "val": "The and-condition." ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   mixed val;
   int truth, i;

   args += local;

   if (!args["if-truth"]) {
      truth = FALSE;
   } else {
      switch(typeof(val = local["val"])) {
      case T_INT:
      case T_FLOAT:
      case T_OBJECT:
	 truth = !!val;
	 break;
      case T_STRING:
	 truth = !!strlen(val);
	 break;
      case T_ARRAY:
	 truth = !!sizeof(val);
	 break;
      case T_MAPPING:
	 truth = !!map_sizeof(val);
	 break;
      }
   }
   /* TODO */
   if (query_colour(content) == COL_ELEMENT) {
      content = ({ query_colour_value(content) });
   } else {
      content = query_colour_value(content);
   }
   if (typeof(content) == T_ARRAY) {
      /* looks like it's a sequence of tags/texts.. OK.. */
      for (i = 0; i < sizeof(content); i ++) {
	 mixed content_i;

	 content_i = query_colour_value(content[i]);
	 if (typeof(content_i) == T_ARRAY) {
	    switch(xmd_element(content[i])) {
	    case "and":
	    case "or":
	       tf_elt(content[i], context, args + ([ "if-truth": truth ]));
	       return TRUE;
	    case "true":
	       if (truth) {
		  tf_elt(content[i], context, args);
		  return TRUE;
	       }
	       break;
	    case "false":
	       if (!truth) {
		  tf_elt(content[i], context, args);
		  return TRUE;
	       }
	       break;
	    default:
	       error("strange element " + content_i[0] + " in if");
	    }
	 }
      }
   }
   return TRUE;
}
