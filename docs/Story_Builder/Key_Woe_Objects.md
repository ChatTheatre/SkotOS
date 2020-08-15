# Key Woe Objects

[![hackmd-github-sync-badge](https://hackmd.io/0__e-1maSYuHiJdJ0drVig/badge)](https://hackmd.io/0__e-1maSYuHiJdJ0drVig)


# Base:Thing

This is a special "clonable" root object for SkotOS. 

Every object, room, character, etc. that exist the virtual game environment are "Base:Thing".

Do not modify, only clone.

These special clonable object (like base:container) are different from parent objects. The actually contain all the "methods" that all objects of that type inherit. This is different than SkotOS's data inheritance.

## Chatters:[theatrename]:[alphatag]:[chatterbrief]

This is where the "in game" chatter bodies are located.

* [theatrename] probably comes from Theatre:Theatre:Name (but not confirmed, could be brief).
* [alpha tag] is the first two characters of [chatterbrief]
* [chatterbrief] is created in the start story. It isn't clear if this name is sync'ed with any name changes of the in game character.

### Notes

#### udat properties

* While a chatter body is possessed, a number of properties that do not show up in the Woe view of the chatter are available. For instance see `+stat me "property:udat:*"`

```
Property: udat:account = "staff"
Property: udat:grand = 0
Property: udat:guide = 0
Property: udat:host = 1
Property: udat:idle = 0
Property: udat:name = "christophera"
Property: udat:object = <UserAPI:UDats:c:christophera>
Property: udat:possessed = 1
Property: udat:premium = 0
```

It isn't clear how these properties is linked to the chatter.

#### the user behind the chatter

To see the user behind the chatter `stat [chattername] "property:udat:object"`. 

From the result, do `+obstat <udat> property` to see all the udat properties

```
-- Properties --
Property: array:assists = ({ 319, 325, 326, 327, 328, 329, 330, 331, 385, 386, 387, 388, 389 })
Property: conn:check = 0
Property: conn:last = "Christophera"
Property: more = 18
Property: see:listeners = ([  ])
Property: source-4842f42c5e7ed791fb0cfe58de53af99-cb-fun = "merry_edit_commit"
Property: source-4842f42c5e7ed791fb0cfe58de53af99-cb-obj = </usr/Tool/sys/merry>
Property: source-4842f42c5e7ed791fb0cfe58de53af99-extra = ({  })
Property: source-4842f42c5e7ed791fb0cfe58de53af99-language = "Merry"
Property: source-4842f42c5e7ed791fb0cfe58de53af99-object = "Mansion:UrMaleDressed"
Property: source-4842f42c5e7ed791fb0cfe58de53af99-script = "act-post:login"
Property: source-4842f42c5e7ed791fb0cfe58de53af99-stamp = 1127342157
Property: source-4842f42c5e7ed791fb0cfe58de53af99-target = <Mansion:UrMaleDressed>
Property: source-4842f42c5e7ed791fb0cfe58de53af99-text = ""
Property: source-4842f42c5e7ed791fb0cfe58de53af99-title = "Merry script act-post:login in Mansion:UrMaleDressed"
Property: sync:seen-verbose = 1
Property: sys:shared:locked-at = 1596844245
Property: sys:shared:locking = <Shared:clothing:ageless:ensemble:robe>
Property: tas:lastalert = 1596842925
Property: textif:chatmode = 0
Property: tool:edit_history = ([ "HQ:Neoct:Verbs:+rename":<HQ:Neoct:Verbs:+rename>, "Mansion:rooms:Hall":<Mansion:rooms:Hall> ])
Property: tool:edit_obj = "HQ:Neoct:Verbs:+rename"
Property: tool:lastprop = ([ <Mansion:rooms:Hall>:"merry:setprop-post:trait:flame" ])
Property: tool:recent = ([ <Mansion:rooms:Hall>:1596505275, <HQ:Neoct:Verbs:+rename>:1596844691 ])
Property: tool:top10 = ({ <Mansion:rooms:Hall>, <HQ:Neoct:Verbs:+rename> })
```

## Core:PropertyContainer

This is a special "clonable" root object for PropertyContainers, which are objects that can contain properties that be addressed by Merry, but are not base:thing and thus not in the game itself. Useful as places to store properties and scripts that are independent of game objects.

Do not modify, only clone.

These special clonable object (like base:thing) are different from parent objects. The actually contain the base "methods" that all objects of that type inherit. This is different than SkotOS's data inheritance.

## Generic:*

These are old "generic" objects from the original game, including many that have the old Bilbo scripts in them. Please do not +spawn them, as they probably all have to be deleted.

## System:Developers

List of users who may be allowed to have access to the wiztool. They will still need someone to add a password using the wiztool to add access.

## TextIF:*

These are objects related to the Text Interface.

### TextIF:ErrorDB

This is where the standard error text for built-in verbs is defined. Note that many built-in commands have been overriden by Merry-based verbs, thus the error text for those will likely be in that verb and not here.

### TextIF:Init

This is where the banner text that is sent to the user on login is defined. Theoretically other intial values for the text interface of a ChatTheatre could be stored here, but right now only banner.

This is a great place to put some initial help and useful [acmds] to get users started with understanding the text interface, but note that users will see these EVERYTIME they log in, so you may want to set properties in the account udat or the chatter body to limit how often it is displayed.


## Theatre:Theatre:[theatrename]

This is used by the xxx (main.sam?) to connect the user (udat) to the a chatter in the ChatTheatre. In the oldest games, it also defined the initial character, and start story "nuggets". Most every game since doesn't seem to thes and has completely custom start stories.

### Properties

Not clear how many of these are actually used, and where.

* id="Gables"
* name="Gables"
* brief="Gables"
* description="Gables"
* ranking="5"
* announcement="The Gables Mansion"
* room="OBJ(Mansion:rooms:Limo)"
* jail="OBJ(Mansion:rooms:Oubliette)"
* home="OBJ(Mansion:rooms:Limo)" 
* office="OBJ(Mansion:rooms:Limo)"
* hostname="game.gables.chattheatre.com"
* realmurl="http://images.gables.chattheatre.com/" 
* bigmapurl="http://images.gables.chattheatre.com/map.jpg"
* restricted="false"

### Bugs

Currently the room that the Gables start story connects the user to is the Limo, which is a virtual room, and thus should  force a "spawn" of that room that will be unique for each chatter. Instead, the are being sent to the parent room, and will be given a virtual room when the return to the limo later.

## Theatre:Web:StartStory:*

This is where the start story is located.

### Theatre:Web:StartStory:AppearanceChoices

A propcontainer that contains all the appearance choices for the start story. Each choice is set up as a mapping and can limit options based on gender and account type.

```
export:choices:features:build

([ "free":({ "slender", "husky", "chubby", "ordinary" }), "standard":({ "athletic", "lissome", "pauncy"}), "premium":({ "muscular", "chiseled", "sculpted", "toned", "wiry", "statuesque" }), "male":({ "masculine" }), "female":({ "buxom", "full-figured", "curvaceous", "feminine"}) ])
```

In addition to the gender options:
* A **free** account will have access to **free** options.
* A **standard** account will have acess to **free and standard** options.
* A **premium** account will have access to **free, standard, and premium** options.

### Theatre:Web:StartStory:StartStoryFrame

This object contains all the start story support code and start story settings. Inherits the object **Theatre:Web:StartStory:AppearanceChoices**.

#### Settings

* **export:setting:characterslots:free=1**
  * The number of character slots a **free** account has access to.
* **export:setting:characterslots:standard=3**
  * The number of characer slots a **standard** account has access to.
* **export:setting:characterslots:premium=5**
  * The number of character slots a **premium** account has access to.
* **export:setting:title="Gables: Character Generation"**
  * The title that appears at the top of the browser while going through the start story.
* **export:theatre:id="Gables"**
  * Used to correctly assign a chatter to the appropriate **Chatter:\*** folder during character creation.
* **export:setting:virtualhome"=<Mansion:rooms:Limo>
    * New characters will be assigned the appointed virtualroom.

#### Scripts

* **merry:lib:initialize**
    * Called when each start story page is loaded. Contains the basic sanity checks to ensure necessary variables are present and player is not attempting to access a body not present on their roster.
* **merry:lib:figurestate**
    * Called when each start story page is loaded. Determines the start story step the character is on and loads the appropriate content for that step. State data is stored in the chatter's **chargen:state** property.
* **merry:lib:check_roster**
    * A script that checks if the account is able to make a new character.
* **merry:lib:check_name**
    * A script that will check if a name is valid. Valid name criteria:
        * *name does not exist*
        * *name does not contain special characters or spaces*
        * *name is less than twelve characters long*
* **merry:lib:list-choices**
    * A script that will parse a string into a series of drop down lists. Anything enclosed in [] will be identified as a choice. For example, during appearance selection, the blurb "You have a [noseshape] nose, and [haircolor] hair." will convert **[noseshape]** and **[haircolor]** into drop down lists of options for these choices.
* **merry:lib:grab-choices**
    * A script that returns an array of start story options for a specified choice. Used to populate the choices found by the *list-choices* script.
* **merry:lib:runscripts**
    * Once a chatter is finalized in the last step, this script is ran to set up final traits and generate a virtualhome. Chatters are flagged with the property **chargen:finished** so they are not able to walk back through the start story to make changes to the chatter.

### Theatre:Web:StartStory:CreateCharacter

This object holds the front-end html code for the start story. Inherits the object **Theatre:Web:StartStory:StartStoryFrame**.

* **export:xtra:stylestheet**
    * Used for styling the start story pages.
* **export:html:index**
    * The core start story page. Calls the intiatlize and figureststate scripts to determine which start story state gets loaded.
* **export:xtra:state:no-body**
    * The first state during the character generation process. No chatter body has been created yet. Player chooses a name and gender.
* **export:xtra:state:[XY]**
    * XY = 01, 02, 03, etc. These are the states that are walked through after the chatter is generated. In Gables, export:extra:state:01 is the state where the player modifies the chatter appearance. More states can be added to allow for more options and customization.
* **export:xtra:state:end**
    * The final state of the start story process. Gives an overview of the chatter body and chosen options, and asks the player to confirm creation.

#### A note about popups and html content

In order to access account-specific data, the following **z tag** should exist at the top of the SAM file before any other html tags.

```
X[S]
    <z dat="$(UDat.Dat)" name="$(UDat.Name)">
        <html>
            Account name: $(name)
            Account udat: $(dat)
        </html>
    </z>
```

This makes vital udat information available as properties that can be referenced with SAM or in-line Merry code.
* $udat = the udat object for the account
* $name = the account name

## Theatre:Web:Theatre

This is the object that holds the page where players can enter the game with a character, delete a character, or create a new character.

### The Roster

The number of characters available depends on the account type. This can be changed with the settings in **Theatre:Web:StartStory:StartStoryFrame**. Additional slots can be granted to an account using the in-game **+roster** command. Staff and VP characters flagged with the property **nocharcost** do not count against the character allotment. Once a roster is at capacity, the player will not be able to create new characters.

#### Frozen Characters

When an account type changes and the number of available players exceeds the maximum allowed characters, the player must select which characters they want to be active by logging into the game with those characters. Once the selection is made, the remaining characters on the account will be frozen and will not be able to log into the game until either the account type is changed or staff grants additional character slots. Frozen characters will be flagged with the property **mothballed**.

The code that prevents frozen characters from logging in is stored in
```
Shared:Systems:GameLibs:All:general_functions/merry:act-post:login%account_check
```
and inherited by the main chatter Ur.


### Character Deletion

Once a character is deleted, the chatter object and virtualhome room will be deleted. **This cannot be undone!**

Something to consider for the future...

Instead of completely deleting characters, remove them from the roster and store in a backup folder. Run a daily cron script to purge backed up characters that are older than 30 days.

## Shared:Systems:GameLibs:All

A shared folder that holds useful functions. It is assigned to the ***general*** script space.

* **merry:lib:getaccounttype**
    * This script takes an account name and returns the account type (free, standard, or premium). Developers can flag an account as free, standard, or premium by setting the property **testing:account-type** in the udat object, but should only do so for testing purposes.
* **merry:act-post:login%account_check**
    * This script is inherited by the main chatter ur and prevents frozen characters from logging into the game. It also handles the unfreezing of characters.
* **merry:lib:countcharacterslots**
    * A script that takes an account type and udat and returns the number of character slots the account is allowed to have.
* **merry:lib:mothballandsortroster**
    * A support script for **merry:act-post:login%account_check**. Takes an account's roster and turns it into a sorted list (active vs frozen characters).
    