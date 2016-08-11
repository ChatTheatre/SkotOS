/*
**	/usr/XML/lib/xmlgen.c
**
**	This inheritable generates XML from the internal data format,
**	and is thus quite closely tied to ~XML/lib/xmlparse.c
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <SID.h>
# include <XML.h>

private inherit "/lib/string";

inherit "/usr/XML/lib/entities";
private inherit "/usr/XML/lib/xmd";
private inherit "/usr/DTD/lib/dtd";

# define RIGHT_MARGIN	70

static string generate_pcdata(mixed pcdata);

# define EscapePCData(str) \
   replace_strings(str,		\
          "\\", "\\\\",         \
          "&", "\\&",		\
          "$", "\\$",		\
	  "<", "\\<",		\
	  ">", "\\>",		\
	  "{", "\\{",		\
	  "}", "\\}",		\
	  "|", "\\|")

string
fiddle_ref(string ref) {
   /* this check should be wider, but in practice this is what happens */
   if (sscanf(ref, "%*s ")) {
      return "\"" + ref + "\"";
   }
   return ref;
}

private
string xml_attr(string tag, mixed *attr) {
   string str, type, astr;
   int i;

   str = "";
   if (!attr) {
      return str;
   }
   for (i = 0; i < sizeof(attr); i += 2) {
      if (attr[i+1] != nil) {
	 if (SID->query_node(tag)) {
	    type = SID->get_node(tag)->query_attribute_type(attr[i]);
	 } else {
	    type = nil;
	 }
	 if (type == XML_SAMREF || query_colour(attr[i+1]) == COL_SAMREF || query_colour(attr[i+1]) == COL_PCDATA) {
	    astr = generate_pcdata(attr[i+1]);
	 } else {
	    astr = typed_to_ascii(attr[i+1], type);
	    astr = replace_strings(astr, "\"", "\\\"");
	    if (type != XML_PCDATA && query_colour(attr[i+1]) != COL_PCDATA) {
	       astr = replace_strings(astr, "$", "\\$");
	    }
	 }
	 str +=" " + fiddle_ref(attr[i]) + "=\"" + astr + "\"";
      }
   }
   return str;
}

private
string xml_head(string tag, mixed *attr, mixed body) {
   string head;
   int i;

   head = "<" + tag + xml_attr(tag, attr);

   if (body == nil) {
      return head + "/>";
   }
   return head + ">";
}

/* obvious module dependency breaker -- temporary */
# define SUGAR	"/usr/SAM/sys/sugar"

static
string generate_pcdata(mixed pcdata) {
   if (!pcdata || !query_colour_value(pcdata)) {
      return "";
   }
   switch(query_colour(pcdata)) {
   case COL_ELEMENT: {
      object node;
      string tmp, tag, type, body;

      if (node = SID->query_node(xmd_element(pcdata))) {
	 tag = node->query_name();
      } else {
	 tag = xmd_element(pcdata);
      }

      if (tmp = SUGAR->generate_specials(tag, xmd_attributes(pcdata), xmd_content(pcdata))) {
	 return tmp;
      }
      if (xmd_content(pcdata) == nil ||
	  (typeof(xmd_content(pcdata)) == T_ARRAY  && sizeof(xmd_content(pcdata)) == 0) ||
	  (typeof(xmd_content(pcdata)) == T_STRING && strlen(xmd_content(pcdata)) == 0)) {
	 return xml_head(tag, xmd_attributes(pcdata), nil);
      }
      type = node ? node->query_type() : nil;
      body = typed_to_ascii(xmd_content(pcdata), type);

      /* try to figure out when escaping has already been done */
      if ((type || typeof(xmd_content(pcdata)) == T_STRING) &&
	  type != XML_PCDATA && query_colour(xmd_content(pcdata)) != COL_PCDATA) {
	 body = EscapePCData(body);
      }
      return xml_head(tag, xmd_attributes(pcdata), xmd_content(pcdata)) +
	 body + "</" + tag + ">";
   }
   case COL_SAMREF: {
      string tmp;

      if (tmp = SUGAR->generate_reference(
	     xmd_ref_ref(pcdata),
	     xmd_ref_attributes(pcdata))) {
	 return tmp;
      }
      return "$(" + fiddle_ref(xmd_ref_ref(pcdata)) +
	 xml_attr(xmd_ref_ref(pcdata), xmd_ref_attributes(pcdata)) + ")";
   }
   case COL_PCDATA: {
      string res;
      int i;

      pcdata = query_colour_value(pcdata);
      if (typeof(pcdata) == T_STRING) {
	 return EscapePCData(pcdata);
      }
      if (typeof(pcdata) == T_ARRAY) {
	 res = "";
	 for (i = 0; i < sizeof(pcdata); i ++) {
	    res += generate_pcdata(pcdata[i]);
	 }
      }
      return res;
   }
   }
   return EscapePCData(untyped_to_ascii(pcdata));
}

static
void generate_xml(mixed data, object res, varargs string indent) {
   object node;
   string tag, head, body, tail, type;
   int i;

   if (!indent) {
      indent = "";
   }

   if (data == nil) {
      return;
   }
   data = query_colour_value(data);
   if (typeof(data) == T_STRING) {
      res->append_string(EscapePCData(data));
      return;
   }
   if (typeof(data) != T_ARRAY) {
      error("weird value"); /* this will change in time TODO */
   }
   if (node = SID->query_node(data[0])) {
      tag = node->query_name();
   } else {
      tag = data[0];
   }
   if (data[2] == nil ||
       (typeof(data[2]) == T_ARRAY  && sizeof(data[2]) == 0) ||
       (typeof(data[2]) == T_STRING && strlen(data[2]) == 0)) {
      res->append_string(indent + xml_head(tag, data[1], nil) + "\n");
      return;
   }

   head = xml_head(tag, data[1], data[2]);
   tail = "</" + tag + ">\n";

   if (!node || node->is_parent()) {
      res->append_string(indent + head + "\n");
      for (i = 0; i < sizeof(data[2]); i ++) {
	 if (typeof(data[2][i]) != T_STRING || data[2][i] != " ") {
	    generate_xml(data[2][i], res, indent + "  ");
	 }
      }
      res->append_string(indent + tail);
      return;
   }
   type = node ? node->query_type() : nil;

   body = typed_to_ascii(data[2], type);
   if (type != XML_PCDATA) {
      body = EscapePCData(body);
   }
   if (strlen(head+body+tail)+strlen(indent) < RIGHT_MARGIN) {
      res->append_string(indent + head + body + tail);
      return;
   }
   body = strip(body);
   res->append_string(indent + head + "\n" +
		      break_paragraph(indent + "   ", body, RIGHT_MARGIN) +
		      indent + tail);
}
