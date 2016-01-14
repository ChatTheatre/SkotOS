/*
 * ~Tool/initd.c
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include <System.h>
# include <HTTP.h>
# include <SAM.h>
# include "~/include/tool.h"

inherit module    "/lib/module";
inherit loader    "/lib/loader";
inherit sequencer "/lib/sequencer";

static void
create()
{
   module::create("TextIF", "HTTP");
   sequencer::create("/usr/Tool/sys/tool",
		     "/usr/Tool/sys/help",
		     "/usr/Tool/sys/parser",
		     "/usr/Tool/sys/bilbo",
		     "/usr/Tool/sys/list",
		     "/usr/Tool/sys/merry",
		     "/usr/Tool/sys/sam",
		     "/usr/Tool/sys/woe");
   load_programs("/usr/Tool/samtags");
   load_programs("/usr/Tool/sys");
   load_programs("/usr/Tool/obj");
   SAMD->register_root("Tool");
}

void
boot(int block)
{
   if (ur_name(previous_object()) == MODULED) {
      INFO("Booting Tool...");

      call_daemons("boot", FALSE);
   }
}

void
reboot(int block)
{
   if (ur_name(previous_object()) == MODULED) {
      call_daemons("reboot", FALSE);
   }
}

static void
TextIF_loaded()
{
   HOST_PARSER->register_additional("~Tool/grammar/tool.y");
   HOST_PARSER->read_grammar();
}

static void
HTTP_loaded()
{
   HTTPD->register_root("Tool");
}

void
patch()
{
   SAMD->register_root("Tool");
}

string
remap_http_request(string root, string url, mapping args)
{
   if (root == "tool") {
      return "/usr/Tool/data/www" + url;
   }
   error("Unknown URL root " + root);
}

mixed
eval_sam_ref(string service, string ref, object context, mapping args)
{
   string param;
   object udat;

   /* TODO: Redirect this to ~/sys/config.c or so */
   if (!context) {
      error("Internal error: No context");
   }
   udat = context->query_udat();
   if (!udat) {
      error("Internal error: No udat");
   }
   if (sscanf(ref, "config:%s", param) == 1) {
      mapping map;
      mixed value;

      map = "~Tool/sys/config"->query_defaults();
      if (map[param] == nil) {
	 return nil;
      }
      value = udat->query_property("tool:config:" + param);
      if (value == nil) {
	 return (string)map[param];
      }
      return (string)value;
   }
   return nil;
}
