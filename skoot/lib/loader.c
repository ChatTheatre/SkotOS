static
void load_programs(string path, varargs int atch) {
   string *programs, program;
   int i;

   if (path[strlen(path)-1] != '/') {
      path += "/";
   }
   programs = get_dir(path + "*.c")[0];
   SysLog("Compiling " + sizeof(programs) + " programs in " + path);
   for (i = 0; i < sizeof(programs); i ++) {
      if (sscanf(programs[i], "%s.c", program)) {
	 if (atch) {
	   catch {
	       find_or_load(path + program);
	    }
	 } else {
	    find_or_load(path + program);
	 }
      }
   }
}
