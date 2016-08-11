static
string name(object ob) {
   string oname;
   object ur;
   int num;

   oname = ::call_other(ob, "query_object_name");
   if (!oname) {
      if (ur = ::call_other(ob, "query_ur_object")) {
	 if (::sscanf(::object_name(ob), "%*s#%d", num)) {
	    if (oname = ::call_other(ur, "query_object_name")) {
	       return "[" + oname + "]#" + num;
	    }
	 }
      }
   }
   return oname ? oname : ::object_name(ob);
}

