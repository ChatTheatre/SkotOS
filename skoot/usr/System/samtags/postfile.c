# include <type.h>

int tag(mixed content, object context, mapping local, mapping args) {
   string file, text;

   args += local;

   if ((file = args["file"]) && (text = args["text"])) {
      remove_file(file + ".tmp");
      write_file(file + ".tmp", text);
      remove_file(file);
      rename_file(file + ".tmp", file);
      return TRUE;
   }
   return FALSE;
}
