private inherit "/lib/png/png";
private inherit "/lib/mapargs";
private inherit "/lib/url";
inherit "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return FALSE;
   case "description":
      return "Creates an image tag, referring back to a generated PNG image";
   case "required":
      return ([ "bitmap": "An array of arrays of integers providing the bitmap to render." ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   object udat;
   int scale, id;

   udat = context->query_udat();
   if (!udat) {
      error("zimage tag without udat");
   }
   args += local;
   if (args["scale"]) {
      scale = args["scale"];
   } else {
      scale = 128 / sizeof(Arg("bitmap", "zbitmap"));
   }
   id = udat->new_entry(({ construct_png(Arg("bitmap", "zbitmap"),
					 FALSE,
					 scale),
			      "image/png" }));
   Output("<img" + (args["valign"] ? " valign=" + args["valign"] : "") + " src=\"/Binary?name=" + udat->query_name() + "&__id=" + id + "\">");
   return TRUE;
}
