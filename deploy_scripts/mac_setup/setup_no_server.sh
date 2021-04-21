#!/bin/bash

set -e
set -x

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ../..
SKOTOS_ROOT="$(pwd)"

if [ -z "$GAME_ROOT" ]
then
    echo "You must set a GAME_ROOT to run this setup script."
    exit -1
fi

cd "$GAME_ROOT"

DGD_PID=$(pgrep -f "dgd/bin/dgd") || echo "DGD not running, which is fine."
if [ -z "$DGD_PID" ]
then
    echo "DGD does not appear to be running. Good."
else
    echo "DGD appears to be running SkotOS already with PID ${DGD_PID}. Shut down this copy of DGD with deploy_scripts/mac_setup/stop_server.sh before messing with the install."
    echo "Or you can run start_server.sh instead of mac_setup.sh. That's fine too."
    false
fi

# This script intends to set up your Mac for SkotOS development. It's as useful to read through
# as to actually run.

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
    echo "DGD exists... Updating."
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

if [ -d websocket-to-tcp-tunnel ]
then
    echo "Tunnel exists... Updating."
    pushd websocket-to-tcp-tunnel
    git pull
    popd
else
    echo "Cloning Tunnel (websocket-to-tcp-tunnel)"
    git clone git@github.com:ChatTheatre/websocket-to-tcp-tunnel.git websocket-to-tcp-tunnel
fi

if [ -d wafer ]
then
    echo "Clone of Wafer repo exists... Updating."
    pushd wafer
    git pull
    popd
else
    echo "Cloning Wafer (dev-mode fake UserDB)"
    git clone git@github.com:ChatTheatre/wafer.git wafer
fi

pushd websocket-to-tcp-tunnel
npm install || echo "Allowing even with outstanding 'npm audit' issues..."
popd
