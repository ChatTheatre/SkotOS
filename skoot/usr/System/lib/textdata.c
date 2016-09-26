# include <nref.h>
# include <System.h>
# include <type.h>

# include "./name.c"
# include "./string.c"
# include "./array.c"

private mixed recurse_through_result(mixed el, string callback);

static
mixed ascii_to_mixed(string val, varargs string objref_callback) {
   return recurse_through_result(::call_other(SYS_TEXTDATA,
					      "ascii_to_private",
					      val),
				 objref_callback);
}

private
mixed recurse_through_result(mixed el, string callback) {
   switch(typeof(el)) {
   case T_STRING:
   case T_INT:
   case T_FLOAT:
   case T_NIL:
      return el;
   case T_ARRAY: {
      int i;

      for (i = 0; i < sizeof(el); i ++) {
	 el[i] = recurse_through_result(el[i], callback);
      }
      return el;
   }
   case T_MAPPING: {
      mixed *ix, tmp;
      int i;

      ix = map_indices(el);

      for (i = 0; i < sizeof(ix); i ++) {
	 tmp = recurse_through_result(el[ix[i]], callback);
	 el[ix[i]] = nil;
	 el[recurse_through_result(ix[i], callback)] = tmp;
      }
      return el;
   }
   case T_OBJECT:
      if (::object_name(el) == "/usr/System/data/badobjref#-1") {
	 if (callback) {
	    return ::call_other(::this_object(), callback,
				::call_other(el, "query_ref"));
	 }

	 if (el->query_ref()) {
	    DEBUG("Warning: unknown object: " + el->query_ref());
	 } else {
	    DEBUG("Warning: unknown object: NIL");
	 }

	 {
	    string context;
	    context = TLSD->query_tls_value("System", "vault-context");

	    if (context) {
	       SysLog("vault context: " + context);
	    }
	 }

	 return nil;
      }
      return el;
   }
}

static
string mixed_to_ascii(mixed value, varargs int verbose) {
   return ::call_other(SYS_TEXTDATA,
		       "private_to_ascii",
		       value,
		       verbose,
		       ([ ]));
}
