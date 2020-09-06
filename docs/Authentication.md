# SkotOS Game Authentication

## Types of Authentication

Most SkotOS games are hosted on Debian servers run through Linode. This means that there are three levels of authentication: Linode auth; server auth; and SkotOS auth

### Linode Auth

**Major Functions:** Manage Machine Backups (Snapshot, Restore), Manage Machines (Create, Destroy)

This is controlled through the Linode control panel. You can give people access at "Account > Users > Add A User". Unfortunately, even if they already have an account, you have to create a new "subaccount" for them. 

You can choose what individual permissions they get, though the default is "Full": it will be initially set to "Full" when you create the user, and you must afterward decrease permissions if desired by choosing "User Permissions" for the user and adjusting accordingly.

### Server Auth

***Major Functions:*** Run Processes (Game, Web Server), Edit Files (Game Files, Images, Web Files), Adjust Cloned Repos (Client, UserDB), Manage Game Backups

This is a "root" user who has super-access to the machine. New users are usually added by inputting their IP address into `/etc/hosts.allow` and running `/etc/init.d/ssh restart` and giving them a root password.

### Game Auth

***Major Functions:*** Edit the Game (Tree of WOE), Access the Game (Admin access)

These are users marked as admins in the game. This is managed through the UserDB. Go to the user pages for you game, and choose the support page (e.g. https://login.URL/support.php). Select the user and on their support page, choose "Admin Access" under "Access:". Be sure to click "Change Info"

### Wiztool Auth

***Major Functions:*** Recompile source code for game, Grant Story Points

This is the highest level of access possible, and is very dangerous: user with this level of access can accidentally wreck the game. The access occurs through port X098 on the machine (e.g., 98 + the base port for the game). Its setup is fully described in "Setting Up a SkotOS Host", since it generally requires shell-level access, and its usage is described in "SkotOS Wiztool".

## Inside SkotOS Auth

SkotOS auth usually follows this procedure: the user logs in on the login page and a cookie is set. That cookie is then verified when they enter the game by a variety of UserDB functions.

### Login Page Login

When a user logins in at https://login.URL, the following happens:

1. They type their username and passwd.
2. Their username and passwd are passed to the local UserDB Auth socket with the `passlogin` command.
3. The username is verified.
4. The password is verified against a saved hash with `password_verify`.
5. A random keycode is generated.
6. The keycode is placed in the `keycodes` database with a timestamp.
7. A `user` cookie is set with the username.
8. A `pass` cookie is set with the keycode.

### Web Page Verification

Whenever a user hits a web page requiring authentication, their `user` and `pass` cookies are checked. If they match the information in the database and the timestamp is not expired (which is all verified with a `keycodeauth` command to the auth server), then the user is allowed access to the page.

### Auth Server Commands

Three different commands could be sent to the auth server to verify a user:

#### `keycodeauth` Command

_Game or Web Page_

`keycodeauth` sends a username and a keycode to the server-auth, which checks it with `is_keycode_ok` and returns successfully if it match the info in the database and the keycode is not expired. 
 
#### `md5login` Command

_Game_

`md5login` sends a username and a hash of the username, the keycode, and a slightly secret word to the server-auth, which checks it with `is_hash_ok` and returns successfully if that matches a hash constructed in the same way by the server with its own knowledge of the keycode. It should probably fail if the keycode is expired, but currently may not do so. This is perhaps slightly more secure than `keycodeauth`, but not much, because its security comes through the obscurity of the hash.

This appears to be run when a user does his initial login into a game.

> _A variant of this command called `md5auth` was used by Genesis games, and expects different returns._

#### `passlogin` Command

_Web Page_

This is fully described above: a username and password are sent, and the hash of the password is tested against what's in the `users` database with `password_verify`.

Note that passwords are hashed with `password_hash` and checked with `password_verify`. These are robust PHP functions that are intended to upgrade to new forms of encryption over time: the type of encryption used is stored as part of the hash, allowing full backward compatibility.

> _There is a variant of this called `passwordauth`. Either it or `passlogin` was used by the Genesis games to allow logins to the game with arbitrary MUD clients._