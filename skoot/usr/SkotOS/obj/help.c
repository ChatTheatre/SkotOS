/*
**	~Skotos/obj/help.c
**
**
**********************************************************************
**	991101 Zell	Initial Revision
**
**	Copyright Skotos Tech Inc 1999
*/

# include <core.h>
# include <base.h>
# include <SkotOS.h>
# include <SAM.h>

inherit "/lib/womble";

private inherit "/lib/string";

inherit core "/core/lib/core_object";

private string	title;
private SAM	description;
private mapping keywords;

private int	dirty_callout;

void womble_description() {
   description = womble(description);
}

# define Dirty() { \
  if (dirty_callout == 0) { \
    dirty_callout = call_out("signal_update", 0); \
  } \
}

void clear_keywords();

string query_state_root() { return "Help:Node"; }

/* creator */

string query_script_registry() {
   return BASE_INITD;
}

string query_dp_registry() {
   return CORE_DP_REGISTRY;
}

void patch_core() {
   core::create();
}

static atomic
void create(int clone) {
   core::create();
   clear_keywords();
   if (!clone) {
      set_object_name("SkotOS:HelpNodes:UrHelp");
   }
}


void clear_keywords() {
   keywords = ([ ]);
}


/* public API */

SAM query_description() {
   return description;
}

void set_description(SAM m) {
   description = m;
   Dirty();
}


string query_title() {
   return title;
}

void set_title(string str) {
   title = str;
   Dirty();
}


string *query_keywords() {
   return map_indices(keywords);
}

int query_keyword(string key) {
   return !!keywords[key];
}

void set_keyword(string keyword) {
   if (keyword) {
      keywords[lower_case(keyword)] = TRUE;
      Dirty();
   }
}

void clear_keyword(string keyword) {
   if (keyword) {
      keywords[lower_case(keyword)] = nil;
      Dirty();
   }
}


static /* atomic */
void signal_update() {
   dirty_callout = 0;
   HELPD->update_help(this_object());
}

mixed
query_downcased_property(string prop) {
   if (sscanf(prop, "help:%s", prop)) {
      switch (prop) {
      case "keywords":
	 return map_indices(keywords);
      case "title":
	 return title;
      case "description":
	 return description;
      default:
	 error("Unknown help: property type: " + prop);
      }
   }
   return ::query_downcased_property(prop);
}

void set_downcased_property(string prop, mixed val, varargs int opaque) {
   if (sscanf(prop, "help:%s", prop)) {
      if (!opaque) {
	 switch (prop) {
	 case "keywords": {
	    int i, sz;

	    clear_keywords();
	    sz = sizeof(val);
	    for (i = 0; i < sz; i++) {
	       set_keyword(val[i]);
	    }
	    break;
	 }
	 case "title":
	    set_title(val);
	    break;
	 case "description":
	    set_description(val);
	    break;
	 default:
	    error("Unknown help: property type: " + prop);
	 }
      }
      return;
   }
   ::set_downcased_property(prop, val, opaque);
}

mapping query_properties(varargs int derived) {
   mapping map;

   map = ::query_properties(derived);
   if (derived) {
      int i, sz;
      string *arr;

      arr = ({ "keywords", "title", "description" });
      sz = sizeof(arr);
      for (i = 0; i < sz; i++) {
	 map["help:" + arr[i]] = query_property("help:" + arr[i]);
      }
   }
   return map;
}
