/*
 * ~UserAPI/obj/broadcast_tcp.c
 *
 * Copyright 2001 Skotos Tech Inc.
 */

# include <kernel/user.h>

inherit "/usr/System/lib/user";

private int do_self_destruct;

void
self_destruct()
{
    do_self_destruct = TRUE;
}

int
login(string name)
{
    string host;

    if (do_self_destruct) {
        return MODE_DISCONNECT;
    }
    host = query_ip_number(previous_object());
#if 0
    if (host != SYS_INITD->query_userdb_hostname()) {
        return MODE_DISCONNECT;
    }
#endif
    connection(previous_object());
    return MODE_LINE;
}

void
logout(int flag)
{
    DEBUG(ur_name(this_object()) + ": Losing connection ... ?");
    destruct_object();
}

int
receive_message(string line)
{
    "~UserAPI/sys/broadcast_tcp"->receive_message(line);
    return MODE_NOCHANGE;
}
