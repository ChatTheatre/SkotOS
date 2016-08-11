/*
 * ~SMTP/obj/smtpconn.c
 *
 * Creates the connection to the smarthost's smtp-daemon and from then on
 * relays all relevant calls to the queue daemon.  For MP this could be
 * optimized by moving the state-machine code into this object and only
 * reporting back to the queue if the message needs to be requeued, bounced,
 * sent to the postmaster, or was successfully sent.
 *
 * Copyright Skotos Tech Inc 2002
 */

# include <SMTP.h>

inherit "/usr/System/lib/outboundapi";

private string request_id;

static void
create(int clone)
{
   ::create();
}

void
initialize(string id)
{
   request_id = id;
   connect(SMTP_SMARTHOST, SMTP_PORT);
}

int
open()
{
   return find_object(SMTP_SYS)->update_status(request_id, "open");
}

void
close(int dest)
{
   find_object(SMTP_SYS)->update_status(request_id, "close");
   call_out("suicide", 0);
}

static void
suicide()
{
   destruct();
}

int
receive_message(string line)
{
   return find_object(SMTP_SYS)->update_status(request_id, "input", line);
}
