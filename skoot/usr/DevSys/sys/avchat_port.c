/*
** ~DevSys/sys/avchat_port.c
**
** Copyright ChatTheatre, 2021
*/

/* avchat_port tracks incoming connections. When asked to send a message,
   it uses the oldest received connection and then waits for a response. */

# define PORT_OFFSET 91

inherit "/usr/System/lib/port";

private mapping connections;
private int port_opened;

private void open_chat_port(void)
{
    int port;

    if(port_opened) { return; }

    port = SYS_INITD->query_portbase() + PORT_OFFSET;
    if (!open_port("tcp", port)) {
        CRITICAL("Panic: failed to open AVChat TCP port " + port);
        port_opened = 0;
    } else {
        CRITICAL("AVChat: opened TCP port " + port + " successfully.");
        port_opened = 1;
    }
}

static void
create()
{
    ::create();

    port_opened = 0;
    connections = ([ ]);

    compile_object("/usr/DevSys/obj/avchat_tcp");

    open_chat_port();
}

int check_port_open(void)
{
    return port_opened;
}

void boot(int block)
{
    if (ur_name(previous_object()) == "/usr/DevSys/initd") {
        open_chat_port();
    }
}

void reboot(int block)
{
    if (ur_name(previous_object()) == "/usr/DevSys/initd") {
        open_chat_port();
    }
}

void patch()
{
    port_opened = 0;
    open_chat_port();
}

static object
open_connection(string host, int port)
{
    object obj;

    obj = clone_object("/usr/DevSys/obj/avchat_tcp");
    INFO("AVChat: avchat_port receiving incoming connection from '" + host + "' on port " + port + " with " + map_sizeof(connections) + " existing connections.");

    if (!map_sizeof(connections)) {
        "/usr/DevSys/sys/avchat"->connected();
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
      "/usr/DevSys/sys/avchat"->disconnected();
    }
  }
}

int
message(string text)
{
    if (ur_name(previous_object()) == "/usr/DevSys/sys/avchat" &&
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
        "/usr/DevSys/sys/avchat"->receive_message(line);
    }
}
