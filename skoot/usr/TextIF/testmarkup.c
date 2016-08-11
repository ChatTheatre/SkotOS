# include <SkotOS.h>

inherit "lib/markup";

void
test()
{
   string in, out;

   in = PRE(TAG("One\n", "imp") +TAG("Two\n", "assist-alert") + TAG("Three\n", "imp"));
   out = expand_markup(in);
   DEBUG("in = " + dump_value(in));
   DEBUG("out = " + dump_value(out));
}

int
query_html_client()
{
   return TRUE;
}

string
get_colour(string tag)
{
   return "black";
}
