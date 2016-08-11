/*
**	/base/lib/living/stance.c
**
**	This code handles the stance of a living thing.
**
**	Copyright Skotos Tech Inc 1999
**
*/

# include <base.h>
# include <base/living.h>
# include <base/preps.h>

/* transient data */

private int	stance;
private int	preposition;
private string	gait;

/* configurable data */

private int	default_stance;

void set_gait(string str) {
   gait = str;
}

string query_gait() {
   return gait;
}

void set_stance(int i) {
   stance = i ? i : STANCE_LIE;
}

int query_stance() {
   return stance;
}

void set_default_stance(int i) {
   default_stance = i;
}

int query_default_stance() {
   return default_stance;
}

void set_preposition(int p) {
   preposition = p;
}

void clear_preposition() {
   preposition = 0;
}

int query_preposition() {
   return preposition;
}

void clear_stance() {
   stance = STANCE_STAND;
   preposition = 0;
}


string describe_stance() {
   switch(stance) {
   case STANCE_CROUCH:
      return "crouching";
   case STANCE_KNEEL:
      return "kneeling";
   case STANCE_LIE:
      return "lying";
   case STANCE_SIT:
      return "sitting";
   case STANCE_STAND:
      return "standing";
   case STANCE_HANG:
      return "hanging";
   case STANCE_FLOAT:
      return "floating";
   }
   return "";
}

string describe_preposition() {
   switch(preposition) {
   case PREP_CLOSE_TO:
      return "close to";
   case PREP_AGAINST:
      return "against";
   case PREP_UNDER:
      return "beneath";
   case PREP_ON:
      return "on";
   case PREP_INSIDE:
      return "in";
   default:
   case PREP_NEAR:
      return "near";
   case PREP_OVER:
      return "above";
   case PREP_BEHIND:
      return "behind";
   case PREP_BEFORE:
      return "in front of";
   case PREP_BESIDE:
      return "next to";
   }
}
