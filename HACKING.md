## Directory Structure

SkotOS uses a DGD root directory called "skoot." Everything outside that is invisible inside running DGD code - DGD's own config file, a "cores" directory (for core dumps?), and documentation and tmp directories.

Inside skoot, there are directories:

* base
* core - seems to be mostly about derived propertiesda
* data - in kernellib "data" means DGD lightweight objects - these are maps, iterators, etc.
* data/modules - a module dependency and library system's dependency data
* data/vault - these are XML objects for lots of Skotos in-game objects
* doc - docs, probably accessible to in-game help system
* include - headers defining constants, especially object names
* kernel - most of the Kernellib code
* lib - high-level libraries, including data structures, interfaces and note functionality and a lightweight PNG (image) LPC implementation
* obj - a few data-structure objects
* usr - all the user directories - a huge variety of work

### Skoot User Directories

User directories contain the vast majority of the interesting game code as well as a number of underlying features and functionality.

There's also a lot of random data in a lot of little hidden corners (and that's just in the Git repo.)

User directories include:

* CSD - the Combat, Skills and Death system
* DTD - XML Data Type Definition system
* DevSys - the Tree of Woe, and object import/export
* Game - a *very* simple Game abstract base class (used or no?)
* Generic - this looks like object vault management, see also SID; has lists of object folders to include or exclude
* HTTP - the HTTP port handler
* Jonkichi - looks like a few random handlers, mostly related to Merry; currently structural and clearly cannot be removed (yet)
* SAM - SAM is an older-style (outdated?) dynamic text generator; seems very much still in use, though
* SID - SAM document handling of some kind (SAM serialisation?)
* SMTP - manage email handling and connecting to an SMTP server
* SkootOnInn - in-game objects and web data for a long-ago prototype game
* SkotOS - in-game systems and data for, inclusively, NPCs, Merry, stories, Bilbo, socials...
* Socials - a little library code related to the Socials system
* System - the "boot-up" objects, various system management and monitoring at runtime, kernellib hooks like wiztool and progdb, module loading, logging, game configuration, outbound API calls...
* TextIF - handles a lot of text commands for a text world, including parsing, text generation and simple communications
* Theatre - for choosing/switching among multiple different games and chatting while doing so
* Tool - various staff commands, mostly related to Sam, Bilbo, Merry; also (another copy of?) editor.c
* UserAPI
* UserDB - user management, plus operations like "broadcast" that operate that the user/account level
* XML - XML parsing and generation
* admin - empty, at least in the SkotOS repo
* nino - some Merry dice handlers that don't look like they're used anywhere
* zell - Small scripts that look in-progress or one-off


## Vocabulary, Local Colour and Small Oddities

### Vault

A repository of objects. It looks as if there are designed to be multiple:

    Noahs-MBP-2:skoot noah$ find . -name vault
    ./usr/DevSys/data/vault
    ./usr/SkotOS/data/vault
    ./usr/TextIF/data/vault
    ./usr/SkootOnInn/data/vault
    ./usr/DTD/data/vault
    ./usr/Generic/data/vault
    ./usr/Socials/data/vault
    ./usr/SID/data/vault
    ./data/vault
    ./base/data/vault


### advcat

An advcat is an "adverb category," used with and related to socials/evokes.

### Alice

Alice was the original Flash client for SkotOS games, and the origin of some features.

### Merry

The most current way to dynamically generate things like object descriptions, as well as create new rules and (sometimes) game systems.

Merry includes an underlying dialect (subset) of C/LPC.

Merry also includes properties and signals, which are used with handlers in that dialect.

### Object Containers (Merry)

These show up in the Tree of Woe containing shared scripts, but have no context (including physical existence) in the game world. You can attach properties to them, but not directly see or interact with them.

Not being in the object environment, they receive no signals.

### SAM, samtags

SAM is a tag-based dynamic string generator, used for HTML and (formerly?) in-game description. Samtags take arguments and generate text, and can be invoked from Sam.

### Bilbo

Bilbo was an older dynamic scripting library. It's still in place, but I can't tell if it's still in use.

### "womble"

Wombling looks like it was a (one-off?) process to replace older-style raw data with DGD lightweight objects throughout a number of structures. But it's still in place. Is that an oversight, or a structural requirement? Is anything still producing old-style structures that need converting?

### TLS, tlvar

Thread-local storage variables. They're set in the call trace.

### claim_node

What looks like a never-implemented system to reserve certain parts of the vault node namespace. The current implementation of claiming a node is a no-op.

### udat

A udat is a per-game user data object. For instance a single user (account) might have three bodies, each corresponding to a udat.
