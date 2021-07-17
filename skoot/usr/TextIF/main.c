/*
**	~TextIF/main.c
**
**	This is a central point for parsing text commands
**	and for describing text events that take place in
**	the virtual world.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <SAM.h>
# include <SkotOS.h>
# include <System.h>
# include <TextIF.h>
# include <UserAPI.h>
# include <XML.h>
# include <base.h>
# include <base/act.h>
# include <base/bulk.h>
# include <base/living.h>
# include <base/preps.h>
# include <base/prox.h>
# include <nref.h>
# include <status.h>
# include <type.h>

inherit "/lib/array";
inherit "/lib/date";
inherit "/lib/string";
inherit "/lib/type";
inherit "/lib/url";
private inherit "/lib/asn";

inherit "/base/lib/toolbox";
inherit "/base/lib/urcalls";

inherit sequencer "/lib/sequencer";

inherit "/usr/SID/lib/stateimpex";
inherit "/usr/SkotOS/lib/basecontext";
inherit "/usr/SkotOS/lib/traversal";
inherit "/usr/SkotOS/lib/auth";
inherit "/usr/SkotOS/lib/ursocials"; 
inherit "/usr/TextIF/lib/debug";
inherit "/usr/TextIF/lib/misc";
inherit "/usr/XML/lib/xmlparse";

inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";
inherit "/usr/SAM/lib/sam";

inherit "./lib/misc";
inherit "./lib/tell";

# include "/usr/SkotOS/include/inglish.h"
# include "/usr/SkotOS/include/noun.h"
# include "/usr/TextIF/include/tell.h"

# define PARSER_BASIC  "~TextIF/sys/parser/basic"
# define PARSER_DEBUG  "~TextIF/sys/parser/debug"
# define PARSER_GUIDE  "~TextIF/sys/parser/guide"
# define PARSER_HOST   "~TextIF/sys/parser/host"
# define PARSER_SOCIAL "~TextIF/sys/parser/social"
# define PARSER_GENERIC "~TextIF/sys/parser/generic"

mapping describers;
mapping profiling;

static
void create() {
    sequencer::create(PARSER_BASIC, PARSER_DEBUG, PARSER_GUIDE, PARSER_HOST,
		      PARSER_SOCIAL, PARSER_GENERIC,
		      "/usr/TextIF/sys/cmds/study_teach");
    describers = ([ ]);
    profiling = ([ ]);
}

void boot(int block) {
    if (ur_name(previous_object()) == "/usr/TextIF/initd") {
	call_daemons("boot", FALSE);
    }
}

mapping query_profiling()
{
   return profiling[..];
}

string
dump_profiling()
{
   int i, sz, sum, *totals;
   string *verbs, output, sum_cost;
   mapping summary, *data, costs;

   summary = ([ ]);
   costs = ([ ]);
   sz   = map_sizeof(profiling);
   data = map_values(profiling);
   for (i = 0; i < sz; i++) {
      int    j, sz_j;
      string *verbs;
      mixed  *totals;

      sz_j   = map_sizeof(data[i]);
      verbs  = map_indices(data[i]);
      totals = map_values(data[i]);
      for (j = 0; j < sz_j; j++) {
	 sum += (typeof(totals[j]) == T_INT ? totals[j] : totals[j][0]);

	 if (!summary[verbs[j]]) {
	    summary[verbs[j]] = 0;
	 }
	 summary[verbs[j]] += (typeof(totals[j]) == T_INT ? totals[j] : totals[j][0]);

	 if (!costs[verbs[j]]) {
	    costs[verbs[j]] = "\0";
	 }
	 costs[verbs[j]] = add_int64_int64(costs[verbs[j]], totals[j][1]);
      }
   }
   output = "Average commands per second: " + ((float)sum / 300.0) + "; " + sum + " commands in 5 minutes.\n";
   sz     = map_sizeof(summary);
   verbs  = map_indices(summary);
   totals = map_values(summary);
   sum_cost = "\0";
   for (i = 0; i < sz; i++) {
      sum_cost = add_int64_int64(sum_cost, costs[verbs[i]]);
      output +=
	 "- " +
	 pad_right(verbs[i] + ":", 20) + " " +
	 pad_left((string)totals[i], 3) + "; " +
	 pad_left(comma(div_int64_int32(costs[verbs[i]], totals[i])), 10) + " ticks [" +
	 pad_left((string)(totals[i] * 100 / sum), 3) + "%]\n";
   }
   output +=
      "Average ticks used per second (in commands, ignoring delayed actions): " + comma(div_int64_int32(sum_cost, 300)) + "\n";
   return output;
}

void command(object user, object body, string line, varargs int force) {
   int    i, len, slash, chop, now, ticks;
   string prefix, fun, verb, word, orig_sum, oocprefix;
   mixed  *tree, target_ob, target_obs;

   if (!body) {
      error("no body in command");
   }
   if (!body->query_environment()) {
      error("body without environment in command");
   }

   if (!force && body->query_busy()) {
      user->message("You are too busy with something else.\n");
      return;
   }

   line = strip(line);
   if (!strlen(line)) {
       return;
   }

   Debug("command() line (1): " + line);
   switch (line[0]) {
   case '!':
       /* Storyguides and storyhosts get this one. */
       if (!sizeof(rfilter(user->query_udat()->query_bodies(), "query_property", "GUIDE")) &&
	   !is_root(user->query_name())) {
	   /* And you're not a storyguide. */
	   user->message("What?\n");
	   return;
       }
       tree = PARSER_GUIDE->parse(line[1..], user, body);
       prefix = "cmd_GUIDE_";
       break;
   case '+':
       /* Storyhosts get this one. */
       if (!is_root(user->query_name())) {
	   /* Check for our honoured guests (lead storyplotters from other games, for instance) */
	   if (!(line == "+sk" || sscanf(line, "+sk %*s") ||
		 line == "+sh" || sscanf(line, "+sh %*s")) ||
	       !sizeof(rfilter(user->query_udat()->query_bodies(), "query_property", "skotos:guest"))) {
	       /* And you're not a storyhost or an honoured guest. */
	       user->message("What?\n");
	       return;
	   }
       }
       tree = PARSER_HOST->parse(line[1..], user, body);
       prefix = "cmd_DEV_";
       break;
   default:
       if (user->query_udat() && user->query_udat()->query_property("TextIF:ChatMode")) {
	  if (line[0] == '/') {
	     line = line[1 ..];
	  } else {
	     mixed foo;

	    /* extract first word */
	    for (i = 1; !word && i < strlen(line); i ++) {
	       switch(line[i]) {
	       case 'a' .. 'z':
	       case 'A' .. 'Z':
	       case '0' .. '9': case '_':
		  break;

	       default:
		  foo = locate_first(user, LF_HUSH_ALL,
				     ({ NewOb(nil, nil, 0, ({ line[.. i-1] })) }),
				     nil, body, body->query_environment());
		  if (foo && typeof(foo) == T_OBJECT) {
		     SysLog("Did find: " + line[.. i-1] + ": " + dump_value(foo));
		     word = line[.. i-1];
		     line = line[i ..];
		  }
		  break;
	       }
	    }
	    if (word) {
	       for (i = 0; !chop && i < strlen(line); i ++) {
		  switch(line[i]) {
		  case 'a' .. 'z':
		  case 'A' .. 'Z':
		  case '0' .. '9': case '_':
		    line = line[i ..];
		    chop = TRUE;
		    break;
		  default:
		    break;
		  }
	       }
	    }
	    if (line[strlen(line)-1] == '?') {
	       verb = "ask";
	    } else if (line[strlen(line)-1] == '!') {
	       verb = "exclaim";
	    } else {
	       verb = "say";
	    }
	    if (word) {
	       line = (verb == "exclaim" ? verb + " to" : verb) + " " + word + " \"" + line + "\"";
	    } else {
	       line = verb + " \"" + line + "\"";
	    }
	  }
       }
       catch {
	  if ((oocprefix = user->query_udat()->query_property("skotos:oocprefix")) && strlen(oocprefix) == 1) {
	     if (line[0] == oocprefix[0]) {
		line[0] = '@';
	     }
	  }
       }

       if (line[0] == '@') {
	 string verb;
	 object *list;

	 if (sscanf(line, "%s ", verb) < 1) {
	   verb = line;
	 }
	 switch (lower_case(verb)) {
	 case "@profile":
	 case "@help":   case "@tip":
	 case "@page":
	   list = SOCIALS->query_verb_obs(verb);
	   if (!list || !sizeof(list)) {
	     line = line[1 ..];
	   }
	   break;
	 default:
	   break;
	 }
       } else {
	 string verb;
	 object *list;

	 if (sscanf(line, "%s ", verb) < 1) {
	   verb = line;
	 }
	 switch (lower_case(verb)) {
	 case "profile":
	 case "help":   case "tip":
	 case "page":
	   list = SOCIALS->query_verb_obs(verb);
	   if (!list || !sizeof(list)) {
	     user->message("\n^^^Please use @" + verb + "^^^\n\n");
	   }
	   break;
	 default:
	   break;
	 }
       }

       /* Anyone gets anything else. */
       tree = PARSER_BASIC->parse(line, user, body);
       prefix = "cmd_";
       break;
   }

   if (!tree) {
       user->message("The command you entered was not understood by the system. Try again with different phrasing.\n");
       return;
   }

   Debug("command() line: " + line);
   Debug("command() tree: " + dump_value(tree));

   /*
    * Profiling the commands, find the hogs.
    */
   now = time();
   if (!profiling) {
      profiling = ([ ]);
   } else {
      profiling = profiling[now - 300..];
   }
   if (!profiling[now]) {
      profiling[now] = ([ ]);
   }

   if (typeof(tree[0]) == T_ARRAY) {
       verb       = tree[0][1];
       target_obs = ({ tree[0][0] });
   } else {
       verb       = tree[0];
       target_obs = ({
	  "/usr/TextIF/cmd/assist",
	     "/usr/TextIF/cmd/basic",
	     "/usr/TextIF/cmd/comm",
	     "/usr/TextIF/cmd/consent",
	     "/usr/TextIF/cmd/debug",
	     "/usr/TextIF/cmd/desc",
	     "/usr/TextIF/cmd/misc",
	     "/usr/TextIF/cmd/movement",
	     "/usr/TextIF/cmd/ownership",
	     "/usr/TextIF/cmd/profile",
	     "/usr/TextIF/cmd/social",
	     "/usr/TextIF/cmd/stance",
	     });
   }
   fun = prefix + lower_case(verb);

   for (i = 0; i < sizeof(target_obs); i ++) {
      if (typeof(target_obs[i]) == T_OBJECT) {
	 target_ob = target_obs[i];
      } else if (typeof(target_obs[i]) == T_STRING) {
	 target_ob = find_object(target_obs[i]);
      }
      if (!target_ob) {
	 continue;
      }

      if (function_object(fun, target_ob)) {
	 ticks = status()[ST_TICKS];

	 call_other(target_ob, fun, user, body, tree[1..]...);
         catch {
	   if (strlen(line) > 1 && line[0] == '+') {
	     int stamp, month;

	     stamp = time();
	     month = get_month(stamp);
	     write_file("/usr/TextIF/log/" + get_year(stamp) + "-" + (month < 10 ? "0" : "") + month + ".log",
			ctime(time()) + ":" + user->query_name() + ":" + name(body) + ":" + line + "\n");
	   }
         }                                                                                                              	 catch {
	 /*
	  * Conversion.
	  */
	 switch (typeof(profiling[now][fun])) {
	 case T_NIL:
	    profiling[now][fun] = ({ 0, "\0" });
	    break;
	 case T_INT:
	    profiling[now][fun] = ({ 1, int64_to_int32(profiling[now][fun]) });
	    break;
	 default:
	    break;
	 }
	 profiling[now][fun][0]++;
	 profiling[now][fun][1] = add_int64_int32(profiling[now][fun][1], ticks - status()[ST_TICKS]);
	 }
	 return;
      }
   }
   error("verb " + verb + " without handler");
}

void describe(string event, object *users, object body, mixed args...) {
   object ob;
   string *target_obs;
   int i;

   if (!describers[event] ||
       !function_object("desc_" + event, describers[event])) {

      SysLog("Goin' hunting for describer to [" + event + "]");

      target_obs = ({
	 "/usr/TextIF/cmd/assist",
	    "/usr/TextIF/cmd/basic",
	    "/usr/TextIF/cmd/comm",
	    "/usr/TextIF/cmd/consent",
	    "/usr/TextIF/cmd/debug",
	    "/usr/TextIF/cmd/desc",
	    "/usr/TextIF/cmd/misc",
	    "/usr/TextIF/cmd/movement",
	    "/usr/TextIF/cmd/ownership",
	    "/usr/TextIF/cmd/profile",
	    "/usr/TextIF/cmd/social",
	    "/usr/TextIF/cmd/stance",
	    });

      for (i = 0; i < sizeof(target_obs); i ++) {
	 if (ob = find_object(target_obs[i])) {
	    if (function_object("desc_" + event, ob)) {
	       SysLog("Got it: " + dump_value(ob));
	       describers[event] = ob;
	    }
	 }
      }
   }
   if (describers[event]) {
      call_other(describers[event], "desc_" + event, users, body, args...);
      return;
   }
   SysLog("failure");
}

void patch()
{
	PARSER_BASIC->destruct_parse_object();
	PARSER_DEBUG->destruct_parse_object();
	PARSER_GUIDE->destruct_parse_object();
	PARSER_HOST->destruct_parse_object();
	PARSER_SOCIAL->destruct_parse_object();
	PARSER_GENERIC->destruct_parse_object();
}
