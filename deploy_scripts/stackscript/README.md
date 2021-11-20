# SkotOS StackScript installation

In this directory, you'll find files to install core SkotOS on a Linode for production usage.

The basic StackScript is in linode_stackscript.sh.

You can copy it into Linode and run it by itself. You can also copy it somewhere and run it from somewhere else.

Like basically all StackScripts, it expects to be run as root with some environment variables set. They're documented at the front of the script in normal Linode style.

Ordinarily the StackScript will clone a SkotOS repository, such as this one, and gets its various attendant scripts (e.g. start_dgd_server.sh) from there. Please note that some of these scripts are written for a "bare" SkotOS installation and so they may do the wrong thing if you're running in a different configuration.

## Invoking the Script as Part of a Full SkotOS Game

What if you're using SkotOS as part of a larger SkotOS-based game, like [The Gables](https://github.com/ChatTheatre/gables_game) does?

In that case this script is still useful but it will need to be invoked differently. You can see [The Gables' Linode script](https://github.com/ChatTheatre/gables_game/blob/master/app_stackscript.sh) for more details. But the short version is "your StackScript can download this StackScript and run it, then make any additional modifications you like."

There are also options your script can set to change how this one operates. All of that is done through environment variables, as you'd expect.

## The Setup

A production SkotOS server has a bit more complicated setup than a local development server. Here's how that's set up.

There is still a DGD server, as you'd expect, and it runs with the same port numbers. A production server also uses NGinX for HTTPS termination &mdash; everything should run on encrypted connections, and NGinX makes that possible.

There is a PHP-based account server called thin-auth located in /var/www/html/user. It runs via NGinX and FPM (a PHP application server.) In addition to the PHP web app, it runs two PHP servers called server-authd and server-ctld. These are SkotOS-specific authentication servers. The DGD-based game server contacts these servers for login and account information.

Because the PHP authentication servers only expect incoming connections, and the DGD server only expects incoming connections, a small shim server is needed to connect one to the other. This shim server is called userdb-authctl. It's in this directory.

SkotOS's DGD server exposes a number of ports, including these:

11098: the old-style DGD telnet port, suitable for admin login and wiztool commands
11070/11071: AuthD/CtlD ports - DO NOT leave these exposed to the internet at large.
11080: the HTTP port for most purposes
11090: the "Tree of WOE" builder tool port, which also works via HTTP
11443: the TextIF-protocol game port -- various SkotOS game clients can connect to this

There are two websocket relays set up to run continuously (and restart if needed):

Relay 1 connects incoming websocket port 11801 to DGD's TextIF port 11443. The Orchil websocket-based game client can use this relay as its websocket.

Relay 2 connects incoming websocket port 11802 to DGD's Tree of WOE port 11090. The in-browser TreeOfWoe.html page can use this relay as its websocket.

NGinX is also used for HTTPS-terminating the connection to DGD. Port 11803 accepts incoming HTTPS connections and uses DGD's port 11080 as the backend. This permits the DGD server to handle https URLs.

A variety of scripts are run frequently via cron to keep all of these servers running and healthy.

## Logfiles

All of these servers can have difficulties, hiccups and even just status messages to reflect normal functionality. The script show_all_logs.sh in this directory, when run, will continuously show all updates to the relevant logfiles.

Or you can just look at the very simple script and that will give you a list of the logfiles to go look at. That's fine too.

## Persistent Objects and skotos.database

SkotOS uses a sort of VM image or object dump called skotos.database. DGD refers to this as a "statedump." If you update the .c files but use the same statedump, those files aren't recompiled automatically. You can connect to the telnet port and use them wiztool to compile the files or you can stop the server, delete skotos.database and restart the server. If you do a full restart it's going to take 5-10 minutes to recompile everything before your game is usable again.

This primarily matters if you want to upgrade the underlying DGD code and/or WOE objects while keeping your game's basic data (e.g. accounts, characters, items) intact. Currently that's a tricky operation and you should consult the documentation when doing so. SkotOS has some unusual upgrade issues due to its persistent nature and how it deals with statedumps.
