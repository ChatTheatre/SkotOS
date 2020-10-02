#!/bin/bash

set -e

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ..

# This script intends to set up your Mac for SkotOS development. It's as useful to read through
# as to actually run. It should be runnable, especially on the first invocation.

# It should run from the SkotOS root directory.



# Set up patched DGD
if [ -d ../dgd ]
then
    echo "DGD Exists"
else
    echo "Cloning DGD"
    git clone git@github.com:ChatTheatre/dgd.git ../dgd
fi

pushd ../dgd/src
if grep "unsigned short ssizet" config.h
then
    echo "DGD is unpatched..."
    sed 's/typedef unsigned short ssizet/typedef unsigned int ssizet/' < config.h > config2.h
    sed 's/# define SSIZET_MAX.*USHRT_MAX/# define SSIZET_MAX UINT_MAX/' < config2.h > config3.h
    sed 's/MAX_STRLEN.*SSIZET_MAX/MAX_STRLEN 1048576/' < config3.h > config.h
    rm config2.h config3.h
else
    echo "DGD is patched, I think."
fi

make DEFINES='-DUINDEX_TYPE="unsigned int" -DUINDEX_MAX=UINT_MAX -DEINDEX_TYPE="unsigned short" -DEINDEX_MAX=USHRT_MAX' install
popd

# Clone other ChatTheatre repos

if [ -d ../orchil ]
then
    echo "Orchil exists"
else
    echo "Cloning Orchil"
    git clone git@github.com:ChatTheatre/orchil.git ../orchil
fi

if [ -d ../websocket-to-tcp-tunnel ]
then
    echo "Tunnel exists"
else
    echo "Cloning Tunnel (websocket-to-tcp-tunnel)"
    git clone git@github.com:ChatTheatre/websocket-to-tcp-tunnel.git ../websocket-to-tcp-tunnel
fi

pushd ../websocket-to-tcp-tunnel
npm install
popd

echo "Copying Orchil config file..."
cp dev_scripts/orchil_profiles.js ../orchil/profiles.js

echo "NGinX configuration..."
cp dev_scripts/dev_nginx.conf /usr/local/etc/nginx/servers/skotos_dev_nginx.conf
brew install nginx || echo "NGinX already installed is fine..."

echo "Testing that NGinX config will load..."
sudo nginx -t

cat dev_scripts/post_install_instructions.txt
