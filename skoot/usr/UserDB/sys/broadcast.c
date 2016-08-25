/*
 * ~UserDB/sys/broadcast.c
 *
 * Implements the broadcast (via some UDP port) facilities for the Intermud
 * feature of our game that allows broadcasting 'assist' requests and sharing
 * a chatline or two if we want.
 *
 * Copyright 2001 Skotos Tech Inc.
 */

# include <UserDB.h>

# define PORT_OFFSET 17

# define DEBUG_INPUT
# undef  DEBUG_OUTPUT

inherit "/lib/url";
inherit "/lib/string";
inherit "/usr/System/lib/port";

/*
 * Variables:
 */

private mapping receivers_forward;
private mapping receivers_reverse;
private mapping receivers_ports;
private int     booted;

/*
 * Prototypes:
 */
void   add_receiver(string ipnr, int portbase, string name);
string *query_receivers();
mixed  *query_receiver(string name);

/*
 * Implementation:
 */

void setup_receivers()
{
}

static void
create()
{
    ::create();

    receivers_forward = ([ ]);
    receivers_reverse = ([ ]);
    receivers_ports   = ([ ]);

    compile_object("~UserDB/obj/broadcast");
}

void patch() {
    int    i, sz;
    string *names;

    create();
#if 0

    names = query_receivers();
    sz = sizeof(names);
    for (i = 0; i < sz; i++) {
	object port;
	mixed *data;

	port = receivers_ports[names[i]];
	if (!port) {
	    data = query_receiver(names[i]);

	    receivers_ports[names[i]] = port = clone_object("~UserDB/obj/broadcast");
	    port->set_ipnumber_and_port(data[0], data[1], PORT_OFFSET);
	}
	port->boot(FALSE);
    }
    booted = TRUE;
#endif
}

void patch_oops() {
    object master, current;

    master = find_object("~UserDB/obj/broadcast");
    current = master->query_next_clone();
    while (current && current != master) {
	object next;

	next = current->query_next_clone();
	DEBUG("Zapping " + dump_value(current));
	current->destruct();

	current = next;
    }
}


void
boot(int block)
{
    if (ur_name(previous_object()) == "/usr/UserDB/initd") {
	int    i, sz;
	string *names;

	names = query_receivers();
	sz = sizeof(names);
	for (i = 0; i < sz; i++) {
	    object port;
	    mixed *data;

	    port = receivers_ports[names[i]];
	    if (!port) {
		data = query_receiver(names[i]);

		receivers_ports[names[i]] = port = clone_object("~UserDB/obj/broadcast");
		port->set_ipnumber_and_port(data[0], data[1], PORT_OFFSET);
	    }
	    port->boot(block);
	}
    }
    booted = TRUE;
}

void
reboot(int block)
{
    boot(block);
}

void
add_receiver(string ipnr, int portbase, string name)
{
    mapping map;

    map = receivers_reverse[ipnr];
    if (!map) {
	receivers_reverse[ipnr] = map = ([ ]);
    }
    map[portbase] = name;

    receivers_forward[name] = ({ ipnr, portbase });

    if (booted) {
	object port;

	port = receivers_ports[name];
	if (!port) {
	    receivers_ports[name] = port = clone_object("~UserDB/obj/broadcast");
	    DEBUG("Activating " + ur_name(port) + " for " + name + " at " + ipnr + " on " + portbase);
	    port->set_ipnumber_and_port(ipnr, portbase, PORT_OFFSET);
	}
	port->boot(FALSE);
    }
}

void
remove_receiver(string name)
{
    int    base;
    string ipnr;
    object port;

    ipnr = receivers_forward[name][0];
    base = receivers_forward[name][1];

    receivers_reverse[ipnr][base] = nil;
    if (map_sizeof(receivers_reverse[ipnr]) == 0) {
	receivers_reverse[ipnr] = nil;
    }
    receivers_forward[name] = nil;

    if (port = receivers_ports[name]) {
	port->destruct();
    }
}

string
find_receiver(string ipnr, int portbase)
{
    return receivers_reverse[ipnr] ? receivers_reverse[ipnr][portbase] : nil;
}

string *
query_receivers()
{
    return map_indices(receivers_forward);
}

/*
 * Find the preferred capitalization of a game's name, to avoid problems.
 */
string
query_receiver_name(string name)
{
    int i, sz;
    string *names;

    name = lower_case(name);
    names = map_indices(receivers_forward);
    sz = sizeof(names);
    for (i = 0; i < sz; i++) {
	if (lower_case(names[i]) == name) {
	    return names[i];
	}
    }
    return nil;
}

mixed *
query_receiver(string name)
{
    return receivers_forward[name][..];
}

int
broadcast_one(string from_ipnr, int from_base, string to_name,
	      string channel, string sub, mixed args...)
{
    int    i, sz, base;
    string name, output, ipnr;
    object port;
    mixed  *data;

    /* Check origin. */
    name = find_receiver(from_ipnr, from_base);
    if (!name) {
	DEBUG("Ignored broadcast_one from " + from_ipnr + ":" + from_base +
	      " (unknown)");
	return 0;
    }
    to_name = query_receiver_name(to_name);

    data = query_receiver(to_name);
    if (!data) {
	DEBUG("Ignored broadcast_one to " + to_name + " (unknown)");
	return 0;
    }
    ipnr = data[0];
    base = data[1];
    port = receivers_ports[to_name];
    if (!port) {
	DEBUG("Broadcast: Missing port object for " + to_name + " (" +
	      ipnr + ":" + (base + PORT_OFFSET));
	return 0;
    }
    args = ({ name, channel, sub }) + args;
    sz = sizeof(args);
    for (i = 0; i < sz; i++) {
	args[i] = url_encode(args[i]);
    }
    output = implode(args, ":");
    if (from_ipnr == ipnr && from_base == base) {
	call_out("send_data", 0, "1:" + output, port);
    } else {
	call_out("send_data", 0, "0:" + output, port);
    }
}

int
broadcast_all(string from_ipnr, int from_base,
	      string channel, string sub, mixed args...)
{
    int     i, sz, total;
    string  name, *names, output;

    name = find_receiver(from_ipnr, from_base);
    if (!name) {
	DEBUG("Ignored broadcast_all from " + from_ipnr + ":" + from_base +
	      " (unknown)");
	return 0;
    }
    args = ({ name, channel, sub }) + args;
    sz = sizeof(args);
    for (i = 0; i < sz; i++) {
	args[i] = url_encode(args[i]);
    }
    output = implode(args, ":");

    names = query_receivers();
    sz = sizeof(names);
    for (i = 0; i < sz; i++) {
	int    base;
	string ipnr;
	object port;
	mixed  *data;

	data = query_receiver(names[i]);
	ipnr = data[0];
	base = data[1];
	port = receivers_ports[names[i]];
	if (!port) {
	    DEBUG("Broadcast: Missing port object for " + names[i] + " (" +
		  ipnr + ":" + (base + PORT_OFFSET));
	    continue;
	}
	if (from_ipnr == ipnr && from_base == base) {
	    call_out("send_data", 0, "1:" + output, port);
	} else {
	    call_out("send_data", 0, "0:" + output, port);
	}
    }
    return sz;
}

void
send_data(string output, object port)
{
# ifdef DEBUG_OUTPUT
    DEBUG("send_data: " + dump_value(output));
# endif
    port->message(output + "\n");
}
