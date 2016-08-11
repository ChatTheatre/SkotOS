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
      return "This tag has the effect of taking the sub-tags, picking one randomly and inserting its contents after evaluating it, while ignoring comment-tags in the random selection.";
   case "required":
      return ([ ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}
int tag(mixed content, object context, mapping local, mapping args) {
   int i;

   if (typeof(content) != T_ARRAY) {
      tf_elt(content, context, args);
      return TRUE;
   }
   content = xmd_force_to_data(content);
   content = xmd_wipe_tags(content, "comment");
   if (sizeof(content) > 0) {
      tf_elt(content[random(sizeof(content))], context, args);
   }
   return TRUE;
}
