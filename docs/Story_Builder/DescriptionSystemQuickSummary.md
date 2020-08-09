## QuickSummary: Description System

### Basic Descriptions

- **brief** - used to constructing output, a noun phrase
- **pbrief** - used to construct output for clustering and plural objects in the CombinablesSystem[?](http://twiki.skotos.net/twiki/bin/edit/Builders/CombinablesSystem?topicparent=Builders.DescriptionSystem)
- **glance** - currently used only rooms, given when arriving in room.
- **look** - default look desc used for non-dark luminosities, otherwise "It's dark."
- **examine** - default examine desc used for non-dark luminosities, otherwise "It's dark."

### [LightSystem](./LightSystem) Descriptions:

- **worn** - used instead of brief when the object is worn. For instance brief "pants" that when worn can be "tight pants".
- **look-full** - look desc used all luminosities, will not give "It's dark." message. Useful for SAM total control over light levels.
- **look-dark** - look desc used for all luminiosities
- **look-dim** - look desc used when when ambient luminosity <1
- **look-bright** - look desc used when ambient luminosity >1 and <10
- **look-superbright** - look desc ambient luminosity > 100,000, used typically for sunlit locations. If no superbright exists, the bright is looked for.
- **examine-full** - examine desc used for all luminiosities, will not give "It's dark." message. Useful for [SamSystem](./SamSystem) total control over light levels.
- **examine-dark** - examine desc used when when ambient luminosity <1
- **examine-dim** - examine desc used when ambient luminosity >1 and <10
- **examine-bright** - examine desc used when ambient luminosity >10
- **examine-superbright** - examine desc ambient luminosity > 100,000, used typically for sunlit locations. If no superbright exists, the bright is looked for.

### [ClientSystem](./ClientSystem) Descriptions:

- **anyurl** - upon examine or look of this detail, the URL will be displayed in a popup window
- **arturl** - upon examine or look of this detail, the graphic-only URL in the desc will be displayed in a popup window, and the window will be resized
- **mapurl** - the map image displayed in the client
- **bigmapurl** - the large map displayed when you click the map image in the client

### Misc. Descriptions

- **myleave** - Obsolete. Used in some old exits. Use react-post:movement/leave and react3-post:movement/leave
- **worn** - Used as the brief instead of the normal brief if the item is worn.

### [SignalSystem](./SignalSystem) Descriptions

Almost every signal in the [SignalSystem](http://twiki.skotos.net/twiki/bin/view/Builders/SignalSystem) can be trapped in a signal description. What follows is are the categories, and a brief list of some of the more commonly used signals.

- `react-pre:*`

  \- Aborts the action before it happens and and outputs to the actor the desc text (and to no one else). Even if the action would have generated a failure message (for example: You cannot get close...) this desc will be used instead.

  - In

    Examples:complete:desc:room-go-nowhere|lamp:

    - `React-pre:touch-dob: You can't quite reach the ceiling lamp fixture.`


- `react:*`

  Aborts the action, but only after approaches and other systems checks are tried first, and then outputs to the actor the desc text (and to no one else, though other people may see the approach and other system checks).

  - In

    Examples:complete:desc:room-go-nowhere|mousehole:

    - `react:enter-into: You are too big to pass through the mousehole.`

  - In ?

    - `react:unwear: The cursed ring can't be removed!`


- `react-desc:*`

  Gives the desc to the actor when the action happens, but does not abort the action.

  - In

    Examples:complete:desc:room-go-nowhere|exit-south:

    - `react-desc:movement/arrive: ... and arrive at ...`
    - `react-desc:movement/leave: You pass through the door to the south ...`
    - `react-desc:close-what: You rapidly close the door.`
    - `react-desc:open-what: You rapidly open the door.`


- `react3-desc:*`

  Gives the desc to the people in the room with the actor when the action happens.

  - In

    Examples:complete:desc:room-go-nowhere|exit-south:

    - `react3-desc:movement/arrive: <describe what="$(Actor)" cap="1"/> arrives from the south, through the door.`
    - `react3-desc:movement/leave: <describe what="$(Actor)" cap="1"/> begins leaving through the southern door.`
    - `react3-desc:close-what: <describe what="$(Actor)" cap="1"/> rapidly closes the door.`
    - `react3-desc:open-what: <describe what="$(Actor)" cap="1"/> rapidly opens the door.`


- show-desc:*

  Gives the desc to the people in the room where the target is.

  - In

    Examples:complete:desc:room-go-nowhere|exit-south:

    - `show-desc:door/closes: <describe what="$(target)"/> slams shut!`
    - `show-desc:door/opens: <describe what="$(target)"/> flies open!`


- react-post:*

  \- output the text after the action happens

  - In

    Examples:complete:desc:rose-red-w/thorns|default:

    - `React-post:sense/smell-dob: The deep red rose has a rich, rose-scented floral bouquet, with a slight sweet undertone of musk.`
    - `React-post:sense/taste-dob: The red rose petals have a somewhat bitter, grassy taste, with a slightly fruity aftertaste somewhat like a ripe peach. The petals have a a kind of a soft gritty texture and are somewhat succulent.`
    - `React-post:sense/touch-dob: The stem of the red rose is stiff and has many sharp thorns, but the petals are velvety soft.`

### Description References

Descriptions are available as [DerivedProperties](./DerivedProperties), thus can be retrieved or set like other properties:

- In a room or object using an object reference: `$object.details:{detail-name}:description:{description-name}`, for example `$this.details:default:descriptions:examine`
- In a room or object using an nref: `$nref.detail:description:{description-name}`, for example `$target.detail:descriptions:look`
- In a detail, if you wish to include the [UrParent](./UrParent)'s description as part of the description for the child, use the special `$(parent-desc)` variable.

