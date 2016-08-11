/*
 * /base/lib/study_teach.c
 *
 * Keep track of who we're studying with, or teaching, and who we've offered to
 * teach or requested to study with.
 *
 * The teacher keeps track of the teaching-offers, the student keeps track of
 * the studying-requests, if necessary the information can be made a little
 * more redundant, of course.
 *
 * Copyright Skotos Tech Inc 2001
 */

# define ST_SKILL    0
# define ST_TEACHER  1
# define ST_STUDENTS 2
# define ST_DATA     3
# define ST_OFFERS   4
# define ST_REQUESTS 5

private mixed *data;

static void
create()
{
    /* Nada zip zilch, don't initialize these variables if they're not used */
}

private void
initialize_data()
{
    if (!data) {
	data = allocate(6);
    }
}

private void
clear_data()
{
    if (sizeof(data - ({ nil })) == 0) {
	data = nil;
    }
}

mixed
cleanup_data()
{
    data = nil;
}

mixed
query_study_teach_data()
{
    return data;
}

/* Students */

void
add_student(object student_ob, string skill_name)
{
    initialize_data();
    if (data[ST_STUDENTS]) {
	data[ST_STUDENTS][student_ob] = skill_name;
    } else {
	data[ST_STUDENTS] = ([ student_ob: skill_name ]);
    }
}

object *
query_students()
{
    return (data && data[ST_STUDENTS]) ? map_indices(data[ST_STUDENTS]) : ({ });
}

void
remove_student(object student_ob)
{
    data[ST_STUDENTS][student_ob] = nil;
    if (!map_sizeof(data[ST_STUDENTS])) {
	data[ST_STUDENTS] = nil;
    }
    clear_data();
}

/* Teacher and skill */

void
set_teacher(object teacher_ob, string skill_name)
{
    initialize_data();
    data[ST_TEACHER] = teacher_ob;
    data[ST_SKILL]   = skill_name;
}

object
query_teacher()
{
    return data ? data[ST_TEACHER] : nil;
}

string
query_teaching()
{
    return data ? data[ST_SKILL] : nil;
}

string
clear_teacher()
{
    data[ST_TEACHER] = nil;
    data[ST_SKILL]   = nil;
    clear_data();
}

/* Study data, countdowns and handles */

void
set_study_data(varargs mixed d...)
{
    initialize_data();
    data[ST_DATA] = d;
}

mixed *
query_study_data()
{
    return data ? data[ST_DATA] : nil;
}

void
clear_study_data()
{
    data[ST_DATA] = nil;
    clear_data();
}

/* Offers to teach someone else a certain skill */

atomic void
add_teach_offer(object student, string skill, int handle)
{
    initialize_data();

    if (data[ST_OFFERS]) {
	if (data[ST_OFFERS][student]) {
	    mixed old_handle;

	    old_handle = data[ST_OFFERS][student][skill];
	    if (old_handle != nil) {
		remove_call_out(old_handle);
	    }
	    data[ST_OFFERS][student][skill] = handle;
	} else {
	    data[ST_OFFERS][student] = ([ skill: handle ]);
	}
    } else {
	data[ST_OFFERS] = ([ student: ([ skill: handle ]) ]);
    }
}

mixed
query_teach_offer(varargs object student, string skill)
{
    /*
     * Return the handle of the expire-timeout-call_out or 0.
     */
    if (student && skill) {
	return (data && data[ST_OFFERS] && data[ST_OFFERS][student]) ?
	       data[ST_OFFERS][student][skill] : 0;
    }
    /*
     * Return an array of skill-names that you have offers to teach to this
     * student.
     */
    if (student) {
	return (data && data[ST_OFFERS] && data[ST_OFFERS][student]) ?
	       map_indices(data[ST_OFFERS][student]) : ({ });
    }
    /*
     * Return an array of students you have offered to teach something to.
     */
    return (data && data[ST_OFFERS]) ? map_indices(data[ST_OFFERS]) : ({ });
}

atomic void
remove_teach_offer(object student, string skill)
{
    data[ST_OFFERS][student][skill] = nil;
    if (!map_sizeof(data[ST_OFFERS][student])) {
	data[ST_OFFERS][student] = nil;
	if (!map_sizeof(data[ST_OFFERS])) {
	    data[ST_OFFERS] = nil;
	}
    }
    clear_data();
}

/* Requests to study a certain skill with someone */

atomic void
add_study_request(object teacher, string skill, int handle)
{
    initialize_data();

    if (data[ST_REQUESTS]) {
	if (data[ST_REQUESTS][teacher]) {
	    data[ST_REQUESTS][teacher][skill] = handle;
	} else {
	    data[ST_REQUESTS][teacher] = ([ skill: handle ]);
	}
    } else {
	data[ST_REQUESTS] = ([ teacher: ([ skill: handle ]) ]);
    }
}

mixed
query_study_request(varargs object teacher, string skill)
{
    /*
     * Return the handle for the request-timeout-call_out or 0.
     */
    if (teacher && skill) {
	return (data && data[ST_REQUESTS] && data[ST_REQUESTS][teacher]) ?
	       data[ST_REQUESTS][teacher][skill] : 0;
    }
    /*
     * Return an array of skill-names that you have requested to study with
     * this teacher.
     */
    if (teacher) {
	return (data && data[ST_REQUESTS] && data[ST_REQUESTS][teacher]) ?
	       map_indices(data[ST_REQUESTS][teacher]) : ({ });
    }
    /*
     * Return an array of teachers you have requested to study something with.
     */
    return (data && data[ST_REQUESTS]) ? map_indices(data[ST_REQUESTS]) : ({ });
}

atomic void
remove_study_request(object teacher, string skill)
{
    data[ST_REQUESTS][teacher][skill] = nil;
    if (!map_sizeof(data[ST_REQUESTS][teacher])) {
	data[ST_REQUESTS][teacher] = nil;
	if (!map_sizeof(data[ST_REQUESTS])) {
	    data[ST_REQUESTS] = nil;
	}
    }
    clear_data();
}
