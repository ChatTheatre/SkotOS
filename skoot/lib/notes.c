/*
**	This program, inherited by programs that export state, supplies
**	a note-keeping functionality to their inheritor. This is mainly
**	used for i.e. quality-control purposes.
**
**	Implemention-wise, this is not entirely dissimilar to logd.c
**
**	Inheritor must call notes::create() as well as notes::setup_state()
**	-after- state::new_state().
**
**	Copyright Skotos Tech Inc 1999
*/

inherit "/lib/womble";

# include <limits.h>
# include <notes.h>

private mapping ix_to_cat;
private mapping ix_to_desc;
private mapping ix_to_state;
private mapping ix_to_user;

private mapping cat_to_ixset;
private mapping cat_to_state;

void womble_notes() {
   ix_to_desc = womble(ix_to_desc);
}

static
void create() {
   ix_to_cat = ([ ]);
   ix_to_desc = ([ ]);
   ix_to_state = ([ ]);
   ix_to_user = ([ ]);

   cat_to_ixset = ([ ]);
   cat_to_state = ([ ]);
}

atomic
void del_note(int ix) {
   mapping set;
   string cat;
   int *ixarr, state, i;

   ix_to_desc[ix] = nil;
   ix_to_state[ix] = nil;

   if (cat = ix_to_cat[ix]) {
      ix_to_cat[ix] = nil;

      if (set = cat_to_ixset[cat]) {
	 set[ix] = nil;
	 cat_to_state[cat] = NOTE_CAT_OPEN;

	 if (map_sizeof(set)) {
	    ixarr = map_indices(set);
	    for (i = sizeof(ixarr)-1; i >= 0; i --) {
	       state = ix_to_state[ixarr[i]];
	       if (state == NOTE_CAT_OPEN || state == NOTE_CAT_CLOSE) {
		  cat_to_state[cat] = state;
		  break;
	       }
	    }
	 } else {
	    cat_to_ixset[cat] = nil;
	 }
      }
   }
}

atomic
void set_note(int ix, string user, string cat, int state, mixed desc) {
   if (!user) {
      error("missing user argument");
   }
   if (!cat) {
      error("missing category argument");
   }

   /* unlink old cat */
   if (ix_to_cat[ix] && cat_to_ixset[ix_to_cat[ix]]) {
      cat_to_ixset[ix_to_cat[ix]][ix] = nil;
   }
   /* insert new cat */
   if (cat_to_ixset[cat]) {
      cat_to_ixset[cat][ix] = TRUE;
   } else {
      cat_to_ixset[cat] = ([ ix: TRUE ]);
   }
   /* finish link */
   ix_to_cat[ix] = cat;

   /* write data */
   ix_to_desc[ix] = desc;
   ix_to_state[ix] = state;
   ix_to_user[ix] = user;
   if (state == NOTE_CAT_CLOSE || state == NOTE_CAT_OPEN) {
      cat_to_state[cat] = state;
   }
}

/* TODO; we should test the rollover */
atomic
int add_note(string user, string cat, int state, mixed desc) {
   int ix;

   for (ix = time(); ix_to_cat[ix]; ix ++);

   set_note(ix, user, cat, state, desc);

   return ix;
}

/* Reinitialize notes data entirely. */
void clear_categories() {
   ix_to_cat = ([ ]);
   ix_to_desc = ([ ]);
   ix_to_state = ([ ]);
   ix_to_user = ([ ]);

   cat_to_ixset = ([ ]);
   cat_to_state = ([ ]);
}

atomic
void clear_category(string cat) {
   mapping set;
   int *ix, i;

   if (set = cat_to_ixset[cat]) {
      ix = map_indices(set);

      for (i = 0; i < sizeof(ix); i ++) {
	 ix_to_cat[ix[i]] = nil;
	 ix_to_desc[ix[i]] = nil;
	 ix_to_state[ix[i]] = nil;
	 ix_to_user[ix[i]] = nil;
      }
      cat_to_ixset[cat] = nil;
      cat_to_state[cat] = nil;
   }
}

string query_user(int ix) {
   return ix_to_user[ix];
}

mixed query_note_description(int ix) {
   return ix_to_desc[ix];
}

atomic
void set_note_description(int ix, mixed desc) {
   /* make sure this is an existing note */
   if (!ix_to_desc[ix]) {
      error("unknown note index " + ix);
   }
   ix_to_desc[ix] = desc;
}

int query_state(int ix) {
   return ix_to_state[ix];
}

string *query_cats() {
   return map_indices(cat_to_ixset);
}

int *query_indices_by_cat(string cat) {
   if (cat_to_ixset[cat]) {
      return map_indices(cat_to_ixset[cat]);
   }
}

int query_state_by_cat(string cat) {
   if (cat_to_state[cat]) {
      return cat_to_state[cat];
   }
   return NOTE_CAT_OPEN;
}

string query_cat(int ix) {
   return ix_to_cat[ix];
}
