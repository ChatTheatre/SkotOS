/*
 * ~Tool/lib/config.c
 *
 * Support for configurable parts.
 *
 * Copyright Skotos Tech Inc 2003.
 */

private inherit "/lib/mapping";

mapping query_defaults();

void
cmd_config_list(object user, object body)
{
   int     i, sz;
   mapping defaults;
   string  text, *props;
   mixed   *values;
   object  udat;

   udat = user->query_udat();
   text = "Known configuration settings:\n";
   defaults = query_defaults();
   sz       = map_sizeof(defaults);
   props    = map_indices(defaults);
   values   = map_values(defaults);
   for (i = 0; i < sz; i++) {
      mixed value;

      value = udat->query_property("tool:config:" + props[i]);
      if (value == nil) {
	 text += props[i] + ": <not set> (" + values[i] + ")\n";
      } else {
	 text += props[i] + ": " + value + " (" + values[i] + ")\n";
      }
   }
   user->message(text);
}

void
cmd_config_get(object user, object body, string param)
{
   mixed value;
   mapping map;

   map = query_defaults();
   if (map[param] == nil) {
      user->message("+tool config get: No such configuration setting known.\n");
      return;
   }
   value = user->query_udat()->query_property("tool:config:" + param);
   if (value == nil) {
      user->message(param + ": <not set> (" + map[param] + ")\n");
   } else {
      user->message(param + ": " + value + " (" + map[param] + ")\n");
   }
}

void
cmd_config_set(object user, object body, string param, string value)
{
   mapping map;

   map = query_defaults();
   if (map[param] == nil) {
      user->message("+tool config set: No such configuration setting known.\n");
      return;
   }
   user->query_udat()->set_property("tool:config:" + param, value);
   value = user->query_udat()->query_property("tool:config:" + param);
   if (value == nil) {
      user->message(param + ": <not set> (" + map[param] + ")\n");
   } else {
      user->message(param + ": " + value + " (" + map[param] + ")\n");
   }
}

void
cmd_config_clear(object user, object body, string param)
{
   mapping map;

   map = query_defaults();
   if (map[param] == nil) {
      user->message("+tool config clear: No such configuration setting known.\n");
      return;
   }
   user->query_udat()->clear_property("tool:config:" + param);
   user->message("Cleared the " + param + " setting; default value is " + map[param] + ".\n");
}
