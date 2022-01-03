# Local Mac Configuration

When you run the mac setup script, it will clone various dependent repos that you need - the DGD programming language, a Node.js-based websocket relay and so on. Those will go under your cloned SkotOS repo.

In this configuration we don't bother with a web server to serve static content or act as a reverse proxy. That wouldn't be good for production, but a local dev server should never have more than a few users. If you want to run a game you should be using the Linode configuration or something similar.

## The Setup

SkotOS's DGD server exposes a number of ports, including these:

11098: the old-style DGD telnet port, suitable for admin login and wiztool commands
11070/11071: AuthD/CtlD ports - DO NOT leave these exposed to the internet at large.
11080: the HTTP port for most purposes
11090: the "Tree of WOE" builder tool port, which also works via HTTP
11443: the TextIF-protocol game port -- various SkotOS game clients can connect to this

(NOTE: These are for portbase 11000 -- the portbase can be any multiple of 1000, up to around 30,000.)

The start_server.sh script also runs two websocket relays. They accept incoming websocket HTTP connections and relay the resulting data to raw TCP/IP network ports.

Relay 1 connects incoming websocket port 11801 to DGD's TextIF port 11443. The Orchil websocket-based game client can use this relay as its websocket.

Relay 2 connects incoming websocket port 11802 to DGD's Tree of WOE port 11090. The in-browser TreeOfWoe.html page can use this relay as its websocket.

The local-dev script does *not* re-run the relays, which have been known to die. If your browser clients can't connect, consider re-running start_server.sh to start them up again.

## Persistent Objects and skotos.database

SkotOS uses a sort of VM image or object dump called skotos.database. DGD refers to this as a "statedump." If you update the .c files but use the same statedump, those files aren't recompiled automatically. You can connect to the telnet port and use them wiztool to compile the files or you can stop the server, delete skotos.database and restart the server. If you do a full restart it's going to take 5-10 minutes to recompile everything before your game is usable again.
