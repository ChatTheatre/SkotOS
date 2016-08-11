/*
**	/base/lib/crafting.c
**
**	Data and logic for making things out of other things.
**
**	Copyright Skotos Tech Inc 1999-2000
*/

# include <base.h>

private inherit "/lib/string";
private inherit "/base/lib/urcalls";
private inherit "/base/lib/toolbox";

inherit "/lib/properties";
inherit "/lib/womble";

# include "/base/include/events.h"
# include "/base/include/motion.h"

/* static configuration */

private string	*ingredients;	/* ingredient properties required to make us */
private mapping	ing_amount;	/* how much of each ingredient? */
private mapping ing_desc;	/* how are the descriptions described? */

private string	see_skill;	/* skill you need to know of this recipe */
private int	see_level;	/* level skill must be at */

private string	do_skill;	/* skill you need to execute this recipe */
private int	do_level;	/* level skill must be at */

private string	*tools;		/* tool properties required to make us */
private mapping	tool_desc;	/* how are the tools described? */

private string	*verbs;		/* verbs that can be used to make us */

private int	time;		/* how long does the crafting process take */

private object	intermediate;	/* half-baked loaf of bread */
private object  failure;	/* what we get if we fail the do_skill test */

private int	flags;		/* bitmap of flags */

# define FLAG_ATTENTION		0x01	/* do we need to be present? */
# define FLAG_HELD		0x02	/* do we hold it while we craft it? */

/* configuration API */

void clear_crafting();

static
void create() {
   clear_crafting();
}

# define Patch()	{ if (!ing_amount) patch_crafting(); }

void womble_crafting() {
  ing_desc  = womble(ing_desc);
  tool_desc = womble(tool_desc);
}

nomask atomic
void patch_crafting() {
   ing_amount = ([ ]);
   ing_desc = ([ ]);
   tool_desc = ([ ]);
}

nomask atomic
void patch_zell() {
   if (sizeof(ingredients) == 1 && ingredients[0] == "new") {
      clear_crafting();
   }
   if (sizeof(tools) == 1 && tools[0] == "new") {
      clear_crafting();
   }
}

atomic
void clear_crafting() {
   if (ingredients || tools) {
      CRAFTD->clear(this_object());
   }
   ingredients = ({ });
   ing_amount = ([ ]);
   ing_desc = ([ ]);

   tools = ({ });
   tool_desc = ([ ]);

   verbs = ({ });
}

string *query_ingredients() {
   return ingredients ? ingredients[..] : ({ });
}

float query_ingredient_amount(string ing) {
   mixed v;

   if (ing && strlen(ing)) {
      Patch();

      ing = lower_case(ing);
      if (v = ing_amount[ing]) {
	 return v;
      }
      return 1.0;
   }
}

string query_ingredient_description(string ing) {
   if (ing && strlen(ing)) {
      Patch();

      ing = lower_case(ing);
      return ing_desc[ing];
   }
}

atomic
void add_ingredient(string str, float amount, string desc) {
   if (str && strlen(str)) {
      Patch();

      str = lower_case(str);
      ingredients |= ({ str });
      ing_amount[str] = amount;
      ing_desc[str] = desc;

      CRAFTD->add_ingredient(this_object(), str);
   }
}

string *query_tools() {
   return tools ? tools[..] : ({ });
}

atomic
void add_tool(string str, string desc) {
   if (str && strlen(str)) {
      Patch();

      str = lower_case(str);
      tools |= ({ str });
      tool_desc[str] = desc;
      CRAFTD->add_tool(this_object(), str);
   }
}

string query_tool_description(string tool) {
   if (tool && strlen(tool)) {
      Patch();

      tool = lower_case(tool);
      return tool_desc[tool];
   }
}

string *query_crafting_verbs() {
   if (!verbs) verbs = ({ });
   return verbs[..];
}

atomic
void add_crafting_verb(string str) {
   if (!verbs) verbs = ({ });
   if (str && strlen(str)) {
      str = lower_case(str);
      verbs |= ({ str });
   }
}


string query_see_skill() {
   return see_skill;
}

int query_see_level() {
   return see_level;
}

string query_do_skill() {
   return do_skill;
}

int query_do_level() {
   return do_level;
}

void set_see_skill(string s) {
   see_skill = s;
}

void set_see_level(int n) {
   see_level = n;
}

void set_do_skill(string s) {
   do_skill = s;
}

void set_do_level(int n) {
   do_level = n;
}

int query_crafting_time() {
   return time;
}

void set_crafting_time(int i) {
   time = i;
}

object query_crafting_intermediate() {
   return intermediate;
}

void set_crafting_intermediate(object ob) {
   intermediate = ob;
}

object query_crafting_failure() {
   return failure;
}

void set_crafting_failure(object ob) {
   failure = ob;
}

# define SetFlag(f, b)	((b) ? (flags |= (f)) : (flags &= ~(f)))
# define GetFlag(f)	(!!(flags & (f)))

void set_crafting_attention(int b) {
   SetFlag(FLAG_ATTENTION, b);
}

int query_crafting_attention() {
   return GetFlag(FLAG_ATTENTION);
}

void set_crafting_held(int b) {
   SetFlag(FLAG_HELD, b);
}

int query_crafting_held() {
   return GetFlag(FLAG_HELD);
}
