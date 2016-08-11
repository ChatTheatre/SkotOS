/*
**	/base/sys/crawlers.c
**
**	The bulk and light systems both perform their simulation by
**	objects recursively adding contributions from their inventory.
**	This would be too slow without a caching mechanism so that an
**	environment caches its calculations until its inventory or a
**	value associated with an object in its inventory changes.
**
**	When such a change occur, bulk/light caches are dirtied going
**	up the environment chain. If a torch's light level is changed,
**	the torch wielder's light cache is dirtied, the room he is in
**	is dirtied, etc, etc.
**
**	This works fine until we intersect with the ur-parent system.
**	When I modify the light levels of the UrTorch, thousands and
**	thousands of caches need to be dirtied. There is no real good
**	way of doing this. Our solution is to start a process running
**	'in the background' that goes through all the ur-children of
**	a modified object and dirties their caches.
**
**	Those processes are called crawlers, and this object performs
**	the necessary calculations.
**
**	Zell 010829
**
**	For sanity's sake, I've revamped the system to use a stack of
**	objects to be crawled, instead of firing up multiple crawlers in
**	parallel, quite often bringing the server to its knees when it
**	happens times that it's actually in use.
**
**	Nino 030619.
**
**	Copyright Skotos Tech Inc. 2001 - 2003.
**
*/

int     handle;
mixed   *stack;
mapping crawlers;	/* ur object to callout handle */
mapping children;

static void create() {
   stack    = ({ });
   crawlers = ([ ]);
   children = ([ ]);
}

void patch() {
   if (!stack) stack = ({ });
   if (!children) children = ([ ]);
}

mixed *query_stack() {
    return stack[..];
}

void clear_data() {
    handle   = 0;
    stack    = ({ });
    children = ([ ]);
    crawlers = ([ ]);
}

private float timediff(mixed *t1, mixed *t2) {
    int   diff_s;
    float diff_m;

    diff_s = t2[0] - t1[0];
    diff_m = t2[1] - t1[1];
    return (float)diff_s + diff_m;
}

# if 0
void start_crawler(object ur, varargs object parent) {
   int     i, sz, count;
   mixed   *lists;
   mapping map;

   if (crawlers[ur]) {
      object *list;

      list = ({ ur });
      while (sizeof(list)) {
	 int i, sz;
	 object *kids;

	 kids = ({ });
	 sz = sizeof(list);
	 for (i = 0; i < sz; i++) {
	    Debug("Crawler[" + name(list[i]) + "]: Aborting for restart...");
	    if (children[list[i]]) {
	       kids += map_indices(children[list[i]]);
	       children[list[i]] = nil;
	       crawlers[list[i]] = nil;
	    }
	 }
	 sz = sizeof(stack);
	 if (sz > 0) {
	     for (i = 0; i < sz; i++) {
		 if (sizeof(({ stack[i][0] }) & list) > 0) {
		     stack[i] = nil;
		 }
	     }
	     stack -= ({ nil });
	 }
	 list = kids;
      }
   }

   count = ur->query_child_count();
   if (!count) {
      return;
   }
   map   = ur->query_child_mapping();
   sz    = map_sizeof(map);
   lists = map_values(map);
   for (i = 0; i < sz; i++) {
      lists[i] = map_indices(lists[i]);
   }
   Debug("Crawler[" + name(ur) + "]: Starting crawler; list size is " + count + "...");
   stack += ({ ({  ur, lists, count, parent }) }) ;
   crawlers[ur] = ({ millitime(), 0, count, parent });
   if (parent) {
       if (!children[parent]) {
	   children[parent] = ([ ]);
       }
       children[parent][ur] = TRUE;
   }
   if (handle == 0) {
       handle = call_out("dirty_caches", 0.1);
   }
}

static
void dirty_caches() {
   handle = 0;

   while (sizeof(stack)) {
       int    count;
       object ur, **lists, parent, ob;

       ur     = stack[0][0];
       lists  = stack[0][1];
       count  = stack[0][2];
       parent = stack[0][3];
       stack  = stack[1..];

       while (sizeof(lists) && !sizeof(lists[0])) {
	   lists = lists[1..];
       }

       if (sizeof(lists)) {
	   ob = lists[0][0];
	   lists[0] = lists[0][1..];

	   if ((count % 100) == 0) {
	       Debug("Crawler[" + name(ur) + "]: Crawling over objects; " + count + " left to do...");
	   }
	   count--;
	   crawlers[ur][1]++;
	   crawlers[ur][2]--;
	   /*
	    * Add to the bottom of the stack, so that all objects will progress,
	    * giving the impression of multithreading while not making the
	    * system crawl.
	    */
	   stack += ({ ({ ur, lists, count, parent }) });

	   if (ob) {
	       if (ob->query_child_count() > 0) {
		   start_crawler(ob, ur);
	       }
	       catch {
		   ob->dirty_caches();
	       }
	   } else {
	       Debug("Crawler[" + name(ur) + "]: destructed ur-sibling.");
	   }
	   if (handle == 0) {
	       handle = call_out("dirty_caches", 0.1);
	   }
	   return;
       } else {
	   /*
	    * Because a crawler is thrown back on the stack with 0 left to go,
	    * it possible that another one on the stack finishes and cleans up
	    * the crawlers[ur] data while traversing the parent links.
	    *
	    * That's why we need to check for crawlers[ur] as well.
	    */
	   while (ur && crawlers[ur] && !crawlers[ur][2] && (!children[ur] || !map_sizeof(children[ur]))) {
	       float diff;

	       parent = crawlers[ur][3];
	       diff = timediff(crawlers[ur][0], millitime());
	       Debug("Crawler[" + name(ur) + "]: Done! (" + diff + " seconds for " + crawlers[ur][1] + " objects; " +
		     ((float)crawlers[ur][1] / diff) + " obj/s)");
	       children[ur] = nil;
	       if (parent) {
		   crawlers[parent][1] += crawlers[ur][1];
		   children[parent][ur] = nil;
		   if (!map_sizeof(children[parent])) {
		       children[parent] = nil;
		   }
	       }
	       crawlers[ur] = nil;
	       ur = parent;
	   }
       }
   }
   Debug("Crawlers done.");
}
# else

atomic
void start_crawler(object ur) {
   object iterator;
   object child;
   int count;

   iterator = ur->query_ur_child_iterator();
   if (!iterator->has_next()) {
      return;
   }
   while (iterator->has_next()) {
      child = iterator->next();

      call_touch(child);

      if (ur->is_child_with_children(child)) {
	 start_crawler(child);
      }
      count ++;
   }
   SysLog("Iterated over <" + name(ur) + "> children: " + count);
}

# endif
