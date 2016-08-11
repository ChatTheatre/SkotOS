static string describe_one(NRef ref, varargs mixed doer,
			   object looker, int style, int cardinal);
static string describe_many(NRef *nrefs, varargs mixed doer,
			    object looker, int style);
mixed **describe_multi(NRef *nrefs, object actor, varargs int style);
static string describe_clothes(mapping clothes_worn, object looker,
			       int style, int clothing);
static string describe_exits(NRef *exits);

static string describe_item(NRef what, object looker, string types...);
static string determine_prep(object *what);
static string *cluster_briefs(NRef *nrefs, object actor,
			      object looker, int style);
