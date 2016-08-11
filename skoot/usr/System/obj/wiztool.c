# include <status.h>
# include <kernel/kernel.h>
# include <kernel/user.h>
# include <System.h>

inherit SYS_LIB_WIZTOOL;

private object user;		/* associated user object */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize object
 */
static create(int clone)
{
    if (clone) {
	::create(200);
    }
}

/*
 * NAME:	message()
 * DESCRIPTION:	pass on a message to the user
 */
static void message(string str)
{
    user->message(str);
}

/*
 * NAME:	input()
 * DESCRIPTION:	deal with input from user
 */
void input(string str)
{
   if (previous_program() == DEV_USER) {
      user = previous_object();
      call_limited("process", str);
      user = nil;
   }
}

/* TODO TODO TODO */
object clone(string clonable) {
   return clone_object(clonable);
}

/* TODO TODO TODO */
void destroy(object ob) {
   destruct_object(ob);
}


/*
 * NAME:	process()
 * DESCRIPTION:	process user input
 */
static void process(string str)
{
    string arg;

    if (query_editor(this_object())) {
	if (strlen(str) != 0 && str[0] == '!') {
	    str = str[1 ..];
	} else {
	    str = editor(str);
	    if (str) {
		message(str);
	    }
	    return;
	}
    }

    if (str == "") {
	return;
    }

    sscanf(str, "%s %s", str, arg);

    switch (str) {
    case "inherits":
    case "inherited":
	call_other(this_object(), "cmd_inheritance", user, str, arg);
	break;

    case "upgrade":
    case "issues":
    case "unfreed":
    case "leaves":
    case "patch":
    case "dpatch":
    case "dbstatus":
    case "debug":
    case "clones":
    case "clonetree":

    case "code":
    case "history":
    case "clear":
    case "compile":
    case "clone":
    case "destruct":

    case "cd":
    case "pwd":
    case "ls":
    case "cp":
    case "mv":
    case "rm":
    case "mkdir":
    case "rmdir":
    case "ed":

    case "access":
    case "grant":
    case "ungrant":
    case "quota":
    case "rsrc":

    case "people":
    case "status":
    case "swapout":
    case "statedump":
    case "shutdown":
    case "reboot":
    case "help":
	call_other(this_object(), "cmd_" + str, user, str, arg);
	break;

    default:
	message("No command: " + str + "\n");
	break;
    }
}
