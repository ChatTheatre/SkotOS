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
# <UDF name="skotos_git_branch" label="Skotos Git Branch" default="master" example="SkotOS branch, tag or commit to clone for your game." optional="false" />
# SKOTOS_GIT_BRANCH=
# <UDF name="dgd_git_url" label="DGD Git URL" default="https://github.com/ChatTheatre/dgd" example="DGD Git URL to clone for your game." optional="false" />
# DGD_GIT_URL=
# <UDF name="dgd_git_branch" label="DGD Git Branch" default="master" example="DGD Git branch, tag or commit to clone for your game." optional="false" />
# DGD_GIT_BRANCH=
# <UDF name="thinauth_git_url" label="Thin-Auth Git URL" default="https://github.com/ChatTheatre/thin-auth" example="Thin-Auth Git URL to clone for your game." optional="false" />
# THINAUTH_GIT_URL=
# <UDF name="thinauth_git_branch" label="Thin-Auth Git Branch" default="master" example="Thin-auth branch, tag or commit to clone for your game." optional="false" />
# THINAUTH_GIT_BRANCH=
# <UDF name="tunnel_git_url" label="Websocket Tunnel Git URL" default="https://github.com/ChatTheatre/websocket-to-tcp-tunnel" example="Websocket Tunnel Git URL to clone for your game." optional="false" />
# TUNNEL_GIT_URL=
# <UDF name="tunnel_git_branch" label="Websocket Tunnel Git Branch" default="master" example="Websocket Tunnel branch, tag or commit to clone for your game." optional="false" />
# TUNNEL_GIT_BRANCH=

# By going to the root of the login URL, you should find a PHP account server (thin-auth)
# where you can log in as "skott" with your supplied password. The skott account can give
# admin permissions to other accounts.

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
echo "USERPASSWORD/dbpassword: (not shown)"
echo "Support and PayPal email: $EMAIL"
echo "SSH_KEY: (not shown)"
echo "SkotOS Git URL: $SKOTOS_GIT_URL"
echo "SkotOS Git Branch: $SKOTOS_GIT_BRANCH"
echo "DGD Git URL: $DGD_GIT_URL"
echo "DGD Git Branch: $DGD_GIT_BRANCH"
echo "Thin-Auth Git URL: $THINAUTH_GIT_URL"
echo "Thin-Auth Git Branch: $THINAUTH_GIT_BRANCH"

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
echo "127.0.0.1 $FQDN_CLIENT $FQDN_LOGIN $HOSTNAME" >> /etc/hosts

echo "$0 - Set localhost"

####
# 2. Bring Debian Up To Date
####

echo "$0 - Starting Debian updates; this may take a while!"

# Certain problems, like Linode and Debian fighting over /etc/default/grub, require
# a non-interactive frontend to avoid the script hanging forever. Of course if the
# default resolution for the problem is wrong then you're just out of luck.
#DEBIAN_FRONTEND=noninteractive

# Linode and Debian are fighting over Grub config. Back up the Linode Grub config before updating
# and turn off interactive front end for apt.
# See:
# * https://www.linode.com/community/questions/17018/grub-updated-its-configuration-and-now-there-are-issues
# * https://www.linode.com/community/questions/17015/grub-default-settings
#
cp /etc/default/grub /root/linode_grub_config

# Make sure all packages are up-to-date
apt-get update -y
apt-get upgrade -y
#apt-get dist-upgrade -y

# Restore Linode grub config
cp /root/linode_grub_config /etc/default/grub
update-grub # Use the reinstalled config

# Set system to automatically update
echo "unattended-upgrades unattended-upgrades/enable_auto_updates boolean true" | debconf-set-selections
apt-get -y install unattended-upgrades

echo "$0 - Updated Debian Packages"

# get uncomplicated firewall and deny all incoming connections except SSH
apt-get install ufw -y
ufw default allow outgoing
ufw default deny incoming
ufw allow ssh

ufw allow 11000:11803/tcp  # for now, allow all DGD incoming ports and tunnel ports

ufw allow 11098/tcp # DGD telnet port
ufw allow 11810/tcp # Gables game WSS websocket
ufw allow 11812/tcp # Gables WOE WSS websocket
ufw allow 10803/tcp # Gables https-ified DGD web port

# Allow these ports in case of local Jitsi install
ufw allow 10000/udp # For Jitsi Meet server
ufw allow 3478/udp # For STUN server
ufw allow 4443/tcp # For RTP media over TCP (says https://jitsi.github.io/handbook/docs/devops-guide/devops-guide-docker)
ufw allow 5349/tcp # For fallback video/audio with coturn

# Currently we do not expose other DGD ports like ExportD or TextIF.
# The security on those ports isn't great, so normally you should
# ssh into the host and connect to them locally. The security on
# AuthD and CtlD are *terrible* and you should NEVER expose them.

ufw deny 11070:11071/tcp # NEVER allow AuthD/CtlD connections from off-VM
ufw allow 80/tcp
ufw allow 443/tcp
ufw enable

####
# 3. Set Up Initial User
####

# Create "skotos" user with optional password and give them sudo capability
/usr/sbin/useradd -m -p `perl -e 'printf("%s\n",crypt($ARGV[0],"password"))' "$USERPASSWORD"` -g sudo -s /bin/bash skotos || echo "Skotos user already exists?"
/usr/sbin/adduser skotos sudo

echo "$0 - Setup skotos with sudo access."

# Authorize root's SSH keys for skotos user too
mkdir -p ~skotos/.ssh
cat ~root/.ssh/authorized_keys >~skotos/.ssh/authorized_keys || echo "OK"
chown -R skotos ~skotos/.ssh
echo "$0 - Added root's .ssh keys to skotos user."

# Setup SSH Key if the user added one as an argument
if [ -n "$SSH_KEY" ]
then
   echo "$SSH_KEY" >> ~skotos/.ssh/authorized_keys
   echo "$0 - Added .ssh key to skotos."
fi

####
# 4. Install Dependencies
####

# NGinX and DGD build requirements
apt-get install git nginx-full cron bison build-essential -y

# Websocket-to-tcp-tunnel requirements
# curl -sL https://deb.nodesource.com/setup_9.x | bash -

curl -sL https://deb.nodesource.com/setup_16.x | bash -
apt install nodejs -y

# Thin-auth requirements
apt-get install mariadb-server php php-fpm php-mysql certbot python3-certbot-nginx -y

# Misc and Debugging
apt-get install debconf-utils -y  # For debconf-get-selections

# Email (outgoing)
# Need to update this? See: https://www.digitalocean.com/community/tutorials/how-to-install-and-configure-postfix-as-a-send-only-smtp-server-on-debian-10
# Need additional selections? See: https://manpages.debian.org/stretch/debconf-utils/debconf-get-selections.1.en.html
echo "postfix postfix/main_mailer_type select Internet Site" | debconf-set-selections
echo "postfix postfix/mailname string $FQDN_LOGIN" | debconf-set-selections
apt-get -y install mailutils postfix
sed -i 's/inet_interfaces = all/inet_interfaces = loopback-only/' /etc/postfix/main.cf
sudo systemctl restart postfix

# Dgd-tools requirements
apt-get install ruby-full zlib1g-dev -y

# Docker for (at least) Jitsi
apt-get install apt-transport-https ca-certificates curl software-properties-common gnupg -y
curl -fsSL https://download.docker.com/linux/debian/gpg | sudo apt-key add -
sudo apt-key fingerprint 0EBFCD88
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/debian $(lsb_release -cs) stable"

apt-get update -y
apt-get install docker-ce -y

# SkotOS should be in the Docker group
usermod -aG docker skotos

curl -L https://github.com/docker/compose/releases/download/1.29.1/docker-compose-`uname -s`-`uname -m` -o /usr/local/bin/docker-compose
chmod +x /usr/local/bin/docker-compose

####
# Install dgd-tools
####

# dgd-tools contains dgd-manifest
gem install dgd-tools bundler

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

# e.g. clone_or_update "$SKOTOS_GIT_URL" "$SKOTOS_GIT_BRANCH" "/var/skotos"
function clone_or_update {
  if [ -d "$3" ]
  then
    pushd "$3"
    git fetch # Needed for "git checkout" if the branch has been added recently
    git checkout "$2"
    git pull
    popd
  else
    git clone "$1" "$3"
    pushd "$3"
    git checkout "$2"
    popd
  fi
  chgrp -R skotos "$3"
  chown -R skotos "$3"
  chmod -R g+w "$3"
}

clone_or_update "$SKOTOS_GIT_URL" "$SKOTOS_GIT_BRANCH" "/var/skotos"

clone_or_update "$DGD_GIT_URL" "$DGD_GIT_BRANCH" /var/dgd
pushd /var/dgd/src
make DEFINES='-DUINDEX_TYPE="unsigned int" -DUINDEX_MAX=UINT_MAX -DEINDEX_TYPE="unsigned short" -DEINDEX_MAX=USHRT_MAX -DSSIZET_TYPE="unsigned int" -DSSIZET_MAX=1048576' install
popd

# Fix the login URL
HTTP_FILE=/var/skotos/skoot/usr/HTTP/sys/httpd.c
if grep -F "www.skotos.net/user/login.php" $HTTP_FILE
then
    # Unpatched - need to patch
    sed -i "s_https://www.skotos.net/user/login.php_https://${FQDN_LOGIN}_" $HTTP_FILE
else
    echo "HTTPD appears to be patched already. Moving on..."
fi

# I feel mixed about hardcoding this file entirely. What about changes?
# But I also don't think sed-surgery is going to work well here. We want
# a prod deployment to have a different UserDB setup than for development.
cat >/var/skotos/skoot/usr/System/data/instance <<EndOfMessage
portbase 11000
hostname $FQDN_CLIENT
login_hostname $FQDN_LOGIN
bootmods DevSys Theatre Jonkichi Tool Generic SMTP UserDB Gables
textport 443
real_textport 11443
webport 11803
real_webport 11080
url_protocol https
access gables
memory_high 128
memory_max 256
statedump_offset 600
freemote +emote
EndOfMessage

mkdir -p /var/log/dgd/
chown skotos:skotos /var/log/dgd/

# Turn off the DGD server and prevent further automatic restarts.
# Normally a game like Gables or RWOT will install its own app directory.
# The 'vanilla' SkotOS dir here is primarily for deploy and ops scripts
# and similar basic infrastructure.
touch /var/skotos/no_restart.txt
/var/skotos/deploy_scripts/stackscript/stop_dgd_server.sh

####
# Set up NGinX for websockets
####

rm -f /etc/nginx/sites-enabled/default /etc/nginx/sites-enabled/*.conf

# TODO: Proper HTTPS termination for connections to port 11080
cat >/etc/nginx/sites-available/skotos_game.conf <<EndOfMessage
# skotos_game.conf

map \$http_upgrade \$connection_upgrade {
    default upgrade;
        '' close;
        }

# Websocket connection via relay to DGD's TextIF port (11443)
upstream gables-ws {
    server 127.0.0.1:11801;
}

# Websocket connection via relay to DGD's WOE port (11090)
upstream woe-ws {
    server 127.0.0.1:11802;
}

# Connection to DGD web port at 11080 so we can https-terminate
upstream skotosdgd {
    server 127.0.0.1:11080;
}

server {
    listen *:11810;
    server_name $FQDN_CLIENT;

    location /gables {
      proxy_pass http://gables-ws;
      proxy_pass_request_headers on;
      proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
      proxy_set_header X-Real-IP \$remote_addr;
      proxy_set_header X-Forwarded-Proto \$scheme;
      proxy_set_header Host \$host;
      proxy_http_version 1.1;
      proxy_set_header Upgrade \$http_upgrade;
      proxy_set_header Connection \$connection_upgrade;
    }
    #ssl_certificate /etc/letsencrypt/live/$FQDN_CLIENT/fullchain.pem; # managed by Certbot
    #ssl_certificate_key /etc/letsencrypt/live/$FQDN_CLIENT/privkey.pem; # managed by Certbot
}

# Tree of WOE wss websocket
server {
    listen *:11812;
    server_name $FQDN_CLIENT;

    location / {
      proxy_pass http://woe-ws;
      proxy_pass_request_headers on;
      proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
      proxy_set_header X-Real-IP \$remote_addr;
      proxy_set_header X-Forwarded-Proto \$scheme;
      proxy_set_header Host \$host;
      proxy_http_version 1.1;
      proxy_set_header Upgrade \$http_upgrade;
      proxy_set_header Connection \$connection_upgrade;
    }
    #ssl_certificate /etc/letsencrypt/live/$FQDN_CLIENT/fullchain.pem; # managed by Certbot
    #ssl_certificate_key /etc/letsencrypt/live/$FQDN_CLIENT/privkey.pem; # managed by Certbot
}

# Pass HTTPS connections on port 11803 to DGD on port 11080 after https termination
server {
    listen *:11803;
    server_name $FQDN_CLIENT;

    location / {
      proxy_pass http://skotosdgd;
      proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
      proxy_set_header X-Real-IP \$remote_addr;
      proxy_set_header X-Forwarded-Proto \$scheme;
      proxy_set_header Host \$host;
    }

    #ssl_certificate /etc/letsencrypt/live/$FQDN_CLIENT/fullchain.pem; # managed by Certbot
    #ssl_certificate_key /etc/letsencrypt/live/$FQDN_CLIENT/privkey.pem; # managed by Certbot
}
EndOfMessage

rm -f /etc/nginx/sites-enabled/skotos_game.conf
ln -s /etc/nginx/sites-available/skotos_game.conf /etc/nginx/sites-enabled/skotos_game.conf

nginx -t  # Verify everything parses correctly
nginx -s reload

####
# 7. Set up Tunnel
####

mkdir -p /var/log/tunnel
chown -R skotos.skotos /var/log/tunnel

clone_or_update "$TUNNEL_GIT_URL" "$TUNNEL_GIT_BRANCH" /usr/local/websocket-to-tcp-tunnel

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
            "name": "gables",
            "listen": 11801,
            "send": 11443,
            "host": "$FQDN_CLIENT",
            "sendTunnelInfo": false
        },
        {
            "name": "gables-tree-of-woe",
            "listen": 11802,
            "send": 11090,
            "host": "$FQDN_CLIENT",
            "sendTunnelInfo": false
        }
    ]
}
EndOfMessage

mkdir -p /var/log/userdb/
chown skotos:skotos /var/log/userdb
touch /var/log/userdb.log  # filename is set by thin-auth
chown skotos /var/log/userdb.log

sudo -u skotos /usr/local/websocket-to-tcp-tunnel/stop-tunnel.sh
sudo -u skotos /usr/local/websocket-to-tcp-tunnel/search-tunnel.sh
cat >>~skotos/crontab.txt <<EndOfMessage
@reboot /usr/local/websocket-to-tcp-tunnel/start-tunnel.sh
* * * * * /usr/local/websocket-to-tcp-tunnel/search-tunnel.sh
* * * * * /bin/bash -c "/var/www/html/user/admin/restartuserdb.sh >>/var/log/userdb/servers.txt"
* * * * * /var/skotos/deploy_scripts/stackscript/keep_authctl_running.sh
1 5 1-2 * * /usr/bin/certbot renew
EndOfMessage
crontab -u skotos ~skotos/crontab.txt

####
# 8. Patch built-in Orchil
####

cat >/var/skotos/skoot/usr/Gables/data/www/profiles.js <<EndOfMessage
"use strict";
// orchil/profiles.js
var profiles = {
        "portal_gables":{
                "method":   "websocket",
                "protocol": "wss",
                "web_protocol": "https",
                "server":   "$FQDN_CLIENT",
                "port":      11810,
                "woe_port":  11812,
                "http_port": 11803,
                "path":     "/gables",
                "extra":    "",
                "reports":   false,
                "chars":    true,
        }
};
EndOfMessage

####
# Set up client redirect to the root URL goes to character creation
####

mkdir -p /var/www/html/client

cat >/var/www/html/client/index.htm <<EndOfMessage
<html>
<head>
<title> Redirecting... </title>
<meta http-equiv="refresh" content="0; url='https://$FQDN_CLIENT:11803/SAM/Prop/Theatre:Web:Theatre/Index'">
</head>
<body>

<p>
  Redirecting you to the game login page...
</p>

</body>
EndOfMessage

#####
## 9. Set up MariaDB for Thin-Auth
#####

clone_or_update "$THINAUTH_GIT_URL" "$THINAUTH_GIT_BRANCH" /var/www/html/user
pushd /var/www/html/user

mysql --user=root <<EndOfMessage
DROP DATABASE IF EXISTS userdb;
CREATE DATABASE userdb;
CREATE USER IF NOT EXISTS 'userdb'@'localhost' IDENTIFIED BY '$USERPASSWORD';
GRANT ALL ON userdb.* TO 'userdb'@'localhost';
FLUSH PRIVILEGES;
EndOfMessage

cat database/userdb-schema.mysql | mysql --user=userdb --password=$USERPASSWORD userdb

export PHP_HASHED_PASS=`php gen-pass.php "$USERPASSWORD"`
export BASH_ESCAPED_PASS=`printf "%q" $PHP_HASHED_PASS`

mysql --user=root userdb <<EndOfMessage
INSERT INTO users (name, email, creation_time, password, pay_day, next_month, next_year, next_stamp, account_type, user_updated)
  VALUES ('skott', 'fake-email@fake-domain.com', 1610538280, '$BASH_ESCAPED_PASS', 13, 12, 2021, 1613130280, 'developer', 3);
INSERT INTO access (ID, game) SELECT ID, 'gables' FROM users WHERE users.name = 'skott';
INSERT INTO flags (ID, flag) SELECT ID, 'terms-of-service' FROM users WHERE users.name = 'skott';

INSERT INTO users (name, email, creation_time, password, pay_day, next_month, next_year, next_stamp, account_type, user_updated)
 VALUES ('nobody', 'fake-email-2@fake-domain.com', 1610538280, '', 13, 12, 2021, 1613130280, 'trial', 3);
INSERT INTO flags (ID, flag) SELECT ID, 'no-email' FROM users WHERE users.name = 'nobody';
INSERT INTO flags (ID, flag) SELECT ID, 'premium' FROM users WHERE users.name = 'nobody';
INSERT INTO flags (ID, flag) SELECT ID, 'grand' FROM users WHERE users.name = 'nobody';
INSERT INTO flags (ID, flag) SELECT ID, 'terms-of-service' FROM users WHERE users.name = 'nobody';
INSERT INTO flags (ID, flag) SELECT ID, 'deleted' FROM users WHERE users.name = 'nobody';
INSERT INTO flags (ID, flag) SELECT ID, 'banned' FROM users WHERE users.name = 'nobody';
EndOfMessage

popd

#####
## 25. Set up Thin-Auth
#####
#
## thin-auth provides a SkotOS UserDB and full authentication facilities.
## It is normally prod-only, and *must* be installed into /var/www/html/user
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
    "siteName": "The Gables",
    "userdbURL": "$FQDN_LOGIN",
    "webURL": "https://$FQDN_LOGIN",
    "woeURL": "https://$FQDN_CLIENT:11803/gables/TreeOfWoe.html",
    "gameURL": "https://$FQDN_CLIENT",
    "supportEmail": "$EMAIL"
}
EndOfMessage

cat >/var/www/html/user/config/server.json <<EndOfMessage
{

    "serverIP": "0.0.0.0",
    "serverAuthPort": 11970,
    "serverCtlPort": 11971
}
EndOfMessage

####
# Set up FQDN_LOGIN site and SkotOS-client site
####

# Enable short tags for PHP
#sed -i 's/short_open_tag = Off/short_open_tag = On/' /etc/php/7.3/apache2/php.ini
#sed -i 's/short_open_tag = Off/short_open_tag = On/' /etc/php/7.3/fpm/php.ini
sed -i 's/short_open_tag = Off/short_open_tag = On/' /etc/php/7.4/fpm/php.ini

#/etc/init.d/php7.3-fpm restart
/etc/init.d/php7.4-fpm restart

cat >/etc/nginx/sites-available/login.conf <<EndOfMessage
server {
    listen *:80;
    server_name $FQDN_LOGIN;

    return 301 https://\$host\$request_uri;
}

server {
    listen *:443;
    server_name $FQDN_LOGIN;

    root /var/www/html/user;
    index index.php index.html index.htm;

    location / {
        try_files \$uri \$uri/ =404;
    }

    location ~ \.php\$ {
        include snippets/fastcgi-php.conf;
        #fastcgi_pass unix:/run/php/php7.3-fpm.sock;
        fastcgi_pass unix:/run/php/php7.4-fpm.sock;
    }

    location ~ /\.ht {
        deny all;
    }

    #ssl_certificate /etc/letsencrypt/live/$FQDN_LOGIN/fullchain.pem; # managed by Certbot
    #ssl_certificate_key /etc/letsencrypt/live/$FQDN_LOGIN/privkey.pem; # managed by Certbot
}
EndOfMessage
rm -f /etc/nginx/sites-enabled/login.conf
ln -s /etc/nginx/sites-available/login.conf /etc/nginx/sites-enabled/login.conf

cat >/etc/nginx/sites-available/skotos-client.conf <<EndOfMessage
server {
    listen *:80;
    server_name $FQDN_CLIENT;

    return 301 https://\$host\$request_uri;
}

server {
    listen *:443;
    server_name $FQDN_CLIENT;

    root /var/www/html/client;
    index index.html index.htm;

    location /assets {
      root /var/skotos/skoot/usr/Gables/data/www/assets;
    }

    location / {
      try_files \$uri \$uri/ =404;
    }

    #ssl_certificate /etc/letsencrypt/live/$FQDN_CLIENT/fullchain.pem; # managed by Certbot
    #ssl_certificate_key /etc/letsencrypt/live/$FQDN_CLIENT/privkey.pem; # managed by Certbot
}
EndOfMessage
rm -f /etc/nginx/sites-enabled/skotos-client.conf
ln -s /etc/nginx/sites-available/skotos-client.conf /etc/nginx/sites-enabled/skotos-client.conf

nginx -t
nginx -s reload

####
# Certbot for SSL
####

# Do this last - it depends on DNS propagation, which can be weird. That way if this fails, only a little needs to be redone.
certbot --non-interactive --nginx --agree-tos certonly -m webmaster@$FQDN_CLIENT -d $FQDN_CLIENT
certbot --non-interactive --nginx --agree-tos certonly -m webmaster@$FQDN_CLIENT -d $FQDN_LOGIN

####
# Reconfigure NGinX for LetsEncrypt
####

pushd /etc/nginx/sites-available
sed -i "s/#ssl_cert/ssl_cert/g" skotos_game.conf  # Uncomment SSL cert usage
sed -i "s/#proxy_ssl_cert/proxy_ssl_cert/g" skotos_game.conf  # Uncomment proxy SSL cert usage, if any
sed -i "s/#ssl_cert/ssl_cert/g" skotos-client.conf  # Uncomment SSL cert usage
sed -i "s/#ssl_cert/ssl_cert/g" login.conf  # Uncomment SSL cert usage

sed -i "s/listen \*.[0-9]*/& ssl/g" skotos_game.conf
sed -i "s/listen \*.443/& ssl/g" skotos-client.conf
sed -i "s/listen \*.443/& ssl/g" login.conf
popd

nginx -t
nginx -s reload

####
# Set up chat_admin_server, but don't actually start it
####

CHAT_ADMIN_GIT_URL="https://github.com/ChatTheatre/chat_admin_server"
CHAT_ADMIN_GIT_BRANCH="main"

clone_or_update "$CHAT_ADMIN_GIT_URL" "$CHAT_ADMIN_GIT_BRANCH" "/var/chat_admin_server"
pushd /var/chat_admin_server
npm install
popd

# For now, don't start the server. Remove this file to start it.
touch /var/chat_admin_server/NO_START.txt

# The cron job will check regularly whether to start the server.
cat >>~skotos/crontab.txt <<EndOfMessage
* * * * * /var/chat_admin_server/deploy/skotos/start-chat-admin.sh
EndOfMessage
crontab -u skotos ~skotos/crontab.txt


####
# Finished
####
touch ~/standup_finished_successfully.txt

####
# 754. Stuff that isn't done yet
####

# * Asset server for images, etc.
# * Backups of any kind
# * Log rotation (see: https://github.com/ChatTheatre/thin-auth#12c-rotating-your-logs)
