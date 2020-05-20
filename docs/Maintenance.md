# SkotOS Maintenance

This document describes the normal care and maintenance of a SkotOS instance

## Standard Files

We recommend the following standard setup for SkotOS files. The directories are options, the contents of `/var/skotos/XX00` are standardized in the SkotOS release.

   * /var/skotos: Directory for general SkotOS files
      * /var/skotos/XX00: Directory for an individual game that runs on base port XX00
         * /var/skotos/XX00/driver.log: File containing current error logs
         * /var/skotos/XX00/skoot: Directory containing main files for the game
            * /var/skotos/XX00/skoot/log: Directory containing all older logs
         * /var/skotos/XX00/skotos.database: File containing current game database
         * /var/skotos/XX00/skotos.database.old: File containing most recent backup of game database         
         * /var/skotos/XX00/skotos.dgd: File for configuration
     * /var/skotos/backups: Directory containing older backups of game database
     * /var/skotos/support: Directory containing miscellaneous files for Skotos
     
     
  * /usr/local/websocket-to-tcp-tunnel: Directory containing [tunnel](https://github.com/skotostech/websocket-to-tcp-tunnel) used by Orchil client
  
  
  * /var/www/html/[gamename] or /var/www/html/www: Directory containing WWW files related to game (e.g., your front page)
  * /var/www/html/client: Directory containing [Orchil Client](https://github.com/skotostech/orchil)
  * /var/www/html/images: Directory containing images used by clients
  * /var/www/html/user: Directory containing [thin auth server](https://github.com/skotostech/thin-auth)
     
### Standard Crons

We recommend the following standard cronjobs, which use the above directory structure:
```
@reboot /var/skotos/runinstance.sh XX00 &
```
Start your game on reboot. This will result in a process like `/var/skotos/XX00/driver.run /var/skotos/XX00/skotos.dgd /var/skotos/XX00/skotos.database`. This will also fire up a piece of glue that attaches the game's ports to the UserDB's ports: ` /usr/bin/perl /var/skotos/support/misc/userdb-authctl XX00`. 
```
* * * * * /var/www/html/user/admin/restartuserdb.sh
```
This runs the two UserDB processes, `/usr/bin/php server-auth.php` and `/usr/bin/php server-control.php`. They depend on the `userdb-authctl` to talk with the game instance and together answer authentication and control requests to the UserDB.
```
@reboot /var/skotos/support/bin/flashpolicyd.pl --port=843 --file=/var/skotos/support/bin/crossdomain.xml 2>&1 < /dev/null > /dev/null &
23 4 * * * /var/skotos/support/bin/restartflash.sh
```
These processes keep the Flash client running. This will likely be irrelevent after 12/31/20, when Flash is deprecated.
```
@reboot /usr/local/websocket-to-tcp-tunnel/start-tunnel.sh
* * * * * /usr/local/websocket-to-tcp-tunnel/search-tunnel.sh
```
These are a vital component in the Orchil client, which flows through the HTML files in `/var/www/html/client` to `nginx` and from there to this tunnel, then the game. See the [Orchil repo](https://github.com/skotostech/orchil) for the full explanation.

```
* * * * * /var/skotos/support/misc/smtp-queue 8000
```
This keeps mail flowing from the game.

### Standard Backups

You should also have functions that regularly backup and store your database files, as you see fit. At Skotos, we kept the `old` database on hand and daily copied out an additional version to the `/var/skotos/backups` directory, where we maintained the most recent four backups. This was done with a program called `/var/skotos/support/misc/backup-dumps`, which also ran out of the cron:
```
40 9,17 * * * /var/skotos/support/misc/backup-dumps 8000
```
We also kept complete machine backups from one-day old, one-week old, and two-weeks old, and of course each of those had six copies of the database, resulting in somewhere around 20 backups available at any time (plus occasional snapshots)l.

## Starting SkotOS

## Killing SkotOS

## Retrieving Backups
