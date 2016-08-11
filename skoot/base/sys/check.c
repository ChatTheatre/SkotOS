/* 
**	check.c
**
**	I am using this to search all the objects looking for a specific
**	thing. 
**
**	Wes Connell
*/

private inherit "/base/lib/urcalls";

object inspect(object ob) {
   object *inv;
   int i, inv_size;

   if (!ob->query_ur_object())
      return ob;
}

void object_inspect() {
   object *list, master, ob, tmp;
   int i, list_size; 
 
   list = ({ });
 
   if (master = find_object("/base/obj/thing")) {
      if (function_object("query_clone_count", master) == CLONABLE) {
	 for (ob = master->query_next_clone();
	      ob != master;
	      ob = ob->query_next_clone()) {
	    if (tmp = inspect(ob))
	       list += ({ tmp });
	 }
      }
   }

  XDebug("XX object_inspect() size list: " + sizeof(list));

}
