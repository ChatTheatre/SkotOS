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

private string gentop(object ob, string det);

static
string genkar(object ob) {
   string *topics, *out;
   int i;

   topics = ob->query_topics();
   out = allocate(sizeof(topics));
   for (i = 0; i < sizeof(topics); i ++) {
      out[i] = gentop(ob, topics[i]);
   }
   return implode(out, "----------------------------------------------------------------------\n");
}

private
string gentop(object ob, string top) {
   string *descs, *out, desc;
   int i;

   descs = ob->query_desc(top);
   out = allocate(sizeof(descs));

   for (i = 0; i < sizeof(descs); i ++) {
      desc = generate_pcdata(descs[i]);
      if (strlen(desc) < 60) {
	 out[i] =  "Tip: " + desc;
      } else {
	 desc = implode(explode(desc, "\n"), " ");
	 out[i] = "Tip: [[[\n  " + brick_paragraph(desc, 2, 70) + "]]]";
      }
   }
   return "TOPIC [" + top + "]\n" + implode(out, "\n") + "\n";
}


/* parsing functionality */

atomic
void parskar(object ob, string str) {
   string topic, type, dstr;
   mixed desc;

   do {
      str = strip_left(str);
      if (!sscanf(str, "TOPIC [%s]%s", topic, str)) {
	 error("expecting TOPIC, not: " + dump_value(str[.. 20]));
      }
      topic = lower_case(topic);
      str = strip_left(str);
      ob->clear_topic(topic);
      while (strlen(str) && str[0] != '-') {
	 if (!sscanf(str, "%s:%s", type, str)) {
	    error("expecting Tip: not " + dump_value(str[.. 20]));
	 }
	 if (lower_case(type) != "tip") {
	    error("expecting Tip: not " + dump_value(type) + ":");
	 }
	 str = strip_left(str);
	 if (str[0] == '[') {
	    if (!sscanf(str, "[[[%s]]]%s", dstr, str)) {
	       error("expecting [[[ desc ]]] in {" + topic + "/" + type + "}");
	    }
	 } else if (!sscanf(str, "%s\n%s", dstr, str)) {
	    dstr = str; str = "";
	 }
	 desc = parse_xml(strip(dstr));
	 ob->add_tip(topic, desc);
	 str = strip_left(str);
      }
      if (strlen(str)) {
	 /* dismiss any line starting with - */
	 sscanf(str, "%*s\n%s", str);
      }
   } while (strlen(str));
}

