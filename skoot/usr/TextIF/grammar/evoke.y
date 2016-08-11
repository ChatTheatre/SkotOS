# ---- begin ~TextIF/grammar/evoke.y ----

EvokeP:	           {{ return ({ ([ ]) })                   }}
EvokeP: Evoke
EvokeP: ',' Evoke  {{ return tree[1 ..];                   }}
Evoke:  evoke      {{
    int n;
    string str;

    n = strlen(str = tree[0]);
    if (n <= 1) {
       return nil;
    }
    if (n > 0) {
	if (str[0] == '\'') {
	    str[0] = '\"';
	    if (str[n-1] == '\'') {
		str[n-1] = '\"';
	    } else if (str[n-1] != '\"') {
		str += "\"";
	    }
	} else if (str[0] == '\"' && str[n-1] != '\"') {
	    if (str[n-1] == '\'') {
		str[n-1] = '\"';
	    } else {
		str += "\"";
	    }
	}
    }
    return ({ ([ "form": "E", "evoke": str, "raw-evoke": str[1 .. strlen(str)-2] ]) });
}}


# ---- end ~TextIF/grammar/evoke.y ----
