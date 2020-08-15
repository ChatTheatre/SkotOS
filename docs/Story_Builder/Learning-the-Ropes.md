# Learning the Ropes

[![hackmd-github-sync-badge](https://hackmd.io/YelqP7JxR3O0YKquoi_5zw/badge)](https://hackmd.io/YelqP7JxR3O0YKquoi_5zw)


> *NOTE TO CO-AUTHORS: This document is intended to be an advance "inside to outside" overview of the SkotOS architecture, starting with the account being given admin priviledges in the ChatTheatre*

So you've been invited to become a staff member in a ChatTheatre, and staff priviledges have been added to your user account.

The next time you login to one of your characters in the ChatTheature client, you will have available new set of special commands prefixed with the `+` symbol. Collectively, we call these the *plus commands* and are only available to ChatTheatre staff. These are in addition to the `@` (aka *"at"*) commands available to all players.

## ChatTheatre Admin Roles

So what are some of the different kinds of ChatTheatre staff roles that need access to these `+commands`? 

* **StoryHosts** have the highest administrative privileges in a ChatTheatre, have priviledges outside of the ChatTheatre engine itself such as the ability to grant and remove staff access, can execute code using the Wiztool admin port, and typically also have administrative control the underlying virtual machines that run the ChatTheatre itself.  These users are the ultimate stewards of a ChatTheatre community, and thus are called *StoryHosts*.
* **StoryCoders** have at least the same staff priviledges you have now, and their focus is on creating new features and scripts for the ChatTheatre. This requires them to have some development skills, in particular in a specials scripting language called *Merry*. But they may also have adminstrative access to the underlying DGD server, which allows them to make fundamental changes to the ChatTheatre using a language called *LPC*. *Merry* is intended to be a "safer" version of *LPC*. Staff priviliges in a ChatTheatre are explicity seperate from those of the underlying DGD and ChatTheatre.
* **StoryBuilders** also have same staff priviledges that you have now, but they focus on building new content for ChatTheatre. Rather than focused on creating new code, they will use tools like the *KarMode* and*Tree of Woe* to create rooms and objects, as well as copy or modify pre-written scripts written by *StoryCoders* for use by ChatTheatre Players.
* **StoryPlotters** also have these same staff priviledges, but instead of being focused on creating persistent rooms or objects they are focused on creating short-term plots and other entertainment for the ChatTheatre Players. They largely use these staff privileges to `+join`, `+summon` and  or `+return` players into plots, `+spawn` or `+slay` one-time objects used by plots, or `+force` or `+posses` objects and adversaries to make them do things that scripts do not (or can't).
* Some larger ChatTheatre's have **StoryGuides**. They also have the same staff priviledges, but they focus on using them to respond to assist requests from other ChatTheatre Players and to facilatate or moderate any problems. In particular, these commands allow the StoryGuide to respond OOC (out-of-character) rather than from their ChatTheater in-player character  bodies.

Note that these terms have evolved from over 20 years of use by games using the SkotOS ChatTheatre engine such as *Castle Marrach*. Each differ slightly from game to game do not need to be used by your ChatTheatre community. For instance *Castle Marrach* no longer has *StoryGuides*. But the point is that there many ways to leverage the `@commands`.

## Some Initial Commands

### who +who and +dark

Type `who` to see what characters are currently logged in and playing in your ChatTheatre. Note now that as an admin you'll see more information than you did as a player, including idle players, other staff, and "dark" staff. Dark staff do not show up in the player `who` command.

Type `+who` to see not only character names, but account names. (this command used to give players IP addresses, but is now broken probably because of Orchil client's HTML5 tunnelling).

Type `+dark` to hide yourself from the player's 'who', and again to make yourself visible. As a staff member you often will be working in some other window, and don't want other players to send you questions or make assumptions about your availability.

### +people and +hotspots

`+people` lists characters logged in by location, grouping those together in the same location.

`+hotspots` is similar, but lists room that contain 2 or more unidle players, thus showing you the more populated areas of the game. This is good for seeing where current activity is, and for directing new players to areas where they can interact. 

### +join, +goto, and +go

You can teleport your current body to the room of another player by using the `+join <player>` command. NOTE: Be careful directling people, or going yourself to the private rooms where only two people are interacting — it maybe an intimate conversation! As a courtesy it is best to `@page <player>` for consent first.

You will notice with the `+hotspots` command, that the rooms are given name with colons in them. These are known as `woe` names, as they are specially named unique objects in the game, in this case, rooms. You can `+goto <woename>` to go to that room.

Finally, there is a more complex `+go` command, where you can give a room a global nickname (i.e. usable by all staff) and be able to teleport there. `+go list` to see what rooms have been given nicknames. Use `+man go` to see how to set nicknames and other features of this command.

### +return and +home

The `+home` command will teleport you to your personal staff room, or a common OOC location for all staff.

`+return` by itself will teleport you to the ChatTheatre has defined your "home" as. For most players this is the room in the game where if you are disconnected from the game for extended time it will send your character's body. 

`+return <playername` will teleport a player to the 



## Your Staff Body

One of the first things that you'll want to do as an new staff is to create yourself another character body to use as your "Staff Body". 

Why?

* It is very easy to accidently destroy or disable your character body with staff commands while learning all that you can do with them. Having an untouched character body to return to recover from mistakes is important.
* In the history of online text games and our own 20 year history of ChatTheatres, it can help to seperate OOC authority as an admin from your the characters you play in the game. Some people try to keep them completely seperate and anonymous, but that is difficult, but at least knowing what hat you are wearing at any one time, and avoiding the "bleed" of authority into play is useful.

Some ChatTheatres do not allow you to have more than one player character body, so you may need some assistance to make one. There may also be rules about how you name them.

When you login all your character bodies will staff priviledges, but your staff body can be set to different defaults, and do things that standard player character bodies can not.

+title

One of the first things you'll probably want to do is to set yourself a title, for instance `+title me "StoryHost"`. 

+chatlines

In addition to OOC `chat` lines that regular ChatTheatre players have access to, staff have acces to additional chat channels. They will vary from ChatTheatre to 





## Your True Name

Like the wizard's of lore, knowing a True Name is one of the keys for understanding how the underlying magic work. The same is true for understanding how a ChatTheatre functions.

First, `+obname me` — you will see that your body has a name, that your body has a parent, and that parent itself also has a parent. This is the "data inheritance" list for your body. Your character body has its own unique core properties, but many of these core properties come from one of its parents, or is modifed or overriden by one of the children.

You can see all the core properties on your body (quite long) with the `+stat me "property:*"` command. You can't tell from this listing where each comes from, or what it does, but all were established when your particular body was created. 

If you click on `[V]` next to the name of your body, it will execute the command `+tool woe view %<woename>` which will open a window view inside of Woe (aka the "Tree of Woe" as once in the past it was extremely slow).  

You will now see all the details in your specific object, of the type `Base:Thing`. There are many values here, and one of the subsections is the a list of `Core:Properties` where you can see many of the core properties defined in this specific `Base:Thing` object. But note that there are a number of other properties that are organized or set on this page that you can't delete, only change their values.

Near the top of this list you will se a link to this specific object's  parent in the `Ur:UrParent` field. Click on that object link and now you will be taking to your bodies parent object. Click in it's `UrObject` field to get to it's parent. Keep on going until you get to top. This is the "UrCharacter". This is the top of the data hierarchy. Many values fall from here, down through each child, until they reach your particular character body.

However, there is something higher than this, which is `Base:Thing` which is a developer object. As everything object in the ChatTheatre game world is a `Base:Thing`, you can't edit it directly, you have to edit it at the underlying SkotOS level. It also has all of the core "methods" for `Base:Thing` that various programming functions call. 

In fact, your body as an object can be and do everything possible that any other object in the game can be or do. You are in effect also a room (if you are big enough you can contain things, and if you add an entrance or exit and they are large enough, players can enter you). If you set the property flag "Base:Misc:Edible" to true, you can be eaten, a little at a time, until you are gone. If set the property flag "Base:Misc:Potable" you can be sipped upon.

As an example of this, lets `+spawn` a new object based on a very simple, generic chair. `+spawn Ur:furniture:chair-wood-oak-comfortable`  — this will add a chair to your inventory. `drop chair` , and `look chair` and `examine chair` it. Then `+obname chair` to see the Woe name of this chair. 

Note that this time the object does not have a specific name, It is spawn, and it's full name is `[Ur:furniture:chair-wood-oak-comfortable]#519351` Those last numbers are its object number. You will not see this object in the standard Tree of Woe since it is just a copy of its parent, but you can click the `[V]` in `+obname` to actually see open the Tree of Woe see this specific object. Here you can even rename this — it is still a spawn, but once named you to be able to find it in the Tree of Woe, and modify it if you like.

Now to demostrate that all objects are in some ways the same, go to the Woe view of your chairs' parent. You will see it has details for arms and legs. Let's try them out!

Now `+possess chair`. You are the chair! Try `wave arm`, `kick bed with my leg`. Now `sit on floor` and go back to your staff body `+possess <name>`. If you now `look` in the room, you will see that the chair is sitting on the floor.

Go back to `+obname me` and you will see that you have been given a specific Woe name. Many commands work with these names, but in, fact, like the chair, you body has a deeper name. Do `+obstat <mywoename> name`. You'd get a result like ` /base/obj/thing#95200` — this is your body's true object name.

However, like all wizardry, this is not the whole truth. You, the player, are inhabiting this character body, but you also just recently inhabited the chair? Where is the real you?

If you now `+stat me "property:udat:object"`

You will now see that your "user data", aka `udat` is `/usr/UserAPI/obj/udat#72319`. 

Now `+obstat <myutdat> property` to see all the properties associated with your USER object. 

So in effect, this `udat` is your True Name.

## Modifying you Staff Body 

+stat me "property:appearance:*

+setp me ""


Grab some ideas from http://wiki.marrach.com/bin/view/Coders/CustomiseYourStaffBody


+setp

+man return

virtualhome:home

+stat me "property:udat:object"

Property: udat:object = </usr/UserAPI/obj/udat#72319>



+tool woe view %Neoct:Staff:Verbs:+obname

+tool woe view %Neoct:Data:Doc:obname



+tool woe view TextIF:Init


## Customize your Staff Office

grab ideas from:

http://wiki.marrach.com/bin/view/Coders/CustomizeYourOffice

http://wiki.marrach.com/bin/view/Coders/KarMode

## Creating your first Object

grab ideas from: 
http://wiki.marrach.com/bin/view/Coders/CreateYourFirstObject

http://wiki.marrach.com/bin/view/Coders/SpawnVsSummon



## Creating your first script


## Other resources

http://wiki.marrach.com/bin/view/Coders/+Commands

+man

Orchil theme issues http://forum.skotos.net/forum/our-games/castle-marrach/castle-marrach-discussion/1780929-orchil-client


## Creating an NPC

+obname butler

+stat butler "property:nip:*

+tool woe view %Kalle:nip:comm:butler

