# "SkotOS" Skotos Open Source Game Library

This is a pre-release version of the Skotos Open Source game library, aka "SkotOS", written for use with [Dworkin's Game Driver](https://github.com/dworkin/dgd), aka "DGD". It a fourth-generation text MUD engine that has been in devopment and active use for almost two decades by [Skotos Tech](http://www.skotos.net/) in a number of commercial web-centric prose online games, including [Castle Marrach](http://www.skotos.net/games/marrach/), [Ironclaw Online](http://www.skotos.net/games/ironclaw/), [Lovecraft Country](http://www.lovecraftcountry.com/comic/), and [The Lazurus Project](http://www.lazarus-project.net/). 

## Status

This version is very pre-release and unsupported — installation is difficult, documentation is minimal, and a new web client needs to be built. The main goal of this initial release was to remove proprietary game content and and personal information from the commercial version used in games offered by Skotos Tech.

We are seeking volunteers to take this code to the 1.0 level suitable for anyone to quickly install the game and create their own web-centric prose games.

## Copyright & Open Source Licenses

The library (SkotOS) and the shared game code and objects in this repository or copyright © 1999-2018 by [Skotos Tech, Inc.](https://www.skotos.net) 

The library itself (all the portions written in LPC) and library documentation are licensed under the [GNU Affero General Public License 3.0](https://www.gnu.org/licenses/agpl-3.0.en.html). All other documentation, the shared game code and common objects (written in a combination of Merry code & XML data) are licensed under the [Creative Commons Attribution Share-Alike 3.0](https://creativecommons.org/licenses/by-sa/3.0/deed.en) license. There is no requirement to release your own game code and objects under any open source license, but any derivative works of the library (AGPL) or contributions to the shared game code and objects (CC-SA) must be released under the same license. All uses of this code must prominently give attribution to "Skotos Tech, Inc" and link to https://www.skotos.net .

## Contributions

We welcome contributions back to this library and to shared game code and objects, but we do require a Contributors License to be signed by any contributor. The contributors license is still a work-in-progress, contact Shannon Appelcline <ShannonA@skotos.net> for more information.

## Docs

Besides the following docs, on installation and setup, you can find additional information in this repo:

* [Maintenance](docs/Maintenance.md). Regular tasks for a SkotOS game
* [Authentication](docs/Authentication.md). Levels of Authentication & how SkotOS' cookies work

----

The following text is very preliminary and a work-in-progress.

----

## Preparing Your VPS

For best reliability and stability, set your SkotOS instance up on a virtual private server (VPS). 

Linode has proven to be a very stable locale for SkotOS games with high uptime, but any VPS should do the job. The following referrals codes can be used if you want to earn a month of free time:

> Linode Referral Code: https://www.linode.com/?r=3c7fa15a78407c9a3d4aefb027539db2557b3765
> Digital Ocean: http://www.digitalocean.com/?refcode=a6060686b88a

### Create Your Machine

SkotOS games have relatively low requirements and can be run on low-end VPSes. At Linode, the Linode 2048 is suggested, which has 30 GB of storage and 1 CPU core — though you could probably get started on the Linode 1024, which has just 20 GB of storage. 

If you've got a decent, modern CPU, then the storage will be the biggest requirement. It will slowly grow with time and usage, but most modern VPSes will let you easily upgrade the time comes.

All of Skotos' SkotOS games run on either Debian 7.5 or Debian 8.0. Debian is generally suggested as a known stable platform, though other versions of Linux will probably work just as well.

## Installing SkotOS 1.0

SkotOS can be installed from this Github repository. Clone it, copy it, or download a ZIP as you see fit.

### Install the Files

You'll want to install the files in their directory, such as /var/skotoslib. This document will use `${SKOTOSLIB}` to refer to the SkotOS directory.

### Edit the Config Files

There are two major config files: `${SKOTOSLIB}/skotos.dgd` contains a variety of information on variable sizes and ports, while `${SKOTOSLIB}/skoot/user/System/data/instance` contains connectivity data.

You will want to:

1. Change the `directory` in `skotos.dgd`. Note that it should be set to `${SKOTOSLIB}/skoot`, which is to say the `skoot` directory in your `${SKOTOSLIB}`, not the main directory itself.
2. Change the XXX.XXX.XXX.XXX instances at the top of `skotos.dgd` to your IP address.
3. Change the `hostname` in `instance`.

### Optionally Change the Ports

By default, SkotOS is set to respond to ports 80 and 443, but it actually listens on a variety of ports in the 10,000 range. If you want to change this you can do so by editing both `skotos.dgd` and`instance`. Be sure to change the `portbase` in `instance` to another number in the form `X000`, then change all of the ports in both files from `10YYY` to `XYYY`. For example, if you choose your `portbase` to be 8000, your `telnet_port` would be 8098 and your first `binary_port` would be 8099.

You probably don't want to adjust the ports unless you plan to run multiple instances of SkotOS on the same machine. But, if you do want to, this is simple: just be sure that each SkotOS game is (1) installed in a different directory; (2) has a different hostname; (3) has a different IP address; and (4) listens to a different range of ports. 

## Adjusting Your Network

You will also need to make a few changes to your local network setup to accomodate the SkotOS instance.

### Open Your Firewall

Be sure to adjust your Firewall so that it accepts connections on all the machines. The following iptables rules should make everything accessible:

```
-A INPUT -p tcp --dport    80 -j ACCEPT
-A INPUT -p tcp --dport   443 -j ACCEPT
-A INPUT -p tcp --dport   843 -j ACCEPT
-A INPUT -p tcp --dport  10080 -j ACCEPT
-A INPUT -p tcp --dport  10090 -j ACCEPT
-A INPUT -p udp --dport  10023 -j ACCEPT
```

You may or may not decide to keep these open:
```
-A INPUT -p tcp --dport  10098 -j ACCEPT
-A INPUT -p tcp --dport  10099 -j ACCEPT
```
These are administrative/wiztool ports, and we suggest restricting them to access only on the local machine, in which case you should not put them in.

Be sure to rerun your firewall rules afterward, usually with a command like the following:

```
$ /sbin/iptables-restore < /etc/your-firewall-rules
```

## Starting SkotOS

At this point SkotOS is easy to start.

### Build the Driver

SkotOS needs a modified build of DGD. It uses more sectors (memory and disk space) than unmodified DGD will accept. DGD has compile options to fix this, but you'll need to use them. Similarly, DGD only allows outgoing network connections if they have been requested during compilation.

TODO: describe how to create a conformant DGD build.

### Run The Driver

To start things going, run:

```
$ ${SKOTOSLIB}/driver.run ${SKOTOSLIB}/skotos.dgd
```

The first time that SkotOS runs you should see lots of creative work. A new file will appear in your home directory, `skotos.database`. This is your database in the file system, which will let you restore your game if the machine crashes.

When the initial cold boot is done, you should see something like this:

```
Apr 27 15:10:57 ** info:Boot completed.
```

### Restart the Driver

You should probably kill your driver now. You'll see messages as it cleanly shuts down and does a statedump. You can now run SkotOS with a standard invocation that also references the new statedump file.

```
${SKOTOSLIB}/driver.run ${SKOTOSLIB}/skotos.dgd ${SKOTOSLIB}/skotos.database
```

You'll know you restore from the backup if you see this:

```
Apr 27 16:25:23 ** info:Reboot completed.
```

### Cron the Driver

You will probably want to set your game to restart at machine start-up by putting something like this in your crontab:

```
@reboot ${SKOTOSLIB}/driver.run ${SKOTOSLIB}/skotos.dgd ${SKOTOSLIB}/skotos.database
```

You'll know you restore from the backup if you see this:

```
Apr 27 16:25:23 ** info:Reboot completed.
```

### Back Up Your SkotOS

## Running SkotOS
