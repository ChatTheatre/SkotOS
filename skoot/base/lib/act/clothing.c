/*
**	/base/lib/act/clothing.c
**
**	Functionality for wearing/wielding items and such.
**
**	Copyright Skotos Tech Inc 1999-2007
*/

private inherit "/base/lib/urcalls";

# include "/base/include/act.h"
# include "/base/include/events.h"
# include "/base/include/clothing.h"
# include "/base/include/thing.h"
# include "/base/include/motion.h"
# include "/base/include/environment.h"
# include "/base/include/prox.h"

# include <base/bulk.h>
# include <base/living.h>
# include <base/act.h>
# include <base.h>

static
float action_wear(object result, string state, int flags, mapping args) {
   object *articles, *success, *failure, env, offered_to;
   int i;

   env = query_environment();

   articles = ArrArg("articles");

   success = ({ });
   failure = ({ });

   for(i = 0; i < sizeof(articles); i++) {
      if (!pre_signal("wear", args, ([ nil: articles[i] ]))) {
	 continue;
      }
      if (!sizeof(ur_parts_covered(articles[i]))) {
	 Add_Error("Error:Wear:NotWearable", "obs", articles[i]);
	 continue;
      }

      if (articles[i]->query_environment() != this_object()) {
	 Add_Error("Error:Wear:NotCarried", "obs", articles[i]);
	 continue;
      }

      if (is_wearing(articles[i])) {
	 if (ur_weapon(articles[i])) {
	    Add_Error("Error:Wield:AlreadyWielded", "obs", articles[i]);
	    continue;
	 }
	 Add_Error("Error:Wear:AlreadyWorn", "obs", articles[i]);
	 continue;
      }

      if (ur_weapon(articles[i])) {
	 Add_Error("Error:Wear:IsWeapon", "obs", articles[i]);
	 continue;
      }

      if (!new_signal("wear", args, ([ nil: articles[i] ]))) {
	 continue;
      }

      /* check if object is offered if so then revoke the offer */
      offered_to = articles[i]->query_offered_to();
      if (offered_to) {
	 insert_action("revoke", ([ "who": offered_to ]));
      }

      add_article(articles[i]);
      success += ({ articles[i] });
      post_signal("wear", args, ([ nil: articles[i] ]));
   }

   if (sizeof(success)) {
      result->new_output("Output:Wear", success, ([
	 "clothing": success
	 ]));
      return 0.0;
   }
   /* if nothing was successfully worn, cause error abort */
   return -1.0;
}

static
float action_wield(object result, string state, int flags, mapping args) {
   object me, offered_to, env, article;
   int success;

   env = query_environment();

   article = ObArg("article");

   if (!pre_signal("wield", args, ([ nil: article ]))) {
      return -1.0;
   }

   if (article->query_environment() != this_object()) {
      Add_Error("Error:Wield:NotCarried", "obs", article);
      return -1.0;
   }
   if (!ur_weapon(article)) {
      Add_Error("Error:Wield:NotWeapon", "obs", article);
      return -1.0;
   }
   if (is_wearing(article)) {
      Add_Error("Error:Wield:AlreadyWielded", "obs", article);
      return -1.0;
   }

   if (!new_signal("wield", args, ([ nil: article ]))) {
      return -1.0;
   }

   /* check if object is offered if so then revoke the offer */
   offered_to = article->query_offered_to();
   if (offered_to) {
      insert_action("revoke", ([ "who": offered_to ]));
   }

   add_article(article);
   this_object()->set_property("base:combat:weapon", article);

   post_signal("wield", args, ([ nil: article ]));

   result->new_output("Output:Wear", ({ article }), ([
      "weapons": article
      ]));
   return 0.0;
}

/* try to remove some clothing */

static
float action_unwear(object result, string state, int flags, mapping args) {
   object *success, *failure, env, *articles;
   int i, articles_size;

   env = query_environment();

   articles = ArrArg("articles");

   articles_size = sizeof(articles);
   success = ({ });
   failure = ({ });

   for(i = 0; i < articles_size; i++) {
      if (!pre_signal("unwear", args, ([ nil: articles[i] ]))) {
	 continue;
      }

      if (!(flags & ACTION_FORCED) && is_covered(articles[i])) {
	 Add_Error("Error:Unwear:IsCovered", "obs", articles[i]);
	 Set_Error("Error:Unwear:IsCovered", "covered", 
		   query_covered_by(articles[i]));
	 continue;
      }

      if (!is_wearing(articles[i])) {
	 Add_Error("Error:Unwear:NotWorn", "obs", articles[i]);
	 continue;
      }

      if (ur_weapon(articles[i])) {
	 Add_Error("Error:Unwear:IsWeapon", "obs", articles[i]);
	 continue;
      }

      if (!new_signal("unwear", args, ([ nil: articles[i] ]))) {
	 continue;
      }

      success += ({ articles[i] });
      unwear_article(articles[i]);

      post_signal("unwear", args, ([ nil: articles[i] ]));

      if (ur_single_wear(articles[i])) {
	 articles[i]->clear_parts_covered();
      }
   }

   if (sizeof(success)) {
      result->new_output("Output:Unwear", success);
      return 0.0;
   }
   return -1.0;
}

/* try to remove a weapon */
static
float action_unwield(object result, string stage, int flags, mapping args) {
   object env, article;

   env = query_environment();

   article = ObArg("article");

   if (!pre_signal("unwield", args, ([ nil: article ]))) {
      return -1.0;
   }

   if (!is_wearing(article)) {
      Add_Error("Error:Unwield:NotWielded", "obs", article);
      return -1.0;
   }

   if (!ur_weapon(article)) {
      Add_Error("Error:Unwield:Wearing", "obs", article);
      return -1.0;
   }

   if (!new_signal("unwield", args, ([ nil: article ]))) {
      return -1.0;
   }

   unwear_article(article);
   this_object()->set_property("base:combat:weapon", nil);

   /* Check if player is currently in a duel, if so: Submit defeat. */
   if (this_object()->query_opponent()) {
      DEBUG("unwield trigger end_duel");
      insert_action("end_duel",
		    ([ "who": ({ this_object(),
				 this_object()->query_opponent() }),
		       "loser": this_object() ]));
   }

   post_signal("unwield", args, ([ nil: article ]));

   if (ur_single_wear(article))
      article->clear_parts_covered();

   result->new_output("Output:Unwear", ({ article }));
}

/* this function will simply act as a router for the objects */
static
float action_remove(object result, string stage, int flags, mapping args) {
   object *clothes, *weapons, *articles;
   int i;

   articles = ArrArg("articles");

   clothes = ({ });
   weapons = ({ });

   for (i = 0; i < sizeof(articles); i++) {
      if (ur_weapon(articles[i])) {
	 weapons += ({ articles[i] });
      } else {
	 clothes += ({ articles[i] });
      }
   }

   if (sizeof(weapons)) {
      insert_action("unwield", ([ "article": weapons[0] ]), flags);
   }
   if (sizeof(clothes)) {
      insert_action("unwear", ([ "articles": clothes ]), flags);
   }
}
