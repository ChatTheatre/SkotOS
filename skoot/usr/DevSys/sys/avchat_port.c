/*
** ~AVChat/sys/avchat_port.c
**
** Copyright ChatTheatre, 2021
*/

/* avchat_port tracks incoming connections. When asked to send a message,
   it uses the oldest received connection and then waits for a response. */

# define PORT_OFFSET 91

inherit "/usr/System/lib/port";

private mapping connections;

static void
create()
{
    ::create();

    connections = ([ ]);

    compile_object("/usr/DevSys/obj/avchat_tcp");
}

void boot(int block)
{
    if (ur_name(previous_object()) == "/usr/DevSys/initd") {
        int port;

        port = SYS_INITD->query_portbase() + PORT_OFFSET;
        if (!open_port("tcp", port)) {
            CRITICAL("Panic: failed to open AVChat TCP port " + port);
        } else {
            INFO("AVChat: opened TCP port " + port + " successfully.");
        }
    }
}

void reboot(int block)
{
    boot(block);
}

static object
open_connection(string host, int port)
{
    object obj;

    obj = clone_object("/usr/AVChat/obj/avchat_tcp");
    INFO("AVChat: avchat_port receiving incoming connection from '" + host + "' on port " + port + " with " + map_sizeof(connections) + " existing connections.");

    if (!map_sizeof(connections)) {
        "~AVChat/sys/avchat"->connected();
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
      "~AVChat/sys/avchat"->disconnected();
    }
  }
}

int
message(string text)
{
    if (ur_name(previous_object()) == "/usr/AVChat/sys/avchat" &&
        map_sizeof(connections)) {
        map_indices(connections)[0]->message(text);
        return TRUE;
    }
    return FALSE;
}

void
receive_message(string line)
{
    if (connections && connections[previous_object()]) {
        "~AVChat/sys/avchat"->receive_message(line);
    }
}
