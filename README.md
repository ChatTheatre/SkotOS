# SkotOS Open Source Game Library

[MUDs](https://en.wikipedia.org/wiki/MUD) were some of the earliest multiplayer games on the Internet, and in some cases before the Internet. Players would read text descriptions and type text commands to control their characters.

[Skotos Tech](https://www.skotos.net/), from 1999 to 2020, was a company that made MUDs and MUD-like games "that were more social, more dynamic, more interactive, more realistic, and more story-focused than anything that had been seen before." They were a critical success but never a commercial powerhouse. MUDs never made a huge amount of money, even the successful ones.

But the massively-multiplayer text interactive fiction world's loss is your gain &mdash; when SkotOS's business failed, they left behind their twenty-year game engine as open source, under the name [ChatTheatre](https://chattheatre.com).

This is that game engine.

Some SkotOS-based games are still around - [Castle Marrach](https://www.marrach.com/), [The Eternal City](https://www.eternalcitygame.com/), [https://allegoryofempires.com/](https://allegoryofempires.com/) and [Multiverse; Revelations](https://home.multirev.net/) as I write this sentence. Many others have ceased operation, such as [Grendel's Revenge](https://www.skotos.net/about/pr/06122002.html), [Lovecraft](http://www.lovecraftcountry.com/comic/) [Country](https://www.skotos.net/about/pr/06292005.html), [Ironclaw Online,](https://www.skotos.net/about/pr/Nov28_2000.html) and [The Lazarus](https://www.lazarus-project.net/) [Project](https://www.skotos.net/about/pr/01032007a.html). And that's not counting the many and [various small stages](https://www.skotos.net/articles/TTnT_47.shtml.html) that were never meant to be full-sized games.

Twenty years of good ideas and bad, authors and programmers, interesting failures and enjoyable successes have been built on this code and its (many) earlier versions. You can find fragments of those attempts [in various ChatTheatre repositories](https://github.com/ChatTheatre) on GitHub. There are others, hidden away in dark corners that may be added as we locate them.

Technically speaking, SkotOS was built atop a [weird and ancient interpreted language called DGD](http://www.dworkin.nl/dgd/) which combines MUD-specific origins with [features so advanced no mainstream language has them yet](https://noahgibbs.github.io/self_conscious_dgd/15_Atomic.html) and a [paid premium version](http://www.dworkin.nl/hydra/) that presents multiple cores as a much faster single core in a way that is generally considered impossible. You can [download the binaries for free right now](http://www.dworkin.nl/hydra/) and try it for yourself if you'd like to fact-check that claim. The free version of DGD is plenty fast for SkotOS, though. Twenty years of [Moore's Law](https://en.wikipedia.org/wiki/Moore%27s_law) has done wonders.

SkotOS's core runs on DGD. But it's encircled in a layer of shellscripts, Node.js, web servers and so forth in order to connect to the modern world. You ***can*** still connect with [telnet](https://en.wikipedia.org/wiki/Telnet), but we assume you'd rather not.

SkotOS is a fourth-generation text MUD engine with a huge number of convenient features and bizarre subsystems. It's normally used to put together your own [text-based browser games](https://github.com/ChatTheatre/gables_game) or [virtual interactive spaces](https://github.com/WebOfTrustInfo/prototype_vRWOT).

You can find [extensive documentation of it in GitHub Pages](https://ChatTheatre.github.io/SkotOS-Doc).

## Status

The SkotOS codebase has transitioned from "unreleased, unsupported and nearly unusable" to "usable by the brave and determined." As the description above suggests, this is not a smooth and easy experience. You'll have an easier time if your team includes at leasts one skilled developer and at least one skilled writer willing to use annoying interfaces. Those two can be the same person if they have a lot of free time.

SkotOS does ***not*** require great sophistication in modern web technologies. There's just enough of that to wrap an old-style monolithic application server and attach it to the current internet.

We'd be happy to accept any code changes you feel like contributing back. In the mean time we'll keep rehabilitating, exploring and documenting, little by little. You can also help out by putting together your own game that uses SkotOS even if you're not really messing with the core of SkotOS. Documentation is good too. We can always use more of that.

## Dev Installation

Are you on a Mac? If you run dev_scripts/mac_setup/mac_setup.sh, the script will check out the other repos you need such as DGD itself, the websockets tunnel and the web client. It will even patch DGD for you before building it. All those repos will wind up under your checked-out SkotOS Git repo.

We don't currently support a Linux or Windows local dev install. We ***do*** support a Linux production deployment using Linode Stackscripts as an example. You can easily reuse those shell scripts to install on your VM provider of choice, or on physical hardware.

There's also a Docker installation, used for a variety of purposes like CI.

## Docs

See [SkotOS-Doc](https://ChatTheatre.github.io/SkotOS-Doc).

## Copyright & Open Source Licenses

The library (SkotOS) and the shared game code and objects in this repository or copyright © 1999-2018 by [Skotos Tech, Inc.](https://www.skotos.net).

The library itself (all the portions written in LPC) and library documentation are licensed under the [GNU Affero General Public License 3.0](https://www.gnu.org/licenses/agpl-3.0.en.html). All other documentation, the shared game code and common objects (written in a combination of Merry code & XML data) are licensed under the [Creative Commons Attribution Share-Alike 3.0](https://creativecommons.org/licenses/by-sa/3.0/deed.en) license. There is no requirement to release your own game code and objects under any open source license, but any derivative works of the library (AGPL) or contributions to the shared game code and objects (CC-SA) must be released under the same license. All uses of this code must prominently give attribution to "Skotos Tech, Inc" and link to https://www.skotos.net .

## Contributions

We welcome contributions back to this library and to shared game code and objects, but we do require a Contributors License to be signed by any contributor. The contributors license is still a work-in-progress, contact Shannon Appelcline <ShannonA@skotos.net> for more information.
