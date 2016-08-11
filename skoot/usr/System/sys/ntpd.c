/*
**	~System/sys/ntpd.c
**
**	This program is actually a NTP client, serving the rest of the
**	system with an accurate GMT clock. We represent this as an offset
**	to time(), in seconds.
**
**	Copyright Skotos Tech Inc 1999
*/

# define LOGNAME	"NTPD"

# include <kernel/kernel.h>
# include <System.h>
# include <System/log.h>

inherit "/kernel/lib/port";

int offset, offsets;
int booting;

void send_request();

static
void create() {
    string str;

    ::create();

    booting = FALSE;
}

int boot(int block) {
   if (block && object_name(previous_object()) == SYS_BOOT) {
        if (!open_port("udp", SYS_INITD->query_portbase() + 23)) {
	   error("failed to open UDP port");
	}
	send_request();
	booting = TRUE;
	return 3;	/* allow a 3-second wait */
   }
}


int reboot(int block) {
   return boot(block);
}

# define JAN_1970	0x83aa7e80	/* 2208988800 1970 - 1900 in seconds */

void send_request() {
    string request;

    /* un-initialized offset */
    offset = offsets = 0;

    request = ("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
	       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
	       "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
    /* set VN to 1 */
    request[0] |= 0x08;
    /* set MODE to 3 */
    request[0] |= 0x03;

    XDebug("sending request datagrams");
    call_out("do_send", 0, request, "157.22.9.7", 123);
    call_out("do_send", 0, request, "128.173.14.71", 123);
    call_out("do_send", 0, request, "171.64.2.220", 123);
    call_out("stop_waiting", 1);
}

static
void do_send(string gram, string host, int port) {
   DEBUG("Sending datagram size " + strlen(gram) + " to " + host + "/" + port);
   send_datagram(gram, host, port);
}


static
void recv_datagram(string gram, string host, int port) {
   int secs, this_offset;

   secs = ((gram[0x28] << 24) + (gram[0x29] << 16) +
	   (gram[0x2A] << 8) + gram[0x2B]);
   secs -= JAN_1970;

   this_offset = secs - time();

   offset = (offset * offsets + this_offset) / (offsets + 1);
   offsets ++;

   DEBUG("received reply from [" + host + ":" + port + "] with offset " + this_offset);
}

static
void stop_waiting() {
   INFO("NTPD: Stored offset of " + offset + " seconds to UTC.\n");
   if (booting) {
      SYS_BOOT->daemon_done();
      booting = FALSE;
   }
}

int ntp_time() {
    return time() + offset;
}
