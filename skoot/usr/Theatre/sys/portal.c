# include <System.h>
# include <HTTP.h>

inherit module "/lib/module";

static void create()
{
    module::create("HTTP");
}

static void HTTP_loaded() {
    HTTPD->claim_port(SYS_INITD->query_real_webport());
}

string query_http_root_url_document() {
    return "/Theatre/Theatres.sam";
}
