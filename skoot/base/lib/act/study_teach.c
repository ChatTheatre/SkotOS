/*
 * /base/lib/act/study_teach.c
 *
 * Do the study and teach thing.
 */

# define STUDY_TEACH "/base/sys/study_teach"

# include <base.h>
# include <nref.h>

# include "/base/include/events.h"
# include "/base/include/act.h"

# define SD_STOP_LEFT   0
# define SD_STOP_HANDLE 1
# define SD_TIME_LEFT   2
# define SD_TIME_HANDLE 3
# define SD_MESG_DELAY  4
# define SD_MESG_HANDLE 5
# define SD_TIMESTAMP   6

static
float action_study_request(object result, string state, int flags,
			   mapping args) {
    object me, here, teacher;
    string skill;
    int    handle;

    teacher = ObArg("teacher");
    skill = StrArg("skill");

    handle = call_out("study_request_timeout", IntArg("timeout"),
		      teacher, skill);

    me = this_object();
    here = me->query_environment();

    me->add_study_request(teacher, skill, handle);

    result->new_output("Output:StudyRequest", ({ teacher }),
		       ([ "student": me,
			  "teacher": teacher,
			  "skill": skill ]));
    return 0.0;
}

static
void study_request_timeout(object teacher, string skill) {
    object me, here, result;

    result = get_result_object();

    me = this_object();
    here = me->query_environment();
    me->remove_study_request(teacher, skill);

    result->new_output("Output:StudyRequestExpire", ({ teacher }),
		       ([ "student": me,
			  "teacher": teacher,
			  "skill": skill ]),
		       TRUE);

    flush_result_object();
}

static
float action_teach_offer(object result, string state, int flags,
			 mapping args) {
    object me, here, student;
    string skill;
    int    handle;

    student = ObArg("student");
    skill = StrArg("skill");

    handle = call_out("teach_offer_timeout", IntArg("timeout"),
		      student, skill);

    me = this_object();
    here = me->query_environment();

    me->add_teach_offer(student, skill, handle);

    result->new_output("Output:TeachOffer", ({ student }),
		       ([ "student": student,
			  "teacher": me,
			  "skill": skill ]));
}

/* not used? */
# if 0
atomic void
act_teach_offer_withdraw(object student, string skill)
{
    int    handle;
    object output, me, here;

    me = this_object();
    here = me->query_environment();

    handle = me->query_teach_offer(student, skill);
    remove_call_out(handle);
    me->remove_teach_offer(student, skill);

    New_Output_Object();
    Set_Output("Output:TeachOfferWithdrawn", "student", student);
    Set_Output("Output:TeachOfferWithdrawn", "teacher", me);
    Set_Output("Output:TeachOfferWithdrawn", "skill",   skill);
    Set_Output_Actors(me, student,
		      here->query_inventory() - ({ me, student }));
    broadcast("output", output);
}
# endif

static atomic
void teach_offer_timeout(object student, string skill) {
    object result, me, here;

    result = get_result_object();

    me = this_object();
    here = me->query_environment();

    me->remove_teach_offer(student, skill);

    result->new_output("Output:TeachOfferExpire", ({ student }),
		       ([ "student": student,
			  "teacher": me,
			  "skill": skill ]),
		       TRUE);
    flush_result_object();
}

static
float action_study_start(object result, string state, int flags,
			 mapping args) {
    object me, teacher;
    string skill;
    int    co_handle1, co_handle2, duration;

    teacher = ObArg("teacher");
    skill = StrArg("skill");
    duration = IntArg("duration");

    me = this_object();

    me->set_teacher(teacher, skill);
    teacher->add_student(this_object(), skill);

    subscribe_event(me,      "moved");
    subscribe_event(teacher, "moved");

    co_handle1 = call_out("action", duration, "study_stop",
			  ([ "successfully": TRUE ]));
    co_handle2 = call_out("action", duration / 10 + random(60),
			  "study_message");

    me->set_study_data(duration,      co_handle1,
		       IntArg("max_skip"),      0,
		       duration / 10, co_handle2,
		       time() + duration);

    /* Assumption: Output has been taken care of already. */
}

/*
 * The teacher accepts the request, but this function should be called in the
 * student's body-object, as that is where the request is 'stored' and the
 * timeout will be running.
 */
static
float action_study_request_accept(object result, string state, int flags,
				  mapping args) {
    object me, here, teacher;
    string skill;
    int    handle, duration;

    teacher = ObArg("teacher");
    skill = StrArg("skill");
    duration = IntArg("duration");

    me     = this_object();
    here   = me->query_environment();
    handle = me->query_study_request(teacher, skill);

    remove_call_out(handle);
    me->remove_study_request(teacher, skill);

    insert_action("study_start", args);

    result->new_output("Output:StudyRequestAccept", ({ teacher }),
		       ([ "student": me,
			  "teacher": teacher,
			  "skill": skill ]));
}

/*
 * The student accepts the offer, but this function should be called in the
 * teacher's body-object, as that is where the offer is 'stored' and the
 * timeout will be running.
 */
static
float action_teach_offer_accept(object result, string state, int flags,
				mapping args) {
    object me, here, student;
    string skill;
    int    handle, duration;

    student = ObArg("student");
    skill = StrArg("skill");
    duration = IntArg("duration");

    me     = this_object();
    here   = me->query_environment();
    handle = me->query_teach_offer(student, skill);

    remove_call_out(handle);
    me->remove_teach_offer(student, skill);

    /*
     * It is however the student that keeps track of the studying.
     */
    student->action("study_start", args + ([ "teacher": me ]));

    /*
     * The Output:TeachOfferAccept is acted from the student's perspective!
     */
    result->new_output("Output:TeachOfferAccept", ({ student }),
		       ([ "student": student,
			  "teacher": me,
			  "skill": skill ]));
}

static
float action_study_message(object result, string state, int flags,
			   mapping args) {
    int    left;
    string skill;
    object me, here, teacher;
    mixed  *data;

    me      = this_object();
    here    = me->query_environment();
    teacher = me->query_teacher();
    skill   = me->query_teaching();
    data    = me->query_study_data();
    left    = (data[SD_TIMESTAMP] - time()) / 60; /* Countdown in minutes */

    result->new_output("Output:TeachStudyMessage", ({ teacher }),
		       ([ "student": me,
			  "teacher": teacher,
			  "skill": skill ]),
		       TRUE);

    data[SD_MESG_HANDLE] = call_out("action", data[SD_MESG_DELAY] + random(60),
				    "study_message");
    me->set_study_data(data...);
}

static
float action_study_stop(object result, string state, int flags,
			mapping args) {
    string skill, skill_handler;
    object me, here, teacher;
    mixed  *data;

    me      = this_object();
    here    = me->query_environment();
    teacher = this_object()->query_teacher();
    skill   = this_object()->query_teaching();

    /*
     * We're done, one way or another, so stop listening to these:
     */
    unsubscribe_event(this_object(), "moved");
    unsubscribe_event(teacher,       "moved");

    this_object()->clear_teacher();
    this_object()->clear_teaching();

    /* Remove the still-running random-message-callout */
    data = this_object()->query_study_data();
    remove_call_out(data[SD_MESG_HANDLE]);

    this_object()->clear_study_data();
    teacher->remove_student(this_object());

    skill_handler = CONFIGD->query("Base:Skills:Handler");
    if (IntArg("successfully")) {
	int *results;

	result->new_output("Output:StudyStop", ({ teacher }),
			   ([ "student": me,
			      "teacher": teacher,
			      "skill": skill ]),
			   TRUE);

	results = skill_handler->skill_advance(this_object(), teacher, skill, TRUE);
	switch (results[0]) {
	case 0:
	    /* Alas, need to study more another time. */
	    result->new_output("Output:StudyStopNoGain", ({ teacher }),
			       ([ "student": me,
				  "teacher": teacher,
				  "skill": skill ]),
			       TRUE);
	    break;
	case 1:
	    /* student advances the skill */
	    result->new_output("Output:StudyStopGain", ({ teacher }),
			       ([ "student": me,
				  "teacher": teacher,
				  "skill": skill ]),
			       TRUE);
	    break;
	case 2:
	default:
	    result->new_output("Output:StudyStopGain2", ({ teacher }),
			       ([ "student": me,
				  "teacher": teacher,
				  "skill": skill ]),
			       TRUE);
	    break;
	}
    } else {
	result->new_output("Output:StudyStopAbort", ({ teacher }),
			   ([ "student": me,
			      "teacher": teacher,
			      "skill": skill ]),
			   TRUE);
    }
}

static nomask atomic
void evt_moved(object who, object from, object to)
{
    object me, env, teacher, teacher_env;
    int    *data;

    me          = this_object();
    env         = me->query_environment();
    teacher     = me->query_teacher();
    teacher_env = teacher->query_environment();
    data        = me->query_study_data();

    if (data[SD_TIME_HANDLE] > 0) {
	/* Check if the student/teacher are back together now. */
	if (!env || !teacher_env || env != teacher_env) {
	    /* No, keep on counting. */
	    return;
	}
	/* Really?  Well, stop the clocks and continue studying! */
	data[SD_TIME_LEFT]   = remove_call_out(data[SD_TIME_HANDLE]);
	data[SD_TIME_HANDLE] = 0;
	data[SD_STOP_HANDLE] = call_out("action",
					data[SD_STOP_LEFT],
					"study_stop",
					([ "successfully": TRUE ]));
	data[SD_MESG_HANDLE] = call_out("action",
					data[SD_MESG_DELAY] + random(60),
					"study_message");
	me->set_study_data(data...);

	get_result_object()->new_output("Output:StudyResume", ({ teacher }),
					([ "student": me,
					   "teacher": teacher,
					   "skill": me->query_teaching() ]),
					TRUE);
	flush_result_object();
	return;
    }
    /* Check if the student/teacher are no longer in the same room. */
    if (env && teacher_env && env == teacher_env) {
	/* Actually, they are.  Oh well, false alarm. */
	return;
    }
    /* Indeed! */
    data[SD_STOP_LEFT]   = remove_call_out(data[SD_STOP_HANDLE]);
    data[SD_STOP_HANDLE] = 0;
    data[SD_TIME_HANDLE] = call_out("action",
				    data[SD_TIME_LEFT],
				    "study_stop",
				    ([ "successfully": FALSE ]));
    remove_call_out(data[SD_MESG_HANDLE]);

    me->set_study_data(data...);

    get_result_object()->new_output("Output:StudySuspend", ({ teacher }),
				    ([ "student": me,
				       "teacher": teacher,
				       "skill": me->query_teaching() ]),
				    TRUE);
    flush_result_object();
}

/*
 * This function is a bit out of place here, but this is the only part of the
 * object that knows what the query_study_data() contents mean, and we need
 * those to be accurate.
 */
int
query_study_countdown()
{
    mixed *data;

    data = this_object()->query_study_data();
    if (data == nil) {
	/* This player is not studying. */
	return -1;
    }
    if (data[SD_STOP_HANDLE] == 0) {
	/* Student and teacher not in same room. */
	return data[SD_STOP_LEFT];
    }
    /* Student and teacher are in the same room. */
    return data[SD_TIMESTAMP] - time();
}

static
float action_assess(object result, string state, int flags, mapping args) {
    insert_action("demand_consent",
		  ([ "target": NRefArg("who"),
		     "message": "assess",
		     "action": "assess_complete",
		     "args": args ]));
}

static
float action_assess_complete(object result, string state, int flags,
			     mapping args) {
    int    val_me, val_other;
    string skill, skill_handler;
    object me, here;
    NRef who;

    who = NRefArg("who");
    skill = StrArg("skill");

    me   = this_object();
    here = me->query_environment();

    skill_handler = CONFIGD->query("Base:Skills:Handler");
    val_me    = skill_handler->skill_value(me, skill);
    val_other = skill_handler->skill_value(NRefOb(who), skill);

    if (val_me == val_other) {
	result->new_output("Output:Assess:Equal", nil,
			   ([ "student": NRefOb(who),
			      "skill": skill ]));
	return 0.0;
    }
    if (val_me > val_other) {
	/* XXX: Assumptions about minimum value! */
	if (val_other == 0) {
	    result->new_output("Output:Assess:NoSkill", nil,
			       ([ "student": NRefOb(who),
				  "skill": skill ]));
	    return 0.0;
	}
	result->new_output("Output:Assess:TeacherBetter", nil,
			   ([ "student": NRefOb(who),
			      "skill": skill,
			      "value": skill_handler->skill_description(val_other) ]));
	return 0.0;
    }
    result->new_output("Output:Assess:StudentBetter", nil,
		       ([ "student": NRefOb(who),
			  "skill": skill ]));
}
