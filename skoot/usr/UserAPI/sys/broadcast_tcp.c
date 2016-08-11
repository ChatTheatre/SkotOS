/*
 * ~UserAPI/sys/broadcast_tcp.c
 *
 * A temporary (famous last words) workaround while the UDP port is still
 * open, preventing us from properly patching the code.
 *
 * This object opens _TCP_ port <base> + PORT_OFFSET (17) and waits for an
 * incoming connection from the UserDB (verified by checking ipnumber).
 *
 * Any other connections are closed as quickly as they are opened.
 *
 * Copyright 2001 Skotos Tech Inc.
 */

# define PORT_OFFSET 17

inherit "/usr/System/lib/port";

private mapping connections;

static void
create()
{
    ::create();

    connections = ([ ]);

    compile_object("/usr/UserAPI/obj/broadcast_tcp");
}

void
patch()
{
    int port;

    if (!connections) {
	connections = ([ ]);
    }
    port = SYS_INITD->query_portbase() + PORT_OFFSET;
    if (!open_port("tcp", port)) {
	CRITICAL("Panic: failed to open Broadcast TCP port " + port);
    } else {
	INFO("Broadcast: opened TCP port " + port + " successfully.");
    }
}

void
boot(int block)
{
    if (ur_name(previous_object()) == "/usr/UserAPI/sys/broadcast") {
	int port;

	port = SYS_INITD->query_portbase() + PORT_OFFSET;
	if (!open_port("tcp", port)) {
	    CRITICAL("Panic: failed to open Broadcast TCP port " + port);
	} else {
	    INFO("Broadcast: opened TCP port " + port + " successfully.");
	}
    }
}

void
check_users()
{
    object master, current;

    master = find_object("/usr/UserAPI/obj/broadcast_tcp");
    current = master->query_next_clone();
    while (master && current && current != master) {
      DEBUG("user             = " + dump_value(current));
      DEBUG("user->conn       = " + dump_value(current->query_conn()));
      if (current->query_conn()) {
	  DEBUG("user->conn->user = " +
		dump_value(current->query_conn()->query_user()));
      }
      current = current->query_next_clone();
    }
}


static object
open_connection(string host, int port)
{
    object obj;

    obj = clone_object("/usr/UserAPI/obj/broadcast_tcp");
    connections[obj] = TRUE;
    return obj;
}

void
receive_message(string line)
{
    if (connections && connections[previous_object()]) {
	"~UserAPI/sys/broadcast"->receive_tcp_message(line);
    }
}

void
message(string text)
{
    if (map_sizeof(connections)) {
        map_indices(connections)[0]->message(text);
    }
}
