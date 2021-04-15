# include <System.h>
# include <kernel/kernel.h>
# include <kernel/user.h>
# include <kernel/rsrc.h>

# include <UserAPI.h>

inherit LIB_USER;
inherit user API_USER;
inherit rsrc API_RSRC;

inherit authapi "/usr/UserAPI/lib/authapi";

inherit CLONABLE;


# define STATE_NORMAL           0
# define STATE_LOGIN            1
# define STATE_WAITING_LOGIN    2
# define STATE_LOGOUT           3

string name;            /* user name */
string Name;            /* capitalized user name */
object wiztool;
object post_login_conn;
int nconn;              /* # of connections */
int state;

object query_wiztool() { return wiztool; }

/*
 * NAME:        create()
 * DESCRIPTION: initialize connection object
 */
static create(int clone)
{
    if (clone) {
        user::create();
        rsrc::create();
        authapi::create();
    }
}

/*
 * NAME:        login()
 * DESCRIPTION: login a new user
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
 * NAME:        logout()
 * DESCRIPTION: logout user
 */
logout(varargs int quit)
{
    if (previous_program() == LIB_CONN && --nconn == 0) {
        ::logout();
        destruct_object();
    }
}

/*
 * NAME:        receive_message()
 * DESCRIPTION: process a message from the user
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
            post_login_conn = previous_object();
            start_passlogin("finish_auth", name, str, name);
            state = STATE_WAITING_LOGIN;
            return MODE_ECHO;

        case STATE_WAITING_LOGIN:
            /* Input should not be received here - awaiting AuthD response */
            post_login_conn->message("\nWaiting on login...\nInput ignored: " + str + "\n");
            return MODE_ECHO;

        case STATE_LOGOUT:
            return MODE_DISCONNECT;
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

static
void finish_auth(int success, string reply, string user) {
   object udat;

   if (success) {
      string account_type, account_flags;

      /* In case of success we ignore the reply - if you haven't agreed to the TOS, we don't care. */

      udat = UDATD->get_udat(user);
      if (!udat) {
         error("oh no! no udat when logging in as developer!");
      }
      if (sscanf(reply, "%*s(%s;%s)", account_type, account_flags) == 3) {
         udat->update_account(account_type, explode(account_flags, " "));
      }

      if (!DEV_USERD->query_wiztool(user)) {
         post_login_conn->message("This user is not a developer! No dev port login for you!\n");
         state = STATE_LOGOUT;
         return;
      }

      state = STATE_NORMAL;
      connection(post_login_conn);
      message((name == "admin") ? "\n# " : "\n> ");
      wiztool = DEV_USERD->query_wiztool(name);
   } else {
      post_login_conn->message("\nBad password.\n");
      state = STATE_LOGOUT;
   }
}
