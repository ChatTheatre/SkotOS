/*
**	/usr/SID/lib/sidnode.c
**
**	This is the repository of state interface definitions for
**	various classes out there. A SID determines what functions
**	may be called in an object to acquire its state as well as
**	supplies some structural and datatype information.
**
**	SID's are akin to DTD's (Document Type Definitions) for XML
**	and indeed, we can (and do) generate formal DTD's from SID's
**	for the object state XML.
**
**
**	Copyright Skotos Tech Inc 1999
*/

# include <SID.h>

private inherit "/lib/string";
private inherit "/usr/DTD/lib/dtd";

string	space;
string	tag;
string	type;
string	comment;

int	recursion_point;
int	transient;

string	*query;
object	*children;

string	iterator_var;
string	*iterator_call;
string	*iterator;

string  *attributes;
mapping	attribute_types;
mapping	attribute_query;
mapping	attribute_default;
mapping	attribute_comment;
mapping attribute_ro;

string	*newitem;
string  *delitem;

string	**callbacks;

atomic void clear_element();
void clear_children();
void clear_attributes();
void clear_callbacks();

static string make_call(string *arr);
static string *break_call(string arr);


static
void create() {
   clear_element();
}

void patch() {
   if (!attribute_comment) {
      attribute_comment = ([ ]);
   }
   if (!attribute_ro) {
      attribute_ro = ([ ]);
   }
}

atomic
void clear_element() {
   space = tag = type = nil;
   query = nil;
   recursion_point = FALSE;
   iterator_var = nil;
   iterator_call = nil;
   clear_attributes();
   clear_children();
   clear_callbacks();
   newitem = nil;
   delitem = nil;
}

string query_namespace() {
   return space;
}

string query_tag() {
   return tag;
}

string query_name() {
   if (!space || !tag) {
      error("node not configured");
   }
   return space + ":" + tag;
}

atomic
int set_name(string s, string t) {
   if (!s || !t) {
      error("bad arguments");
   }
   if (s != space || t != tag) {
      if (space && tag) {
	 SID->deregister(space, tag);
      }
      SID->register(s, t);
      space = s; tag = t;
      return TRUE;
   }
   return FALSE;
}

string query_type() {
   return type;
}

void set_comment(string c) {
   comment = c;
}

string query_comment() { return comment; }

int is_parent() {
   return !!sizeof(children);
}

int is_leaf() {
   return !!query;
}

object *query_children() {
   return children[..];
}

string make_query() {
   if (query) {
      return make_call(query);
   }
   return nil;
}

string *query_leaf() {
   if (query) {
      return query[..];
   }
}

void add_child(object ob) {
   if (query) {
      error("object is a leaf node");
   }
   type = nil;
   children += ({ ob });
}

void clear_children() {
   children = ({ });
}


void set_leaf(string t, string q...) {
   if (sizeof(children)) {
      error("object is a parent node");
   }
   type = t;
   query = q;
}

void set_made_leaf(string t, string q) {
   if (t && q) {
      if (sizeof(children)) {
	 error("object is a parent node");
      }
      type = t;
      query = break_call(q);
   } else {
      type = nil;
      query = nil;
   }
}

int query_recursion_point() {
   return recursion_point;
}

void set_recursion_point(int val) {
   recursion_point = val;
}

int query_transient() {
   return transient;
}

void set_transient(int val) {
   transient = val;
}

void clear_attributes() {
   attributes = ({ });
   attribute_types = ([ ]);
   attribute_query = ([ ]);
   attribute_default = ([ ]);
   attribute_comment = ([ ]);
   attribute_ro = ([ ]);
}

string *query_attributes() {
   return attributes[..];
}

string query_attribute_type(string attr) {
   return attribute_types[attr];
}

void set_attribute_comment(string attr, string comment) {
   attribute_comment[attr] = comment;
}

string query_attribute_comment(string attr) {
   return attribute_comment[attr];
}

void set_attribute_readonly(string attr, int state) {
   attribute_ro[attr] = state ? TRUE: nil;
}

int query_attribute_readonly(string attr) {
   return attribute_ro[attr] ? TRUE: FALSE;
}

string *query_attribute_query(string attr) {
   return attribute_query[attr];
}


string make_attribute_query(string attr) {
   string *query;

   if (query = attribute_query[attr]) {
      return make_call(query);
   }
   return nil;
}

mixed query_default_value(string attr) {
   mixed val;

   if (val = attribute_default[attr]) {
      return val;
   }
   if (val = attribute_types[attr]) {
      return default_value(val);
   }
   error("can't figure default value for attribute " + attr);
}

void set_default_value(string attr, mixed def) {
   attribute_default[attr] = def;
}

void add_attribute(string attr, string type, string query...) {
   attributes += ({ attr });
   attribute_types[attr] = type;
   attribute_query[attr] = sizeof(query) ? query : nil;
}

void add_made_attribute(string attr, string type, varargs string query) {
   attributes += ({ attr });
   attribute_types[attr] = type;
   attribute_query[attr] = query ? break_call(query) : nil;
}

string **query_callbacks() {
   return callbacks[..];
}

string *make_callbacks() {
   string *out;
   int i;

   out = allocate(sizeof(callbacks));
   for (i = 0; i < sizeof(out); i ++) {
      out[i] = make_call(callbacks[i]);
   }
   return out;
}

void add_callback(string call...) {
   callbacks += ({ call });
}

void add_made_callback(string call) {
   if (call) {
      callbacks += ({ break_call(call) });
   }
}

void clear_callbacks() {
   callbacks = ({ });
}

string *query_newitem() {
   if (newitem) {
      return newitem[..];
   }
}

string make_newitem() {
   if (newitem) {
      return make_call(newitem);
   }
}

void set_newitem(string call...) {
   newitem = call[..];
}

void set_made_newitem(string call) {
   if (call) {
      newitem = break_call(call);
   }
}

string *query_delitem() {
   if (delitem) {
      return delitem[..];
   }
}

string make_delitem() {
   if (delitem) {
      return make_call(delitem);
   }
}

void set_delitem(string call...) {
   delitem = call[..];
}

void set_made_delitem(string call) {
   if (call) {
      delitem = break_call(call);
   }
}

string query_iterator_variable() {
   return iterator_var;
}
   
string *query_iterator_call() {
   if (iterator_call) {
      return iterator_call[..];
   }
}

string make_iterator_call() {
   if (iterator_call) {
      return make_call(iterator_call);
   }
}

void set_iterator(string var, string call...) {
   iterator_var = var;
   iterator_call = call[..];
}

void set_made_iterator(string var, string call) {
   if (var && call) {
      iterator_var = var;
      iterator_call = break_call(call);
   } else {
      iterator_var = nil;
      iterator_call = nil;
   }
}

/* internal */

static
string make_call(string *arr) {
   if (sizeof(arr) == 1) {
      return arr[0];
   }
   return arr[0] + "(" + implode(arr[1..], ", ") + ")";
}   

static
string *break_call(string call) {
   string fun, argstr, *args;
   int i;

   if (sscanf(call, "%s(%s)", fun, argstr) == 2) {
      args = explode(argstr, ",");
      for (i = 0; i < sizeof(args); i ++) {
	 /* be nice; allow spacing */
	 args[i] = strip(args[i]);
      }
      return ({ fun }) + args;
   }
   if (sscanf(call, "%*s(")) {
      error("expecting closing paranthesis in '" + call + "'");
   }
   return ({ call });
}

void die() { destruct_object(); }
