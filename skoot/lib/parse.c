/*
**	Buffer against the parse_string() grammar with a bit of
**	syntactic sugar.
**
**	The inheritor must have write-access to the scratch-object
**	filename that is passed to create().
**
**	Copyright Skotos Tech Inc 1999
*/


# include <status.h>

inherit "/lib/file";
private inherit "/lib/string";

private string last_raw_grammar;
private string transformed_grammar;
private string scratch_file;
private object parse_ob;
private int debug_ticks;

private void transform_grammar(string grammar);

static
void create(string scratch, varargs int debug) {
    scratch_file = scratch;
    debug_ticks = !!debug;
}

static
mixed *parse_string(string grammar, string str, varargs int trees) {
   if (!parse_ob || grammar != last_raw_grammar) {
      transform_grammar(grammar);
   }
   return parse_ob->parse(str, trees);
}

private
string interpret_special_chars(string str) {
   return replace_strings(str,
			  "\\a", "\a",
			  "\\b", "\b",
			  "\\t", "\t",
			  "\\n", "\n",
			  "\\v", "\v",
			  "\\f", "\f",
			  "\\r", "\r");
}

private mixed *
prepare_lines(string filename, string text)
{
    int   i, sz;
    mixed *lines;

    lines = explode(text, "\n");
    for (i = 0, sz = sizeof(lines); i < sz; i++) {
	lines[i] = ({ filename, i + 1, lines[i] });
    }
    return lines;
}

private
void transform_grammar(string grammar) {
   string  file, head, middle, tail, source;
   string *codes;
   string  code, *words;
   mixed  *lines;
   mapping word_map;
   int     i, num, sz;
   mapping func_code;

   /*
   **	Go through the grammar, leave token definitions, but check
   **	rule definitions for embedded LPC, replace them with function
   **	calls and finally write the functions to a scratch file, load
   **	it, delete the file, and return.
   **
   **   Extra feature: __STRINGS__.  This is a special rule that
   **   matches all the string-constants found in the grammar.
   **
   **   Disabled __STRINGS__ rule because nobody uses it and it takes up
   **   valuable grammar-space that can be used for other rules -- 2008/01/18
   */

   if (!sscanf(grammar, "%sGrammarBegin%sGrammarEnd%s",
	       head, middle, tail)) {
      error("no GrammarBegin/GrammarEnd section found");
   }

   /* Preparation, 
   /* 1st pass: Look for any '#include' statements and expand them */
   lines = prepare_lines("\"grammar\"", middle);
   while (i < sizeof(lines)) {
       string line;

       line = lines[i][2];
       if (line && strlen(line) && line[0] == '#') {
	   string file;

	   if (sscanf(line, "#include \"%s\"", file) ||
	       sscanf(line, "# include \"%s\"", file)) {
	       string text;
	       mixed  *newlines;

	       text = read_file(file);
	       if (!text) {
		   error("include file does not exist on line " + lines[i][1] + " of " + lines[i][0]);
	       }
	       newlines = prepare_lines(file, text);
	       lines = lines[..i - 1] + newlines + lines[i + 1..];
	       continue;
	   }
       }
       i++;
   }
   /*
    * 2nd pass: Look for inlined functions and replace them with external ones
    * with functionnames of the format f<number>.
    */
   codes = ({ });
   word_map = ([ ]);
   i = 0;
   func_code = ([ ]);
   while (i < sizeof(lines)) {
      string lhs, rhs, code;
      int j;

      lines[i][2] = interpret_special_chars(lines[i][2]);

      if (strlen(lines[i][2]) && lines[i][2][0] == '#') {
	 lines[i][2] = nil;
      } else {
	  string line, pre, text, post;

	  if (sscanf(lines[i][2], "%s:%s{{%s", lhs, rhs, lines[i][2]) == 3) {
	      code = "";
	      j = i;
	      while (i < sizeof(lines) && !sscanf(lines[i][2], "%s}}", lines[i][2])) {
		  code += lines[i][2] + "\n";
		  lines[i][2] = nil;
		  i ++;
	      }
	      if (i == sizeof(lines)) {
		  error("grammar missing }} counterpart to {{ on line " + lines[j][1] + " of " + lines[j][0]);
	      }
	      code += lines[i][2];
	      code = strip(code);
	      if (func_code[code]) {
		 lines[i][2] = lhs + ":" + rhs + " ? f" + func_code[code];
	      } else {
		 func_code[code] = num;
		 codes += ({ code });
		 lines[i][2] = lhs + ":" + rhs + " ? f" + num;
		 num ++;
	      }
	  }
 	  if (lines[i][2]) {
 	      string line;
	      
 	      line = lines[i][2];
 	      while (sscanf(line, "%s'%s'%s", pre, text, post) == 3) {
 		  if (!word_map[text]) {
 		      word_map[text] = 1;
 		  }
 		  line = post;
 	      }
 	  }
      }
      i ++;
   }

if (sscanf(middle, "%*s__STRINGS__")) {
   words = map_indices(word_map);
   sz = sizeof(words);
   if (sz > 0) {
       for (i = 0; i < sz; i++) {
 	   lines += ({ ({ nil, 0, "__STRINGS__: '" + words[i] + "'" }) });
       }
   }
}

   source = head +
      "private string grammar;\n" +
      (debug_ticks ? "# include <status.h>\n    private mapping function_ticks;\n" : "") +
      "\n" +
      "void set_grammar(string str) {\n" +
      "    grammar = str;\n" +
      "}\n" +
      "\n" +
      "mixed *parse(string str, varargs int trees) {\n" +
      "    this_object()->prepare();\n" +
      (debug_ticks ? "    function_ticks = ([ ]);\n" : "") +
      "    return parse_string(grammar, str, trees);\n" +
      "}\n" +
      "\n";
   if (debug_ticks) {
      source +=
	 "private void register_function_ticks(string func, int cost) {\n" +
	 "    mixed *data;\n" +
	 "    if (!function_ticks) {\n" +
	 "        function_ticks = ([ ]);\n" +
	 "    }\n" +
	 "    data = function_ticks[func];\n" +
	 "    if (!data) {\n" +
	 "        function_ticks[func] = data = ({ 0, 0 });\n" +
	 "    }\n" +
	 "    data[0]++;\n" +
	 "    data[1] += cost;\n" +
	 "}\n" +
	 "\n" +
	 "mapping query_function_ticks() {\n" +
	 "    return function_ticks[..];\n" +
	 "}\n";
   }
   if (debug_ticks) {
      for (i = 0; i < num; i ++) {
	 source +=
	    "static mixed *real" + i + "(mixed *tree) {\n" +
	    codes[i] + ";\n" +
	    "    return tree;\n" +
	    "}\n" +
	    "static mixed *f" + i + "(mixed *tree) {\n" +
	    "    int cost;\n" +
	    "    cost = status()[ST_TICKS];\n" +
	    "    tree = real" + i + "(tree);\n" +
	    "    register_function_ticks(\"f" + i + "\", cost - status()[ST_TICKS]);\n" +
	    "    return tree;\n" +
	    "}\n";
      }
   } else {
      for (i = 0; i < num; i ++) {
	 source += 
	    "static mixed *f" + i + "(mixed *tree) {\n" +
	    codes[i] + ";\n return tree;\n}\n";
      }
   }
   source += tail;

   for (i = 0, sz = sizeof(lines); i < sz; i++) {
       lines[i] = lines[i][2];
   }
   transformed_grammar = implode(lines - ({ nil }), "\n") + "\n";

   source += 
      "# if 0\n" +
      transformed_grammar +
      "# endif\n";

   SysLog("Trying to find: " + scratch_file);
   if (find_object(scratch_file)) {
      SysLog("OK: Destroying: " + scratch_file);
      scratch_file->destruct();
      SysLog("find_object() now returns... " +
	     dump_value(find_object(scratch_file)));
   }
   remove_file(scratch_file);
   write_file(scratch_file, source);
   parse_ob = compile_object(scratch_file, source);

   last_raw_grammar = grammar;
   parse_ob->set_grammar(transformed_grammar);
}

string query_transformed_grammar() { return transformed_grammar; }
object query_parse_object() { return parse_ob; }

void destroy_parse_object() {
   if (parse_ob) {
      parse_ob->destruct();
   }
}
