/*
** ~DevSys/sys/avchat.c
**
** Copyright ChatTheatre, 2021
*/

# include <kernel/user.h>
# include <System/log.h>

# define LOG_MESSAGE(s)  SYSLOG(LOG_WARNING, "AVChat: " + s + "\n")

inherit "/usr/System/lib/outboundapi";

private int is_connected;
private int counter;
private mapping requests;
private mapping request_funcs;

/* Default to 600 seconds for valid JSON token time */
# define DEFAULT_VALID_PERIOD 600

void create() {
    is_connected = 0;
    counter = 0;
    requests = ([ ]);
    request_funcs = ([ ]);

    find_or_load("/usr/DevSys/sys/avchat_port");
}

void connected() {
    LOG_MESSAGE("connected!");
    is_connected = 1;
}

int check_connected() {
    return is_connected;
}

void disconnected() {
    LOG_MESSAGE("disconnected!");
    is_connected = 0;
}

/*** Queries ***/

private int
send_token_request(object reply_to, string cb_func,
    string display_name,
    string channel_name,
    int valid_for);

void
token_for_channel_and_name(string display_name, string channel_name, object cb_object, string cb_func)
{
    send_token_request(cb_object, cb_func, display_name, channel_name, DEFAULT_VALID_PERIOD);
}

/*** Low-level I/O functions ***/

/* Send a non-moderator token request using validFor rather than validUntil */
private int
send_token_request(object reply_to, string cb_func,
    string display_name,
    string channel_name,
    int valid_for)
{
    string cas_msg;
    string nul_char;

    LOG_MESSAGE("Sending token request (" + counter + ") for user " + display_name + " and channel " + channel_name);

    cas_msg = "{ \"cmd\": \"jwt_token\", \"displayName\": \"" + display_name +
        "\", \"channel\": \"" + channel_name +
        "\", \"moderator\": false, \"validFor\": " + (string)valid_for +
        ", \"seq\": " + (string)counter +
        " }";
    requests[counter] = reply_to;
    request_funcs[counter] = cb_func;
    call_out("timeout", 60, counter);
    counter += 1;

    nul_char = " ";
    nul_char[0] = 0;

    "~DevSys/sys/avchat_port"->message(cas_msg + nul_char);
}

static
void timeout(int counter) {
    object reply_to;
    string reply_func;

    /* If this request is still pending... */
    if (reply_to = requests[counter]) {
        reply_func = request_funcs[counter];
        LOG_MESSAGE("Timeout for token request: " + counter + ", reply_func: " + reply_func);

        call_other(reply_to, reply_func, 0, "TIMEOUT", nil);
        requests[counter] = nil;
        request_funcs[counter] = nil;
    }
}

int
receive_message(string line) {
    object reply_to;
    string reply_func;
    int seq;

    if(previous_program() == "/usr/DevSys/sys/avchat_port") {
        /* LOG_MESSAGE("Token request reply: " + line); */
        if (sscanf(line, "%*s\"seq\":%d", seq) == 2) {
            reply_to = requests[seq];
            reply_func = request_funcs[seq];
            if (sscanf(line, "%*s\"success\":true")) {
                string token;
                if (sscanf(line, "%*s\"token\":\"%s\"", token) == 2) {
                    call_other(reply_to, reply_func, 1, "OK", token);
                } else {
                    LOG_MESSAGE("Token reply marked successful, but no token found!");
                    call_other(reply_to, reply_func, 0, "NO TOKEN FOUND", nil);
                }
            } else {
                /* TODO: extract message from token server reply */
                call_other(reply_to, reply_func, 0, "Failure from token server: " + line, nil);
                /* Failed */
                LOG_MESSAGE("Failure from token server: " + line);
            }
            requests[seq] = nil;
            request_funcs[seq] = nil;
        } else {
            /* This is an internal error of some kind. If bad args to send_request cause this, it's a bug in avchat. */
            LOG_MESSAGE("No sequence number found in reply: " + line);
        }
        return MODE_NOCHANGE;
    }
}
