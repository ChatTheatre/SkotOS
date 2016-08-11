# include <type.h>
# include <mapargs.h>
# include <SkotOS.h>

# define Output(str) context->do_output(str)

# define AssertLocal(v, n)\
  { if (!local[v]) error("attribute " + (v) + " undefined for " + (n)); }
# define AssertArg(v, n)\
  { if (!args[v]) error("attribute " + (v) + " undefined for " + (n)); }

/* sam.c prototypes */
static string unsam(mixed xml, varargs mapping args, object user, object this);
static void sam(mixed xml, object target, varargs mapping args,
		object user, object this);
static void transform_to_html(mixed elts, object data, mapping args);
static mixed tf_elt(mixed elt, object context, mapping args);
static void call_recipients(object *tarr, string fun, mixed args...);
