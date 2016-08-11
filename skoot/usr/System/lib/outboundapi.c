/*
**	~System/lib/outboundapi.c
**
**	Allow the rest of the mudlib to create outbound connections;
**	relay all relevant API calls invisibly. This inheritable does
**	its real work with the help of ~System/obj/outbound.c clones.
**
**
**	010207 Zell	Initial Revision
**
**	Copyright Skotos Tech 1999-2001
*/

# include <kernel/user.h>

private object relay;

static
void destructing() {
   if (relay) {
      relay->connector_destructing();
   }
}

static
void connect(string destination, int port) {
   if (!relay) {
      relay = clone_object("/usr/System/obj/outbound");
      relay->initialize(this_object());
   }
   relay->connect(destination, port);
}

static
void disconnect() {
   if (relay) {
      relay->disconnect();
   }
}

int message(string str) {
   if (relay) {
      return relay->message(str);
    }
    return 0;
}

/* should be masked by inheritor */
int receive_message(string str) {
   return MODE_NOCHANGE;
}

/* should be masked by inheritor */
int message_done() {
   return MODE_NOCHANGE;
}
