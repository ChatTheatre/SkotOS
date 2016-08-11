/*
 *	/usr/Game/lib/data.c
 *
 *	Copyright Skotos Tech Inc 2001
 *
 *	This is a generic object that can be used as the data-container for
 *	both Theatres and GrandTheatres like Castle Marrach.
 */

private int    web_port;
private string name, brief, description, announcement,
               realm_url, bigmap_url, root_url, login_url;
private object start_room, home, office, jail;

static void
create()
{
   /*
    * Nothing here yet, might be used to initialise arrays/mappings later.
    */
}

string query_name()               { return name;         }
string query_brief()              { return brief;        }
string query_description()        { return description;  }
string query_announcement()       { return announcement; }
object query_start_room()         { return start_room;   }
object query_home()               { return home;         }
object query_office()             { return office;       }
object query_jail()               { return jail;         }
int    query_web_port()           { return web_port;     }
string query_realm_url()          { return realm_url;    }
string query_bigmap_url()         { return bigmap_url;   }
string query_root_url()           { return root_url;     }
string query_login_url()          { return login_url;    }

void   set_name(string s)         { name = s;            }
void   set_brief(string s)        { brief = s;           }
void   set_description(string s)  { description = s;     }
void   set_announcement(string s) { announcement = s;    }
void   set_start_room(object o)   { start_room = o;      }
void   set_home(object o)         { home = o;            }
void   set_office(object o)       { office = o;          }
void   set_jail(object o)         { jail = o;            }
void   set_web_port(int i)        { web_port = i;        }
void   set_realm_url(string s)    { realm_url = s;       }
void   set_bigmap_url(string s)   { bigmap_url = s;      }
void   set_root_url(string s)     { root_url = s;        }
void   set_login_url(string s)    { login_url = s;       }

