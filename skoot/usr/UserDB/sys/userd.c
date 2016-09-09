/*
**	~UserDB/sys/userd.c
**
**	Keep track of user data.
**
**	Copyright Skotos Tech 2001
*/

private inherit "/lib/string";
private inherit "/lib/mapping";
# include <SAM.h>
# include <SkotOS.h>
# include <mapargs.h>

mapping users;
mapping emails;
mapping purged;
mapping banned;
mapping staff;

int     motd_timestamp,
        motd_expire;
string  motd_message;

static
void create() {
   users  = ([ ]);
   purged = ([ ]);
   banned = ([ ]);
   staff  = ([ ]);

   compile_object("/usr/UserDB/obj/user");

   SAMD->register_root("UserDB");

   set_object_name("UserDB:Daemon");
}

object *query_users_by_email(string email);

/* user/udat stuff */

atomic
object make_user(string uname, string password, string email) {
   mapping map;
   object user;
   int i;

   uname = lower_case(uname);

   for (i = 0; i < strlen(uname); i ++) {
      switch(uname[i]) {
      case 'a' .. 'z': case '_': case '0' .. '9':
	 break;
      default:
	 error("bad character #" + (i+1) + " in user name");
      }
   }
   if (email && query_users_by_email(email)) {
      error("there is already a user with that email");
   }
   if (strlen(uname) < 2) {
      error("username too short");
   }
   user = clone_object("/usr/UserDB/obj/user");
   user->set_name(uname);
   user->set_password(password);
   if (email) {
      user->set_email(email);
   }
   return user;
}



object query_user(string uname) {
   mapping map;

   if (strlen(uname) < 2) {
      return nil;
   }
   uname = lower_case(uname);

   if (map = users[uname[.. 1]]) {
      return map[uname[2 ..]];
   }
   return nil;
}

mapping query_users() {
   return users;
}

atomic
void set_name(string uname, object user) {
   mapping map;

   uname = lower_case(uname);

   map = users[uname[.. 1]];
   if (!map) {
      map = users[uname[.. 1]] = ([ ]);
   }
   if (map[uname[2 ..]] && map[uname[2 ..]] != user) {
      error("user exists");
   }
   map[uname[2 ..]] = user;
}

void clear_email(string email, object user) {
    mapping map, nap;

    email = lower_case(email);

    if (strlen(email) < 2) {
	/* This should not be possible consider it was added somehow. */
	error("email too short");
    }
    map = emails[email[.. 1]];
    if (map) {
	nap = map[email[2 ..]];
	if (nap) {
	    nap[user] = nil;
	    if (map_sizeof(nap) == 0) {
		map[email[2 ..]] = nil;
		if (map_sizeof(map) == 0) {
		    emails[email[.. 1]] = nil;
		}
	    }
	} else {
	    error("Email missing from sub-mapping");
	}
    } else {
	error("Email missing from mapping");
    }
}

void set_email(string email, object user) {
   mapping map, nap;

   /* for indexing purposes we work with lowercased address */

   email = lower_case(email);

   if (strlen(email) < 2) {
      error("email too short");
   }
   map = emails[email[.. 1]];
   if (!map) {
      map = emails[email[.. 1]] = ([ ]);
   }
   if (nap = map[email[2 ..]]) {
      nap[user] = TRUE;
   } else {
      map[email[2 ..]] = ([ user: TRUE ]);
   }
}

object *query_users_by_email(string email) {
   mapping map;

   email = lower_case(email);

   if (strlen(email) < 2) {
      return nil;
   }
   if (map = emails[email[.. 1]]) {
      if (map[email[2 ..]]) {
	 if (map_sizeof(map[email[2 ..]])) {
	    return map_indices(map[email[2 ..]]);
	 }
	 map[email[2 ..]] = nil;
      }
   }
   return nil;
}

mapping query_users_by_email_prefix(string email) {
    mapping map;

    email = lower_case(email);
    if (strlen(email) < 2) {
        return nil;
    }
    if (map = emails[email[.. 1]]) {
        mapping sub;

	sub = prefixed_map(map, email[2 ..]);
        if (sub && map_sizeof(sub)) {
	    int i, sz;
	    string *str;
	    mapping *obs;
	    mapping result;

	    DEBUG("sub = " + dump_value(sub));
	    result = ([ ]);
	    sz  = map_sizeof(sub);
	    str = map_indices(sub);
	    obs = map_values(sub);
	    for (i = 0; i < sz; i++) {
	        result[email[.. 1] + str[i]] = map_indices(obs[i]);
	    }
	    return result;
	}
    }
    return nil;
}

int query_purged_user(string uname) {
    mapping map;

    if (strlen(uname) < 2) {
	return FALSE;
    }
    uname = lower_case(uname);
    if (map = purged[uname[.. 1]]) {
	return !!map[uname[2 ..]];
    }
    return FALSE;
}

void add_purged_user(string uname) {
    mapping map;

    uname = lower_case(uname);

    if (query_user(uname)) {
	error("User still exists!");
    }
    if (strlen(uname) < 2) {
	error("username too short");
    }
    map = purged[uname[.. 1]];
    if (!map) {
	map = purged[uname[.. 1]] = ([ ]);
    }
    if (map[uname[2 ..]]) {
	error("user already on purged list");
    }
    map[uname[2 ..]] = TRUE;
}

void del_purged_user(string uname) {
    uname = lower_case(uname);

    purged[uname[.. 1]][uname[2 ..]] = nil;
}

atomic void
purge_user(string uname)
{
    object user;

    uname = lower_case(uname);

    if (query_purged_user(uname)) {
        DEBUG(uname + ": Purging already purged user?");
	return;
    }
    user = query_user(uname);
    if (user) {
	string email;

	email = user->query_email();
	if (email) {
	    DEBUG(uname + ": Purging existing user (email: " + email + ", " +
		  "due: " + ctime(user->query_next_stamp()) + ")");
	    clear_email(email, user);
	} else {
	    DEBUG(uname + ": Purging existing user (email: unknown, " +
		  "due: " + ctime(user->query_next_stamp()) + ")");
	}
	user->destruct();
    } else {
	DEBUG(uname + ": Purging non-existing user");
    }
    add_purged_user(uname);
}

/*
 * Rename a user, marking the old name as purged.
 * Email data does not need to be moved around, it's object-based.
 */
atomic void
rename_user(string name_old, string name_new, varargs int override)
{
    object user;
    mapping map;

    name_old = lower_case(name_old);
    name_new = lower_case(name_new);

    if (!name_old) {
	error("Bad argument 1 to function rename_user");
    }
    if (!name_new) {
	error("Bad argument 2 to function rename_user");
    }
    if (name_old == name_new) {
	error(name_old + ": Identical names!");
    }
    user = query_user(name_old);
    if (!user) {
	error(name_old + ": Does not exist");
    }
    if (query_user(name_new)) {
	error(name_new + ": Already exists");
    }
    if (query_purged_user(name_new)) {
	if (override) {
	    /* Unpurge and reuse the name, then. */
	    del_purged_user(name_new);
	} else {
	    error(name_new + ": Cannot rename to purged name");
	}
    }

    /* Remove old name from 2-level mapping structure in 'users' */
    map = users[name_old[ .. 1]];
    map[name_old[2 ..]] = nil;
    if (!map_sizeof(map)) {
	users[name_old[.. 1]] = nil;
    }

    /* Add new name in its place */
    set_name(name_new, user);

    DEBUG(name_old + ": Renamed to \"" + name_new + "\"");

    /* Purge old name */
    purge_user(name_old);
}

atomic int
add_user_ban(string name, string who, string reason)
{
    object  ob;
    mapping map;

    name = lower_case(name);
    ob = query_user(name);
    if (!ob) {
	return FALSE;
    }
    map = banned[name[.. 1]];
    if (!map) {
	banned[name[.. 1]] = map = ([ ]);
    }
    map[ob] = ({ time(), who, reason });
    ob->set_user_ban();
    return TRUE;
}

atomic int
remove_user_ban(string name)
{
    object  ob;
    mapping map;

    name = lower_case(name);
    ob = query_user(name);
    if (!ob) {
	return FALSE;
    }
    map = banned[name[.. 1]];
    if (!map) {
	return FALSE;
    }
    map[ob] = nil;
    if (map_sizeof(map) == 0) {
	banned[name[.. 1]] = nil;
    }
    ob->clear_user_ban();
    return TRUE;
}

string **
query_users_banned()
{
    int     i, sz_i;
    string  *prefixes;
    mixed   *names;
    mapping *data;

    sz_i     = map_sizeof(banned);
    prefixes = map_indices(banned);
    data     = map_values(banned);
    names    = allocate(sz_i);
    for (i = 0; i < sz_i; i++) {
	int j, sz_j;
	object *obs;

	names[i] = ([ ]);
	obs = map_indices(data[i]);
	sz_j = sizeof(obs);
	for (j = 0; j < sz_j; j++) {
	    names[i][obs[j]->query_name()] = TRUE;
	}
	if (map_sizeof(names[i])) {
	    names[i] = map_indices(names[i]);
	} else {
	    names[i] = nil;
	}
    }
    return names - ({ nil });
}

mixed *
query_user_banned(string name)
{
    object  ob;
    mapping map;

    name = lower_case(name);
    ob = query_user(name);
    if (!ob) {
	return nil;
    }
    map = banned[name[.. 1]];
    if (!map) {
	return nil;
    }
    return map[ob] ? map[ob][..] : nil;
}

void
add_staff_user(string name)
{
    object user;

    user = query_user(name);
    if (!user) {
        error("No such user");
    }
    if (!staff) {
        staff = ([ ]);
    }
    staff[name] = user;
}

void
clear_staff_user(string name)
{
    object user;

    user = query_user(name);
    if (!user) {
        error("No such user");
    }
    if (!staff || !staff[name]) {
        error("Not a staff user");
    }
    staff[name] = nil;
}

int
query_staff_user(string name)
{
    return staff && staff[name];
}

string *
query_staff_users()
{
    return staff ? map_indices(staff) : ({ });
}

void set_motd_message(string str) {
    motd_message = str;
}

void set_motd_timestamp(int i) {
    motd_timestamp = i;
}

void set_motd_expire(int i) {
    motd_expire = i;
}

string query_motd_message() {
    return motd_message;
}

int query_motd_timestamp() {
    return motd_timestamp;
}

int query_motd_expire() {
    return motd_expire;
}

#if 0
void patch() {
   users = ([ ]);
}
#endif

#if 0
void patch_purged() {
    purged = ([ ]);
}
#endif

#if 0
void patch() {
    banned = ([ ]);
}
#endif
