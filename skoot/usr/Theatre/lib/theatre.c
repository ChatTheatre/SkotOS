/*
**      /usr/Theatre/lib/theatre.c
**
**      Copyright Skotos Tech Inc 2000
*/

# include <HTTP.h>
# include <Theatre.h>

private inherit "/lib/string";

inherit "/usr/SkotOS/lib/startstory";

private string id;
private string name, brief, description;


private object start_room, home, office, jail;

private object *male_bodies, *female_bodies;

private object web_page;

private string realm_url, login_url, bigmap_url;

private int    restricted;
private string *guests;

private string announcement;

private int    ranking;

private string hostname;
private string *hostnames;

string query_state_root() { return "Theatre:Theatre"; }

string query_http_root_url_document() {
   if (web_page) {
      return "/SAM/Prop/" + name(web_page);
   }
   return "/SAM/Prop/Theatre:Web:Theatre";
}   

string query_http_login_document() {
    return login_url;
}

static
void create() {
   ::create();
}

void patch() {
   ::create();
}

void set_realm_url(string p) {
   realm_url = p;
}

string query_realm_url() { return realm_url; }

void set_bigmap_url(string p) {
   bigmap_url = p;
}

string query_bigmap_url() { return bigmap_url; }

void set_name(string s, string i) {
   object ob;

   name = s;
   id = i;

   if (!id) {
      id = "theatre";
   }
   do {
      ob = find_object("Theatre:Theatres:" + id);
      if (ob == this_object()) {
	 ob = nil;
      }
      if (ob) {
	 id += "_new";
      }
   } while (ob);

   set_object_name("Theatre:Theatres:" + id);
}

string query_name() {
   return name;
}

string query_id() {
   return id;
}

void set_brief(string b) {
   brief = b;
}

string query_brief() {
   return brief ? brief : name;
}

void set_description(string d) {
   description = d;
}

string query_description() {
   return description ? description : brief ? brief : name;
}

void set_web_page(object ob) {
   web_page = ob;
}

object query_web_page() {
   return web_page;
}

void set_start_room(object ob) {
   start_room = ob;
}

object query_start_room() {
   return start_room;
}

void add_male_body() {
   male_bodies |= ({ find_object("/base/obj/thing") });
}

void set_male_body(object ob) {
   male_bodies |= ({ ob });
}

void clear_male_bodies() {
   male_bodies = ({ });
}

object *query_male_bodies() {
   return male_bodies;
}

void add_female_body() {
   female_bodies |= ({ find_object("/base/obj/thing") });
}

void set_female_body(object ob) {
   female_bodies |= ({ ob });
}

void clear_female_bodies() {
   female_bodies = ({ });
}

object *query_female_bodies() {
   return female_bodies;
}

void set_home(object ob){
    home = ob;
}

object query_home() {
    return home ? home : query_start_room();
}

void set_office(object ob) {
    office = ob;
}

object query_office() {
    return office ? office : query_home();
}

void set_jail(object ob) {
    jail = ob;
}

object query_jail() {
    return jail ? jail : query_office();
}

void set_login_url(string url) {
    login_url = url;
}

string query_login_url() {
    return login_url;
}

void set_restricted(int flag) {
    restricted = !!flag;
}

int query_restricted() {
    return restricted;
}

string *query_guests() {
    return guests ? guests : ({ });
}

void clear_guests() {
    guests = ({ });
}

void add_guest(varargs string name) {
    if (!name) {
        name = "NEW";
    }
    if (!guests) {
        guests = ({ });
    }
    guests |= ({ name });
}

void del_guest(string name) {
    if (!guests) {
        guests = ({ });
    } else {
        guests -= ({ name });
    }
}

void set_announcement(string text) {
    if (text && strlen(text)) {
        announcement = text;
    } else {
        announcement = nil;
    }
}

string query_announcement() {
    return announcement;
}

void set_ranking(int i) {
    ranking = i;
}

int query_ranking() {
    return ranking;
}

void set_hostname(string str) {
    if (str) {
       str = strip(str);
       if (!strlen(str)) {
	  str = nil;
       }
    }
    if (hostname != str) {
       if (hostname) {
	  HTTPD->unregister_hostname(this_object(), hostname);
       }
       hostname = str;
       if (hostname) {
	  HTTPD->register_hostname(this_object(), hostname);
       }
    }
}

string query_hostname() {
   return hostname;
}

void set_hostnames(string str) {
    int    i, sz;
    string *new_str, *diff;

    str = implode(explode(str, ","), " ");
    new_str = explode(str, " ") - ({ nil, "" });

    if (!hostnames) {
	hostnames = hostname ? ({ hostname }) : ({ });
    }
    diff = hostnames - new_str;
    if (sz = sizeof(diff)) {
	/* Remove hostnames no longer in use. */
	for (i = 0; i < sz; i++) {
	    HTTPD->unregister_hostname(this_object(), diff[i]);
	}
    }
    diff = new_str - hostnames;
    if (sz = sizeof(diff)) {
	/* Add new hostnames */
	for (i = 0; i < sz; i++) {
	    HTTPD->register_hostname(this_object(), diff[i]);
	}
    }
    hostnames = new_str;
}

string query_hostnames()
{
    return hostnames ? implode(hostnames, " ") : (hostname ? hostname : "");
}
