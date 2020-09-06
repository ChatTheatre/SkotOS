# SkotOS Wiztool

# About Wiztool

Each of the games has a "wiztool" port at XX98 (e.g. 6098 for The Gables), which can be used to access a number of functions that sit between the [Layers](./Layers.md) of DGD's Kernel Library functionality, and the underlying SkotOS LPC code that runs the ChatTheatre.

You should rarely, if almost never need to use this functionality, and you should be aware that you have the opportunity to unretrievably destroy the ChatTheatre. (The Gables VPS instance is backed up every evening at 11:05pm PDT, so some can restore back to that point, but all work by staff and all player changes can be lost.)

## Setting up access to the Wiztool

Before you can be given access to the Wiztool, you'll need to make sure that your staff ChatTheatre user account is marked as "Developer", and you will need to add that account name to the "System:Developers" list in the Tree of WOE. You probably can do this yourself as you should already have staff permissions.

For information on how an existing StoryHost can set up access for your account to access the Wiztool, see [Setting Up a SkotOS Host]().

## Accessing the Wiztool

Don't do this unless you have a specific need and know what you're doing!

From an IP address that that has authorized access, you `ssh <shellaccount>@<game.theatre.domain.ext`., or in the case of gables `ssh <shellaccount>@game.gables.chattheatre.com`.

Once you are logged in to the server, all the SkotOS files are located at `/var/skotos/XX00/skoot/` (e.g. for the Gables `cd /var/skotos/6000/skoot`). Files modified here will not actually make changes in the ChatTheatre itselft — you'll need to execute them from the SkotOS Wiztool.

For wiztool access you `telnet game-URL XX98`, for example `telnet game.gables.chattheatre.com 6098`.


## Wiztool Commands

### > help

```
Available commands in the wiztool:

Coding / compiling / upgrading:
  upgrade   issues    unfreed   leaves    patch     possess   depossess
  command   dbstatus  debug     clones    code      history   clear
  compile   clone     destruct  clonetree inherited inherits

Files / directories:
  cd        pwd       ls        cp        mv        rm        mkdir
  rmdir     ed

Access / resources:
  access    grant     ungrant   quota     rsrc

Miscellaneous:
  people    status    swapout   statedump shutdown  reboot

````
### > access
```
> help access
Command:     access
Synopsis:    access [<user>]
             access <directory>
             access global
Description: Shows what access you or someone else has.
See also:    grant, ungrant
```

### > help code

```
Command:     code
Synopsis:    code <LPC-code>
Description: Allows you to execute LPC code without having to specifically
             create an object, as this has been automated for you.  For your
             convenience the files <float.h>, <limits.h>, <status.h>,
             <trace.h> and <type.h> are automatically included and there are
             26 variables (a..z) of type mixed to use.
See also:    clear, clone, compile, history
```


## Recompliling Code

If you ever need to make a change to the DGD/LPC code, this is how you do it! YOu edit the file, and then you log in to the admin port and run compile /path/name/under/skoot

> compile "/usr/Theatre/sys/portal"
Don't do this unless you really know what you're doing!

Adding SP Permissions

You can also run specific DGD/LPC code from the admin port by indicating a file name and a function.

A few specific functions:

#### Set Wiztool account password

You can set a wiztool account password with
```
>"code "~System/sys/devuserd"->set_password("name", "pass")
```
where the name is their account name and the pass their unique password for the wiztool admin port.

#### Storypoints Grant Access

This will allow you to grant story-point-granting permissions to certain staff accounts:

This allows you to list current SP granters:

> code "/usr/TextIF/sys/storypoints"->query_accounts() 
$171 = ({ "alice", "bob" })
You can add people by referencing that return ($171):

> code "/usr/TextIF/sys/storypoints"->set_accounts($171 + ({ "charlie" }))
$172 = nil
You can similarly remove people:

> code "/usr/TextIF/sys/storypoints"->set_accounts($171 - ({ "bob" }))
$172 = nil
Or you could just bypass all of that and create a totally new list of people:

> code "/usr/TextIF/sys/storypoints"->set_accounts(({"charlie", "dan"}))
$172 = nil


