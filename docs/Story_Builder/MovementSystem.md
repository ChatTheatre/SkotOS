# The Movement System — A Technical Summary

## The virtual world

The most distinct difference between a chat-room (e.g. an IRC channel) and a virtual world is the concept of movement. Restrictions and rules for moving around make the virtual world seem more real.

The movement system enables the story builders to define and customize the events regarding movement, inside rooms as well as between rooms.

## Introduction

The movement system in SkotOS is what makes it possible for characters to move around in rooms (proximity change) as well as between rooms (environment change). More importantly, however, the system makes it possible to intercept the various stages which comprise movement, to abort them ("You are too weak to move."), to change how they appear to the players ("With no small amount of effort, you climb the giant steps leading out of the crypt."), or to add special features to the act of moving ("As you approach the gilded statue, the sound of stone scraping against stone is heard, and you feel the floor-tile at your foot shift and move...").

### Usage

The movement system is comprised of a number of signals (see [SignalSystem](./SignalSystem.md)), which are triggered at appropriate places. The following table lists the various signals used by the movement system, sorted into "proximity" and "environment":

| [Signal]() | [Type]() | [Pre]() | [Prime]() | [Post]() | Description                                                 |
| ---------- | -------- | ------- | --------- | -------- | ----------------------------------------------------------- |
| enter      | env      |         | *         | *        | Catch-all enter signal (both entering and leaving the room) |
| enter-into | env      |         | *         | *        | Leaving the room                                            |
| enter-from | env      |         | *         | *        | Entering the room                                           |
| teleport   | env      |         | *         | *        | Teleporting into a room (but not out of)                    |
| approach   | prox     | *       | *         | *        | Approaching (a detail of) the object                        |

These signals can be handled in a number of ways (just like all other signals). The following is a complete list of the places where the signals can be used:

- Environment types:
  - In a `merry:react:` script, in the room which is entered or left.
  - In a description type `react:`, in the exit detail which is entered or left.
  - In a `witness:` script in an item inside the room.
  - In an `act:`script in the body entering or leaving a room.
- Proximity types:
  - In a `merry:react:` script, in the object being approached.
  - In a description type `react:`, in the object being approached.
  - In a `witness:` script in an item in the room in which another item being approached should be noticed.
  - In an `act:` script in the body approaching an(y) object.

This system is demonstrated in the object `Examples:incomplete:rooms:movement-room`

