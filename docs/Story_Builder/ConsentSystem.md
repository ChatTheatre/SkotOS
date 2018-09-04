# The Proximity System - a Technical Summary

See also: [ProximitySystem](./ProximitySystem.md)

Articles: [An Evening at Chez Skoot – Proxes Part 1: Consent](https://www.skotos.net/articles/chezskoot.html)

## Not With My Body, You Don't!

### Allowing and Repelling Invasions of Personal Space

Our games describe physical space, with persons moving about in them and performing actions. Some actions or motions can lead to unwanted (or at least unexpected) intimacy. The Consent System allows players to manage their "personal space," providing permissions or denials -- on a one-time, temporary, or permanent basis -- for other characters' actions in that space.

### Existing Approaches

Several methods are employed in existing MUDs and MUSHes to handle potentially 'harrassing' actions. Either some actions are just not available, or they are freely available (and easily abused), or available only after special permission. In Simutronics' game _Dragonrealms_ , some of the more 'intimate' consent-only actions are available only to married characters -- at a hefty charge.

### Problems Solved by Consent System

Proxes affect what players can choose to do to each other. Our Proximity system is consensual; it enforces a MUSH-style sensibility. Thus there are opportunities to deny or avoid characters who are becoming too intimately close.

------

## Summary of the Consent System

The Consent System activates when one character attempts to take up an _adjacent_ proximity with another, or attempts "intimate" actions when within that proximity.

### Adjacent Proximity

This is the category of direct relationships between two objects; one object is the parent, the other is the dependent. Prepositions normally indicate the nature of the relationship -- "Bob is next to the wall" indicates the wall is a parent, Bob is the dependent.

### Character

Any object with 'volition' is a character for the purpose of this section. This can be any object 'inhabited' by player or host, or any object with a 'brain.' In the examples below, the 'active' character is the one attempting (by use of some verb) to perform an action; the 'passive' character is the one the action effects. For attempts to establish an _adjacent_ proximity, the 'passive' character becomes the parent if the actions succeeds.

### Intimacy

Various prepositions can be used to establish and define an _adjacent_ proximity relationship. Some of these prepositions create especially 'personal' relationships, and are termed "intimate." Other actions within a character's _adjacent_ proximity are merely "un-intimate."

------

## Avoiding or Allowing Intimacy

There are three ways to resolve a consent test:
   * passive character provides or denies explicit permission
   * passive character moves to avoid being within _adjacent_ proximity
   * passive character has given implicit permission
   * a contest, test of skill, or some other use of physical action.

## Explicit Permission

Volitional objects, such as players, NPCs, and other living creatures, may refuse or avoid the too-close approach of another volitional. Certain commands like kiss, slap, etc. will require that the active character succeed in getting close (within "non-intimate" _adjacent_ proximity) to the receiving player first.

If Bob attempts to snuggle up to Carol (an "intimate" action), she will see the message "Bob begins to snuggle up to you -- D)eny, A)ccept, R)eturn, or C)ounter".

   * If Carol D)enies the approach, only Bob will know that the attempt failed. If Carol types nothing, the default is D)eny.
   * If Carol A)ccepts the approach, everyone will see "Bob snuggles up to Carol".
   * If Carol R)eturns the approach, everyone will see "Bob and Carol snuggle up together".
   * If Carol C)ounters the approach, everyone will see that Bob failed to get close -- perhaps "Carol rejects Bob's attempt to snuggle."

A decision will have to be made as to how much of the original 'action sentence' structure to preserve in the result descriptions.

As any objects that they drop will also be "intimately" _adjacent_ , once Bob is snuggling with Carol, either of them can pick up any dropped item without getting permission from the other.

A good description of this system in operation is available at AnEveningAtChezSkootProxesPart1Consent.

### Implicit Permission

When a character Accepts an "intimate" action by another character, this provides temporary permission (acceptance) for further "intimate" actions by both characters. This temporary permission vanishes when the characters are not in _adjacent_ proximity for more than ten seconds; thus they have ten seconds to return to _adjacent_ proximity without retriggering the Consent System. This allows for brief changes of stance, pose, or prox for expressive purposes.

See also the section "Automatic Consent," below.

### Avoiding Non-Intimate Adjacency

Players may also avoid "non-intimate" approaches by moving (changing their prox) before the attempt is complete.

For example, Bob approaches "near" Carol, then tries to "snuggle up" to her. Moving "near" Carol would place Bob in her _adjacent_ proximity, but not "intimately." Snuggling, however, is intimate. Carol can Deny the "intimate" approach, but can't do anything when Bob is just coming "near" --except move away. Carol might see, "Bob is going to sit near you; you have seven seconds to A)void this by leaving." There is a delay before Bob is successful; Carol can leave the table (if she chooses Avoid, or leaves by more ordinary ways), causing Bob's attempt to fail. Bob will remain at the table; he must start all over to try to approach Carol again.

### Contested Permission

In games with a combat system, characters will perform some 'intimate' actions without using the consent system. There will, however, have to be some sort of division between combat activities out-of-combat activities.

### Incapacity or Inability

If the passive character is unconscious, asleep, bound and tied, etc. they may be unable to deny permission to other characters.

### Automatic Response

Characters may establish, within their profiles (see ProfileSystem )or by other methods, lists of persons who receive pre-determined responses on consensual activities. A character might always "Deny Bob," or "Accept Ursula," or even "Counter anyone except June and Godzilla."

If Bob persists attempting to get close to Carol, she can put him on her "Deny" or "Avoid" lists, and all attempts to get close will automatically be denied or avoided without delay. Or, she can put him on her "Counter" list and he will automatically be opposed and everyone will see her rejection. She could also put him on her "Accept" or "Return" list, and every time he attempts to get close it will automatically succeed or be returned without delay.

With each of these lists, she can also specify that they only work with specific commands. For instance, Carol can allow a particular person to kiss her, but disallow other commands that require intimate proxes.

### Automatic Avoid

In most cases, attempts to enter the _adjacent_ prox (or other preposition proxes in the delayed category) can't be avoided without the target leaving his or her own prox first.

Though it is an imperfect solution, Carol can put Bob on her "Avoid" list, which will cause Bob's attempts to enter her delayed preposition (non-intimate) prox to be denied. Only she and Bob would see this failure. We are considering if a "Strongly Avoid" list will be necessary; if so, it would always allow Carol to publicly reject any of Bob's approaches.

-----

## Consent Settings

See the [ProfileSystem](./ProfileSystem.md).

-----

## State of Development

We should be so lucky.

### Open Issues in the Consent System

   * Should there be a "Strongly Avoid" option?

   * So we've slightly changed things, so that now all of the consent things are clearly defined as involving the "adjacent" prox. We don't have a "near" and a "close" anymore, which is good, because they were confusing. But, how does this relate to the approach command? Before you approached through the near and close proximities. Now? Probably, this can still be done the same, but it needs to be considered.

   * Will people abuse the near proxes, thus requiring consent for all prox operations rather then just intimate ones? Is a "strongly avoid" list required?

   * The consent system is probably still open to some abuses that we have not thought of.

   * Should we have a paranoia flag that automatically puts everyone on your "avoid" list?

   * The idea behind return is that if someone approaches you intimately, you could in return respond by doing another intimate action. A kiss could be returned with another kiss or a slap. How restrictive do we want to be as to what actions can be returned? What actions can be returned for each intimate action? How do we prompt the player as to what actions are possible?

   * What types of failure messages should people see when you counter different actions? How do the messages differ for the approacher, the counterer, and those nearby?

   * It is possible that some intimate actions should not end with both parties close, especially such casual actions such as shaking hands.

   * What's the default non-intimate preposition when approaching?

   * What's the default intimate preposition when approaching?

---

## TECHNICAL REQUIREMENTS

### General Requirements:

   1 Some actions are intimate and *Must(Dependent)* be restricted
   2 *May* restrict some non-intimate actions.
   3 *Must* provide a mechanism that allows consent

The whole purpose of the consent system: you can't do certain things unless people have allowed you into their personal space.

### Distance Requirements:

   1 *Must* define intimate space
   2 *Must* define which actions require intimate space
   3 *Should* define other actions which do not require intimate space, but DO require consent

Consent is related to two adjacent objects. This is a specific classification of proximity which is used to define a proximity relationship where one object is the parent and the other object is the child. It's a close distance.

Just exactly how close the proximity is depends upon the preposition that is used. "On" is clearly pretty darned intimate while "by" is probably not. A specific set of prepositions must be defined as intimate, for use with the prox system. Implicitly, the other prepositions are defined as non-intimate.

### Approach Requirements:

   1 *Must* allow characters to arbitrarily try to enter any non-intimate prox
   2 *Must* allow characters to arbitrarily try to enter any intimate prox
   3 *Should* support an approach command
   4 Approach *Should* allow character to first enter a default non-intimate prox
   5 Approach *Should* allow character to next enter a default intimate prox
   6 Consensual actions *Should* automatically approach to correct proximity if an intimate action is attempted.

So, how do you get into this adjacent proxes? The first thing you can do is enter an arbitrary command to get to the specific prox that you want: "stand by julie", "jump on jen". Alternatively, you can use the approach command: "approach janet"--"you are at the table with janet"; "approach janet"--"you are by janet"; "approach janet"--"you are near janet". Then there's a third option: if you type a consentual command, such as "kiss jasmine", you should automatically enter the right proximity, or at least try to.

### Casual (Semi-intimate) Consent Requirements:

   1 *Should* warn a character when someone enters their adjacent, non-intimate prox
   2 *Should* enforce a delay before an adjacent, non-intimate prox can be entered
   3 *Should* fail if person a character is trying to approach moves before approach is completed
   4 *May* allow characters to block entry into proximity via other means.

So, you're trying to get into an adjacent, non-intimate prox ("by"). You type in whatever command to get there "approach joanne" or "stand by jezebel". There is a delay of a few seconds. In that time the person you're trying to approach gets a message. They thus have a few seconds to move before the person steps up to them. If they do, the approach fails.

### Intimate Consent Requirements:

   1 *Must(Dependent)* queue requests (delay system required)
   2 *Must* prompt character when someone tries to enter their adjacent, intimate prox
   3 *Must* delay entering character while waiting for prompt reply
   4 *Must* allow character to "Accept" or "Deny" approach
   5 *Must* automatically refuse approach if there is no reply within the timeout
   6 *Must* give reasonably meaningful messages on denial
   7 *Should* allow "Return" with an identical or similar verb (i.e., returning a kiss with a kiss).
   8 *May* allow "Return", which is an acceptance with a different message -- not identical verbs (i.e., returning a kiss with a slap)
   9 *Should* allow "Counter", which is a denial with a different message or verb
   10 *May* allow people to force entry into consensual space

If someone tries to enter your adjacent, intimate space, they are delayed. You get a message and are prompted with several options. You give a specific reply, and the approacher is either accepted or refused. There may be a number of options to acceptance and refuse with varying levels of vehemence.

In later games, people may be able to enter consensual space by force (ie, a combat system).

### Profile Requirements:

   1 *Must* allow a player to automatically respond with GLOBALCR
   2 *Should* allow a player to automatically respond to attempts to enter intimate space on a character-by-character basis
   3 *May* allow a player to automatically respond to intimate actions on a character-by-characterion *and* action-by-action basis
   4 *May* allow a player to automatically avoid certain characters entering adjacent space

Players should be able to automate responses to certain characters in their personal profiles: ie, always leave when Jolette enters my adjacent prox; or, always "Accept" Jeanie's intimate approach; or, always "Deny" Jullianna's intimate approach; or, always "Counter" Julie's intimate approach. In a far distant program, this may be done on an action-by-action basis (ie, always "Accept" Jeanie's hugs, but always "Deny" Jeanie's kisses; or, always "Accept" Jeanie's hugs, but don't define a default for her kisses).


----

## References:

   * Other systems that impacts Consent:  [ProximitySystem](./ProximitySystem.md)

   * Or systems Consent impacts:


Articles: [An Evening at Chez Skoot – Proxes Part 1: Consent](https://www.skotos.net/articles/chezskoot.html)

-----

## notes

-----

From an email to the S7 list at http://listserv.skotos.net/mailman/private/skotos-seven/2002-August/001291.html

Hey Kalle,

> I just made a crossbow, and I'd like to add to it that it actually
> checks for consent before smacking a quarrel in someone's head.
>
> Is there or is it possible to add a command in merry to force a consent
> query?
>
> Something like:
>
>  if( !AskConsent( NRefOb( $target )))
>    EmitTo( $actor, "He won't let you." );
>  else
>    EmitTo( $actor, "He lets you! Wee!" );

That would require genuine multi-threading, and LPC can't do it. That
said, the concent mechanism is accessible from Merry, it just requires a
callback approach. Most trivially this is demonstrated on the command
line with:

```
<verbatim>
+tool merry exec Act($actor, "demand_consent", $target: $actor,
$message: "la la la", $action: "eat", $args: ([ "what": $actor ]));
</verbatim>
```
In other words, there is an action "demand_consent" which takes arguments:

```
<verbatim>
  target: [object that is asked for consent],
  message: [description of the attempted action]
  action: [the action that consent YES should trigger]
  args: [the arguments to send to that action]
</verbatim>
```

A common thing to do is to have an action called, uh, 'shoot', say,
which goes something like this:

```
<verbatim>
if ($consented) {
   /* this code is not executed the first time around */
   EmitTo($killob, "You've been shot! You die!\n");
   /* do something to kill the target */
   return TRUE;
}
/* consent not yet given, demand it */
Act($actor, "demand_consent",
     $target: $killob,
     $message: "shoot",
     $action: "shoot", /* note that shoot is the name of THIS action */
     $args: args + ([ "consented": TRUE ]));
return TRUE;
</verbatim>
```

So. The first time the code is run, it slips down to the action that
demands the consent. The arguments there set up a callback to the same
'shoot' action, with the same arguments except with the addition of
$consented being TRUE. So when the $killob accepts the shooting, the
shoot action is called back, $consent is TRUE, and the top-most part of
the code is executed.

Zell
