/*
 * notes.c
 *
 * Copyright Skotos Tech Inc 2004
 */

inherit "/lib/string";
inherit "/lib/type";

# include <base.h>
# include <core.h>
# include <dp_handler.h>
# include <notes.h>

string query_derived_root() {
   return "notes";
}

static void create() {
   BASE_PROPERTIES->register_root_handler("notes", this_object());
   CORE_DP_REGISTRY->register_root_handler("notes", this_object());
}

mapping r_query_properties(object ob, string root, int derived) {
   if (root == "notes" && derived) {
      int i, sz_i;
      string *cats, *lcats;
      mapping map;

      cats = ob->query_cats();

      sz_i = sizeof(cats);
      lcats = allocate(sz_i);
      for (i = 0; i < sz_i; i++) {
	 lcats[i] = lower_case(cats[i]);
      }

      map = ([ "categories": lcats ]);
      if (sz_i > 0) {
	 for (i = 0; i < sz_i; i++) {
	    int j, sz_j, *indices;
	    string cat;

	    cat = lower_case(cats[i]);
	    switch (ob->query_state_by_cat(cats[i])) {
	    case NOTE_CAT_OPEN:
	       map["cat:" + cat + ":state"] = "Open";
	       break;
	    case NOTE_CAT_COMMENT:
	       map["cat:" + cat + ":state"] = "Comment";
	       break;
	    case NOTE_CAT_CLOSE:
	       map["cat:" + cat + ":state"] = "Close";
	       break;
	    }
	    indices = ob->query_indices_by_cat(cats[i]);
	    map["cat:" + cat + ":notes"] = Arr(map["cat:" + cat + ":notes"]) + indices;
	    sz_j = sizeof(indices);
	    for (j = 0; j < sz_j; j++) {
	       switch (ob->query_state(indices[j])) {
	       case NOTE_CAT_OPEN:
		  map["cat:" + cat + ":note:" + indices[j] + ":state"] = "Open";
		  break;
	       case NOTE_CAT_COMMENT:
		  map["cat:" + cat + ":note:" + indices[j] + ":state"] = "Comment";
		  break;
	       case NOTE_CAT_CLOSE:
		  map["cat:" + cat + ":note:" + indices[j] + ":state"] = "Close";
		  break;
	       }
	       map["cat:" + cat + ":note:" + indices[j] + ":user"] = ob->query_user(indices[j]);
	       map["cat:" + cat + ":note:" + indices[j] + ":text"] = ob->query_note_description(indices[j]);
	    }
	 }
      }
      return map;
   }
   return ([ ]);
}

mixed r_query_property(object ob, string root, string prop) {
   if (root == "notes") {
      string cat, rest;

      if (prop == "categories") {
	 int i, sz;
	 string *cats, *lcats;

	 cats = ob->query_cats();
	 sz = sizeof(cats);
	 lcats = allocate(sz);
	 for (i = 0; i < sz; i++) {
	    lcats[i] = lower_case(cats[i]);
	 }
	 return lcats;
      } else if (sscanf(prop, "cat:%s:%s", cat, rest) == 2) {
	 int note;
	 string detail;

	 if (rest == "notes") {
	    int i, sz, *indices;
	    string *cats;

	    indices = ({ });
	    cats = ob->query_cats();
	    sz = sizeof(cats);
	    for (i = 0; i < sz; i++) {
	       if (lower_case(cats[i]) == cat) {
		  indices |= ob->query_indices_by_cat(cats[i]);
	       }
	    }
	    return indices;
	 } else if (sscanf(rest, "note:%d:%s", note, detail) == 2) {
	    string notecat;

	    notecat = ob->query_cat(note);
	    if (notecat && lower_case(notecat) == cat) {
	       switch (detail) {
	       case "state":
		  switch (ob->query_state(note)) {
		  case NOTE_CAT_OPEN:
		     return "Open";
		  case NOTE_CAT_COMMENT:
		     return "Comment";
		  case NOTE_CAT_CLOSE:
		     return "Close";
		  }
		  break;
	       case "text":
		  return ob->query_note_description(note);
	       case "user":
		  return ob->query_user(note);
	       }
	    }
	 }
      }
   }
}

int r_set_property(object ob, string root, string prop, mixed value,
		   mixed *retvalue) {
   if (root == "notes") {
      int ix, state_nr;
      string cat, state, user, field;
      mixed  text;

      if (sscanf(prop, "cat:%s:note:%d:%s", cat, ix, field) == 3) {
	 if (value == nil) {
	    ob->del_note(ix);
	 } else {
	    user     = ob->query_user(ix);
	    user     = user ? user : "- not set -";
	    text     = ob->query_note_description(ix);
	    text     = text ? text : "- not set -";
	    state_nr = ob->query_state(ix);
	    switch (field) {
	    case "state":
	       switch (lower_case(value)) {
	       case "open":
		  state_nr = NOTE_CAT_OPEN; break;
	       case "close":
		  state_nr = NOTE_CAT_CLOSE; break;
	       case "comment":
		  state_nr = NOTE_CAT_COMMENT; break;
	       }
	       break;
	    case "user":
	       user = value;
	       break;
	    case "text":
	       text = value;
	       break;
	    }
	    ob->set_note(ix, user, cat, state_nr, text);
	 }
      } else if (sscanf(prop, "cat:%s:note:%s", cat, field)) {
	 if (value) {
	    state_nr = NOTE_CAT_OPEN;
	    user = "- not set -";
	    text = "- not set -";
	    switch (field) {
	    case "state":
	       switch (lower_case(value)) {
	       case "open":
		  state_nr = NOTE_CAT_OPEN;
		  break;
	       case "close":
		  state_nr = NOTE_CAT_CLOSE;
		  break;
	       case "comment":
		  state_nr = NOTE_CAT_COMMENT;
		  break;
	       }
	       break;
	    case "user":
	       user = value;
	       break;
	    case "text":
	       text = value;
	       break;
	    }
	    retvalue[0] = ob->add_note(user, cat, state_nr, text);
	 }
      }
      return TRUE;
   }
}
