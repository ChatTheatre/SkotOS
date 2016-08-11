/*
 * ~SMTP/lib/smtp_api.c
 *
 * Provide an api for the SMTP daemon/queue.
 *
 * Copyright Skotos Tech Inc 2002-2006
 */

# include <SMTP.h>

static string
send_message(string envelope, string from_name, string from, mapping to,
	     mapping cc, string subject, string *x_headers, string message)
{
   return find_object(SMTP_SYS)->queue_message(envelope, from_name, from,
					       to, cc, subject,
					       x_headers, message);
}

static string *
query_queue_list()
{
   /* Obsolete. */
   return ({ });
}

static mapping
query_queue_entry(string id)
{
   /* Obsolete. */
   return nil;
}
