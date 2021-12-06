/*
 * ~TextIF/sys/chatlines.c
 *
 * The collective memory of the StoryPlotter and StoryHost lines.
 *
 * Copyright Skotos Tech Inc 2001.
 */

# define MAX_RECALL 40

private inherit "/lib/string";
private inherit "/lib/date";
private inherit "/lib/url";

inherit        "/usr/TextIF/lib/misc";
inherit        "/usr/SkotOS/lib/auth";
inherit        "/usr/SkotOS/lib/describe";
inherit ctlapi "/usr/UserAPI/lib/ctlapi";
inherit        "/usr/SAM/lib/sam";

# include "/usr/SAM/include/sam.h"
# include "~SkotOS/include/describe.h"
# include <SkotOS.h>
# include <UserAPI.h>
# include <TextIF.h>

private mapping recall;

static void
create()
{
    ctlapi::create();

    recall = ([ ]);
    BROADCAST->add_listener("storyhost", this_object());
}

void patch() {
#if 0
    ctlapi::create();

    BROADCAST->add_listener("storyhost", this_object());
#endif
    BROADCAST->add_listener("coders", this_object());
}

int
check_blocked(object body, string line)
{
    mixed val;

    val = body->query_property("chatline:" + line + ":blocked");
    if (val == nil) {
	val = FALSE;
	body->set_property("chatline:" + line + ":blocked", FALSE);
    }
    return !!val;
}

/*
 * Describe one of the listeners, depending on whether we are in a Theatre
 * environment, and whether we are showing usernames or not.
 */
private string
describe_listener(object user, object body, varargs int flag)
{
    string str;

    if (map_sizeof(MODULED->query_modules() & ({ "Theatre" })) > 0) {
	if (lower_case(user->query_name()) == lower_case(describe_one(body))) {
	    str = capitalize(user->query_name());
	} else if (flag) {
	    str = capitalize(user->query_name()) + "/" + describe_one(body);
	} else {
	    str = capitalize(describe_one(body));
	}
	if (body->query_property("Theatre:ID")) {
	    str += " @ " + body->query_property("Theatre:ID");
	}
    } else if (flag) {
	str = capitalize(user->query_name()) + "/" + describe_one(body);
    } else {
	str = capitalize(describe_one(body));
    }
    if (time() > user->query_idle_timestamp()  + 3600) {
       str += " (idle)";
    }
    return str;
}

void
generic_whoquery(string line, string prop, string from, int self, string who)
{
    int     i, sz;
    object  *users;
    mapping on, off;

    users = INFOD->query_current_user_objects();
    sz    = sizeof(users);
    on    = ([ ]);
    off   = ([ ]);

    for (i = 0; i < sz; i++) {
	object user;

	user = users[i];
	if (is_root(user->query_name())) {
	    string str;
	    object obj;

	    obj = user->query_body();
	    if (!obj) {
		continue;
	    }
	    switch (line) {
	    case "storyhost":
#if 0
	       str = describe_listener(user, obj, TRUE);
#else
	       if (user->query_udat()) {
		  str = user->query_udat()->query_property("skotos:sh:secured");
	       }
	       if (!str) {
		  str = capitalize(describe_one(obj));
	       }
	       if (time() > user->query_idle_timestamp()  + 3600) {
		  str += " (idle)";
	       }
#endif
	       break;
	    case "coders":
	       str = capitalize(user->query_name());
	       if (time() > user->query_idle_timestamp()  + 3600) {
		  str += " (idle)";
	       }
	       break;
	    }
	    if (check_blocked(obj, prop)) {
		off[str] = TRUE;
	    } else {
		on[str] = TRUE;
	    }
	}
    }
    start_broadcast_one("", from, line, "whoreply",
			({ who, implode(map_indices(on), ","), implode(map_indices(off), ",") }));
}

void
channel_storyhost_whoquery(string from, int self, string who)
{
   generic_whoquery("storyhost", "sh", from, self, who);
}

void
channel_coders_whoquery(string from, int self, string who)
{
   generic_whoquery("coders", "cc", from, self, who);
}

void
generic_whoreply(string prefix, string from, int self, string who,
		 string on_data, string off_data)
{
    string *on, *off;
    object user;

    user = find_object(who);
    if (!user) {
	return;
    }
    on  = explode(on_data,  ",");
    off = explode(off_data, ",");

    if (!sizeof(on) && !sizeof(off)) {
	/* Nobody home, no point in spamming us with it. */
	return;
    }
    if (self) {
	user->message(prefix + " online:  " +
		      (sizeof(on) ? implode(on, ", ") : "None") + ".\n" +
		      prefix + " offline: " +
		      (sizeof(off) ? implode(off, ", ") : "None") + ".\n");
    } else {
	user->message(from + ":" + prefix + " online:  " +
		      (sizeof(on) ? implode(on, ", ") : "None") + ".\n" +
		      from + ":" + prefix + " offline: " +
		      (sizeof(off) ? implode(off, ", ") : "None") + ".\n");
    }
}

void
channel_storyhost_whoreply(string from, int self, string who,
			   string on_data, string off_data)
{
   generic_whoreply("StoryHosts", from, self, who, on_data, off_data);
}

void
channel_coders_whoreply(string from, int self, string who,
			string on_data, string off_data)
{
   generic_whoreply("CC", from, self, who, on_data, off_data);
}

void broadcast_locally(string line, string name, int override,
			      string str, varargs string extra);

void
local_chat(string line, string name, int override, string str)
{
   broadcast_locally(line, name, override, str);
    switch (line) {
    case "sh":
	start_broadcast_all("", "storyhost", "chat",
			    ({ name, override, str }));
	break;
    case "cc":
	start_broadcast_all("", "coders", "chat", ({ name, str }));
	break;
    case "sp":
    case "lg":
    default:
	break;
    }
}

void
channel_storyhost_chat(string from_game, int self, string name,
		       string override, string str)
{
    if (!self) {
        broadcast_locally("sh", name, (int)override, str, from_game);
    }
}

void
channel_coders_chat(string from_game, int self, string name, string str)
{
    if (!self) {
        broadcast_locally("cc", name, FALSE, str, from_game);
    }
}

void
broadcast_locally(string line, string name, int override, string str,
		  varargs string extra)
{
    object *users, obj, *alerts;
    mixed **list;
    string prefix;
    int    i, sz;

    list = recall[line];
    if (!list) {
	list = recall[line] = ({ });
    }
    while (sizeof(list) >= MAX_RECALL) {
	list = list[1..];
    }

    switch(line) {
    case "sh":
       prefix = "Hosts";
       break;
    case "sp":
       prefix = "Plotters";
       break;
    case "lg":
       prefix = "LocalGame";
       break;
    case "cc":
       prefix = "CC";
       break;
    default:
       error("unknown line");
    }

    if (extra) {
        prefix = extra + ":" + prefix;
    }

    name = capitalize(name);

    str = str[1..strlen(str) - 2];

    if (strlen(str)) {
	switch (str[0]) {
	case ':':
	    str = "[" + prefix + "] \"" + name + " " +
	      strip_left(str[1..]) + "\"\n";
	    break;
	case ';':
	    str = "[" + prefix + "] \"" + name + strip_left(str[1..]) + "\"\n";
	    break;
	default:
	    str = "[" + prefix + "|" + name + "] \"" + str + "\"\n";
	    break;
	}
    } else {
        str = "[" + prefix + "|" + name + "] \"\"\n";
    }

    recall[line] = list + ({ ({ time(), str }) });

    users = INFOD->query_current_user_objects();

    str = smalltime(time()) + " " + str;

    for (i = 0, sz = sizeof(users); i < sz; i++) {
	object user;

	user = users[i];
	switch (line) {
	case "cc":
	case "lg":
	case "sh":
	case "sp":
	    if (is_root(user->query_name())) {
		obj = user->query_body();
		if (override || (obj && !check_blocked(obj, line))) {
		   user->message(TAG(str, prefix));
		}
	    }
	    break;
	}
    }
}

void
query_wholist(string line, object user)
{
    string prefix;

    switch (line) {
    case "cc":
	start_broadcast_all("", "coders", "whoquery", ({ ur_name(user) }));
	break;
    case "sh":
	start_broadcast_all("", "storyhost", "whoquery", ({ ur_name(user) }));
	break;
    case "lg":
    case "sp": {
	int    i, sz;
	object *users;
	string *on, *off, prefix;

	users = INFOD->query_current_user_objects();
	sz    = sizeof(users);
	on    = allocate(sz);
	off   = allocate(sz);

	for (i = 0; i < sz; i++) {
	    object user;

	    user = users[i];
	    if (is_root(user->query_name())) {
		object obj;
		string str;

		obj = user->query_body();
		if (!obj) {
		    continue;
		}
		str = describe_listener(user, obj, TRUE);
		if (check_blocked(obj, line)) {
		    off[i] = str;
		} else {
		    on[i] = str;
		}
	    }
	}
	switch (line) {
	case "lg":
	   prefix = "LocalGame";
	   break;
	case "sp":
	   prefix = "StoryPlotters";
	   break;
	}
	on  -= ({ nil });
	off -= ({ nil });
	user->message(prefix + " online:  " +
		      (sizeof(on) ? implode(on, ", ") : "None") + ".\n" +
		      prefix + " offline: " +
		      (sizeof(off) ? implode(off, ", ") : "None") + ".\n");
	break;
    }
    default:
	break;
    }
}

mixed **
query_recall(string line)
{
    mixed **list;

    list = recall[line];
    if (!list) {
	return ({ });
    }
    return list[..];
}

mapping
query_chatlines(object user, object body)
{
   object *alerts;
   mapping results;

   results = ([ ]);
   if (is_root(user->query_name())) {
      results["StoryHosts"]    = check_blocked(body, "sh") ? "off" : "on";
      results["StoryPlotters"] = check_blocked(body, "sp") ? "off" : "on";
      results["LocalGame"]     = check_blocked(body, "lg") ? "off" : "on";
      results["CC"]            = check_blocked(body, "cc") ? "off" : "on";
   }

   return results;
}
