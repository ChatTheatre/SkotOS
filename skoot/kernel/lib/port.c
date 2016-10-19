# include <status.h>
# include <kernel/kernel.h>
# include <config.h>
# include <kernel/user.h>

private object userd;
private string porttype;

static void create()
{
    userd = find_object(USERD);
}

static int open_port(string protocol, int port, varargs int udp)
{
    rlimits (-1; -1) {
	catch {
	    int i, sz, *list;

	    switch (protocol) {
	    case "telnet":
		list = status()[ST_TELNETPORTS];
		porttype = "telnet";
		break;
	    case "tcp":
		list = status()[ST_BINARYPORTS];
		porttype = "binary";
		break;
	    default:
		error("open_port: Protocol" + protocol + " is not supported.");
		break;
	    }
	    sz = sizeof(list);
	    for (i = 0; i < sz; i++) {
		if (list[i] == port) {
		    call_other(userd, "set_" + porttype + "_manager", i,
			       this_object());
		    return 1;
		}
	    }
	    return 0;
	} : {
	    return 0;
	}
    }
}

string query_banner(object ob)
{
    return nil;
}

object open_connection(string host, int port);

object _F_open_connection(string host, int port)
{
    return open_connection(host, port);
}

int query_timeout(object obj)
{
    return DEFAULT_TIMEOUT;
}
