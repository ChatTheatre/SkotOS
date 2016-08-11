/*
**	/usr/SkotOS/lib/memories.c
**
**	Copyright Skotos Tech Inc 2000
*/

/*
 * Backward compatibility wise the memories mapping has a major
 * identity crisis.
 *
 * For player bodies which use the add_memory() function from
 * /base/lib/act/basic.c, you set up a mapping with integer indices
 * and strings (the memories) as values.
 *
 * For CNPC bodies which use the set_raw_memory() function from
 * ~SkotOS/sys/npc-brains.c, you set up a mapping from string indices
 * and integers (timestamps) as values!
 *
 * Should these two ever encounter eachother, I fear a nice big clash
 * and the universe collapsing. --Erwin.
 */

# define PAGE_HISTORY_MAX 100
# define PAGE_HISTORY_DEF  30

private inherit "/lib/string";

mapping memories;
int memcnt;

private mixed **page_history;

static
void create() {
   memories = ([ ]);   
   memcnt = 0;
}

void add_memory ( string text ) {
   memories[ memcnt++ ] = text;
}

void reorganize ( void ) {
   string *sar;
   int i;

   sar = ({ });
   for ( i=0; i<memcnt; i++ ) {
      if ( memories[i] ) {
         sar += ({ memories[i] });
	 memories[i] = nil;
      }
   }
   memcnt = sizeof(sar);
   for( i=0; i<memcnt; i++ )
      memories[i] = sar[i];
}

string forget_memory ( string phrase ) {
   int i, cnt;

   for( i=cnt=0; i<memcnt; i++ ) {
      if ( sscanf( memories[i], "%*s"+phrase+"%*s" ) ) {
         memories[i] = nil;
	 cnt++;
      }
   }
   if ( cnt > 0 )
      reorganize();
   
   return "Memories forgotten: "+cnt+"\n";
}

/*
 * Used by /base/sys/base.c to make the memories exportable and
 * most importantly settable via the dynamic property
 * "base:memories".
 */
void set_memories(string *mem) {
   int i, sz;

   memories = ([ ]);
   sz = sizeof(mem);
   for (i = 0; i < sz; i++) {
      memories[i] = mem[i];
   }
   memcnt = sz;
}

void set_raw_memory ( string mem, mixed data ) {
   memories[ mem ] = data;
}

mixed query_raw_memory( mixed mem ) {
   return memories[ mem ];
}

/*
 * Show all memories unless an array of words is passed on.
 */
string *query_memories(varargs string *words) {
    string *list;

    list = ({ });
    if (words) {
	int i, nr_words;

	nr_words = sizeof(words);
	for (i = 0; i < memcnt; i++) {
	    int j, match;

	    match = FALSE;
	    for (j = 0; j < nr_words; j++) {
		if (sscanf(lower_case(memories[i]),
			   "%*s" + lower_case(words[j]))) {
		    match = TRUE;
		    break;
		}
	    }
	    if (match) {
		list += ({ memories[i] });
	    }
	}
    } else {
	int i;

	for (i = 0; i < memcnt; i++) {
	    list += ({ memories[i] });
	}
    }
    return list;
}

/*
 * Page recall access.
 */

void
add_page_history(int when, string from, string to, string page)
{
   catch {
      if (!page_history) {
	 page_history = ({ });
      } else {
	 int sz;

	 sz = sizeof(page_history);
	 if (sz > PAGE_HISTORY_MAX) {
	    page_history = page_history[sz - PAGE_HISTORY_MAX..];
	 }
      }
      page_history += ({ ({ when, lower_case(from), lower_case(to), page }) });
   }
}

mixed *
query_page_history(int total, varargs string name)
{
   int   i, sz;
   mixed **list;

   if (!page_history) {
      return ({ });
   }
   total = total > PAGE_HISTORY_MAX ? PAGE_HISTORY_MAX : total > 0 ? total : PAGE_HISTORY_DEF;
   list = allocate(total);
   sz = sizeof(page_history);
   if (name) {
      int found;

      name = lower_case(name);
      for (i = sz - 1; i >= 0 && found < total; i--) {
	 if (page_history[i][1] == name || page_history[i][2] == name) {
	    found++;
	    list[total - found] = page_history[i];
	 }
      }
      list = list[total - found..];
   } else {
      total = total > sz ? sz : total;
      list = page_history[sz - total..];
   }
   return list;
}
