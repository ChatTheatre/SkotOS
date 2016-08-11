/*
**	/usr/DevSys/lib/karthing.c
**
**	We brutally interrogate a physical thing to generate an
**	easily read description of detail descriptions.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <type.h>

private inherit "/lib/string";
inherit "/usr/XML/lib/xmlparse";
inherit "/usr/XML/lib/xmlgen";

private string gendet(object ob, string det);
private string *karsort(string *types);

static
string genkar(object ob) {
   string *in, *out, res;
   mixed val;
   int i;

   in = ob->query_property_indices(TRUE);
   out = allocate(sizeof(in));

   for (i = 0; i < sizeof(in); i ++) {
      val = ob->query_property(in[i]);
      if (typeof(val) != T_STRING) {
	 continue;
      }
      if (strlen(val) < 40) {
	 out[i] = "PROP [" + in[i] + "] " + val + "\n";
      } else {
	 out[i] = "PROP [" + in[i] + "]\n" + val + "\n";
      }
   }
   out -= ({ nil });
   if (sizeof(out)) {
      res = implode(out + ({ "" }), "----------------------------------------------------------------------\n");
   }
   return res;
}

/* parsing functionality */

# define PError(err) \
  if (strlen(str) < 20) error(err + " -- got " + str); \
  else error(err + " -- got " + str[.. 19] + "...");



atomic
void parskar(object ob, string str) {
   string detail, type, dstr, prop;
   mixed desc;

   str = strip_left(str);

   do {
      str = strip_left(str);
      if (sscanf(str, "PROP [%s]%s", prop, str)) {
	 prop = lower_case(prop);
	 str = strip_left(str);
	 if (!sscanf(str, "%s-----%s", desc, str)) {
	    desc = str; str = "";
	 }
	 desc = strip(desc);
	 ob->set_property(prop, desc);
      } else {
	 PError("expecting PROP");
      }
      str = strip_left(str);
      if (strlen(str)) {
	 if (str[0] == '-') {
	    /* dismiss any line starting with - */
	    sscanf(str, "%*s\n%s", str);
	 } else {
	    PError("unexpected data");
	 }
      }
   } while (strlen(str));
}

