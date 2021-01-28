/*
 * ~UserAPI/sys/authd_port.c
 *
 * Copyright 2006 Skotos Tech Inc.
 */

/* authd_port is a weird setup, designed to avoid outgoing connections from DGD. */

/* Specifically, it waits for incoming connections and keeps track of them. If UserAPI
   is queried for auth or control information, authd_port will send the request
   out to the oldest (first) connection and wait for a response. */

/* The incoming connection is normally (circa Jan 2021) from the userdb-authctl
   utility shim program, which exists to make connections to SkotOS's DGD server
   (ports PORTBASE + 70 and PORTBASE + 71) and thin-auth's authd and ctld
   servers (ports 9970 and 9971) and forward traffic back and forth between
   them. So it operates *as if* the PHP servers made an outgoing connection to
   DGD or vice-versa.

   If there are multiple incoming connections (e.g. multiple copies of userdb-authctl
   running, or another server making a connection to PORTBASE + 70) then the situation
   can get confusing. DGD will always send over the *oldest* connection.

   The PHP servers (server-auth.php and server-control.php) don't obviously appear to
   *originate* any outgoing traffic, so it's not clear that they ever have to handle
   that problem. */

/* For debugging I'm adding logging to the accepting of incoming connections. Note that
   we can't easily log incoming or outgoing traffic function because message() and
   receive_message() are usually called in an atomic context. */

# define PORT_OFFSET 70

inherit "/usr/System/lib/port";

private mapping connections;

static void
create()
{
    ::create();

    connections = ([ ]);

    compile_object("/usr/UserAPI/obj/authd_tcp");
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
	CRITICAL("Panic: failed to open AUTHD TCP port " + port);
    } else {
	INFO("AUTHD: opened TCP port " + port + " successfully.");
    }
}

void
boot(int block)
{
    if (ur_name(previous_object()) == "/usr/UserAPI/initd") {
	int port;

	port = SYS_INITD->query_portbase() + PORT_OFFSET;
	if (!open_port("tcp", port)) {
	    CRITICAL("Panic: failed to open AUTHD TCP port " + port);
	} else {
	    INFO("AUTHD: opened TCP port " + port + " successfully.");
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

    master = find_object("/usr/UserAPI/obj/authd_tcp");
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

    obj = clone_object("/usr/UserAPI/obj/authd_tcp");
#if 0
    if (host != SYS_INITD->query_userdb_hostname() && host != "198.232.133.5") {
	obj->self_destruct();
    }
#endif
    INFO("UserDB: authd_port receiving incoming connection from '" + host + "' on port " + port + " with " + map_sizeof(connections) + " existing connections.");
    if (!map_sizeof(connections)) {
       "~UserAPI/sys/authd"->delayed_open();
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
      "~UserAPI/sys/authd"->close(FALSE);
    }
  }
}

void
receive_message(string line)
{
    if (connections && connections[previous_object()]) {
	"~UserAPI/sys/authd"->receive_message(line);
    }
}

int
message(string text)
{
    if (ur_name(previous_object()) == "/usr/UserAPI/sys/authd" &&
        map_sizeof(connections)) {
        map_indices(connections)[0]->message(text);
        return TRUE;
    }
    return FALSE;
}
