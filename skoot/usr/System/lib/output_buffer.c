/*
**	~System/lib/output_buffer.c
**
**	This code sits on top of the inheritable kernel user object,
**	and extends DGD's output buffering. Inheritors may dump any
**	amount of data (or close to it) into this buffer and all will
**	be dealt with efficiently, with a single call-back when done.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <kernel/kernel.h>
# include <kernel/user.h>
# include <System/log.h>

# define CHUNK	49152	/* must be less than 64K, DGD's max string length */

inherit "/lib/version";
inherit "/lib/file";
inherit "/kernel/lib/user";

private mixed *output;		/* this is the buffer */
private int co_handle;
private int clearable_output;
private int committed_output;
private int awaiting_message_done;

mixed query_output_data() { return output; }

static
void clear_output() {
   output = nil;
   if (co_handle) {
      remove_call_out(co_handle);
      co_handle = 0;
   }
}

/* public API */
static
void send_message(string data) {
    int sz;

    clearable_output = TRUE;
    if (output == nil) {
	output = ({ });
    }
    sz = sizeof(output);
    /*
     * For a simple string, just plonk it into the buffer or merge it
     * with the previous one, if that's a string and the combined
     * length doesn't get too long.
     */
    if (sz > 0 && typeof(output[sz - 1] == T_STRING) &&
	strlen(data) + strlen(output[sz - 1]) <= CHUNK) {
        output[sz - 1] += data;
    } else {
        output += ({ data });
    }
    if (!co_handle && !awaiting_message_done) {
	/* defer actual outputing until the end of this thread */
	co_handle = call_out("do_output_data", 0);
    }
    XDebug("sent " + strlen(data) + " bytes... co-handle = " + co_handle);
}

static
void send_data(object data) {
   clearable_output = TRUE;
   if (output == nil) {
      output = ({ });
   }
   output += data->query_chunks();
   if (!co_handle && !awaiting_message_done) {
      /* defer actual outputing until the end of this thread */
      co_handle = call_out("do_output_data", 0);
   }
}

static
void send_file(string file, int file_size) {
    clearable_output = TRUE;
    if (output == nil) {
	output = ({ });
    }
    /* for a file, just store the path and a file offset of zero */
    output += ({ ({ file, 0, file_size }) });
    if (!co_handle && !awaiting_message_done) {
	/* defer actual outputing until the end of this thread */
	co_handle = call_out("do_output_data", 0);
    }
}

int query_clearable_output() { return clearable_output; }
int query_committed_output() { return committed_output; }

static nomask void _do_output_data();

static nomask
void do_output_data() {
    int ix;

    XDebug("I AM NOW IN DO_OUTPUT_DATA");

    co_handle = 0;

    /* step through the buffer, sending as much as we can */
    ix = 0;
    while (ix < sizeof(output)) {
	string data;

	/* is this buffer entry a file pointer? */
	if (typeof(output[ix]) == T_ARRAY) {
	    int eof, left;

	    /* yes, so send as much as we can of the file */
	    eof = output[ix][2];
	    if (file_size(output[ix][0]) < eof) {
		/* this should happen very rarely! */
		eof = file_size(output[ix][0]);
	    }
	    left = eof - output[ix][1];
	    if (left > CHUNK) {
		/* bite off another chunk of the file */
		XDebug("Sending CHUNK at " + output[ix][1]);
		data = read_file(output[ix][0], output[ix][1], CHUNK);
		output[ix][1] += CHUNK;
	    } else {
		/* bite off the tail of the file */
	        XDebug("Sending TAIL");
		data = read_file(output[ix][0], output[ix][1], left);
		/* and proceed to the next buffer entry */
		ix ++;
	    }
	} else {
	    /* else the data is just a string, send as is and proceed */

	    data = output[ix];
	    ix ++;
	}
	committed_output = TRUE;

	if (!message(data)) {
	    /* we filled DGD's buffer; exit and wait for callback */
	    XDebug("Buffer full -- waiting");
	    output = output[ix ..];
	    awaiting_message_done = TRUE;
	    return;
	}
    }
    output = output[ix ..];
    /* we're done! notify our inheritor */
    if (dgd_version() < 11097) {
       XDebug("calling out to message_sent");
       call_out("message_sent", 3);
    }
}

/*
**	This is the callback from /kernel/lib/user; it means
**	the driver buffer has been transmitted -- so send more.
*/

nomask
int message_done() {
   if (KERNEL() || SYSTEM()) {
      awaiting_message_done = FALSE;
      if ((!output || !sizeof(output)) && dgd_version() >= 11097) {
	 XDebug("In message_done:: returning");
	 call_out("message_sent", 1);
	 return MODE_NOCHANGE;
      }
      XDebug("In message_done:: outputting more data");
      do_output_data();
      return MODE_NOCHANGE;
   }
}
