/*
**	/base/lib/act/crafting.c
**
**	Functionality for combining things into other things.
**
**	Copyright Skotos Tech Inc 1999-2000
*/

# include <base.h>
# include <base/thing.h>

private inherit "/base/lib/toolbox";
private inherit "/base/lib/urcalls";

# include "/base/include/act.h"
# include "/base/include/motion.h"
# include "/base/include/environment.h"
# include "/base/include/events.h"

# if 0
/*
**	Open questions [test object: bread = water + flour + butter]:
**
**		- What if I combine water and flour but not butter?
**		- What if I combine water, flour, butter and, uh, wood?
**
**	Commands:
**		bake bread
**		bake bread on stove
**		combine water, flour and butter
**		combine bread on stove using water, flour, butter
*/

atomic
void act_combine(object *stuff, object *tools) {
   mapping propmap, map;
   object *compositions, new, output;
   string *props;
   int i, j;

   /* go through the ingredients and see what they can make */

   propmap = ([ ]);
   compositions = ({ });

   for (i = 0; i < sizeof(stuff); i ++) {
      map = stuff[i]->query_properties();
      propmap += map;

      props = map_indices(map);

      for (j = 0; j < sizeof(props); j ++) {
	 compositions |= CRAFTD->query_objects_needing_ingredient(props[j]);
      }
   }

   /* go through everything they can make and see if we have ingredients */

   for (i = 0; i < sizeof(compositions); i ++) {
      props = compositions[i ++]->query_ingredients();
      if (map_sizeof(propmap & props) < sizeof(props)) {
	 compositions[i] = nil;
      }
   }
   compositions -= ({ nil });

   if (sizeof(compositions) == 0) {
      error("this stuff makes nothing");
   }

   propmap = ([ ]);

   for (i = 0; i < sizeof(tools); i ++) {
      propmap += tools->query_properties();
   }

   for (i = 0; i < sizeof(compositions); i ++) {
      props = compositions[i]->query_tools();

      if (map_sizeof(propmap & props) < sizeof(props)) {
	 compositions[i] = nil;
      }
   }
   if (sizeof(compositions) == 0) {
      error("you're missing some tools");
   }

   if (sizeof(compositions) > 1) {
      error("too many possible combinations");
   }

   new = spawn_thing(compositions[i]);

   if (new->move(this_object()) < 0) {
      if (!query_environment() ||
	  new->move(this_object()->query_environment()) < 0) {
	 error("couldn't put it inside you or your environment");
      }
      /* you dropped it! */
   }
   /* now what is consumed? */
   props = compositions[0]->query_ingredients();
   for (i = 0; i < sizeof(stuff); i ++) {
      map = stuff[i]->query_properties();
      
      if (map_sizeof(map & props) > 0) {
	 props -= map_indices(map);
      } else {
	 stuff[i] = nil;
      }
   }
   stuff -= ({ nil });

   New_Output_Object();
   Set_Output("Output:Combine", "who", this_object());
   Set_Output("Output:Combine", "new", new);
   Set_Output("Output:Combine", "stuff", stuff);
   Set_Output("Output:Combine", "tools", tools);
   Set_Output_Actors(this_object(), nil,
		     query_environment()->query_inventory() -
		     ({ this_object() }));
   broadcast("output", output);

   for (i = 0; i < sizeof(stuff); i ++) {
      stuff[i]->suicide();
   }
}

atomic
void act_make(object thing, string verb1, string verb3, string adverb) {
   mapping map, partial, ing_amts, ing_refs;
   string *iprops, *tprops, *props;
   object *istuff, *estuff, *tools, *arr;
   object new, output, error;
   int i, j;

   if (!sizeof(thing->query_verbs())) {
      New_Error_Object();
      Set_Error("Error:Crafting:NotCraftable", "item", thing);
      broadcast("error", error);
      return;
   }

   istuff = query_inventory();
   estuff = query_environment()->query_inventory();

   tprops = thing->query_tools();
   tools = ({ });

   /* tools: search environment */
   for (i = 0; i < sizeof(estuff); i ++) {
      map = estuff[i]->query_properties();

      partial = map & tprops;
      if (map_sizeof(partial) > 0) {
	 tprops -= map_indices(partial);
	 tools += estuff[i .. i];
      } 
   }

   iprops = thing->query_ingredients();
   ing_amts = ([ ]);
   ing_refs = ([ ]);

   /* tools and ingredients: search inventory */
   for (i = 0; i < sizeof(istuff); i ++) {
      map = istuff[i]->query_properties();

      partial = map & tprops;
      if (map_sizeof(partial) > 0) {
	 tprops -= map_indices(partial);
	 tools += istuff[i .. i];
      }
      partial = map & iprops;
      if (map_sizeof(partial) > 0) {
	 float amount;

	 props = map_indices(partial);

	 iprops -= props[0 .. 0];	/* only deal with first */

	 amount = thing->query_ingredient_amount(props[0]);

	 if (istuff[i]->query_amount() < amount) {
	    /* don't have enough */
	    New_Error_Object();
	    Set_Error("Error:Crafting:Ingredients",
		      "item", thing);
	    Set_Error("Error:Crafting:Ingredients",
		      "desc", thing->query_ingredient_description(props[0]));
	    Set_Error("Error:Crafting:Ingredients",
		      "more", TRUE);
	    broadcast("error", error);
	    return;
	 }
	 ing_amts[istuff[i]] = amount;
	 ing_refs[istuff[i]] =
	    MutateNRef(istuff[i]->prime_reference(this_object()), amount);
      }
   }

   if (sizeof(tprops) > 0) {
      New_Error_Object();
      Set_Error("Error:Crafting:Tools",
		"item", thing);
      Set_Error("Error:Crafting:Tools",
		"desc", thing->query_tool_description(tprops[0]));
      broadcast("error", error);
      return;
   }
   if (sizeof(iprops) > 0) {
      New_Error_Object();
      Set_Error("Error:Crafting:Ingredients",
		"item", thing);
      Set_Error("Error:Crafting:Ingredients",
		"desc", thing->query_ingredient_description(iprops[0]));
      broadcast("error", error);
      return;
   }

   /* let script code have its say */

   if (!run_signal("combine", thing->prime_reference(this_object()))) {
      return;
   }

   /* OK, do it */

   new = spawn_thing(thing);

   if (new->move(this_object()) < 0) {
      if (!query_environment() ||
	  new->move(this_object()->query_environment()) < 0) {
	 new->suicide();
	 New_Error_Object();
	 Set_Error("Error:Crafting:MoveFail", "item", thing);
	 broadcast("error", error);
	 return;
      }
      /* you dropped it! */
   }
   /* now what is consumed? */

   New_Output_Object();
   Set_Output("Output:Combine", "who", this_object());
   Set_Output("Output:Combine", "new", new);
   Set_Output("Output:Combine", "verb1", verb1);
   Set_Output("Output:Combine", "verb3", verb3);
   Set_Output("Output:Combine", "adverb", adverb ? adverb : "");
   Set_Output("Output:Combine", "stuff", map_values(ing_refs));
   Set_Output("Output:Combine", "tools", tools);
   Set_Output_Actors(this_object(), nil,
		     query_environment()->query_inventory() -
		     ({ this_object() }));
   broadcast("output", output);

   arr = map_indices(ing_amts);
   for (i = 0; i < sizeof(arr); i ++) {
      arr[i]->decrease_amount(ing_amts[arr[i]]);
   }
}

# endif
