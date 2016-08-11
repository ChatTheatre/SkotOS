/*
 * notes.c
 *
 * Copyright Skotos Tech Inc 2004
 */

# include <base.h>
# include <dp_handler.h>

string query_derived_root() {
   return "notes";
}

static void create() {
   BASE_PROPERTIES->register_root_handler("notes", this_object());
}

mapping r_query_properties(object ob, string root, int derived) {
   if (root == "notes" && derived) {
      int i, sz_i;
      string *cats;
      mapping map;

      cats = ob->query_cats();
      map = ([ "notes:categories": cats ]);
      if (cats && (sz_i = sizeof(cats)) > 0) {
	 for (i = 0; i < sz_i; i++) {
	    int j, sz_j, *indices;

	    switch (ob->query_state_by_cat(cats[i])) {
	    case NOTE_CAT_OPEN:
	       map["notes:cat:" + cats[i] + ":state"] = "Open";
	       break;
	    case NOTE_CAT_COMMENT:
	       map["notes:cat:" + cats[i] + ":state"] = "Comment";
	       break;
	    case NOTE_CAT_CLOSE:
	       map["notes:cat:" + cats[i] + ":state"] = "Close";
	       break;
	    }
	    indices = ob->query_indices_by_cat(cats[i]);
	    sz_j = sizeof(indices);
	    for (j = 0; j < sz_j; j++) {
	       switch (ob->query_state(indices[j])) {
	       case NOTE_CAT_OPEN:
		  map["notes:cat:" + cats[i] + ":note:" + indices[j] + ":state"] = "Open";
		  break;
	       case NOTE_CAT_COMMENT:
		  map["notes:cat:" + cats[i] + ":note:" + indices[j] + ":state"] = "Comment";
		  break;
	       case NOTE_CAT_CLOSE:
		  map["notes:cat:" + cats[i] + ":note:" + indices[j] + ":state"] = "Close";
		  break;
	       }
	       map["notes:cat:" + cats[i] + ":note:" + indices[j] + ":user"] = ob->query_user(indices[j]);
	       map["notes:cat:" + cats[i] + ":note:" + indices[j] + ":text"] = ob->query_note_description(indices[j]);
	    }
	 }
      }
      return map;
   }
   return ([ ]);
}
