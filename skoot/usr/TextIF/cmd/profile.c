# include <type.h>
# include <TextIF.h>
# include <SkotOS.h>
# include <UserAPI.h>

private inherit "/lib/string";
private inherit "/lib/array";
private inherit "/lib/date";

inherit "/usr/SkotOS/lib/describe";
inherit "/usr/TextIF/lib/misc";

string *query_themes(object udat, object body) {
   mapping themes;
   string *props, theme;
   int i;

   themes = ([ ]);

   props = map_indices(THEMES->query_properties());

   for (i = 0; i < sizeof(props); i ++) {
      if (sscanf(props[i], "colours:%s:", theme)) {
	 themes[theme] = TRUE;
      }
   }
   return map_indices(themes);
}


void cmd_profile(object user, object body, varargs mixed args...) {
   if (sizeof(args) > 0) {
      switch(lower_case(args[0])) {
      case "theme": {
	 string theme;

	 if (sizeof(args) == 1) {
	    if (body->query_property("client:theme")) {
	       user->paragraph("Your current theme profile is '" +
			       body->query_property("client:theme") + "'.");
	       return;
	    }
	    user->paragraph("You have no current theme profile.");
	    return;
	 }
	 if (typeof(args[1]) == T_STRING) {
	    switch(args[1]) {
	    case "none": case "clear": case "default":
	       user->paragraph("Your client theme profile is now being reset to the default!");
	       body->clear_property("client:theme");
	       user->initialize_theme();
	       return;
	    }
	    error("can't happen");
	 }
	 theme = lower_case(implode(args[1], " "));
	 if (!member(theme, query_themes(user->query_udat(), body))) {
	    user->paragraph("There is no such theme!");
	    return;
	 }
	 user->paragraph("Your client theme profile is now being switched to '" + theme + "'.");
	 body->set_property("client:theme", theme);
	 user->initialize_theme();
	 return;
      }
      return;
      case "themes":
	 user->paragraph("Current themes: " +
			 implode(query_themes(user->query_udat(), body), " ") +
			 ".");
	 return;
      }
   }
   user->html_message("Usage: @profile themes\n" +
		      "Usage: @profile theme\n" +
		      "Usage: @profile theme &lt;theme&gt;\n" +
		      "Usage: @profile theme none\n");
}
