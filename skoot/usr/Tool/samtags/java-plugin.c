/*
 * ~Tool/samtags/java-plugin.c
 *
 * This SAMtag should save us the trouble of creating and maintaining the ugly
 * nested OBJECT + EMBED tags, which means always making sure you keep the
 * appropriate parameters synchronized between the two versions, this aside
 * from the fact that it's just plain annoying to have to do this.
 *
 * Copyright Skotos Tech Inc 2002
 */

inherit "/usr/SAM/lib/sam";
inherit "/lib/url";

# include "/usr/SAM/include/sam.h"

int tag(mixed content, object context, mapping local, mapping args)
{
   int    i, sz;
   string *params, browser;
   mixed  *values;

   local = local[..];

   browser = local["browser"]; local["browser"] = nil;
   if (!browser) {
      string agent;

      agent = context->query_user()->query_header("user-agent");
      if (context->query_user()->query_macintosh()) {
	 /* Both Netscape and IE require applet, it looks like. */
	 browser = "applet";
      } else if (sscanf(agent, "%*sOpera %*d.%*d") == 3) {
	 /* Both applet and embed work for Opera on Linux */
	 browser = "applet";
      } else if (context->query_user()->query_mozilla()) {
	 /* Confirmed to work for Mozilla on Linux. */
	 browser = "applet";
      } else if (context->query_user()->query_netscape()) {
	 /* Netscape likes being the odd one out. */
	 browser = "embed";
      } else {
	 /* As does IE, currently . */
	 browser = "object";
      }
   }

   if (!local["cookie"]) {
      local["cookie"] = context->query_user()->query_header("cookie");
   }
   DEBUG("browser = " + dump_value(browser) + "; user-agent = " +
	 dump_value(context->query_user()->query_header("user-agent")));

   switch (browser) {
   case "embed":
      if (!local["type"]) {
	 local["type"] = "application/x-java-applet;version=1.3";
      }

      sz     = map_sizeof(local);
      params = map_indices(local);
      values = map_values(local);

      Output("<EMBED pluginspage=\"http://java.sun.com/products/plugin/1.3/plugin-install.html\"");
      for (i = 0; i < sz; i++) {
	 Output(" " + params[i] + "=\"" + values[i] + "\"");
      }
      Output(">\n");
      Output("</EMBED>\n");
      break;
   case "applet": {
      string width, height, codebase, code, archive;

      width    = local["width"];    local["width"]    = nil;
      height   = local["height"];   local["height"]   = nil;
      codebase = local["codebase"]; local["codebase"] = nil;
      code     = local["code"];     local["code"]     = nil;
      archive  = local["archive"];  local["archive"]  = nil;

      sz     = map_sizeof(local);
      params = map_indices(local);
      values = map_values(local);

      Output("<APPLET" +
	     (width    ? " width=\""    + width    + "\"" : "") +
	     (height   ? " height=\""   + height   + "\"" : "") +
	     (codebase ? " codebase=\"" + codebase + "\"" : "") +
	     (code     ? " code=\""     + code     + "\"" : "") +
	     (archive  ? " archive=\""  + archive  + "\"" : "") +
	     ">\n");
      for (i = 0; i < sz; i++) {
	 Output("  <PARAM name=\"" + params[i] + "\" value=\"" + values[i] +
		"\">\n");
      }
      Output("</APPLET>\n");
      break;
   }
   default: {
      /* object for IE on PCs*/
      string height, width;

      height = local["height"]; local["height"] = nil;
      width  = local["width"];  local["width"] = nil;
      if (!local["type"]) {
	 local["type"] = "application/x-java-applet;version=1.3";
      }

      sz     = map_sizeof(local);
      params = map_indices(local);
      values = map_values(local);
      Output("<OBJECT " +
	     "classid=\"clsid:8AD9C840-044E-11D1-B3E9-00805F499D93\" " +
	     (width  ? "width=\""  + width  + "\" " : "") +
	     (height ? "height=\"" + height + "\" " : "") +
	     "codebase=\"http://java.sun.com/products/plugin/1.3/jinstall-13-win32.cab#Version=1,3,0,0\">\r\n");
      sz     = map_sizeof(local);
      params = map_indices(local);
      values = map_values(local);
      for (i = 0; i < sz; i++) {
	 Output("  <PARAM name=\"" + params[i] +
		"\" value=\"" + values[i] + "\">\r\n");
      }
      Output("</OBJECT>\r\n");
      break;
   }
   }
   return TRUE;
}
