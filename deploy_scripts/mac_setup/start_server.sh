#!/bin/bash

set -e
set -x

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ../..

# Start websocket-to-tcp tunnels for Orchil client and for Tree of WOE
PID1=$(pgrep -f "listen=10801") || echo "Relay1 not yet running, which is fine"
PID2=$(pgrep -f "listen=10802") || echo "Relay2 not yet running, which is fine"

if [ -z "$PID1" ]
then
    echo "Running Relay.js for port 10801->10443"
    pushd websocket-to-tcp-tunnel
    nohup node src/Relay.js --listen=10801 --send=10443 --host=localhost --name=gables --wsHeartbeat=30 --shutdownDelay=3 --tunnelInfo=false &
    popd
else
    echo "Relay is already running for port 10801->10443"
fi

if [ -z "$PID2" ]
then
    echo "Running Relay.js for port 10802->10090"
    pushd websocket-to-tcp-tunnel
    nohup node src/Relay.js --listen=10802 --send=10090 --host=localhost --name=gables --wsHeartbeat=30 --shutdownDelay=3 --tunnelInfo=false &
    popd
else
    echo "Relay is already running for port 10802->10090"
fi

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
open -a Terminal -n deploy_scripts/mac_setup/show_dgd_logs.sh

# Wait until SkotOS is booted and responsive
./deploy_scripts/shared/wait_for_full_boot.sh

WAFER_PID=$(pgrep -f "ruby -Ilib ./exe/wafer") || echo "Wafer not yet running, which is fine"
if [ -z "$WAFER_PID" ]
then
    echo "Running Wafer (auth server)"
    pushd wafer
    ruby -Ilib ./exe/wafer >../log/wafer_log.txt 2>&1 &
    popd
else
    echo "Wafer is already running with PID $WAFER_PID"
fi

cat deploy_scripts/mac_setup/post_install_instructions.txt

open -a "Google Chrome" "http://localhost:2072/"
#open -a Terminal -n "telnet localhost 10098"
# TODO: open iTerm/terminal window to telnet port
