## Quick Summary Detail System

### Object Details

Every physical object in the game may have one or more more details. Each detail can be thought of as sub-objects within a master object, giving that object greater depth and opportunity for player interaction. In a room object, for example, there could be a detail for each of the doors, a fireplace, an alcove, and so on. For a character object, there could be details describing its face, hands, feet, and so forth.

Details can be overlapping. Their purpose is to describe everything within an object, not to do so without repetition. So, "doors" might be one detail which describes all of the doors together, while each door could be an individual detail as well.

### ID

Every detail has an ID, a Story Builder reference that along with the name of the object that uniquely points the detail. This name is not visible to players, but is used by other objects to refer back to this object. Detail IDs can be anything, but my convention the name `default` is used for the prime detail, and every other detail is typically least unique first, most unique last, i.e. if there are multiple walls, the IDs might be `wall-east` and `wall-west`. The object name combined with the detail name is known as an 'nref', for instance `NREF(Examples:complete:desc:room-go-nowhere|exit-south)`

### Prime Details

One detail within an object is usually defined as the Prime Detail. This is a default detail. It contains all of the detail variables for the main object. For example, every detail can have a "look" description, as noted by the [DescriptionSystem](./DescriptionSystem.md). If a player types "look" or "look at room", he'll get the look description associated with the prime detail.

It is possible for an object to have multiple prime details. This typically happens when faces are used, as described in [DetailSystem](http://twiki.skotos.net/twiki/bin/view/Builders/DetailSystem). If there are multiple prime details the most appropriate one for each individual viewer will be chosen as the default detail.

### Definite

Some details need to be definate, such as 'the altar' rather then 'an altar', or are proper names like "Rodger" or 'Excalibur'. Selecting the `Def` flag means that no article such as 'a' or 'an' will be used with the brief description is used, so the brief needs to contain the article (if any) such as 'the altar'.

### Abstract

Some details refer to things in the abstract. Typically these are things that you should not be able to point to, such as smell, wind, darkness, etc. If a detail is abstract, you may not interact with it using a verb, though you may look and examine it. If you should be able to point to it, for instance "point at sun" then don't use the Abstract flag, instead turn off all prepositions (see below).

### Face

Not used often, but faces allow details will look differently depending upon where you are viewing them from. By defining Faces, multiple details can be associated with the same real, physical "thing". For example, a tent might have an Inside Face, which would be one detail, with the Face variable set to Inside, and an Outside Face, which would be a second detail, with the Face variable set to Outside.

There are other possibilities for multiple faces. That same tent could have an "Above" and a "Below" face, for example, to ensure that skydivers and tunnelers each have their own unique view of the object.

Objects with multiple face details may have multiple prime details. In the case of the tent, both the detail with the inside face and the detail with outside face could be marked as prime details. Where a viewer is standing would determine which prime detail is actually used as the default.

### Hidden Details

Although details are permanent parts of objects, they can be temporarily hidden. If a detail is hidden, players can neither view it nor interact with it. A StoryBuilder[?](http://twiki.skotos.net/twiki/bin/edit/Builders/StoryBuilder?topicparent=Builders.DetailSystem) can make a detail hidden by marking the "Hidden" checkbox on the detail. In addition, details can be marked as hidden (or not) by using SAM, Bilbo, or Merry. Hidden (disabled) details have a number of uses:

- You can write two prime details of an object, one with the normal description, the other with the different names and adjectives. Then turn one off and the other on based on viewing something (say with SAM) or by doing an an action with something (with BILBO). For instance, a frog that will be transformed into a prince.


- Objects that should be secret can be revealed by taking certain actions. For instance, the classic secret door. Make an exit detail that is hidden that is only revealed when a certain action takes place (using BILBO) or if something is viewed (using SAM). Be aware that this effect should only be for those cases when everyone should be able to interact with the revealed detail after the change. Don't use this technique for situations where one person would see and be able to interact with the detail because of an action, but others would not be able to -- we'll have another system for that type of thing.

### Luminosity

Some details emit light, for instance a lamp, the sun, a window, etc. Luminosity defines how much light this detail contributes to the total light in the room. See [LightSystem](./LightSystem.md) for information on how this works, and [DescriptionSystem](./DescriptionSystem) on how luminosity affects descriptions.

### Prepositions

Objects may be approachable in a variety of ways. The preposition list shows what proximities are allowed by an object (see [ProximitySystem](./ProximitySystem) for more information). For instance, you can be 'under a streetlamp' but you probably can't be 'on' it.

The possible prepositions are: `close` (close to, close by, close beside), `against`, `under` (beneath, below), `on` (upon, on top of, across, atop), `inside` (in, within), `near` (by, nearby, at), `over` (above), `behind`, `before` (in front of), and `beside` (next to).

Note that if `near` is selected, it will be the default preposition used if no preposition is defined by the player. For instance "approach bench" will result in the player's character being "near the bench". If `near` is not selected, the first allowable preposition on the list will be used by default.

If no prepositions are selected, the object is not approachable, but still can be referred to with distant verbs, for instance "point at sun".

### Detail Names

All details have names associated with them. These are nouns which can be used to name the detail. For example, if there is a fireplace detail in a room object it could have the names "fireplace" and "hearth". Using any of these names will access the detail. Every detail name should also include the plural version of the name "fireplaces" and "hearths" so that if there is more then one of them they can be identified as a group.

### Adjectives

Typically adjectives will be required to further differentiate details in a room. If there were two door details, one might have the adjectives "white", "wood", "northern", and "north" associated with it, while another might use the adjectives "steel", "eastern", and "east". Many adjectives will be shared between details (multiple doors might be wood, for example).

### Descriptions

Being able to associate a number of variables with specific details allows StoryBuilders[?](http://twiki.skotos.net/twiki/bin/edit/Builders/StoryBuilders?topicparent=Builders.DetailSystem) a great deal of power, but the use of details really pays off in the types of descriptions that it allows. You can build an arbitrarily large numbers of descriptions into the details of an object. These could describe how the detail smells, how it feels, or how it looks. In particular SkotOS defines many types of detail descriptions which are related to sight.

These are all discussed in the [DescriptionSystem](./DescriptionSystem.md) document.

### Exits

Invariably, objects have exits. To support this, a detail can have exit information associated with it. The doors and windows in a room, for example, can be set as detail-based exits. Where the exits lead to is clearly defined by a woe. Directions can also be associated with the exit variables, so that, say, when a player types "north" an exit detail can claim that verb and send the player off as appropriate.

There are a variety of attributes for exits:

- **dest** -- The room and returning exit that this exit attaches to, in the form NREF(Woe name|detail)
- **door** -- If true, this detail can be opened and closed, such a door or lid.
- **never_obvious** -- True for invisible exits, such as a trap door
- **obvious_when_open** -- True for exits that are only visible when open
- **hidden_when_closed** -- True for exits that disappear when closed, such as many secret doors
- **separate** -- True for doors where closing or locking one side doesn't affect the other
- **keycode** -- A text string. The door may only be locked by a key with the correct property `base:keycode` with the same value.
- **self-locking** -- true for doors that should automatically lock when they close
- **deadbolt** -- true for doors on the side that they should be lockable without a key
- **Base:Climbable** & **Base:Climbable - dest** -- Not used at this time.