/*
**	/usr/SkotOS/obj/mersamtag.c
**
**	MerSAMTag, Merry SAM Tag, a plugin onject conforming to the
**	interface required by the SAM engine.
*/

# include <base.h>
# include <core.h>
# include <type.h>
# include <SAM.h>
# include <XML.h>

private inherit sam   "/usr/SAM/lib/sam";
private inherit       "/usr/SkotOS/lib/merryapi";
        inherit       "/usr/SkotOS/lib/describe";
        inherit props "/core/lib/core_object";

# include "/usr/SAM/include/sam.h"

string tag_name;

string query_state_root() { return "SkotOS:MerSAMTag"; }

string query_script_registry() {
   return BASE_INITD;
}

string query_dp_registry() {
   return CORE_DP_REGISTRY;
}

static
void create(int clone) {
   props :: create();
   sam   :: create();
   if (!clone) {
      set_object_name("SkotOS:MerSAMTag");
   }
}

string query_tag_name() {
   return tag_name;
}

atomic
void set_tag_name(string s) {
   if (tag_name) {
      SAMTAGS->deregister_handler(tag_name);
   }
   tag_name = s;
   if (tag_name) {
      while (SAMTAGS->query_handler(tag_name)) {
         tag_name += "_backup";
      }
      SAMTAGS->register_handler(tag_name, this_object());
   }
}


int tag(SAM content, object context, mapping local, mapping args) {
   string *ix;
   mixed *save, code, val;
   int i;

   ix = map_indices(local);

   /* if $foo was set locally, also set $(local:foo) */

   save = allocate(sizeof(ix)*2);
   for (i = 0; i < sizeof(ix); i ++) {
      save[i*2+0] = ix[i];
      save[i*2+1] = args["local:" + ix[i]];
      args["local:" + ix[i]] = args[ix[i]];
   }
   args["tag-content"] = content;

   code = query_property("sam:global:tag");

   if (code == nil) {
      /* there is no SAM definition */

      code = run_merry(this_object(), "tag", "global", args);

      if (code == TRUE) {
	 /* if there was no script, use the content */
	 code = content;
      }
   }

   if (typeof(code) == T_STRING) {
      content = code;
   } else if (typeof(code) == T_NIL) {
      content = nil;
   } else {
      object data, *oldtargets;

      data = ::new_object("/data/data");

      oldtargets = context->query_targets();

      args["tag-code"] = this_object();

      context->set_targets(data);

      tf_elt(code, context, args);

      context->flush_output();

      content = implode(data->query_chunks(), "");
      args["tag-code"] = nil;

      context->set_targets(oldtargets...);
   }
   args["tag-content"] = content;
   val = run_merry(this_object(), "tag", "global-post", args);

   if (val != TRUE && val != FALSE && val != nil) {
      /* merry script is really returning something */
      if (typeof(val) != T_STRING) {
	 error("merry:tag:global-post must return a string");
      }
      content = val;
   }

   if (content != nil) {
      if (typeof(content) != T_STRING) {
	 error("SAM tag content not a string");
      }
      Output(content);
   }

   for (i = 0; i < sizeof(save); i += 2) {
      args["local:" + save[i]] = save[i+1];
   }
   return TRUE;
}


/* this is an open relay -- currently called from merrynode */

void delayed_call(object ob, string fun, mixed delay, mixed args...) {
   call_out("perform_delayed_call", delay, ob, fun, args);
}

static
void perform_delayed_call(object ob, string fun, mixed *args) {
   call_other(ob, fun, args...);
}

void die() {
    destruct_object();
}

void suicide() {
    call_out("die", 0);
}
