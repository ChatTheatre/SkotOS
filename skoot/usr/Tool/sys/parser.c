/*
 * ~Tool/sys/parser.c
 *
 * Copyright 2002 Skotos Tech Inc 
 */

# include <type.h>
# include "~/include/tool.h"

inherit "/lib/parse";

private string grammar;

static void
create()
{
   ::create("~/tmp/grammar_scratch");
}

void
generate_grammar()
{
   int     i, sz_i, *nrs, counter;
   string  *str, result;
   object  *list, ob;
   mapping tokens, funcs;
   mixed   **data;

   tokens = ([ ]);
   funcs  = ([ ]);

   grammar =
      "inherit \"~/lib/grammar_functions\";\n" +
      "inherit \"~/lib/grammar_tokens\";\n" +
      "\n" +
      "mixed *make_text(mixed *tree) {\n" +
      "    return ({ tree[0] });\n" +
      "}\n" +
      "\n" +
      "mixed *append_text(mixed *tree) {\n" +
      "    return ({ tree[0] + tree[1] });\n" +
      "}\n" +
      "\n" +
      "mixed *list_entry(mixed *tree) {\n" +
      "    return ({ tree });\n" +
      "}\n" +
      "\n" +
      "mixed *list_append(mixed *tree) {\n" +
      "    return ({ tree[0] + ({ tree[2] }) });\n" +
      "}\n" +
      "\n" +
      "mixed *grammar_fallback(mixed *tree) {\n" +
      "    return ({ \"/usr/Tool/sys/tool\", \"cmd_fallback\", tree[0] });\n" +
      "}\n" +
      "\n" +
      "GrammarBegin\n" +
      "\n" +
      "# include \"~/data/tokens.y\"\n" +
      "\n";

   list = SYS_TOOL->query_handler_obs();
   sz_i = sizeof(list);
   for (i = 0; i < sz_i; i++) {
      int     j, sz_j;
      string  *lines;

      data = list[i]->query_grammar();
      if (!data) {
	 continue;
      }
      sz_j  = sizeof(data);
      for (j = 0; j < sz_j; j++) {
	 int k, sz_k;
	 string *words;

	 words = explode(data[j][0], " ") - ({ "" });
	 sz_k = sizeof(words);
	 for (k = 0; k < sz_k; k++) {
	    switch (words[k]) {
	    case "<object>":
	       words[k] = "object";
	       break;
	    case "<object>+":
	       words[k] = "object_list";
	       break;
	    case "<text>":
	       words[k] = "text";
	       break;
	    case "<text>+":
	       words[k] = "text_list";
	       break;
	    case "<number>":
	       words[k] = "number";
	       break;
	    case "<number>+":
	       words[k] = "number_list";
	       break;
	    case "<string>":
	       words[k] = "string";
	       break;
	    case "<string>+":
	       words[k] = "string_list";
	       break;
	    case "<anything>":
	       words[k] = "any";
	       break;
	    default:
	       tokens[words[k]] = TRUE;
	       words[k] = "token_" + words[k];
	       break;
	    }
	 }
	 grammar += "grammar: " + implode(words, " spaces ") + " ? grammar_" + counter + "\n";
	 funcs[counter] = ({ list[i] }) + data[j][1..];
	 counter++;
      }
   }

   sz_i = map_sizeof(tokens);
   str = map_indices(tokens);

   result =
      "/*\n" +
      " * token functions auto-generated on " + ctime(time()) + "\n" +
      " */\n" +
      "\n" +
      "private inherit \"/lib/string\";\n" +
      "inherit \"~/lib/grammar_utils\";\n" +
      "\n" +
      "static void initialize() {\n" +
      "    initialize_tokens(([\n";
   for (i = 0; i < sz_i; i++) {
      result += "        \"" +str[i] + "\": TRUE,\n";
   }
   result +=
      "    ]));\n" +
      "}\n" +
      "\n";
   for (i = 0; i < sz_i; i++) {
      result +=
	 "\n" +
	 "static mixed *token_" + str[i] + "(mixed *tree) {\n" +
	 "    return prefix_match(tree[0], \""+ str[i] + "\") ? tree : nil;\n" +
	 "}\n";
   }
   remove_file("~/lib/grammar_tokens.c");
   write_file("~/lib/grammar_tokens.c", result);

   result =
      "/*\n" +
      " * grammar functions auto-generated on " + ctime(time()) + "\n" +
      " */\n";
   sz_i = map_sizeof(funcs);
   nrs  = map_indices(funcs);
   data = map_values(funcs);
   for (i = 0; i < sz_i; i++) {
      int   j, sz_j;
      mixed *list;

      list = data[i];
      list[0] = "\"" + ur_name(list[0]) + "\"";
      list[1] = "\"" + list[1] + "\"";
      sz_j = sizeof(list);
      for (j = 2; j < sz_j; j++) {
	 if (typeof(list[j]) == T_STRING) {
	    list[j] = "tree[" + ((int)list[j] * 2) + "]";
	 } else {
	    list[j] = (string)list[j];
	 }
      }
      result +=
	 "\n" +
	 "static mixed *grammar_" + nrs[i] + "(mixed *tree) {\n" +
	 "    return ({ " + implode(list, ", ") + " });\n" +
	 "}\n";
   }
   remove_file("~/lib/grammar_functions.c");
   write_file("~/lib/grammar_functions.c", result);

   grammar +=
      "\n" +
      "grammar: any ? grammar_fallback\n" +
      "\n" +
      "any: bit     ? make_text\n" +
      "any: any bit ? append_text\n" +
      "\n" +
      "bit: object\n" +
      "bit: text\n" +
      "bit: number\n" +
      "bit: string\n" +
      "bit: other\n" +
      "bit: spaces\n" +
      "\n" +
      "object_list: object                    ? list_entry\n" +
      "object_list: object_list spaces object ? list_append\n" +
      "\n" +
      "text_list: text                  ? list_entry\n" +
      "text_list: text_list spaces text ? list_append\n" +
      "\n" +
      "number_list: number                    ? list_entry\n" +
      "number_list: number_list spaces number ? list_append\n" +
      "\n" +
      "string_list: string                    ? list_entry\n" +
      "string_list: string_list spaces string ? list_append\n" +
      "\n";
   sz_i = map_sizeof(tokens);
   str = map_indices(tokens);
   for (i = 0; i < sz_i; i++) {
      grammar += "token_" + str[i] + ": text ? token_" + str[i] + "\n";
   }
   grammar +=
      "\n" +
      "GrammarEnd\n";
   if (ob = find_object("~/tmp/grammar_scratch")) {
      ob->destruct();
   }
   if (status("~/lib/grammar_functions")) {
      compile_object("~/lib/grammar_functions");
   }
   if (status("~/lib/grammar_tokens")) {
      compile_object("~/lib/grammar_tokens");
   }
}

string
query_grammar()
{
   if (!grammar) {
      generate_grammar();
   }
   return grammar;
}

mixed *
parse(string line)
{
   if (!grammar) {
      generate_grammar();
   }

   return parse_string(grammar, line);
}
