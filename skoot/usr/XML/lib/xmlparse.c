/*
**	This is an alternate XML parser, using parse_string() only for
**	lexical purposes; creating a token stream -- which we parse here
**	using simple recursive descent.
**
**	Alright, we dumped parse_string() for that too -- it's now an
**	all-out LPC lexer/parser. Slow. Hm.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <XML.h>
# include <SID.h>
# include <type.h>
# include <System.h>
# include <nref.h>

inherit "/lib/womble";

private inherit "/lib/string";

inherit "/usr/XML/lib/entities";
private inherit "/usr/XML/lib/xmd";
private inherit "/usr/DTD/lib/dtd";

private int peek, loose;

private string *translate;

# define EOF	-1

# include "/usr/XML/include/XMLIn.h"
# include "/usr/XML/include/XMLOut.h"

void womble_xml() {
   res = nil;
   attr = nil;
   stack = nil;
}

private mixed convert(mixed content, varargs string type, int strip);

private void p_virgin(varargs int oneof);
private void p_oneof();
private void p_ref();
private void p_tag();
private void p_attr(varargs int ref);



static
void create() {
   string str;
   int i;

   ::create();	/* entities */

   translate = allocate(0x100);

   str = "'\\x00'";
   for (i = 0; i < 0x20; i ++) {
      str[4] = i % 10;
      str[3] = i / 10;
      translate[i] = str;
   }
   for (i = 0x80; i < 0x100; i ++) {
      str[4] = i % 10;
      str[3] = i / 10;
      translate[i] = str;
   }
   str = "' '";
   for (i = 0x20; i < 0x80; i ++) {
      str[1] = i;
      translate[i] = str;
   }
}


/* in peek mode, we stop parsing after reading the first tag */

static
mixed parse_xml(mixed str, varargs string file, int peekflag, int looseflag) {
   if (typeof(str) != T_ARRAY) {
      str = ({ str });
   }
   peek = peekflag;
   loose = looseflag;

   /* NewStream expects array of strings */
   NewStream(str, file ? file : "N/A");
   NewOut();
   NewStack();
   p_virgin();

   return convert(Result(), XML_PCDATA, TRUE);
}

/*
** parsing XML requires a simple state machine: being in the virgin
** state means we're going through normal text looking for markup
*/

private
void p_virgin(varargs int oneof) {
   for (;;) {
      switch(char) {
      case EOF:
	 if (stack) {
	    LexErr("unexpected EOF while still in tag " + TopTag());
	 }
	 Flush();
	 return;
      case ' ':
	 if (!type || type == XML_PCDATA) {
	    string spaces;
	    spaces = "";
	    while (char == ' ') {
	       spaces += " ";
	       Scan();
	    }
	    if (char != EOF && char != '\n') {
	       Nugget(spaces);
	    }
	 } else {
	    CharOut(char);
	    Scan();
	 }
	 break;
      case '\n':
	 if (!type || type == XML_PCDATA) {
	    string spaces;

	    while (char == '\n') {
	       while (char == '\n') {
		  Scan();
	       }
	       spaces = "";
	       while (char == ' ') {
		  spaces += " ";
		  Scan();
	       }
	    }
	    if (char != EOF) {
	       Nugget("\n");
	       if (strlen(spaces)) {
		  Nugget(spaces);
	       }
	    }
	 } else {
	    CharOut('\n');
	    Scan();
	 }
	 break;
      case '|': case '}':
	 if (!type || type == XML_PCDATA) {
	    if (oneof) {
	       /* we break here and let p_oneof deal */
	       return;
	    }
	 }
	 CharOut(char);
	 Scan();
	 break;
      case '$':
	 if (!type || type == XML_PCDATA) {
	    Scan();
	    p_ref();
	 } else {
	    CharOut('$');
	    Scan();
	 }
	 break;
      case '<':
	 Scan();
	 p_tag();
	 if (peek) {
	    Flush();
	    return;
	 }
	 break;
      case '&': {
	 string ent;
	 mixed out;

	 Scan();
	 ent = ScanEntity();
	 if (ent) {
	    out = entity_to_ascii(ent);
	    if (out == nil) {
	       TxtOut("&" + ent + ";");
	    } else {
	       CharOut(out);
	    }
	    if (char == ';') {
	       Scan();
	    }
	 } else {
	    CharOut('&');
	 }
	 break;
      }
      case '\\':
	 Scan();
	 XDebug("After scanning past \\, char is " + translate[char]);
	 CharOut(char);
	 Scan();
	 break;
      case '{':
	 if (!type || type == XML_PCDATA) {
	    if (!peek) {
	       /* no Scan() here, p_oneof likes it that way */
	       p_oneof();
	    }
	 } else {
	    CharOut(char);
	    Scan();
	 }
	 break;
      default:
	 CharOut(char);
	 Scan();
	 break;
      }
   }
}

/* most of p_oneof would be in ~SAM if we had the necessary infrastructure */

private
void p_oneof() {
   mixed *nuggets, retval;
   int first, i;

   nuggets = ({ });

   Push();
   do {
      Scan();
      NewOut();
      p_virgin(TRUE);
      Flush();
      nuggets += ({ convert(Result(), XML_PCDATA) });
      if (char != '}' && char != '|') {
	 LexErr("got " + translate[char] + "; expecting '|' or '}'");
      }
   } while (char != '}');
   Pop();

   retval = "/usr/SAM/sys/sugar"->parse_specials(nuggets);
   if (typeof(retval) == T_STRING) {
      LexErr(retval);
   }
   Nugget(retval);

   Scan();
}

private
int is_merry_expression(string str) {
   int i, j;

   str = strip(str);

   switch(str[strlen(str)-1]) {
   case ';':
      /* it's definitely a statement */
      return FALSE;
   case '}':
      /* could be the end of a statement block, or a ${} reference */
      for (j = strlen(str)-2; j >= 0; j --) {
	 if (str[j] == ';') {
	    /* it's a statement */
	    return FALSE;
	 }
	 if (str[j] == '{') {
	    /* it's an expression */
	    return TRUE;
	 }
      }
      /* weird; assume expression */
   }
   /* otherwise I guess it's an expression? */
   return TRUE;
}


/* we're here when we've just seen a $; we expect a variable reference */

private
void p_ref() {
   string str;

   switch(char) {
   case EOF:
      LexErr("surprising EOF in SAM reference");
      return;
   case '(':
      Scan();
      str = ScanName();
      if (char == ' ' || char == '\n') {
	 /* allow e.g. $(Capitalize str=$Name) */
	 p_attr(TRUE);
      } else {
	 attr = ({ });
      }
      Expect(')'); Scan();
      Nugget(xmd_ref(str, attr));
      return;
   case 'a'..'z': case 'A'..'Z':
   case '\"': case '\'':
      str = ScanName();
      Nugget(xmd_ref(str, nil));
      return;
   case '[':
      /* The whole Merry-inside-XML-parser thing is pretty nasty :) */
      Scan();
      str = strip(ScanMerry());

      if (is_merry_expression(str)) {
	 str = "return " + str + ";";
      }
      Nugget(xmd_ref("Merry.Eval", ({
	 "node",
	    new_object("/usr/SkotOS/data/merry", str),
	    "line", _line,
	    })));
      Scan();
      return;
   default:
      LexErr("unexpected token");
      break;
   }
}

private
void p_tag() {
   string tmp;
   mixed content;

   switch(char) {
   case EOF:
      /* TODO: some error */
      return;
   case '?': {
      int hide;

      /* <? XXX ?> construction can contain references */
      Scan();

      switch(char) {
      case 'a'..'z': case 'A'..'Z':
	 tmp = ScanName();
	 break;
      default:
	 LexErr("<? followed by " + translate[char]);
      }
      hide = (tmp == "hide");

      if (!hide) {
	 TxtOut("<?" + tmp);
      }
      do {
	 while (char != '?') {
	    switch(char) {
	    case EOF:
	       LexErr("unexpected EOF in ? construct");
	    case '$':
	       Scan();
	       p_ref();
	       break;
	    default:
	       CharOut(char);
	       Scan();
	       break;
	    }
	 }
	 Scan();
	 if (!hide || char != '>') {
	    CharOut('?');
	 }
      } while (char != '>');
      if (!hide) {
	 CharOut('>');
      }
      Scan();
      return;
   }
   case '!': {
      int seen;

      Scan();
      if (char != '-') {
	 LexErr("confusing <! construct");
      }
      Scan();
      if (char != '-') {
	 LexErr("confusing <! construct");
      }
      Scan();
      TxtOut("<!--");
      for (;;) {
	 switch(char) {
	 case '-':
	    CharOut(char);
	    Scan();
	    seen ++;
	    break;
	 case '>':
	    CharOut(char);
	    Scan();
	    if (seen >= 2) {
	       return;
	    }
	    seen = 0;
	    break;
	 case EOF:
	    LexErr("eof in comment");
	 default:
	    CharOut(char);
	    Scan();
	    seen = 0;
	    break;
	 }
      }
   }
   case 'a'..'z': case 'A'..'Z':
      Debug("Nugget: " + dump_value(nugget));
      tmp = tag = ScanName();
      XDebug("scanned tag " + tag);
      p_attr();
      if (char == '/') {
	 Scan();
	 Expect('>');
	 Scan();
	 Nugget(xmd_text(tag, attr, nil));
	 Debug("Nogget: " + dump_value(nugget));
	 return;
      }
      if (char == '>') {
	 Scan();
	 if (peek) {
	    /* return this tag as if it were empty */
	    Nugget(xmd_text(tag, attr, nil));
	    return;
	 }
	 Push();
	 NewOut(); /* this will clear tag, so use tmp */
	 if (SID->query_node(tmp)) {
	    type = SID->query_node(tmp)->query_type();
	    /* SysLog("Setting TYPE: " + dump_value(type)); */
	 }
	 XDebug("after push, stack is " + dump_value(stack));
	 return;
      }
      LexErr("expecting / or >");
   case '/':
      Flush();
      content = convert(Result(), type ? type : XML_PCDATA, !!type);
      Scan();

      tmp = ScanName();

      if (Empty()) {
	 LexErr("closing tag " + tmp + " before opening tag");
      }
      Pop();

      if (!tag) {
	 error("empty tag");
      }	
      if (lower_case(tmp) != lower_case(tag)) {
	 LexErr("closing tag " + tmp + " doesn't match opening tag " + tag);
      }
      ScanPastBlanks();
      Expect('>');
      Scan();

      Nugget(xmd_text(tag, attr, content));
      break;
   default:
      LexErr("unexpected token");
      break;
   }
}

private
void p_attr(varargs int ref) {
   string var, atype;
   object node;
   mixed content;
   int stop;

   attr = ({ });

   for (;;) {
      ScanPastBlanks();
      switch(char) {
      case '/': case '>':
	 if (ref) {
	    LexErr("unexpected character " + translate[char]);
	 }
	 return;
      case ')':
	 if (!ref) {
	    LexErr("unexpected character " + translate[char]);
	 }
	 return;
      case 'a'..'z': case 'A'..'Z': case '0'..'9':
	 var = ScanName();
	 break;
      case EOF:
	 LexErr("unexpected EOF expecting attribute name");
      default:
	 LexErr("expecting attribute name, got " + translate[char]);
      }

      node = tag ? SID->query_node(tag) : nil;
      atype = node ? node->query_attribute_type(var) : nil;
      if (!loose && node && !atype) {
	 LexErr("unexpected attribute " + var + " in element " + tag);
      }

      switch(char) {
      case '=':
	 Scan();
	 switch(char) {
	 case 'a'..'z': case 'A'..'Z': case '0'..'9':
	 case '#': case '-': case '_': case '.':
	    attr += ({ var, ScanValue() });
	    break;
	 case '$':		    /* TODO: get rid of this */
	    Scan();
	    Push();
	    NewOut();
	    p_ref();
	    content = convert(Result(), atype);
	    Pop();
	    attr += ({ var, content });
	    break;
	 case '\'': case '\"':
	    stop = char;
	    Scan();

	    Push();
	    NewOut();

	    while (char != stop) {
	       switch(char) {
	       case EOF:
		  LexErr("unexpected EOF expecting attribute value");
	       case '\n':
		  LexErr("unexpected end of line in value of attribute " + var);
	       case '$':
		  Scan();
		  if (!atype || atype == XML_PCDATA) {
		     p_ref();
		  } else {
		     CharOut('$');
		  }
		  break;
	       case '\\':
		  Scan();
		  if (TRUE || char == stop) {
		     CharOut(char);
		     Scan();
		  } else {
		     CharOut('\\');
		  }
		  break;
	       default:
		  CharOut(char);
		  Scan();
		  break;
	       }
	    }
	    Scan();
	    Flush();
	    content = Result();
	    Pop();

	    content = convert(content, atype);

	    attr += ({ var, content });
	    break;
	 default:
	    if (!tag) tag = "UNKNOWN";
	    if (!var) var = "UNKNOWN";
	    LexErr("bad value for tag '" + tag + "' attribute '" + var + "'");
	 }
	 break;
      case ' ': case '\n':
      case '/': case '>':
      case ')':
	 attr += ({ var, TRUE });
	 break;
      default:
	 LexErr("bad token " + char + " attribute value for " + var);
      }
   }
}

private
mixed convert(mixed content, varargs string ltype, int strip) {
   int implode, i;

   /*
   **	The biggest source of confusion regarding this function is that
   **	it is called in three very different cases: one in which type is
   **	XML_PCDATA, one in which it's nil, and one when it's something
   **	else.
   **
   **	When ltype == XML_PCDATA, 'content' already contains what it's
   **	supposed to contain: we run the optimization loop and then tidy
   **	things up through xmd_make_pcdata.
   **
   **	When ltype == nil, we're parsing the attribute value of a SAM
   **	tag: the type of those is never known. If 'content' is a string
   **	we perform mixed_to_ascii, which performs the basic 'untyped'
   **	conversions defined in ~DTD -- we check for OBJ() and NREF().
   **
   **	When ltype is something else, we perform a real conversion:
   **	this is the case when e.g. editing properties, which are of
   **	type lpc_mixed.
   */


   /* regardless of what 'content' is, we can do certain optimizations */
   while (typeof(query_colour_value(content)) == T_ARRAY) {
      /* loop while there are still useful optimizations */
      if (sizeof(query_colour_value(content)) == 0) {
	 content = "";
      } else if (sizeof(query_colour_value(content)) == 1) {
	 /* collapse single-element arrays */
	 content = query_colour_value(content)[0];
      } else {
	 /* leave optimization loop */
	 break;
      }
   }
   if (ltype == XML_PCDATA) {
      /* this is not really a conversion! */
      if (!query_colour(content)) {
	 /* unless it's already e.g. COL_ELEMENT, create a PCDATA */
	 content = NewXMLPCData(content);
	 if (strip) {
	    content = xmd_strip_pcdata(content);
	 }
      }
      return content;
   }
   /* if the content is a string, convert according to ltype (may be nil) */
   if (typeof(content) == T_STRING) {
      catch {
	 content = ascii_to_typed(content, ltype);
      } : {
         DEBUG("content = " + dump_value(content));
	 LexErr(SYSLOGD->query_last_error());
      }
   } else if (!ltype && !query_colour(content)) {
      /* argh! hacks hacks hack! */
      content = NewXMLPCData(content);
   }
   if (!ltype) {
      /* if there is no type, we're an attribute value: do no more */
      return content;
   }
   if (query_type_colour(ltype)) {
      int val;

      /* if there is a colour associated with this type, paint the value */
      switch (val = query_type_colour(ltype)) {
      default:
      case COL_NREF:
         if (query_colour(content) == COL_NREF) {
            return content;
         }
         DEBUG("content = " + dump_value(content));
	 error("Does this even happen in practice? [colour " + val + "]");
      case COL_ELEMENT:
	 return NewXMLElement(content);
      case COL_PCDATA:
	 return NewXMLPCData(content);
      case COL_SAMREF:
	 return NewXMLSAMRef(content);
      }
   }
   /* otherwise, check that the data is what it's supposed to be */
   if (!test_raw_data(content, ltype)) {
      XDebug(dump_value(content));
      LexErr("expecting type " + ltype);
   }
   /* done */
   return content;
}
