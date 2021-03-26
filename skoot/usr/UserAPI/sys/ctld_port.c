/*
 * ~UserAPI/sys/ctld_port.c
 *
 * Copyright 2006 Skotos Tech Inc.
 */

# define PORT_OFFSET 71

inherit "/usr/System/lib/port";

private mapping connections;

static void
create()
{
    ::create();

    connections = ([ ]);

    compile_object("/usr/UserAPI/obj/ctld_tcp");
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
	CRITICAL("Panic: failed to open CTLD TCP port " + port);
    } else {
	INFO("CTLD: opened TCP port " + port + " successfully.");
    }
}

void
boot(int block)
{
    if (ur_name(previous_object()) == "/usr/UserAPI/initd") {
	int port;

	port = SYS_INITD->query_portbase() + PORT_OFFSET;
	if (!open_port("tcp", port)) {
	    CRITICAL("Panic: failed to open CTLD TCP port " + port);
	} else {
	    INFO("CTLD: opened TCP port " + port + " successfully.");
	}
    }
}

void
reboot(int block)
{
   boot(block);
}

void
check_users()
{
    object master, current;

    master = find_object("/usr/UserAPI/obj/ctld_tcp");
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

    obj = clone_object("/usr/UserAPI/obj/ctld_tcp");
    INFO("ctld_port receiving incoming connection from '" + host + "' on port " + port + " with " + map_sizeof(connections) + " existing connections.");
    if (!map_sizeof(connections)) {
       "~UserAPI/sys/ctld"->delayed_open();
    }
    connections[obj] = TRUE;
    return obj;
}

void
close_connection()
{
   if (map_sizeof(connections)) {
      connections[previous_object()] = nil;
      if (!map_sizeof(connections)) {
 	 "~UserAPI/sys/ctld"->close(FALSE);
      }
   }
}

void
receive_message(string line)
{
    if (connections && connections[previous_object()]) {
	"~UserAPI/sys/ctld"->receive_message(line);
    }
}

int
message(string text)
{
    if (ur_name(previous_object()) == "/usr/UserAPI/sys/ctld" &&
        map_sizeof(connections)) {
        map_indices(connections)[0]->message(text);
        return TRUE;
    }
    return FALSE;
}
