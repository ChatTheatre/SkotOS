/*
 * ~SMTP/sys/queue.c
 *
 * Keep track of the queue, retry messages a few times, slowing down as they
 * fail repeatedly, or redirect them to the fall-back address (assuming _that_
 * one works at all times).
 *
 * Copyright Skotos Tech Inc 2002-2006
 */

# include <type.h>
# include <SMTP.h>

private inherit "/lib/string";
private inherit "/lib/url";

private int counter;

static void
create()
{
    counter  = 0;
}

private string
compose_address(string name, string address)
{
    if (name && strlen(name)) {
	return "\"" + name + "\" <" + address + ">";
    }
    return "<" + address + ">";
}

private string *
compose_message(mapping map)
{
    int     i, sz;
    string  *lines, *names, *addresses, *str;

    /* From: header. */
    lines = ({ "From: " + compose_address(map["from_name"], map["from"]) });

    /* To: header. */
    sz        = map_sizeof(map["to"]);
    addresses = map_indices(map["to"]);
    names     = map_values(map["to"]);
    lines += ({ "To: " + compose_address(names[0], addresses[0]) });
    for (i = 1; i < sz; i++) {
	lines += ({ "\t" + compose_address(names[i], addresses[i]) });
    }

    /* Cc: header. */
    sz        = map_sizeof(map["cc"]);
    if (sz > 0) {
	addresses = map_indices(map["cc"]);
	names     = map_values(map["cc"]);
	lines += ({ "Cc: " + compose_address(names[0], addresses[0]) });
	for (i = 1; i < sz; i++) {
	    lines += ({ "\t" + compose_address(names[i], addresses[i]) });
	}
    }

    /* Subject: header. */
    lines += ({ "Subject: " + map["subject"] });

    /* X-* headers. */
    str = map["x_headers"];
    if (str) {
	for (i = 0, sz = sizeof(str); i < sz; i++) {
	    lines += ({ "X-" + str[i] });
	}
    }

    /* Indicator of who sent this. */
    lines += ({ "X-Mailer: Skoot SMTP module on " +
		SYS_INITD->query_hostname() + ":" +
		SYS_INITD->query_portbase() });

    /* Separator between headers and text. */
    lines += ({ "" });

    str = explode("\n" + map["message"] + "\n", "\n");
    for (i = 0, sz = sizeof(str); i < sz; i++) {
	if (strlen(str[i]) > 0 && str[i][0] == '.') {
	    str[i] = "." + str[i];
	}
    }
    lines += str;

    return lines;
}

private void write_headers(mapping map)
{
    int    i, sz;
    string fname, *fields;
    mixed  *data;

    fname = SMTP_QUEUE + "new/" + map["id"] + ".meta";
    sz = map_sizeof(map);
    fields = map_indices(map);
    data = map_values(map);
    for (i = 0; i < sz; i++) {
	int j, sz_j;
	string *ind, *val;
	mixed *bits;

	switch (typeof(data[i])) {
	case T_ARRAY:
	    sz_j = sizeof(data[i]);
	    if (sz_j > 0) {
		bits = allocate(sz_j);
		for (j = 0; j < sz_j; j++) {
		    bits[j] = url_encode(data[i][j]);
		}
		write_file(fname, fields[i] + " " + implode(bits, " ") + "\n");
	    }
	    break;
	case T_MAPPING:
	    sz_j = map_sizeof(data[i]);
	    if (sz_j > 0) {
		ind = map_indices(data[i]);
		val = map_values(data[i]);
		bits = allocate(sz_j * 2);
		for (j = 0; j < sz_j; j++) {
		    bits[j * 2]     = url_encode(ind[j]);
		    bits[j * 2 + 1] = url_encode(val[j]);
		}
		write_file(fname, fields[i] + " " + implode(bits, " ") + "\n");
	    }
	    break;
	case T_STRING:
	    write_file(fname, fields[i] + " " + url_encode(data[i]) + "\n");
	    break;
	case T_INT:
	    write_file(fname, fields[i] + " " + data[i] + "\n");
	    break;
	default:
	    break;
	}
    }
}

private void write_message(mapping map)
{
    string fname, *lines;

    fname = SMTP_QUEUE + "new/" + map["id"] + ".data";
    lines = compose_message(map);
    write_file(fname, implode(lines, "\n") + "\n");
}

string
queue_message(string envelope, string from_name, string from, mapping to,
	      mapping cc, string subject, string *x_headers, string message)
{
   int     stamp;
   string  id;
   mapping map;

   if (previous_program() != SMTP_API) {
      error("send_message: Try using the API");
   }
   if (!to || !map_sizeof(to)) {
      error("send_message: 'to' parameter required");
   }
   if (!subject) {
      error("send_message: 'subject' parameter required");
   }
   if (!message) {
      error("send_message: 'message' parameter required.");
   }
   if (!from) {
      from = "ce@skotos.net";
   }
   if (!envelope) {
      envelope = from;
   }
   if (!cc) {
      cc = ([ ]);
   }
   if (!x_headers) {
      x_headers = ({ });
   }
   counter++;
   id    = (string)counter;
   stamp = time();
   map = ([
      "id":         id,
      "envelope":   envelope,
      "from_name":  from_name,
      "from":       from,
      "to":         to,
      "cc":         cc,
      "recipients": map_indices(cc) | map_indices(to),
      "subject":    subject,
      "x_headers":  x_headers,
      "message":    message,
      "scheduled":  stamp,
   ]);

   /* Create headers and body in temporary location. */
   write_headers(map);
   write_message(map);

   /*
    * Then move them to actual queue, message first, headers last so that
    * when the background script looks for the headers, the message is
    * guaranteed to be there already.
    */
   rename_file(SMTP_QUEUE + "new/" + id + ".meta",
	       SMTP_QUEUE + id + ".meta");
   rename_file(SMTP_QUEUE + "new/" + id + ".data",
	       SMTP_QUEUE + id + ".data");

   return id;
}
