# SkotOS software stack

For a full comprehension of the skotos software stack, please also refer to dgd/layers.md as well as the lpc-doc package in http://github.com/dworkin/lpc-doc

## DGD

Nothing in skotoslib has access to anything below DGD (such as the os, network interfaces, files outside the LPC root), and all access to OS resources by LPC code must be done through DGD's kfun api.

## Kernel Library

The kernel library sits between DGD and the rest of the software.  Originally written by dworkin it has since been modified by skotos engineering staff.

It most importantly provides the driver and auto objects mentioned in the DGD documentation.

The klib provides basic security for objects and developer users, as well as the modules containing and owning the objects

### Security

Most of the security provided by the klib is enforced by functions in the auto object masking kfuns in dgd, such as anything to do with files or objects.

One key part of an object is its creator, which is the owner of the file the object was compiled from.  Creator determines many privileges.

The System creator has unique privileges from the klib, including being able to inherit libraries from the kernel library (apart from the auto object of course), call certain functions in the klib, and override certain privilege checks that apply to other creators.

Please refer to the kernel library's documentation for further information.

Creators and owners can be real people, or game modules.

The admin user is given the ability to back-door using the first binary port listed in the config file, provided their password in the klib shell (located in /kernel/data) is enabled.

### Ownership

All objects in the klib are owned.  Master objects are owned by their creators, while clones are owned by the object that cloned them.

The klib forbids kernel objects from being cloned or destructed except by other kernel objects, and forbids objects from destructing other objects that do not have the same owner, unless the destructing object has the System creator.

### Resources

Many things are optionally limited.  Ticks, stack space, callouts, objects, and other things can be given a quota which is enforced on a creator-wise basis by the klib.

## LPC

On top of the klib, SkotOS has its own layer of its LPC code.  This handles such things as basic authentication, bulk, rooms, exits, details, and so on, including the parser.  There are some staff commands implemented directly here.

Different kinds of objects can be cloned for the game, including but not limited to things, verbs, property containers, help nodes, tips, storynuggets, and probably others.  The "type" of an object cannot be changed once cloned.  However, SkotOS's famous data inheritance means that some things can be surprisingly configurable at runtime.

Importantly, the LPC layer also handles compilation and execution of merry scripts.

This also provides the devuser shell that was probably used by the skotos engineers, and most of the code in this layer was probably written by them.

## Merry/Woe

This is the layer most familiar to skotos staff.
