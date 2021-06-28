/*
**	/usr/SkotOS/lib/memories.c
**
**	Copyright Skotos Tech Inc 2000
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

void set_raw_memory ( string mem, mixed data ) {
   memories[ mem ] = data;
}

mixed query_raw_memory( mixed mem ) {
   return memories[ mem ];
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
