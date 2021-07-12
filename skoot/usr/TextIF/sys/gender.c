/*
**	~TextIF/sys/themes.c
**
**	Copyright Skotos Tech Inc 1999
*/

private inherit "/lib/string";
private inherit "/lib/mapping";

inherit "/lib/properties";

string query_state_root() { return "Core:Properties"; }

static
void create() {
   ::create();

   set_object_name("TextIF:Gender");
}

string query_gender(int gender) {
    if(query_property("gender_map")) {
        return query_property("gender_map")[gender];
    } else {
        switch(gender) {
            case 0: return "neuter";
            case 1: return "male";
            case 2: return "female";
            default: return "neuter";
        }
    }
}

int query_gender_reversed(string gender) {
    if(query_property("gender_map")) {
        return reverse_mapping(query_property("gender_map"))[gender];
    } else {
        switch(gender) {
            case "neuter": return 0;
            case "male": return 1;
            case "female": return 2;
            default: return 0;
        }
    }
}

string query_possessive(int gender) {
    if(query_property("gender_map") && query_property("gender_map")[gender]) {
        return query_property("gender:" + query_property("gender_map")[gender] + ":possessive");
    } else {
        switch(gender) {
            case 0: return "its";
            case 1: return "his";
            case 2: return "her";
            default: return "its";
        }
    }
}

string query_pronoun(int gender) {
    if(query_property("gender_map") && query_property("gender_map")[gender]) {
        return query_property("gender:" + query_property("gender_map")[gender] + ":pronoun");
    } else {
        switch(gender) {
            case 0: return "it";
            case 1: return "he";
            case 2: return "she";
            default: return "it";
        }
    }
}

string query_objective(int gender) {
    if(query_property("gender_map") && query_property("gender_map")[gender]) {
        return query_property("gender:" + query_property("gender_map")[gender] + ":objective");
    } else {
        switch(gender) {
            case 0: return "it";
            case 1: return "him";
            case 2: return "her";
            default: return "it";
        }
    }
}

string query_genderstring(int gender) {
    if(query_property("gender_map") && query_property("gender_map")[gender]) {
        return query_property("gender:" + query_property("gender_map")[gender] + ":genderstring");
    } else {
        switch(gender) {
            case 0: return "neuter";
            case 1: return "male";
            case 2: return "female";
            default: return "neuter";
        }
    }
}
