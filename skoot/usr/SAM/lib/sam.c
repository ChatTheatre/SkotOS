/*
**	/usr/SAM/lib/sam.c
**
**	This program scans a file for macros and control structures
**	of various kinds and expands them. The primary use for this
**	functionality is in populating static HTML templates with
**	dynamic data from the runtime system.
**
**	A brief 'language manual' will be written once the syntax is
**	frozen. For now, examine files with the .sam suffix e.g. in
**	the /data/www directory and subdirectories.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <SAM.h>
# include <XML.h>

private inherit "/lib/file";
private inherit "/lib/string";
private inherit "/lib/date";

inherit "/usr/XML/lib/entities";
private inherit "/usr/XML/lib/xmd";
private inherit "/usr/DTD/lib/dtd";

# include "/usr/SAM/include/sam.h"

static
mixed *copy_map_into_map(mapping a, mapping b);

static
void restore_map(mapping a, mixed *old);


static
string unsam(mixed xml, varargs mapping args, object user, object this) {
   object obj;
   string res;
   mixed old_html;

   if (args) {
      old_html = args["HTML"];
      args["HTML"] = nil;
   } else {
      args = ([ ]);
      old_html = nil;
   }
   obj = ::new_object("/usr/SAM/data/unsam");
   obj->set_targets(obj);
   obj->set_this(this);
   if (user) {
      obj->set_user(user);
   }
   obj->set_render(TRUE);
   tf_elt(xml, obj, args);
   obj->flush_output();
   /* Restore HTML parameter. */
   args["HTML"] = old_html;
   return implode(obj->query_chunks(), "");
}

static
void sam(mixed xml, object target, varargs mapping args, object user,
	 object this) {
   object obj;

   obj = ::new_object("/usr/SAM/data/context");
   obj->set_targets(target);
   obj->set_this(this);
   if (user) {
      obj->set_user(user);
   }
   obj->set_render(TRUE);

   tf_elt(xml, obj, args ? args : ([ ]));

   obj->flush_output();
}

static
void transform_to_html(mixed xmd, object context, mapping args) {
   context->set_render(TRUE);
   context->set_html(TRUE);
   tf_elt(xmd, context, args + ([ "HTML": TRUE ]));
   context->flush_output();
}

/* macros to help reduce stack_depth in tf_elt recursion */

# define Recurse(a, b, c)	tf_elt(a, b, c)	/* optimize later */

# define TfPCData() {				\
   int i;					\
						\
   elt = query_colour_value(elt);		\
   if (typeof(elt) == T_STRING) {		\
      Output(elt);				\
   } else if (typeof(elt) == T_ARRAY) {		\
      for (i = 0; i < sizeof(elt); i ++) {	\
         Recurse(elt[i], context, args);        \
      }						\
   }						\
}


# define RetPCData() {						\
   string *bits;						\
   int old, i;							\
								\
   elt = query_colour_value(elt);				\
   if (typeof(elt) == T_STRING) {				\
      return elt;						\
   }								\
   bits = allocate(sizeof(elt));				\
   old = context->query_render();				\
   for (i = 0; i < sizeof(elt); i ++) {				\
      context->set_render(FALSE);					\
      bits[i] = untyped_to_ascii(tf_elt(elt[i], context, args));	\
   }								\
   context->set_render(old);					\
   return implode(bits, "");					\
}

# define ReproduceTag() {				\
   string tag, *argix;					\
   int i;						\
							\
   tag = "<" + xmd_element(elt);					\
							\
   argix = map_indices(local);				\
   for (i = 0; i < sizeof(argix); i ++) {		\
      tag += " " + argix[i] + "=\"" +			\
	 untyped_to_ascii(local[argix[i]]) + "\"";	\
   }							\
   Output(tag + ">");					\
							\
   if (xmd_content(elt)) {				\
      Recurse(xmd_content(elt), context, args);		\
   }							\
   Output("</" + xmd_element(elt) + ">");		\
}

mixed *evaluate_arguments(object context, mixed *attr,
			  mapping local, mapping args) {
   mixed *save;
   int old, i;

   /* first copy the current attribute arguments from 'args' mapping */
   save = attr[..];

   for (i = 0; i < sizeof(save); i += 2) {
      save[i+1] = args[save[i]];
   }

   /* then evaluate the new attributes and copy them into 'args' mapping */
   old = context->query_render();
   for (i = 0; i < sizeof(attr); i += 2) {
      context->set_render(FALSE);
      local[attr[i]] = args[attr[i]] = tf_elt(attr[i+1], context, args);
   }
   context->set_render(old);
   return save;
}

void restore_arguments(mixed *save, mapping args) {
   int i;

   for (i = 0; i < sizeof(save); i += 2) {
      args[save[i]] = save[i+1];
   }
}

static
mixed tf_elt(mixed elt, object context, mapping args) {
   mapping local;
   string str;
   object ob;
   mixed  elt_data;

   local = ([ ]);

   elt_data = query_colour_value(elt);
   switch(query_colour(elt)) {
   case COL_SAMREF: {
      mixed *save, tmp;

      save = evaluate_arguments(context, xmd_ref_attributes(elt), local, args);

      tmp = SAMD->resolve_reference(elt_data[0], context, args, local);
      if (context->query_render()) {
	 tmp = tf_elt(tmp, context, args);
      }
      restore_arguments(save, args);
      return tmp;
   }
   case COL_PCDATA:
      if (context->query_render()) {
	 TfPCData();
	 return "N/A";
      }
      RetPCData();
   case COL_ELEMENT: {
      mixed *save;

      save = evaluate_arguments(context, xmd_attributes(elt), local, args);

      if (!context->query_render()) {
	 /* context is only false when called from resolve_arguments */
	 error("context nil for element " + elt_data[0]);
      }
      {
	 string *handlers;
	 int i;

	 /* TODO: temporary */
	 handlers = ({ "SID", "DevSys", "HTTP", "SAM", "SkotOS", "System", "Bane", "Marrach", "TextIF", "Theatre", "Tool", "UserAPI", "UserDB" });

	 for (i = 0; i < sizeof(handlers); i ++) {
	    ob = ::find_object("/usr/" + handlers[i] + "/samtags/" + elt_data[0]);
	    if (!ob) {
	       ob = SAMTAGS->query_handler(elt_data[0]);
	    }
	    if (ob && function_object("tag", ob) &&
		ob->tag(elt_data[2], context, local, args)) {
	       restore_arguments(save, args);
	       return context;
	    }
	 }
      }
      ReproduceTag();
      restore_arguments(save, args);
      return context;
   }
   default:
      if (typeof(elt_data) == T_STRING && !context->query_html() && elt_data == "\n") {
	 /* if normal text, ignore linefeeds */
	 elt_data = " ";
      }
      if (context->query_render()) {
	 if (elt_data != nil) {
	    Output(untyped_to_ascii(elt_data));
	 }
	 return "N/A";
      }
      return elt;
   }
}

/* toolbox */

static
mixed *copy_map_into_map(mapping a, mapping b) {
   mixed *old, *ix;
   int i;

   old = allocate(map_sizeof(a) * 2);
   ix = map_indices(a);

   for (i = 0; i < sizeof(ix); i ++) {
      old[i]   = ix[i];
      old[i+1] = b[ix[i]];
      b[ix[i]] = a[ix[i]];
   }
   return old;
}

static
void restore_map(mapping a, mixed *old) {
   int i;

   for (i = 0; i < sizeof(old); i += 2) {
      a[old[i]] = old[i+1];
   }
}

