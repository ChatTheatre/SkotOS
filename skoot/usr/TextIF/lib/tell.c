/*
**	/usr/TextIF/lib/tell.c
**
**	This file must be inherited by a program that inherits SAM.
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/base/lib/urcalls";

# include <SAM.h>
# include "/usr/SAM/include/sam.h"

static
void tell_to(object *users, string msg, object bodies...) {
   object *us;
   int i, sz;

   us = ({ });
   for (i = 0, sz = sizeof(bodies); i < sz; i ++) {
      us |= bodies[i]->query_souls();
   }
   us &= users;

   for (i = 0, sz = sizeof(us); i < sz; i ++) {
      us[i]->paragraph(msg);
   }
}

/*
 * Split up text into part leading up to first ", part from there until the
 * last ", and whatever comes after that.
 */
private string *
split_speech(string str)
{
    int i, sz;
    string pre, speech, post;

    sz = strlen(str);
    while (i < sz) {
	if (str[i] == '"') {
	    pre = str[..i - 1];
	    speech = str[i + 1..];
	    break;
	}
	i++;
    }
    if (!pre) {
	/* No " in the entire line. */
	return nil;
    }
    i = strlen(speech) - 1;
    while (i >= 0) {
	if (speech[i] == '"') {
	    post   = speech[i + 1..];
	    speech = speech[..i - 1];
	    break;
	}
	i--;
    }
    if (!post) {
	/* Must've been a single " in the line. */
	return nil;
    }
    return ({ pre, speech, post });
}

static 
void tell_each ( object *users, object who, string msg, object body) {
   int    i, sz;
   object *us;

   /* Pick out the relevant souls for this body. */
   us = body->query_souls() & users;

   /* Most likely one, but you never know. */
   for (i = 0, sz = sizeof(us); i < sz; i++) {
       string *split;

# if 0
       split  = split_speech(msg);
       if (split) {
	   string s1, speech, s2;

	   s1     = split[0];
	   speech = split[1];
	   s2     = split[2];
	   speech = LANG->do_garble(speech, who, body);
	   s1     = LANG->do_lang_name(s1, who, body);
	   us[i]->paragraph(s1 + "\"" + speech + "\"" + s2);
       } else {
	   us[i]->paragraph(msg);
       }
# else
       us[i]->paragraph(msg);
# endif
   }
}

static
void tell_all_but(object *users, string msg, object except...) {
    int i, sz;

    for (i = 0, sz = sizeof(except); i < sz; i ++) {
	users -= except[i]->query_souls();
    }

    for (i = 0, sz = sizeof(users); i < sz; i ++) {
	users[i]->paragraph(msg);
    }
}

static
void tell(object *users, string first, string third, object bodies...) {
    tell_to(users, first, bodies...);
    tell_all_but(users, third, bodies...);
}

static
void general_sam(object *users, object actor, object *lookers, SAM what,
		 varargs mapping args, int public) {
   object *souls, *remains;
   int i, sz_i, j, sz_j;

   if (!args) {
      args = ([ ]);
   }
   args["actor"] = args["who"] = actor;
   remains = users;
   if (lookers) {
      for (i = 0, sz_i = sizeof(lookers); i < sz_i; i++) {
	 args["looker"] = lookers[i];
	 souls = lookers[i]->query_souls();
	 if (users) {
	    souls &= users;
	    remains -= souls;
	 }
	 for (j = 0, sz_j = sizeof(souls); j < sz_j; j++) {
	    sam(what, souls[j], args, souls[j]);
	 }
      }
   }
   if (users && public) {
      /* remains holds watching souls that are not in *lookers */
      for (j = 0, sz_j = sizeof(remains); j < sz_j; j++) {
	 sam(what, remains[j], args, remains[j]);
      }
   }
}
