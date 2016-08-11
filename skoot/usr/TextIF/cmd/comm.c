# include <type.h>
# include <TextIF.h>
# include <SkotOS.h>
# include <XML.h>
# include <UserAPI.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/date";

inherit authapi "/usr/UserAPI/lib/authapi";

inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/auth";
inherit "/usr/TextIF/lib/misc";

# define RP_STAMP  0
# define RP_HANDLE 1
# define RP_F_USER 2
# define RP_F_BODY 3
# define RP_T_NAME 4
# define RP_T_GAME 5
# define RP_TEXT   6

private int     remote_id;
private mapping remote_pages;

static void
create()
{
   BROADCAST->add_listener("page", this_object());
   remote_pages = ([ ]);
}

void patch() {
   BROADCAST->add_listener("page", this_object());
   remote_pages = ([ ]);
}

/*
 * Outgoing broadcasts:
 *   "page" "request"
 *
 * Incoming broadcasts:
 *   "page" "failure"
 *   "page" "success"
 *   "page" "pending"
 *
 * Incoming broadcasts as a result of the pending page being handled:
 *   "page" "deny"
 *   "page" "accept"
 */

private void send_page_failure(string game, string id, string msg)
{
   BROADCAST->broadcast_one(game, "page", "failure", ({ id, msg }));
}

private void send_page_pending(string game, string id, string pronoun)
{
   BROADCAST->broadcast_one(game, "page", "pending", ({ id, pronoun }));
}

private void send_page_accept(string game, string t_user, string t_body, string f_id, string text)
{
   BROADCAST->broadcast_one(game, "page", "accept", ({ t_user, t_body, f_id, text }));
}

private void send_page_deny(string game, string t_user, string t_body, string f_id, string text)
{
   BROADCAST->broadcast_one(game, "page", "deny", ({ t_user, t_body, f_id, text }));
}

void
desc_page_message(object *users, object body,
		  mixed msg,
		  varargs mixed alt_msg)
{
   int i, sz;

   sz = sizeof(users);
   for (i = 0; i < sz; i++) {
      if (alt_msg && is_root(users[i]->query_name())) {
	 if (typeof(alt_msg) == T_STRING) {
	    users[i]->message(TAG(alt_msg, "Page"));
	 } else {
	    users[i]->html_message(TAG(unsam(alt_msg), "Page"));
	 }
      } else {
	 if (typeof(msg) == T_STRING) {
	    users[i]->message(TAG(msg, "Page"));
	 } else {
	    users[i]->html_message(TAG(unsam(msg), "Page"));
	 }
      }
   }
}

void
channel_page_request(string f_game, int self, string page_id,
		     string f_id, mixed f_sh, string f_user, string f_body,
		     string to_name, string to_game, string text)
{
   int     t_sh;
   string  from, to_text;
   object  to_user, to_body;
   mapping map;

   f_sh = (int)f_sh;
   to_user = determine_user(to_name);
   if (!to_user) {
      send_page_failure(f_game, page_id, "Could not find anyone by that name.");
      return;
   }
   t_sh = !!is_root(to_user->query_name());
   to_body = to_user->query_body();
   if (!f_sh && !(t_sh && to_body->query_property("STORYHOST"))) {
      send_page_failure(f_game, page_id,
			"You can only page StoryHosts remotely.");
      return;
   }
   map = to_body->query_property("Page:BlockingRemote");
   if (to_body->query_property("Page:Busy") ||
       (map && map[lower_case(f_body + "@" + f_game)])) {
      send_page_failure(f_game, page_id,
			capitalize(describe_one(to_body)) + "@" + to_game +
			" is automatically blocking your page and will not " +
			"receive notification that you sent it.");
      return;
   }
   map = to_body->query_property("Page:PendingRemote");
   if (map && map[lower_case(f_body + "@" + f_game)]) {
      send_page_failure(f_game, page_id,
			"You are already waiting for page consent from " +
			capitalize(describe_one(to_body)) + "@" + to_game + ".");
      return;
   }
   map = to_body->query_property("Page:AllowRemote");
   if (!to_body->query_property("Page:AllowAll") &&
       (!map || !map[lower_case(f_body + "@" + f_game)])) {
      string cmd_accept, cmd_deny;

      map = to_body->query_property("Page:PendingRemote");
      if (!map) {
	 map = ([ ]);
      }
      /* ... */
      map[lower_case(f_body + "@" + f_game)] = ({ f_id, f_user, f_body, f_game, text });
      to_body->set_property("Page:PendingRemote", map);

      cmd_accept =
	 (to_user->query_udat()->query_property("TextIF:ChatMode") ? "/" : "") +
	 "@page accept " + capitalize(f_body) + "@" + f_game;
      cmd_deny =
	 (to_user->query_udat()->query_property("TextIF:ChatMode") ? "/" : "") +
	 "@page deny " + capitalize(f_body) + "@" + f_game;

      to_body->occur("page_message",
		     capitalize(f_body) + "@" + f_game + " is trying to page you.\n" +
		     "If you would like to receive the page, type: " + LINK(cmd_accept, cmd_accept, "command") + "\n" +
		     "If you would like to refuse the page, type: " +  LINK(cmd_deny, cmd_deny, "command") + "\n");

      send_page_pending(f_game, page_id, to_body->query_pronoun());
      return;
   }

   from = capitalize(f_body) + "@" + f_game;
   to_text = "[OOC Page] from " + from    + ": \"" + text + "\"\n";
   to_body->occur("page_message", to_text);
   to_body->add_page_history(time(),
			     f_body,
			     describe_one(to_body),
			     to_text);

   BROADCAST->broadcast_one(f_game, "page", "success",
     ({ page_id, to_user->query_name(), describe_one(to_body) }));
}

void
channel_page_failure(string f_game, int self, mixed id, string text)
{
   mapping data;

   id = (int)id;
   if (!remote_pages[id]) {
      return;
   }
   data = remote_pages[id];
   remote_pages[id] = nil;
   if (data[RP_F_BODY]) {
      data[RP_F_BODY]->occur("page_message", text + "\n");
   }
}

void
channel_page_pending(string f_game, int self, mixed id, string pronoun)
{
   mapping data;

   id = (int)id;
   if (!remote_pages[id]) {
      return;
   }
   data = remote_pages[id];
   remote_pages[id] = nil;
   if (data[RP_F_BODY]) {
      data[RP_F_BODY]->occur("page_message",
			     capitalize(data[RP_T_NAME]) + "@" + f_game +
			     " has been informed of your page and " +
			     "must consent before " + pronoun +
			     " receives it.\n");
   }
}

void
channel_page_deny(string f_game, int self, string t_user, string t_body, string f_id, string text)
{
   string str;
   object f_body;

   f_body = find_object(f_id);
   if (!f_body) {
      return;
   }
   str = capitalize(t_body) + "@" + f_game + " has chosen to dismiss your page unread.\n";
   f_body->add_page_history(time(), describe_one(f_body), capitalize(t_body) + "@" + f_game, str);
   if (f_body->query_souls() && sizeof(f_body->query_souls())) {
      f_body->occur("page_message", str);
   } else {
      mapping map;

      map = f_body->query_property("Page:Denied");
      if (!map) {
	 map = ([ ]);
      }
      map[capitalize(t_body) + "@" + f_game] = "\"" + text + "\"";
      f_body->set_property("Page:Denied", map);
   }
}

void
channel_page_accept(string f_game, int self, string t_user, string t_body, string f_id, string text)
{
   string str;
   object f_body;

   f_body = find_object(f_id);
   if (!f_body) {
      return;
   }
   str = "[OOC Page] accepted by " + capitalize(t_body) + "@" + f_game + ": \"" + text + "\"\n";
   f_body->add_page_history(time(), describe_one(f_body), capitalize(t_body) + "@" + f_game, str);
   if (f_body->query_souls() && sizeof(f_body->query_souls())) {
      f_body->occur("page_message", str);
   } else {
      mapping map;

      map = f_body->query_property("Page:Accepted");
      if (!map) {
	 map = ([ ]);
      }
      map[capitalize(t_body) + "@" + f_game] = "\"" + text + "\"";
      f_body->set_property("Page:Accepted", map);
   }
}

void
channel_page_success(string f_game, int self, mixed id,
		     string to_user, string to_body)
{
   int f_sh;
   string to, text;
   mapping data;
   object body;

   id = (int)id;
   if (!remote_pages[id]) {
      return;
   }
   data = remote_pages[id];
   remote_pages[id] = nil;
   body = data[RP_F_BODY];
   if (!body) {
      return;
   }
   to = capitalize(to_body) + "@" + f_game;
   text = "[OOC Page] to " + to + ": \"" + data[RP_TEXT] + "\"\n";
   body->occur("page_message", text);
   body->add_page_history(data[RP_STAMP], describe_one(body), to_body, text);
}

static int
show_pages_pending(object user, object body)
{
    int    output, i, sz;
    string *names;
    mixed  *list, pending_map;

    output = FALSE;

    pending_map = body->query_property("Page:Pending");
    if (typeof(pending_map) == T_MAPPING && map_sizeof(pending_map)) {
	sz   = map_sizeof(pending_map);
	list = map_indices(pending_map);
	names = allocate(sz);

	for (i = 0; i < sz; i++) {
	    string desc;

	    desc = describe_one(list[i]);
	    names[i] = LINK(desc, "@page accept " + desc, "command");
	}
	body->occur("page_message",
		    "You have pending pages from " +
		    (sz == 1 ? names[0] : (implode(names[.. sz - 2], ", ") + " and " + names[sz - 1])) + ".\n");
	output = TRUE;
    }

    pending_map = body->query_property("Page:PendingRemote");
    if (typeof(pending_map) == T_MAPPING && map_sizeof(pending_map)) {
	sz    = map_sizeof(pending_map);
	list  = map_indices(pending_map);
	names = allocate(sz);

	for (i = 0; i < sz; i++) {
	    names[i] = LINK(list[i], "@page accept " + list[i], "command");
	}
	body->occur("page_message",
		    "You have pending remote pages from " +
		    (sz == 1 ? names[0] : (implode(names[.. sz - 2], ", ") + " and " + names[sz - 1])) + ".\n");
	output = TRUE;
    }

    return output;
}

static string
show_command_list(mixed *bodies, string prefix)
{
    int i, sz;
    string *names;

    sz = sizeof(bodies);
    names = allocate(sz);
    for (i = 0; i < sz; i++) {
	string desc;

	if (typeof(bodies[i]) == T_STRING) {
	    desc = bodies[i];
	} else {
	    desc = describe_one(bodies[i]);
	}
	names[i] = LINK(desc, prefix + " " + desc, "command");
    }
    if (sz == 1) {
	return names[0];
    }
    return implode(names[.. sz - 2], ", ") + " and " + names[sz - 1];
}

static void
show_page_status(object user, object body) {
    int   output;
    mixed pending_map, accepted_map, denied_map;

    output = FALSE;
    if (body->query_property("Page:AllowAll")) {
        body->occur("page_message", "You are allowing pages from everyone.\n");
	output = TRUE;
    } else {
	mixed allow_map, block_map;
	string *allow_r, *block_r;
	object *allow, *block;

	allow_map = body->query_property("Page:Allow");
	if (typeof(allow_map) != T_MAPPING || map_sizeof(allow_map) == 0) {
	    allow = nil;
	} else {
	    allow = map_indices(allow_map);
	}

	allow_map = body->query_property("Page:AllowRemote");
	if (typeof(allow_map) != T_MAPPING || map_sizeof(allow_map) == 0) {
	   allow_r = nil;
	} else {
	   allow_r = map_indices(allow_map);
	}

	if (allow && block) {
	    /*
	     * Block overrides allow, aside from the fact that blocking someone
	     * should kick them out of your allow list anyway, and vice versa!
	     */
	    allow -= block;
	    if (!sizeof(allow)) {
		allow = nil;
	    }
	}

	block_map = body->query_property("Page:Blocking");
	if (typeof(block_map) != T_MAPPING || map_sizeof(block_map) == 0) {
	    block = nil;
	} else {
	    block = map_indices(block_map);
	}

	block_map = body->query_property("Page:BlockRemote");
	if (typeof(block_map) != T_MAPPING || map_sizeof(block_map) == 0) {
	   block_r = nil;
	} else {
	   block_r = map_indices(block_map);
	}

	if (allow_r && block_r) {
	   allow_r -= block_r;
	   if (!sizeof(allow_r)) {
	      allow_r = nil;
	   }
	}

	if (allow) {
	    body->occur("page_message",
			"You are allowing pages from " +
			show_command_list(allow, "@page unallow") + ".\n");
	    output = TRUE;
	}
	if (allow_r) {
	    body->occur("page_message",
			"You are allowing remote pages from " +
			show_command_list(allow_r, "@page unallow") + ".\n");
	    output = TRUE;
	}
	if (block) {
	    body->occur("page_message",
			"You are blocking pages from " +
			show_command_list(block, "@page unblock") + ".\n");
	    output = TRUE;
	}
	if (block_r) {
	    body->occur("page_message",
			"You are blocking remote pages from " +
			show_command_list(block_r, "@page unblock") + ".\n");
	    output = TRUE;
	}
    }

    output = output | show_pages_pending(user, body);

    accepted_map = body->query_property("Page:Accepted");
    if (typeof(accepted_map) == T_MAPPING && map_sizeof(accepted_map)) {
	int    i, sz;
	string result, *pages;
	mixed *bodies;

	result = "The following pages were accepted during your absence:\n";
	sz = map_sizeof(accepted_map);
	bodies = map_indices(accepted_map);
	pages = map_values(accepted_map);
	for (i = 0; i < sz; i++) {
	    if (typeof(bodies[i]) == T_OBJECT) {
	       bodies[i] = describe_one(bodies[i]);
	    }
	    result += capitalize(bodies[i]) + " accepted your page: " + pages[i] + "\n";
	}
	body->occur("page_message", result);
	body->clear_property("Page:Accepted");
	output = TRUE;
    }

    denied_map = body->query_property("Page:Denied");
    if (typeof(denied_map) == T_MAPPING && map_sizeof(denied_map)) {
	int    i, sz;
	string result, *pages;
	mixed  *bodies;

	result = "The following pages were denied unseen during your absence:\n";
	sz = map_sizeof(denied_map);
	bodies = map_indices(denied_map);
	pages = map_values(denied_map);
	for (i = 0; i < sz; i++) {
	    if (typeof(bodies[i]) == T_OBJECT) {
	       bodies[i] = describe_one(bodies[i]);
	    }
	    result += capitalize(bodies[i]) + " accepted your page: " + pages[i] + "\n";
	}
	body->occur("page_message", result);
	body->clear_property("Page:Denied");
	output = TRUE;
    }

    if (!output) {
       body->occur("page_message",
		   "You are not allowing or blocking anyone, " +
		   "nor are there any pending pages.\n");
    }
}

private void
do_actual_page(object from_user, object from_body, object to_user, object to_body, string text)
{
    string from_text, from_text_sh, to_text, to_text_sh;

    /*
     * If the sender is a StoryHost/StoryPlotter/CE/whatever, show the
     * username of the recipient as well.
     */

    from_text    = "[OOC Page] to " + describe_one(to_body) + ": \"" + text +
                   "\"\n";
    from_text_sh = "[OOC Page] to [" + capitalize(to_user->query_name()) +
                   "/" + describe_one(to_body) + "]: \"" + text + "\"\n";

    /*
     * If the recipient is a StoryHost/StoryPlotter/CE/whatever, show the
     * username of the sender as well.
     */
    to_text    = "[OOC Page] from " + describe_one(from_body) + ": \"" + text +
                 "\"\n";
    to_text_sh = "[OOC Page] from [" + capitalize(from_user->query_name()) +
                 "/" + describe_one(from_body) + "]: \"" + text + "\"\n";

    from_body->occur("page_message", from_text, from_text_sh);
    to_body->occur("page_message", to_text, to_text_sh);

    catch {
       from_body->add_page_history(time(),
				   describe_one(from_body),
				   describe_one(to_body),
				   from_text);
       to_body->add_page_history(time(),
				 describe_one(from_body),
				 describe_one(to_body),
				 to_text);
    }
}

void
cmd_DEV_page(object user, object body, varargs mixed *strings, mixed msg)
{
    string to_name, text;
    object to_user;

    if (!strings || !msg || !msg["evoke"]) {
        body->occur("page_message", "Usage: +page person \"message\"\n");
	return;
    }
    to_name = implode(strings, " ");

    text = msg["evoke"];
    text = text[1..strlen(text) - 2];

    to_user = determine_user(to_name, TRUE);

    if (!to_user) {
        body->occur("page_message", "Could not find anyone with that name.\n");
	return;
    }
# if 0     /* none of your business! */
    if (user == to_user) {
        body->occur("page_message", "Talking to yourself again, eh?\n");
	return;
    }
# endif
    do_actual_page(user, body, to_user, to_user->query_body(), text);
}

void
cmd_DEV_page_remote(object user, object body, mixed target, mixed msg)
{
   int    handle;
   string name, game, text;

   if (!msg || !msg["evoke"]) {
      body->occur("page_message", "Usage: +page person@game \"message\"\n");
      return;
   }
   if (typeof(target) == T_ARRAY) {
      target = implode(target, " ");
   }
   sscanf(target, "%s@%s", name, game);

   name = lower_case(name);
   game = lower_case(game);

   text = msg["evoke"];
   text = text[1..strlen(text) - 2];

   if (!remote_pages) {
      remote_pages = ([ ]);
   }
   remote_id++;
   handle = call_out("delayed_page_remote", 30, remote_id);
   remote_pages[remote_id] = ([
       RP_STAMP:  time(),
       RP_HANDLE: handle,
       RP_F_USER: user,
       RP_F_BODY: body,
       RP_T_NAME: name,
       RP_T_GAME: game,
       RP_TEXT:   text
   ]);
   BROADCAST->broadcast_one(game, "page", "request",
			    ({ (string)remote_id,
			       ur_name(body),
			       !!is_root(user->query_name()),
			       user->query_name(),
			       describe_one(body),
			       name, game, text }));
}

void
cmd_page_remote(object user, object body, mixed target, mixed msg)
{
   cmd_DEV_page_remote(user, body, target, msg);
}

static void
delayed_page_remote(int id)
{
   mapping data;

   if (!remote_pages[id]) {
      return;
   }
   data = remote_pages[id];
   data[RP_F_BODY]->occur("page_message",
			  "No response was received for your page to " +
			  capitalize(data[RP_T_NAME]) + "@" + data[RP_T_GAME] +
			  ".  " +
			  "This may be due to lag, or because the game does " +
			  "not exist or isn't currently running.  " +
			  "If a response does arrive within the next hour, " +
			  "you will be notified, after that it will be " +
			  "discarded.\n");
   call_out("purge_page_remote", 3600, id);
}

static void
purge_page_remote(int id)
{
   remote_pages[id] = nil;
}

void
cmd_GUIDE_page(object user, object body, varargs mixed *strings, mixed msg)
{
    string to_name, text;
    object to_user;

    if (check_storyguide_body(user, body)) {
	return;
    }
    if (!strings || !msg || !msg["evoke"]) {
        body->occur("page_message", "Usage: !page person \"message\"\n");
	return;
    }
    to_name = implode(strings, " ");

    text = msg["evoke"];
    text = text[1..strlen(text) - 2];

    to_user = determine_user(to_name);

    if (!to_user) {
        body->occur("page_message", "Could not find anyone with that name.\n");
	return;
    }
    if (user == to_user) {
        body->occur("page_message", "Talking to yourself again, eh?\n");
	return;
    }
    do_actual_page(user, body, to_user, to_user->query_body(), text);
}


void cmd_page(object user, object body, varargs mixed *strings, mixed msg)
{
    string text, to_name;
    object to_user, to_body;
    mixed  map;

# if 0
    /*
     * Enable this if you want StoryGuides and StoryHosts/Plotters to
     * automatically use their !page and +page commands, instead.
     */
    if (is_root(user->query_name())) {
	cmd_DEV_page(user, body, strings, msg);
	return;
    }
    if (body->query_property("GUIDE")) {
	cmd_GUIDE_page(user, body, strings, msg);
	return;
    }
# endif

    if (!strings && (!msg || !msg["evoke"])) {
        string output;

	output = 
	  "Usage:\n" +
	  "@page <name>; \"message\"\n" +
	  "@page (accept | deny | allow | unallow | block | unblock | " +
	  LINK("busy",   "@page busy",   "command") + " | " +
	  LINK("recall", "@page recall", "command") + " | " +
	  LINK("status", "@page status", "command") + ") ...\n";
	body->occur("page_message", output);
	show_pages_pending(user, body);
	return;
    }

    if (!strings || !msg || !msg["evoke"]) {
        body->occur("page_message", "Usage: @page person \"message\"\n");
	return;
    }

    to_name = implode(strings, " ");

    text = msg["evoke"];
    text = text[1..strlen(text) - 2];

    to_user = determine_user(to_name);
    if (!to_user) {
        body->occur("page_message", "Could not find anyone by that name.\n");
	return;
    }
    if (user == to_user) {
        body->occur("page_message", "Talking to yourself again, eh?\n");
	return;
    }
    to_body = to_user->query_body();
    if (to_body->query_property("disguised")) {
        body->occur("page_message", "Could not find anyone by that name.\n");
	return;
    }
    if (!CONFIGD->query("Base:Pages:Global")) {
	/*
	 * This could go in one if-statement, yes, but I'm trying to keep my
	 * head straight on. :) -EH.
	 */
	if (!is_root(user->query_name()) &&
	    !sizeof(rfilter(user->query_udat()->query_bodies(), "query_property", "GUIDE"))) {
	    /*
	     * You're not a StoryHost and none of your user's characters are StoryGuides
	     */
	    if (!(is_root(to_user->query_name()) && to_body->query_property("STORYHOST")) && !to_body->query_property("GUIDE")) {
		/*
		 * Ah, but your target isn't a StoryGuide or StoryHost, no go!
		 */
 	        body->occur("page_message",
			    "You can only page StoryHosts and StoryGuides.\n");
		return;
	    }
	}
    }

    /*
     * Checking for 'consent', the-page-way.
     */
    map = to_body->query_property("Page:Blocking");
    if (to_body->query_property("Page:Busy") ||
	(typeof(map) == T_MAPPING && map[body])) {
       body->occur("page_message",
		   capitalize(describe_one(to_body)) +
		   " is automatically blocking your page and will not " +
		   "receive notification that you sent it.\n");
       return;
    }

    map = to_body->query_property("Page:Pending");
    if (typeof(map) == T_MAPPING && map[body]) {
       body->occur("page_message",
		   "You are already waiting for page consent from " +
		   capitalize(describe_one(to_body)) + ".\n");
       return;
    }
    map = to_body->query_property("Page:Allow");
    if (!to_body->query_property("Page:AllowAll") &&
	(typeof(map) != T_MAPPING || !map[body])) {
        string cmd_accept, cmd_deny;

	map = to_body->query_property("Page:Pending");
	if (!map) {
	    map = ([ ]);
	}
	map[body] = msg["evoke"];
	to_body->set_property("Page:Pending", map);

	cmd_accept =
	   (to_user->query_udat()->query_property("TextIF:ChatMode") ? "/" : "") +
	   "@page accept " + describe_one(body);
	cmd_deny =
	   (to_user->query_udat()->query_property("TextIF:ChatMode") ? "/" : "") +
	   "@page deny " + describe_one(body);
	to_body->occur("page_message",
		       capitalize(describe_one(body)) + " is trying to page you.\n" +
		       "If you would like to receive the page, type: " + LINK(cmd_accept, cmd_accept, "command") + "\n" +
		       "If you would like to refuse the page, type: " + LINK(cmd_deny, cmd_deny, "command") + "\n");

	body->occur("page_message",
		    capitalize(describe_one(to_body)) + " has been informed " +
		    "of your page and must consent before " +
		    to_body->query_pronoun() + " receives it.\n");
	return;
    }

    /*
     * The actual page.
     */
    do_actual_page(user, body, to_user, to_body, text);
}


void cmd_DEV_busy ( object user, object body, varargs string str) {
    string verb;

    if (is_root(user->query_name())) {
	verb = "+busy";
    } else {
	verb = "!busy";
    }
    if (!str) {
	user->paragraph(TAG("Usage:\n" +
			    verb + "      shows this message\n" +
			    verb + " on   blocks incoming pages\n" +
			    verb + " off  unblocks incoming pages\n",
			    "Page"));
	return;
    }
    switch(lower_case(str)) {
    case "on":
    case "yes":
	body->set_property("Page:Busy", TRUE);
	user->paragraph(TAG("Busy mode ON.\n", "Page"));
	return;
    case "off":
    case "no":
	body->clear_property("Page:Busy");
	user->paragraph(TAG("Busy mode OFF.\n", "Page"));
	return;
    }
}

/*
 * Do The 'page accept <name>' thing.
 */
private void
do_page_accept(object user, object body,
	       object t_user, object t_body,
	       mapping pending_map)
{
    string page;
    string text;

    page = pending_map[t_body];
    pending_map[t_body] = nil;
    if (map_sizeof(pending_map)) {
        body->set_property("Page:Pending", pending_map);
    } else {
        body->clear_property("Page:Pending");
    }
    text = "[OOC Page] accepted by " + capitalize(describe_one(body)) +
	   ": " + page + "\n";
    catch {
	t_body->add_page_history(time(),
				 describe_one(t_body),
				 describe_one(body),
				 text);
    }
    if (t_user) {
       t_body->occur("page_message", text);
    } else {
        mapping map;

	map = t_body->query_property("Page:Accepted");
	if (!map) {
	    map = ([ ]);
	}
	map[body] = page;
	t_body->set_property("Page:Accepted", map);
    }
    text = "[OOC Page] accepted from " + describe_one(t_body) + ": " + page +
	   "\n";
    body->occur("page_message", text);
    catch {
	body->add_page_history(time(),
			       describe_one(t_body),
			       describe_one(body),
			       text);
    }
}

private void
do_page_accept_remote(object user, object body, string t_name, mapping pending_map)
{
   string text;
   mixed  *data;

   data = pending_map[t_name];
   pending_map[t_name] = nil;
   if (map_sizeof(pending_map)) {
      body->set_property("Page:PendingRemote", pending_map);
   } else {
      body->clear_property("Page:PendingRemote");
   }
   /* id, user, body, game, text */
   text = "[OOC Page] accepted from " + data[2] + "@" + data[3] + ": \"" + data[4] + "\"\n";
   body->occur("page_message", text);
   body->add_page_history(time(), data[2] + "@" + data[3], describe_one(body), text);
   send_page_accept(data[3], user->query_name(), describe_one(body), data[0], data[4]);
}

/*
 * Do The 'page deny <name>' thing.
 */
private void
do_page_deny(object user, object body,
	     object t_user, object t_body,
	     mapping pending_map)
{
    string page;
    string text;

    page = pending_map[t_body];
    pending_map[t_body] = nil;
    if (map_sizeof(pending_map)) {
	body->set_property("Page:Pending", pending_map);
    } else {
	body->clear_property("Page:Pending");
    }
    text = capitalize(describe_one(body)) +
	   " has chosen to dismiss your page unread.\n";
    catch {
	t_body->add_page_history(time(),
				 describe_one(t_body),
				 describe_one(body),
				 text);
    }
    if (t_user) {
	t_body->occur("page_message", text);
    } else {
	mapping map;

	map = t_body->query_property("Page:Denied");
	if (!map) {
	    map = ([ ]);
	}
	map[body] = page;
	t_body->set_property("Page:Denied", map);
    }
    text = "You dismiss the page from " +
	   describe_one(t_body) + " unread.\n";
    body->occur("page_message", text);
    catch {
	body->add_page_history(time(),
			       describe_one(t_body),
			       describe_one(body),
			       text);
    }
}

private void
do_page_deny_remote(object user, object body, string t_name, mapping pending_map)
{
   string text;
   mixed  *data;

   data = pending_map[t_name];
   pending_map[t_name] = nil;
   if (map_sizeof(pending_map)) {
      body->set_property("Page:PendingRemote", pending_map);
   } else {
      body->clear_property("Page:PendingRemote");
   }
   /* id, user, body, game, text */
   text = "You dismiss the page from " + data[2] + "@" + data[3] + " unread.\n";
   body->occur("page_message", text);
   body->add_page_history(time(), data[2] + "@" + data[3], describe_one(body), text);
   send_page_deny(data[3], user->query_name(), describe_one(body), data[0], data[4]);
}

void
cmd_page_consent(object user, object body, varargs string what, mixed words)
{
    string t_name, page;
    object t_user, t_body;
    mixed  allow_map, block_map, pending_map,
           allow_r_map, block_r_map, pending_r_map;

    if (!what || (what != "busy" && !words)) {
	/* Show who we are blocking, what's pending, etc. */
	show_page_status(user, body);
	return;
    }
    if (words) {
       if (typeof(words) == T_STRING) {
	  t_name = lower_case(words);
       } else {
	  t_name = lower_case(implode(words, " "));
       }
    }

    allow_map = body->query_property("Page:Allow");
    if (typeof(allow_map) != T_MAPPING) {
	allow_map = nil;
    }

    block_map = body->query_property("Page:Blocking");
    if (typeof(block_map) != T_MAPPING) {
	block_map = nil;
    }

    pending_map = body->query_property("Page:Pending");
    if (typeof(pending_map) != T_MAPPING) {
	pending_map = nil;
    }

    allow_r_map = body->query_property("Page:AllowRemote");
    if (typeof(allow_r_map) != T_MAPPING) {
	allow_r_map = nil;
    }

    block_r_map = body->query_property("Page:BlockingRemote");
    if (typeof(block_r_map) != T_MAPPING) {
	block_r_map = nil;
    }

    pending_r_map = body->query_property("Page:PendingRemote");
    if (typeof(pending_r_map) != T_MAPPING) {
	pending_r_map = nil;
    }

    switch (what) {
    case "accept":
	t_user = determine_user(t_name);
	if (t_user) {
	    t_body = t_user->query_body();
	} else {
	    t_body = UDATD->query_body(t_name);
	}
	if (!t_body) {
	    body->occur("page_message",
			"Could not find anyone by that name.\n");
	    return;
	}
	if (!pending_map || !pending_map[t_body]) {
	    body->occur("page_message",
			capitalize(describe_one(t_body)) +
			" is not trying to send you any pages. " +
			"Perhaps you want the '@page allow' command?\n");
	    return;
	}
	do_page_accept(user, body, t_user, t_body, pending_map);
	return;
    case "accept_remote":
       if (!pending_r_map || !pending_r_map[t_name]) {
	   body->occur("page_message",
		       capitalize(t_name) + " is not trying to send you any " +
		       "pages. Perhaps you want the '@page allow' command?\n");
	   return;
       }
       do_page_accept_remote(user, body, t_name, pending_r_map);
       return;
    case "allow":
	switch (t_name) {
	case "all":
	    if (body->query_property("Page:AllowAll")) {
		body->occur("page_message",
			    "You are already allowing pages from everyone.\n");
	    } else {
		body->set_property("Page:AllowAll", TRUE);
		body->occur("page_message",
			    "You are now allowing pages from everyone.\n" +
			    "To return to discriminatory mode, type: " +
			    LINK("@page unallow all", "@page unallow all", "command") + "\n");
	    }
	    return;
	case "notall":
	    /* BCC - EH */
	    body->occur("page_message",
			"This command has been replaced with '@page unallow all'.\n");
	    return;
	default:
	    t_user = determine_user(t_name);

	    if (t_user) {
		t_body = t_user->query_body();
	    } else {
		t_body = UDATD->query_body(t_name);
	    }
	    if (!t_body) {
	        body->occur("page_message",
			    "Could not find anyone by that name.\n");
		return;
	    }
	    if (allow_map && allow_map[t_body]) {
		body->occur("page_message",
			    "You already accept pages from " +
			    describe_one(t_body) + ".\n");
		return;
	    }
	    if (pending_map && pending_map[t_body]) {
	        /* First do a 'page accept <name>'. */
	        do_page_accept(user, body, t_user, t_body, pending_map);

		/* Then fall-through and the actual 'page allow <name>' thing. */
	    }
	    /*
	     * Add to Page:Allow map and optionally remove from Page:Blocking
	     * map, not much point in having someone in both lists.
	     */
	    if (!allow_map) {
		allow_map = ([ ]);
	    }
	    allow_map[t_body] = TRUE;
	    body->set_property("Page:Allow", allow_map);
	    if (block_map && block_map[t_body]) {
		block_map[t_body] = nil;
		if (map_sizeof(block_map)) {
		    body->set_property("Page:Blocking", block_map);
		} else {
		    body->clear_property("Page:Blocking");
		}
	    }
	    body->occur("page_message",
			"You now accept pages from " + describe_one(t_body) +
			".\n");
	    return;
	}
	return;
    case "allow_remote":
       if (allow_r_map && allow_r_map[t_name]) {
	  body->occur("page_message",
		      "You already accept pages from " + capitalize(t_name) +
		      ".\n");
	  return;
       }
       if (pending_r_map && pending_r_map[t_name]) {
	  do_page_accept_remote(user, body, t_name, pending_r_map);
       }
       if (!allow_r_map) {
	  allow_r_map = ([ ]);
       }
       allow_r_map[t_name] = TRUE;
       body->set_property("Page:AllowRemote", allow_r_map);
       if (block_r_map && block_r_map[t_body]) {
	  block_r_map[t_name] = nil;
	  if (map_sizeof(block_r_map)) {
	     body->set_property("Page:BlockingRemote", block_r_map);
	  } else {
	     body->clear_property("Page:BlockingRemote");
	  }
       }
       body->occur("page_message",
		   "You now accept pages from " + capitalize(t_name) + ".\n");
       return;
    case "busy":
       if (t_name && strlen(t_name)) {
	  switch(lower_case(t_name)) {
	  case "on":
	  case "yes":
	     body->set_property("Page:Busy", TRUE);
	     user->paragraph(TAG("Busy mode ON.\n", "Page"));
	     return;
	  case "off":
	  case "no":
	     body->clear_property("Page:Busy");
	     user->paragraph(TAG("Busy mode OFF.\n", "Page"));
	     return;
	  }
       }
       user->paragraph(TAG("Usage:\n" +
			   "@page busy     shows this message\n" +
			   LINK("@page busy on", "@page busy on", "command")   + "  blocks incoming pages\n" +
			   LINK("@page busy off", "@page busy off", "command") + " unblocks incoming pages\n",
			   "Page"));
       return;
    case "unallow":
	switch (t_name) {
	case "all":
	    if (body->query_property("Page:AllowAll")) {
	        body->occur("page_message",
			    "You are no longer allowing pages from everyone.\n");
		body->clear_property("Page:AllowAll");
	    } else {
		body->occur("page_message",
			    "You are not allowing pages from everyone.\n");
	    }
	    return;
	default:
	    t_user = determine_user(t_name);
	    if (t_user) {
		t_body = t_user->query_body();
	    } else {
		t_body = UDATD->query_body(t_name);
	    }
	    if (!t_body) {
	        body->occur("page_message",
			    "Could not find anyone by that name.\n");
		return;
	    }
	    if (!allow_map || !allow_map[t_body]) {
		body->occur("page_message",
			    "You are not accepting pages from " +
			    describe_one(t_body) + ".\n");
		return;
	    }
	    allow_map[t_body] = nil;
	    if (map_sizeof(allow_map)) {
		body->set_property("Page:Allow", allow_map);
	    } else {
		body->clear_property("Page:Allow");
	    }
	    body->occur("page_message",
			"You are no longer accepting pages from " +
			describe_one(t_body) + ".\n");
	    return;
	}
	return;
    case "unallow_remote":
       if (!allow_r_map || !allow_r_map[t_name]) {
	  body->occur("page_message",
		      "You are not accepting pages from " +
		      capitalize(t_name) + ".\n");
	  return;
       }
       allow_r_map[t_name] = nil;
       if (map_sizeof(allow_r_map)) {
	  body->set_property("Page:AllowRemote", allow_r_map);
       } else {
	  body->clear_property("Page:AllowRemote");
       }
       body->occur("page_message",
		   "You are no longer accepting pages from " +
		   capitalize(t_name) + ".\n");
       return;
    case "deny":
	t_user = determine_user(t_name);
	if (t_user) {
	    t_body = t_user->query_body();
	} else {
	    t_body = UDATD->query_body(t_name);
	}
	if (!t_body) {
	    body->occur("page_message",
			"Could not find anyone by that name.\n");
	    return;
	}
	if (!pending_map || !pending_map[t_body]) {
	    body->occur("page_message",
			capitalize(describe_one(t_body)) +
			" is not trying to send you any pages. " +
			"Perhaps you want the '@page block' command?\n");
	    return;
	}
	do_page_deny(user, body, t_user, t_body, pending_map);
	return;
    case "deny_remote":
       if (!pending_r_map || !pending_r_map[t_name]) {
	   body->occur("page_message",
		       capitalize(t_name) +
		       " is not trying to send you any pages. " +
		       "Perhaps you want the '@page block' command?\n");
	   return;
       }
       do_page_deny_remote(user, body, t_name, pending_r_map);
       return;
    case "block":
	t_user = determine_user(t_name);
	if (t_user) {
	    t_body = t_user->query_body();
	} else {
	    t_body = UDATD->query_body(t_name);
	}
	if (!t_body) {
	    body->occur("page_message",
			"Could not find anyone by that name.\n");
	    return;
	}
	if (block_map && block_map[t_body]) {
	    body->occur("page_message",
			"You already block " + describe_one(t_body) + ".\n");
	    return;
	}
	if (pending_map && pending_map[t_body]) {
	    /* First do a 'page deny <name>'. */
	    do_page_deny(user, body, t_user, t_body, pending_map);

	    /* Then fall-through and do the actual 'page block <name>' thing. */
	}
	/*
	 * Add to Page:Blocking map and optionally remove from Page:Allow
	 * map, not much point in having someone in both lists.
	 */
	if (!block_map) {
	    block_map = ([ ]);
	}
	block_map[t_body] = TRUE;
	body->set_property("Page:Blocking", block_map);
	if (allow_map && allow_map[t_body]) {
	    allow_map[t_body] = nil;
	    if (map_sizeof(allow_map)) {
		body->set_property("Page:Allow", allow_map);
	    } else {
		body->clear_property("Page:Allow");
	    }
	}
	body->occur("page_message",
		    "You now block pages from " + describe_one(t_body) +
		    ".\n");
	return;
    case "block_remote":
       if (block_r_map && block_r_map[t_name]) {
	  body->occur("page_message",
		      "You already block " + capitalize(t_name) + ".\n");
	  return;
       }
       if (pending_r_map && pending_r_map[t_name]) {
	  do_page_deny_remote(user, body, t_name, pending_r_map);
       }
       if (!block_r_map) {
	  block_r_map = ([ ]);
       }
       block_r_map[t_name] = TRUE;
       body->set_property("Page:BlockingRemote", block_r_map);
       if (allow_r_map && allow_r_map[t_name]) {
	  allow_r_map[t_name] = nil;
	  if (map_sizeof(allow_r_map)) {
	     body->set_property("Page:AllowRemote", allow_r_map);
	  } else {
	     body->clear_property("Page:AllowRemote");
	  }
       }
       body->occur("page_message",
		  "You now block pages from " + capitalize(t_name) + ".\n");
       return;
    case "unblock":
	t_user = determine_user(t_name);
	if (t_user) {
	    t_body = t_user->query_body();
	} else {
	    t_body = UDATD->query_body(t_name);
	}
	if (!t_body) {
	    body->occur("page_message",
			"Could not find anyone by that name.\n");
	    return;
	}
	if (!block_map || !block_map[t_body]) {
	    body->occur("page_message",
			"You are not blocking " + describe_one(t_body) +
			".\n");
	    return;
	}
	block_map[t_body] = nil;
	if (map_sizeof(block_map)) {
	    body->set_property("Page:Blocking", block_map);
	} else {
	    body->clear_property("Page:Blocking");
	}
	body->occur("page_message",
		    "You are no longer blocking pages from " +
		    describe_one(t_body) + ".\n");
	return;
    case "unblock_remote":
       if (!block_r_map || !block_r_map[t_name]) {
	  body->occur("page_message",
		      "You are not blocking " + capitalize(t_name) + ".\n");
	  return;
       }
       block_r_map[t_name] = nil;
       if (map_sizeof(block_r_map)) {
	  body->set_property("Page:BlockingRemote", block_map);
       } else {
	  body->clear_property("Page:BlockingRemote");
       }
       body->occur("page_message",
		   "You are no longer blocking pages from " +
		   capitalize(t_name) + ".\n");
       return;
    default:
        body->occur("page_message",
		    "Internal error: You should not be able to get here.\n");
	return;
    }
}

private string
time_difference(int then, int now)
{
    int t, d, h, m;
    string *bits;

    t = now - then;
    bits = ({ nil, nil, nil });
    d = t / 86400;
    t -= d * 86400;
    h = t / 3600;
    t -= h * 3600;
    m = t / 60;
    bits = ({
	d ? d == 1 ? "1 day"    : d + " days"    : nil,
	h ? h == 1 ? "1 hour"   : h + " hours"   : nil,
	m ? m == 1 ? "1 minute" : m + " minutes" : nil
    });
    bits -= ({ nil });
    if (sizeof(bits) > 2) {
	bits[..1];
    }
    return implode(bits, " and ");
}

void cmd_page_recall(object user, object body, varargs string *words)
{
    int    i, sz, ts;
    string name, result;
    mixed  *data;

    if (words) {
	name = implode(words, " ");
    }
    data = body->query_page_history(20, name);
    sz = sizeof(data);
    if (!sz) {
	if (name) {
	    user->message("You do not recall pages to or from that person.\n");
	} else {
	    user->message("You do not recall any pages.\n");
	}
	return;
    }
    result = "You recall the following pages:\n";
    result += data[0][3];
    ts = data[0][0];
    for (i = 1; i < sz; i++) {
	if (data[i][0] > ts + 5 * 60) {
	    result += "[" + time_difference(ts, data[i][0]) + "]\n";
	}
	ts = data[i][0];
	result += data[i][3];
    }
    if (time() > ts + 5 * 60) {
	result += "[" + time_difference(ts, time()) +"]\n";
    }
    user->message(result);
}

/* chatline stuff */


void cmd_DEV_broadcast( object user, object body, mixed str ) {
    int    i, sz;
    string verb, text;
    object *users;

    if (is_root(user->query_name())) {
	verb = "+broadcast";
    } else {
	verb = "!broadcast";
    }

    if (!str || !str["evoke"]) {
        user->message("Usage: " + verb + " \"text\"\n");
        return;
    }
    text = str["evoke"];
    users = INFOD->query_current_user_objects();
    sz = sizeof(users);
    for (i = 0; i < sz; i++) {
        users[i]->paragraph("[Broadcast] " + text + "\n" );
    }
}

private int
check_blocked(object body, string line)
{
    return CHATLINES->check_blocked(body, line);
}

private void
set_blocked(object body, string line, int flag)
{
    body->set_property("chatline:" + line + ":blocked", !!flag);
}

void
cmd_DEV_sh_chat(object user, object body, int override, mapping map)
{
    string str, secured;

    str = map["evoke"];
    if (!str || strlen(str) <= 2) {
	user->html_message("<PRE>" +
			   replace_html("Syntax: +sh \"message\"\n" +
					"    or: +sh all \"message\"' to send to all StoryHosts\n" +
			                "    or: +sh \"off\n" +
					"        +sh off              to turn off the StoryHost line\n" +
			                "    or: +sh \"on\n" +
					"        +sh on               to turn it back on\n" +
					"    or: +sh who              to see who's listening\n" +
					"    or: +sh recall           to check the last few chats\n" +
					"    or: +sh secure           to secure your current body name.\n") +
			   "</PRE>\n");
	return;
    }

    switch (str[1 .. strlen(str) - 2]) {
    case "on":
       this_object()->cmd_DEV_sh_set(user, body, TRUE);
       return;
    case "off":
       this_object()->cmd_DEV_sh_set(user, body, FALSE);
       return;
    default:
       break;
    }
    if (secured = user->query_udat()->query_property("skotos:sh:secured")) {
       CHATLINES->local_chat("sh", secured, override, str);
    } else {
       CHATLINES->local_chat("sh", capitalize(describe_one(body)), override, str);
    }
    if (check_blocked(body, "sh")) {
	user->message("Message sent. Note: you have the StoryHost line turned off.\n");
    }
}

void
cmd_DEV_sh_set(object user, object body, int flag)
{
    if (flag) {
	/* Turn line on: */
	if (!check_blocked(body, "sh")) {
	    user->message("You're already listening to the StoryHost line.\n");
	    return;
	}
	set_blocked(body, "sh", FALSE);
	user->message("StoryHost line turned ON.\n");
    } else {
	if (check_blocked(body, "sh")) {
	    user->message("You're not listening to the StoryHost line.\n");
	    return;
	}
	set_blocked(body, "sh", TRUE);
	user->message("StoryHost line turned OFF.\n");
    }
}

void
cmd_DEV_sh_who(object user, object body)
{
    CHATLINES->query_wholist("sh", user);
}

void
cmd_DEV_sh_recall(object user, object body)
{
    int sz;
    mixed **list;

    list = CHATLINES->query_recall("sh");
    sz = sizeof(list);
    if (sz) {
	int i;
	string *lines;

	lines = allocate(sz);
	for (i = 0; i < sz; i++) {
	    lines[i] = smalltime(list[i][0]) + " " + list[i][1];
	}
	user->message(TAG("The last few entries on the StoryHost line:\n" + 
			  implode(lines, ""), "Hosts"));
    }
}

void
cmd_DEV_sh_secure(object user, object body)
{
   string secured;

   secured = capitalize(describe_one(body));
   user->query_udat()->set_property("skotos:sh:secured", secured);
   user->message("You will now be known as " + secured + " on the Hosts line, regardless of which character you are using.  " +
		 "Please remember that this only applies to the Hosts line, and does not affect any other chat channels.\n");
}

void
cmd_DEV_assist_chat(object user, object body, mapping map)
{
    string str;

    str = map["evoke"];
    if (!str || strlen(str) <= 2) {
       /*** XXX: Hack alert. ***/
       "~/cmd/assist"->cmd_DEV_assist(user, body);
       return;
    }
    CHATLINES->local_chat("assist", describe_one(body), FALSE,
			  map["evoke"]);
    if (check_blocked(body, "assist")) {
	user->message("Message sent. Note: you have the Assist-line turned off.\n");
    }
}

void
cmd_DEV_assist_who(object user, object body)
{
    CHATLINES->query_wholist("assist", user);
}

void
cmd_DEV_assist_recall(object user, object body)
{
    int sz;
    mixed **list;

    list = CHATLINES->query_recall("assist");
    sz = sizeof(list);
    if (sz) {
	int i;
	string *lines;

	lines = allocate(sz);
	for (i = 0; i < sz; i++) {
	    lines[i] = smalltime(list[i][0]) + " " + list[i][1];
	}
	user->message(TAG("The last few entries on the Assist line:\n" + 
			  implode(lines, ""),
			  "Assist"));
    }
}

void
cmd_GUIDE_assist_chat(object user, object body, mapping map)
{
    cmd_DEV_assist_chat(user, body, map);
}

void
cmd_GUIDE_assist_who(object user, object body)
{
    cmd_DEV_assist_who(user, body);
}

void
cmd_GUIDE_assist_recall(object user, object body)
{
    cmd_DEV_assist_recall(user, body);
}

void
cmd_DEV_sp_chat(object user, object body, int override, mapping map)
{
    int    i, sz;
    string str;
    object *users, obj;

    str = map["evoke"];
    if (!str || strlen(str) <= 2) {
	user->html_message("<PRE>" +
			   replace_html("Syntax: +sp \"message\"\n" +
					"    or: +sp all \"message\"' to send to all StoryPlotters\n" +
					"    or: +sp off              to turn off the StoryPlotter line\n" +
					"    or: +sp on               to turn it back on\n" +
					"    or: +sp who              to see who's listening\n" +
					"    or: +sp recall           to check the last few chats") +
			   "</PRE>\n");
	return;
    }

    CHATLINES->local_chat("sp", user->query_name(), override, str);

    if (check_blocked(body, "sp")) {
	user->message("Message sent. Note: you have the StoryPlotter line turned off.\n");
    }
}

void
cmd_DEV_sp_set(object user, object body, int flag)
{
    if (flag) {
	/* Turn line on: */
	if (!check_blocked(body, "sp")) {
	    user->message("You're already listening to the StoryPlotter line.\n");
	    return;
	}
	set_blocked(body, "sp", FALSE);
	user->message("StoryPlotter line turned ON.\n");
    } else {
	if (check_blocked(body, "sp")) {
	    user->message("You're not listening to the StoryPlotter line.\n");
	    return;
	}
	set_blocked(body, "sp", TRUE);
	user->message("StoryPlotter line turned OFF.\n");
    }
}

void
cmd_DEV_sp_who(object user, object body)
{
    CHATLINES->query_wholist("sp", user);
}

void
cmd_DEV_sp_recall(object user, object body)
{
    int sz;
    mixed **list;

    list = CHATLINES->query_recall("sp");
    sz = sizeof(list);
    if (sz) {
	int i;
	string *lines;

	lines = allocate(sz);
	for (i = 0; i < sz; i++) {
	    lines[i] = smalltime(list[i][0]) + " " + list[i][1];
	}
	user->message(TAG("The last few entries on the StoryPlotter line:\n" + 
			  implode(lines, ""),
			  "Plotters"));
    }
}

void
cmd_DEV_lg_chat(object user, object body, int override, mapping map)
{
    int    i, sz;
    string str;
    object *users, obj;

    str = map["evoke"];
    if (!str || strlen(str) <= 2) {
	user->html_message("<PRE>" + replace_html(
	   "Syntax: +lg \"message\"\n" +
	   "    or: +lg all \"message\"'   to send to all LocalGame listeners\n" +
	   "    or: +lg off              to turn off the LocalGame line\n" +
	   "    or: +lg on               to turn it back on\n" +
	   "    or: +lg who              to see who's listening\n" +
	   "    or: +lg recall           to check the last few chats") +
	   "</PRE>\n");
	return;
    }

    CHATLINES->local_chat("lg", user->query_name(), override, str);

    if (check_blocked(body, "lg")) {
	user->message("Message sent. Note: you have the LocalGame line turned off.\n");
    }
}

void
cmd_DEV_lg_set(object user, object body, int flag)
{
    if (flag) {
	/* Turn line on: */
	if (!check_blocked(body, "lg")) {
	    user->message("You're already listening to the LocalGame line.\n");
	    return;
	}
	set_blocked(body, "lg", FALSE);
	user->message("LocalGame line turned ON.\n");
    } else {
	if (check_blocked(body, "lg")) {
	    user->message("You're not listening to the LocalGame line.\n");
	    return;
	}
	set_blocked(body, "lg", TRUE);
	user->message("LocalGame line turned OFF.\n");
    }
}

void
cmd_DEV_lg_who(object user, object body)
{
    CHATLINES->query_wholist("lg", user);
}

void
cmd_DEV_lg_recall(object user, object body)
{
    int sz;
    mixed **list;

    list = CHATLINES->query_recall("lg");
    sz = sizeof(list);
    if (sz) {
	int i;
	string *lines;

	lines = allocate(sz);
	for (i = 0; i < sz; i++) {
	    lines[i] = smalltime(list[i][0]) + " " + list[i][1];
	}
	user->message(TAG("The last few entries on the LocalGame line:\n" + 
			  implode(lines, ""),
			  "LocalGame"));
    }
}

void
cmd_DEV_cc_chat(object user, object body, mapping map)
{
    string str;

    str = map["evoke"];
    if (!str || strlen(str) <= 2) {
	user->html_message("<PRE>" +
replace_html("Syntax: +cc \"message\"\n" +
	     "    or: +cc off              to turn off the Coders Channel\n" +
	     "    or: +cc on               to turn it back on\n" +
	     "    or: +cc who              to see who's listening\n" +
	     "    or: +cc recall           to check the last few chats") +
			   "</PRE>\n");
	return;
    }

    CHATLINES->local_chat("cc", user->query_name(), FALSE, str);

    if (check_blocked(body, "cc")) {
	user->message("Message sent. Note: you have the Coders Channel turned off.\n");
    }
}

void
cmd_DEV_cc_set(object user, object body, int flag)
{
    if (flag) {
	/* Turn line on: */
	if (!check_blocked(body, "cc")) {
	    user->message("You're already listening to the Coders Channel.\n");
	    return;
	}
	set_blocked(body, "cc", FALSE);
	user->message("Coders Channel turned ON.\n");
    } else {
	if (check_blocked(body, "cc")) {
	    user->message("You're not listening to the Coders Channel.\n");
	    return;
	}
	set_blocked(body, "cc", TRUE);
	user->message("Coders Channel turned OFF.\n");
    }
}

void
cmd_DEV_cc_who(object user, object body)
{
    CHATLINES->query_wholist("cc", user);
}

void
cmd_DEV_cc_recall(object user, object body)
{
    int sz;
    mixed **list;

    list = CHATLINES->query_recall("cc");
    sz = sizeof(list);
    if (sz) {
	int i;
	string *lines;

	lines = allocate(sz);
	for (i = 0; i < sz; i++) {
	    lines[i] = smalltime(list[i][0]) + " " + list[i][1];
	}
	user->message(TAG("The last few entries on the Coders Channel:\n" + 
			  implode(lines, ""), "CC"));
    }
}

void
cmd_DEV_chatlines(object user, object body)
{
   int     i, sz;
   string  *names, *states, result;
   mapping lines;

   lines = CHATLINES->query_chatlines(user, body);
   sz = map_sizeof(lines);
   names = map_indices(lines);
   states = map_values(lines);
   result = "The state of your chatlines:\n";
   for (i = 0; i < sz; i++) {
      result += names[i] + ": " + capitalize(states[i]) + "\n";
   }
   user->message(result);
}

void cmd_GUIDE_busy(object user, object body, mixed args...) {
    if (check_storyguide_body(user, body)) {
	return;
    }
    this_object()->cmd_DEV_busy(user, body, args...);
}

void cmd_GUIDE_broadcast(object user, object body, mixed args...) {
    if (check_storyguide_body(user, body)) {
	return;
    }
    this_object()->cmd_DEV_broadcast(user, body, args...);
}
