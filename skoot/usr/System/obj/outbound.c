/*
**	~System/obj/outbound.c
**
**	Do the real work of maintaining outbound connections; this
**	cloneable can only be handled by ~System/lib/outboundapi.
**
**
**	010207 Zell	Initial Revision
**
**	Copyright Skotos Tech 1999-2001
*/

# include <System/log.h>
# include <System.h>

inherit CLONABLE;
inherit "/kernel/lib/user";

object	connector;

# define API()	(previous_program() == "/usr/System/lib/outboundapi")
# define CONN()	(previous_program() == "/kernel/lib/connection")

static
void create(int clone) {
   if (clone) {
      call_out("perhaps_suicide", 0);
   }
}

static
void perhaps_suicide() {
   if (!connector) {
      destruct_object();
   }
}

/* calls from ~System/lib/outboundapi */

void connector_destructing() {
   if (API()) {
      ::disconnect();
   }
}

void initialize(object obj) {
   if (API()) {
      connector = obj;
   }
}

void connect(string destination, int port) {
   if (API()) {
      ::connect(destination, port);
   }
}

void disconnect() {
   if (API()) {
      ::disconnect();
   }
}

int message(string str) {
   if (API()) {
      return ::message(str);
   }
}

/* calls to be forwarded */
int login(string name) {
   if (CONN() && connector) {
      connection(previous_object());
      return connector->open();
   }
}

void logout(int dest) {
   if (CONN() && connector) {
      connector->close(dest);
      destruct_object();
   }
}

int receive_message(string str) {
   if (CONN() && connector) {
      return connector->receive_message(str);
   }
}

int message_done() {
   if (CONN() && connector) {
      return connector->message_done();
   }
}
