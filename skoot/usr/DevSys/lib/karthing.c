/*
**	/usr/DevSys/lib/karthing.c
**
**	We brutally interrogate a physical thing to generate an
**	easily read description of detail descriptions.
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/lib/string";
inherit "/usr/XML/lib/xmlparse";
inherit "/usr/XML/lib/xmlgen";

private string gendet(object ob, string det);
private string *karsort(string *types);

static
string genkar(object ob) {
   string *in, *out, res, val;
   int i;

   in = ob->query_sorted_details();
   out = allocate(sizeof(in));
   for (i = 0; i < sizeof(in); i ++) {
      out[i] = gendet(ob, in[i]);
   }
   res = implode(out, "----------------------------------------------------------------------\n");   
   return res;
}

private
string gendet(object ob, string det) {
   string *types, *typout, out;
   mixed desc;
   int i, j;

   types = ob->query_description_types(det);
   types = karsort(types);
   typout = allocate(256);
   for (i = 0; i < sizeof(types);i ++) {
      desc = ob->query_description(det, types[i]);
      out = generate_pcdata(desc);
      if (strlen(out) < 60) {
	 typout[j ++] = capitalize(types[i]) + ": " + out;
      } else {
	 out = implode(explode(out, "\n"), " ");
	 typout[j ++] = capitalize(types[i]) + ": [*\n  " +
	    brick_paragraph(out, 2, 70) + "*]";
      }
   }

   types = ob->query_snames(det) - ({ nil });
   for (i = 0; i < sizeof(types); i ++) {
      typout[j ++] = "SName: " + types[i];
   }

   types = ob->query_pnames(det) - ({ nil });
   for (i = 0; i < sizeof(types); i ++) {
      typout[j ++] = "PName: " + types[i];
   }

   types = ob->query_adjectives(det) - ({ nil });
   for (i = 0; i < sizeof(types); i ++) {
      typout[j ++] = "Adjective: " + types[i];
   }

   typout = typout[.. j-1];
   return "DETAIL [" + det + "]\n" + implode(typout, "\n") + "\n";
}

/* Karen's preferred order: ({ glance, brief, look, examine }) */

private
string *karsort(string *types) {
   return
      (types & ({ "brief" })) +
      (types & ({ "glance" })) +
      (types & ({ "look" })) +
      (types & ({ "examine" })) +
      (types - ({ "glance", "brief", "look", "examine" }));
}



/* parsing functionality */

# define PError(err) \
  if (strlen(str) < 40) error(err + " -- got " + str); \
  else error(err + " -- got " + str[.. 39] + "...");



atomic
void parskar(object ob, string str) {
   string detail, type, dstr, prop;
   mixed desc;
   int snames, pnames, adjectives;

   str = strip_left(str);

   do {
      string val;

      str = strip_left(str);
      if (sscanf(str, "DETAIL [%s]%s", detail, str)) {
	 detail = lower_case(detail);
	 if (sscanf(detail, "%*s\n")) {
	    error("expecting detail ID -- got " + dump_value(detail));
	 }
	 snames = pnames = adjectives = FALSE;	/* seen any yet? */
	 str = strip_left(str);
	 while (strlen(str) && str[0] != '-') {
	    if (!sscanf(str, "%s: %s", type, str)) {
	       PError("expecting description type: in {" + detail + "}");
	    }
	    type = lower_case(strip_right(type));
	    if (sscanf(type, "%*s\n")) {
	       error("expecting description type in {" + detail + "} -- got " + dump_value(type));
	    }
	    /* catch common case of forgotten trailing colon */
	    if (sscanf(type, "%*s[")) {
	       PError("expecting description type in {" + detail + "}");
	    }
	    str = strip_left(str);
	    if (str[0] == '[') {
	       if (str[1] == '*') {
		  if (!sscanf(str, "[*%s*]%s", dstr, str)) {
		     PError("expecting [* desc *] in {" + detail + "/" + type + "}");
		  }
	       } else if (!sscanf(str, "[%s]%s", dstr, str)) {
		  PError("expecting [ desc ] in {" + detail + "/" + type + "}");		  
	       }
	    } else if (!sscanf(str, "%s\n%s", dstr, str)) {
	       dstr = str; str = "";
	    }
	    if (type == "name" || type == "sname") {
	       string sname, pname;

	       if (!snames) {
		  snames = TRUE;
		  ob->clear_snames(detail);
	       }
	       if (sscanf(dstr, "%s/%s", sname, pname)) {
		  ob->add_sname(detail, strip(sname));
		  ob->add_pname(detail, strip(pname));
	       } else {
		  ob->add_sname(detail, strip(dstr));
	       }
	    } else if (type == "pname") {
	       string sname, pname;

	       if (!pnames) {
		  pnames = TRUE;
		  ob->clear_pnames(detail);
	       }
	       ob->add_pname(detail, strip(dstr));
	    } else if (type == "adjective") {
	       if (!adjectives) {
		  adjectives = TRUE;
		  ob->clear_adjectives(detail);
	       }
	       ob->add_adjective(detail, strip(dstr));
	    } else {
	       desc = parse_xml(strip(dstr));
	       ob->set_description(detail, type, desc);
	    }
	    str = strip_left(str);
	 }
      } else {
	 PError("expecting DETAIL or INITPROP");
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

