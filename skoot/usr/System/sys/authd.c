/*
**	~System/sys/authd.c
**
**	We store passwords in cleartext here, and _never_ export them.
**	This does mean the statedump has to be kept very private.
**
**	Copyright Skotos Tech Inc 1999
*/

inherit "/usr/System/lib/sys_auto";

private inherit "/lib/string";
private inherit "/lib/url";

mapping *passwords_arr;
mapping passwords;

static
void create() {
    int i;

    passwords = ([ ]);
    passwords_arr = allocate(256);
    for (i = 0; i < 256; i++) {
	passwords_arr[i] = ([ ]);
    }
    set_object_name("System:Auth");
}

string query_state_root() { return "System:Auth"; }

string *query_groups(string name) {
   return ({ "SkotOS" });
}

string **query_users() {
    int    i;
    string **list;

    list = allocate(256);
    for (i = 0; i < 256; i++) {
	if (map_sizeof(passwords_arr[i]) > 0) {
	    list[i] = map_indices(passwords_arr[i]);
	}
    }
    return list - ({ nil });
}

int check_password(string name, string pass) {
   string stored;

   name = lower_case(name);

   if (stored = passwords_arr[name[0]][name]) {
      return stored == pass;
   }
   return !pass || !strlen(pass);
}

int test_hash(string name, string secret, string hash) {
   string stored;

   name = lower_case(name);

   if (stored = passwords_arr[name[0]][name]) {
      stored = url_encode(stored);
      return hash_md5(name + stored + secret) == hash;
   }
   /* TODO: insecure? */
   return hash_md5(name + secret) == hash;
}

string query_hash2(string name, string secret) {
   return to_hex(hash_md5(name + url_encode(passwords_arr[name[0]][name]) + secret));
}   

int test_hex_hash(string name, string secret, string hash) {
   string stored;

   name = lower_case(name);

   if (stored = passwords_arr[name[0]][name]) {
      stored = url_encode(stored);
      return to_hex(hash_md5(name + stored + secret)) == hash;
   }
   /* TODO: insecure? */
   return hash_md5(name + secret) == hash;
}

/* TODO: PROTECT THIS :-) */

void set_password(string name, string pass) {
    name = lower_case(name);
    passwords_arr[name[0]][name] = pass;
}

string query_password(string name) {
    name = lower_case(name);

    return passwords_arr[name[0]][name];
}
