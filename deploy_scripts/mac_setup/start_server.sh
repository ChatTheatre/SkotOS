#!/bin/bash

# This is an example start script, similar to the ones other SkotOS games
# should supply. See https://github.com/ChatTheatre/gables_game for an
# up-to-date example.

# A heavily-customised game may need to stop using the standard startup and
# shutdown scripts. But if you *can* use them it can save you some headache.

set -e
set -x

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ../..
export GAME_ROOT="$(pwd)"

./deploy_scripts/mac_setup/prestart_no_server.sh

DGD_PID=$(pgrep -f "dgd ./skotos.dgd") || echo "DGD not yet running, which is fine"
if [ -z "$DGD_PID" ]
then
    if [ -f skotos.database ]
    then
        echo "Hot-booting DGD from existing statedump..."
        dgd/bin/dgd ./skotos.dgd skotos.database >log/dgd_server.out 2>&1 &
    else
        echo "Cold-booting DGD with no statedump..."
        dgd/bin/dgd ./skotos.dgd >log/dgd_server.out 2>&1 &
    fi
else
    echo "DGD is already running! We'll let it keep doing that."
fi

# Open iTerm/terminal window showing DGD process log
deploy_scripts/mac_setup/new_terminal.sh deploy_scripts/mac_setup/show_dgd_logs.sh

# Wait until SkotOS is booted and responsive, start auth server
./deploy_scripts/mac_setup/poststart_no_server.sh

cat ./deploy_scripts/mac_setup/post_install_instructions.txt

open -a "Google Chrome" "http://localhost:2072/"
#open -a Terminal -n "telnet localhost 10098"
# TODO: open iTerm/terminal window to telnet port
