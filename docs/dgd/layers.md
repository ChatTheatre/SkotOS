# Layers

DGD operates in several layers

## Below DGD

These layers are below DGD itself, but some things must be provided for support

### Machine

This is the machine, bare metal or VM, that is running the server.

Should provide enough memory so DGD can run without being pushed into OS level swap

### Kernel

Probably linux

### OS

This is probably the linux distro you installed on the machine.

Requirements for compiling DGD

* C++ compiler and support libraries (probably gcc with g++ enabled as well as libstdc++)
* yacc (probably bison)
* make

## DGD

Once dgd itself is compiled and started, you can see things from DGD's view

### Configuration

The config file specified on the command line when dgd is invoked specifies some important parameters:

* objects, call\_outs, swap\_size, array\_size

Running out of objects or callouts, or making an array (or mapping) too big will cause a runtime error

Running out of swap will cause a fatal error, crash, and a core dump

* directory

This must be absolute, and points to what will become the root directory as seen by LPC code.  Barring shenanigans like bind mounts or symlinks it is impossible for LPC to leave this directory.

### Filesystem

Files that can be accessed by LPC are kept under the root directory specified before.

The driver and auto object's .c files MUST exist when dgd is first cold booted

### Objects

Every task/event that DGD injects into the LPC environment is started by a call to an object

Objects are created by:

* compilation, which creates a master object
* cloning a master object

The driver and auto objects are automatically compiled by DGD on cold boot.

Objects can inherit other objects during compilation.

All objects implicitly inherit the auto object that is in service when the object is compiled.

Henceforth, the word "program" is used to refer to a particular master object.  This may refer to a given object's master, or one of its inherits any distance up the inheritance tree.  An inherited program functions the same way in LPC as a base class does in C++.

### Functions

There are three kinds of functions in DGD

* kfuns
* afuns
* normal funs

A kfun is a function provided by DGD itself.

An afun is a function defined by the auto object, and can mask a kfun.  Such masking is often done for security reasons and the kernel library's auto object provides a good demonstration of this concept.

Any object, clone or master, may have LPC functions defined by the source that was used to compile it.  Except for the initial cold boot for the driver and auto objects, any object can be compiled from literal source supplied to the compile_object kfun, but by default reads the source from the corresponding LPC source file.

DGD itself may call functions, called applies, which work like hooks.

Full documentation for DGD's applies can be found in the lpc-doc package.

Functions can have attributes:

* private - a private function can only be called inside the program it is defined in, is invisible to inheriting programs and external objects, and cannot be masked by an inheriting program
* static - a static function can only be called inside the object it is defined in, can be called by inheriting programs, and can be masked by an inheriting program.  it is invisible to external objects
* nomask - a nomask function cannot be masked and an attempt to do so causes a compile error
* atomic - an atomic function is guaranteed to have no side effects if it is aborted due to an error.  There are some things that are therefore not allowed inside an atomic function call

Technically, all calls to external objects from LPC are done by invoking the call\_other kfun.  Overriding this in the auto object will cause performance penalties, but may be useful.  By default, attempting to call a nonexistent (and possibly invisible due to being static or private) function in any object returns nil.  Kotaka overrides call\_other to raise a runtime error if an attempt is made to call a non-existent function.

A callout is a delayed self-call, that must be directed at a static function in the object by invoking the call\_out kfun.

### Exception handling

Errors can happen either by explicit request with the error kfun, or from DGD itself if things go wrong

Errors cause the LPC call stack to unwind until the error is either caught or escapes the top call on the stack, at which point the task/event is terminated.

An error that escapes an atomic function call will cause that call to be rolled back.  Atomic function calls can be nested if desired and each atomic layer will be rolled back separately as the error goes up the stack.

Attempting to catch an "out of ticks" error will fail if the context where it is caught also is out of ticks.

### Rlimits

It is possible to impose runtime limits on stack or tick usage by means of the rlimits construct.

rlimits constructs are evaluated by the driver object both when the program using them is compiled, and if necessary, also during runtime.  A denial by the driver on an rlimits attempt will cause a runtime error.

Exhausting the stack limit will cause a runtime error.

### Fatal errors

Some conditions are fatal in DGD

* running out of memory, detected when malloc fails
* running out of stack, and thusly causing the OS to raise a segmentation fault
* running out of sectors in the swap file
* DGD attempting to call a function that does not exist in the driver object

All fatal errors cause dgd to abort and probably dump core, and cannot be caught by LPC exception handlers