#!/bin/bash

set -e

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ..

DGD_PID=$(ps aux | grep "dgd ./skotos.dgd" | grep -v grep | cut -c 14-25)
if [ -z "$DGD_PID" ]
then
    echo "DGD does not appear to be running already. Good."
else
    echo "DGD appears to be running SkotOS already with PID ${DGD_PID}. Shut down that copy of DGD before running another."
    false
fi

# Start websocket-to-tcp tunnels for Orchil client and for Tree of WOE

PID1=$(ps aux | grep "listen=10801" | grep -v grep | cut -c 14-25)
PID2=$(ps aux | grep "listen=10802" | grep -v grep | cut -c 14-25)

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

# Run NGinX to forward to websocket relays

NG_PID=$(ps aux | grep "nginx: master process" | grep -v grep | cut -c 14-25)

if [ -z "$NG_PID" ]
then
    sudo nginx
else
    echo "NGinX is already running... Reloading config."
    sudo nginx -s reload
fi

cat dev_scripts/post_install_instructions.txt

if [ -f skotos.database ]
then
    echo "Hot-booting DGD from existing statedump..."
    dgd/bin/dgd ./skotos.dgd skotos.database
else
    echo "Cold-booting DGD with no statedump..."
    dgd/bin/dgd ./skotos.dgd
fi
