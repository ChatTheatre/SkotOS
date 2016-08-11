/*
**      /usr/SAM/sys/tags.c
**
**	Maintain a database of SAM tags.
**
**      Copyright Skotos Tech Inc 2003
**
*/

private inherit "/lib/string";

mapping handlers;

static
void create() {
   handlers = ([ ]);
}

void deregister_handler(string tag_name) {
   if (!tag_name) {
      error("nil?");
   }
   tag_name = lower_case(tag_name);
   if (!handlers[tag_name]) {
      error("handler already registered");
   }
   SysLog("Deregistering TAG handler[" + tag_name + "]");
   handlers[tag_name] = nil;
}

void register_handler(string tag_name, object handler) {
   if (!tag_name) {
      error("nil?");
   }
   tag_name = lower_case(tag_name);
   if (handlers[tag_name] && handlers[tag_name] != handler) {
      error("handler already registered");
   }
   SysLog("Registering TAG handler[" + tag_name + "]");
   handlers[tag_name] = handler;
}

object query_handler(string tag_name) {
   return handlers[lower_case(tag_name)];
}

string *query_tag_names() {
   return map_indices(handlers);
}
