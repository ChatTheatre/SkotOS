private inherit "/lib/file";
inherit "/usr/SAM/lib/sam";

# include <SAM.h>
# include <HTTP.h>
# include "/usr/SAM/include/sam.h"

mixed tag_documentation(string type) {
   switch (type) {
   case "public":
      return FALSE;
   case "description":
      return "Intended for the webpages to easily include some standard section in several of them.";
   case "required":
      return ([ "page": "The URI of the file to insert into the output." ]);
   case "optional":
      return ([ ]);
   default:
      return nil;
   }
}

int tag(mixed content, object context, mapping local, mapping args) {
   string url, file;

   AssertLocal("page", "insert");

   url = local["page"];

   file = HTTPD->remap_url(url, args);
   /* trust the coder: just insert the raw data here */
   if (file && file_size(file) > 0) {
      /* TODO */
      if (path_suffix(file) == "sam") {
	 mixed sub_elts;

	 sub_elts = SAMD->get_sam_file(file);
	 if (!sub_elts) {
	    error("parse error");
	 }
	 args += local;

	 args["content"] = content;

	 XDebug("INSERTING " + file);

	 tf_elt(sub_elts, context, args);
      } else {
	 Output(read_file(file));
      }
      return TRUE;
   }
   if (args["HTML"]) {
      Output("<blink>File " + url + " not found</blink>");
   } else {
      Output("-- file " + url + " not found --");
   }
   return TRUE;
}

