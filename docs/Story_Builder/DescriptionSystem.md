# The Description System - A Technical Summary

See Also: [DetailSystem](./DetailSystem.md), [ProximitySystem](./ProximitySystem.md)

Articles: [Trials, Triumphs & Trivialities #48: Building Blocks: Rooms](http://www.skotos.net/articles/TTnT_48.shtml) [Triumps, Trials & Trivalities #50: Building Blocks: Details](http://www.skotos.net/articles/TTnT_50.shtml) & [Room Attributes](http://www.skotos.net/articles/trenches03.shtml)

## Quick Summary

See [Description System Quick Summary](./DescriptionSystemQuickSummary.md)

## Sight Descriptions

Most human perception is based on sight. For this reason a number of sight-based description variables have been defined for details:

- Brief
- Glance
- Look
- Examine
- Distant

The explanations of the descriptions that follow show how the basic SkotOS system is set up. Individual Story Builders will be able to define them as they see fit (or they can use the defaults).

These sight descriptions are most commonly associated with prime details--a room will likely have brief, glance, look, examine, and distant descriptions. However, any detail could have any of the description variables associated with it. A wooden shelf with chewing gum stuck under it might include a description of the chewing gum in the wooden shelf's examine description.

Many of these descriptions, particularly glance and look, can actually be multisensory, including the most important smells, sounds, etc. related to a detail, not just the sights.

### Brief Descriptions

This is the shortest possible description. It is a noun phrase that is used when the description is embedded in a sentence. "The King's dining room", "the dining room", and "an opulent dining room" are all examples of brief descriptions.

### Glance Descriptions

Synonyms: glance, scan

A glance is the initial description of a detail. It's most commonly used when a player walks into a room, in which case he sees the glance description of the room's prime detail. A glance is usually only a sentence or two long. It doesn't include any secret or hidden information. The SkotOS library will tend to add a sentence or two to the glance description, mentioning portables, people, and other mobile objects.

### Look Descriptions

Synonyms: look, peer, squint, watch

A look description is longer than a glance description. It includes everything obvious about the detail. The look description of the prime detail should include information on all other details, particularly including all exits. The SkotOS Server will add information to the look description, mentioning all objects in the room. These objects will be clustered, as described in the Clustering System.

### Examine Descriptions

Synonyms: examine, survey, reconnoiter, search

The examine description provides the maximum amount of information about a detail. It's used when someone takes the time to carefully examine a detail. In a skill-based system there might be skill tests before an examine description is actually offered up to a player.

### Distant Descriptions

Any type of description might include, as embedded text, a distant description of another object in the game. This is done by using SAM code to include one of the types of description from the other object in your current object.

For example, in Galactic Emperor: Hegemony's Abode, the look description of a northern door includes the following description:

```
A glowing archway frames the entrance to a passageway in the north. The archway seems to pulsate with a pale energy similar to that visible through the floor. On the other side of the archway you can see <describe-view type="glance" view="OBJ(Hegemony:rooms:HologramNexus)"/>.
```

The describe-view SAM provides a distant description of the Hologram Nexus, using that room's glance description. Meanwhile the same detail's examine description provides a longer ("look") distant description of the distant room:

```
A glowing archway frames the entrance to a passageway in the north. The archway seems to pulsate with a pale energy similar to that visible through the floor. On the other side of the archway you can see: <describe-view type="look" view="OBJ(Hegemony:rooms:HologramNexus)"/>
```

## Other Descriptions

We have discussed adding other types of descriptions to our games. They would be easy to implement, but no verbs are currently set up to access them. Top ideas include:

- Hearing
- Touch
- Smell
- Taste

Temperature, pain, and somatic senses aren't attached to specific description variables, but these could be included if required.

The following descriptions outline our ideas regarding these other sense descriptions.

### Hearing

Synonyms: hear, listen, monitor, eavesdrop, overhear

Hearing refers to listening for sounds that a detail might make: the crackling of a fireplace, the whirring of an electronic turbine, or the creaking of an old house. Many hearing details will be dynamic, such as crowd noises created on the other side of a door (see the technical summary on the Sound System).

### Touching

Synonyms: touch, feel, fondle, rub, handle, poke

Touching relates to what you can feel about a detail: the icky, oozy feeling of the Necronomicon, the cracked, scaly feel of a faux alligator belt, or the fuzzy feel of a navel orange. These verbs may require interaction with the Consent System to be succesfully used.

### Smelling

Synonyms: smell, sniff, inhale

Smelling refers to the odors of a detail: the burnt, wooden smell of a fireplace, the rotting smell of garbage, or the earthy smell of a recent rainstorm.

### Tasting

Synonyms: taste, lick, sample

Tasting refers to the, well, taste of a detail: the burnt, wooden taste of a fireplace, the salty taste of sardines, or the clean taste of fresh water.

## OLD UNSORTED DOCS & NOTES

### Odd Thoughts

merry:react:stop allows a merry script to stop an object from being slayed. Probably a react:stop desc will do the same.

react-pre:offer-to might be able to be used to prevent someone from offering an item to a CNPC or PC.

react-desc:place could be interesting.

------

------

#### Misc. Notes

Whenever there is a merry:react-X:Y, there are descriptions

```
   react-X:Y
   react3-X:Y
   show-X:Y
```

All four phases can be caught (-pre, std, -desc, -post) . The implied return value is always false. Thus react-pre can be used to give error messages to the player before any sanity checking appens. Plain react (-prime) can be done when the action is just about to happen, but for some reason it shouldn't. The desc phase is overridden whenever the action should go through as usual but with a custom-made description... and the post phase, of course, to display an extra messages after the action has successfully completed.

The 'react' type goes to the actor. The 'react3' goes to the inventory of the environment of the actor. The 'show' goes to the inventory of the acting object.

> This syntax will become a bit obsolete soon, but I'm going to need to do an automatic conversion anyway, so go ahead and use it and feel fairly certain that your descriptions will continue working. -- Zell

#### Sense Notes

Verbs can have additional actions added to them. For instance, "sense/taste" has been added to taste, nibble, lick, gulp, guzzle, quaff, and swill. These are the verbs that work with the current Bilbo-based generic drink and food system. In addition, swallow also has been added, but is not part of the Bilbo-based generic drink and food system, and eat and drink are currently not socials (hopefully they will be soon.)

Having a sense/taste action available, means that if you add a "react-post:sense/taste-dob" description to an object, all the above verbs will give that output, i.e.:

```
React-post:sense/taste-dob: [
  The red rose petals have a somewhat bitter, grassy taste, with a slightly
fruity aftertaste somewhat like a ripe peach. The petals have a a kind of a
soft gritty texture and are somewhat succulent.
]
```

Results in:

```
> lick my rose
You lick your red rose.
The red rose petals have a somewhat bitter, grassy taste, with a slightly
fruity aftertaste somewhat like a ripe peach. The petals have a a kind of a
soft gritty texture and are somewhat succulent.
```

Note: if you add a react-post:sense/taste-dob to a food or drink that is in the existing generic [UrFood](./UrFood)/UrDrink system, you'll have to be a bit careful. As that system will also generate a taste, your sense/taste description should probably take that into effect

For example, if you had a child of "generic:food:fruit:apple" and you lick it you'll normally get:

```
> lick my apple
You lick your apple. Crisp and sweet.
>
```

If you add:

```
React-post:sense/taste-dob: [
  The apple has a slight aftertaste of bitterness.
]
```

Then when you:

```
> lick my apple
You lick your apple. Crisp and sweet.
The apple has a slight aftertaste of bitterness.
>
```

-- [ChristopherAllen]() - 07 Feb 2003
