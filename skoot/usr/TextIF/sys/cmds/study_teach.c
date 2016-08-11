/*
 * /usr/TextIF/sys/cmds/study_teach.c
 *
 * Three optional command-implementations: 'study', 'teach' and 'skills'
 *
 * Copyright Skotos Tech Inc 2001
 */

# include <System.h>

private inherit "/lib/string";

inherit "/usr/SAM/lib/sam";
inherit "/usr/SkotOS/lib/describe";
inherit "/usr/SkotOS/lib/noun";
inherit "/base/lib/urcalls";

# include "/usr/SkotOS/include/noun.h"

# define STUDY_TEACH "/base/sys/study_teach"

private string skill_handler;

static void
create()
{
    skill_handler = CONFIGD->query("Base:Skills:Handler");
}

void
boot(int block)
{
    subscribe_event(find_object(CONFIGD), "updated");
}

void
evt_updated(object ob, string index)
{
    switch (index) {
    case "Base:Skills:Handler":
	/* It may have changed, make sure we have the most recent data */
	skill_handler = CONFIGD->query("Base:Skills:Handler");
	break;
    default:
	break;
    }
}

private void
show_study_info(object user, object body)
{
    int left;

    left = body->query_study_countdown() / 60;
    user->message("You are currently studying your " +
		  body->query_teaching() + " skill with " +
		  describe_one(body->query_teacher()) + ", " +
		  (left > 1 ?
		   "with about " + left + " minutes" :
		   left == 1 ?
		   "with about " + left + " minute" :
		   "with less than a minute") +
		  " left to go.\n");
}

private string
describe_delay(int delay)
{
    return (delay < 3600 ?
	    "less than an hour" :
	    delay < 2 * 3600 ?
	    "at least one hour" :
	    delay < 86400 ?
	    "at least " + (delay / 3600) + " hours" :
	    delay < 2 * 86400 ?
	    "at least one day" :
	    "at least " + (delay / 86400) + " days");
}

int
characters_are_alternatives(object body1, object body2)
{
    int     i, sz;
    object  *users1, *users2;
    mapping names;

    users1 = body1->query_souls();
    users2 = body2->query_souls();

    if (!users1 || !sizeof(users1) || !users2 || !sizeof(users2)) {
	/* One or more of these bodies is not a PC? */
	return FALSE;
    }
    if (sizeof(users1 & users2) > 0) {
	/* These bodies share a connection. */
	return TRUE;
    }

    /*
     * This might be more complex than necessary, I'm not sure if it's possible
     * for a body to have two different users connect to it at once, for
     * instance if someone 'possesses' it.
     */
    names = ([ ]);
    sz = sizeof(users1);
    for (i = 0; i < sz; i++) {
	names[users1[i]->query_name()] = TRUE;
    }
    sz = sizeof(users2);
    for (i = 0; i < sz; i++) {
	if (names[users2[i]->query_name()]) {
	    /* Same user(name) from different connections. */
	    return TRUE;
	}
    }
    return FALSE;
}

void
cmd_study(object user, object body, varargs mixed skill, Ob teacher)
{
    int    delay;
    string skill_name, prop;
    object teacher_ob;
    NRef teacher_ref;

    if (!skill && !teacher) {
	if (body->query_teacher()) {
	    show_study_info(user, body);
	} else {
	    user->message("You are not currently studying anything.\n");
	}
	return;
    }

    if (skill) {
	skill = implode(skill, " ");
	/* Is that an existing 'skill'? */
	if (!skill_handler->skill_exists(skill)) {
	    user->message("That is either not a valid skill name or the teacher does not have that skill.\n");
	    return;
	}
	if (!skill_handler->skill_taught(skill)) {
	    user->message("That is not a skill you can study.\n");
	    return;
	}
	skill = skill_handler->skill_name(skill);
    }

    if (teacher) {
	teacher_ref = locate_one(user, LF_HUSH_ALL, ({ teacher }), body,
				 body->query_environment());
	if (!teacher_ref) {
	    user->message("There is noone by that name here to teach you.\n");
	    return;
	}
	teacher_ob = NRefOb(teacher_ref);
    }

    if (teacher_ob == body) {
	/* Uhm */
	user->message("You need someone else to study with.\n");
	return;
    }

    if (!skill) {
	user->message("What skill did you want to study with " + describe_one(teacher_ob) + "?\n");
	return;
    }

    if (!teacher) {
	user->message("You must study a skill with a teacher, i.e. \"study <skillname> with <teachername>\".\n");
	return;
    }

    /*
     * You can't study and teach at the same time.
     */
    if (sizeof(body->query_students()) > 0) {
	user->message("Trying to study while you are teaching others is " +
		      "going to take too much concentration.\n");
	return;
    }

    /*
     * You can't study when you're already studying something else either.
     */
    if (body->query_teacher()) {
	user->message("Trying to study with more than one teacher is going " +
		      "to take too much concentration.\n");
	return;
    }

    if (teacher_ob->query_teacher()) {
	/* The teacher can't teach if he/she is studying him/herself. */
	user->message("Unfortunately " + describe_one(teacher_ob) +
		      " is studying already and teaching you would take too " +
		      "much concentration, try again later.\n");
	return;
    }

    /*
     * Did you request this exact same thing already?
     */
    if (body->query_study_request(teacher_ob, skill)) {
	/*
	 * Indeed, why ask twice?
	 */
	user->message("Your previous request did not go unheard, " +
		      "no need to ask more than once.\n");
	return;
    }

    /* Not too soon, please. */
    delay = skill_handler->can_study(body);
    if (delay > 0) {
	user->message("You may only practice only one skill each day.  " +
		      "You have to wait " + describe_delay(delay) + ".\n");
	return;
    }
    delay = skill_handler->can_study(body, skill);
    if (delay > 0) {
	user->message("You are still absorbing the lessons from last time.  " +
		      "You have to wait " + describe_delay(delay) + ".\n");
	return;
    }

    if (skill_handler->skill_value(teacher_ob, "teaching") < 1) {
	user->message(describe_one(teacher_ob) + " does not have teaching skill.\n");
    }

    switch (skill_handler->can_teach(teacher_ob)) {
    case -1:
	user->message(describe_one(teacher_ob) + " has no teaching skill.\n");
	return;
    case 0:
	user->message(describe_one(teacher_ob) +
		      " is unable to take another student right now, try again later.\n");
	return;
    default:
	break;
    }
    if (!skill_handler->can_teach(teacher_ob, body, skill)) {
	user->message(describe_one(teacher_ob) +
		      " is not skilled enough to be teaching you that.\n");
	return;
    }

    if (characters_are_alternatives(body, teacher_ob)) {
	user->message("OOC: Sorry, but you may not teach your alternative characters.\n");
	return;
    }

    /*
     * Check if the teacher is already teaching something, you can only
     * teach one skill at a time, atlhough to more than one person, yes.
     */
    if (sizeof(teacher_ob->query_students()) > 0) {
	object *list;

	list = teacher_ob->query_students();
	if (skill != list[0]->query_teaching()) {
	    user->message(describe_one(teacher_ob) +
			  " can only teach one skill at a time.\n");
	    return;
	}
    }

    /*
     * Is this something the teacher offered us already?
     */
    if (teacher_ob->query_teach_offer(body, skill)) {
	/*
	 * Yes, sure did.  Well, take him/her up on it, then.
	 */
	teacher_ob->action("teach_offer_accept",
			   ([ "student": body,
			      "skill": skill,
			      "duration": skill_handler->skill_study_duration(),
			      "max_skip": skill_handler->skill_study_absence()
			      ]));
    } else {
	/*
	 * Nope.  Well, make the request then and wait to see how the teacher
	 * reacts.
	 */
	body->action("study_request",
		     ([ "teacher": teacher_ob,
			"skill": skill,
			"timeout": skill_handler->skill_request_timeout() ]));
    }
}

private void
show_teach_info(object user, object body)
{
    int i, sz;
    object *list;
    string result;

    list = body->query_students();
    sz = sizeof(list);
    
    result = "You are currently teaching " +
	     (sz == 1 ? "1 student" : sz + " students") + ":\n";
    for (i = 0; i < sz; i++) {
	int left;

	left = list[i]->query_study_countdown() / 60;
	result += describe_one(list[i]) + " has " +
		  (left > 1 ?
		   "about " + left + " minutes" :
		   left == 1 ?
		   "about 1 minute" :
		   "less than a minute") + " left to go.\n";
    }
    user->message(result);
}

void
cmd_teach(object user, object body, varargs Ob student, mixed skill)
{
    int    delay;
    string skill_name, prop;
    object student_ob;
    NRef student_ref;

    if (!student && !skill) {
	if (sizeof(body->query_students()) > 0) {
	    show_teach_info(user, body);
	} else {
	    user->message("You are currently not teaching anyone.\n");
	}
	return;
    }
    if (skill) {
	skill = implode(skill, " ");
	if (!skill_handler->skill_exists(skill)) {
	    user->message("That is not a valid skill name.\n");
	    return;
	}
	if (!skill_handler->skill_taught(skill)) {
	    user->message("That is not a skill you can teach.\n");
	    return;
	}
	skill = skill_handler->skill_name(skill);
	/*
	 * Check if we're already teaching something, you can only teach
	 * one skill at a time, although to more than one person, yes.
	 */
	if (sizeof(body->query_students()) > 0) {
	    object *list;

	    list = body->query_students();
	    if (skill != list[0]->query_teaching()) {
		user->message("You can only teach one skill at a time.\n");
		return;
	    }
	}
    }
    if (student) {
	student_ref = locate_one(user, LF_HUSH_ALL, ({ student }), body,
				 body->query_environment());
	if (!student_ref) {
	    user->message("There is noone by that name here to teach.\n");
	    return;
	}
	student_ob = NRefOb(student_ref);
    }
    if (!skill) {
	/* Check if the student has any outstanding requests. */
	string *skills;

	skills = student_ob->query_study_request(body);
	switch (sizeof(skills)) {
	case 0:
	    user->message("What skill did you want to teach " +
			  describe_one(student_ob) + "?\n");
	    break;
	case 1:
	    user->message(describe_one(student_ob) +
			  " has requested to study " +
			  student_ob->query_possessive() + " " +
			  skills[0] + " skill with you, is that the skill " +
			  "you want to teach " +
			  student_ob->query_objective() + "?\n");
	    break;
	default:
	    break;
	}
	return;
    }
    if (!student) {
	user->message("You must teach a skill to a student, i.e. \"teach <skillname> to <studentname>\".\n");
	return;
    }

    if (student_ob == body) {
	/* Uhm */
	user->message("You can't teach yourself.\n");
	return;
    }

    if (body->query_teach_offer(student_ob, skill)) {
	/* You made this offer already and it hasn't expired yet. */
	user->message("Your previous offer did not go unheard, " +
		      "no need to make it more than once.\n");
	return;
    }

    /* You can't study more than one thing at the same time. */
    if (student_ob->query_teacher()) {
	user->message(describe_one(student_ob) +
		      " is already studying with " +
		      describe_one(student_ob->query_teacher()) +
		      ", studying something else at the same time is not possible.\n");
	return;
    }

    /* Can't teach and study at the same time. */
    if (sizeof(student_ob->query_students()) > 0) {
        user->message(describe_one(student_ob) +
		      " is already teaching one or more students, " +
		      "studying something at the same time is going to take too much concentration.\n");
        return;
    }

    if (body->query_teacher()) {
        user->message("Teaching " + describe_one(student_ob) +
		      " while you yourself are studying is going to take too much of your concentration.\n");
        return;
    }

    /* Not too soon, please. */
    delay = skill_handler->can_study(student_ob);
    if (delay > 0) {
#if 0
	user->message("One may practice only one skill each day, " +
		      "it has been too recent for " +
		      describe_one(student_ob) + ".  " +
		      capitalize(student_ob->query_pronoun()) +
		      " has to wait " + describe_delay(delay) + ".\n");
#else
	user->message("One may practice only one skill each day, " +
		      "it has been too recent for " +
		      describe_one(student_ob) + ".  " +
		      capitalize(student_ob->query_pronoun()) +
		      " will have to wait.\n");
#endif
	return;
    }
    delay = skill_handler->can_study(student_ob, skill);
    if (delay > 0) {
#if 0
	user->message(describe_one(student_ob) +
		      " is still absorbing the lessons from " +
		      student_ob->query_possessive() + " last time.  " +
		      capitalize(student_ob->query_pronoun()) +
		      " has to wait " + describe_delay(delay) + ".\n");
#else
	user->message(describe_one(student_ob) +
		      " is still absorbing the lessons from " +
		      student_ob->query_possessive() + " last time.  " +
		      capitalize(student_ob->query_pronoun()) +
		      " will have to wait.\n");
#endif
	return;
    }

    /* Ah, but is he a qualified (and available) teacher? */
    switch (skill_handler->can_teach(body)) {
    case -1:
	user->message("You do not have teaching skill.\n");
	return;
    case 0:
        user->message("You cannot take another student right now, try again later.\n");
	return;
    default:
	break;
    }
    if (!skill_handler->can_teach(body, student_ob, skill)) {
	user->message("You are not skilled enough to be teaching " +
		      describe_one(student_ob) + " that.\n");
	return;
    }

    if (characters_are_alternatives(body, student_ob)) {
	user->message("OOC: Sorry, but you may not teach your alternative characters.\n");
	return;
    }

    /*
     * Is this something the student requested from us already?
     */
    if (student_ob->query_study_request(body, skill)) {
	/*
	 * Yes, sure did.  Well, take him/her up on it, then.
	 */
	student_ob->action("study_request_accept",
			   ([ "teacher": body,
			      "skill": skill,
			      "duration": skill_handler->skill_study_duration(),
			      "max_skip": skill_handler->skill_study_absence() ]));
    } else {
	/*
	 * No.  Well, make the offer and wait to see how the student reacts.
	 */
	body->action("teach_offer",
		     ([ "student": student_ob,
			"skill": skill,
			"timeout": skill_handler->skill_offer_timeout() ]));
    }
}

/* Find out what teachable skills there are and what their values are. */
void
cmd_skills(object user, object body, varargs string group)
{
    int    i, sz_i;
    string *list, *groups;
    string *results;

    if (body->query_teacher()) {
	show_study_info(user, body);
    }
    if (sizeof(body->query_students()) > 0) {
	show_teach_info(user, body);
    }
    groups = skill_handler->list_categories();
    if (group) {
	if (sizeof(groups & ({ lower_case(group) })) == 0) {
	    user->message("That category of skills is unknown.\n");
	    return;
	} else {
	    groups = ({ group });
	}
    }
    sz_i = sizeof(groups);
    if (sz_i == 0) {
	user->message("You have no known skills.\n");
	return;
    }
    results = ({ });
    for (i = 0; i < sz_i; i++) {
	int j, sz_j;
	string sub_result;

	sub_result = "";
	list = skill_handler->list_skills(groups[i]);
	sz_j = sizeof(list);
	for (j = 0; j < sz_j; j++) {
	    mixed  value;
	    string training, prop;

	    if (!skill_handler->skill_exists(list[j])) {
		continue;
	    }
	    value = skill_handler->skill_value(body, list[j]);
	    training = skill_handler->skill_training_description(body, list[j]);
	    if (value > 0) {
		sub_result += capitalize(list[j]) + ": " +
		              skill_handler->get_level_description(value) +
		              (training ? " (" + training + ")" : "") +
		              "\n";
	    } else if (training) {
		sub_result += capitalize(list[j]) + ": Unskilled (" +
		              training + ")\n";
	    }
	}
	if (strlen(sub_result) > 0) {
	    results += ({ "---- " + capitalize(groups[i]) + ": ----\n" + sub_result });
	}
    }
    if (sizeof(results) > 0) {
	user->message(implode(results, "\n"));
    } else {
	if (group) {
	    user->message("You have no skills in that category.\n");
	} else {
	    user->message("You have no known skills.\n");
	}
    }
}

void
cmd_assess(object user, object body, varargs Ob student, string *words)
{
    string skill;
    object student_ob;
    NRef   student_ref;

    if (!student) {
	user->message("Try assess <studentname> or assess <studentname> for <skillname>.\n");
	return;
    }
    student_ref = locate_one(user, 0, ({ student }), body, body->query_environment());
    if (!student_ref) {
	return;
    }
    student_ob = NRefOb(student_ref);
    if (student_ob == body) {
	user->message("You don't need to assess yourself, you know your own skills.\n");
	return;
    }
    if (!ur_volition(student_ob)) {
	user->message("That is not something you can assess.\n");
	return;
    }
    if (words) {
	skill = implode(words, " ");
    } else {
	/* See if this one has requested to study anything with me. */
	string *list;

	list = student_ob->query_study_request(body);
	if (sizeof(list) != 1) {
	    user->message("For what skill did you want to assess " + describe_one(student_ob) + "?\n");
	    return;
	}
	skill = list[0];
    }
    if (!skill_handler->skill_exists(skill)) {
	user->message("That is not a valid skill name.\n");
	return;
    }
    skill = skill_handler->skill_name(skill);
    if (skill_handler->skill_value(body, "teaching") == 0) {
	user->message("You aren't skilled in teaching.\n");
	return;
    }
    body->action("assess", ([ "who":   student_ref, "skill": skill ]));
}
