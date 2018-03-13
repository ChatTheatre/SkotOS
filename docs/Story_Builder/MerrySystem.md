# Merry System — A Technical Summary

MERIADOC - Mediated Execution Registry Involving A Dialect Of C, a.k.a "Merry"

# Introduction

### What is Merry?

Merry is not a language in itself, but rather the LPC language made available to the story builder. LPC is the native language offered by the DGD driver that the SkotOS library is written in. LPC is an object-oriented, interpreted dialect of C, and Merry in turn is a limited subset of LPC. If you already have some experience with C, Merry is going to look familiar to you.

### Problems Solved by Merry System

Merry executes in a "sandboxed" environment. This means some of LPC's most powerful and general abilities have been deliberately limited and re-formulated, so as to limit the accidental damage a story builder could do with an erroneous piece of code.

The name Merry comes from MERIADOC "Mediated Execution Registry Involving A Dialect Of C" -- which although somewhat a joke because it is the successor to an earlier deprecated language named Bilbo "Built In Language Between Objects", but the acronym that Merry stands for actually does describe it well.

The key is the the "mediated" part -- this means that Merry is somewhat sandboxed -- not to the scale that Java is, but helpful. For instance, in Merry code we try to ensure that Merry scripts inside game objects only interact with objects that involve the virtual game world, not random other objects it has no business with. Only a limited number of functions can be called from Merry, and even then they are only allowed implicitly, like fetching/setting properties, Act(), etc. File system access is completely disabled, as well as things like starting things up or shutting down processes, and some communications functions.

In addition, the langage is run through a complex parse_string() grammar, which gives us the ability to add some useful synatactic sugar additions: constants, per-thread global \$variables, \$foo: function-parameters, you can directly access the property database for values through \${someobject}.property:name, and a few other useful things like the ability to do inline SAM (Skotos Active Markup) with $"{oneof:one|two|three}".

Independent of the Merry language itself, there are a large variety of useful game specific functions, such as EmitTo(), EmitIn(), Set(), etc.

## Summary of the Merry System

## State of Development

### Open Issues in theMerry System

### Technical Requirements

### Other Approaches

### References



