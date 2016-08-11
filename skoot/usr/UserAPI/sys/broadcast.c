/*
 * ~UserAPI/sys/broadcast.c
 *
 * The receiving end of ~UserDB/sys/broadcast.c, provides hooks for objects so
 * that they can claim to listen to certain channels and be served up with any
 * data that is sent to those channels on whatever sub-channel.
 *
 * Copyright 2001 Skotos Tech Inc.
 */

# define PORT_OFFSET 17

inherit           "/lib/url";
inherit sequencer "/lib/sequencer";
inherit lib_port  "/usr/System/lib/port";

/*
 * Variables:
 */

private mapping listeners;

/*
 * Implementation:
 */

static void
create()
{
    sequencer::create("~UserAPI/sys/broadcast_tcp");
    lib_port::create();

    listeners = ([ ]);
}

void
patch()
{
    sequencer::create("~UserAPI/sys/broadcast_tcp");
}

void
boot(int boot)
{
    call_daemons("boot", FALSE);
}

void
reboot(int block)
{
    boot(block);
}

void
add_listener(string channel, object who)
{
    mapping map;

    map = listeners[channel];
    if (!map) {
	listeners[channel] = map = ([ ]);
    }
    map[who] = TRUE;
}

void
remove_listener(string channel, object who)
{
    listeners[channel][who] = nil;
    if (map_sizeof(listeners[channel]) == 0) {
	listeners[channel] = nil;
    }
}

object *
query_listeners(string channel)
{
    mapping map;

    map = listeners[channel];
    return map ? map_indices(map) : ({ });
}

static void
recv_datagram(string gram, string host, int port)
{
    int    i, sz;
    string *data, orig_self, orig_name, channel, sub, *args;
    object *list;

    data = explode(":" + gram + ":", ":");
    if (sizeof(data) < 4) {
        error("Bad datagram");
    }
    for (i = 0, sz = sizeof(data); i < sz; i++) {
        data[i] = url_decode(data[i]);
    }
    orig_self = data[0];
    orig_name = data[1];
    channel   = data[2];
    sub       = data[3];
    args      = data[4..];

    list = query_listeners(channel);
    for (i = 0, sz = sizeof(list); i < sz; i++) {
	call_out("broadcast_local", 0, list[i], orig_name, (int)orig_self,
		 channel, sub, args);
    }
}

void
receive_tcp_message(string text)
{
    recv_datagram(text, nil, 0);
}

static void
broadcast_local(object target, string game, int self,
		string channel, string sub, mixed *args)
{
    call_other(target, "channel_" + channel + "_" + sub, game, self, args...);
}

void
broadcast_one(string to_game, string channel, string sub, mixed *args)
{
    int i, sz;

    args = ({ "broadcast_one", to_game, channel, sub }) + args;
    for (i = 0, sz = sizeof(args); i < sz; i++) {
	args[i] = url_encode((string)args[i]);
    }
    "~UserAPI/sys/broadcast_tcp"->message(implode(args, " ") + "\n");
}

void
broadcast_all(string channel, string sub, mixed *args)
{
    int i, sz;

    args = ({ "broadcast_all", channel, sub }) + args;
    for (i = 0, sz = sizeof(args); i < sz; i++) {
	args[i] = url_encode((string)args[i]);
    }
    "~UserAPI/sys/broadcast_tcp"->message(implode(args, " ") + "\n");
}
