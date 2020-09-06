# Staff + Commands

## NOTE:

Many of these commands were game-specific, in particular for Castle Marrach, so many will need to be tested, considered for deletion or fixed, renamed to be in the the correct place in Woe, and all need to be documented better (in particular, where their code resides.) 

## COMPLETED

## REVIEW IN PROGRESS

#### +dark
   * Usage: `+dark`
   * Decription: Hides/unhides your name from the who list. When you are in the hidden state (+darked), you will appear as invisible to regular account bodies, on the who list. To anyone a guide or a host (coder, plotter, host, et al.), you will still appear on the who list under the category "Characters dark:" or "Characters dark and idle:"
   [ ] See also: TBW (who, or if renamed maybe future @who)
   * Origin: `+to wo ed %Neoct:Staff:Verbs:+dark`
   * Author: Written by Jess Brickner [revised 1072052436 December 22, 2003 @ 12:20:36 am UTC]
   * +command FUNCTIONAL
   [+] +man DOCUMENTED [last modified Tue Sep 14 04:43:57 2004], but needs See also:

#### +owners

   * Usage: +owners "woename" 
   * Description: Returns a list of children/spawns of that woename and where it is in game. The listing is done by the woename of each child/spawn and woename of the  locations. 
   This command is used to find the owner of all the instances of an object. 
   [last modified Fri Jul  8 06:20:59 2005] 
Examples:
   +obname "Generic:clothing:robe-dressing" 
See also:
   +whereis 
   Origin: `+to wo ed %Neoct:Staff:Verbs:owners`
   * Author: Written originally by kargh (Jess Brickner?) [last revised Monday, June 5, 2006 8:42:03 PM]
   [+] +man DOCUMENTED
   [!] +command FUNCTIONAL, but Woe name doesn't include +, breaking convention. As there are some databases invoved, renaming may not be easy.


#### +who
   * Usage: +who
   * Description: Lists accounts, ip names, and characters of those logged in
   * See also: See also +customers, +hotspots, +staff, +people 
   [ ] ORIGIN: This is probably an original + command, not Merry. Unconfirmed.
   * Author: Unknown
   [!] +command FUNCTIONAL BUT NEEDS WORK
   [+] +man DOCUMENTED
   * Note: The Marrach version of this command also shows staff that are +dark. Thus it is likely there is a Merry version of +who on Marrach that is not on Gables.
   * Note: Currently "Hostname" column is just "game", probably because of Orchil logins rather than direct logins. Columns do not line up. need to puzzle through solution.

#### +woe
   +woe[last modified Sat Sep 18 06:45:16 2004] 
   * Usage: +woe <string>|abort 
   * Description: Search through the Woe and display every object whose name contains the supplied string. (Note that searching for "food meat" will not search for the words "food" and "meat" separately, but will instead search for any objects with "food meat" in the name.) The command will continue the scan until either 1) it has gone through Woe, 2) you begin a new search, or 3) you type +woe abort. 
   * Examples:
   
```   
   +woe horse
        Search for any woename whose name contains the word "horse"
   +woe abort
        Abort the current +woe search. 
```



  
   
   
### Inconsistencies

#### who @who +who
   * who looks like a merry command, and hides staff in dark. @who shows what you call yourself and your stance and prox. +who appears to be an old non-Merry internal command, doesn't show IP address anymore, but does give account name. Consider changing who to @who, and find another verb for what @who does now. Consider deleting +who.


## TO BE REVIEWED

#### +ac
   * Usage: Alchemy Chat
   * An unused chat line

#### +accfinger 
   * Usage: +accfinger [accountname]
   * Pulls up information about a players account
   * For more information, type: +man accfinger

#### +action
   * Usage: Locate an action and return its handler(s).
   * Seemingly not fully implemented.

#### +activate
   * Usage: unknown
   * Seemingly not fully implemented. 

#### +adddesc
   * Usage: +adddesc [type] [detailID] "[text]" 
   * Adds an additional description line to a room.
   * For more information, type: +man adddesc

#### +addnote 
   * Usage: +addnote [accountname] "[message]"
   * Adds a note to a user account
   * For more information, type: +man addnote

#### +addshout
   * Usage: +addshout
   * Allows 'shouting' between non-exit details.
   * Opens the "Shout" pop-up on Skotos-supported clients.
   * For more information, type: +man addshout

#### +advapp
   * Usage: Type '+advapp h' for help on the command-line syntax.
   * Allows the creation of "Choose Your Own Adventure" action sets.
   * Opens the CYOA Popup on Skotos-supported clients.
   * See Troll/Builders TWiki for more information.

#### +adverb
   * Usage: +adverb "[adverb]" 
   * Modify the adverbs in the game.
   * For more information, type: +man adverb

#### +aim
   * Usage: unknown
   * Seemingly not fully implemented. 

#### +apropos
   * Usage: this command scans the available +man documentation, attempting to locate all pages which contain the specified keywords.  The command and the truncated beginning of the description is the displayed to the user. 
   * For more information, type: +man apropos

#### +areport
   * Usage: +areport [report] 
   * displays online activity reports for various subjects.
   * Type +areport alone for usage information.

#### +article
   * Usage: allows modification or addition to the online help articles available through the @article command.
   * Opens the Article pop-up window on Skotos-supported clients.

#### +ask
   * Usage: unknown
   * Seemingly not fully implemented. 

#### +assist
   * Usage: +assist [various]
   * Displays the "New" TAS queue.
   * Equivalent to typing +tas alone.

#### +attack
   * Usage: n/a
   * Part of unused Theater Combat System.

#### +awakeners
   * Usage: Show all awakeners online at the moment.
   * For more information, type: +man apropos

#### +badname
   * Usage: +badname [name] 
   * Add a name to the badnames database. Any names declared as 'badnamed' are rendered invalid to any user attempting to create a new character with that name. 
   * For more information, type: +man badname

#### +badnames
   * Usage: +badnames 
   * Display all names declared as bad. Long list.
   * For more information, type: +man badnames

#### +behead
   * Usage: n/a 
   * Used for executions.
   * Development incomplete. (See +maim for current development track.)

#### +bilbodebug
   * Usage: Nothing. Bilbo is evil.
   * You heard me the first time.

#### +bodies
   * Usage: +bodies [account] 
   * Lists the name, Woe name, and Brief name of all bodies on an indicated account.

#### +bring
   * Usage: +bring charname
   * Brings the character, logged in or not, to your location
   * +unbring returns them to that location.

#### +broadcast
   * Usage: +broadcast "Message"
   * Sends a message to the entire game world/server. Does NOT show your name so if you want it to be known, it should be added into the message
   * See also +emitto, +emitin , +emitub, +emitib, and +emitob

#### +bugs
   * Usage: +bugs
   * Shortcut used to bring up the bugs queue of the TAS system

#### +burn 
   * Usage: +burn target
   * "Burns" the target, whether or not you have something to burn it with. Shows emits of you burning the target. 

#### +busy
   * Usage:+busy [on|off]
   * Blocks (on) or unblocks (off) incoming pages

#### +cast
   * Usage: incomplete implementation associated with Wizardry.

#### +catalog
   * Usage: administrative interface to the Virtual Catalog system.

#### +cc
   * Usage: +cc (brings up helpfile)
   * This is the coders chatline, which is cross game meaning it is heard by those on other servers. Look at helpfile for the various chatline commands

#### +challenge
   * Usage: admin tool for administering Squires, Knights, and other challenges.
   * Type +challenge alone for help prompts.

#### +chat
   * Usage: admin interface to the Newlie Chat line.

#### +chatlib
   * Admin interface for chatlines.

#### +chatlines
   * Usage: +chatlines
   * Shows you a list of all chatlines you have access to and which ones you have turned on or off
   * Same as +listchats

#### +chatwipe
   * Usage: Remove a person by account from all chatlines in the system.
   * For more information, type: +man chatwipe

#### +chuck
   * Usage: unclear - apparently protype for the 'throw' verb

#### +clearprop
   * Usage: clears the indicated property in a character.
   * equivalent of using +setp/+setp2 to set a property to nil

#### +cloakcheck
   * Usage: command used by SPs to track usage of disguise cloaks
   * Outdated. Does not track current cloaks.

#### +clone [target]
   * Usage: completely duplicates the target object.
   * Works only with Generic:* based objects, not MGeneric:*
   * Property inheritance issues.

#### +cobj
   * Usage: most often +cobj object "Woename" though +cobj shows you a list of other uses
   * Creates a brand new object in WOE for you

#### +compose
   * Usage: test command for @compose
   * Should be deprecated. 

#### +cprof
   * Usage: +cprof (brings up help file)
   * This allows you to set "profiles" for your chatlines. You could for instance turn off all chatlines but TAS and then store it with +cprof with the name ASSIST. Then if you want to turn off all chatlines but the TAS line, you'd use this command to pull up this profile preference.

#### +create
   * Usage: tool intended to allow for creation of complex objects with external relationships.
   * Tool creation incomplete.

#### +createinvite
   * Usage: tool to create multiple IC event invitations and put them directly in the scroll queue.
   * Unclear if development complete.

#### +cron
   * Usage: admin tool for the cron daemon script scheduler. 

#### +customemote
   * Usage: Nothing. Bilbo is evil.
   * You heard me the first time.

#### +customers
   * Usage: +customers
   * Will show everyone logged into the game who is not NDA staff

#### +cyoa
   * Usage: activate a CYOA script set.
   * Use +advapp to create a CYOA (Create Your Own Adventure) script set.

#### +dark
   * Usage: Hides/unhides your name from the who list

#### +deactivate
   * Usage: unknown
   * Seemingly not fully implemented. 

#### +death
   * Usage: +death target
   * Kills the target dead.

#### +defunct [name]
   * Usage: Move a deleted character into the special 'defunct' account, effectively undeleting them. 

#### +delays [target]
   * Shows the #delays() running in the target.

#### +demote
   * Usage: incomplete command designed to manage promotions/demotions

#### +detail
   * Usage: +detail [target->object] "[detail-ID]:[sname]:[pname]:[adjective]:[brief]|[look]|[examine]"
   * Adds or modifies the detail with detail-ID on the target object. Good for setting a detail in-game, if you aren't wanting to get into anything more than the arguments available. Merryized.

#### +delays
   * Usage: +delays cnpc
   * Used to tell if a NIP CNPC is turned on or not. You may use the brief in this, i.e. +delays snake. If you get several responses saying "heartbeat", the CNPC is on. If it is not on, it will tell you there are no delays running on it.

#### +diff
   * Usage: Display diffs between two properties.
   * Experimental - see Kalle.

#### +disconnect
   * Usage: +disconnect name
   * Terminate the target character body's connection to the server.
   * Requires you to be in same room as target.

#### +disfavor
   * Usage: SP command that allows the viewing of Favor lists on other characters remotely.
   * See also +favor
   * For more information, type: +man disfavor

#### +disguise
   * Usage: unclear/unknown
   * Source not found in Woe.

#### +distrosys
   * Usage: admin tool for the MGeneric distribution system
   * BROKEN - distrosys disable. 

#### +drag
   * Usage: +drag character
   * Lets you restrain the character so you can drag him/her from the room. +drag nobody to release them
   * OOC equivalent of restrain.

#### +drink
   * Usage: unknown
   * Seemingly not fully implemented. 

#### +drinkfull
   * Usage: unknown
   * Seemingly not fully implemented.  

#### +drinklarge
   * Usage: unknown
   * Seemingly not fully implemented.  

#### +drinksmall
   * Usage: unknown
   * Seemingly not fully implemented. 

#### +drinktaste
   * Usage: unknown
   * Seemingly not fully implemented. 

#### +echo
   * Usage: unknown
   * Source not found in Woe.
   * Possibly related to +emit?

#### +echoall
   * Usage: unknown
   * Source not found in Woe.
   * Possibly related to +emitall?

#### +effects
   * Usage: Opens a pop-up tool that allows creation of an Effect.
   * See "Effects" on Troll/Builder.

#### +emit
   * Usage: +emit message (no quotes)
   * Sends an emit to your environment

#### +emitall
   * Usage: +emitall message (no quotes)
   * Sends an emit to everyone in the game

#### +emitallfemale
   * Usage: +emitfemale message (no quotes)
   * Sends an emit to all female characters in the game

#### +emitallmale
   * Usage: +emitmale message (no quotes)
   * Sends an emit to all male characters in the game

#### +emitallnewly
   * Usage: +emitallnewly message (no quotes)
   * Sends an emit to all new characters in the game

#### +emitapp
   * Usage: +EmitApp is a tool to create advanced emit schedules. It has a minor array of tools available through a dialog system using the popup-technology of SkotOS. 
   * For more information, type: +man emitapp

#### +emitfemale
   * Usage: +emitfemale message (no quotes)
   * Sends an emit to your environment, females only

#### +emitib
   * Usage: +emitib message (no quotes)
   * Sends an emit to the IB only

#### +emitin
   * Usage: +emitin "([delay]) [room->woename] [text]" 
   * Emit a message into a specific environment. Optionally, wait a specific amount of time before emitting by specifying the amount of seconds before the woename of the environment.

#### +emitmale
   * Usage: +emitmale message (no quotes)
   * Sends an emit to your environment, males only

#### +emitnewly
   * Usage: +emitnewly message (no quotes)
   * Sends an emit to all new characters in your environment

#### +emitob
   * Usage: +emitob message (no quotes)
   * Sends an emit to the OB only

#### +emitter
   * Usage: Opens a pop-up tool that allows the creation of scheduled emitter scripts in designated environments.

#### +emitto
   * Usage: +emitto [person 1], [person 2], [person 3], [...]: "[text]"
   * Emit a message to one or several characters.
   * For more information, type: +man emitto

#### +emitub
   * Usage: +emitub message (no quotes)
   * Sends an emit to the UB only

#### +emote
   * Usage: +emote message (no quotes)
   * Sends an emote to the room prefaced by your name. +emote smiles will come up <nop>StoryCoder Geli smiles.

#### +erasescripts
   * Usage: unknown
   * Seemingly not fully implemented.  

#### +exits
   * Usage: unknown
   * Seemingly not fully implemented.  

#### +finger
   * Usage: +finger character
   * Gives you same information as +accfinger, except you use a character name instead of account name
   * For more information, type: +man finger

#### +force
   * Usage: +force object "action"
   * Forces an object/character to perfom an action. Actions should be written as you would write them into the parser. i.e. +force table "stand in middle" or +force jerboa "go west"
   * For more information, type: +man force

#### +free
   * Usage: +free character
   * OOC tool to free someone from the pillories when the code is acting up.

#### +freebubble
   * Usage: +freebubble character
   * OOC tool to free someone from the stick-bubbles when the code is acting up.

#### +gaitify
   * Usage: +gaitify verb
   * Turn a singular verb into a gait verb, enabling the "walk happily through EXIT" feature. 
   * For more information, type: +man gaitify

#### +gc
   * Usage: +gc (brings up helpfile)
   * General Chat line, server specific, for all staff including plotter trainees and guides. (Non NDA)

#### +get
   * Usage: +get object
   * EXPERIMENTAL - GIMLI codebase implementation by Kalle
   * Check with Kalle before using.

#### +gjoin
   * Usage: unknown
   * Purpose unknown
   * Not found in Woe.

#### +go
   * Usage: +go
   * This command has combined 3 previous commands. It works as follows:
      * Teleport/move into a specific object, specified by its name in the system.
      * Teleport to another character or object.
      * +go is used to store/return to areas. By prefixing the area name with a +, 
   you are adding it to the list of go-able places. Omitting the + means you wish 
   to go to the place previously defined as <name>.
   * WARNING: All entries are global for the server.
   * IMPORTANT for more information, type: +man go

#### +goodname
   * Usage: +goodname "name" 
   * Remove a name from the badnames database.
   * For more information, type: +man goodname

#### +goto
   * Usage: +goto woename
   * Teleport/move into a specific object, specified by its name in the system. 
   * For more information, type: +man goto

#### +grab (deprecated - do not use)
   * Usage: +grab character
   * Brings that character to your environment if they are logged on and stores 
   the position from where they were grabbed. If you use +grab on a character that has been grabbed before without being 
   resent to their stored location, the original location will remain stored. 
   * IMPORTANT - Note that +grab is deprecated and that you should use +bring instead unless you know what you are doing. 
   * See also +ungrab
   * For more information, type: +man grab

#### +grantvisions
   * Usage: +grantvisions
   * Allows everyone in the current room to see the newly visions for two days.

#### +guides
   * Usage: +guides
   * Shows which guides are logged in

#### +heal
   * Usage: +heal character
   * Resets Fatique to 100.

#### +healers
   * Usage: +heakers
   * Display all healers who are online, and whether or not they are wearing their healer's pins.
   * For more information, type: +man healers

#### +healfull
   * Usage: +healfull character
   * This command heals the target character - wounds and bruises - in their entirety. The command cannot heal partially, sorry.

#### +helene
   * Usage: unclear
   * Uknown command created by Kalle for an unclear purpose...
   * Eh?

#### +helppopup
   * Usage: +helppopup
   * EXPERIMENTAL tool created by Kalle to inteface with the Help system.
   * Status unknown.

#### +home
   * Usage: +home
   * Teleports you to where the server designates as "home". In Marrach, this is the unused Green room.
      * This should be reset to main staff office. (Will follow-up with Nino.)

#### +hop
   * Usage: hop [+|++|-] name
   * +hop is used to store/return to areas. By prefixing the area name with a +, you are adding it to the list of hoppable places. Omitting the + means you wish to hop to the place previously defined as <name>.
   * For more information, type: +man hop

#### +hosts
   * Usage: +hosts
   * Lists all NDA staff that is logged on. Lists account name, character name, idle time, and woename of location. 
   * For more information, type: +man hosts

#### +hotspots
   * Usage: +hotspots
   * Lists rooms that contain 2 or more unidle bodies, showing you the more populated areas of the game. Good for directing new players to areas where they can interact
   * For more information, type: +man hotspots

#### +household
   * Usage: +household
   * Admin command for setting up Noble household suites, and adding or removing members of that household.
   * For more information, type: +man household
   
#### +ideas
   * Usage: +ideas
   * Shortcut to pull up the ideas queue in TAS. Same as +tas ideas
   * For more information, type: +man ideas

#### +idleness
   * Usage: +idleness [watch] name ["message"] (or just +idle)
   * Find out when a person submitted a command last. The command additionally allows you to keep a tab on the person that will stay there until they either disconnect or return from idleness.
   * For more information, type: +man idleness

#### +info
   * Usage: +info account name
   * Display information about all bodies logged in by a particular account.
   * For more information, type: +man info

#### +infoline
   * Usage: +infoline on/off or +infoline recall
   * When line is turned on, it notifies you every time someone logs on, logs out or disconnects. Using the recall will show you the most recent of these notices
   * For more information, type: +man infoline

#### +inspect (do not use)
   * Usage: +inspect
   * Used by SP in relation to particular plot/item
   * Ignore

#### +integrate
   * Usage: +integrate (brings up helpfile)
   * Integrates an object into another object, hiding it from view but still making it interactable. Used mainly with furniture to keep the looks of rooms uncluttered. If you use the syntax that calls for a woename, it will automatically spawn that object and then integrate it
   * For more information, type: +man integrate

#### +inventory
   * Usage: +inv (optional $character/$here)
   * +inv alone will produce the inventory of the body you are in. +inv $here will produce the inventory of the room you are in. +inv $character will give you the inventory of that character whether they are logged in or not. +inv $character.container (i.e. +inv $geli.bag) will give you the inventory of that container. 
   * For more information, type: +man inventory
   * __Please do not abuse this command!!!__

#### +ipnames
   * Usage: +ipnames
   * Displays all the ip names for everyone online
   * For more information, type: +man ipnames

#### +ipsearch
   * Usage: +ipsearch (brings up helpfile)
   * Allows you to search similar ip names to help track down trial account abusers
   * For more information, type: +man ipsearch

#### +join
   * Usage +join character
   * Usage +join woename
   * Teleports you to that character. They need not be logged in, but their body must be in an environment other than NIL.
   * For more information, type: +man join

#### +kawho
   * Usage: EXPERIMENTAL
   * Kalle project - do no use.

#### +kelen
   * Usage: EXPERIMENTAL
   * Do no use.

#### +kemit
   * Usage: EXPERIMENTAL
   * Experimental version of +emit. DO NOT USE.
   * Crashes on use.

#### +kemitall
   * Usage: EXPERIMENTAL
   * Experimental version of +emitall. DO NOT USE.
   * Crashes on use.

#### +kemote
   * Usage: EXPERIMENTAL
   * Experimental version of +emote. DO NOT USE.
   * Crashes on use.

#### +keycode
   * Usage: +keycode door target
   * +keycode is used to determine which locks (exits) a particular key works on, or which keys a particular lock handles. It can also be used to detect invalid settings in locks (for instance the exit pair between two rooms not having the same keycodes).
   * For more information, type: +man keycode

#### +killnote
   * Usage: +killnote account
   * Kill the last entry in the specified account's notes. 
   * For more information, type: +man killnote

#### +kpe
   * Usage: +kpe
   * Experimental fork of the Property Editor on Marrach.
   * Not properly implemented (yet).

#### +kwhereis
   * Usage: +kwhereis name|woename
   * Testing version of +whereis.
   * Use +whereis.

#### +lara
   * Coder experimentation
   * Ignore

#### +lastseen
   * Usage: +lastseen character
   * Shows if that character is online or offline, their idle time if they are online, and when that character last connected and disconnected. It also shows the average time spent online per connect. 
   * For more information, type: +man lastseen

#### +leads
   * Usage: +leads (brings up helpfile)
   * Team Lead chat, server specific, Lead staff only

#### +lfc
   * Usage: +lfc (brings up help text)
   * The LFC (log feed channel) exists to provide the developers with an extended tool for debugging runtime systems. The LFC uses the chatlib code with a series of debugging- and filtering-specific extensions.
   * For more information, type: +man lfc

#### +lg 
   * Usage: +lg (brings up helpfile)
   * Local Game chat, server specific, NDA staff only

#### +list
   * Usage: Unknown
   * Command causes crash.

#### +listchats
   * Usage: +listchats
   * Display a list of all chatlines, your current body's status (on/off), the purpose of each line and whether it is cross-game (multi-server). 
   * Same as +chatlines.
   * For more information, type: +man listchats

#### +listobjects
   * Usage: +listobjects
   * Displays characters and rooms that have 10 or more objects in their inventory. The character listing is displayed first with character name and total number of objects. The room listing is displayed second and lists the room brief, the room woename and total number of objects. 
   * Purpose unclear
   * For more information, type: +man listobjects

#### +lock
   * Usage: +lock door
   * Locks a door from both sides
   * For more information, type: +man lock
   * See also +unlock

#### +log
   * Usage: +log (shows help text)
   * Seems to be part of a change-log system not used on Marrach.
   * May be Skotos tool.

#### +look
   * Usage: +look
   * Experimental Merry version of core LPC "Look" action/verb.
   * Ignore.

#### +maim
   * Usage: +maim (displays help data)
   * OOC tool to assist in executions and punishment.
   * In development by Geist (Jul2007)

#### +make
   * Usage: unknown
   * Command does not work.
   * Possibly experimentally associated with MGeneric crafting.

#### +man
   * Usage: +man (displays help data)
   * +man is the system's manual pager. It is used to look up information on specific commands. Manual pages are either local or global. Global manual pages, available on every server, can only be written from the master server (at point of writing, Castle Marrach server); local pages, only available on the server where they were written, can be added/modified anywhere. Local manual pages take precedence over global.
   * For more information, type: +man man

#### +manuever
   * Usage: +manuever
   * Unimplemented command.
   * Use unknown. Perhaps related to experimental combat system.

#### +marry
   * Usage: +marry person and person
   * Links a couple together so that they will be able to enter/use a new, shared virtual room.
   * +marry displays help info.
   * Note that there is no +unmarry...

#### +mgmovedetail
   * Usage: +mgmovedetail character "[scar|tattoo] [current location] [move to location]"
   * MGeneric implementation tool that allowed the re-positioning of generic scars or tattoos onto specific left/right MGeneric body locations.

#### +mgupdate
   * Usage: +mgupdate name
   * Re-runs makeexaminedescpart on MGeneric bodies only
   * Neccessary after some uses of morph/freemorph.

#### +mhelp
   * Usage: +mhelp (brings up helpfile)
   * This function is used to get quick reference help for a specific merry function. You may also use +mhelp to add new/modify/remove existing references.
   * For more information, type: +man mhelp

#### +miki
   * Usage: +miki
   * A game-internal TWiki-like system to reduce dependancy on external TWiki.
   * Note that Marrach/TWiki is not administered by Skotos.
   * Basically unused.

#### +morph
   * Usage: +morph character
   * Allows full manipulation of all descriptive traits in a character. Note that morph only allows alteration to the trait list available throught the normal alteration skill. Freemorph does not verify the validty of the trait.
   * Same as "morph".

#### +mref
   * Usage: +mref (dislays the helpfile)
   * Display a Merry function reference for an object or a specific function in an object.

#### +msearch
   * Usage: +msearch (dislays the helpfile)
   * Search the Merry library database for entries matching one or several keywords. Note that this is not the same database +MHelp is using, as +MHelp is for internal Merry commands and +MSearch is for Merry libraries written IN Merry. 
   * For more information, type: +man msearch

#### +mstamp 
   * Usage: +mstamp "woename"
   * Apply a stamp to an object containing Merry scripts, scanning each script individually in an attempt to find a description for it. 
   * For more information, type: +man mstamp

#### +munstamp 
   * Usage: +munstamp "woename"
   * Presumably removes the effects of +mstamp.

#### +nip
   * Usage: +nip (brings up helpfile)
   * Used with NIP CNPCs to turn them on or off, turn something into a CNPC, record behavior for them as well as other functions

#### +objects
   * Usage: +objects
   * Displays characters that have 10 or more objects in their inventory

#### +obname
   * Usage: +obname here/object/character
   * Displays the woename for that room, object, or character

#### +opage
   * Usage: +opage character "message"
   * Leaves a pending page for that character to retrieve when they next log on
   * also: +opage reset <name> 
   * also: +opage see

+page
   * Usage: +page character/accountname "MESSAGE"
   * Paging that allows you to override someone who has page busy on (DO NOT ABUSE) or allows you to page someone who is possessing an inanimate object.

+people
   * Usage: +people
   * Lists characters logged in by location (grouping those together in the same location)

+possess
   * Usage: +possess object/character
   * Puts you in the body of object or character

+quote
   * Usage: +quote "message"
   * Used to display code scripts so that full-stops aren't added to the end like they do when you use the parser to say "message".

+rename
   * Usage: +rename oldname to newname
   * Renames the old name to the new name
   * Can take multiple names, so you can +rename aggie sue to harry bob

+return
   * Usage: +return (character optional)
   * Using +return by itself will send you back to your virtual home if it is set. +return character sends that char to their virtual home if it is set

+see
   * Usage: +see (brings up helpfile)
   * Used to notify you whenever a specific character logs into/out of the game.

+setp
   * Usage: +setp object "script"
   * Allows you to set a property on that object. +setp by itself will give you the helpfile

+sh
   * Usage: +sh (brings up helpfile)
   * Story Host chatline, cross game, NDA staff only

+sk
   * Usage: +sk (brings up helpfile)
   * Skotos Chatline, cross game, NDA staff only. Game leads MAY have access to this on servers they are not staffing so that their staff from their own game has the ability to reach them

+slay
   * Usage: +slay object
   * Will destroy that object completely. Will not (or should not) allow you to slay an item that has children/spawns. Please be careful as once you slay something, you can NOT get it back

+socials
   * Usage: +socials
   * Shortcut displaying the socials queue in TAS

+sp
   * Usage: +sp (brings up helpfile)
   * <nop>StoryPlotter chatline. While all NDA staff has access to it, only plotters should have this line turned on or do recalls on it. If you are not a plotter, please do not abuse the fact that you have access to it

+spawn
   * Usage: +spawn (brings up lengthy helpfile)
   * Spawns an object into the game. Most commonly used is +spawn "woename"

+staff
   * Usage: +staff
   * Displays NDA staff logged on

+summon
   * Usage: +summon woename
   * Used to bring an object to your environment. Great to use for spawned items. Please do not summon original object to you unless it is a character (and then we prefer you use +bring or +grab) or a CNPC you are building.

+suspend
   * Usage: +suspend (brings up helpfile)
   * Used to suspend an account from the game. Only Guides and the game lead should use this command

+tas
   * Usage: +tas (brings up helpfile)
   * Task and Assist System. Very extensive system for the filing of assists, bugs, typos, ideas, and socials. Also allows staff to file tasks into personal queues. __As a staff member, you must become familiar with the system as it will be used often. It is not a complicated system, but there are a lot of commands to review. Any questions on its usage, please ask Geli.__

+title
   * Usage: +title character "title"
   * Sets the <nop>StoryPlotter, <nop>StoryCoder, or <nop>StoryGuide title for staff

+tool
   * Usage: +tool help (brings up helpfile)
   * +tool is used to open up various popups for editing objects in WOE or adding merry code as well as various other things 

+tool quote
   * Usage: +tool quote __enter__ lines of text __enter__ . __enter__
   * This allows you to display on the client screen several lines of text at once (useful for showing lines of code). After typing +tool quote, hit enter. Copy/paste your text into the client and hit enter. Type a single . and hit enter.

+transfer
   * Usage: +transfer character from accountname to accountname
   * Transfers a character from one account to another. Useful to move VPs into a player's account or back to the generic npcs account

+typos
   * Usage: +typos
   * Shortcut to view typos queue in TAS

+unbring
   * Usage: +unbring character
   * Acts like unsummon, sending character to the nil. You need only type the character's name, not his/her woename

+ungrab
   * Usage: +ungrab character
   * Sends the character back to the room where they were +grab-bed from

+unlock
   * Usage: +unlock door
   * Unlocks a door on both sides

+unsummon
   * Usage: +unsummon woename
   * Sends the object back to the nil

+upskills
   * Usage: +upskills (brings up helpfile)
   * Can set all skill levels to 10 or clear out skill levels to 0. __This does not affect the morpher skill which will need to be set separately.__

+users
   * Usage: +users
   * Lists: free accounts, paying customers, accounts online, characters online

+verb
   * Usage: +verb "verb"
   * Lists the roles and signals for the verb

+whereis
   * Usage: +whereis charname/woename
   * Lists the woename of the environment for that character or object. You may be able to also use !whereis which can only be done on a character and not a woename of an object and allows you to see the brief of the room they are in instead of the environment's woename.


+whois
   * Usage: +whois character (must be in the room with you)
   * Lists information about that character object such as charname, snames, pnames, theatre id, and possessee name

+wound
   * Usage +wound character
   * Gives that character a wound
   
   
#### +owners
   *Usage: +owners[last modified Fri Jul  8 06:20:59 2005] 
Synopsis:
   +owners "woename" 
Description:
   Returns a list of children/spawns of that woename and where it is in game. 
   The listing is done by the woename of each child/spawn and woename of the 
   locations. 
   This command is used to find the owner of all the instances of an object. 
Examples:
   +obname "Generic:clothing:robe-dressing" 
See also:
   +whereis 




If you find any mistakes, need something explained further, or have any general questions about this doc, please feel free to speak with your Lead.

