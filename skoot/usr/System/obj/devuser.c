# include <System.h>
# include <kernel/kernel.h>
# include <kernel/user.h>
# include <kernel/rsrc.h>

inherit LIB_USER;
inherit user API_USER;
inherit rsrc API_RSRC;

inherit CLONABLE;


# define STATE_NORMAL		0
# define STATE_LOGIN		1
# define STATE_OLDPASSWD	2
# define STATE_NEWPASSWD1	3
# define STATE_NEWPASSWD2	4

string name;		/* user name */
string Name;		/* capitalized user name */
string newpasswd;	/* for changing password */
object wiztool;
int nconn;		/* # of connections */
int state;

object query_wiztool() { return wiztool; }

/*
 * NAME:	create()
 * DESCRIPTION:	initialize connection object
 */
static create(int clone)
{
    if (clone) {
	user::create();
	rsrc::create();
    }
}

/*
 * NAME:	login()
 * DESCRIPTION:	login a new user
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	if (sscanf(ur_name(previous_object()), BINARY_CONN + "#%*d")) {
	    previous_object()->message("Use this binary port only for the 'admin' user -- in emergencies.\n");
	    return MODE_DISCONNECT;
	}
	if (nconn == 0) {
	    ::login(str);
	}
	nconn++;
	if (strlen(str) == 0 || sscanf(str, "%*s ") != 0 ||
	    sscanf(str, "%*s/") != 0) {
	    return MODE_DISCONNECT;
	}
	Name = name = str;
	if (Name[0] >= 'a' && Name[0] <= 'z') {
	    Name[0] -= 'a' - 'A';
	}
	if (name == "admin") {
	    previous_object()->message("Use the binary port for the 'admin' user.\n");
	    return MODE_DISCONNECT;
	}
	previous_object()->message("Password:");
	state = STATE_LOGIN;
	return MODE_NOECHO;
    }
}

/*
 * NAME:	logout()
 * DESCRIPTION:	logout user
 */
logout(varargs int quit)
{
    if (previous_program() == LIB_CONN && --nconn == 0) {
	::logout();
	destruct_object();
    }
}

/*
 * NAME:	receive_message()
 * DESCRIPTION:	process a message from the user
 */
int receive_message(string str)
{
    if (previous_program() == LIB_CONN) {
	string cmd;
	object user, *users;
	int i, sz;

	switch (state) {
	case STATE_NORMAL:
	    cmd = str;
	    if (strlen(str) != 0 && str[0] == '!') {
		cmd = cmd[1 ..];
	    }
	    if (!wiztool || !query_editor(wiztool) || cmd != str) {
		/* check standard commands */
		if (strlen(cmd) != 0) {
		    switch (cmd[0]) {
		    case '\'':
			if (strlen(cmd) > 1) {
			    cmd[0] = ' ';
			    str = cmd;
			}
			cmd = "say";
			break;

		    case ':':
			if (strlen(cmd) > 1) {
			    cmd[0] = ' ';
			    str = cmd;
			}
			cmd = "emote";
			break;

		    default:
			sscanf(cmd, "%s ", cmd);
			break;
		    }
		}

		switch (cmd) {
		case "tell":
		    if (sscanf(str, "%*s %s %s", cmd, str) != 3 ||
			!(user=user::find_user(cmd))) {
			message("Usage: tell <user> <text>\n");
		    } else {
			user->message(Name + " tell you: " + str + "\n");
		    }
		    str = nil;
		    break;

		case "users":
		    users = users();
		    str = "Logged on:";
		    for (i = 0, sz = sizeof(users); i < sz; i++) {
			cmd = users[i]->query_name();
			if (cmd) {
			    str += " " + cmd;
			}
		    }
		    message(str + "\n");
		    str = nil;
		    break;

		case "password":
		    message("Old password:");
		    state = STATE_OLDPASSWD;
		    return MODE_NOECHO;

		case "quit":
		    return MODE_DISCONNECT;
		}
	    }

	    if (str) {
		if (wiztool) {
		    wiztool->input(str);
		} else if (strlen(str) != 0) {
		    message("No command: " + str + "\n");
		}
	    }
	    break;
	case STATE_LOGIN:
	    if (!DEV_USERD->test_password(name, str)) {
		previous_object()->message("\nBad password.\n");
		return MODE_DISCONNECT;
	    }
	    connection(previous_object());
	    message("\n");
	    wiztool = DEV_USERD->query_wiztool(name);
	    break;
	case STATE_OLDPASSWD:
	    if (!DEV_USERD->test_password(name, str)) {
		message("\nBad password.\n");
		break;
	    }
	    message("\nNew password:");
	    state = STATE_NEWPASSWD1;
	    return MODE_NOECHO;
	case STATE_NEWPASSWD1:
	    newpasswd = str;
	    message("\nRetype new password:");
	    state = STATE_NEWPASSWD2;
	    return MODE_NOECHO;

	case STATE_NEWPASSWD2:
	    if (newpasswd == str) {
	        DEV_USERD->set_password(name, newpasswd);
		message("\nPassword changed.\n");
	    } else {
		message("\nMismatch; password not changed.\n");
	    }
	    newpasswd = nil;
	    break;
	}

	str = (wiztool) ? query_editor(wiztool) : nil;
	if (str) {
	    message((str == "insert") ? "*\b" : ":");
	} else {
	    message((name == "admin") ? "# " : "> ");
	}
	state = STATE_NORMAL;
	return MODE_ECHO;
    }
}
