# The Detail System - A Technical Summary

See Also: [DescriptionSystem](./DescriptionSystem), [ProximitySystem](./ProximitySystem)

Articles: [Triumphs, Trials & Trivalities #50: Building Blocks: Details](http://www.skotos.net/articles/TTnT_50.shtml) & [Room Attributes](http://www.skotos.net/articles/trenches03.shtml)

## The Devil Is In The Details

### Details, Details, Details

Look around the room that you're sitting in. It's maybe twenty-by-twenty, ten foot tall. That describes the prime characteristics of the basic "object"--which is to say the room. There's probably a lot of stuff in it too. Your black, ergonomic, computer chair sits on top of a faux Persian rug. Your laptop computer sits atop your oak desk. These are all discrete "objects" within the room. But there's a lot more to notice. Your walls have plaid wallpaper, your windows are dirty and permanently sealed shut, and the ceiling fan never turns any more. These are all examples of "details", which are descriptive elements for an object. They're permanently there and can't be moved or removed (without a lot of work).

## Quick Summary

See [Detail System Quick Summary](DetailSystemQuickSummary.md)

------

## NREFs - References to Details

NRefs are wonderful beasts. They are what we use internally as the primary way to reference a physical object -- a room, a doorway in a room; a player, a player's nose; a sword, the hilt of a sword.

In traditional MUD games physical objects and programmatic objects are more or less identical: a sword is a physical object and it's also an object in your code.

In SkotOS, many physical objects are also full programmatic objects -- anything you hold in your inventory is a full object, as is any room, and any body you possess. However, all these objects also have a host of *details* on them, and each of those details is a full *physical* object.

Thus if 'pool of water' is a detail in the room, I can wave at the pool of water, I can (try to) enter it, etc. So we need a data type to point to things that are physical objects but not programmatic objects. These things are NRefs.

So, basically, an NRef is a pair: an object (e.g. the room) and a string to hold the detail name (e.g. 'pool'). The detail name is identical to the one you see in Woe.

In Merry, most arguments are NRefs. To get the object component of such an nref Z, you use the function

```
NRefOb(z)
```

and to get the detail component you use

```
NRefDetail(z)
```

These are very (very) common functions for scripts to use. A typical (although idiotic) example would be to put a merry:react:enter script in a room. The code would start with something like,


```
if (NRefDetail($what) == "exit east") {
    EmitTo($actor, "You try to enter the eastern exit but fail.\n");
    return FALSE;
  }
  return TRUE;
```

------

## OLD UNSORTED DOCS & Notes

## Creating Detail Chains

All of the details for an object should be reachable by a logical and reasonable path through intermediate details from the first detail given. After all, if a bedroom contains a rocket ship, but no other detail mentions it, players will never know to type "look at rocket ship".

When describing objects, the overall description should resemble a tree. Brief and look details will lead to other details, which in turn will lead to more details. For example, a bedroom might be described as having a few simple furnishings in the glance description of the prime detail. Furniture would be another detail.

Typing "look at furniture" would access the look description of the furniture detail. This could respond "a bed, a table, and a wooden stool"--three more details. When the table is examined, it could return information on its legs and its wooden surface, and then these details could be examined in turn.

At some point a Story Builder will decide that a room has been described sufficiently and he will stop describing smaller details. How deep the description of an object goes will depend upon the importance of that particular object, its size, and the whims of the Story Builder.

Skotos articles which provide further discussion on how to create detail chains (and on details in general) include [TT&T #50: Building Blocks: Details](http://www.skotos.net/articles/TTnT_50.shtml) and [ItT #3: Room Attributes/a>.](http://www.skotos.net/articles/trenches03.shtml)

## State of Development

The basic detail system is implemented. Some visual details link to the Clustering System, which is not fully implemented as yet.

One type of detail which has never been implemented because of the lack of links to the Cluster system is:

- Significant Details, which are details that are important enough that they stand out when details are clustered.

In addition, as noted previously, other sensory details have not been implemented, though it would be trivial to do so.

### Open Issues in the Detail System

Open Issues in the Detail System

- Right now everything is equally significant -- how much value is there starting on [SignificanceSystem](./SignificanceSystem) ?
- How does the addition of hidden details, silence, invisibility, virtual exits, etc. affect the [DetailSystem](./DetailSystem) in non-obvious ways?
- Right now glace details are only used for interior of rooms when passing through fast. Could glance details be useful for the [SignificanceSystem](./SignificanceSystem) or the [ClusterSystem](./ClusterSystem) ?
- Are there hard and fast rules for what goes in the brief and glance descriptions?
- Are there any ways to automate details?
- For the glance descriptions, should really important objects be mentioned, or is just mentioning mobile things sufficient?
- Does the system automatically list details when describing a room or do we depend on the Story Builder? to do so? How much by each?
- Are there other interactions with the [SignificanceSystem](./SignificanceSystem) ? With the [ClusterSystem](./ClusterSystem) ?

------

## Basic Requirements

Most aspects of the Detail System have already been implemented. The basic requirements for the Detail System are:

1. Details **must** be integrated with the [DoorSystem](./DoorSystem) to support room navigation properly
2. Details **must** be able to contain variables useful to the game engine
3. Details **must** support both adjective and noun naming schemes
4. Details variables **must** be kept in synch by the game engine when two seperate details represent the state of a single object (such as a door, if it is implemented as two details, one for each side)
5. The detail system **must** support detail chaining, which the player can explore in deeper and deeper levels. When a user first inspects an object, he shouldn't see all details associated with it, only those at the "top level" of perception.
6. Prime details **must** be supported as default details for an object, and only one detail can be marked as prime. *This is not currently the case, as I understand it -- Michael*
7. Details **must** be able to be marked as "significant", so that the are listed in room descriptions (for example) regardless of any clustering effects in dense areas

------

### Existing Approaches

Existing interactive fiction doesn't tend to have details. It's all about objects. You have an object for your room, an object for the door, and so on. You might be able to investigate specific things, like that wallpaper, but it takes a lot of work to program them in, so this doesn't happen much. In addition, all of these individual elements usually aren't attached to the main object, which can cause problems when you're trying to keep things consistant.

### Problems Solved by the Skotos Detail System

Skotos has a system of details, where every object has a wide variety of details associated with it. These details represent all the significant features of an object. Each detail can take on a number of variables: names, adjectives, faces, descriptions, and exits. Thus, there is a large set of details, and each detail can contain a wide variety of information.

The big plusses of this system are:

- It's easy to detail minor things without having to create entire new objects.
- There is lots of potential to describe objects.
- It's easy to give objects a variety of names and adjectives.
- It's easy to create exits.

## Even Older Notes

Requirements:

- the use of "distant details"
- front and back, interior and exterior details
- exits and entrances
- details have attributes (adjectives, etc.)
- define the general details for each game (brief, glance, look, etc. are the **Skotos** default, for example)
- possibly make details bring themselves into descriptions by giving them obviousness, distinctiveness, etc. ratings
- we need a "near the middle of the room" detail in our rooms

------

------

References:

- Other systems that impact the Detail System: ClusterSystem, [ExitSystem](./ExitSystem)


- The Detail System impacts:

------

------

See also:

- [DetailGuide](./DetailGuide)
- [DetailNotes](./DetailNotes)
- [EricsNotesOnDetails](./EricsNotesOnDetails)

------

From [MudDev](http://twiki.skotos.net/twiki/bin/view/Skotos/MudDev):

- <http://www.kanga.nu/archives/MUD-Dev-L/1997Q4/msg00009.html>
- <http://www.kanga.nu/archives/MUD-Dev-L/1997Q4/msg00023.html>
- <http://www.kanga.nu/archives/MUD-Dev-L/1999Q2/msg00333.html>

------

------

**The Concept and Use of Details**

Consciousness consists of an awareness of your thoughts, existence and sensations. Within a role-playing game, players assume the identities of other persons (real or imaginary), simulating the complex of needs, beliefs, attitudes and aptitudes which make up a person. The existence and capabilities of the simulated person are agreed upon by the players, the referees or developers, and the rules or programming of the game. The mediator between the game data and the player (whether human referee or computer code) provides oral, graphical or written descriptions of what would be actual sensation in a real person.

All objects and groups of objects (including persons) have an identity: the characteristics and behaviors by which they are known. When something is identified, it is both assimilated into a group (“Bob is this person here”) and individualized (“This person here is Bob”).

Within the Skotos game engine, objects have several classes of properties. Numeric values of mass, bulk, and so forth may be determined by the players through measurement or their effects in the game world. Objects which are designated as rooms or actors will be obviously different in their actions or uses. Actors or machines will have behaviors or operations which the players will learn from observing their actions. Any object or event, though, will have potentially perceptible and describable qualities.

The text descriptions of these qualities in Skoot are usually meant to represent the results of the player’s perceptual abilities. Skoot uses these text items, called details, as the basic element of description.

A detail may be the description of a specific sensory impulse -- "The sky is blue" -- or a more general synthesis of several senses -- "The stable is dim and smells bad." It can (if desired) represent value judgements separate from actual senses -- "The princesses dance terribly." The statement of a detail can imply certain sub-actions your character takes to perceive the detail; when you say >TOUCH WALL, the detail description may be, "You reach out and knock on the cold, stone wall."

Details will usually be based on some sense, or on an 'intellectual' perception. Human perception actually breaks down into three components: organic sensation, attention, and comprehension.

First, organic sensation must be possible – the appropriate organs must be present and ‘operating,’ some light is needed for vision, a sound-conducting substance for hearing, etc. Also, if a detail to be perceived is hidden and requires ‘searching’ in some specific area to see, hear, etc. then it will not immediately be perceptible. ‘Enhanced’ senses (infra-red vision, a bloodhound’s nose, a shark’s lateral electrical sense) will of course improve the chance of some detail being sensible.

Second, attention must be paid to the detail. In our text-based games, the ‘focus’ of attention is set by (a) the player specifically naming a detail to examine, or (b) the game engine providing a detail by default. These default details include the basic room description received upon entry into an area; the details of any outstanding and prominent anomalies; and the details of ongoing perceptible events in a player’s ‘presence.’

Third, cognition provides the mind with its interpretation of the senses. The process of cognition fills in missing details (elaboration), simplifies subjects (idealization), identifies important parts (segmentation), and sorts subjects into groups (categorization). The differences between dimensions and characteristics of items in a group or category are minimized (assimilation), while the differences between objects in different categories are exaggerated (contrast). Familiarity or expertise will allow differentiation between items that would appear identical to other persons. Objects are sensed as being composed of various previously-named significant or relevant parts (segmentation); objects with greater dimensions (of sight, smell, sound, etc.) receive more attention and cognitive ‘value.’ Continuous patterns are mentally linked together (concatenation). Unexpected and previously unrecognized objects draw more attention, but may be difficult for the mind to understand.

The principle of elaboration allows the game designer to describe most objects with a minimum of text; the players should be able to ‘fill in’ appropriate details. For example: “This is a low, cobwebbed stone passage leading into the darkness,” implies a floor, walls or some kind of sides, a ceiling or overhead. If the player knows this is part of a castle, then he or she will assume there are mortared joints; the player should also reasonably expect that things could be hidden in the darkness.

Idealization allows us to use a single word, such as “table,” for many objects during basic description. Knowing the usual segmented portions of objects – “legs” and “top” – will guide developers in describing the most often examined details. Collections of similar objects can be described by with a single detail, which in turn might be part of a more general category: five chairs might be described as “The five chairs” or “A bunch of chairs,” and in turn the chairs can be part of the detail “There is furniture present.” Until the chairs are examined individually, they will be perceived as being of the same size (assimilation); persons with a particular knowledge of chairs might learn more about them using the same attention: “You see a set of Louis XV chairs.”

------

The description of areas (‘rooms’) is the first and probably commonest use of details by the designer. The designer will provide an initial description – called a ‘glance’ in Skoot – which is the shortest, basic sentence or two about the entire room. The ‘glance’ does not usually include anything secret or hidden, and is presented automatically when the player enters the room or area. The game engine will add a sentence or two providing details about any mobile or dynamic objects present; hopefully, the static and dynamic object descriptions will mesh properly in terms of style.

All of the other details for an object should be reachable by a logical and reasonable path through intermediate details from the first detail given. After all, if a bedroom contains a rocket ship, but no other details mention it, players will never know to ask LOOK AT ROCKET SHIP.

Other common visual details include the ‘brief’ (used when the description is to be embedded in a sentence), and the ‘look’ (a slightly longer description, including all the obvious exits). In a game where the characters do not have differing skills or abilities, the brief, glance or look descriptions will be the same for all characters (aside from the effects of ‘one-ofs’). An ‘examine’ detail begins to require decisions by the designer about concealment and perception skills and abilities; details which can be found only by I SEARCH THE NORTH WALL FOR SECRET DOORS and similar statements definitely require a system to determine success.

The overall Skoot description of a room or area should resemble a tree, in that the glance and brief details will lead to other details, which in turn lead to more details, etc. A bedroom might be described as having a few simple furnishings; these in turn would be listed for a LOOK AT FURNITURE request – “A bed, a table, and a wooden stool.” When the table is examined, it will in turn reveal its ‘segments,’ legs and a top surface, whose details might be examined in their turn. At some point, of course, the developer has to stop segmenting details.

A sample room might have the basic descriptive detail, “This is a small, square room with only one exit; a table and several chairs are present.” The floor, ceiling, exit and four walls might each be details, along with the table and four chairs. If the developer wants to keep the design slim and trim, the more-detailed segments of these details will not have separate descriptions – no separate table leg details, for example. The usual way Skoot designers signal the end of the branching detail structure is by adding the names of the non-detailed segments of an object to the object’s own name list. Thus, the table might have the description, “This is a sturdy wooden trestle table, large enough for six persons to sit around.” If a player asks about the surface, the underside or bottom, or the legs (singly or in plural) of the table, he or she receives the same “This is a sturdy wooden trestle table … “ description.

In our flagship games, each sense has various cognate forms. The details provided for a ‘look’ request, for example, will also be found by peering, squinting, watching and several other verbs. Different sensory verbs imply different levels of focus and attention; ‘examine’ is more productive of descriptive detail than ‘scan.’ Some of the attention-setting verbs, and their cognates, include:

*Sight*

- brief (in most cases)
- glance = scan (note that the ‘glance’ detail is multi-sensory)
- look = peer, squint, watch
- examine = survey, reconnoiter

Hearing

- listen = monitor, eavesdrop, overhear

Touch

- touch = feel, fondle, rub, handle, poke

Smell

- smell = sniff, inhale

Taste

- taste = lick, sample

Temperature, pain, balance and somatic senses aren't usually going to be linked to specific details. Another set of multi-sense attention verbs are those implying a hands-on search; probably ‘search,’ ‘pry,’ and ‘investigate,’ for example.

-- MikeBlum - 29 Mar 2000



## Emails]

```
----- Original Message -----
From: "Par Winzell" <zell@skotos.net>
To: <skotos-seven@skotos.net>
Sent: Wednesday, November 06, 2002 10:47 AM
Subject: Re: [skotos-seven] Base:Preps question

> I can't think of a single example of this. If you were under a low ceiling,
> then you can be near the low ceiling as it is so low "MichaelB and Zell are
> near the ceiling" says to me that you guys are very tall or the room is very
> short, or both. In this case, you should be able to touch the ceiling.
>
> Thus I'm not sure Y.A.F. is needed, just prevent touching if near isn't set.

Summary of the ways detail proxes and socials have changed in the last
month -- this is a good read for anybody on this list --

  * In days of yore, you could never approach a detail which didn't have
the NEAR preposition set. It was impossible to make something in a room
that players could be e.g. UNDER, but not NEAR -- a balcony, say.

  * Something like a month ago, I changed this so that a detail could be
approached as long as any preposition was set on it. Also, you would be
forced into the correct preposition if you approached it -- 'approach
balcony' would succeed and leave you 'under the balcony'.

  * Since that patch, it's been possible to touch the balcony, which is
a bad thing. Today, I changed this so that close socials can only be
done on details with the NEAR preposition set. Thus you can now approach
the balcony, but you cannot touch it.

Zell
```

------

```
----- Original Message -----
From: "Christopher Allen" <ChristopherA@skotos.net>
To: <skotos-seven@skotos.net>
Sent: Monday, April 07, 2003 6:31 PM
Subject: [skotos-seven] Don't use spaces in Detail IDs!


Skotos-Seven Leads,

One problem that I notice in LC:ABN, and in some of the S7 games is that your
details have Detail ids with spaces in them. For instance, ID= "exit up" rather
then "exit-up".

This will cause problems with referring to objects in SAM with derived
properties, which will become increasingly more commonplace as we add new
capabilities.

The problem is that when you use derived properties with SAM it can get confused
with spaces. As details are referenced by detail name, i.e. to get or set the
luminosity of a detail, you would use "$(target.details.default.luminosity)".

To illustrate the problem some more, I will use +tool sam exec:

   > +tool sam exec $actor.base:environment.details:default:descriptions:brief
   Original input:  $actor.base:environment.details:default:descriptions:brief
   Output:          " the sewer "
   >
   > +tool sam exec $actor.base:environment.details:exit up:descriptions:brief
   Original input:  $actor.base:environment.details:exit up:descriptions:brief
   Output:          " up:descriptions:brief"
   >

The result should have been " the manhole".

There a variant with the parenthesis that helps with some SAM problems, but in
this case it doesn't help either.

   > +tool sam exec $(actor.base:environment.details:exit up:descriptions:brief)
   Original input:  $(actor.base:environment.details:exit up:descriptions:brief)
   Output:          ""
   >

The only way to make the IDs with spaces in them to work in SAM is to resort to
very ugly inline Merry:

   > +tool sam exec $[Get(Get($actor, "base:environment"), "details:exit
up:descriptions:brief")]
   Original input:  $[Get(Get($actor, "base:environment"), "details:exit
up:descriptions:brief")]
   Output:          " the manhole"
   >

-- Christopher Allen
```

------

```
----- Original Message -----
From: "Par Winzell" <zell@skotos.net>
To: <skotos-seven@skotos.net>
Sent: Monday, April 07, 2003 9:57 PM
Subject: Re: [skotos-seven] Don't use spaces in Detail IDs!

>> This will cause problems with referring to objects in SAM with derived
>> properties, which will become increasingly more commonplace as we add new
>> capabilities.
>
> I think this warrants some internal discussion. We have thousands of
> rooms that use these already and while it's possible that we could do an
> automatic change-over, I would really much rather just make spaces in
> detail ID's work everywhere.

OK, so -- first, don't worry. There's no reason to go through rooms and
change detail names.

The way to refer to a SAM reference with a space in it is to put it in
quotes -- e.g.

   $("room.details:exit west:descriptions:look")

rather than

   $(room.details:exit west:descriptions:look)

Bing!

Zell
```

------

```
From: Erwin Harte <harte@skotos.net>  
Reply-To: List for Skotos Seven developers and their cohorts <skotos-seven@skotos.net>
To: List for Skotos Seven developers and their cohorts <skotos-seven@skotos.net>
Date: Oct 14, 2005 10:10 AM
Subject: Re: [skotos-seven] Removing details (through Merry).
On 9/21/05 7:00 PM Kalle wrote:
> Hola,
>
> Does anyone know of a way to actually remove a detail from an object,
> using Merry? Even removing everything from a detail (names, adjectives,
> descriptions, you name it) does not actually remove the detail, neither
> from the "base:details" array or from the object in Woe.
>
> If I've missed something, maybe someone knows how to do this?


Not sure this was actually announced to anyone but Kalle, but as a result
of his question I've added support so that you can now remove a detail by
doing something along the lines of

    $obj."details:<detailname>" = nil;

Hope that helps,

Erwin.
```

### Ancient History of Detail System

**Dormant Object Fields**

Traditional Mud worlds are composed of interconnected rooms, which are usually simply objects. They have text descriptions, and often sport a list of details, **\*items***, which also have descriptions. For example,

> ------
>
> \> look
>
> This is the town square, surrounded on all sides by high buildings. In the middle of the square sits a fountain.
>
> \> look at the buildings
>
> There's a hotel to the north, a hardware store to the east, a tobacco shop lies west and south is a bank.
>
> \> examine hardware store ...

Rooms also contain other objects, usually representing physical objects with their own descriptions and item lists:

> ------
>
> \> look
>
> This is the town square, surrounded on all sides by high buildings. In the middle of the square sits a fountain.
> A child is playing in the street.
>
> \> look at the child
>
> The child is a young boy, raggedly dressed but with a cheerful smile.

The child above would be an example of an object, typically an instance of e.g. /base/obj/living.c or somesuch. The child can move between rooms in exactly the same way that a player can.

Between them, these lists of static items and explicit object inventories constitute the fullness of a traditional Mud's expressive power. This is not enough for us.

If we stand in a town square, there may indeed be present only a few physical objects worthy of immediate note. However, when we consider the expanse of a whole world, there must naturally be trillions upon trillions of objects that exist independent of our observation - and if we do observe them and interact with them, these implicit objects must instantiate seamlessly into explicit ones.

We need a whole new abstraction layer for this. To whatever degree possible, higher-level code that describes the world and acts upon it should be able to refer to physical objects regardless of their state, explicit or implicit.

There are unforeseen advantages to investing in this additional complexity. Examples of functionality that can take advantage of dual representation include,

- In the Wilderness:

  As opposed to moving from room to room in a city, travel in the wilderness implies long distances covers. Among other things, travellers are likely to encounter hostile beasts. Infact, the entire countryside will be overlaid with scalar fields representing continuously varying population density fields of various kinds of beasts.The notion of representing all these beasts as objects is ludicrous. They will not come into explicit existence until confronted by a true observer (and exactly what that means is something we'll need to think about closely). The rest of the time, their actions (procreation, movement, predation, warfare) take the form not of single beasts moving around and interacting, but of simulations working directly on the population densities.Of close relevance to this are the *SimMud* and *Object Life Cycles* entries found elsewhere on this page.

- In Town:

  The child in the example above is a fairly typical example of an object used in the traditional sense to give some life to the Mud environs. Using our object fields, however, the idea that children are likely to be found playing in the street can be represented by a setting on the zone level, or even for the entire domain. With this simplification, we can suddenly with ease give the child lots of company: sunday strollers, police men, street sweepers, drunken mercenaries, thieves, beggars, and three-legged dogs.In this scenario, the strength of the SimMud layer becomes more apparant than perhaps it is for the Wilderness. With a well-configured simulation, different parts of town would attract different kinds of inhabitants quite naturally.Even more alluring an idea is that players can have a very direct effect on this state: For example, kill some key NPC (a crimelord, say) which plays a role in the simulation equations, and watch a bad part of town slowly respond to the new situation and become a pleasant suburb. Going even further out on the SimMud limb: if the player who killed this NPC was truly clever, she'd have bought some property in this part of town and now watch her wealth accumulate.

- Everywhere:

  Most Muds have to deal with heap-style objects, and so do we. For example, players regularly carry around hundreds or thousands of coins, and these are never individual objects, but rather single objects that represent many. A bunch of arrows is another example of this, or shotgun shells, or bandaids, etc, etc.These heaps are quite closely related to the general problem of implicit objects. In the same way that a player who sees a bunch of mercenaries hanging out on a street will want to be able to examine one of them closely, it must be possible to count your money, to split the heap into parts, to take a single coin out.We must decide how to handle state for these implicit objects; there should be local variation, but when is the dice rolled and a specific mercenary taken out of the grab-bag of varied-but-generic ones?

-- [ParWinzell]() - 30 Jun 1999
