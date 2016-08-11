/*
**	/base/lib/detail.c
**
**	This is where our very general 'description' notion is implemented.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <nref.h>
# include <base.h>
# include <base/preps.h>

inherit "/lib/womble";
private inherit "/lib/array";
private inherit "/lib/string";
private inherit "/base/lib/urcalls";

# include <urcalls.h>

# include "/base/include/events.h"
# include "/base/include/light.h"

# define FLAG_PRIME			(1 << 0)
# define FLAG_ABSTRACT			(1 << 1)
# define FLAG_DEFINITE			(1 << 2)

# define FLAG_IS_DOOR			(1 << 3)
# define FLAG_IS_CLOSED			(1 << 4)
# define FLAG_IS_LOCKED			(1 << 5)

# define FLAG_IS_HIDDEN 		(1 << 6)

# define FLAG_NEVER_OBVIOUS		(1 << 7)
# define FLAG_OBVIOUS_WHEN_OPEN		(1 << 8)
# define FLAG_HIDDEN_WHEN_CLOSED	(1 << 9)

# define FLAG_SEPARATE			(1 << 10)

# define FLAG_SELF_LOCKING		(1 << 11)
# define FLAG_DEADBOLT			(1 << 12)

# define FLAG_TARGETABLE_IN_DARK	(1 << 13)

# define FLAG_WAS_DOOR			(1 << 14)

# define FLAG_PLURAL			(1 << 15)

private mapping id_to_flags;		/* Id to flag bitmask */
private mapping id_to_preps;		/* Id to Preposition	*/
private mapping id_to_descs;		/* Id to Description	*/
private mapping id_to_snames;		/* Id to Singular Name */
private mapping id_to_pnames;		/* Id to Plural Name */
private mapping id_to_adjs;		/* Id to Adjective */
private mapping id_to_face;		/* Id to Face */
private mapping id_to_dest;		/* Id to Exit Dest */
private mapping id_to_dirs;		/* Id to Exit Dirs */
private mapping id_to_keycode;		/* Id to KeyCode */
private mapping id_to_climbable;	/* Id to Climbable Destinaiton */
private mapping id_to_luminosity;	/* Id to Luminosity */
private mapping sname_to_ids;		/* Singular Name to Id Array	*/
private mapping pname_to_ids;		/* Plural Name to Id Array */
private mapping sname_to_pname;		/* Singular Name to Plural Name */
private mapping dir_to_ids;		/* Direction to Id Array */

mapping query_id_to_flags() { return id_to_flags; }
mapping query_id_to_preps() { return id_to_preps; }
mapping query_id_to_descs() { return id_to_descs; }
mapping query_id_to_snames() { return id_to_snames; }
mapping query_id_to_pnames() { return id_to_pnames; }
mapping query_id_to_adjs() { return id_to_adjs; }
mapping query_id_to_face() { return id_to_face; }
mapping query_id_to_dest() { return id_to_dest; }
mapping query_id_to_dirs() { return id_to_dirs; }
mapping query_id_to_keycode() { return id_to_keycode; }
mapping query_id_to_climbable() { return id_to_climbable; }
mapping query_id_to_luminosity() { return id_to_luminosity; }
mapping query_sname_to_ids() { return sname_to_ids; }
mapping query_pname_to_ids() { return pname_to_ids; }
mapping query_sname_to_pname() { return sname_to_pname; }
mapping query_dir_to_ids() { return dir_to_ids; }

private
string *hidden_filter(string *arr);

int door_closes(string id, varargs int silent);
int door_opens(string id, varargs int silent);
int door_locks(string id, varargs int silent);
int door_unlocks(string id, varargs int silent);

void clear_details();
string *query_details();

static
void create() {
   clear_details();
}

void patch_luminosity() {
   if (!id_to_climbable) {
      id_to_climbable = ([ ]);
   }
   if (!id_to_luminosity) {
      id_to_luminosity = ([ ]);
      this_object()->dirty_local_light_cache();
   }
}
void patch_plural() {
   if (!id_to_pnames) {
      int i, sz_i;
      string *pnames, **ids;

      DEBUG("[patch_plural] " + name(this_object()) + "...");
      id_to_pnames = ([ ]);
      sz_i   = map_sizeof(pname_to_ids);
      pnames = map_indices(pname_to_ids);
      ids    = map_values(pname_to_ids);
      for (i = 0; i < sz_i; i++) {
	 int j, sz_j;

	 sz_j = sizeof(ids[i]);
	 for (j = 0; j < sz_j; j++) {
	    if (id_to_pnames[ids[i][j]]) {
	       id_to_pnames[ids[i][j]] |= ({ pnames[i] });
	    } else {
	       id_to_pnames[ids[i][j]]  = ({ pnames[i] });
	    }
	 }
      }
      sname_to_pname = ([ ]);
   }
}

void womble_details() {
   id_to_dest      = womble(id_to_dest);
   id_to_climbable = womble(id_to_climbable);
   id_to_descs     = womble(id_to_descs);
}

void clear_details() {
   string *ix;
   int i;

   if (!id_to_flags) {
      id_to_flags = ([ ]);
   }
   ix = map_indices(id_to_flags);
   for (i = 0; i < sizeof(ix); i ++) {
      /* for flags, save only CLOSED/LOCK status */
      id_to_flags[ix[i]] &= FLAG_IS_CLOSED | FLAG_IS_LOCKED | FLAG_WAS_DOOR;
   }

   id_to_dirs = ([ ]);
   id_to_dest = ([ ]);
   id_to_keycode = ([ ]);
   id_to_preps = ([ ]);
   id_to_descs = ([ ]);
   id_to_snames = ([ ]);
   id_to_pnames = ([ ]);
   id_to_adjs = ([ ]);
   id_to_face = ([ ]);
   id_to_climbable = ([ ]);

   sname_to_ids = ([ ]);
   pname_to_ids = ([ ]);
   sname_to_pname = ([ ]);
   dir_to_ids = ([ ]);
}   


/* create a list of NRefs from a list of details */
NRef *create_nrefs(string *details) {
   NRef *nrefs;
   int i;

   nrefs = allocate(sizeof(details));
   for (i = 0; i < sizeof(details); i++)
      nrefs[i] = NewNRef(this_object(), details[i]);

   return nrefs;
}

/* add a preposition for a detail */
atomic
void set_id_prep(string id, int prep, int bool) {
   if (id_to_preps[id]) {
      if (bool) {
	 id_to_preps[id] |= prep;
      } else {
	 id_to_preps[id] &= ~prep;
      }
   } else if (bool) {
      id_to_preps[id]  = prep;
   }
}

int query_id_preps(string id) {
   mixed val;

   if (val = id_to_preps[id]) {
      return val;
   }
   return 0;
}

int test_id_prep(string id, int prep) {
   if (id_to_preps[id]) {
      return !!(id_to_preps[id] & prep);
   }
   return 0;
}


/* set a description of given type and face */
atomic
void set_description(string id, string type, mixed desc) {
   mapping descs;

   if (!id) {
      error("bad argument 1 to set_description");
   }
   if (!type) {
      error("bad argument 2 to set_description");
   }
   /* find the description mapping for the detail */
   descs = id_to_descs[id];
   if (!descs) {
      descs = id_to_descs[id] = ([ ]);
   }
   /* and set the description of type 'type' to desc */
   descs[type] = desc;
}

atomic
void remove_description(string id, string type) {
   mapping map;

   if (map = id_to_descs[id]) {
      map[type] = nil;
   }
}

void clear_adjectives(string id) {
   id_to_adjs[id] = nil;
}

void clear_preps(string id) {
   id_to_preps[id] = nil;
}

void clear_descs(string id) {
   id_to_descs[id] = nil;
}

void clear_snames(string id) {
   string *arr;

   if (arr = id_to_snames[id]) {
      int i, sz;

      sz = sizeof(arr);
      for (i = 0; i < sz; i++) {
	 if (sname_to_ids[arr[i]]) {
	    sname_to_ids[arr[i]] -= ({ id });
	    if (!sizeof(sname_to_ids[arr[i]])) {
	       sname_to_ids[arr[i]] = nil;
	    }
	 }
      }
      id_to_snames[id] = nil;
   }
}

void clear_pnames(string id) {
   string *arr;

   patch_plural();

   if (arr = id_to_pnames[id]) {
      int i, sz;

      sz = sizeof(arr);
      for (i = 0; i < sz; i++) {
	 if (pname_to_ids[arr[i]]) {
	    pname_to_ids[arr[i]] -= ({ id });
	    if (!sizeof(pname_to_ids[arr[i]])) {
	       pname_to_ids[arr[i]] = nil;
	    }
	 }
      }
      id_to_pnames[id] = nil;
   }
}

void clear_names(string id) {
   clear_snames(id);
   clear_pnames(id);
}

atomic void remove_sname(string id, string sname) {
   sname = lower_case(strip(sname));

   if (id_to_snames[id]) {
      id_to_snames[id] -= ({ sname });
      if (!sizeof(id_to_snames[id])) {
	 id_to_snames[id] = nil;
      }
   }
   if (sname_to_ids[sname]) {
      sname_to_ids[sname] -= ({ id });
      if (!sizeof(sname_to_ids[sname])) {
	 sname_to_ids[sname] = nil;
      }
   }
}

atomic void remove_pname(string id, string pname) {
   patch_plural();

   pname = lower_case(strip(pname));

   if (id_to_pnames[id]) {
      id_to_pnames[id] -= ({ pname });
      if (!sizeof(id_to_pnames[id])) {
	 id_to_pnames[id] = nil;
      }
   }
   if (pname_to_ids[pname]) {
      pname_to_ids[pname] -= ({ id });
      if (!sizeof(pname_to_ids[pname])) {
	 pname_to_ids[pname] = nil;
      }
   }
}

void remove_name(string id, string sname) {
   remove_sname(id, sname);
}

void clear_exit(string id) {
   string *arr;
   int i;

   /* clean up the dir_to_ids data associated with an exit */
   if (arr = id_to_dirs[id]) {
      for (i = 0; i < sizeof(arr); i ++) {
	 if (dir_to_ids[arr[i]]) {
	    dir_to_ids[arr[i]] -= ({ id });
	 }
      }
   }
   id_to_dest[id] = nil;
   id_to_dirs[id] = nil;
   id_to_keycode[id] = nil;
}

void clear_climbable(string id) {
   patch_luminosity();
   id_to_climbable[id] = nil;
}

atomic
void remove_detail(string id) {
   patch_luminosity();
   clear_preps(id);
   clear_descs(id);
   clear_names(id);
   clear_exit(id);
   clear_climbable(id);
   clear_adjectives(id);

   id_to_face[id]  = nil;

   id_to_keycode[id] = nil;
   id_to_luminosity[id] = nil;

   /* for flags, save only CLOSED/LOCK status */
   if (id_to_flags[id]) {
      id_to_flags[id] &= FLAG_IS_CLOSED | FLAG_IS_LOCKED | FLAG_WAS_DOOR;
   }
}

# define SetFlag(id, f) \
  { id_to_flags[id] = id_to_flags[id] ? (id_to_flags[id] | (f)) : (f); }

# define ClearFlag(id, f) \
  { id_to_flags[id] = id_to_flags[id] ? (id_to_flags[id] & ~(f)) : nil; }

# define GetFlag(id, f) \
  (id_to_flags[id] ? ((id_to_flags[id] & (f)) != 0) : 0)

/* set a detail to be prime */
atomic
void set_id_prime(string id, int val) {
   if (val) {
      SetFlag(id, FLAG_PRIME);
   } else {
      ClearFlag(id, FLAG_PRIME);
   }
}

/* is the given detail prime? */
int query_prime(string id) {
   return GetFlag(id, FLAG_PRIME);
}

/* set a detail to be definite */
atomic
void set_id_def(string id, int val) {
   if (val) {
      SetFlag(id, FLAG_DEFINITE);
   } else {
      ClearFlag(id, FLAG_DEFINITE);
   }
}   

/* is the given detail definite? */
int query_def(string id) {
   return GetFlag(id, FLAG_DEFINITE);
}

void
set_id_abstract(string id, int val) {
   if (val) {
      SetFlag(id, FLAG_ABSTRACT);
   } else {
      ClearFlag(id, FLAG_ABSTRACT);
   }

}

int
query_abstract(string id) {
   return GetFlag(id, FLAG_ABSTRACT);
}

/* set a detail to be plural */
void
set_id_plural(string id, int val) {
   if (val) {
      SetFlag(id, FLAG_PLURAL);
   } else {
      ClearFlag(id, FLAG_PLURAL);
   }
}

/* is a given detail plural? */
int
query_plural(string id) {
   return GetFlag(id, FLAG_PLURAL);
}

/* make a detail climbable */
atomic
void set_climbable(string id, NRef dest) {
   patch_luminosity();
   id_to_climbable[id] = dest;
}

/* make a detail an exit */
atomic
void set_exit(string id, NRef dest, string dirs...) {
   id_to_dirs[id] = dirs;
   id_to_dest[id] = dest;
}

/* set the destination of an exit */
void set_exit_dest(string id, mixed dest) {
   id_to_dest[id] = dest;
}

/* set the direction of an exit */
atomic
void add_exit_dir(string id, string dir) {
   if (id && strlen(id)) {
      if (!id_to_dirs[id]) {
	 id_to_dirs[id] = ({ });
      }
      id_to_dirs[id] |= ({ dir });
      if (dir_to_ids[dir]) {
	 dir_to_ids[dir] |= ({ id });
      } else {
	 dir_to_ids[dir]  = ({ id });
      }
   }
}

atomic
void remove_exit_dir(string id, string dir) {
   if (id && strlen(id)) {
      if (!id_to_dirs[id]) {
	 return;
      }
      id_to_dirs[id] -= ({ dir });
      if (dir_to_ids[dir]) {
	 dir_to_ids[dir] -= ({ id });
      }
   }
}

void set_exit_keycode(string id, string str) {
   id_to_keycode[id] = str;
}

int query_is_door(string det) {
   return GetFlag(det, FLAG_IS_DOOR);
}

atomic
void set_is_door(string det, int val, varargs int implicit) {
   NRef dest;

   if (val) {
      if (!GetFlag(det, FLAG_WAS_DOOR)) {
	 /* if something becomes a door for the first time, close & lock it */
	 call_out("maybe_close_and_lock", 0, det);
      }
      SetFlag(det, FLAG_IS_DOOR);
   } else {
      ClearFlag(det, FLAG_IS_DOOR);
      ClearFlag(det, FLAG_WAS_DOOR);
   }
}

static
void maybe_close_and_lock(string det) {
   if (GetFlag(det, FLAG_IS_DOOR)) {
      if (door_closes(det, FALSE)) {
	 if (ur_exit_keycode(this_object(), det) ||
	     ur_self_locking(this_object(), det)) {
	    door_locks(det, FALSE);
	 }
      }
      SetFlag(det, FLAG_WAS_DOOR);
   }
}


int query_never_obvious(string det) {
   return GetFlag(det, FLAG_NEVER_OBVIOUS);
}

int query_obvious_when_open(string det) {
   return GetFlag(det, FLAG_OBVIOUS_WHEN_OPEN);
}

int query_hidden_when_closed(string det) {
   return GetFlag(det, FLAG_HIDDEN_WHEN_CLOSED);
}

int query_separate(string det) {
   return GetFlag(det, FLAG_SEPARATE);
}

int query_self_locking(string det) {
   return GetFlag(det, FLAG_SELF_LOCKING);
}

int query_deadbolt(string det) {
   return GetFlag(det, FLAG_DEADBOLT);
}

int query_targetable_in_dark(string det) {
   return GetFlag(det, FLAG_TARGETABLE_IN_DARK);
}

void set_never_obvious(string det, int val) {
   if (val) {
      SetFlag(det, FLAG_NEVER_OBVIOUS);
   } else {
      ClearFlag(det, FLAG_NEVER_OBVIOUS);
   }
}

void set_obvious_when_open(string det, int val) {
   if (val) {
      SetFlag(det, FLAG_OBVIOUS_WHEN_OPEN);
   } else {
      ClearFlag(det, FLAG_OBVIOUS_WHEN_OPEN);
   }
}

void set_hidden_when_closed(string det, int val) {
   if (val) {
      SetFlag(det, FLAG_HIDDEN_WHEN_CLOSED);
   } else {
      ClearFlag(det, FLAG_HIDDEN_WHEN_CLOSED);
   }
}

void set_separate(string det, int val) {
   if (val) {
      SetFlag(det, FLAG_SEPARATE);
   } else {
      ClearFlag(det, FLAG_SEPARATE);
   }
}

void set_self_locking(string det, int val) {
   if (val) {
      SetFlag(det, FLAG_SELF_LOCKING);
   } else {
      ClearFlag(det, FLAG_SELF_LOCKING);
   }
}

void set_deadbolt(string det, int val) {
   if (val) {
      SetFlag(det, FLAG_DEADBOLT);
   } else {
      ClearFlag(det, FLAG_DEADBOLT);
   }
}

void set_targetable_in_dark(string det, int val) {
   if (val) {
      SetFlag(det, FLAG_TARGETABLE_IN_DARK);
   } else {
      ClearFlag(det, FLAG_TARGETABLE_IN_DARK);
   }
}


string query_exit_keycode(string id) {
   return id_to_keycode[id];
}

atomic void add_sname(string id, string sname) {
   sname = lower_case(strip(sname));

   if (sname_to_ids[sname]) {
      sname_to_ids[sname] |= ({ id });
   } else {
      sname_to_ids[sname]  = ({ id });
   }
   if (id_to_snames[id]) {
      id_to_snames[id] |= ({ sname });
   } else {
      id_to_snames[id]  = ({ sname });
   }
}

atomic void add_pname(string id, string pname) {
   patch_plural();

   pname = lower_case(strip(pname));
   if (pname_to_ids[pname]) {
      pname_to_ids[pname] |= ({ id });
   } else {
      pname_to_ids[pname]  = ({ id });
   }
   if (id_to_pnames[id]) {
      id_to_pnames[id] |= ({ pname });
   } else {
      id_to_pnames[id]  = ({ pname });
   }
}

/* add an alias for a detail, optionally with a plural form */
atomic void add_alias(string id, string sname, varargs string pname) {
   string tmp;

   if (!id) {
      error("bad argument 1 to add_alias");
   }
   if (sname) {
      add_sname(id, sname);
   }

   if (pname) {
      add_pname(id, pname);
   }
}

atomic void remove_alias(string id, string sname, varargs string pname) {
   string p;

   if (!id) {
      error("bad argument 1 to remove_alias");
   }
   if (sname) {
      remove_sname(id, sname);
   }
   if (pname) {
      remove_pname(id, pname);
   }
}

void set_face(string id, string face) {
   id_to_face[id] = face;
}

/* add an adjective to a detail */
atomic
void add_adjective(string id, string adj) {
   if (id && adj) {
      if (!id_to_adjs[id]) {
	 id_to_adjs[id] = ([ ]);
      }
      id_to_adjs[id][strip(adj)] = TRUE;
   }
}

atomic
void remove_adjective(string id, string adj) {
   mapping map;

   if (id) {
      if (map = id_to_adjs[id]) {
	 map[strip(adj)] = nil;
      }
   }
}


/* find the first detail out of an array of details with the given face */
private
string face_lookup(string *idarr, varargs string face) {
   int i;

   for (i = 0; i < sizeof(idarr); i ++) {
      if (id_to_face[idarr[i]] == face) {
	 return idarr[i];
      }
   }
   return nil;
}

/* find all the details out of an array of details with the given face */
private
string *match_faces(string *idarr, string face) {
   mapping tmp;
   int i;

   tmp = ([ ]);
   for (i = 0; i < sizeof(idarr); i ++) {
      if ((face && ur_face(this_object(), idarr[i]) == face) ||
	  !ur_face(this_object(), idarr[i])) {
	 tmp[idarr[i]] = TRUE;
      }
   }
   return map_indices(tmp);
}

/* return the details that match a given singular name */
string *query_sname_ids(string name) {
   return sname_to_ids[name] ? sname_to_ids[name] : ({ });
}

/* return the details that match a given plural name */
string *query_pname_ids(string name) {
   return pname_to_ids[name] ? pname_to_ids[name] : ({ });
}

/* return the description of a given type in a given detail */
mixed query_description(string id, string type) {
   if (id_to_descs[id]) {
      return id_to_descs[id][type];
   }
   return nil;
}

mapping query_description_mapping(string id) {
  if (id_to_descs[id]) {
    return id_to_descs[id];
  }
  return ([ ]);
}

/* find a prime reference of a given face */
string get_prime_id(string face) {
   string *idarr, id;
   int i;

   if (idarr = map_indices(id_to_descs)) {
      for (i = 0; i < sizeof(idarr); i ++) {
	 if (!GetFlag(idarr[i], FLAG_PRIME)) {
	    idarr[i] = nil;
	 }
      }
      idarr -= ({ nil });

      if (id = face_lookup(idarr, face)) {
	 return id;
      }
      return face_lookup(idarr);
   }
}

/* return the destination of a given climbable detail */
NRef query_climbable_dest(string id) {
   patch_luminosity();
   return id_to_climbable[id];
}

/* return the destination of a given exit detail */

mixed query_exit_dest(string id) {
   return id_to_dest[id];
}

/* return the directions of a given exit detail */
string *query_exit_dirs(string id) {
   return id_to_dirs[id] ? id_to_dirs[id] - ({ nil, "" }) : ({ });
}

int test_exit_dir(string id, string dir) {
   if (id_to_dirs[id]) {
      return member(dir, id_to_dirs[id]);
   }
   return FALSE;
}

string *query_dirs() {
   return map_indices(dir_to_ids) - ({ nil });
}

string *query_details_by_dir(string dir) {
   if (dir_to_ids[dir]) {
      return dir_to_ids[dir];
   }
   return ({ });
}

string *query_furry_details() {
   mapping map;

   patch_plural();

   map = id_to_preps +
      id_to_descs +
      id_to_snames +
      id_to_pnames +
      id_to_adjs +
      id_to_face +
      id_to_dest +
      id_to_dirs +
      id_to_keycode +
      id_to_climbable;

   patch_luminosity();
   return map_indices(id_to_flags + id_to_luminosity) - map_indices(map);
}

/* return all details: somewhat expensive */
string *query_details() {
   mapping map;
   string *ix;
   int i;

   patch_plural();

   map = id_to_preps +
      id_to_descs +
      id_to_snames +
      id_to_pnames +
      id_to_adjs +
      id_to_face +
      id_to_dest +
      id_to_dirs +
      id_to_keycode +
      id_to_climbable;

   ix = map_indices(id_to_flags);
   for (i = 0; i < sizeof(ix); i ++) {
      if ((id_to_flags[ix[i]] & ~(FLAG_IS_CLOSED | FLAG_IS_LOCKED | FLAG_WAS_DOOR)) != 0) {
	 map[ix[i]] = TRUE;
      }
   }
   return map_indices(map);
}

/* helper function */
string *query_sorted_details() {
   return
      filter(query_details(), "query_prime", this_object()) |
      filter(query_details(), "query_exit_dest", this_object()) |
      query_details();
}

/* return the description types of a given detail */
string *query_description_types(string id) {
   if (id_to_descs[id]) {
      return map_indices(id_to_descs[id]);
   }
   return ({ });
}

int test_sname(string id, string name) {
   string *arr;

   if (arr = id_to_snames[id]) {
      return member(name, arr);
   }
}

int test_pname(string id, string name) {
   string *arr;

   patch_plural();

   if (arr = id_to_pnames[id]) {
      return member(name, arr);
   }
}

/* return the singular names of a given detail */
string *query_snames(string id) {
   return id_to_snames[id] ? id_to_snames[id] : ({ });
}

/* return the plural names of a given detail */
string *query_pnames(string id) {
   patch_plural();

   return id_to_pnames[id] ? id_to_pnames[id] : ({ });
}

int test_adjective(string id, string adj) {
   mapping map;

   if (map = id_to_adjs[id]) {
      return !!map[adj];
   }
}

/* return the plural names of a given detail */
string *query_adjectives(string id) {
   mapping map;

   if (map = id_to_adjs[id]) {
      return map_indices(map);
   }
   return ({ });
}

/* return the face of a given detail */
string query_face(string id) {
   return id_to_face[id];
}

/* map a singular name to a plural name */
string query_pname_of_sname(string sname) {
   return sname_to_pname[sname];
}

float query_luminosity(string id) {
   mixed f;

   patch_luminosity();
   if (f = id_to_luminosity[id]) {
      return f;
   }
   return 0.0;
}

# define DirtyLight() CRAWLERS->start_crawler(this_object()); dirty_light_cache(TRUE);

void set_luminosity(string id, float i) {
   patch_luminosity();

   if (!id_to_luminosity[id]) {
      id_to_luminosity[id] = 0.0;
   }
   if (id_to_luminosity[id] != i) {
      DirtyLight();
      id_to_luminosity[id] = i;
   }
}

/* set a detail to be hidden */
atomic
void set_id_hidden(string id, int val) {
   if (val) {
      SetFlag(id, FLAG_IS_HIDDEN);
      this_object()->leave_proximates(id);
   } else {
      ClearFlag(id, FLAG_IS_HIDDEN);
   }
   DirtyLight();
}

/* is the given detail hidden? */
int query_hidden(string id) {
   return GetFlag(id, FLAG_IS_HIDDEN);
} 

/*** DEBUG ***/
string *query_all_snames()
{
   return map_indices(sname_to_ids);
}


/* calls only expected in leaf objects */

string figure_side(varargs object looker) {
   if (looker && looker->contained_by(this_object())) {
      return "inside";
   }
   return "outside";
}

int query_closed(string det) {
   return ur_is_door(this_object(), det) && GetFlag(det, FLAG_IS_CLOSED);
}

int query_locked(string det) {
   return query_closed(det) && GetFlag(det, FLAG_IS_LOCKED);
}

void set_locked(string det, int val) {
   if (val) {
      SetFlag(det, FLAG_IS_LOCKED);
   } else {
      ClearFlag(det, FLAG_IS_LOCKED);
   }
}

atomic
void do_close_door(string id, varargs int silent_here, int silent_there) {
   object ob;
   NRef dest;

   if (ur_is_door(this_object(), id)) {
      if (!GetFlag(id, FLAG_IS_CLOSED)) {
	 if (door_closes(id, silent_here)) {
	    if (!ur_separate(this_object(), id)) {
	       if (dest = id_to_dest[id]) {
		  ob = dest->query_object();
		  if (this_object()->is_child_of(ob)) {
		     /* rewrite: see /base/lib/act/movement.c */
		     ob = this_object();
		  }
		  if (!ob->door_closes(dest->query_detail(this_object()),
				       silent_there)) {
		     return;
		  }
	       }
	    }
	 }
      }
   }
}

int door_closes(string id, varargs int silent) {
   NRef this;

   if (ur_is_door(this_object(), id)) {
      this = NewNRef(this_object(), id);
      if (!pre_signal("door/closes", ([ "target": this ]),
		      ([ nil: this ]))) {
	 return FALSE;
      }
      if (!prime_signal("door/closes", ([ "target": this ]),
			([ nil: this ]))) {
	 return FALSE;
      }
      SetFlag(id, FLAG_IS_CLOSED);
      if (ur_hidden_when_closed(this_object(), id)) {
	 this_object()->leave_proximates(id);
      }
      if (!silent && desc_signal("door/closes", ([ "target": this ]),
				 ([ nil: this ]))) {
	 broadcast_event("closes", this_object(), id);
      }
      post_signal("door/closes", ([ "target": this ]),
		  ([ nil: this ]));
      DirtyLight();
   }
   if (ur_self_locking(this_object(), id)) {
      call_out("door_locks", 0, id, FALSE);
   }
   return TRUE;
}

atomic
void do_open_door(string id, varargs int silent_here, int silent_there) {
   object ob;
   NRef dest;

   if (ur_is_door(this_object(), id)) {
      if (GetFlag(id, FLAG_IS_CLOSED)) {
	 if (door_opens(id, silent_here)) {
	    if (!ur_separate(this_object(), id)) {
	       if (dest = id_to_dest[id]) {
		  ob = dest->query_object();
		  if (this_object()->is_child_of(ob)) {
		     /* rewrite: see /base/lib/act/movement.c */
		     ob = this_object();
		  }
		  if (!ob->door_opens(dest->query_detail(this_object()),
				      silent_there)) {
		     return;
		  }
	       }
	    }
	 }
      }
   }
}

int door_opens(string id, varargs int silent) {
   NRef this;

   if (ur_is_door(this_object(), id)) {
      this = NewNRef(this_object(), id);
      if (!pre_signal("door/opens", ([ "target": this ]),
		      ([ nil: this ]))) {
	 return FALSE;
      }
      if (!prime_signal("door/opens", ([ "target": this ]),
			([ nil: this ]))) {
	 return FALSE;
      }
      ClearFlag(id, FLAG_IS_CLOSED);
      if (!silent && desc_signal("door/opens", ([ "target": this ]),
				 ([ nil: this ]))) {
	 broadcast_event("opens", this_object(), id);
      }
      post_signal("door/opens", ([ "target": this ]),
		  ([ nil: this ]));
      DirtyLight();
   }
   return TRUE;
}


int door_locks(string id, varargs int silent);
int door_unlocks(string id, varargs int silent);

atomic
void do_lock_door(string id, varargs int silent_here, int silent_there) {
   object ob;
   NRef dest;

   if (ur_is_door(this_object(), id)) {
      if (!GetFlag(id, FLAG_IS_LOCKED)) {
	 if (door_locks(id, silent_here)) {
	    if (!ur_separate(this_object(), id)) {
	       if (dest = id_to_dest[id]) {
		  ob = dest->query_object();
		  if (this_object()->is_child_of(ob)) {
		     /* rewrite: see /base/lib/act/movement.c */
		     ob = this_object();
		  }
		  if (!ob->door_locks(dest->query_detail(this_object()),
				      silent_there)) {
		     return;
		  }
	       }
	    }
	 }
      }
   }
}

int door_locks(string id, varargs int silent) {
   NRef this;

   if (ur_is_door(this_object(), id)) {
      this = NewNRef(this_object(), id);
      if (!pre_signal("door/locks", ([ "target": this ]),
		      ([ nil: this ]))) {
	 return FALSE;
      }
      if (!prime_signal("door/locks", ([ "target": this ]),
			([ nil: this ]))) {
	 return FALSE;
      }
      SetFlag(id, FLAG_IS_LOCKED);
      if (!silent && desc_signal("door/locks", ([ "target": this ]),
				 ([ nil: this ]))) {
	 broadcast_event("locks", this_object(), id);
      }
      post_signal("door/locks", ([ "target": this ]),
		  ([ nil: this ]));
   }
   return TRUE;
}

atomic
void do_unlock_door(string id, varargs int silent_here, int silent_there) {
   object ob;
   NRef dest;

   if (ur_is_door(this_object(), id)) {
      if (GetFlag(id, FLAG_IS_LOCKED)) {
	 if (door_unlocks(id, silent_here)) {
	    if (!ur_separate(this_object(), id)) {
	       if (dest = id_to_dest[id]) {
		  ob = dest->query_object();
		  if (this_object()->is_child_of(ob)) {
		     /* rewrite: see /base/lib/act/movement.c */
		     ob = this_object();
		  }
		  if (!ob->door_unlocks(dest->query_detail(this_object()),
					silent_there)) {
		     return;
		  }
	       }
	    }
	 }
      }
   }
}

int door_unlocks(string id, varargs int silent) {
   NRef this;

   if (ur_is_door(this_object(), id)) {
      this = NewNRef(this_object(), id);
      if (!pre_signal("door/unlocks", ([ "target": this ]),
		      ([ nil: this ]))) {
	 return FALSE;
      }
      if (!prime_signal("door/unlocks", ([ "target": this ]),
			([ nil: this ]))) {
	 return FALSE;
      }
      ClearFlag(id, FLAG_IS_LOCKED);
      if (!silent && desc_signal("door/unlocks", ([ "target": this ]),
				 ([ nil: this ]))) {
	 broadcast_event("unlocks", this_object(), id);
      }
      post_signal("door/unlocks", ([ "target": this ]),
		  ([ nil: this ]));
   }
   return TRUE;
}



string *query_doors(varargs object looker) {
   mapping doors;
   string *details;
   int i;

   details = ur_details(this_object());
   if (looker) {
      details = match_faces(details, figure_side(looker));
   }
   doors = ([ ]);

   for (i = 0; i < sizeof(details); i++) {
      if (ur_is_door(this_object(), details[i])) {
	 doors[details[i]] = TRUE;
      }
   }
   return map_indices(doors);
}

int is_obvious(string str);

string *query_open_doors(varargs object looker, int obvious) {
   string *doors, *open;
   int i;

   doors = query_doors(looker);
   open = allocate(sizeof(doors));

   for(i = 0; i < sizeof(doors); i++) {
      if (!GetFlag(doors[i], FLAG_IS_CLOSED)) {
	 if (!obvious || is_obvious(doors[i])) {
	    open[i] = doors[i];
	 }
      }
   }

   open -= ({ nil });
   return open;
}

string *query_closed_doors(varargs object looker, int obvious) {
   string *doors, *closed;
   int i;

   doors = query_doors(looker);
   closed = allocate(sizeof(doors));

   for(i = 0; i < sizeof(doors); i++) {
      if (GetFlag(doors[i], FLAG_IS_CLOSED)) {
	 if (!obvious || is_obvious(doors[i])) {
	    closed[i] = doors[i];
	 }
      }
   }

   closed -= ({ nil });
   return closed;
}


/* return a detail with an exit in the given direction */
string query_detail_by_dir(string dir) {
   mixed *arr;
   int i;

   arr = ur_details_by_dir(this_object(), dir);
   if (arr) {
      arr = hidden_filter(arr);
      if (sizeof(arr) > 0) {
	 return arr[0];
      }
   }
}

/* find all the exits that match this face */
string *match_exit_details(string face, varargs int obvious) {
   string *list, *res;
   mapping tmp;
   int i;

   tmp = ([ ]);
   res = ({ });

   list = ur_details(this_object());

   for (i = 0; i < sizeof(list); i++) {
      if (!ur_exit_dest(this_object(), list[i])) {
	 continue;
      }
      if (obvious) {
	 /* if this argument is set, only list obvious exits */
	 if (query_never_obvious(list[i])) {
	    continue;
	 }
	 if (query_obvious_when_open(list[i]) && query_closed(list[i])) {
	    continue;
	 }
      }

      if (face == nil && id_to_face[list[i]] != "outside") {
	 tmp[list[i]] = TRUE;
      } else if (id_to_face[list[i]] == face) {
	 tmp[list[i]] = TRUE;
      }
   }

   if (!sizeof(map_indices(tmp))) {
      return ({ });
   }

   res = map_indices(tmp);
   res = hidden_filter(res);

   return res;
}

/* find the details out of an array of details that match given adjectives */
private
string *adj_filter(string *idarr, string *adj) {
   string *res;
   mapping lookup;
   int i, j;

   if (!adj || !sizeof(adj)) {
      return idarr;
   }
   res = ({ });
   for (i = 0; i < sizeof(idarr); i ++) {
      j = 0;
      while (j < sizeof(adj) && ur_test_adjective(this_object(),
						  idarr[i], adj[j])) {
	 j ++;
      }
      if (j == sizeof(adj)) {
	 res += ({ idarr[i] });
      }
   }
   return res;
}

/* is the given exit obvious? */
int is_obvious(string id) {
   if (ur_never_obvious(this_object(), id)) {
      return FALSE;
   }
   if (ur_obvious_when_open(this_object(), id) && query_closed(id)) {
      return FALSE;
   }
   return TRUE;
}

/* is the given detail hidden? */
int is_hidden(string id) {
   return ur_hidden(this_object(), id) ||
      (query_closed(id) && ur_hidden_when_closed(this_object(), id));
}

/* filters out all of the hidden details */
private
string *hidden_filter(string *idarr) {
   string *res;
   int i;

   res = ({ });
   for (i = 0; i < sizeof(idarr); i++) {
      if (!is_hidden(idarr[i])) {
	 res += ({ idarr[i] });
      }
   }

   return res;
}

float get_luminosity(string id) {
   if (is_hidden(id)) {
      return 0.0;
   }
   return ur_luminosity(this_object(), id);
}


/* find all the details that match a singular name and adjective list */
string *match_single_detail(string name, string face, varargs string *adj) {
   string *idarr;

   if (name) {
      idarr = ur_sname_ids(this_object(), name);
   } else {
      idarr = ur_details(this_object());
   }
   if (idarr) {
      idarr = match_faces(idarr, face);
      idarr = adj_filter(idarr, adj);
      idarr = hidden_filter(idarr);
      return idarr;
   }
   return ({ });
}

/* find all the NRef's that match a plural name and adjective list */
string *match_plural_detail(string name, string face, varargs string *adj) {
   string *idarr;

   if (name) {
      idarr = ur_pname_ids(this_object(), name);
   } else {
      idarr = ur_details(this_object());
   }
   if (idarr) {
      idarr = match_faces(idarr, face);
      idarr = adj_filter(idarr, adj);
      idarr = hidden_filter(idarr);
      return idarr;
   }
   return ({ });
}

