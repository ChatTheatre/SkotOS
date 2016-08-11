/*
**	/lib/xmd.c
**
**	This defines the XMD (xtended markup data) structure, which is
**	pretty much just the internal -- binary -- version of XML. The
**	name XML should really be used for the ASCII version.
**
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>
# include <XML.h>

private inherit "/lib/array";
private inherit "/lib/string";

static
mixed xmd_elts(string name, mixed *attr, mixed subelts...) {
   return NewXMLElement(({ lower_case(name), attr, subelts }));
}

static
mixed xmd_text(string name, mixed *attr, mixed value) {
   return NewXMLElement(({ lower_case(name), attr, value }));
}

static mixed xmd_element(mixed elt)	{ return query_colour_value(elt)[0]; }
static mixed xmd_attributes(mixed elt)	{ return query_colour_value(elt)[1]; }
static mixed xmd_content(mixed elt)	{ return query_colour_value(elt)[2]; }

static
mixed xmd_ref(string ref, varargs mixed *attr) {
   return NewXMLSAMRef(({ ref, attr ? attr : ({ }) }));
}

static string xmd_ref_ref(mixed ref) { return query_colour_value(ref)[0]; }
static mixed *xmd_ref_attributes(mixed ref) { return query_colour_value(ref)[1]; }

static
mixed *xmd_wipe_tags(mixed elts, string tags...) {
   int i;

   elts = query_colour_value(elts)[..];	/* copy */

   for (i = 0; i < sizeof(elts); i ++) {
      if (typeof(elts[i]) == T_ARRAY && member(xmd_element(elts[i]), tags)) {
	 elts[i] = nil;
      }
   }
   return elts - ({ nil });
}

# define WS(n)	((n) == ' ' || (n) == '\n')

static
mixed xmd_strip_pcdata(mixed xmd) {
   if (typeof(query_colour_value(xmd)) == T_STRING) {
      return NewXMLPCData(strip(query_colour_value(xmd)));
   }
   if (typeof(query_colour_value(xmd)) == T_NIL) {
      return NewXMLPCData(nil);
   }
   if (typeof(query_colour_value(xmd)) != T_ARRAY) {
      error("not xmd: " + dump_value(xmd));
   }
   if (query_colour(xmd) == COL_SAMREF || query_colour(xmd) == COL_ELEMENT) {
      /* done */
      return xmd;
   }
   /* Must be PCDATA then. */
   xmd = query_colour_value(xmd);
   if (sizeof(xmd) > 0) {
      mixed str;

      xmd = xmd[..];	/* copy */

      if (typeof(xmd[0]) == T_STRING) {
	 str = strip_left(xmd[0]);
	 if (!strlen(str)) {
	    xmd = xmd[1 ..];
	 } else {
	    xmd[0] = str;
	 }
      }
      str = xmd[sizeof(xmd)-1];
      if (typeof(str) == T_STRING) {
	 str = strip_right(str);
	 if (!strlen(str)) {
	    xmd = xmd[ .. sizeof(xmd)-2];
	 } else {
	    xmd[sizeof(xmd)-1] = str;
	 }
      }
   }
   return NewXMLPCData(xmd);
}

static
mixed xmd_force_to_data(mixed xmd) {
   int i, j;

   if (!xmd) {
      /* nil is a valid element sequence */
      return NewXMLPCData(nil);
   }
   if (typeof(xmd) == T_STRING) {
      if (xmd == "" || xmd == " " || xmd == "\n") {
	 return NewXMLPCData(nil);
      }
      error("expecting data, got " + dump_value(xmd));
   }

   /* it had better be an array now */
   if (query_colour(xmd) == COL_ELEMENT) {
      /* an element is definitely data: return PCDATA */
      return NewXMLPCData(({ xmd }));
   }
   /* otherwise assume it's PCDATA already: check the guts */
   xmd = query_colour_value(xmd)[..];	/* copy */

   for (i = 0; i < sizeof(xmd); i ++) {
      mixed xmd_i;

      xmd_i = query_colour_value(xmd[i]);
      if (typeof(xmd_i) == T_STRING) {
	 for (j = 0; j < strlen(xmd_i); j ++) {
	    switch(xmd_i[j]) {
	    case ' ': case '\n':
	       /* OK, whitespace */
	       break;
	    default:
	       error("expecting data, got " + dump_value(xmd_i));
	    }
	 }
	 xmd[i] = nil;
      }
   }
   return NewXMLPCData(xmd - ({ nil }));
}


static
mapping attributes_to_mapping(mixed *attr) {
   mapping map;
   int i;

   map = ([ ]);
   for (i = 0; i < sizeof(attr); i +=2) {
      map[attr[i]] = attr[i+1];
   }
   return map;
}

static
mixed xmd_optimize(mixed xmd) {
   mixed xmd_value;

   while (typeof(xmd_value = query_colour_value(xmd)) == T_ARRAY &&
	  sizeof(xmd_value) == 1) {
      xmd = xmd_value[0];
   }
   return xmd;
}

