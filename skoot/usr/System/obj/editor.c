/*
 * ~Tool/obj/editor.c
 *
 * An editor for (merry/bilbo/sam) scripts and whatever else comes to
 * mind later.
 *
 * Copyright Skotos Tech Inc 2002
 */

/*
 * TODO: Need some way to know if the user has disappeared, unless we want
 * to keep these objects around forever?
 */

private string target_file;
private object user;

static void
create(int clone)
{
}

private string editor_help(string text)
{
   switch (text) {
   case "h":
      return
"                        Editor Quick Reference\n" +
"\n" +
"          --LINES--                           --REGULAR EXPRESSIONS--\n" +
".       current line                    .       any single character\n" +
"$       last line                       ^   $   start of line, end of line\n" +
"5+7     line 12                         [a-z]   any character in range a .. z\n" +
"/pat/   first line after current        *       zero or more occurances of\n" +
"        in which pat appears                    preceding character\n" +
"?pat?   like /pat/, but searching       \\<  \\>  start of word, end of word\n" +
"        backwards\n" +
"                              --COMMANDS--\n" +
"(.)     a         append lines after .  (.,.) #         show numbered lines\n" +
"(.,.)   c         change lines          (.,.) p         show lines\n" +
"(.,.)   d         delete lines                q         quit edit session\n" +
"        e file    edit file             (.,.) s/pat/str substitute pat by str\n" +
"        f         show current file     (.,.) t line    copy lines\n" +
"(1,$)   g/pat/cmd run cmd on lines            u         undo last change\n" +
"                  matching pat          (1,$) w file    write to file\n" +
"(.)     i         insert lines before .       x         write changes and quit\n" +
"(.,.+1) j         join lines            (.+1) z         show page of lines\n" +
"(.,.)   m line    move lines            (1,$) I         indent LPC code\n";
   default:
      return "No such command.\n";
   }
}

string
initialize_editor(object obj, string path, string text)
{
   string str;

   user = obj;
   target_file = path;
   if (text) {
      remove_file(target_file);
      write_file(target_file, text);
   }
   str = editor("e " + target_file);
   if (text) {
      remove_file(target_file);
   }
   return str;
}

string
query_target_file()
{
   return target_file;
}

string
run_editor(string str)
{
   if (query_editor(this_object()) == "command" &&
       str && strlen(str) && str[0] == 'h') {
      return editor_help(str);
   }
   return editor(str);
}

nomask string
path_read(string path)
{
   return path == target_file ? path : nil;
}

nomask string
path_write(string path)
{
   return path == target_file ? path : nil;
}

void
die()
{
   switch (query_editor(this_object())) {
   case "insert":
      editor(".");
   case "command":
      editor("q!");
   default:
      destruct_object(this_object());
      break;
   }
}
