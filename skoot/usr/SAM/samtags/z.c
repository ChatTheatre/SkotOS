inherit "/usr/SAM/lib/sam";

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return FALSE;
   case "description":
      return "Undocumented at the moment.";
   case "required":
      return ([ ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   tf_elt(content, context, args);
   return TRUE;
}
