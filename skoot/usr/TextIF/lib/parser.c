# include <type.h>
# include <System.h>
# include <XML.h>

# define T_COMMAND   "command"
# define T_FORMAT    "format"
# define T_FUNCTION  "function"
# define T_HANDLER   "handler"
# define T_KEYWORDS  "keywords"
# define T_SOCIAL    "social"
# define T_VALUE     "value"
# define T_VERB      "verb"

inherit parse "/lib/parse";
inherit xml "~XML/lib/xmlparse";
inherit "~XML/lib/xmd";

private string *additional;
private object current_user, current_body;

static void create(string file) {
    parse::create(file);
    additional = ({ });
}

/* Register/unregister/list additional files with grammar */

void
register_additional(string file)
{
    if (!additional) {
	additional = ({ });
    }
    additional |= ({ file });
}

void
unregister_additional(string file)
{
    if (additional) {
	additional -= ({ file });
    }
}

string *
list_additional()
{
    return additional ? additional[..] : ({ });
}

private mapping convert_xml_param(mixed *arr) {
    int i, sz;
    mapping map;

    map = ([ ]);
    sz = sizeof(arr);
    for (i = 0; i < sz; i += 2) {
	map[arr[i]] = arr[i + 1];
    }
    return map;
}

private string make_format(string verb, string format) {
    return implode(explode("%s" + format + "%s", "%s"), verb);
}

private string make_function(string function) {
    return "{{ " + function + " }}";
}

private string expand_includes(string grammar) {
    string pre, property, filename, post;

    /* Look for <!--#include configd="..." --> entries and fill them in. */
    while (sscanf(grammar, "%s<!--#include configd=\"%s\" -->%s",
		  pre, property, post) == 3) {
	string filename;

	filename = CONFIGD->query(property);
	if (filename) {
	    string filetext;

	    filetext = read_file(filename);
	    if (filetext) {
		grammar = pre + filetext + post;
	    } else {
		/* Silently ignore non-existing files. */
		grammar = pre + post;
	    }
	} else {
	    /* Silently ignore non-existing properties. */
	    grammar = pre + post;
	}
    }

    /* Look for <!--#include file="..." --> entries and fill them in. */
    while (sscanf(grammar, "%s<!--#include file=\"%s\" -->%s",
		  pre, filename, post) == 3) {
	string filetext;

	filetext = read_file(filename);
	if (filetext) {
	    grammar = pre + filetext + post;
	} else {
	    /* Silently ignore non-existing properties. */
	    grammar = pre + post;
	}
    }

    /*
     * Look for <!--#additional --> and fill in the contents listed in
     * the files from the 'additional' list.
     */
    if (sscanf(grammar, "%s<!--#additional -->%s", pre, post) == 2) {
	if (additional && sizeof(additional) > 0) {
	    int i, sz;
	    string text;

	    text = "";
	    sz = sizeof(additional);
	    for (i = 0; i < sz; i++) {
		string filetext;

		filetext = read_file(additional[i]);
		if (filetext) {
		    text += filetext;
		} else {
		    WARNING(name(this_object()) + ": " + additional[i] + " does not exist.");
		}
	    }
	    grammar = pre + text + post;
	} else {
	    grammar = pre + post;
	}
    }
    return grammar;
}

/*
 * Look for a <COMMANDS>...</COMMANDS> section and parse it as XML, then replace
 * the original text with 'Cmd: ...' entries, define a __VERBS__ rule and
 * optionally define a few extra rules for several that are combined into one
 * command + param (+ function).
 */
static mixed *preprocess_grammar(string str) {
    string pre, xml, post;
    mapping free_verbs, other_verbs, keywords, social_verbs;

    /* Expand includes, if necessary */
    str = expand_includes(str);

    /* Let the games begin! */
    free_verbs   = ([ ]);
    other_verbs  = ([ ]);
    keywords     = ([ ]);
    social_verbs = ([ ]);
    if (sscanf(str, "%s<COMMANDS>%s</COMMANDS>%s", pre, xml, post) == 3) {
	mixed data;

	data = query_colour_value(parse_xml(xml));
	if (typeof(data) == T_ARRAY) {
	    int i, sz_i, list_counter;
	    string *lines;

	    list_counter = 0;
	    lines = ({ });
	    sz_i = sizeof(data);
	    for (i = 0; i < sz_i; i++) {
	        switch (query_colour(data[i])) {
		case COL_ELEMENT: {
		    string **verb_list, **format_list;
		    mapping param;

		    if (xmd_element(data[i]) != T_COMMAND && xmd_element(data[i]) != T_SOCIAL) {
			break;
		    }
		    param = convert_xml_param(xmd_attributes(data[i]));
		    if (param[T_KEYWORDS]) {
			int j, sz_j;
			string *words;

			words = explode(param[T_KEYWORDS], ",");
			sz_j = sizeof(words);
			for (j = 0; j < sz_j; j++) {
			    keywords[words[j]] = 1;
			}
		    }
		    if (xmd_content(data[i])) {
			/* Complex situation with one or more <FORMAT> or <VERB> elements. */
			int j, sz_j;
			string **verb_list, **format_list, token;
			mixed *content;

			content = query_colour_value(xmd_content(data[i]));
			verb_list   = ({ });
			format_list = ({ });
			sz_j = sizeof(content);
			for (j = 0; j < sz_j; j++) {
			    if (query_colour(content[j]) == COL_ELEMENT) {
				switch (xmd_element(content[j])) {
				case T_VERB: {
				    mapping param;

				    param = convert_xml_param(xmd_attributes(content[j]));
				    if (!social_verbs[param[T_VALUE]]) {
					verb_list += ({ ({ param[T_VALUE], param[T_FUNCTION] }) });
					if (param[T_KEYWORDS]) {
					    int k, sz_k;
					    string *words;

					    words = explode(param[T_KEYWORDS], ",");
					    sz_k = sizeof(words);
					    for (k = 0; k < sz_k; k++) {
						keywords[words[k]] = 1;
					    }
					}
				    }
				    break;
				}
				case T_FORMAT: {
				    mapping param;

				    param = convert_xml_param(xmd_attributes(content[j]));
				    format_list += ({ ({ param[T_VALUE], param[T_FUNCTION] }) });
				    if (param[T_KEYWORDS]) {
					int k, sz_k;
					string *words;

					words = explode(param[T_KEYWORDS], ",");
					sz_k = sizeof(words);
					for (k = 0; k < sz_k; k++) {
					    keywords[words[k]] = 1;
					}
				    }
				    break;
				}
				default:
				    /* Anything else, ignore. */
				    break;
				}
			    }
			}
			/* Check contradictions. */
			if (sizeof(verb_list) > 0 && param[T_VERB]) {
			    error(param[T_VERB] + ": Cannot have both a verb parameter and one or more <verb> elements");
			}
			if (sizeof(format_list) > 0 && param[T_FORMAT]) {
			    error(param[T_VERB] + ": Cannot have both a format parameter and one or more <format> elements");
			}
			if (xmd_element(data[i]) == T_SOCIAL) {
			    /*
			     * Add the verbs to the social_verbs map.
			     */
			    for (j = 0, sz_j = sizeof(verb_list); j < sz_j; j++) {
				social_verbs[verb_list[j][0]] = TRUE;
			    }
			} else {
			    /*
			     * Remove whatever verbs that are overruled by social verbs.
			     */
			    for (j = 0, sz_j = sizeof(verb_list); j < sz_j; j++) {
				if (social_verbs[verb_list[j][0]]) {
				    verb_list[j] = nil;
				}
			    }
			    verb_list -= ({ nil });
			}
			if (sizeof(verb_list) > 0 && sizeof(format_list) > 0) {
			    /* We have both! */
			    list_counter++;
			    token = "__LIST" + list_counter + "__";

			    for (j = 0, sz_j = sizeof(format_list); j < sz_j; j++) {
				lines += ({ "Cmd: " + make_format(token, format_list[j][0]) +
					    (format_list[j][1] ? " " + make_function(format_list[j][1]) : "") });
			    }
			    for (j = 0, sz_j = sizeof(verb_list); j < sz_j; j++) {
				other_verbs[verb_list[j][0]] = 1;
				lines += ({ token + ": '" + verb_list[j][0] + "'" +
					    (verb_list[j][1] ? " " + make_function(verb_list[j][1]) : "") });
			    }
			} else if (sizeof(verb_list) > 0) {
			    /* Just a list of verbs. */
			    list_counter++;
			    token = "__LIST" + list_counter + "__";
			    if (!param[T_FORMAT]) {
				param[T_FORMAT] = "%s";
			    }
			    lines += ({ "Cmd: " + make_format(token, param[T_FORMAT]) +
					(param[T_FUNCTION] ? " " + make_function(param[T_FUNCTION]) : "") });
			    for (j = 0, sz_j = sizeof(verb_list); j < sz_j; j++) {
				other_verbs[verb_list[j][0]] = 1;
				if (!param[T_FORMAT]) {
				    param[T_FORMAT] = "%s";
				}
				lines += ({ token + ": '" +
					    make_format(verb_list[j][0], param[T_FORMAT]) +
					    (verb_list[j][1] ? " " + make_function(verb_list[j][1]) : "") });
			    }
			} else if (sizeof(format_list) > 0) {
			    /* Just a list of formats for a given verb. */
			    if (param[T_VERB]) {
				if (xmd_element(data[i]) == T_SOCIAL) {
				    social_verbs[param[T_VERB]] = TRUE;
				} else if (social_verbs[param[T_VERB]]) {
				    break;
				}
				other_verbs[param[T_VERB]] = 1;
				for (j = 0, sz_j = sizeof(format_list); j < sz_j; j++) {
				    lines += ({ "Cmd: " + make_format("'" + param[T_VERB] + "'", format_list[j][0]) +
						(format_list[j][1] ? " " + make_function(format_list[j][1]) : "") });
				}
			    } else {
				DEBUG("Missing \"verb\" parameter for <COMMAND> entry?");
			    }
			} else {
			    /* Simple situation after all? */
			    if (param[T_VERB] && param[T_FORMAT] == "*") {
				if (xmd_element(data[i]) == T_SOCIAL) {
				    social_verbs[param[T_VERB]] = TRUE;
				} else if (social_verbs[param[T_VERB]]) {
				    break;
				}
				/* Free-format commands. */
				if (param[T_HANDLER]) {
				    free_verbs[param[T_VERB]] = param[T_HANDLER];
				} else {
				    free_verbs[param[T_VERB]] = 1;
				}
			    } else {
				/* Simple command. */
				if (xmd_element(data[i]) == T_SOCIAL) {
				    social_verbs[param[T_VERB]] = TRUE;
				} else if (social_verbs[param[T_VERB]]) {
				    break;
				}
				other_verbs[param[T_VERB]] = 1;
				if (!param[T_FORMAT]) {
				    param[T_FORMAT] = "%s";
				}
				lines += ({ "Cmd: " + make_format("'" + param[T_VERB] + "'", param[T_FORMAT]) +
					    (param[T_FUNCTION] ? " " + make_function(param[T_FUNCTION])            : "") });
			    }
			}
		    } else {
			/* Simple situation, no sub-elements. */

			if (xmd_element(data[i]) == T_SOCIAL) {
			    if (param[T_VERB]) {
				social_verbs[param[T_VERB]] = TRUE;
			    }
			} else if (param[T_VERB] && social_verbs[param[T_VERB]]) {
			    break;
			}
			if (param[T_VERB] && param[T_FORMAT] == "*") {
			    /* Free-format commands. */
			    if (param[T_HANDLER]) {
			       free_verbs[param[T_VERB]] = param[T_HANDLER];
			    } else {
			       free_verbs[param[T_VERB]] = 1;
			    }
			} else {
			    /* Simple command. */
			    other_verbs[param[T_VERB]] = 1;
			    if (!param[T_FORMAT]) {
				param[T_FORMAT] = "%s";
			    }
			    lines += ({ "Cmd: " + make_format("'" + param[T_VERB] + "'", param[T_FORMAT]) +
					(param[T_FUNCTION] ? " " + make_function(param[T_FUNCTION])            : "") });
			}
		    }
		    break;
		}
		default:
		    break;
		}
	    }
	    str = pre + "\n" +
		  implode(lines, "\n") + "\n" +
		  "\n" +
		  (map_sizeof(free_verbs) ?
		   "__FREE__: '" + implode(map_indices(free_verbs), "'\n__FREE__: '") + "'\n\n" :
		   "") +
		  (map_sizeof(other_verbs) ?
		   "__VERBS__: '" + implode(map_indices(other_verbs), "'\n__VERBS__: '") + "'\n\n" :
		   "") +
		  (map_sizeof(keywords) ?
		   "__KEYWORDS__: '" + implode(map_indices(keywords), "'\n__KEYWORDS__: '") + "'\n\n" :
		   "") +
		  post;
	}
    }
    return ({ str, free_verbs, other_verbs, keywords });
}

object query_current_user() {
    return current_user;
}

object query_current_body() {
    return current_body;
}

mixed *parse_string(string grammar, string str, varargs object user, object body) {
    current_user = user;
    current_body = body;

    return ::parse_string(grammar, str);
}
