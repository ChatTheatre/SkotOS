# SkotOS Open Source Game Library

Remember [MUDs](https://en.wikipedia.org/wiki/MUD)? Early massively-multiplayer network games, sometimes pre-Internet? Text-input, text-output?

[Skotos Tech](https://www.skotos.net/), from 1999 to 2020, made MUDs "that were more social, more dynamic, more interactive, more realistic, and more story-focused than anything that had been seen before." They were a critical success but never a commercial powerhouse. Eventually they stopped...

But the MUD world's loss is your gain. Skotos Tech released their twenty-year game engine as open source.

**SkotOS is that game engine**. [ChatTheatre](https://github.com/ChatTheatre) is the organisation maintaining it.

Twenty years of good ideas and bad, authors and programmers, [interesting failures and enjoyable successes](https://ChatTheatre.github.io/SkotOS-Doc/Games.html) have been built on this code and its (many) earlier versions. You can find fragments of those attempts [in various ChatTheatre repositories](https://github.com/ChatTheatre) on GitHub. There are others, hidden away in dark corners that may be added as we locate them.

You can use SkotOS to build your own games, probably [starting from a simple one](https://github.com/ChatTheatre/gables_game). You should start from the [extensive documentation in GitHub Pages](https://ChatTheatre.github.io/SkotOS-Doc).

SkotOS's technical foundations are built atop a [somewhat-bizarre interpreted language called DGD](http://www.dworkin.nl/dgd/) which combines MUD-specific origins with [features so advanced no mainstream language has them yet](https://noahgibbs.github.io/self_conscious_dgd/15_Atomic.html) and a [paid premium version](http://www.dworkin.nl/hydra/) that presents multiple cores as a much faster single core in a way that is generally considered impossible. You can [download the binaries for free right now](http://www.dworkin.nl/hydra/) and try it for yourself if you'd like to fact-check that claim.

If you just want to play a SkotOS-based game, these are good choices:

* [Castle Marrach](https://www.marrach.com/)
* [The Eternal City](https://www.eternalcitygame.com/)
* [Allegory of Empires](https://allegoryofempires.com/)
* [Multiverse; Revelations](https://home.multirev.net/)

You can also try out a small SkotOS-based stage at [The Gables](https://gables.chattheatre.com).

## Status

The SkotOS codebase has transitioned from "unreleased, unsupported and nearly unusable" to "usable by the brave and determined." As the description above suggests, this is not a smooth and easy experience. You'll have an easier time if your team includes at least one skilled developer and at least one skilled writer willing to use annoying interfaces. Those two can be the same person if they have a lot of free time.

SkotOS does ***not*** require great sophistication in modern web technologies. There's just enough of that to wrap an old-style monolithic application server and attach it to the current internet.

We'd be happy to accept any code changes you feel like contributing back. In the mean time we'll keep rehabilitating, exploring and documenting, little by little. You can also help out by putting together your own game that uses SkotOS even if you're not really messing with the core of SkotOS. Documentation is good too. We can always use more of that.

## Dev Installation

Are you on a Mac? If you run dev_scripts/mac_setup/mac_setup.sh, the script will check out the other repos you need such as DGD itself, the websockets tunnel and the web client. It will even patch DGD for you before building it. All those repos will wind up under your checked-out SkotOS Git repo.

We don't currently support a Linux or Windows local dev install. We ***do*** support a Linux production deployment using Linode Stackscripts as an example. You can easily reuse those shell scripts to install on your VM provider of choice, or on physical hardware.

There's also a Docker installation, used for purposes like CI.

## Docs

See [SkotOS-Doc](https://ChatTheatre.github.io/SkotOS-Doc).

## Copyright & Open Source Licenses

The library (SkotOS) and the shared game code and objects in this repository or copyright © 1999-2018 by [Skotos Tech, Inc.](https://www.skotos.net).

The library itself (all the portions written in LPC) and library documentation are licensed under the [GNU Affero General Public License 3.0](https://www.gnu.org/licenses/agpl-3.0.en.html). All other documentation, the shared game code and common objects (written in a combination of Merry code & XML data) are licensed under the [Creative Commons Attribution Share-Alike 3.0](https://creativecommons.org/licenses/by-sa/3.0/deed.en) license. There is no requirement to release your own game code and objects under any open source license, but any derivative works of the library (AGPL) or contributions to the shared game code and objects (CC-SA) must be released under the same license. All uses of this code must prominently give attribution to "Skotos Tech, Inc" and link to https://www.skotos.net .

## Contributions

We welcome contributions back to this library and to shared game code and objects, but we do require a Contributors License to be signed by any contributor. The contributors license is still a work-in-progress, contact Shannon Appelcline <ShannonA@skotos.net> for more information.
