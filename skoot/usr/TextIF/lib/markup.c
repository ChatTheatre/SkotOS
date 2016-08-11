/*
**	We contain the code to post-process game output just
**	before it's sent to the user.
**
**	Current structure:
**
**	0x00 <id> <header> 0x01 <text1> 0x03 <text2> 0x03 ... <textN> 0x02
**
**	where 'header' is clean ASCII, 'text' can contain recursively
**	other markup, and 'id' is a single byte used to identify what
**	it is we're doing. 0x00 begins the fragment, 0x01 ends the header,
**	0x03 separates text arguments, and 0x02 ends the fragment.
**
**	LATE NOTE: We are NOT doing the 0x03 separators right now; we
**	are only allowing a single <text> segment. When we do go ahead
**	and allow several, remember that the TextBit rule will yield
**	tree[1 .. sizeof(tree-2)] separate Texts that must each be
**	imploded with "", and then imploded with \003 together to
**	reconstruct the original string (in mutate).
**
**	For a future addition, I suggest we let the grammar return
**	mappings or perhaps an expanded array structure that includes
**	the original string, so we don't have to rebuild it in the
**	expand() and mutate() functions.
**
**	Zell 030708
*/

# include <type.h>

private inherit "/lib/string";

object query_body();
int query_html_client();
string get_colour(string id);

private
string query_grammar(string fun) {
   return
      "header = /\x00.[^\x01]*\x01/\n" +
      "string = /[^\x00-\x03]+/\n" +

      "Text: TextBit ? make_text_array\n" +
      "Text: Text TextBit ? build_text_array\n" +
      "TextBit: string\n" +
      "TextBit: header Text '\002' ? " + fun + "\n" +
      "TextBit: header '\002' ? " + fun + "\n";
}

static
mixed *make_text_array(mixed *tree) {
   return ({ tree });
}

static
mixed *build_text_array(mixed *tree) {
   return ({ tree[0] + ({ tree[1] }) });
}

static
mixed *make_tag(mixed *tree) {
   string colour, str;
   int nl;

   str = implode(tree[4], "");

   /* if there is a newline at the end of the embedded text, extract it */
   if (strlen(str) > 0 && str[strlen(str)-1] == '\n') {
      str = str[.. strlen(str)-2];
      nl = TRUE;
   }

   if (query_html_client()) {
      if (colour = get_colour(tree[2])) {
	 str = "<font color=" + colour + ">" + str + "</font>";
      }
   }
   if (nl) {
      str += "\n";
   }
   return ({ str });
}

static
mixed *make_cmd(mixed *tree) {
   string colour, tag, cmd, str;
   int nl;

   str = implode(tree[4], "");

   /* if there is a newline at the end of the embedded text, extract it */
   if (str[strlen(str)-1] == '\n') {
      str = str[.. strlen(str)-2];
      nl = TRUE;
   }

   if (query_html_client() && sscanf(tree[2], "%s|%s", tag, cmd)) {
      if (colour = get_colour(tag)) {
	 str = "<font color=" + colour + ">" + str + "</font>";
      }
      str = "<a xch_cmd='" + cmd + "'>" + str + "</a>";
   }
   if (nl) {
      str += "\n";
   }
   return ({ str });
}

static
mixed *make_indent(mixed *tree) {
   string str;

   str = implode(tree[4], "");

   if (query_html_client()) {
      str = "<table><tr><td>" + tree[2] +
	 "</td><td>" + str +
	 "</td></tr></table>\n";
      SysLog("Mutating string into: " + str);
   }
   return ({ str });
}

static
mixed *make_pre(mixed *tree) {
   string str;

   str = implode(tree[4], "");

   if (query_html_client()) {
      str = "<pre>" + str + "</pre>";
   }
   return ({ str });
}

static
mixed *make_lang(mixed *tree) {
   string str, lang;
   int nl, leg;

   str = implode(tree[4], "");

   /* if there is a newline at the end of the embedded text, extract it */
   if (str[strlen(str)-1] == '\n') {
      str = str[.. strlen(str)-2];
      nl = TRUE;
   }

   if (sscanf(tree[2], "%s|%d", lang, leg)) {
      mapping args;

      args = ([ "text": str,
		"language": lang,
		"legibility": leg
		 ]);

      query_body()->immediate_action("language/encounter", args);
      if (typeof(args["text"]) == T_STRING) {
	 str = args["text"];
      }
   }
   if (nl) {
      str += "\n";
   }
   return ({ str });
}

static
mixed *make_prop(mixed *tree) {
   string prop, true_value, false_value;

   false_value = implode(tree[4], "");

   if (sscanf(tree[2], "%s|%s", prop, true_value) == 2) {
      if (query_body()->query_property(prop)) {
	 false_value = true_value;
      }
   }
   return ({ false_value });
}

static
mixed *expand(mixed *tree) {
   int c;

   if (sizeof(tree) == 2) {
       tree = ({ tree[0], ({ "" }), tree[1] });
   }
   c = tree[0][1];

   if (c >= 0x80) {
      /* this should not happen: mutation should have taken care of it */
      return ({ tree[0] + implode(tree[1], "") + "\002" });
   }

   tree = ({ "", "", tree[0][2 .. strlen(tree[0])-2], "", tree[1] });

   switch(c) {
   case 3:
      return make_tag(tree);
   case 4:
      return make_cmd(tree);
   case 5:
      return make_indent(tree);
   case 7:
      return make_pre(tree);
   }
   error("unknown whatnot");
}

static
mixed *mutate(mixed *tree) {
   int c;

   if (sizeof(tree) == 2) {
       tree = ({ tree[0], ({ "" }), tree[1] });
   }
   c = tree[0][1];

   if (c < 0x80) {
      /* reconstruct original string for the second sweep */
      return ({ tree[0] + implode(tree[1], "") + "\002" });
   }

   tree = ({ "", "", tree[0][2 .. strlen(tree[0])-2], "", tree[1] });

   switch (c) {
   case 0x86: /* INLANG(...) */
      return make_lang(tree);
   case 0x88: /* PROP(...) */
      return make_prop(tree);
   default:
      break;
   }
   error("unknown whatnot");
}

static
mixed *make_skip(mixed *tree) {
   return ({ "" });
}

string expand_markup(string str) {
   mixed *tree;

   if (!strlen(str)) {
      return "";
   }
   catch {
      tree = parse_string(query_grammar("expand"), str);
   }
   if (!tree) {
      error("BAD PARSE: " + dump_value(str));
      return str;
   }

   return implode(tree[0], "");
}

string mutate_markup(string str) {
   mixed *tree;

   if (!strlen(str)) {
      return "";
   }
   catch {
      tree = parse_string(query_grammar("mutate"), str);
   }
   if (!tree) {
      error("BAD PARSE: " + dump_value(str));
      return str;
   }

   return implode(tree[0], "");
}
