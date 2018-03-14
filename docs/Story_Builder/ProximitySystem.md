# The Proximity System - a Technical Summary

See also: [DetailSystem](./DetailSystem.md) [DescriptionSystem](./DescriptionSystem.md)

Articles: [An Evening at Chez Skoot – Proxes Part 1: Consent](https://www.skotos.net/articles/chezskoot.html)

## A Sense of Place

### Characterization and a Sense of Place

In fiction offering a sense of place is an important part of the author's craft. Most writing instructors suggest giving the reader a distinct sense of place and time in the opening paragraph if not the first sentence. If a writer fails to do so, the reader can be confused. Without a location to anchor him, a reader will be unable to envision either the characters inhabiting that location or the actions they engage in.

When multiple characters are involved, it is also important to explain how characters are positioned relative to another. This not only gives the reader a better sense of place, but also offers characterization. As an example, notice the sense of place in the opening sentence of Dashiel Hammett's *The Thin Man*:

> *I was leaning against the bar in a speakeasy on Fifty-second Street, waiting for Nora to finish her Christmas shopping, when a girl got up from the table where she had been sitting with three other people and came over to me.*

In this introduction we discover the protagonist is quite comfortable lounging in an illegal bar, probably in the twenties, in some large urban city in the USA. He has a girlfriend or wife that is wandering elsewhere, and he isn't too worried about being separated from her. A young woman, socializing with others, notices him, though he's doing nothing in particular to attract attention, and goes over to him. You already know that she wants to speak to him.

This paragraph gives a remarkable sense of place and of characterization in just a few words.

### Interactivity and Problems of Place

In multiplayer interactive fiction, a basic sense of place is easy. Either the player exists in a room that can be examined or the player exists in a scene that will be described as he takes actions. However, multiplayer interactive fiction does a poor job of integrating that sense of place with other players in the game.

This probably comes from multiplayer interactive fiction's roots in single player games like *Adventure* and *Zork* , where there is truly only one significant and realistic character--the player. At Infocom's height, the best NPC were just a notch above a puzzle--not truly a character.

In multiplayer interactive fiction, everyone is significant, and everyone passes the Turing test (well, almost). So we need to find a way to describe a player's interaction with his environment and with other players, in a fashion that is significant to characterization.

The locale has been described, but how does the player fit in? Is he comfortably leaning against a wall, resting at a table with his head in his arms, or nervously standing at the door, waiting for someone? When someone else arrives, how does he react? How does he show his interest or lack of interest?

### Problems solved by the Skotos Proximity System

Skotos solves some of the problems of characterization and a sense of place through a technique we call "Proximity". With the Skotos Proximity System, combined with our Skotos Expression System (see the Technical Summary), a player can be near a door, sit at a table, pray before the altar, or kneel in front of his bride.

The proximity system also solves other problems. If someone approaches you to kiss you, you have an opportunity to retreat before the kiss is completed. If you drop something, it is close to you and someone else will have to be just as close before they can pick it up. If you are sitting at a table, you will not notice other people in the room unless they are shouting, or unless you decide to pay particular attention.

------

## Summary of the Skotos Proximity System

The Skotos Proximity System creates proxes, or proximities, which qualify associations between objects.

- Proxes are defined by two objects, a vector, and a preposition.
- Prox chains describe how an object is linked up to the container it is inside.
- Prox trees describe how all of the objects inside of a container are linked.
- The categories of adjacent, nearby, vicinity, chamber, and bordering describe how far apart proxes are.

### The Prox

A prox (short for proximity) is a distinct association between two objects that describes the spatial relationship between those two objects. It always includes a preposition (on, near, in, etc.) and a vector.

Example:

```
   spoon->ON->table
```

In this case the preposition is *on* and the vector is from the spoon to the table. In this situation, the spoon is described as "The spoon is on the table" and the table is described as "The table has a spoon on it."

The spoon is considered to be a "child" of the table, and the table is considered to be the "parent" of the spoon. The vector from the spoon to the table (i.e. child to parent) is considered "up", and the reverse vector from the table to the spoon (i.e. parent to child) is considered "down".

### Prox Chains

If you choose an object, find its parent (there can only be one), follow that parent's prox up to another parent, etc., the entire set is known as the "prox chain".

For example:

```
   statue->ON->pedestal->NEAR->northwall->IN->room
```

### Prox Tree

If you choose an object, find its children, and discover all the children of every child, all the way to the bottom of every chain, the entire set is known as a "prox tree".

For example, this is a prox tree of a room:

```
   statue->ON->pedestal->NEAR->northwall->IN->room
                 painting->ON-/              |
        bowl->ON->table->NEAR->southwall->IN-/
       spoon->ON-/                           |
     chair->NEAR-/                           |
                                   floor->IN-/
```

### Adjacent, Nearby, Vicinity, Chamber, and Bordering Proxes

Every object is "adjacent" to its parent and children. For instance Alice is sitting next to Carol who is sitting next to the table; Carol is adjacent to both Carol and the table, but Carol is only adjacent to the table.

By default, unobvious actions and quiet sounds can be heard by all objects that are adjacent. Thus when Alice whispers, Carol can hear her. When Carol whispers, Alice can hear her, and if the table could listen, it would hear her as well.

The "nearby" class of an object is determined by going up the object's tree until the first non-volitional parent is found. All children of that parent, down several levels, are "nearby". For instance, everyone sitting at a table is nearby to each other, including the Alice who is sitting next to Carol who is sitting at the table, and Bob who is standing on the table.

By default everyone that is nearby will see all the actions and hear all the sounds of those who are also nearby. Thus everyone speaking in a normal voice will be heard by everyone at the table, and they will see all normal actions.

"Vicinity" is defined as everything associated with the same detail of a room (or container). The prox chain is traced up until a room/container detail is reached. Then, all children down several levels are considered to be in the "vicinity". For instance, Carol is sitting at a table in the back corner of a room (a detail of the room). She is in the vicinity of everyone at her table, of everyone to in the back corner of a room, and of everyone sitting at other tables in the back corner of the room.

By default only significant actions and loud sounds will be seen or heard by all that are in the same vicinity. Thus when Bob shouts and dances on the table, all in the vicinity will see and hear it, but they will not hear the normally voiced complaints of those that are at Bob's table.

Every object that is inside the same room (or container) is considered to be in the same "chamber". By default, a few very significant actions and very loud sounds can be seen or heard be everyone in the same chamber.

Two rooms (or containers) that share a common entrance/exit pair, are considering to be "bordering" one another. Very loud sounds or a general "din" of noise may also be heard in rooms bordering each other.

## Proxes in Practice

Proxes have a number of visible effects in the game:

- They can change how objects are described.
- They can interact with stances and poses.

(How proxes interact with intimate space and sound are described in another section.)

### Level of Description

In most cases when an object is described, only one level of proximity up and down is detailed.

```
   spoon->ON->plate->ON->table->IN THE MIDDLE OF->room
```

If you "look at room" you'll see "You are in a room with a table in the middle of it."

If you "look at table" you'll see "You see a table with a plate on it, in the middle of the room."

If you "look at plate" you'll see "You see a plate with a spoon on it, on a table."

If you "look at spoon" you'll see "You see a spoon on a plate."

### Proxes with Stances & Poses

Prepositions are often combined with stances (i.e. "standing against the wall" or "sitting near the flowers"), or with poses (i.e., "reading in the chair") or both (i.e. "kneeling and praying before the altar"), as per the Skotos Expression System (see that Technical Summary).

However, certain proxes may be disallowed with specific stances or poses; for instance, you can't "kneel above the altar". Others stances or poses may require specific kinds of proxes, for instance, "with his arm around Carol".

## Creating a Prox

Proxes are created by examining four values:

- An active object. The active object is placed in relation to some static object which is already part of a prox tree.
- A static object. The static object is already part of a prox tree; an active object is placed in relation to it.
- A vector. This is the relation between the active object and the static object. It defines one of them (usually the active object) as a child and the other (usually the static object) as a parent.
- A preposition. This is a description of how the child and parent are related.

In order to create a prox all four of these values must be known.

Usually a player will issue a command that will make these choices obvious. Take the sample command: "put knife on plate". Clearly, the active object is the knife and the static object is the plate. A vector leads from the knife (child) to the plate (parent). The preposition is "on".

Sometimes all of this information will not be obvious. Alternatively, some of the "obvious" values may be changed to create more consistent and interesting descriptions. In particular:

- The active object should always be obvious from the command.
- The static object will fall back to a default if it is not obvious from the command.
- A default choice of vector is always obvious. However the vector direction may be overridden to improve readability.
- The preposition will fall back to a default if it is not obvious from the command. In addition, the vector direction may be overridden to improve readability.

### Choosing the Active Object

The **default** active object is always the object which is being acted on. A ball is dropped or a frisbee is thrown or a rabbit is hidden in a hat.

### Choosing the Static Object

The static object will usually be obvious. A player will choose to put the active object on (or in or under or over) a static object. For example "put screwdriver in toolbox" or "brush dust under rug" or "put paint bucket over doorway". However there are occasions when a prox will have to be created without this information, such as when the command "drop grenade" or "throw stink bomb through north door" is issued. In these cases, a default static object is chosen.

The **default** static object is usually defined by one of a few special rules:

- If an object just entered a room (typically a volitional object like a player or NPC, but possibly a thrown item like a stink bomb) the static object will be the entrance it came from.
- If an object is dropped, the static object will be the volitional object that dropped it, i.e. `grenade->CLOSE->player`.
- If the static object can not be determined, then the room or container which the active object is in is chosen.

### Choosing the Vector

The **default** vector selects the active object as the child and the static object as the parent. For example, in the command "put the flowers next to the altar" the flowers, which are the active object, are chosen as the child and the altar, which is the static object, is chosen as the parent: `flowers->NEXT>altar`.

#### Overriding Default Vectors

In some cases the default vector will confuse players reading a room description. For instance, if the player puts the altar next the flowers, the following prox would be created by default:

```
   altar->NEXT->flowers
```

Clearly "The altar is next to the flowers" is not desirable outcome.

Whenever creating a prox, the distinctiveness of both items is examined. If there is a large difference in distinctiveness between the two objects, the system will override the player's choice of vector.

The general algorithm used to determine the vector is:

- If the two objects are within 33% of each other in bulk, then the static object is made the parent.
- If the two objects are more then 33% different in bulk, then the item that is larger is selected to be the parent.

However a number of other considerations are taken into place, to determine the distinctiveness of an object:

- If an object has an adjective, it is more distinctive. For example a silver spoon should be more distinctive then a spoon. A gold spoon would be even more distinctive.
- Volitional objects (i.e. living beings) are considered to be somewhat more distinctive then non-volitional objects.
- There is also a "distinctiveness" modifier. This is used for objects that are quite notable. This allows the magic sword Excalibur to be the most distinctive object, or a moving but non-volitional object like a pendulum to be more distinctive.

A total distinctiveness value is calculated, and then it is used as a multiplier to bulk for purpose of the vector-determination algorithm.

### Choosing the Preposition

Commands like "drop grenade" or "throw stink bomb through north door" also leave the preposition to use up in the air. In these cases a default preposition is chosen.

The **default** proposition is the relationship that most distinctively describes the relationship between two objects:

Take the case of these two potential proxes:

```
   table->NEAR->floor
   table->ON->floor
```

The table can be "near" the floor, or "on" the floor. "On" is more distinctive, so it is the default prox.

A number of rules are used for special cases when choosing propositions:

- If an object just entered a room (typically a volitional object like a player or NPC, but possibly a thrown item) it will default to be "near" the entrance that it came from, i.e. `dog->NEAR->doggydoor`.
- If an object is dropped, it will default to be "close" to the volitional that dropped it, i.e. `stone->CLOSE->player`.
- If a prox can't be found for an object, and the object that contains it is quite large in relation (for instance a room), the prox default is "near middle of", i.e. `grenade->NEAR MIDDLE OF->room`.
- If the container is not too much larger then the object, then it will default to being just "in" the object, i.e. `rock->IN->chest`.

#### Overriding Preposition Choices

Sometimes there will be a conflict between multiple propositions, such as when a player puts a paper "below a sofa" when there is already a rock "under a sofa". Which preposition should be used, below or under?

A system of priority decides which prox to use when synonymous proxes such as "under" and "below" conflict. Each synonym is given a different priority.

If a new prox is created with a synonymous preposition to an old prox the higher priority preposition is used for both objects. In our example of a "paper below a sofa" and a "rock under a sofa", they would become "a paper and rock under a sofa".

In the lists below the prepositions to the left are given priority (though the choices are somewhat arbitrary).

**Container Proxes** (Inventory Related):

- in, within
  (special prox for items in a container that does not close)
- inside
  (special prox for items in a container that does close)
- enclosed
  (special prox for things inside a closed container)
- attached, held
  (special prox for thing that move with the parent object when it is moved, but are not inside.)

**Intimate Proxes** (Deniable Consent)

- close to, close by, close beside
- against
- under, beneath, below
- on, upon, on top of, across
  (these may also be container proxes)
- around

**Near Proxes** (Delayable Consent)

- near, by
- over, above
- behind
- in front of, before
- beside, next to

**Other Proxes**

- with, among
  (not an actual prox; will join the objects parent's prox)
- in the center of, in the middle of, near middle of
  (used as part of the leave command, or a default -- the parent must be a room or a detail of a room)
- at the edge of
  (parent must be non-volitional, a room, or a detail of a room)
- outside of
  (?)
- beyond
  (?)
- between
  (NOTE: Not to be implemented in v1 at it requires two proxes)

#### Disallowed Prepositions

Some kinds of objects may disallow certain prox prepositions. This prevent such actions such as "put paper behind the wall", or "put paper in the fork", etc.

------

## Changing Proxes

Once proxes are created they can be changed in a number of ways:

- If an object is removed, it breaks a prox chain and a new one must be formed.
- A player can choose to join an existing prox.
- A player can choose to leave an existing prox.
- A player can choose to approach a volitional, such as a player. He will join the volitional's existing prox, then enter a "near" prox, then enter an "intimate" (or "close") prox.

### Breaking a Prox

When an object is moved, the prox between it and its parent is broken, as well as the proxes between it and its children (except for container proxes.) A new prox will be created for the orphaned children, leading to the moved object's parent.

Take the following example of a prox chain:

```
   flowers->IN FRONT OF->picture->ON->table->NEAR MIDDLE OF->room
```

If you remove the picture, the prox chain for the flowers becomes:

```
   flowers->ON->table->NEAR MIDDLE OF->room
```

### Joining a Prox

The join command allows a player to move up to another volitional. It causes the player's parent prox to become the same as the parent prox of the volitional that he is trying to join.

For instance:

```
   bob->NEAR->westdoor->IN->westwall->IN->room
   carol->AT->table->NEAR->westsideofroom->IN->room
```

Bob types "join carol" and the proxes change:

```
   bob->AT->table->NEAR->westsideofroom->IN->room
   carol->AT->table->NEAR->westsideofroom->IN->room
```

### Leaving a Prox

Another simple player command is "leave" or "leave *object* " -- this will cause the player's prox to become "near" his parent prox's parent. If the new parent prox is the room that the player is in, it prox will default to "near middle of room". If the player types leave from "near middle" he will be "at the edge of room". If the player types leave one more time, if there is a default "out" exit, it will be taken, otherwise, a random exit other then the exit he originally came from used, or if it is the only exit, the exit he arrived from. (NOTE TO ENGINEERING -- we currently are not storing the entrance that a player arrived from.)

Example of the results from multiple leave commands:

```
   bob->NEAR->door->IN->westsideofroom->IN->room
   bob->NEAR->westsideofroom->IN->room
   bob->NEAR MIDDLE OF->room
   bob->AT EDGE OF->room
   bob->NEAR->door->IN->eastwallofnewroom->IN->newroom
```

### Approaching a Prox

A more complex way for a player to change his prox is to "approach" another object.

If the player and the object he is approaching have different parents, but are in the same room (or other container), the approach command will change the players' parent prox to be the same as the object he is approaching. If the player and the object he is approaching are not already in the same prox, this has the exact same effect as join.

As a start of an example:

```
   bob->NEAR->westdoor->IN->westwall->IN->room
   carol->AT->table->NEAR->westsideofroom->IN->room
```

After Bob gives the command "approach carol", and he will receive the message "You begin to approach the table that Carol is at" and then after a short delay: "You are at the table that Carol is at" Others nearby, including Carol, will see only that Bob is approaching the table. The proxes are now as follows:

```
   bob->AT->table->NEAR->westsideofroom->IN->room
   carol->AT->table->NEAR->westsideofroom->IN->room
```

If the player and the object he is approaching already have the same parents, the approach command will change the player's parent prox to be "near" the object he is approaching.

So if Bob gives the command "approach carol" again, he will receive the message "You begin to approach near to Carol." If Carol doesn't move, he will receive the message "You have approached near to Carol" after a short delay. Others nearby will see these messages that Bob is trying to be near to Carol. The proxes are now:

```
   bob->NEAR->carol->AT->table->NEAR->westsideofroom->IN->room
   carol->AT->table->NEAR->westsideofroom->IN->room
```

If the player is already "near" an object and gives an approach command a third time, he will move "close to" it (into the intimate proximity).

So if Bob gives the command "approach carol" one more time, he will receive the message "You try to approach close to Carol." After a delay, if he is successful, he will see "You get closer to Carol" -- others may or may not see a message (see the [ConsentSystem](./ConsentSystem.md) ).

```
   bob->CLOSE TO->carol->AT->table->NEAR->westsideofroom->IN->room
   carol->AT->table->NEAR->westsideofroom->IN->room
```

Bob could have said "approach near to carol" to skip one step, or "approach close to carol" and skipped two steps; however, all the intermediary messages will still happen as if the commands were separate.

Bob also could have chosen other prepositions other then "near", for instance "approach next to carol" or "approach in front of carol".

If Bob is "close to" Carol and types "approach", the system will say that Bob is as close as he can get to Carol. The prepositions "on", "against", and "under" are also considered intimately "close"; thus further approaching isn't allowed.

## Old Notes

### State of Development

The Skotos Proximity System is partially completed, the proof of concept being quite powerful. Skotos' goal is to have the completed Skotos Proximity System as described here ready for a consensual conflict (i.e. MUSH-style) environment by September for the release in the Castle Marrach game and for Stages shortly after that. After that, Skotos will adapt the Skotos Proximity System to other (i.e. MUD-style) games with non-consensual conflict.

### Open Issues with Proxes

- It is possible in some situations to accidentally get into a prox circle. These need to be eliminated.


- Is the distinction between close and near proxes significant enough for new users?


- There are a number of issues with proxes between three objects, so we don't allow it. Right now prox chains need to be unique. Thus you can't have two people "next to each other" -- at least one has to have a prox leading up into the room. Similarly, you can't have a between prox (i.e. Bob is between Carol and Alice).


- Describing a room full of objects with proxes associated them with needs to be integrated into the Skotos Crowd & Cluster Systems, and it is unclear what all the issues are.


- Distinctiveness is also used by the Skotos Crowd & Cluster Systems -- are there any cases where the logic that works for one does not work for the other?


- Proxes between crowds or clusters (i.e. "the crowd moves toward the stadium") are way beyond our scope right now.


- There is a relationship between container proxes (in, inside, enclosed) and some of the intimate proxes (on, attached) with inventory. The parallel of these proxes with inventory is clear, but some of the implications of distinctiveness and order of chaining are not. Many of the details need to still be worked out.


- There is a deep question related to our current haphazard chain-extension approach. Should we try to maintain some of the properties of a tree, such as attempting to keep it to a star shape with more distinctive objects in the middle and least objects distinctive toward the leaves? How much do we override player choices to maintain such a tree?


- - Example: When a very distinctive object enters a prox, it will not become the child but the parent of the prox. This switching occurs recursively so that more distinctive objects travel towards a center of the net, which thus becomes star-shaped. In a worst-case scenario, you could have lots of distinctive objects entering and then leaving a long prox chain where each such operation is O(N) where N is the length of the chain.
    Say a dustmite is next to a fly is next to a bowling ball is next to an anvil is next to a statue, and the highly distinctive David Bowie enters the room and approaches the dustmite. He will recursively be pushed to the head of this prox chain each time he approaches the dustmite. This is not really an O(N^2) scenario though. Currently our implementation of *describing* prox chains is, but that is purely so that I wouldn't spend lots of time coming up with clever algorithms to do it faster. The problem is not inherently O(N^2).
    -- Par


- Right now the offer and accept command applies to any transfer between two volitionals. How closely should we integrate into the prox system? For instance should you only be able to offer to someone in a near or intimate prox? Should there be any automatic acceptance? For instance do you need someone to separately accept a gift if they have already accepted an intimate prox? In addition to offer and accept, we may wish to have a "trade" object function -- does that change any interaction with the prox system, such as not only working for close and near, but also nearby?


- There are some thief related close actions such as jostle, bump, etc. Most of them should wait until we integrate a combat system into the prox system. However, are there any easy things we can do now to prepare for them?


- Details should have proxes to each other. Is the shrubbery near the wall or is it in the center of the room? -- [WesConnell]() - 27 Apr 2000


- We should also think about prox redirection. A player should not be able to have a prox to the roof. The roof should redirect the prox to be the center of the room or the table in the center of the room. -- [WesConnell]() - 27 Apr 2000


- Various room details need to be proxes of each other. The fireplace, built into the north wall, is "near" or "part of" the north wall; the north wall is "behind" the fireplace.


- Should we only allow legal stances and poses?

------

------

**Area below not part of article, but just for notes and such when we're putting the article together**

------

------

**TECHNICAL REQUIREMENTS**

**General Requirements:**

1. **Must (dependent)** define spatial relationships between a pair of objects (a prox)
2. **Must** define spatial relationships that lead back to room details
3. **Should** define spatial relationship between a pair of details
4. **May** define spatial relationships between a triplet of objects or details

This one is a bit hard to list requirements for, because we've got the whole system mapped out so well. But, the basic idea is that we want to be able to build spatial relationships between objects. We do this in two parts: by building these spatial relationships between individual objects, and then by building these spatial relationships between individual proxes. Whether we do it as chains, trees, or what is an implementation detail.

The triplet of objects would allow "between" and "among." We'll see.

**Usage Requirements:**

1. **Must** accept legal stances and poses
2. **Must** interact with the [ConsentSystem](./ConsentSystem)
3. **Must (dependent)** interact with the [ClusterSystem](./ClusterSystem)
4. **Must** support inventory
5. **Should** abstract common prox/inventory processes
6. **Should** interact with the [SoundSystem](./SoundSystem)

This is all pretty vague, but it gives us a good description of what we want proxes to do. First, they have to arrange items in a way that good room descriptions can be created. Fifth, inventory should be built with this prox system. Sixth, proxes need to interact with the [SoundSystem](./SoundSystem). These are probably already done with the prox system outlined here.

Second, objects must create appropriate proxes for legal stances and poses. Lean on wall would be OK, but lean on floor would not. Open issue as to how to do that.

Third, certain proxes need to be defined by intimate. In the current implementation, this would be done via the adjacent prox and a certain list of prepositions.

Fourth, we need the proxes to help out with clustering. Probably, any system we come up will be fine, and the exact itneraction will be an implementation detail.

**Creating Prox Requirements:**

1. **Must (dependent)** fall back to a default object if no static object is given to prox to
2. **Must** fall back to a default preposition if no static preposition is given to prox to
3. **Must** determine which object is more important if no importance is given
4. **Should** choose which of two objects in prox is more important even if an importance is given
5. **Should** choose which of two synonymous prepositions is more apt

The heart of the prox system is really built into how they're created. First, the system needs to do it with insufficient information: mainly, it needs to figure out what to do if there's no static object and no preposition (ie, "drop grenade" really only gives an active object, and the system will have to figure out everything else). Beyond that, it would be nice if the system can clean things up by making the most important objects parents and by decreasing the number of prepositions that are used by getting rid of synonyms.

**Deleting Prox Requirements:**

1. **Must (dependent)** automatically rebuild series of proxes when objects are removed

In our current system, if something comes out of the middle of a prox chain, the prox chain needs to rebuild already it.

**Moving Through Proxes Requirements:**

1. **Must** allow a volitional to move straight into a prox
2. **Should** automatically move into a prox if an action requires it
3. **May** allow a volitional to slowly move toward something
4. **May** allow a volitional to slowly move away from something

So the final question is, how are these proxes used? You need to be able to move into a prox you see (ie, "sit down at table") at the most basic level. You should also automatically move into a prox if you have to be there for a certain action (ie, "pick lock" moves you into the "near door" prox). Finally "approach" and "leave" commands would be nice, which move you through proxes until you're right up next to something. This is partially defined by the [ConsentSystem](.ConsentSystem.md) .

------

------

References:

- Other systems that impacts the Proximity System:


- System that the Proximity System impacts: [ConsentSystem](./ConsentSystem.md) , [StanceGaitPoseSystem](./StanceGaitPoseSystem.md), [EvocationSystem](./EvocationSystem.md) , [SoundSystem](./SoundSystem.md)

------

A earlier version of the Proximity System Tech Summary was archived at:

- <http://www.kanga.nu/archives/MUD-Dev-L/archive/msg00414.html>

------

------

Proxes! I've thought about these a bit now. They are not really anything but a subset of all NRef's, i.e. pointers to certain details. There's much less to an initial implementation than I thought:

- Generalize the stance_target property in stance.c to handle NRef's, and update the stance commands as well as the general description code to handle this generalization.
  - DONE
- Add a registration/callback mechanism for proxes that can move, i.e. certain prime Nref's -- if a sword lies next to me and I leave, the sword must be informed that I've left, otherwise I could return and it would still be lying near me, which is not how it ought to work.
  - DONE
- Generalize the destination attribute to exits to point out not only the target room, but the target exit/entrance there. An arch in room 1 leading into room 2 should point to the other side of the arch in 2... again, this is just a matter of allowing a NRef instead of an object.
  - Zell 991001 Done
- Fix all the things that I'm not thinking of just now.
  - Probably not done, but c'est la vie ! :-)
- Release Critical

------

So what remains? As of today, November 9th, there is an implementation in place that handles the core behaviour of what we originally envisioned proxes were. The room-description displays prox information and the basic movement routines respect them. What's still lacking are higher level things;

- The 'approach' functionality, for picking up items, for consensual social verbs, eventually for combat;
- The 'sitting around a table in a pub talking amongst ourselves' scenario

and more. However, these are really

applications

 of proxes, not core support, and so I'm going to consider this entry done.

- - Zell 991109

-- [ParWinzell]() - 01 Oct 1999
