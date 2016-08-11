/*
**	/base/lib/consent.c
**
**	This code serves as the foundation to the consent system.
**
**	Copyright Skotos Tech Inc 2000
**
**	Initial Revision - Wes Connell
**
*/

inherit "/lib/womble";

private mapping pending_requests; /* requests to violate my prox */
private object prox_target;	/* object I want to approach */

private mapping args;
private string action;		/* machine-readable what I'm trying to do */

private string message;		/* human-readable what I'm trying to do */

private mapping consent_list;

void womble_consent() {
    args = womble(args);
}

static
void create() {
   pending_requests = ([ ]);
   consent_list = ([ ]);
}

mapping query_requests() {
   return pending_requests;
}

string query_request_info(object who) {
   return pending_requests[who];
}

object query_prox_target() {
   return prox_target;
}

void set_prox_target(object who) {
   prox_target = who;
}

void clear_requests() {
   pending_requests = ([ ]);
}

void add_request(object who, string message) {
   pending_requests[who] = message;
}

void remove_request(object who) {
   pending_requests[who] = nil;
}

void set_pending_action(string new_action, mapping new_args) {
   action = new_action;
   args = new_args;
}

string query_pending_action() {
   return action;
}

mapping query_pending_args() {
   return args;
}

void set_consent_message(string str) {
   message = str;
}

string query_consent_message() {
   return message;
}

void add_consent(mixed index, string type) {
   consent_list[index] = type;
}

void remove_consent(mixed index) {
   consent_list[index] = nil;
}

void clear_consent() {
   consent_list = ([ ]);
}

mapping query_consent() {
   return consent_list;
}


int check_auto_allow(object who) {
   return consent_list["all"] == "allow" || consent_list[who] == "allow";
}

int check_auto_deny(object who) {
   return consent_list["all"] == "deny" || consent_list[who] == "deny";
}
