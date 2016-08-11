/*
 * ~Tool/lib/generic.c
 *
 * Some utilities that don't fit the other categories at this point.
 *
 * Copyright Skotos Tech Inc 2002.
 */

# include <status.h>

# include "~/include/tool.h"
# include <TextIF.h>

private inherit "/lib/string";
private inherit "/lib/url";
        inherit "utility";
        inherit "/usr/SkotOS/lib/describe";
        inherit "/usr/SkotOS/lib/merryapi";

void
cmd_quote(object user, object body, varargs string arg)
{
   int    i, sz;
   object user_to, *users;

   if (!arg) {
      prompt_to(user, "End your text with a single period.\n",
		"input_quote", "");
      return;
   }
   arg = lower_case(arg);
   users = INFOD->query_current_user_objects();
   sz = sizeof(users);
   for (i = 0; i < sz; i++) {
      if (users[i]->query_name() == arg) {
	 prompt_to(user, "End your text to " + capitalize(arg) +
		   " with a single period.\n", "input_quote", "",
		   arg, users[i]);
	 return;
      }
   }
   user->message("Could not find " + capitalize(arg) + "!\n");
}

void
cmd_quote_string(object user, object body, string text)
{
   int    i, sz;
   object *users, *list, *inv;

   users = INFOD->query_current_user_objects();
   inv = body->query_environment()->query_inventory();
   sz = sizeof(inv);
   list = ({ });
   for (i = 0; i < sz; i++) {
      list += inv[i]->query_souls();
   }
   list = (list & users) - ({ user });
   sz = sizeof(list);
   for (i = 0; i < sz; i++) {
      list[i]->message(capitalize(describe_one(body)) + " quotes:\n" +
		       "  " + text + "\n");
   }
   user->message("You quote:\n" +
		 "  " + text + "\n");
}

static void
input_quote(object user, object body, string arg, string text,
	    varargs string name_to, object user_to)
{
   if (arg == ".") {
      if (name_to && !user_to) {
	 user->message("It looks like " + capitalize(name_to) +
		       " has disappeared.\n");
	 return;
      }
      if (name_to) {
	 /* user_to must be there, otherwise we wouldn't get here. */
	 pre_message(user_to,
  "--- Quoted text from " + describe_one(body) + " to you: ---\n" +
  text +
  "--- Quoted text ends ---\n");
	 user->message("Ok.\n");
      } else {
	 int    i, sz;
	 object *users, *list, *inv;

	 users = INFOD->query_current_user_objects();
	 inv = body->query_environment()->query_inventory();
	 sz = sizeof(inv);
	 list = ({ });
	 for (i = 0; i < sz; i++) {
	    list += inv[i]->query_souls();
	 }
	 list = (list & users) - ({ user });
	 sz = sizeof(list);
	 for (i = 0; i < sz; i++) {
	    pre_message(list[i],
  "--- Quoted text from " + describe_one(body) + ": ---\n" +
  text +
  "--- Quoted text ends ---\n");
	 }
	 pre_message(user, "Ok.\n");
      }
      return;
   }
   prompt_to(user, "", "input_quote", text + arg + "\n", name_to, user_to);
}

void
cmd_ide(object user, object body, string obj)
{
   object target;

   if (!obj) {
      user->message("Usage: +tool ide %<object>\n");
      return;
   }
   target = interpret_argument(user, body, obj);
   if (!target) {
      user->message("+tool ide: Could not resolve \"" + obj + "\".\n");
      return;
   }
   if (user->query_skotos_client()) {
      user->message("Creating IDE for " + obj + " in popup window.\n");
      send_popup(user, "/Tool/IDE.sam?obj=" + url_encode(name(target)));
      user->message("\n");
   } else {
      user->message("+tool ide: Cannot create a popup window with your client.\n");
   }
}

void
patch_tool_lib_popup()
{
   object popup;

   popup = find_object("Tool:Lib:Popup");
   if (popup->query_property("html:go")) {
      return;
   }
   popup->set_property("html:go", ascii_to_mixed("\
X[S] \
<z uname=\"$(UDat.Name)\" uhost=\"$(UDat.Host)\" udat=\"$(UDat.dat)\">\
   $[$update_obj = $obj; $update_prop = $prop; ::editor_update();] $(this.html:propedit)\
</z>"));
}

void
cmd_popup_object(object user, object body, string obname)
{
   if (user->query_skotos_client()) {
      object target;

      patch_tool_lib_popup();
      target = interpret_argument(user, body, obname);
      if (!target) {
	 user->message("+tool popup: Could not resolve \"" + obname + "\".\n");
	 return;
      }
      user->message("Creating a popup window.\n");
      body->action("client_control",
		   ([ "id": ANYURL,
 		      "val": url_absolute(make_url("/SAM/Prop/" +
						   url_encode("Tool:Lib:Popup") + "/html:go",
						   ([ "body": body, "obj": target ]), TRUE)) ]));
      user->message("\n");
   } else {
      user->message("+tool popup: Cannot create a popup window with your client.\n");
   }
}

void
cmd_popup_property(object user, object body, string obname, string prop)
{
   if (user->query_skotos_client()) {
      object target;

      patch_tool_lib_popup();
      target = interpret_argument(user, body, obname);
      if (!target) {
	 user->message("+tool popup: Could not resolve \"" + obname + "\".\n");
	 return;
      }
      user->message("Creating a popup window.\n");
      body->action("client_control",
		   ([ "id": ANYURL,
 		      "val": url_absolute(make_url("/SAM/Prop/" +
						   url_encode("Tool:Lib:Popup") + "/html:go",
						   ([ "body": body, "obj": target, "prop": prop ]), TRUE)) ]));
      user->message("\n");
   } else {
      user->message("+tool popup: Cannot create a popup window with your client.\n");
   }
}

void
cmd_popup(object user, object body)
{
   if (user->query_skotos_client()) {
      user->message("Creating a popup window.\n");
      body->action("client_control",
		   ([ "id": ANYURL,
 		      "val": url_absolute(make_url("/SAM/Prop/" +
						   url_encode("Tool:Lib:Popup") + "/html:index",
						   ([ "body": body ]), TRUE)) ]));
      user->message("\n");
   } else {
      user->message("+tool popup: Cannot create a popup window with your client.\n");
   }
}

void
cmd_profiler_info(object user, object body)
{
}

void
cmd_profiler_textif(object user, object body)
{
   pre_message(user, "~TextIF/main"->dump_profiling());
}

void
cmd_profiler_merry(object user, object body, varargs mixed minimum, mixed type)
{
   minimum = minimum ? (int)minimum : 2500000;
   type = type ? "daily" : "hourly";

   pre_message(user, "~SkotOS/sys/profiler"->dump_profile(type, minimum));
}

void
cmd_profiler_object(object user, object body, string obj, varargs mixed type)
{
   object target;

   target = interpret_argument(user, body, obj);
   if (!target) {
      user->message("+tool profiler: Could not resolve \"" + obj + "\".\n");
      return;
   }
   type = type ? "daily" : "hourly";
   pre_message(user, "~SkotOS/sys/profiler"->dump_profile(type, 1, name(target)));
}

void
cmd_profiler_archives(object user, object body, varargs mixed type)
{
   int i, sz;
   mixed *stamps;

   type = type ? "daily" : "hourly";
   /* Use the Merry methods, for convenience. */
   stamps = "~SkotOS/sys/profiler"->call_method("archives", ([ "type": type ]));
   sz = sizeof(stamps);
   for (i = 0; i < sz; i++) {
      stamps[i] = (i + 1) + ". " + ctime(stamps[i]) + "\n";
   }
   user->message("The following " + capitalize(type) + " Profiler summaries are available in the archive:\n" +
		 implode(stamps, "") +
		 "You can review one by doing \"+tool profiler merry " + (type == "daily" ? "daily " : "") + "archive <index>\".\n");
}

void
cmd_profiler_archive(object user, object body, mixed index, varargs mixed type)
{
   string archive;

   index = (int)index;
   type = type ? "daily" : "hourly";
   archive = "~SkotOS/sys/profiler"->call_method("archive", ([ "index": index - 1, "type": type ]));
   if (!archive) {
      user->message("+tool: Archive index out of range.\n");
      return;
   }
   pre_message(user, archive);
}

# define SPACE16 "                "

static string pad_left(mixed var, int size)
{
   var = ::pad_left((string)var, size);
   if (strlen(var) > size) {
      var = var[strlen(var) - size..];
   }
   return var;
}

private string swapnum(int num, int div)
{
    string str;

    str = (string) ((float) num / (float) div);
    str += (sscanf(str, "%*s.") != 0) ? "00" : ".00";
    if (strlen(str) > 4) {
	str = (str[3] == '.') ? str[.. 2] : str[.. 3];
    }
    return str;
}

# define MB (1024 * 1024)

void
cmd_status(object user, object body)
{
   int uptime, seconds, minutes, hours, short, long, i;
   string str;
   mixed *status;

   status = status();
   str =
"                                          Server:       " +
  (string) status[ST_VERSION] + "\n" +
"------------ Swap device -------------\n" +
"sectors:  " + pad_left(comma(status[ST_SWAPUSED]), 9) + " / " +
	       pad_left(comma(status[ST_SWAPSIZE]), 9) + " (" +
  pad_left((int) status[ST_SWAPUSED] * 100 / (int) status[ST_SWAPSIZE], 3) +
  "%)    Start time:   " + ctime(status[ST_STARTTIME])[4 ..] + "\n" +
"sector size:   " + (((float) status[ST_SECTORSIZE] / 1024.0) + " kB" +
		     SPACE16)[..15];
   if ((int) status[ST_STARTTIME] != (int) status[ST_BOOTTIME]) {
      str += "           Last reboot:  " +
	 ctime(status[ST_BOOTTIME])[4 ..];
   }

   uptime = status[ST_UPTIME];
   seconds = uptime % 60;
   uptime /= 60;
   minutes = uptime % 60;
   uptime /= 60;
   hours = uptime % 24;
   uptime /= 24;
   short = status[ST_NCOSHORT];
   long = status[ST_NCOLONG];
   str += "\n" +
      "swap average:  " + (swapnum(status[ST_SWAPRATE1], 60) + ", " +
			   swapnum(status[ST_SWAPRATE5], 300) + SPACE16)[.. 15] +
      "           Uptime:       " +
      ((uptime == 0) ? "" : uptime + ((uptime == 1) ? " day, " : " days, ")) +
      pad_left("00" + hours, 2) + ":" + pad_left("00" + minutes, 2) + ":" +
      pad_left("00" + seconds, 2) + "\n\n" +
      "--------------- Memory ---------------" +
      "    ------------ Callouts ------------\n" +
      "static:   " + pad_left(comma((status[ST_SMEMUSED] + MB - 1) / MB) + " MB", 9) + " / " +
                     pad_left(comma((status[ST_SMEMSIZE] + MB - 1) / MB) + " MB", 9) + " (" +
                     pad_left((int) ((float) status[ST_SMEMUSED] * 100.0 /
				   (float) status[ST_SMEMSIZE]), 3) +
      "%)    short term:   " + pad_left(short, 5) + "         (" +
      ((short + long == 0) ? "  0" : pad_left(100 - long * 100 / (short + long), 3)) +
      "%)\n" +
      "dynamic:  " + pad_left(comma((status[ST_DMEMUSED] + MB - 1) / MB) + " MB", 9) + " / " +
                     pad_left(comma((status[ST_DMEMSIZE] + MB - 1) / MB) + " MB", 9) + " (" +
                     pad_left((int) ((float) status[ST_DMEMUSED] * 100.0 /
				   (float) status[ST_DMEMSIZE]), 3) +
      "%) +  other:        " + pad_left(long, 5) + "         (" +
      ((short + long == 0) ? "  0" : pad_left(long * 100 / (short + long), 3)) +
      "%) +\n" +
      "          " +
      pad_left(comma((status[ST_SMEMUSED] + status[ST_DMEMUSED] + MB - 1) / MB) + " MB", 9) + " / " +
      pad_left(comma((status[ST_SMEMSIZE] + status[ST_DMEMSIZE] + MB - 1) / MB) + " MB", 9) + " (" +
      pad_left((int) (((float) status[ST_SMEMUSED] +
		     (float) status[ST_DMEMUSED]) * 100.0 /
		    ((float) status[ST_SMEMSIZE] +
		     (float) status[ST_DMEMSIZE])), 3) +
      "%)                  " + pad_left(short + long, 5) + " / " +
      pad_left(status[ST_COTABSIZE], 5) + " (" +
      pad_left((short + long) * 100 / (int) status[ST_COTABSIZE], 3) + "%)\n\n" +
      "Objects:  " + pad_left(comma(status[ST_NOBJECTS]), 9) + " / " +
      pad_left(comma(status[ST_OTABSIZE]), 9) + " (" +
      pad_left((int) status[ST_NOBJECTS] * 100 / (int) status[ST_OTABSIZE], 3) +
      "%)\n";
   pre_message(user, str);
}

void
cmd_status_object(object user, object body, string obj)
{
   object target;
   string str, name;
   mixed *status, *calls;

   target = interpret_argument(user, body, obj);
   if (!target) {
      user->message("+tool status: Could not resolve \"" + obj + "\".\n");
      return;
   }
   name = ur_name(target);
   sscanf(name, "%s#", name);
   switch (name) {
   case "/base/obj/thing":
      name = "Virtual World";
      break;
   case "/obj/properties":
      name = "Property Container";
      break;
   case "/usr/SkotOS/obj/verb":
      name = "Verb Object";
      break;
   case "/usr/SkotOS/obj/meracha":
      name = "Merry Action Handler";
      break;
   default:
      name = "Unknown";
      break;
   }
   status = status(target);

   calls = target->query_delayed_calls();
   if (!calls) {
      calls = ({ });
   }
   str = 
      "Object:         " + name(target) + "\n" +
      "Compiled at:    " + ctime(status[O_COMPILETIME])[4 ..] + "\n" +
      "Size:           " + comma(status[O_NSECTORS] * status()[ST_SECTORSIZE]) + " bytes (in " + status[O_NSECTORS] + " sectors).\n" +
      "Delays running: " + sizeof(calls) + "\n" +
      (sizeof(calls) > 0 ? "                " + implode(calls, "\n                ") + "\n" : "") +
      "Type:           " + name + "\n";
   pre_message(user, str);
}

void cmd_profiler_socials(object user, object body, mixed type, varargs mixed threshold)
{
   type = ({ "running", "hourly", "daily" })[type];
   threshold = threshold ? (int)threshold : 2500000;

   pre_message(user, "~TextIF/sys/profiler"->dump_profile(type, threshold));
}

void cmd_convert(object user, object body, varargs string arg)
{
    object command;

    command = find_merry(find_object("Tool:Lib:Generic"), "convert", "command");
    if (!command) {
	user->message("+tool convert: Not yet implemented.\n");
	return;
    }
    run_merry(find_object("Tool:Lib:Generic"), "convert", "command", ([ "actor": body, "line": arg ]));
}
