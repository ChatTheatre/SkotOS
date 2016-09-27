/*
 * ~UserDB/obj/broadcast.c
 *
 * Connection object that (attempts to) connect to one of the games with which
 * we exchange broadcast_{one,all} information.  If the link cannot be made we
 * retry after OPEN_RETRY (now 5) seconds.
 *
 * Copyright 2001 Skotos Tech Inc.
 */

# define CLOSE_DELAY 1

# include <kernel/user.h>

private inherit "/lib/url";

inherit "~System/lib/outboundapi";

private string ipnr;
private int    base;
private int    port;
private int    is_open;

static void
create(int clone)
{
    ::create();
}

void
set_ipnumber_and_port(string i, int b, int p)
{
    ipnr = i;
    base = b;
    port = p;
}

void die() {
  call_out("destruct_object", 0);
}

string query_ipnr() { return ipnr; }

void
boot(int block)
{
    if (ipnr && base) {
	call_out("reconnect", 0);
    }
}

static void
reconnect()
{
    connect(ipnr, base + port);
}

int
open()
{
    is_open = TRUE;
    DEBUG("Broadcast:Opened connection to " + ipnr + ":" + (base + port));
    return MODE_LINE;
}

void
close(int dest)
{
    if (is_open) {
	DEBUG("Broadcast:Lost connection to " + ipnr + ":" + (base + port));
	is_open = FALSE;
	/* Try immediately, once, after that only every CLOSE_DELAY seconds. */
	call_out("reconnect", 0);
    } else {
        DEBUG("Broadcast:Failed to make connection to " + ipnr + ":" + (base + port));
	call_out("reconnect", CLOSE_DELAY);
    }
}

int
receive_message(string str)
{
    int i, sz;
    string *chunks;

    chunks = explode(" " + str + " ", " ");
    sz = sizeof(chunks);
    for (i = 0; i < sz; i++) {
	chunks[i] = url_decode(chunks[i]);
    }
    switch (chunks[0]) {
    case "broadcast_one":
	"~UserDB/sys/broadcast"->broadcast_one(ipnr, base, chunks[1..]...);
	break;
    case "broadcast_all":
	"~UserDB/sys/broadcast"->broadcast_all(ipnr, base, chunks[1..]...);
	break;
    default:
	break;
    }
    return MODE_NOCHANGE;
}
