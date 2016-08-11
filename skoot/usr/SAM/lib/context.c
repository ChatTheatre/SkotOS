/*
**	/usr/SAM/lib/context.c
**
**	The first version of this just conveniently packages
**	a lot of information to be send around all the various
**	bits of the SAM system.
**
**	010131 Zell	Initial Revision
**
**	Copyright Skotos Tech Inc 1999-2001
*/

# include <SAM.h>

private inherit "/lib/string";

private object  user;	/* originating connection */
private object	udat;	/* associated udat */
private object	this;	/* object containing the SAM, if any */
private object *targets;	/* objects to transmit to */
private int render;	/* are we rendering to ascii? */
private int html;	/* are we rendering to HTML? */
private string pending_space;	/* what space is pending? */

static
void create() {
   pending_space = "";
   targets = ({ });
}

void set_targets(object obj...) {
   targets = obj;
}

object *query_targets() {
   return targets;
}

void set_user(object obj) {
   if (user = obj) {
      udat = user->query_udat();
   }
}

object query_user() {
   return user;
}
object query_udat() {
   return udat;
}

void set_this(object obj) {
   this = obj;
}

object query_this() {
   return this;
}


void set_render(int r) {
   render = r;
}

int query_render() {
   return render;
}

void set_html(int r) {
   html = r;
}

int query_html() {
   return html;
}

void add_pending_space(string str) {
   if (pending_space) {
      pending_space += str;
   } else {
      pending_space = str;
   }
}

void flush_output() {
   object *tarr;
   string str, out;
   int i;

   if (str = pending_space) {
      if (strlen(str) == 0) {
	 return;
      }
      str = strip(str, TRUE);

      out = "";
      while (strlen(str)) {
	 out += "\n";
	 str = strip_left(str[1 ..], TRUE);
      }
      if (!strlen(out)) {
	 out = " ";
      }
      tarr = query_targets();
      for (i = 0; i < sizeof(tarr); i ++) {
	 catch {
	    tarr[i]->append_string(out);
	 }
      }
      pending_space = nil;
   }      
}

void do_output(string str) {
   object *tarr;
   int i;

   if (!strlen(str)) {
      return;
   }

   if (!html) {
      if (str == "\n" || !strlen(strip(str))) {
	 add_pending_space(str);
	 return;
      }
      flush_output();
   }
   tarr = query_targets();
   for (i = 0; i < sizeof(tarr); i ++) {
      catch {
	 tarr[i]->append_string(str);
      }
   }
}
