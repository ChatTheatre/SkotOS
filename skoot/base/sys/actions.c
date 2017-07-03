/*
**      /base/sys/actions.c
**
**	Maintain a database of action handlers. No resemblance to the
**	original version of this file.
**
**      Copyright Skotos Tech Inc 2000
**
*/

inherit "/lib/string";

mapping handlers;

static
void create() {
   set_object_name("Base:Actions");
   handlers = ([ ]);
}

void patch() {
   set_object_name("Base:Actions");
}

void deregister_handler(string action) {
   if (!action) {
      error("nil?");
   }
   if (!handlers[action]) {
      error("handler already registered");
   }
   handlers[action] = nil;
}

void register_handler(string action, object handler) {
   if (!action) {
      error("nil?");
   }
   if (handlers[action] && handlers[action] != handler) {
      error("handler already registered");
   }
   handlers[action] = handler;
}

object query_handler(string action) {
   return handlers[action];
}

string *query_actions() {
   return map_indices(handlers);
}

mixed query_property(string prop) {
   if (prop) {
      prop = lower_case(prop);
      if (sscanf(prop, "handler:%s", prop)) {
	 if (prop == "*") {
	    return map_values(handlers);
	 }
	 /*
	  * In preparation for future situation where multiple handlers are
	  * possible we'll return this in array form.
	  */
	 return handlers[prop] ? ({ handlers[prop] }) : nil;
      }
      if (prop == "action:*") {
	 return map_indices(handlers);
      }
   }
}
