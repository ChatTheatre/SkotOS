#!/bin/bash

# Script to stand up a production-flavoured SkotOS instance on Linode.

# Based loosely on https://github.com/BlockchainCommons/Bitcoin-Standup-Scripts/blob/master/Scripts/LinodeStandUp.sh

# Tested against: Debian 10

# This block defines the variables the user of the script needs to input
# when deploying using this script.
#
# <UDF name="hostname" label="Short Hostname" example="Example: my-awesome-game"/>
# HOSTNAME=
# <UDF name="fqdn_client" label="Fully Qualified Client Hostname" example="Example: my-awesome-game.my-domain.com"/>
# FQDN_CLIENT=
# <UDF name="fqdn_login" label="Fully Qualified Thin-Auth Hostname" example="Example: my-awesome-game.my-domain.com"/>
# FQDN_LOGIN=
# <UDF name="userpassword" label="Deployment User Password" example="Password for the host deployment account." />
# USERPASSWORD=
# <UDF name="email" label="Support and PayPal Email" default="" example="Email for game support and for PayPal payments, if configured" optional="false" />
# EMAIL=
# <UDF name="ssh_key" label="SSH Key" default="" example="SSH Key for automated logins to host's deployment account." optional="true" />
# SSH_KEY=
# <UDF name="skotos_git_url" label="Skotos Git URL" default="https://github.com/ChatTheatre/SkotOS" example="SkotOS Git URL to clone for your game." optional="false" />
# SKOTOS_GIT_URL=
# <UDF name="dgd_git_url" label="DGD Git URL" default="https://github.com/ChatTheatre/dgd" example="DGD Git URL to clone for your game." optional="false" />
# DGD_GIT_URL=
# <UDF name="thinauth_git_url" label="Thin-Auth Git URL" default="https://github.com/ChatTheatre/thin-auth" example="Thin-Auth Git URL to clone for your game." optional="false" />
# THINAUTH_GIT_URL=


# Some differences from full real SkotOS setup:
#
# 1. No IP address whitelisting for SSH
# 2. Only 2 DNS names, client and login (thin-auth)
# 3. No hosting multiple games per host

# Some URLs you can check when testing:
#
# * http://$FQDN_CLIENT/gables/gables.htm

set -e  # Fail on error
set -x

# Force check for root. If you are not logged in as root then the script will not execute
if ! [ "$(id -u)" = 0 ]
then
  echo "$0 - You need to be logged in as root!"
  exit 1
fi

cd /

# Output stdout and stderr to ~root files
exec > >(tee -a /root/standup.log) 2> >(tee -a /root/standup.log /root/standup.err >&2)

echo "Hostname: $HOSTNAME"
echo "FQDN client: $FQDN_CLIENT"
echo "FQDN login: $FQDN_LOGIN"
echo "Support and PayPal email: $EMAIL"
echo "USERPASSWORD/dbpassword: (not shown)"
echo "SSH_KEY: (not shown)"
echo "SkotOS Git URL: $SKOTOS_GIT_URL"
echo "DGD Git URL: $DGD_GIT_URL"
echo "Thin-Auth Git URL: $THINAUTH_GIT_URL"

####
# 1. Update Hostname
####

echo $HOSTNAME > /etc/hostname
/bin/hostname $HOSTNAME

# Set the variable $IPADDR to the IP address the new Linode receives.
IPADDR=`hostname -I | awk '{print $1}'`

echo "$0 - Set hostname as $FQDN_CLIENT ($IPADDR)"
echo "$0 - TODO: Put $FQDN_CLIENT with IP $IPADDR in your main DNS file."
echo "$0 - TODO: Put $FQDN_LOGIN with IP $IPADDR in your main DNS file."

# Add localhost aliases

echo "127.0.0.1    localhost" > /etc/hosts
echo "127.0.1.1 $FQDN_CLIENT $FQDN_LOGIN $HOSTNAME" >> /etc/hosts

echo "$0 - Set localhost"

####
# 2. Bring Debian Up To Date
####

echo "$0 - Starting Debian updates; this may take a while!"

# Make sure all packages are up-to-date
apt-get update -y
apt-get upgrade -y
apt-get dist-upgrade -y

# Set system to automatically update
echo "unattended-upgrades unattended-upgrades/enable_auto_updates boolean true" | debconf-set-selections
apt-get -y install unattended-upgrades

echo "$0 - Updated Debian Packages"

# get uncomplicated firewall and deny all incoming connections except SSH
apt-get install ufw -y
ufw default allow outgoing
ufw default deny incoming
ufw allow ssh
ufw allow 10000:10802/tcp  # for now, allow DGD incoming ports and tunnel ports
ufw allow 80/tcp
ufw allow 81/tcp
ufw enable

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
apt-get install mariadb-server php-fpm php php-mysql -y

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
    git clone ${SKOTOS_GIT_URL} /var/skotos
    #git clone https://github.com/ChatTheatre/SkotOS /var/skotos
    chgrp -R skotos /var/skotos
    chmod -R g+w /var/skotos
fi

if [ -d "/var/dgd" ]
then
    pushd /var/dgd
    git pull
    popd
else
    git clone ${DGD_GIT_URL} /var/dgd
    #git clone https://github.com/ChatTheatre/dgd /var/dgd
fi

pushd /var/dgd/src
make DEFINES='-DUINDEX_TYPE="unsigned int" -DUINDEX_MAX=UINT_MAX -DEINDEX_TYPE="unsigned short" -DEINDEX_MAX=USHRT_MAX -DSSIZET_TYPE="unsigned int" -DSSIZET_MAX=1048576' install
popd

cat >>~skotos/crontab.txt <<EndOfMessage
@reboot /var/skotos/dev_scripts/stackscript/start_dgd_server.sh &
EndOfMessage

/var/skotos/dev_scripts/stackscript/start_dgd_server.sh &

####
# Set up NGinX
####

# Fix a bad default in php-fpm
sed 's/;cgi.fix_pathinfo=1/cgi.fix_pathinfo=0/' </etc/php/7.3/fpm/php.ini >/etc/php/7.3/fpm/php-fixed.ini
mv /etc/php/7.3/fpm/php-fixed.ini /etc/php/7.3/fpm/php.ini

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
    listen *:80 default_server;

    server_name $FQDN_CLIENT;
    index index.html index.htm ;

    root /var/www/html/client;
    location / {
        try_files \$uri \$uri/index.html \$uri.html =404;
    }
}

server {
    listen *:80 ;
    listen *:81 default_server ;

    server_name $FQDN_LOGIN ;

    root /var/www/html/user ;
    index index.php index.html index.htm ;

    location / {
        try_files \$uri \$uri/index.php \$uri/index.html \$uri.html =404 ;
    }

    location ~ \.php$ {
        include snippets/fastcgi-php.conf ;
        fastcgi_pass unix:/run/php/php7.3-fpm.sock ;
    }

    location ~ /\.ht {
        deny all;
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
            "host": "$FQDN_CLIENT",
            "sendTunnelInfo": false
        },
        {
            "name": "skotos-tree-of-woe",
            "listen": 10802,
            "send": 10090,
            "host": "$FQDN_CLIENT",
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

cat >/var/www/html/client/profiles.js <<EndOfMessage
"use strict";
// orchil/profiles.js
var profiles = {
        "portal_gables":{
                "method":   "websocket",
                "protocol": "ws",
                "server":   "$FQDN_CLIENT", //"chat.gables.chattheatre.com",
                "port":      10800,
                "woe_port":  10802,
                "http_port": 10080,
                "path":     "/gables",
                "extra":    "",
                "reports":   false,
                "chars":    true,
        }
};
EndOfMessage

#####
## 9. Set up MariaDB for Thin-Auth
#####

mysql --user=root <<EndOfMessage
CREATE DATABASE userdb;
CREATE USER 'userdb'@'localhost' IDENTIFIED BY '$USERPASSWORD';
GRANT ALL ON userdb.* TO 'userdb'@'localhost';
EndOfMessage

#####
## 25. Set up Thin-Auth
#####
#
# For thin-auth, add UDF variable for DBPASSWORD, plus
# all the game-specific stuff including your logo, etc. Hard to say if those
# could be UDF variables or if they need to be a different fork of thin-auth.
#
## thin-auth provides a SkotOS UserDB and full authentication facilities.
## It is normally prod-only, and *must* be installed into /var/www/html/user
git clone $THINAUTH_GIT_URL /var/www/html/user
cat >/var/www/html/user/config/database.json <<EndOfMessage
{

    "dbName": "userdb",
    "dbUser": "userdb",
    "dbPass": "$USERPASSWORD"
}
EndOfMessage

cat >/var/www/html/user/config/financial.json <<EndOfMessage
{

    "paypalAcct": "$EMAIL",
    "basicMonth": "12.95",
    "basicQuarter": "34.95",
    "basicYear": "129.95",
    "premiumMonth": "29.95",
    "premiumQuarter": "79.95",
    "premiumYear": "299.95",
    "premiumToBasicConversion": "2.3",
    "monthlyPremiumSPs": "50",
    "spMaxBasic": "50",
    "spCostBasic": "34.95",
    "spMaxPremium": "250",
    "spCostPremium": "24.95"
    "royalties": {
    }

}
EndOfMessage

cat >/var/www/html/user/config/general.json <<EndOfMessage
{

    "gameID": "gables",
    "siteLogo": "gables-small.jpg",
    "siteName": "Gables",
    "userdbURL": "$FQDN_LOGIN",
    "webURL": "$FQDN_CLIENT:10080/gables/gables.htm",
    "woeURL": "$FQDN_CLIENT:10080/gables/TreeOfWoe.html",
    "gameURL": "$FQDN_CLIENT",
    "supportEmail": "$EMAIL"
}
EndOfMessage

cat >/var/www/html/user/config/server.json <<EndOfMessage
{

    "serverIP": "0.0.0.0",
    "serverAuthPort": 9970,
    "serverCtlPort": 9971
}
EndOfMessage

####
# Finished
####
nginx -s reload
touch ~/standup_finished_successfully.txt

####
# See Also
####

# https://github.com/ChatTheatre/thin-auth/blob/master/README.md
# https://github.com/ChatTheatre/orchil

####
# 754. Stuff that isn't done yet
####

# * Asset server for images, etc.
# * Backups of any kind
