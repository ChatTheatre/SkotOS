/*
**	~System/lib/sys_auto.c
**
**	This is automatically inherited by all clonable programs, i.e.
**	ones with /obj/ but not /lib/ in their paths. It allows us to
**	maintain the double-linked list of clones for every clonable
**	master object.
**
**	Copyright Skotos Tech Inc 1999
*/

inherit "./sys_auto";

private object next, prev;
private int count;

nomask
void _S_next_clone(object obj) {
    if (previous_program() == "/usr/System/lib/sys_auto") {
	next = obj;
    }
}

nomask
void _S_prev_clone(object obj) {
    if (previous_program() == "/usr/System/lib/sys_auto") {
	prev = obj;
    }
}

nomask
void _F_add_to_clone_count(int n) {
    if (previous_program() == "/usr/System/lib/sys_auto") {
	count += n;
    }
}    

nomask
object query_next_clone() {
    return next;
}

nomask
object query_previous_clone() {
    return prev;
}

nomask
int query_clone_count() {
    return count;
}

/* TODO; debug */

void bump_clone_count_by(int n) { count += n; }
void set_links(object p, object n) { prev = p; next = n; }
