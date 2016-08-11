/*
**      /base/lib/clothing.c
**
**      This code handles the manipulation of clothing.
**	Include manipulations for wearing clothing and 'being' clothing.
**
**      Copyright Skotos Tech Inc 1999-2007
**
*/

/* 
** PHASE 1 
**
** Basic add/remove of clothing
** We are assuming that this object HAS worn the articles on their proper
** locations.
**
*/

private inherit "/lib/array";

private inherit "/base/lib/urcalls";

private string *parts_covered;		/* what parts I cover when worn */
private object worn_by;			/* who is wearing me */
private mapping clothes_worn;		/* what clothes I am wearing */
private int single_wear;		/* can I be only worn once? */
private int transparency;		/* can you see through me? */

/* the clothes_worn mapping is a mapping of stacks. the index is a part
   and value is a stack of clothes. the stack simulates the stack of clothing
   physically on the object's part. */

object *list_clothes();

static
void create() {
   parts_covered = ({ });
   clothes_worn = ([ ]);
}

static
void destructing() {
   if (worn_by) {
      worn_by->unwear_article(this_object());
   }
}

/* worn_by manipulations */
object query_worn_by() {
   return worn_by;
}

void set_worn_by(object owner) {
   worn_by = owner;
}

/* transparency manipulations */
int query_transparency() {
   return transparency;
}

void set_transparency(int value) {
   if (value) {
      transparency = TRUE;
   } else {
      transparency = FALSE;
   }
}

/* parts_covered manipulations */
string *query_parts_covered() {
   return parts_covered;
}

void set_parts_covered(string *new_parts) {
   if (worn_by) {
      error("article is worn");
   }
   parts_covered = new_parts;
}

void add_parts_covered(string new_part) {
   if (worn_by) {
      error("article is worn");
   }
   if (new_part) {
      parts_covered |= ({ new_part });
   }
}

void clear_parts_covered() {
   parts_covered = ({ });
}

/* clothes_worn manipulations */
mapping query_clothes_worn() {
   return clothes_worn[..];
}

/*
 * NAME:        clear_clothes_worn()
 * DESCRIPTION: An emergency-break in case things go hayware.   Clears the
 *              clothes_worn mapping and loops over the inventory to make
 *              sure all objects have their Base:Clothing:Worn property reset.
 */
void clear_clothes_worn() {
   int i, sz;
   object *inv;

   clothes_worn = ([ ]);
   inv = this_object()->query_inventory();
   sz = sizeof(inv);
   for (i = 0; i < sz; i++) {
      inv[i]->clear_property("Base:Clothing:Worn");
      inv[i]->set_worn_by(nil);
   }
}

/* single wear manipulations */
void set_single_wear(int value) {
   single_wear = value;
}

int query_single_wear() {
   return single_wear;
}

string *query_worn_on_location(string location) {
   if (clothes_worn[location]) {
      return clothes_worn[location] - ({ nil });
   }
   return ({ });
}


/* search the clothes_worn for a specific article */
int is_wearing(object article) {
   object *generic;
   string *parts;
   int i, j;

   parts = map_indices(clothes_worn);

   /* lets iterate over the list of bodyparts covered by clothing */
   for(i = 0; i < sizeof(parts); i++) {
      if (generic = clothes_worn[parts[i]]) {
	 if (member(article, generic)) {
	    return TRUE;
	 }
      }
   }
   return FALSE;
}


/* push an article of clothing onto the locations listed in parts_covered */
atomic
void add_article(object article) {
   int i, covered_size;
   string *covered;

   if (article->query_environment() != this_object()) {
      error("article to be worn not in inventory");
   }

   covered = ur_parts_covered(article);
   covered_size = sizeof(covered);

   /* lets iterate the list of parts_covered by 'article' */
   for(i = 0; i < covered_size; i++) {
      if (clothes_worn[covered[i]] == nil)
	 clothes_worn[covered[i]] = ({ });

      clothes_worn[covered[i]] += ({ article });
   }

   if (article == nil)
      XDebug("XX - add_article() article is nil.");

   article->set_property("Base:Clothing:Worn", TRUE);
   article->set_worn_by(this_object());
}

/* 
** EXTRACT an article from the stacks. this will magically remove any article
** regardless of what is on top of it. the checks should be done in act_
*/
atomic
void unwear_article(object article) {
   string *covered;
   int i;

   covered = map_indices(clothes_worn);

   for (i = 0; i < sizeof(covered); i++) {
      if (clothes_worn[covered[i]] != nil) {
	 clothes_worn[covered[i]] -= ({ article });
	 if (sizeof(clothes_worn[covered[i]]) < 1) {
	    clothes_worn[covered[i]] = nil;
	 }
      }
   }
   article->set_property("Base:Clothing:Worn", nil);
   article->set_worn_by(nil); 
}

/* check if an article is covered by another */
int is_covered(object article) {
   int i, j;
   string *covered;
   object *generic;

   covered = ur_parts_covered(article);

   /* lets iterate the list of parts_covered by 'article' */
   for(i = 0; i < sizeof(covered); i++) {
      if (clothes_worn[covered[i]] != nil) {
	 generic = clothes_worn[covered[i]];

	 /* we also have to iterate through each stack */
	 for (j = 0; j < sizeof(generic); j++) {
	    if ((article == generic[j])
	       && ((j+1) < sizeof(generic))) {
	       return TRUE;
	    }
	 }

      }
   }

   return FALSE;
}

/* return the articles covering an article */
object *query_covered_by(object article) {
   int i, j;
   string *covered;
   object *generic, *list;
   
   covered = ur_parts_covered(article);
   list = ({ });

   /* lets iterate the list of parts_covered by 'article' */
   for(i = 0; i < sizeof(covered); i++) {
      if (clothes_worn[covered[i]] != nil) {
	 generic = clothes_worn[covered[i]];

	 /* we also have to iterate through each stack */
	 for (j = 0; j < sizeof(generic); j++) {
	    if ((article == generic[j])
	       && ((j+1) < sizeof(generic))) {
	       list |= generic[j+1 .. sizeof(generic) - 1];
	    }
	 }

      }
   }

   return list;
}


/* return a list of the totally covered clothes in array format */
object *list_totally_covered_clothes() {
   string *parts;
   object *list, *clothes, top;
   mapping tmp_map;
   int i;

   parts = map_indices(clothes_worn);
   list = ({ });

   /* first generate a list of clothes */
   clothes = list_clothes();

   if (!sizeof(clothes))
      return ({ });

   /* next remove any articles that appear at the top of a stack */
   for (i = 0; i < sizeof(parts); i++) {
      if (clothes_worn[parts[i]] != nil) {
	 list = clothes_worn[parts[i]];
	 clothes -= ({ list[sizeof(list) - 1] });
      }
   }

   return clothes;
}

/* return a list of the partially covered clothes in array format */
object *list_partially_covered_clothes() {
   string *parts;
   object *list, *clothes, *covered;
   mapping tmp_map, tops;
   int i;

   parts = map_indices(clothes_worn);
   list = ({ });
   tops = ([ ]);

   /* generate a list of the stack tops that are covered elsewhere */
   for (i = 0; i < sizeof(parts); i++) {
      if (clothes_worn[parts[i]] != nil) {
	 list = clothes_worn[parts[i]];
	 
	 /* i think this could be done better -- Wes */
	 if (is_covered(list[sizeof(list) - 1]))
	    tops[list[sizeof(list) - 1]] = TRUE;
      }
   }

   return map_indices(tops);
}


/* return a list of the clothes in array format */
object *list_clothes() {
   int i, j, covered_size, generic_size;
   string *covered;
   object *generic;
   mapping tmp_map;

   covered = map_indices(clothes_worn);
   covered_size = sizeof(covered);
   tmp_map = ([ ]);

   /* lets iterate the list of parts_covered by 'article' */
   for(i = 0; i < covered_size; i++) {
      if (clothes_worn[covered[i]] != nil) {
	 generic = clothes_worn[covered[i]];
	 generic_size = sizeof(generic);

	 /* we also have to iterate through each stack */
	 for (j = 0; j < generic_size; j++) {
	    if (generic[j] != nil)
	       tmp_map[generic[j]] = TRUE;
	 }

      }
   }

   return map_indices(tmp_map);
}

/* return a list of all the wielding items */
object *query_weapons() {
   int i, j;
   string *indices;
   object *generic;
   mapping tmp_map;

   indices = map_indices(clothes_worn);
   tmp_map = ([ ]);

   for(i = 0; i < sizeof(indices); i++) {
      if (clothes_worn[indices[i]] != nil) {
	 generic = clothes_worn[indices[i]];

	 for (j = 0; j < sizeof(generic); j++) {
	    if (ur_weapon(generic[j]))
	       tmp_map[generic[j]] = TRUE;
	 }

      }
   }

   return map_indices(tmp_map);
}
   
