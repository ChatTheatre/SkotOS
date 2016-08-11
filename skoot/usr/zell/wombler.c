# include <status.h>

int total_wombles;

int query_total() { return total_wombles; }

void loop(varargs int ix) {
   object ob;

   while (ix < status()[ST_OTABSIZE]) {
      if (ob = find_object("/base/obj/thing#" + ix)) {
	 catch {
	    ob->womble_prox();
	    ob->womble_details();
	    ob->womble_properties();
	 }
	 if (ob->query_wombles() > 0) {
	    total_wombles += ob->query_wombles();
	    SysLog("WOMBLER:: " + name(ob) + " -> #" + ob->query_wombles());
	 }
	 call_out("loop", 0.01, ix+1);
	 return;
      }
      ix ++;
   }
   SysLog("WOMBLER:: Done. A full " + total_wombles + " NRefs were changed.");
}

