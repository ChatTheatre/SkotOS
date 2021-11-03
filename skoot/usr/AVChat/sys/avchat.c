/*
** ~AVChat/sys/avchat.c
**
** Copyright ChatTheatre, 2021
*/

# include <kernel/user.h>

inherit "/usr/System/lib/outboundapi";

private int is_connected;
private int counter;
private mapping requests;

void create() {
    is_connected = 0;
    counter = 0;
    requests = ([ ]);
}

void connected() {
    is_connected = 1;
}

void disconnected() {
    is_connected = 0;
}

private int
send_request(object reply_to, string display_name, string channel_name, int valid_until)
{
    string cas_msg = "{ displayName: \"" + display_name +
        "\", channel: \"" + channel_name +
        "\", moderator: false, validUntil: " + valid_until.toString() +
        ", seq: " + counter.toString() +
        " }";
    requests[counter] = reply_to;
    counter += 1;
    /* TODO: sizeof? */
    int l = sizeof(cas_msg);

    "~AVChat/sys/avchat_port"->message(l.toString() + NUL_CHAR + cas_msg);
}

int
receive_message(string line) {
    object reply_to;
    int seq;

    if(previous_object() == "~AVChat/sys/avchat_port") {
        if (sscanf(line, "seq: %d", seq) == 1) {
            reply_to = requests[seq];
            if (sscanf(line, "success: true")) {

            } else {
                /* Failed */
                error("Failure from token server: " + line);
            }
            requests[seq] = nil;
        } else {
            error("No sequence number found in reply: " + line);
        }
        return MODE_NOCHANGE;
    }
}
