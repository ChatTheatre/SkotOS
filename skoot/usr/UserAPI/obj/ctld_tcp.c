/*
 * ~UserAPI/obj/ctld_tcp.c
 *
 * Copyright 2006 Skotos Tech Inc.
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
    if (do_self_destruct) {
        return MODE_DISCONNECT;
    }
    connection(previous_object());
    return MODE_LINE;
}

void
logout(int flag)
{
    "~UserAPI/sys/ctld_port"->close_connection();
    destruct_object();
}

int
receive_message(string line)
{
    "~UserAPI/sys/ctld_port"->receive_message(line);
    return MODE_NOCHANGE;
}
