/*
**	/base/lib/living/gender.c
**
**	This code handles the gender of a living thing.
**
**	Copyright Skotos Tech Inc 1999
**
*/

# include <base/living.h>
# include <TextIF.h>

private inherit "/base/lib/urcalls";

private int gender;

void set_gender(int i) { gender = i; }
int query_gender() { 
    SysLog("GENDER IS: " + dump_value(GENDER->query_possessive(gender))); 
           return gender; 
}

void set_neuter() { gender = GENDER_NEUTER; }
void set_male() { gender = GENDER_MALE; }
void set_female() { gender = GENDER_FEMALE; }

int query_neuter() { return ur_gender(this_object()) == GENDER_NEUTER; }
int query_male() { return ur_gender(this_object()) == GENDER_MALE; }
int query_female() { return ur_gender(this_object()) == GENDER_FEMALE; }

string query_possessive() {
   return GENDER->query_possessive(gender);
}

string query_objective() {
   return GENDER->query_objective(gender);
}

string query_pronoun() {
   return GENDER->query_pronoun(gender);
}

string query_gender_string() {
   return GENDER->query_genderstring(gender);
}
