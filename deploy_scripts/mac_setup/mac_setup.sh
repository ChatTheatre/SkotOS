#!/bin/bash

set -e
set -x

# Set PORTBASE to 11 if unset
export PORTBASE=${PORTBASE:-11}

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ../..
SKOTOS_ROOT="$(pwd)"

echo "-------------------------"
echo "THIS SCRIPT IS OBSOLETE."
echo "YOU SHOULD BE RUNNING THE SETUP SCRIPT FROM A SKOTOS-BASED APP SUCH AS THE GABLES."
echo "THIS SCRIPT IS DEPRECATED AND WILL STOP RUNNING AT SOME FUTURE POINT."
echo "The setup_no_server script is what you want for setting up everything except the DGD process."
echo "-------------------------"

export GAME_ROOT="$SKOTOS_ROOT"

./deploy_scripts/mac_setup/setup_no_server.sh
./deploy_scripts/mac_setup/start_server.sh
