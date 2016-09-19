/*
**	/usr/SID/sys/sid.c
**
**	We coordinate the SID nodes (~SID/obj/sid.c) here.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <DTD.h>
# include <SID.h>
# include <type.h>
# include <XML.h>

# define XML_PARENT	"xml_parent"

private inherit "/lib/string";

inherit "/usr/DTD/lib/dtd";
inherit "/usr/SID/lib/vaultnode";
inherit "/usr/SID/lib/stateimpex";

mapping namespaces;

private void configure_initial_nodes();

static
void create() {
   ::create("/usr/SID/data/sid");

   namespaces = ([ ]);
   set_object_name("SID:Daemon");

   compile_object(SID_NODE);

   configure_initial_nodes();
}

int boot(int block) {
   ::create("/usr/SID/data/sid");

   claim_node("SID");

   INFO("Loading SID files");

   "~System/sys/tool/vault"->load_subdir("SID:System");
   "~System/sys/tool/vault"->load_subdir("SID:Vault");
   "~System/sys/tool/vault"->load_subdir("SID");

   "~System/sys/tool/vault"->queue_signal("archives_parsed");

   return 60;
}

void archives_parsed() {
   "/usr/SID/initd"->daemon_done();
}

nomask void patch() {
   set_object_name("SID:Daemon");
}

# define Node(a,b) \
  ((tmp = find_object("SID:" + (a) + ":" + (b))) ? \
   (tmp->clear_element(), tmp->set_name(a, b), tmp) : \
   (tmp = clone_object(SID_NODE), tmp->set_name(a, b), tmp))

/* this can be run if somebody edits a SID:SID:* element badly */

void patch_elements() {
   configure_initial_nodes();
}

private
void configure_initial_nodes() {
   object tmp, element, children, attributes, iterator, callbacks;

   {
      object child;

      child = Node("SID", "Child");

      child->add_attribute("node", LPC_OBJ);
      child->set_iterator("node", "query_children");
      child->add_callback("add_child", "node");

      children = Node("SID", "Children");
      children->add_child(child);
      children->add_callback("clear_children");
   }

   {
      object attribute;

      attribute = Node("SID", "Attribute");
      attribute->add_attribute("id", LPC_STR);
      attribute->set_iterator("id", "query_attributes");
      attribute->add_attribute("atype", LPC_STR,
			       "query_attribute_type", "id");
      attribute->add_attribute("acomment", LPC_STR,
			       "query_attribute_comment", "id");
      attribute->add_attribute("areadonly", XML_BOOL,
			       "query_attribute_readonly", "id");
      attribute->add_attribute("aquery", LPC_STR,
			       "make_attribute_query", "id");
      attribute->add_callback("add_made_attribute", "id", "atype", "aquery");
      attribute->add_callback("set_attribute_comment", "id", "acomment");
      attribute->add_callback("set_attribute_readonly", "id", "areadonly");

      attributes = Node("SID", "Attributes");
      attributes->add_child(attribute);
      attributes->add_callback("clear_attributes");
   }

   {
      object callback;

      callback = Node("SID", "Callback");
      callback->add_attribute("call", LPC_STR);
      callback->set_iterator("call", "make_callbacks");
      callback->add_callback("add_made_callback", "call");

      callbacks = Node("SID", "Callbacks");
      callbacks->add_child(callback);
      callbacks->add_callback("clear_callbacks");
   }

   iterator = Node("SID", "Iterator");
   iterator->add_attribute("var", LPC_STR, "query_iterator_variable");
   iterator->add_attribute("call", LPC_STR, "make_iterator_call");
   iterator->add_callback("set_made_iterator", "var", "call");

   element = Node("SID", "Element");
   element->add_attribute("type", LPC_STR, "query_type");
   element->add_attribute("ns", LPC_STR, "query_namespace");
   element->add_attribute("tag", LPC_STR, "query_tag");
   element->add_attribute("recpoint", XML_BOOL, "query_recursion_point");
   element->add_attribute("transient", XML_BOOL, "query_transient");
   element->add_attribute("query", LPC_STR, "make_query");
   element->add_attribute("newitem", LPC_STR, "make_newitem");
   element->add_attribute("delitem", LPC_STR, "make_delitem");
   element->add_attribute("comment", LPC_STR, "query_comment");
   element->add_child(children);
   element->add_child(attributes);
   element->add_child(iterator);
   element->add_child(callbacks);
   element->add_callback("set_name", "ns", "tag");
   element->add_callback("set_recursion_point", "recpoint");
   element->add_callback("set_transient", "transient");
   element->add_callback("set_made_leaf", "type", "query");
   element->add_callback("set_made_newitem", "newitem");
   element->add_callback("set_made_delitem", "delitem");
   element->add_callback("set_comment", "comment");
}


/* temporary helpers */

mixed *export_state(mixed args...) {
   return ::export_state(args...);
}

void import_state(mixed args...) {
   ::import_state(args...);
}

object query_node(string name, varargs string extra) {
   mapping map;
   string space, tag;

   if (extra != nil) {
      space = name; tag = extra;
   } else if (!sscanf(name, "%s:%s", space, tag)) {
      /* TODO */
      space = "X"; tag = name;
   }
   if (map = namespaces[lower_case(space)]) {
      return map[lower_case(tag)];
   }
}

object get_node(string name, varargs string extra) {
   object ob;

   /* TODO */
   if (name == "SkotOS:Socials:AdvCat") {
      name = "Socials:AdvCat";
   }

   if (ob = query_node(name, extra)) {
      return ob;
   }
   error("node " + name + " unknown");      
}

object get_root_node(object ob) {
   string el;

   if (el = ob->query_state_root()) {
      return get_node(el);
   }
   error("no SID root node defined for object");
}

void register(string space, string tag) {
   mapping map;

   if (!space || !tag) {
      error("bad arguments");
   }
   space = lower_case(space);
   map = namespaces[space];
   if (!map) {
      map = namespaces[space] = ([ ]);
   }
   tag = lower_case(tag);
   if (map[tag] && map[tag] != previous_object()) {
      error("tag already exists");
   }
   map[tag] = previous_object();
}

void deregister(string space, string tag) {
   mapping map;

   if (map = namespaces[lower_case(space)]) {
      map[lower_case(tag)] = nil;
   }
}

string *query_namespaces() {
   return map_indices(namespaces);
}

string *query_tags(string space) {
   mapping map;

   if (map = namespaces[lower_case(space)]) {
      return map_indices(map);
   }
   return nil;
}
