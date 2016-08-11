/*
**	~System/obj/dev_port.c
**
**	Copyright Skotos Tech Inc 1999
*/


# include <kernel/kernel.h>
# include <kernel/user.h>
# include <System.h>

inherit SYS_AUTO;
inherit "/kernel/lib/port";

static
void create(int clone) {
    ::create();
}

int open_port(string type, int port) {
   if (previous_program() == DEV_USERD) {
      return ::open_port(type, port);
   }
}

/* this is called for every connection */
static object open_connection(string ipaddr, int port) {
    return nil;
}

string query_banner(object connection) {
   return DEV_USERD->query_banner(connection);
}

object select(string name) {
   return DEV_USERD->select(name);
}
