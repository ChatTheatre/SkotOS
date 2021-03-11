#!/bin/bash

set -e

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ../..

SKOTOS_ROOT="$(pwd)"

DGD_PID=$(ps aux | grep "dgd ./skotos.dgd" | grep -v grep | cut -c 14-25)
if [ -z "$DGD_PID" ]
then
    echo "DGD does not appear to be running. Good."
else
    echo "DGD appears to be running SkotOS already with PID ${DGD_PID}. Shut down this copy of DGD with dev_scripts/mac_setup/stop_server.sh before messing with the install."
    false
fi

# Patch SkotOS devuserd.c to add a dev user...

DEVUSERD=skoot/usr/System/sys/devuserd.c
if grep -F "user_to_hash = ([ ])" $DEVUSERD
then
    # Unpatched - need to patch

    #ruby -n -e 'puts $_.gsub("user_to_hash = ([ ])", "user_to_hash = ([ \"admin\": to_hex(hash_md5(\"adminpw\")), \"bobo\": to_hex(hash_md5(\"bobopw\")) ])")' < skoot/usr/System/sys/devuserd.c
    sed 's/user_to_hash = (\[ \]);/user_to_hash = ([ "admin": to_hex(hash_md5("admin" + "adminpwd")), "bobo": to_hex(hash_md5("bobo" + "bobopwd")) ]);/g' < $DEVUSERD > /tmp/d2.c
    mv /tmp/d2.c $DEVUSERD
else
    echo "DevUserD appears to be patched already. Moving on..."
fi

# This script intends to set up your Mac for SkotOS development. It's as useful to read through
# as to actually run. It should be runnable, especially on the first invocation.

# It should run from the SkotOS root directory.

# Install prereqs: Homebrew
which brew || /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
which git || brew install git
which npm || brew install npm

if [ -f ~/.ssh/id_rsa.pub ]
then
    echo "Yes, you have an ~/.ssh/id_rsa.pub"
else
    # Generate the key and add it to the Mac keychain
    echo "You have no RSA public key. You'll need to create one and register it with GitHub! I recommend an empty passphrase for convenience."
    ssh-keygen -t rsa -f ~/.ssh/id_rsa
    ssh-add -K ~/.ssh/id_rsa
fi

if git clone git@github.com:noahgibbs/env_mem.git /tmp/env_mem
then
    rm -rf /tmp/env_mem
    echo "Git clone succeeded - you can clone repos from GitHub."
else
    echo "You can't check out a repo on GitHub. That probably means you need to register your SSH key with GitHub. Please check https://github.com/settings/keys and add ~/.ssh/id_rsa.pub so that you can check out repositories."
    false  # Script will exit with error due to "set -e" above
fi

# Set up DGD
if [ -d dgd ]
then
    echo "DGD Exists"
    pushd dgd
    git pull
    popd
else
    echo "Cloning DGD"
    git clone git@github.com:ChatTheatre/dgd.git dgd
fi

pushd dgd/src
make DEFINES='-DUINDEX_TYPE="unsigned int" -DUINDEX_MAX=UINT_MAX -DEINDEX_TYPE="unsigned short" -DEINDEX_MAX=USHRT_MAX -DSSIZET_TYPE="unsigned int" -DSSIZET_MAX=1048576' install
popd

# Clone other ChatTheatre repos

if [ -d orchil ]
then
    echo "Orchil exists, pulling..."
    pushd orchil
    git pull
    popd
else
    echo "Cloning Orchil"
    git clone git@github.com:ChatTheatre/orchil.git orchil
fi

if [ -d websocket-to-tcp-tunnel ]
then
    echo "Tunnel exists"
    pushd websocket-to-tcp-tunnel
    git pull
    popd
else
    echo "Cloning Tunnel (websocket-to-tcp-tunnel)"
    git clone git@github.com:ChatTheatre/websocket-to-tcp-tunnel.git websocket-to-tcp-tunnel
fi

pushd websocket-to-tcp-tunnel
npm install
popd

./dev_scripts/mac_setup/start_server.sh

