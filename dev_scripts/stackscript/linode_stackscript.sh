#!/bin/bash

# Script to stand up a production-flavoured SkotOS instance on Linode.

# Based loosely on https://github.com/BlockchainCommons/Bitcoin-Standup-Scripts/blob/master/Scripts/LinodeStandUp.sh

# Tested against: Debian 10

# This block defines the variables the user of the script needs to input
# when deploying using this script.
#
# <UDF name="hostname" label="Short Hostname" example="Example: my-awesome-game"/>
# HOSTNAME=
# <UDF name="fqdn" label="Fully Qualified Hostname" example="Example: my-awesome-game.my-domain.com"/>
# FQDN=
# <UDF name="userpassword" label="Deployment User Password" example="Password for the host deployment account." />
# USERPASSWORD=
# <UDF name="ssh_key" label="SSH Key" default="" example="SSH Key for automated logins to host's deployment account." optional="true" />
# SSH_KEY=
# <UDF name="skotosgiturl" label="Skotos Git URL" default="https://github.com/ChatTheatre/SkotOS" example="SkotOS Git URL to clone for your game." optional="false" />
# SKOTOS_GIT_URL=
# <UDF name="dgdgiturl" label="Skotos DGD URL" default="https://github.com/ChatTheatre/dgd" example="SkotOS DGD URL to clone for your game." optional="false" />
# DGD_GIT_URL=



# Some differences from full real SkotOS setup:
#
# 1. No IP address whitelisting for SSH
# 2. No DNS setup with multiple names - one IP address, one domain name, one host
# 3. No hosting multiple games per host

set -e  # Fail on error
set -x

# Force check for root, if you are not logged in as root then the script will not execute
if ! [ "$(id -u)" = 0 ]
then
  echo "$0 - You need to be logged in as root!"
  exit 1
fi

cd /

# Output stdout and stderr to ~root files
exec > >(tee -a /root/standup.log) 2> >(tee -a /root/standup.log /root/standup.err >&2)

####
# 1. Update Hostname
####

echo $HOSTNAME > /etc/hostname
/bin/hostname $HOSTNAME

# Set the variable $IPADDR to the IP address the new Linode receives.
IPADDR=`hostname -I | awk '{print $1}'`

echo "$0 - Set hostname as $FQDN ($IPADDR)"
echo "$0 - TODO: Put $FQDN with IP $IPADDR in your main DNS file."

# Add localhost aliases

echo "127.0.0.1    localhost" > /etc/hosts
echo "127.0.1.1 $FQDN $HOSTNAME" >> /etc/hosts

echo "$0 - Set localhost"

####
# 2. Bring Debian Up To Date
####

echo "$0 - Starting Debian updates; this will take a while!"

# Make sure all packages are up-to-date
apt-get update -y
apt-get upgrade -y
apt-get dist-upgrade -y

# Set system to automatically update
echo "unattended-upgrades unattended-upgrades/enable_auto_updates boolean true" | debconf-set-selections
apt-get -y install unattended-upgrades

echo "$0 - Updated Debian Packages"

# get uncomplicated firewall and deny all incoming connections except SSH
#apt-get install ufw -y
#ufw allow ssh
#ufw enable

####
# 3. Set Up Initial User
####

# Create "skotos" user with optional password and give them sudo capability
/usr/sbin/useradd -m -p `perl -e 'printf("%s\n",crypt($ARGV[0],"password"))' "$USERPASSWORD"` -g sudo -s /bin/bash skotos || echo "Skotos user already exists?"
/usr/sbin/adduser skotos sudo

echo "$0 - Setup skotos with sudo access."

# Setup SSH Key if the user added one as an argument
if [ -n "$SSH_KEY" ]
then
   mkdir ~skotos/.ssh
   echo "$SSH_KEY" >> ~skotos/.ssh/authorized_keys
   chown -R skotos ~skotos/.ssh

   echo "$0 - Added .ssh key to skotos."
fi

####
# 4. Install Dependencies
####

apt-get install git nginx-full cron bison build-essential -y

# Websocket-to-tcp-tunnel requirements
curl -sL https://deb.nodesource.com/setup_9.x | bash -
apt install nodejs npm -y

# Thin-auth requirements
#apt-get install mariadb-server libapache2-mod-php php php-mysql -y

####
# Set up Directories, Groups and Ownership
####

groupadd skotos || echo "Skotos group already exists"

rm -f ~skotos/crontab.txt
touch ~skotos/crontab.txt
chown -R skotos.skotos ~skotos/

####
# Set up SkotOS and DGD
####

# A nod to local development on the Linode
if [ -d "/var/skotos" ]
then
    pushd /var/skotos
    git pull
    popd
else
    #git clone ${SKOTOS_GIT_URL} /var/skotos
    git clone https://github.com/ChatTheatre/SkotOS /var/skotos
    chgrp -R skotos /var/skotos
    chmod -R g+w /var/skotos
fi

# A nod to local development on the Linode
if [ -d "/var/dgd" ]
then
    pushd /var/dgd
    git pull
    popd
else
    #git clone ${DGD_GIT_URL} /var/dgd
    git clone https://github.com/ChatTheatre/dgd /var/dgd
fi

pushd /var/dgd/src
make DEFINES='-DUINDEX_TYPE="unsigned int" -DUINDEX_MAX=UINT_MAX -DEINDEX_TYPE="unsigned short" -DEINDEX_MAX=USHRT_MAX -DSSIZET_TYPE="unsigned int" -DSSIZET_MAX=1048576' install
popd

cat >>~skotos/crontab.txt <<EndOfMessage
@reboot /var/skotos/dev_scripts/stackscript/start_dgd_server.sh &
EndOfMessage

####
# Set up NGinX
####

cat >/etc/nginx/sites-available/skotos_game.conf <<EndOfMessage
# skotos_game.conf

map \$http_upgrade \$connection_upgrade {
    default upgrade;
        '' close;
        }

upstream gables {
    server 127.0.0.1:10801;
}

server {
    listen *:80 default;

    root /var/www/html/client;
    location / {
        try_files \$uri \$uri/index.html \$uri.html =404;
    }
}

server {
    listen *:10800;
    proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
    location /gables {
      proxy_pass http://gables;
      proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
      proxy_http_version 1.1;
      proxy_set_header Upgrade \$http_upgrade;
      proxy_set_header Connection \$connection_upgrade;
    }
}
EndOfMessage

rm -f /etc/nginx/sites-enabled/default
ln -s /etc/nginx/sites-available/skotos_game.conf /etc/nginx/sites-enabled/skotos_game.conf

nginx -t  # Verify everything parses correctly
nginx -s reload

####
# 7. Set up Tunnel
####

mkdir -p /var/log/tunnel
chown -R skotos.skotos /var/log/tunnel

if [ -d /usr/local/websocket-to-tcp-tunnel ]
then
    pushd /usr/local/websocket-to-tcp-tunnel
    git pull
    popd
else
    git clone https://github.com/ChatTheatre/websocket-to-tcp-tunnel /usr/local/websocket-to-tcp-tunnel
fi
chown -R skotos.skotos /usr/local/websocket-to-tcp-tunnel

pushd /usr/local/websocket-to-tcp-tunnel/
npm install
chmod a+x *.sh  # Can remove?
popd

cat >/usr/local/websocket-to-tcp-tunnel/config.json <<EndOfMessage
{
    "pidFileDirectory": "./",
    "logDirectory": "/var/log/tunnel/",
    "websocketHeartbeat": 30,
    "shutdownDelay": 60,
    "servers": [
        {
            "name": "skotosgame",
            "listen": 10801,
            "send": 10443,
            "host": "localhost",
            "sendTunnelInfo": false
        },
        {
            "name": "skotos-tree-of-woe",
            "listen": 10802,
            "send": 10090,
            "host": "localhost",
            "sendTunnelInfo": false
        }
    ]
}
EndOfMessage

sudo -u skotos /usr/local/websocket-to-tcp-tunnel/search-tunnel.sh
cat >>~skotos/crontab.txt <<EndOfMessage
@reboot /usr/local/websocket-to-tcp-tunnel/start-tunnel.sh
* * * * * /usr/local/websocket-to-tcp-tunnel/search-tunnel.sh
EndOfMessage
crontab -u skotos ~skotos/crontab.txt

####
# 8. Set up Orchil
####

mkdir -p /var/www/html
git clone https://github.com/ChatTheatre/orchil /var/www/html/client

cat >>/var/www/html/client/profiles.js <<EndOfMessage
"use strict";
// orchil/profiles.js
var profiles = {
        "portal_gables":{
                "method":   "websocket",
                "protocol": "ws",
                "server":   "localhost", //"chat.gables.chattheatre.com",
                "port":      10800,
                "woe_port":  10802,
                "path":     "/gables",
                "extra":    "",
                "reports":   false,
                "chars":    true,
        }
};
EndOfMessage

#####
## 25. Set up Thin-Auth
#####
#
# For thin-auth, add UDF variables for DBPASSWORD and PAYPALACCOUNT, plus
# all the game-specific stuff including your logo, etc. Hard to say if those
# could be UDF variables or if they need to be a different fork of thin-auth.
#
## thin-auth provides a SkotOS UserDB and full authentication facilities.
## It is normally prod-only, and *must* be installed into /var/www/html/user
git clone https://github.com/ChatTheatre/thin-auth /var/www/html/user
cp /var/www/html/user/config/database.json-SAMPLE /var/www/html/user/config/database.json
cp /var/www/html/user/config/financial.json-SAMPLE /var/www/html/user/config/financial.json
cp /var/www/html/user/config/general.json-SAMPLE /var/www/html/user/config/general.json
cp /var/www/html/user/config/server.json-SAMPLE /var/www/html/user/config/server.json

####
# See Also
####

# "Setting Up a SkotOS Host": https://hackmd.io/L_6wWAXVRamgCWFPG9SqTw
# https://github.com/ChatTheatre/thin-auth/blob/master/README.md
# https://github.com/ChatTheatre/orchil

####
# 754. Stuff that isn't done yet
####

# * UFW setup
# * Thin-Auth, including configuring its JSON files and its assets, game name, etc.
# * Asset server for images, etc.
# * Backups of any kind
