# The Signal System — a Technical Summary

## Introduction

When things happen to objects in the game world, signals are triggered. These signals are hooks onto which developers hang code; code that makes special effects happen, making the world more dynamic.

When a player takes a bite out of a donut, for example, there are certain things the system always does automatically -- reduces the bulk of the donut, or if it's the last bite, destroys the donut object.

A developer can make a magical donut with fragments of scripting code in it, code that catches the various 'eat' signals, so that something special happens when the player takes a bite. For example, perhaps each nibble you take out of the donut realistically increases the player's weight by a small amount 24 hours later? That would be quite simple to do using signals.

## Signal Names

Currently all scripting code fragments are written Merry, and the code is stored in properties on the object. These properties have names, and the names are chosen according to a specific naming scheme — all Merry script property names start with the text `merry:`.

After this language identifier comes the type of signal. The current list of types is:

- act
- react
- witness
- show

The last part of the property name identies which actual signal it is. For example, in the case of the eaten donut, the signal is named `eat`. To catch a player eating the magic donut, then, you would place your code in the property e.g. `merry:react:eat`.

## Objects And More On Signal Types

There are usually three groups of objets that may take an interest in any given physical event --

- The object performing the action -- e.g. the player eating the donut. This is where the **act** type is used.
- The target of the action -- e.g. the donut. Use the **react** type here.
- The other objects in the room, who observe the action taking place but do not take part in it. Use **witness** here.

Those are the most straight-forward groups, but there are others --

- When unlocking a door, the player `acts`, the door `react`'s', but what about the key? The key should also have signals run in it. I may want a key to break when used, or maybe set the player on fire, or... whatever. Thus there is need for more than one type of `react`; one for each class of objects in an action.
- When I'm moving from one room to another, there are witnesses both in the room I'm leaving, and in the room I'm entering; there is also both an entrance detail and an exit detail that should be given the chance to react.
- If I shout very loudly, the shout may reach 20 rooms. Each one is full of witnesses.
- It may be the environment itself should also always be given a chance to react to whatever is being done in it.

For all these reasons, we extend the signal type name. Thus when unlocking a door, we have:

- `merry:act:unlock`
  - in the actor
- `merry:react:unlock`
  - in the lock
- `merry:react:unlock-using`
  - in the key
- `merry:react:unlock-in`
  - in the room
- `merry:witness:unlock`
  - in everyone watching

When moving between rooms,

- `merry:act:enter`

- `merry:react:enter-into`

  - in the current room's exit detail

  - > Note: merry:react-pre:enter-into is invalid. ~Death

- `merry:react:enter-from`

  - in the destination room's entrance detail

  - > Note: merry:react-pre:enter-from is invalid. Keep in mind, -pre is not required here as it does fire before the character uses the passage. ~Death

- ``merry:witness:enter-into``

  - in everyone watching in current room

- `merry:witness:enter-from`

  - in everyone watching in destination room

## Signal Timing

If all this weren't complicated enough, there is yet another dimensions in which these signals spread out — time. Each signal is actually triggered three times, under different conditions, and each type is used for a different purpose. (There is also the -desc timing used primarily to change output of movement.)

First, the `pre` signal is triggered almost immediately when the action begins. It occurs before most error checking is done on the parameters. If I try to eat an inedible object, the action should abort with a "You can't eat that." error message, but not before the pre signal has been run. The developer may replace the standard error message, and thus respond in some interesting way to the player trying to eat e.g. a doorknob.

Signals return true or false. The pre signal's return value determines wether to abort the action or let it proceed. This signal is also the only one that is allowed to actually modify the signal's input values. For example, if I had a mute curse, I might have a script that intercepts socials with evokes and turns the evoke into "...".

The main signal itself -- is run when the action is just about to irreversibly execute. All sanity checking has been done at this point and the action *will* execute unless the signal forbids it. Once again, the return value determines wether to abort the signal or not.

Note: Any signal that does choose to abort the action must provide a message to the user explaining why, or the action will fail silently.

The last signal, the `post` one, is used when the action has already succeeded. It is run in a separate thread, and its return value is ignored (it obviously cannot stop the action).

> Kalle: Its return value **IS NOT IGNORED!**  Its return value determines if other post scripts for the same signal may trigger. For example, if 3 gnomes in a room all witness the "entering" signal's post stage, and the first one returns false, the other two's witness scripts will never, ever execute! It can output extra text to the user, of course, but more commonly it is used to handle side-effects of the action, often involving delays. Eating a poisoned donut, the post version of the eat signal may be used to start a 6-hour delay, at the end of which the player vomits profusely and, perhaps, dies. NB Witness does not allow -post signals - Leanan Witness does allow -post signals, but not all signals support post. In fact, witnessing a login attempt MUST be done as -post, because the login signal only triggers the post action. -Kalle.

> GEIST - July 31, 2011 - Through various experimentation I have not been able to get react-post:login or witness-post:login to work. I have, however, confirmed that act-post:login does work. If it didn't we'd have big problems, but...

Examples of valid signal names, then, include

- `merry:witness-re:move-in`
- ​

## -desc

> -desc timing needs to be documented)

## Signals and Socials

Social verbs are special in that they normally only cause messages (and possibly prox movement and consent requests), but it is possible to hook actions into specific verbs as well. For example, the developer may write a react-script for 'kiss' in a ring, and trigger some magical effect when a player kisses a ring.

A word of warning first: it is very easy to accidentally create 'guess the verb' puzzles using this functionality -- i.e. situations where the player is forced to type a specific verb to trigger a script, when from the player's perspective any of the verb's synonyms ought to have the same effect. Guess-the-verb puzzles are **strongly** discouraged. If you can 'tug' a rope, you should be able to 'pull' it with the same effect.

> A system of semantic tags has been developed to address this problem, but it is not fully documented yet and likely has some missing or broken semantics in it. For now, you have to create properties for each verb synonym, and use inheritance to point to a single script that handles all the verbs in the same way.

The script names for socials are constructed from the imperative form of the social and, in the case of the react scripts, the role identifiers of the various social objects that are reacting. These role identifiers are not yet settled for all the social verbs. At the moment, they tend to be 'dob' or 'iob' most of the time, for direct object and indirect object.

If we wanted to make a sword that glimmers when it's waved, you'd create a script called `merry:react:wave-dob` (because when you wave a sword, the sword is the direct object -- the `dob`). If you wanted to make a guard who reacts when you wave at him, you'd give him a `merry:react:merry-iob` (because when you wave -at- something, it's an indirect object -- the `iob`).

So these `dob` and `iob` role names are identical to the 'using' role in the unlock signal example above. Soon, socials should receive more sensible role names, more like 'using' than the current unintuitive 'iob'.

**Signal Reference**

Here is a table of existing signals, the roles they take, and the arguments they set:

| signal         | pre  | sig  | post | arg      | type    | description                            | role  | description                           |
| -------------- | ---- | ---- | ---- | -------- | ------- | -------------------------------------- | ----- | ------------------------------------- |
|                |      |      |      |          |         |                                        |       |                                       |
| quit           |      |      | X    |          |         |                                        |       |                                       |
| disconnect     |      |      | X    |          |         | disconnect from the game               |       |                                       |
| linkdie        |      |      | X    |          |         |                                        |       |                                       |
| login          |      |      | X    | default  | obj     | fall-back starting location            |       |                                       |
| open           | X    | X    | X    | what     | NRef *  | nrefs to attempt to open               | *     | valid doors to attempt to open        |
| close          | X    | X    | X    | what     | NRef *  | nrefs to attempt to close              | *     | valid doors to attempt to close       |
| unlock         | X    | X    | X    | what     | NRef *  | nrefs to attempt to unlock             | *     | valid doors to attempt to unlock      |
|                |      |      |      | keys     | obj *   | keys available for use                 | using | keys available for use                |
|                |      |      |      | explicit | int     | did the player specify keys to use?    | in    | the environment of the player         |
| lock           | X    | X    | X    | what     | NRef *  | nrefs to attempt to lock               | *     | valid doors to attempt to lock        |
|                |      |      |      | keys     | obj *   | keys available for use                 | using | keys available for use                |
|                |      |      |      | explicit | int     | did the player specify keys to use?    | in    | the environment of the player         |
| drop           | X    | X    | X    | what     | NRef *  | nrefs to attempt to drop               | *     | valid objects to attempt to drop      |
|                |      |      |      |          |         |                                        | into  | where the dropped objects go          |
| take           | X    | X    | X    | what     | NRef *  | nrefs to attempt to take               | *     | valid objects to attempt to drop      |
|                |      |      |      |          |         |                                        | from  | where each object is taken from       |
| place          | X    | X    | X    | what     | NRef *  | nrefs to attempt to place              | *     | valid objects to attempt to place     |
|                |      |      |      | where    | object  | destination of objects                 | onto  | destination for each object           |
|                |      |      |      | prep     | int     | intended preposition of objects        |       |                                       |
|                |      |      |      | stance   | int     | intended stance of objects             |       |                                       |
|                |      |      |      | v2       | string  | second-person form of verb used        |       |                                       |
|                |      |      |      | v3       | string  | third-person form of verb used         |       |                                       |
| drag           | X    | X    | X    | victim   | object  | who to drag                            | *     | who to drag                           |
| eat            | X    | X    | X    | what     | NRef    | nref to attempt to eat                 | *     | valid object to attempt to eat        |
| drink          | X    | X    | X    | what     | NRef    | nref to attempt to drink               | *     | valid object to attempt to drink      |
| wear           | X    | X    | X    | articles | obj *   | objects to attempt to wear             | *     | valid objects to attempt to wear      |
| wield          | X    | X    | X    | article  | object  | object to attempt to wield             | *     | valid object to attempt to wield      |
| unwear         | X    | X    | X    | articles | obj *   | objects to attempt to unwear           | *     | valid objects to attempt to unwear    |
| unwield        | X    | X    | X    | article  | object  | object to attempt to unwield           | *     | valid object to attempt to unwield    |
| allow          | X    | X    | X    | who      | object  | object being allowed                   | *     | object being allowed                  |
| deny           | X    | X    | X    | who      | object  | object being denied                    | *     | object being denied                   |
| demand-consent | X    | X    | X    | target   | object  | object being asked                     | *     | object being asked                    |
|                |      |      |      | message  | string  | what is displayed to target            |       |                                       |
|                |      |      |      | action   | string  | action we want to perform              |       |                                       |
|                |      |      |      | args     | mapping | arguments of action we want to perform |       |                                       |
| enter          | X    | X    | X    | what     | NRef    | detail we're trying to enter           | *     | detail we're trying to enter          |
|                |      |      |      |          |         |                                        | from  | detail we emerge from                 |
|                |      |      |      |          |         |                                        | into  | detail we enter into                  |
| teleport       | X    | X    | X    | dest     | object  | where to go                            | *     | where to go                           |
| approach       | X    | X    | X    | target   | NRef    | which detail to approach               | *     | which detail to approach              |
| leave          | X    | X    | X    |          |         |                                        |       |                                       |
| offer          | X    | X    | X    | what     | obj *   | objects being offered                  | *     | objects being offered                 |
|                |      |      |      | who      | object  | to whom they're offered                | to    | to whom they're offered               |
| accept         | X    | X    | X    | what     | obj *   | objects being accepted                 | *     | objects being accepted                |
|                |      |      |      | who      | object  | from whom they're accepted             | from  | from whom they're accepted            |
| revoke         | X    |      | X    | who      | object  | object to whom revoked offer was made  | *     | object to whom revoked offer was made |
| refuse         | X    |      | X    | who      | object  | whose offer to revoke                  | *     | whose offer to revoke                 |
| stance         | X    | X    | X    | target   | NRef    | prox target of stance (e.g. altar)     | *     | prox target of stance                 |
|                |      |      |      | stance   | int     | stance to take, e.g. STANCE_KNEEL      |       |                                       |
|                |      |      |      | prep     | int     | preposition to use, e.g. PREP_BEFORE   |       |                                       |

------

## Deprecated

There are two old signals that should not be used, but there may be some legacy code using them.

- auto
- solo

There is also an old language Bilbo, which also worked with the Signal System. It's prefix was `bilbo:` — some legacy objects may still have Bilbo code in it, but you should not re-use them.Old Emails

------

## Old Emails

### doors

```
> My milk container now emits it is closing once spawned... looks rather
> strange. I haven't used it in ages, but originally it didn't emit that
> message. Again, previously worked fine.
> 
> <Lazarus:rooms:asylum:props:milk>
```

The start script does:

```
0010  if this.details:$detail:exit:door = 1 then
           set this.details:$detail:exit:closed to 1
```

which closes the door... this was silent in the past but now has a message. I realize that's a backwards compatiblity breaker -- sorry about that. You can fix it by changing the property to:

```
0010  if this.details:$detail:exit:door = 1 then
           set this.details:$detail:exit:closed:silently to 1
                                                ^^^^^^^^
```

i.e. adding 'silently' at the end.

-- [ParWinzell]() - 25 Feb 2003

### all evoke signals

```
----- Original Message ----- 
From: "Par Winzell" <zell@skotos.net>
To: <skotos-seven@skotos.net>
Sent: Wednesday, February 19, 2003 1:26 PM
Subject: Re: [skotos-seven] Distant emits


Scott,

> I guess I'm suggesting a signal for merry:witness:emit.
>   The only information it needs to catch is any text that
>   is sent to the room. Of course, since I have not seen
>   the underlying code, this might be impossible. But
>   wouldn't this be generally useful in lots of different
>   situations? Spying? Watching several room at once?
>   Logging of events and occurrences for later use?

Firing an event for every single line of text is questionable, for
performance reasons if nothing else. However, one -is- fired for each
command that has been converted to Merry, and that's what the lighter
examplified, if you looked closer. It defines three scripts:

       <Core:Property property="merry:witness-post:command">
       <Core:Property property="merry:witness-post:evoke">
       <Core:Property property="merry:witness-post:take">

The COMMAND event is triggered for every dynamic verb ('social') that is
executed. That's everything a player types except the actions that have
not yet been converted to Merry.

The EVOKE event is triggered whenever somebody does something that
evokes something. This means you can automatically spy on everything
that is actually said in a room -- every action that involves a
free-form text string. An evoke.

The TAKE is just there as an example of catching an action that has yet
to be converted to Merry. It's possible that I could rig things so that
the COMMAND is sent in those cases, too, if you think that'd be useful.

Zell
```

