/*
**	/usr/System/include/find_or_load.c
**
**	Copyright Skotos Tech Inc 1999
*/

object find_or_load(string path) {
   object ob;

   ob = find_object(path);
   if (!ob) {
      ob = ::compile_object(path);
      if (!sscanf(path, "%*s/lib/")) {
	 ::call_other(path, "???");
      }
   }
   return ob;
}
