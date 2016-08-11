private inherit "/lib/string";

inherit a "/usr/UserAPI/lib/authapi";
inherit c "/usr/UserAPI/lib/ctlapi";

# include <type.h>

# define CHUNK	15

int stop, count, create;

static
void create() {
   a::create();
   c::create();
}

void go(int n) {
   object **dats;

   create = n;
   stop = count = 0;

   dats = "/usr/SkotOS/sys/udatd"->query_udats();

   call_out("handle_some_dats", 0, dats);
}

void stop() {
   stop = 1;
}

private string str(string str) {
   if (str) {
      str = strip(str);
      if (strlen(str)) {
	 return str;
      }
   }
   return nil;
}

static
void handle_some_dats(object **dats) {
   string name, street1, street2, city, state, zip, country;
   object *now;
   int i, j;

   if (stop) {
      SysLog("UDAT ITERATION ABORTED");
      return;
   }
   if (sizeof(dats) == 0) {
      SysLog("UDAT ITERATION COMPLETE");
      return;
   }
   if (sizeof(dats[0]) > CHUNK) {
      now = dats[0][.. CHUNK-1];
      dats[0] = dats[0][CHUNK ..];
   } else {
      now = dats[0];
      dats = dats[1 ..];
   }

   call_out("handle_some_dats", 1, dats);

   for (i = 0; i < sizeof(now); i ++) {
      mapping propmap;
      string user, *props;
      int j;

      user = now[i]->query_name();

      propmap = now[i]->query_properties();
      props = map_indices(propmap);

      for (j = 0; j < sizeof(props); j ++) {
	 if (sscanf(props[j], "startstory:%*s") || props[j] == "beta") {
	    propmap[props[j]] = nil;
	 }
      }

      start_passlogin("post_login",
		      user,
		      "/usr/System/sys/authd"->query_password(user),
		      user,
		      "/usr/System/sys/authd"->query_password(user),
		      now[i]->query_creation_time(),
		      now[i]->query_email(),
		      propmap);

      count ++;
   }
   SysLog("Wrote " + count + "/" + sizeof(now));
}

static
void post_login(int success, string reply, string user, string pass,
		int created, string email, mapping props) {
   if (!success) {
      if (reply == "NO SUCH USER") {
	 if (!email) {
	    email = "[unknown]";
	 }
	 if (create) {
	    start_create("created", user, pass, email,
			 user, created, email, props);
	 }
	 return;
      }
      SysLog("User [" + user + "] error " + reply);
      return;
   }
   if (create) {
      SysLog("User [" + user + "] exists already.");
      call_out("created", 0, TRUE, "foo", user, created, email, props);
   }
}

static
void created(int success, string reply, string user, int created,
	     string email, mapping propmap) {
   string *props;
   int i;

   if (!success) {
      SysLog("User [" + user + "] creation FAILED [" + reply + "]; exiting");
      return;
   }

   props = map_indices(propmap) - ({ nil });

   for (i = 0; i < sizeof(props); i ++) {
      if (typeof(propmap[props[i]]) == T_STRING ||
	  typeof(propmap[props[i]]) == T_INT) {
	 start_setprop("post_setprop", user,
		       props[i], (string) propmap[props[i]],
		       user);
      }
   }
}

void post_setprop(int success, string reply, string user) {
   if (!success) {
      SysLog("Property setting PROBLEM [" + user + "] " + reply);
   } else {
      SysLog("Property successfully set on [" + user + "]");
   }
}
