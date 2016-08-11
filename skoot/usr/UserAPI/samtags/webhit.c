int tag(mixed content, object context, mapping local, mapping args) {
    object udat;

    if (!context) {
	error("internal error: No context");
    }
    udat = context->query_udat();
    if (!udat) {
	error("internal error: no udat");
    }
    if (!context->query_user()) {
	error("internal error: no context connection");
    }
    if (!context->query_user()->query_arguments_secure()) {
	error("internal error: arguments not secure");
    }
    args += local;
    udat->web_hit(args["theatre"]);
    return TRUE;
}
