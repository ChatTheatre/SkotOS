#!/bin/bash

set -e

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ..

SKOTOS_ROOT="$(pwd)"

if [ -f /Applications/Docker.app ]
then
    echo "Docker already installed, continuing..."
else
    echo "You'll need to install Docker Desktop. Please download the DMG file from this page, open it, and copy Docker into applications."
    echo "        https://hub.docker.com/editions/community/docker-ce-desktop-mac/"

    exit
fi

# Install prereqs: Homebrew
which brew || /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
brew install telnet

./dev_scripts/start_server.sh
