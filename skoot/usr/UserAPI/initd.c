/*
**	~UserAPI/initd.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <kernel/kernel.h>
# include <System.h>
# include <SkotOS.h>
# include <base.h>
# include <core.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/loader";

inherit sequencer "/lib/sequencer";

inherit vaultnode "/usr/SID/lib/vaultnode";

static
void create() {
   vaultnode::create("/usr/UserAPI/data/vault");

   set_object_name("UserAPI:Init");

   sequencer::create("~UserAPI/sys/authd",
		     "~UserAPI/sys/ctld",
		     "~UserAPI/sys/udatd",
		     "~UserAPI/sys/broadcast",
                     "~UserAPI/sys/authd_port",
                     "~UserAPI/sys/ctld_port");

   load_programs("/usr/UserAPI/samtags");
   load_programs("/usr/UserAPI/sys");

   claim_node("UserAPI");

   BASE_PROPERTIES->register_root_handler("udat", this_object());
   CORE_DP_REGISTRY->register_root_handler("udat", this_object());
}

void patch() {
   sequencer::create("~UserAPI/sys/authd",
		     "~UserAPI/sys/ctld",
		     "~UserAPI/sys/udatd",
		     "~UserAPI/sys/broadcast",
                     "~UserAPI/sys/authd_port",
                     "~UserAPI/sys/ctld_port");

   BASE_PROPERTIES->register_root_handler("udat", this_object());
   CORE_DP_REGISTRY->register_root_handler("udat", this_object());
}

void boot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      SysLog("Booting UserAPI...");
      call_daemons("boot", FALSE);
   }
}

static
void boot_sequence_done() {
}

void prepare_reboot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      call_daemons("prepare_reboot", FALSE);
   }
}

void reboot(int boot) {
   if (ur_name(previous_object()) == MODULED) {
      call_daemons("reboot", FALSE);
   }
}

/*
 * Play root-handler for udat:* properties.
 */

mapping
r_query_properties(object who, string root, int derived)
{
   int    guide;
   object *souls, soul, udat;

   if (root != "udat") {
      return nil;
   }
   if (!derived) {
      return nil;
   }
   souls = who->query_souls();
   if (souls) {
      souls -= ({ find_object("SkotOS:NPC-Bodies") });
      if (sizeof(souls)) {
	 soul = souls[0];
	 udat = soul->query_udat();
	 if (udat) {
	    guide = sizeof(rfilter(udat->query_bodies(),
				   "query_property", "guide"));
	 }
      }
   }
   return ([
      "name":      soul ? soul->query_name() : nil,
      "host":      soul ? soul->query_host() : FALSE,
      "idle":      soul ? time() - soul->query_idle_timestamp() : 0,
      "possessed": !!soul,
      "account":   udat ? udat->query_account_type() : nil,
      "premium":   udat ? udat->query_account_flag("premium") : FALSE,
      "grand":     udat ? udat->query_account_flag("grand") : FALSE,
      "guide":     !!guide,
      "object":    udat
   ]);
}

mixed
r_query_property(object who, string root, string prop)
{
   int    guide;
   object *souls, soul, udat;

   if (root != "udat") {
      return nil;
   }
   souls = who->query_souls();
   if (souls) {
      souls -= ({ find_object("SkotOS:NPC-Bodies") });
      if (sizeof(souls)) {
	 soul = souls[0];
	 udat = soul->query_udat();
	 if (udat) {
	    guide = sizeof(rfilter(udat->query_bodies(),
				   "query_property", "guide"));
	 }
      }
   }
   switch (prop) {
   case "name":
      return soul ? soul->query_name() : nil;
   case "host":
      return soul ? soul->query_host() : FALSE;
   case "idle":
      return soul ? time() - soul->query_idle_timestamp() : 0;
   case "possessed":
      return !!soul;
   case "account":
      return udat ? udat->query_account_type() : nil;
   case "premium":
      return udat ? udat->query_account_flag("premium") : FALSE;
   case "grand":
      return udat ? udat->query_account_flag("grand") : FALSE;
   case "guide":
      return !!guide;
   case "object":
      return udat;
   default:
      return nil;
   }
}

int
r_set_property(object who, string root, string prop, mixed value, mixed *retval)
{
   object *souls;

   if (root != "udat") {
      return FALSE;
   }
   switch (prop) {
   case "name":
   case "host":
   case "idle":
   case "possessed":
   case "account":
   case "premium":
   case "grand":
   case "guide":
   case "object":
      return TRUE;
   default:
      return FALSE;
   }
}

void
r_clear_all_properties(object who, string root)
{
   if (root != "udat") {
      return;
   }
   /* Very funny, run that by me again? */
}
