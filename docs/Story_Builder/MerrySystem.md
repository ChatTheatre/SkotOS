# Merry System — A Technical Summary

_**MERIADOC** - Mediated Execution Registry Involving A Dialect Of C, a.k.a "Merry"_

# Introduction

### What is Merry?

Merry is not a language in itself, but rather LPC scripts made available for use inside the text-based virtual environment. LPC is the native language in which the SkotOS library is written, and is an object-oriented, interpreted dialect of C. If you already have some experience with C, Merry is going to look familiar to you.

Merry scripts run in a "sandboxed" execution environment. This means some of LPC's most powerful and general abilities have been deliberately limited and re-formulated, so as to limit the accidental damage a developer could do with an erroneous piece of code. 

Merry scripts themselves are added as specially named properties stored inside data objects in the ChatTheatre. They are initiated soley by [signals](./SignalSystem) sent to those objects after various events.

For instance, the drop signal is sent to an object when it is dropped, and a merry script can be written to react to when that happens.

```
merry:react:drop
    set(this, "details:default:luminosity", 0);
```

### Problems Solved by Merry System

#### The Merry Sandbox

The name Merry comes from MERIADOC "Mediated Execution Registry Involving A Dialect Of C" -- which although originally was an inside joke as it is the successor to an earlier, now deprecated language named Bilbo "Built In Language Between Objects", but the acronym that Merry stands for actually does describe it well.

The key is the the "mediated" part -- this means that Merry is somewhat sandboxed and thus "safer" to use -- not to the scale that Java or more modern "safe" languages offer, but helpful. 

For instance, in Merry scripts:

* Execution is initated only through [signals](./SignalSystem) sent to virtual world objects rather than through some form of main() loop or execution queue. 
* We ensure that Merry scripts inside these virtual objects can only interact with objects that involve the virtual game world, not random other objects it has no business with.
* Only a limited number of functions can be called from Merry, and even then they are only allowed implicitly, like fetching/setting properties, Act(), etc.
* There are run-time limits on how long a Merry script can execute or how much memory it can use before being terminated.
* File system access, most other I/O communications functions, and things like starting up or shutting down processes are completely disabled.

#### Text Environment Syntax

In addition to the sandbox, the langage is run through a complex parse_string() grammar, which gives us the ability to add some useful synatactic sugar additions: constants, per-thread global `$variables`, `$foo:` function-parameters, and you can directly access the [property]](./PropertySystem) database for values through `{someobject}.property:name`.

Independent of the Merry language itself, there are a large variety of useful ChatTheatre specific functions availabe to Merry scripts, such as EmitTo(), EmitIn(), Set(), etc, as well a few other useful things like the ability to do inline SAM (Skotos Active Markup) with `$"{oneof:one|two|three}".`

#### Leverage the PropertySystem and SignalSystem

Merry is designed to be fully integrated with the [PropertySystem](./PropertySystem) and the [SignalSystem](./SignalSystem) of the SkotOS text-based virtual environment.

As Merry scripts are stored as properties inside SkotOS data objects, rather than as files, that means that they can take advantage of the data inheritance model used by SkotOS. This also makes it easy for new StoryCoders to create a child of an existing scripted object and add only the behaviors they wish to change.

Script execution is soley initiated as reactions to in-game verbs or by [signals](./SignalSystem), making it very easy to for new StoryCoders to write small scripts that are immediately useful, while also make it easier for large projects to be easily refactored and managable by advanced coders.

## QuickSummary of the Merry System

TBW

## State of Development

### Open Issues in theMerry System

### Technical Requirements

### Other Approaches

### References

### History

#### How does Merry compare to Bilbo?

BILDO "Built In Language Between Object" was the first scripting language used for SkotOS-based games. Bilbo grew organically from extremely humble beginnings. It was designed to be simple and intuitive and to let developers add complex abilities to their objects with a minimum of effort.

It soon became clear that developers hungered for more complicated systems, where Bilbo's simplicity became a liability rather than an asset. Bilbo became extremely frustrating to write and almost unreadable for any project that exceeds even a fairly modest level of complexity.

Merry, in contrast, is essentially LPC, which has the expressive power of a full programming language. Merry's syntax is immediately cryptic, where Bilbo starts out looking almost like English; however, Merry scales easily to complex structures, where Bilbo falls apart very quickly.

Both Bilbo are property & signal-based: 

```
bilbo:react:drop > run
bilbo:react:drop:0010 > set this.details:default:luminosity to 0
```

the Merry programmer would have

```
merry:react:drop > Set(this, "details:default:luminosity", 0);
```

Apart from scaling well to complex projects, Merry is also much faster than Bilbo. Furthermore, LPC is compiled from source-code to an internal binary formats, and many errors are caught in the compilation. As irritating as it can be to have to spend half an hour debugging your code before you can even commit it successfully, you can then take joy in the knowledge it's much more likely to run right than Bilbo, where errors are never caught before execution.


