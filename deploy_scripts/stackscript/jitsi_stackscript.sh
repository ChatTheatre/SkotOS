#!/bin/bash

# Jitsi stackscript, extracted from linode_stackscript.sh Jitsi install code

# This block defines the variables the user of the script needs to input
# when deploying using this script.
#
# <UDF name="fqdn_jitsi" label="Full Hostname" example="Example: jitsi.my-awesome-game.com"/>
# FQDN_JITSI=
# <UDF name="userpassword" label="Deployment User Password" example="Password for the host deployment account." />
# USERPASSWORD=
# <UDF name="ssh_key" label="SSH Key" default="" example="SSH Key for automated logins to host's deployment account." optional="true" />
# SSH_KEY=

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

export HOSTNAME=`echo "$FQDN_JITSI" | cut -f 1 -d .`

echo "Jitsi Server Hostname: $FQDN_JITSI"
echo "Short hostname: $HOSTNAME"

echo $HOSTNAME > /etc/hostname
/bin/hostname $HOSTNAME

# Set the variable $IPADDR to the IP address the new Linode receives.
IPADDR=`hostname -I | awk '{print $1}'`

echo "$0 - Set hostname as $FQDN_JITSI ($IPADDR)"
echo "$0 - TODO: Put $FQDN_JITSI with IP $IPADDR in your main DNS file."

# Add localhost aliases

echo "127.0.0.1    localhost" > /etc/hosts
echo "127.0.0.1 $FQDN_JITSI $HOSTNAME" >> /etc/hosts

echo "$0 - Set localhost"

## Update Ubuntu

# Make sure all packages are up-to-date
apt-get update -y
apt-get upgrade -y
#apt-get dist-upgrade -y

# Set system to automatically update
echo "unattended-upgrades unattended-upgrades/enable_auto_updates boolean true" | debconf-set-selections
apt-get -y install unattended-upgrades

# get uncomplicated firewall and deny all incoming connections except SSH
apt-get install ufw -y
ufw default allow outgoing
ufw default deny incoming
ufw allow ssh

# Jitsi needs these open
ufw allow 10000/udp # For Jitsi Meet server
ufw allow 3478/udp # For STUN server
ufw allow 4443/tcp # For RTP media over TCP (says https://jitsi.github.io/handbook/docs/devops-guide/devops-guide-docker)
ufw allow 5349/tcp # For fallback video/audio with coturn

# HTTP, HTTPS for Jitsi Meet and IFrame interface
ufw allow 80/tcp
ufw allow 443/tcp

ufw enable

####
# Set Up Initial User
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
# Various packages
####

# Misc and Debugging
apt-get install debconf-utils -y  # For debconf-get-selections
apt-get install silversearcher-ag # For recursive file search

# NGinX
apt-get install git nginx-full cron build-essential -y

# Certbot
apt-get install certbot python3-certbot-nginx -y

####
# Docker and Docker-Compose Install
####

#apt-get install apt-transport-https ca-certificates curl software-properties-common gnupg -y
#curl -fsSL https://download.docker.com/linux/debian/gpg | sudo apt-key add -
#sudo apt-key fingerprint 0EBFCD88
#sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/debian $(lsb_release -cs) stable"
#
#apt-get update -y
#apt-get install docker-ce -y
#
## SkotOS should be in the Docker group
#usermod -aG docker skotos
#
#curl -L https://github.com/docker/compose/releases/download/1.29.1/docker-compose-`uname -s`-`uname -m` -o /usr/local/bin/docker-compose
#chmod +x /usr/local/bin/docker-compose

####
# Set up Directories, Groups and Ownership
####

groupadd skotos || echo "Skotos group already exists"

rm -f ~skotos/crontab.txt
touch ~skotos/crontab.txt
chown -R skotos.skotos ~skotos/

####
# Jitsi local not-yet-JWT install
####

# Jitsi requires OpenJDK 8 or 11 right now.
apt-get install openjdk-11-jdk
update-java-alternatives -s /usr/lib/jvm/java-1.11.0-openjdk-amd64

apt-get install -y gnupg2 apt-transport-https ca-certificates curl

# Add Jitsi package repository (https://jitsi.github.io/handbook/docs/devops-guide/devops-guide-quickstart)
curl https://download.jitsi.org/jitsi-key.gpg.key | sh -c 'gpg --dearmor > /usr/share/keyrings/jitsi-keyring.gpg'
echo 'deb [signed-by=/usr/share/keyrings/jitsi-keyring.gpg] https://download.jitsi.org stable/' | tee /etc/apt/sources.list.d/jitsi-stable.list > /dev/null

# Need to install a specific prosody version for Ubuntu 18
# See: https://jitsi.github.io/handbook/docs/devops-guide/devops-guide-quickstart#for-ubuntu-1804-add-prosody-package-repository
#echo deb http://packages.prosody.im/debian $(lsb_release -sc) main | tee -a /etc/apt/sources.list
#wget https://prosody.im/files/prosody-debian-packages.key -O- | apt-key add -

apt-get -y update
echo "jitsi-videobridge jitsi-videobridge/jvb-hostname string $FQDN_JITSI" | debconf-set-selections
echo "jitsi-videobridge2 jitsi-videobridge/jvb-hostname string $FQDN_JITSI" | debconf-set-selections
echo "jitsi-meet jitsi-meet/cert-choice select Self-signed certificate will be generated" | debconf-set-selections
export DEBIAN_FRONTEND=noninteractive
apt install -y jitsi-meet

# TODO: adjust /etc/systemd/system.conf limits (these numbers OR HIGHER)
# (Right now these limits are fine by default on Linode Ubuntu 20.)
# After changing: systemctl daemon-reload
# After changing: systemctl restart jitsi-videobridge2
# DefaultLimitNOFILE=65000
# DefaultLimitNPROC=65000
# DefaultTasksMax=65000

echo "admin@$FQDN_JITSI" | /usr/share/jitsi-meet/scripts/install-letsencrypt-cert.sh

# Do we need /usr/share/jitsi-meet/scripts/install-letsencrypt-cert.sh? Didn't reliably work with older config...

JITSI_NGINX_ENABLED_CONF="/etc/nginx/sites-enabled/${FQDN_JITSI}.conf"
JITSI_NGINX_AVAIL_CONF="/etc/nginx/sites-available/${FQDN_JITSI}.conf"

rm -f /etc/nginx/sites-enabled/default
rm -f $JITSI_NGINX_ENABLED_CONF # It's still in sites-available if we want it

# The default Jitsi conf fails because it contains a duplicate server_names_hash_bucket_size directive
sed -i 's/server_names_hash_bucket_size/#server_names_hash_bucket_size/' $JITSI_NGINX_AVAIL_CONF

# The config will fail on this line if you haven't added a DNS entry or if the DNS info hasn't propagated yet.
certbot certonly --non-interactive --nginx --agree-tos -m webmaster@$FQDN_JITSI -d $FQDN_JITSI

# re-add Jitsi NGinX file
ln -s /etc/nginx/sites-available/$FQDN_JITSI.conf /etc/nginx/sites-enabled/$FQDN_JITSI.conf

nginx -t # Test config
nginx -s reload # reload config

####
# Jitsi JWT install
####

# Source: https://jitsi.github.io/handbook/docs/devops-guide/devops-guide-quickstart
# Source: https://jitsi.github.io/handbook/docs/devops-guide/secure-domain
# Source: https://doganbros.com/index.php/jitsi/jitsi-installation-with-jwt-support-on-ubuntu-20-04-lts/
# Source: https://meetrix.io/blog/webrtc/jitsi/meet/how-to-authenticate-users-to-Jitsi-meet-using-JWT-tokens.html

# Install security repo - we need Ubuntu bionic (v. old) for libssl1.0-dev
echo deb http://security.ubuntu.com/ubuntu bionic-security main | tee -a /etc/apt/sources.list.d/debian_security_repo.list > /dev/null

# In a different world we could install on Debian with the Stretch repo, but that's not working for me
#echo deb http://ftp.de.debian.org/debian stretch main | tee -a /etc/apt/sources.list.d/debian_older_repo.list > /dev/null
# If we add Strech as a repo, we need to make Buster the explicit default
#echo 'APT::Default-Release "buster";' > /etc/apt/apt.conf.d/80buster_default

apt-get -y update

# Install Lua's dependencies for JWT
apt-get -y install git unzip lua5.2 liblua5.2 liblua5.2-dev luarocks
apt-get -y install libssl1.0-dev

# Install luacrypt as a Lua lib for JWT
luarocks install luacrypto # Is this working with current deps? Test.

# Install modified lua-cjson as a dep for luajwtjitsi
# How do I avoid doing this repeatedly on every script run?
pushd ~
rm -rf lua-cjson
git clone https://github.com/mpx/lua-cjson.git
cd lua-cjson
sed -i 's/lua_objlen/lua_rawlen/g' lua_cjson.c
sed -i 's|$(PREFIX)/include|/usr/include/lua5.2|g' Makefile
luarocks make
popd

# Need older version of luajwtjitsi (see: https://community.jitsi.org/t/error-while-starting-to-use-jitsi-with-jwt-cert/108556/9)
luarocks install luajwtjitsi 2.0-0

luarocks install basexx

# Asks application ID and application secret - this sets them in the prosody config
echo "jitsi-meet-tokens jitsi-meet-tokens/appid string  $JITSI_APP_ID" | debconf-set-selections
echo "jitsi-meet-tokens jitsi-meet-tokens/appsecret     password        $JITSI_APP_SECRET" | debconf-set-selections
apt install -y jitsi-meet-tokens

sed -i 's/authentication = "token"/authentication = "token"\n    allow_empty_token = false/' /etc/prosody/conf.avail/$FQDN_JITSI.cfg.lua

# Set up jicofo JWT auth in /etc/jitsi/jicofo/jicofo.conf?
# See: https://jitsi.github.io/handbook/docs/devops-guide/secure-domain
#jicofo {
#  authentication: {
#    enabled: true
#    type: JWT
#    login-url: jitsi-meet.example.com
# }

# token moderation plugin
# https://github.com/nvonahsen/jitsi-token-moderation-plugin

wget https://raw.githubusercontent.com/nvonahsen/jitsi-token-moderation-plugin/master/mod_token_moderation.lua
mv mod_token_moderation.lua /usr/share/jitsi-meet/prosody-plugins/

sed -i 's/"token_verification";/"token_verification";\n        "token_moderation";/g' /etc/prosody/conf.d/$FQDN_JITSI.cfg.lua
sed -i 's/"token_verification";/"token_verification";\n        "token_moderation";/g' /etc/prosody/conf.avail/$FQDN_JITSI.cfg.lua

systemctl restart jicofo
systemctl restart prosody
systemctl restart jitsi-videobridge2

####
# Finished
####
touch ~/standup_finished_successfully.txt
