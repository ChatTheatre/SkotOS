/*
** ~DevSys/sys/avchat.c
**
** Copyright ChatTheatre, 2021
*/

# include <kernel/user.h>

inherit "/usr/System/lib/outboundapi";

private int is_connected;
private int counter;
private mapping requests;
private mapping request_args;

/* Default to 600 seconds for valid JSON token time */
# define DEFAULT_VALID_PERIOD 600

void create() {
    is_connected = 0;
    counter = 0;
    requests = ([ ]);
    request_args = ([ ]);

    find_or_load("/usr/DevSys/sys/avchat_port");
}

void connected() {
    is_connected = 1;
}

int check_connected() {
    return is_connected;
}

void disconnected() {
    is_connected = 0;
}

/*** Queries ***/

void
token_for_channel_and_name(string display_name, string channel_name, object cb_object, mixed cb_arg)
{
    send_token_request(cb_object, cb_arg, display_name, channel_name, DEFAULT_VALID_PERIOD);
}

/*** Low-level I/O functions ***/

/* Send a non-moderator token request using validFor rather than validUntil */
private int
send_token_request(object reply_to, mixed cb_arg,
    string display_name,
    string channel_name,
    int valid_for)
{
    string cas_msg;
    string nul_char;

    cas_msg = "{ cmd: \"jwt_token\", displayName: \"" + display_name +
        "\", channel: \"" + channel_name +
        "\", moderator: false, validFor: " + (string)valid_for +
        ", seq: " + (string)counter +
        " }";
    requests[counter] = reply_to;
    request_args[counter] = cb_arg;
    call_out("timeout", 60, counter);
    counter += 1;

    nul_char = " ";
    nul_char[0] = 0;

    "~DevSys/sys/avchat_port"->message(cas_msg + nul_char);
}

static
void timeout(int counter) {
    object reply_to;
    object reply_arg;

    /* If this request is still pending... */
    if (reply_to = requests[counter]) {
        reply_arg = request_args[counter];

        reply_to->avchat_error(reply_arg, "TIMEOUT");
        requests[counter] = nil;
        request_args[counter] = nil;
    }
}

int
receive_message(string line) {
    object reply_to;
    int seq;

    if(previous_program() == "~DevSys/sys/avchat_port") {
        if (sscanf(line, "seq: %d", seq) == 1) {
            reply_to = requests[seq];
            reply_arg = request_args[seq];
            if (sscanf(line, "success: true")) {
                string token;
                if (sscanf(line, "token: \"%s\"", token) == 1)
                reply_to->avchat_token(reply_arg, token);
            } else {
                reply_to->avchat_error(reply_arg, line);
                /* Failed */
                error("Failure from token server: " + line);
            }
            requests[seq] = nil;
            request_args[seq] = nil;
        } else {
            /* This is an internal error of some kind. If bad args to send_request cause this, it's a bug in avchat. */
            error("No sequence number found in reply: " + line);
        }
        return MODE_NOCHANGE;
    }
}
