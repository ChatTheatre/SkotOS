/*
 * /usr/SkotOS/sys/chatlines.c
 *
 * Copyright Skotos Tech Inc 2003.
 */

# include <type.h>
# include <UserAPI.h>
# include <SkotOS.h>

inherit      "/lib/string";
inherit      "/usr/SkotOS/lib/merryapi";
inherit prop "/lib/properties";

private mapping chatlines;

static void
create()
{
    prop::create();
    set_object_name("SkotOS:Chatlines");
    chatlines = ([ ]);
    BROADCAST->add_listener("chatline", this_object());
}

void
patch()
{
    clear_property("merry:lib:relay_who_query");
    clear_property("merry:lib:relay_who_reply");
    clear_property("merry:lib:relay_chat");
}

/*
 * Interfacing with Merry:
 */

int
query_method(string name)
{
    return !!function_object("method_" + name, this_object());
}

mixed
call_method(string name, mapping args)
{
    return call_other(this_object(), "method_" + name, args);
}

/*
 * Implementation of the actual object methods:
 */

static int
method_register(mapping args)
{
    string line;
    mixed  relay;

    line  = args["line"];
    if (!line || !strlen(line)) {
	return FALSE;
    }

    relay = args["relay"];
    switch (typeof(relay)) {
    case T_NIL:
	return FALSE;
    case T_STRING:
	relay = find_object(relay);
    case T_OBJECT:
	relay = name(relay);
	break;
    default:
	return FALSE;
    }

    chatlines[line] = ([ "relay": relay ]);
    /*
     * Optional parameters:
     */
    if (typeof(args["chat"]) == T_STRING) {
	chatlines[line]["chat"] = args["chat"];
    }
    if (typeof(args["query"]) == T_STRING) {
	chatlines[line]["query"] = args["query"];
    }
    if (typeof(args["reply"]) == T_STRING) {
	chatlines[line]["reply"] = args["reply"];
    }
    return TRUE;
}

static int
method_deregister(mapping args)
{
    string line;

    line = args["line"];
    if (!line || !strlen(line)) {
	return FALSE;
    }
    if (!chatlines[line]) {
	return FALSE;
    }
    chatlines[line] = nil;
    return TRUE;
}

static mapping
method_status(mapping args)
{
    string line;

    line = args["line"];
    if (!line || !strlen(line)) {
	return nil;
    }

    return chatlines[line] ? chatlines[line][..] : nil;
}

static string *
method_list(mapping args)
{
    return map_indices(chatlines);
}

static mixed
method_chat(mapping args)
{
    string line, name, chat;

    line = args["line"];
    if (!line || !strlen(line)) {
	return FALSE;
    }
    if (!chatlines[line]) {
	return FALSE;
    }

    name = args["name"];
    if (!name || !strlen(name)) {
	return FALSE;
    }

    chat = args["chat"];
    if (!chat || !strlen(chat)) {
	return FALSE;
    }

    BROADCAST->broadcast_all("chatline", "chat", ({ line, name, chat }));
    return TRUE;
}

static int
method_who(mapping args)
{
    string line;
    mixed  from;
    line = args["line"];
    if (!line || !strlen(line)) {
	return FALSE;
    }
    if (!chatlines[line]) {
	return FALSE;
    }

    from = args["from"];
    switch (typeof(from)) {
    case T_STRING:
       if (!strlen(from)) {
	  return FALSE;
       }
       break;
    case T_OBJECT:
       from = name(from);
       break;
    default:
       return FALSE;
    }

    BROADCAST->broadcast_all("chatline", "who_query", ({ line, from }));
    return TRUE;
}

/*
 * Implementation of the hooks for the broadcast daemon:
 */

void
channel_chatline_chat(string game, int self, string line, string name, string chat)
{
    string method;
    object relay;
    mapping data;

#if 1
    if (self) {
	return;
    }
#endif
    data = chatlines[line];
    if (!data) {
	return;
    }
    relay = find_object(data["relay"]);
    if (!relay) {
	return;
    }

    method = "relay_" + (data["chat"] ? data["chat"] : "chat");
    run_merry(relay, method, "lib", 
	      ([ "game": game,
		 "line": line,
		 "name": name,
		 "chat": chat ]));
}

void
channel_chatline_who_query(string game, int self, string line, string from)
{
    string  method, online, offline, *on, *off;
    object  relay;
    mapping data, args;
    mixed   result;

#if 1
    if (self) {
	return;
    }
#endif
    data = chatlines[line];
    if (!data) {
	return;
    }
    relay = find_object(data["relay"]);
    if (!relay) {
	return;
    }

    method = "relay_" + (data["query"] ? data["query"] : "who_query");
    args = ([ "game": game, "line": line, "from": from ]);
    run_merry(relay, method, "lib", args);

    if (typeof(args["online"]) == T_ARRAY && typeof(args["offline"]) == T_ARRAY) {
	BROADCAST->broadcast_one(game, "chatline", "who_reply",
				 ({ line, from, implode(args["online"], ","), implode(args["offline"], ",") }));
    }
}

void
channel_chatline_who_reply(string game, int self, string line, mixed from, string online, string offline)
{
    string  method, *on, *off;
    object  relay;
    mapping data;

#if 1
    if (self) {
	return;
    }
#endif
    data = chatlines[line];
    if (!data) {
	return;
    }
    relay = find_object(data["relay"]);
    if (!relay) {
	return;
    }

    if (find_object(from)) {
       from = find_object(from);
    }
    on  = explode(online, ",");
    off = explode(offline, ",");
    method = "relay_" + (data["reply"] ? data["reply"] : "who_reply");
    run_merry(relay, method, "lib", 
	      ([ "game":    game,
		 "line":    line,
		 "from":    from,
		 "online":  on,
		 "offline": off ]));
}
