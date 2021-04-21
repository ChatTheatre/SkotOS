#!/bin/bash

set -e
set -x

if [ -z "$GAME_ROOT" ]
then
    echo "You must set a GAME_ROOT to run this startup script."
    exit -1
fi
cd "$GAME_ROOT"

WAFER_PID=$(pgrep -f "rackup -p 2072") || echo "Wafer's web server not yet running, which is fine"
if [ -z "$WAFER_PID" ]
then
    echo "Wafer's web server is not running - good, it can get wedged."
else
    kill $WAFER_PID # Error on first kill wouldn't be good
    sleep 0.5
    kill -9 $WAFER_PID || echo "Error on kill -9, that means Wafer exited gracefully"
    rm -f log/wafer_log.txt
fi

WAFER_PID=$(pgrep -f "ruby -Ilib ./exe/wafer") || echo "Wafer wrapper not running, which is fine"
if [ -z "$WAFER_PID" ]
then
    echo "Wafer wrapper is not yet running - good, it can get wedged."
else
    kill -9 $WAFER_PID
    rm -f log/wafer_log.txt
fi

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
