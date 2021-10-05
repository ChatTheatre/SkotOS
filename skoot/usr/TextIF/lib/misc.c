/*
**	/usr/TextIF/lib/misc.c
**
**	Copyright Skotos Tech Inc 2000
*/

# include <TextIF.h>
# include <SkotOS.h>
# include <System.h>
# include <UserAPI.h>

private inherit "/usr/SkotOS/lib/auth";

private inherit "/base/lib/urcalls";
private inherit "/lib/string";
private inherit "/lib/array";

# include "/usr/SAM/include/sam.h"
# include "/usr/SkotOS/include/describe.h"

/*
 * Utility function, check if the StoryGuide is actually using his/her SG
 * body and if not, make a comment about this and return TRUE to indicate that
 * the action should be aborted.  Return FALSE otherwise.
 */

static int
check_storyguide_body(object user, object body)
{
    /*
     * First check if this is a SH or a SG:
     */
    if (!sizeof(rfilter(user->query_udat()->query_bodies(),
			"query_property", "GUIDE")) ||
	is_root(user->query_name())) {
	/* Must be a SH. */
	return FALSE;
    }
    /* This must be a SG then. */
    if (body->query_property("GUIDE")) {
	/* This is the actual GUIDE body.  Everything is fine then. */
	return FALSE;
    }
    user->message("You can only do this while in your official StoryGuide body.\n");
    return TRUE;
}

static
object **classify_bodies(mixed obs, varargs mapping fill) {
   mapping sort, fmap;
   object ob;
   mixed class, *arr;
   int i;

   if (typeof(obs) != T_ARRAY) {
      obs = ({ obs });
   }

   sort = ([ ]);
   fmap = ([ ]);
   for (i = 0; i < sizeof(obs); i ++) {
      class = nil;
      if (IsNRef(obs[i])) {
	 ob = NRefOb(obs[i]);
      } else if (typeof(obs[i]) == T_OBJECT) {
	 ob = obs[i];
      }
      class = ob->query_perception_class();
      if (class != nil) {
	 if (!sort[class]) {
	    sort[class] = ([ ]);
	 }
	 sort[class][ob] = TRUE;
	 if (fill) {
	    if (!fmap[class]) {
	       fmap[class] = ([ ]);
	    }
	    fmap[class][obs[i]] = TRUE;
	 }
      }
   }
   arr = map_indices(sort);
   for (i = 0; i < sizeof(arr); i ++) {
      if (fill) {
	 fill[i] = map_indices(fmap[arr[i]]);
      }
      arr[i] = map_indices(sort[arr[i]]);
   }
   return arr;
}

static
void transmit_to_client(object *users, object who, string msg, int id) {
   int i;

   if (who) {
      if (users) {
	 users &= who->query_souls();
      } else {
	 users  = who->query_souls();
      }
   }

   for (i = 0; i < sizeof(users); i ++) {
      if (users[i]->query_skotos_client()) {
	 if (id == MAPURL) {
	    users[i]->raw_line("MAPURL " + strip(msg) + "\n");
	 } else if (users[i]->query_skoot_client() ||
		    id == ARTURL || id == BIGMAPURL || id == ANYURL) {
	    users[i]->raw_line("SKOOT " + id + " " + strip(msg) + "\n");
	 }
      }
   }
}

static
void transmit_description_to_client(object *users, object who, NRef det,
				    string desc, int id,
				    varargs string def) {
   string text;
   mixed sam;

   if (sam = ur_description(NRefOb(det), NRefDetail(det), desc)) {
      text = unsam(sam);
   } else {
      text = def;
   }
   if (text) {
      transmit_to_client(users, who, text, id);
   }
}

/* build an array of mappings which is the who list */
/* each element of the array is an entry in the list */
static
mixed *build_who_list() {
   mapping list, ip, who_tmp, who_list, idle;
   object master, ob, *users;
   string name, *names;
   int i, j;

   list = ([ ]);
   ip = ([ ]);
   idle = ([ ]);
   who_list = ([ ]);
   who_tmp = ([ ]);

   users = INFOD->query_current_user_objects();

   /* get the IP information of each user */
   for(i = 0; i < sizeof(users); i++) {
      if (users[i]->query_conn() && (name = users[i]->query_name())) {
	 object body;

	 if (!list[name]) {
	    list[name] = ({ });
	 }
	 if (!idle[name]) {
	    idle[name] = ({ });
	 }
	 if (!ip[name]) {
	    ip[name] = query_ip_name(users[i]->query_conn());
	 }
	 if (body = users[i]->query_body()) {
	    list[name] += ({ body });
	    idle[name] += ({
	       users[i]->query_idled_out() ?
		  time() - users[i]->query_idle_timestamp() : 0
		  });
	 }
      }
   }

   names = map_indices(list);

   /* slam the information into who_tmp */
   /* slam who_tmp into who_list */
   for (i = 0; i < sizeof(names); i++) {
      who_tmp = ([ ]);

      who_tmp["user"] = names[i];
      who_tmp["ip"] = ip[names[i]];
      who_tmp["body"] = list[names[i]];
      who_tmp["idle"] = idle[names[i]];

      if (is_root(names[i])) {
	 who_tmp["staff"] = TRUE;
      } else {
	 who_tmp["staff"] = FALSE;
      }

      who_list[i] = who_tmp;
   }

   return map_values(who_list);
}

string info_username(object user) {
   return user->query_name();
}

string info_ip(object user) {
   return query_ip_name(user->query_conn());
}

object info_body(object user) {
   return user->query_body();
}

string info_location(object body) {
   return body->query_environment() ? name(body->query_environment()) : nil;
}

int info_staff(object user) {
   if (is_root(user->query_name())) {
      return TRUE;
   } 

   return FALSE;
}

int info_usercount() {
   return sizeof(INFOD->query_current_user_objects());
}

int info_staffcount() {
   object *users;
   int i, count;

   users = INFOD->query_current_user_objects();

   for (i = 0; i < sizeof(users); i++) {
     if (is_root(users[i]->query_name())) {
	count++;
     }
   }

   return count;
}

int info_customercount() {
   object *users;
   int i, count;

   users = INFOD->query_current_user_objects();

   for (i = 0; i < sizeof(users); i++) {
     if (!is_root(users[i]->query_name())) {
	count++;
     }
   }

   return count;
}

static string
wrap_string(string prefix, string text, int width)
{
    int    i, sz, total, current_len;
    string *lines, current, *words, spaces;

    /*
     * Allocate enough lines for a a worst-case scenario, one word per line.
     * Each word would then be at least floor(width / 2) + 1 characters.
     */

    width -= strlen(prefix);
    lines = allocate((strlen(text) + (width / 2)) / (width / 2));

    spaces = "                                        "[0..strlen(prefix) - 1];

    words = explode(text, " ");
    sz = sizeof(words);

    current     = words[0];
    current_len = strlen(current);
    total       = 0;

    for (i = 1; i < sz; i++) {
	if (current_len + 1 + strlen(words[i]) < width) {
	    current += " " + words[i];
	    current_len += 1 + strlen(words[i]);
	} else {
	    lines[total++] = current;
	    current        = words[i];
	    current_len    = strlen(current);
	}
    }
    lines[total++] = current;
    return prefix + implode(lines[..total - 1], "\n" + spaces) + "\n";
}

static string
group_people(object user, object body, string str, string mode)
{
  int     i, sz, *sums;
  string  result;
  object  *list, *bodies;
  mapping env_map, body_map, totals, *room_maps;

  env_map  = ([ ]);
  body_map = ([ ]);
  totals   = ([ ]);
  list     = INFOD->query_current_user_objects();
  sz       = sizeof(list);

  for (i = 0; i < sz; i++) {
    int total;
    object user_body, env;
    mapping map;

    if (is_root(list[i]->query_name())) {
      if (mode == "customers") {
	continue;
      }
    } else if (mode == "staff") {
        continue;
    }
    user_body = list[i]->query_body();
    if (!user_body) {
      continue;
    }
    if (body_map[user_body]) {
      continue;
    }
    body_map[user_body] = TRUE;
    env = user_body->query_environment();
    map = env_map[env];
    if (map) {
      total = map_sizeof(map);
      totals[total][env] = nil;
      if (!map_sizeof(totals[total])) {
	totals[total] = nil;
      }
    } else {
      map = env_map[env] = ([ ]);
      total = 0;
    }
    map[describe_one(user_body, nil, nil, STYLE_IGNORE_VISIBILITY)] = TRUE;
    total = map_sizeof(map);
    if (totals[total]) {
      totals[total][env] = TRUE;
    } else {
      totals[total] = ([ env: TRUE ]);
    }
  }

  result    = "";
  sz        = map_sizeof(totals);
  sums      = map_indices(totals);
  room_maps = map_values(totals);

  for (i = sz - 1; i >= 0; i--) {
    int    j, sz_k;
    object *rooms;

    rooms = map_indices(room_maps[i]);
    for (j = 0, sz_k = sizeof(rooms); j < sz_k; j++) {
      string *names, str;

      names = map_indices(env_map[rooms[j]]);
      if (sums[i] > 1) {
	str = implode(names[..sums[i] - 2], ", ") +
	  " and " + names[sums[i] - 1];
      } else {
	str = names[0];
      }
      result += ( "<B>" +
		  ( sums[i] > 1 ?
		    sums[i] + " characters " :
		    "1 character " ) +
		  ( rooms[j] ? "in " + describe_one(rooms[j], nil, nil, STYLE_IGNORE_VISIBILITY) :
		    "without environment" ) +
		  ":</B> " + capitalize(str) + ".\n");
    }
  }
  return result;
}

static object
determine_user(string str, varargs int find_usernames)
{
    int    i, sz;
    object *users, body;

    users = INFOD->query_current_user_objects();
    sz = sizeof(users);
    str = lower_case(str);

    body = UDATD->query_body(str);

    if (body) {
       object *list;

       body = body->get_ultimate_possessee();	/* never nil */

       list = users & body->query_souls();
       if (sizeof(list) > 0) {
	  return list[0];
       }
    }

    /*
     * First check all bodies.
     */
    for (i = 0; i < sz; i++) {
	object body;

	body = users[i]->query_body();
	if (body) {
	    if (lower_case(describe_one(body)) == str) {
		return users[i];
	    }
	}
    }

    /*
     * Then, if the one doing this is a StoryHost/StoryPlotter/etc, check if
     * it was perhaps a username.
     */
    if (find_usernames) {
	for (i = 0; i < sz; i++) {
	    if (users[i]->query_name() == str) {
		return users[i];
	    }
	}
    }
    return nil;
}
