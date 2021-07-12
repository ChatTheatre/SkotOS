/*
**	/base/lib/living/gender.c
**
**	This code handles the gender of a living thing.
**
**	Copyright Skotos Tech Inc 1999
**
*/

# include <base/living.h>

private inherit "/base/lib/urcalls";

private int gender;

void set_gender(int i) { gender = i; }
int query_gender() { 
    SysLog("GENDER IS: " + dump_value(gender)); 
           return gender; 
}

void set_neuter() { gender = GENDER_NEUTER; }
void set_male() { gender = GENDER_MALE; }
void set_female() { gender = GENDER_FEMALE; }

int query_neuter() { return ur_gender(this_object()) == GENDER_NEUTER; }
int query_male() { return ur_gender(this_object()) == GENDER_MALE; }
int query_female() { return ur_gender(this_object()) == GENDER_FEMALE; }

string query_possessive() {
   switch(ur_gender(this_object())) {
   case GENDER_MALE:	return "his";
   case GENDER_FEMALE:	return "her";
   case GENDER_NEUTER:
   default:
      return "its";
   }
}

string query_objective() {
   switch(ur_gender(this_object())) {
   case GENDER_MALE:	return "him";
   case GENDER_FEMALE:	return "her";
   case GENDER_NEUTER:
   default:
      return "it";
   }
}

string query_pronoun() {
   switch(ur_gender(this_object())) {
   case GENDER_MALE:	return "he";
   case GENDER_FEMALE:	return "she";
   case GENDER_NEUTER:
   default:
      return "it";
   }
}

string query_gender_string() {
   switch(ur_gender(this_object())) {
   case GENDER_MALE:	return "male";
   case GENDER_FEMALE:	return "female";
   case GENDER_NEUTER:
   default:
      return "neuter";
   }
}
