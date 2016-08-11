# include <SkotOS.h>


private inherit "/lib/string";

private void handle_chunk(string chunk, int k);
private string figure_inflection(string verb);

/* static TODO */
void load_verbs(string file) {
   string text, *chunks;
   int k;

   if (text = read_file(file)) {
      chunks = explode(text, "---\n");
      for (k = 0; k < sizeof(chunks); k ++) {
	 Debug("Handling chunk " + (k+1) + " of " + sizeof(chunks));
	 catch {
	    handle_chunk(chunks[k], k);
	 }
      }
      return;
   }
   error("could not find file " + file);
}

private void handle_chunk(string chunk, int k) {
   string oname, line, verb, third;
   string *forms, *verbs, *preps;
   object obj, urverb;
   int i;

   if (!sscanf(chunk, "%s\n%s", line, chunk)) {
      error("could not extract forms line from chunk #" + k);
   }
   forms = explode(line, " ") - ({ "" });
   if (!sscanf(chunk, "%s\n%s", line, chunk)) {
      error("could not extract preps line from chunk #" + k);
   }
   preps = explode(line, " ") - ({ "" });
   if (!sscanf(chunk, "%s\n%s", oname, chunk)) {
      error("could not extract oname line from chunk #" + k);
   }

   urverb = clone_object(VERB);

   for (i = 0; i < sizeof(preps); i ++) {
      urverb->set_preposition(preps[i]);
   }
   for (i = 0; i < sizeof(forms); i ++) {
      urverb->set_form(forms[i], nil);
   }
   urverb->set_power(2);

   if (sizeof(preps)) {
      urverb->set_object_name(oname + "Ur-" + implode(preps, "-"));
   } else {
      urverb->set_object_name(oname + "Ur");
   }

   verbs = explode(chunk, "\n");

   for (i = 0; i < sizeof(verbs); i ++) {
      verb = strip(verbs[i]);
      obj = SOCIALS->query_verb_ob(verb);
      if (obj) {
	 obj->clear_verb();
	 Debug("Verb cleared: " + verb);
      } else {
	 obj = clone_object(VERB);
	 Debug("Verb created: " + verb);
      }
      obj->set_imperative(verb);
      obj->set_second(verb);

      third = figure_inflection(verb);

      obj->set_third(third);
      obj->set_object_name(oname + verb);
      obj->set_ur_object(urverb);
   }
}

private string figure_inflection(string verb) {
   int len;

   len = strlen(verb);

   if (len > 2) {
      switch(verb[len-2 ..]) {
      case "sh": case "ch":
	 return verb + "es";
      case "ay": case "ey": case "iy": case "oy": case "uy":
	 return verb + "s";
      }
   }
   if (len > 1) {
      switch(verb[len-1 ..]) {
      case "y":
	 return verb[.. len-2] + "ies";
      case "s": case "z": case "x":
	 return verb + "es";
      }
   }
   return verb + "s";
}
