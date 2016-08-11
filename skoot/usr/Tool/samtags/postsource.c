# include <type.h>

private inherit "/lib/string";

int
tag(mixed content, object context, mapping local, mapping args)
{
   string id, script, text, cb_fun;
   object target, udat, cb_obj;
   mixed  *extra;

   udat = context->query_udat();
   if (!udat) {
      return TRUE;
   }

   args += local;

   id   = args["id"];
   text = args["source"];
   if (!id) {
      return TRUE;
   }

   cb_obj  = udat->query_property("source-" + id + "-cb-obj");
   cb_fun  = udat->query_property("source-" + id + "-cb-fun");
   target  = udat->query_property("source-" + id + "-target");
   script  = udat->query_property("source-" + id + "-script");
   extra   = udat->query_property("source-" + id + "-extra");

   if (id && text && cb_obj && cb_fun) {
      string new_text;
      mixed  data;

      data = call_other(cb_obj, cb_fun, target, script, text, extra...);
      if (typeof(data) == T_STRING) {
	 new_text = data;
	 udat->clear_property("source-" + id + "-info");
      } else {
	 /* Structure returns both compile errors and (the original) text. */
	 udat->set_property("source-" + id + "-info", data[0]);
	 new_text = data[1];
      }
      udat->set_property("source-" + id + "-text", replace_html(new_text));
      udat->set_property("source-" + id + "-stamp", time());
   }
   return TRUE;
}
