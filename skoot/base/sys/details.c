/*
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <base.h>

# include <dp_handler.h>

private inherit "/base/lib/urcalls";
private inherit "/base/lib/toolbox";
private inherit "/lib/string";
private inherit "/lib/array";

inherit "/core/lib/dp_handler";


string query_derived_root() {
   return "details";
}

static
void create() {
   BASE_PROPERTIES->register_root_handler("details", this_object());
   set_object_name("Base:DerivedDetails");
}


mapping get_configuration() {
   return ([
	      "abstract":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "is this object abstract, e.g. 'ambience'" }),
	      "adjectives":
	      ({ C_F_READ | C_F_LOCAL, T_STRING | C_T_ARRFLAG,
		    "adjectives this detail responds to" }),
	      "climbable":
	      ({ C_F_RW | C_F_LOCAL, T_OBJECT,
		    "where  we end up if we climb this detail [obsolete]" }),
	      "definite":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "should the system refrain from adding articles or possessives before the brief of this detail?" }),
	      "exit:closed":
	      ({ C_F_RW, T_INT,
		    "is this detail a closed door?" }),
	      "exit:deadbolt":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "can this door be locked and unlocked without a key?" }),
	      "exit:dest":
	      ({ C_F_RW | C_F_LOCAL, T_OBJECT,
		    "the other side of this exit detail" }),
	      "exit:directions":
	      ({ C_F_RW | C_F_LOCAL, T_STRING | C_T_ARRFLAG,
		    "the directions that leads through this exit detail" }),
	      "exit:door":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "is this exit detail a door?" }),
	      "exit:keycode":
	      ({ C_F_RW | C_F_LOCAL, T_STRING,
		    "the keycode for the lock on this door" }),
	      "exit:locked":
	      ({ C_F_RW, T_INT,
		    "is this door locked?" }),
	      "exit:self-locking":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "does this door lock automatically when closed?" }),
	      "face":
	      ({ C_F_RW | C_F_LOCAL, T_STRING,
		    "the specific facet that this detail represents, e.g. 'inside' or 'outside', but most often left empty" }),
	      "hidden":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "is this detail currently disabled?" }),
	      "hidden-when-closed":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "does this detail automatically disable when it's closed?" }),
	      "luminosity":
	      ({ C_F_RW | C_F_LOCAL, T_FLOAT,
		    "the luminous intensity this detail contributes" }),
	      "never-obvious":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "should this exit detail never be shown as an obvious exit?" }),
	      "obvious-when-open":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "should this exit detail be shown as obvious only when open?" }),
	      "plural":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "is this detail plural" }),
	      "prepositions":
	      ({ C_F_READ | C_F_LOCAL, T_STRING | C_T_ARRFLAG,
		    "the prepositional relationships an object can take to this detail as a proximity, e.g. PREP_BESIDE | PREP_UNDER" }),
	      "prime":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "does this detail represent the object as a whole (e.g. sword) rather than a subset of it (e.g. hilt)" }),
	      "separate":
	      ({ C_F_RW | C_F_LOCAL, T_INT,
		    "should this door open, close, lock and unlock separately from its counter-part on the other side?" }),
	      "snames":
	      ({ C_F_READ | C_F_LOCAL, T_STRING | C_T_ARRFLAG,
		    "the singular names this detail responds to" }),
	      "pnames":
	      ({ C_F_READ | C_F_LOCAL, T_STRING | C_T_ARRFLAG,
		    "the plural names this detail responds to" }),
	      ]);
}


private
mixed query_detail_property(object ob, string detail, string property) {
   string base, rest;

   if (sscanf(property, "description:%s", rest) ||
       sscanf(property, "descriptions:%s", rest)) {
      if (sscanf(rest, "%s:local", rest)) {
	 return ob->query_description(detail, rest);
      }
      return ur_description(ob, detail, rest);
   }
   if (sscanf(property, "preposition:%s", rest)) {
      int num;

      if (sscanf(rest, "%s:local", rest)) {
	 num = prep_atoi(rest);
	 if (num == -1) {
	    return FALSE;
	 }
	 return ob->test_id_prep(detail, num);
      }

      num = prep_atoi(rest);
      if (num == -1) {
	 return FALSE;
      }
      return ur_id_prep(ob, detail, num);
   }
   if (property != "adjectives:local" &&
       (sscanf(property, "adjective:%s", rest) ||
	sscanf(property, "adjectives:%s", rest))) {
      if (sscanf(rest, "%s:local", rest)) {
	 return ob->test_adjective(detail, rest);
      }
      return ur_test_adjective(ob, detail, rest);
   }
   if (property != "snames:local" &&
       (sscanf(property, "sname:%s", rest) ||
	sscanf(property, "snames:%s", rest))) {
      if (sscanf(rest, "%s:local", rest)) {
	 return ob->test_sname(detail, rest);
      }
      return ur_sname(ob, detail, rest);
   }
   if (property != "pnames:local" &&
       (sscanf(property, "pname:%s", rest) ||
	sscanf(property, "pnames:%s", rest))) {
      if (sscanf(rest, "%s:local", rest)) {
	 return ob->test_pname(detail, rest);
      }
      return ur_pname(ob, detail, rest);
   }
   if (property != "exit:directions:local" &&
       (sscanf(property, "exit:direction:%s", rest) ||
	sscanf(property, "exit:directions:%s", rest))) {
      if (sscanf(rest, "%s:local", rest)) {
	 return ob->test_exit_dir(detail, rest);
      }
      return ur_exit_dir(ob, detail, rest);
   }
   switch(property) {
   case "abstract":
      return ur_abstract(ob, detail);
   case "abstract:local":
      return ob->query_abstract(detail);
   case "adjectives":
      return ur_adjectives(ob, detail);
   case "adjectives:local":
      return ob->query_adjectives(detail);
   case "climbable":
      return ur_climbable_dest(ob, detail);
   case "climbable:local":
      return ob->query_climbable_dest(detail);
   case "definite":
      return ur_def(ob, detail);
   case "definite:local":
      return ob->query_def(detail);
   case "exit:closed":
   case "exit:closed:local":
      return ob->query_closed(detail);
   case "exit:deadbolt":
      return ur_deadbolt(ob, detail);
   case "exit:deadbolt:local":
      return ob->query_deadbolt(detail);
   case "exit:dest":
      return ur_exit_dest(ob, detail);
   case "exit:dest:local":
      return ob->query_exit_dest(detail);
   case "exit:directions":
      return ur_exit_dirs(ob, detail);
   case "exit:directions:local":
      return ob->query_exit_dirs(detail);
   case "exit:door":
      return ur_is_door(ob, detail);
   case "exit:door:local":
      return ob->query_is_door(detail);
   case "exit:keycode":
      return ur_exit_keycode(ob, detail);
   case "exit:keycode:local":
      return ob->query_exit_keycode(detail);
   case "exit:locked":
   case "exit:locked:local":
      return ob->query_locked(detail);
   case "exit:self-locking":
      return ur_self_locking(ob, detail);
   case "exit:self-locking:local":
      return ob->query_self_locking(detail);
   case "face":
      return ur_face(ob, detail);
   case "face:local":
      return ob->query_face(detail);
   case "hidden":
      return ur_hidden(ob, detail);
   case "hidden:local":
      return ob->query_hidden(detail);
   case "hidden-when-closed":
      return ur_hidden_when_closed(ob, detail);
   case "hidden-when-closed:local":
      return ob->query_hidden_when_closed(detail);
   case "luminosity":
      return ob->get_luminosity(detail);
   case "luminosity:local":
      return ob->query_luminosity(detail);
   case "never-obvious":
      return ur_never_obvious(ob, detail);
   case "never-obvious:local":
      return ob->query_never_obvious(detail);
   case "obvious-when-open":
      return ur_obvious_when_open(ob, detail);
   case "obvious-when-open:local":
      return ob->query_obvious_when_open(detail);
   case "plural":
      return ur_plural(ob, detail);
   case "plural:local":
      return ob->query_plural(detail);
   case "prepositions":
      return ur_id_preps(ob, detail);
   case "prepositions:local":
      return ob->query_id_preps(detail);
   case "prime":
      return ur_prime(ob, detail);
   case "prime:local":
      return ob->query_prime(detail);
   case "separate":
      return ur_separate(ob, detail);
    case "separate:local":
      return ob->query_separate(detail);
   case "snames":
      return ur_snames(ob, detail);
   case "snames:local":
      return ob->query_snames(detail);
   case "pnames":
      return ur_pnames(ob, detail);
   case "pnames:local":
      return ob->query_pnames(detail);
   }
}

private
mapping query_detail_properties(object ob, string detail) {
   mapping map, config;
   string *ix;
   int i;

   map = ([ ]);

   config = get_configuration();

   ix = map_indices(config);

   for (i = 0; i < sizeof(ix); i ++) {
      map[detail + ":" + ix[i]] = query_detail_property(ob, detail, ix[i]);
      if (config[ix[i]][CONFIG_FLAGS] & C_F_LOCAL) {
	 map[detail + ":" + ix[i] + ":local"] =
	    query_detail_property(ob, detail, ix[i] + ":local");
      }
   }

   ix = ob->query_description_types(detail);
   for (i = 0; i < sizeof(ix); i ++) {
      map[detail + ":description:" + ix[i]] =
	 query_detail_property(ob, detail,
			       "description:" + ix[i]);
      map[detail + ":description:" + ix[i] + ":local"] =
	 query_detail_property(ob, detail,
			       "description:" + ix[i] + ":local");
   }

   return map;
}

mapping r_query_properties(object ob, string root, int derived) {
   if (root == "details" && derived) {
      mapping map;
      string *details;
      int i;

      SysLog("root = " + dump_value(root));

      map = ([ ]);

      details = ob->query_details();

      for (i = 0; i < sizeof(details); i++) {
	 map += query_detail_properties(ob, details[i]);
      }
      return map;
   }
   return ([ ]);
}

mixed r_query_property(object ob, string root, string prop) {
   string detail;

   if (root == "details" && sscanf(prop, "%s:%s", detail, prop)) {
      return query_detail_property(ob, detail, prop);
   }
}


private
int set_detail_property(object ob, string detail, string property,
			mixed value) {
   string base, rest, *bits;
   int silent;

   bits = explode(property, ":");
   if (member("silently", bits) || member("silent", bits)) {
      silent = TRUE;
      property = implode(bits - ({ "silent", "silently" }), ":");
   }

   if (sscanf(property, "description:%s", rest) ||
       sscanf(property, "descriptions:%s", rest)) {
      ob->set_description(detail, rest, value);
      return TRUE;
   }
   if (sscanf(property, "preposition:%s", rest) ||
       sscanf(property, "prepositions:%s", rest)) {
      int num;

      num = prep_atoi(rest);
      if (num >= 0) {
	 ob->set_id_prep(detail, num, !!value);
      }
      return TRUE;
   }
   if (sscanf(property, "sname:%s", rest) ||
       sscanf(property, "snames:%s", rest)) {
      if (value) {
	 if (typeof(value) == T_INT) {
	    ob->add_alias(detail, rest);
	 } else {
	    ob->add_alias(detail, rest, value);
	 }
      } else {
	 ob->remove_alias(detail, rest);
      }
      return TRUE;
   }
   if (sscanf(property, "pname:%s", rest) ||
       sscanf(property, "pnames:%s", rest)) {
      if (value) {
	 if (typeof(value) == T_INT) {
	    ob->add_alias(detail, nil, rest);
	 } else {
	    ob->add_alias(detail, value, rest);
	 }
      } else {
	 ob->remove_alias(detail, nil, rest);
      }
      return TRUE;
   }
   if (sscanf(property, "adjective:%s", rest) ||
       sscanf(property, "adjectives:%s", rest)) {
      if (value) {
	 ob->add_adjective(detail, rest);
      } else {
	 ob->remove_adjective(detail, rest);
      }
      return TRUE;
   }
   if (sscanf(property, "exit:direction:%s", rest) ||
       sscanf(property, "exit:directions:%s", rest)) {
      if (value) {
	 ob->add_exit_dir(detail, rest);
      } else {
	 ob->remove_exit_dir(detail, rest);
      }
      return TRUE;
   }
   switch(property) {
   case "prime":
      ob->set_id_prime(detail, !!value);
      break;
   case "definite":
      ob->set_id_def(detail, !!value);
      break;
   case "abstract":
      ob->set_id_abstract(detail, !!value);
      break;
   case "climbable":
      ob->set_climbable_def(detail, value);
      break;
   case "exit:dest":
      ob->set_exit_dest(detail, value);
      break;
   case "exit:keycode":
      ob->set_exit_keycode(detail, value);
      break;
   case "exit:door":
      ob->set_is_door(detail, !!value);
      break;
   case "exit:closed/act":
      if (value) {
	 ob->do_close_door(detail, TRUE, FALSE);
      } else {
	 ob->do_open_door(detail, TRUE, FALSE);
      }
      break;
   case "exit:locked/act":
      if (value) {
	 ob->do_lock_door(detail, TRUE, FALSE);
      } else {
	 ob->do_unlock_door(detail, TRUE, FALSE);
      }
      break;
   case "exit:closed":
      if (value) {
	 ob->do_close_door(detail, silent, silent);
      } else {
	 ob->do_open_door(detail, silent, silent);
      }
      break;
   case "exit:locked":
      if (value) {
	 ob->do_lock_door(detail, silent, silent);
      } else {
	 ob->do_unlock_door(detail, silent, silent);
      }
      break;
   case "face":
      ob->set_face(detail, value);
      break;
   case "hidden-when-closed":
      ob->set_hidden_when_closed(detail, !!value);
      break;
   case "luminosity":
      ob->set_luminosity(detail, value);
      break;
   case "hidden":
      ob->set_id_hidden(detail, !!value);
      break;
   case "never-obvious":
      ob->set_never_obvious(detail, !!value);
      break;
   case "obvious-when-open":
      ob->set_obvious_when_open(detail, !!value);
      break;
   case "plural":
      ob->set_id_plural(detail, !!value);
      break;
   case "separate":
      ob->set_separate(detail, !!value);
      break;
   case "exit:self-locking":
      ob->set_self_locking(detail, !!value);
      break;
   case "exit:deadbolt":
      ob->set_deadbolt(detail, !!value);
      break;
   default:
      if (get_configuration()[property]) {
	 /* Attempt to set one of the readonly properties? */
	 DEBUG("Attempt to set read-only property details:" + detail + ":" + property + " in " + name(ob));
	 return TRUE;
      }
      return FALSE;
   }
   return TRUE;
}

int r_set_property(object ob, string root, string prop, mixed value,
		   mixed *retval) {
   string detail;

   if (root == "details") {
      if (sscanf(prop, "%s:%s", detail, prop) == 2) {
         sscanf(prop, "%s:local", prop);
         return set_detail_property(ob, detail, prop, value);
      } else {
         if (value == nil) {
            ob->remove_detail(prop);
            return TRUE;
         }
      }
   } 
   return FALSE;
}

void
r_clear_all_properties(object ob, string root)
{
    if (root == "base") {
	/* Ain't touching this one, no sir. */
    }
}

