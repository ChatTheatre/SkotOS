#!/bin/bash

set -e
set -x

# Set PORTBASE to 11 if unset
export PORTBASE=${PORTBASE:-11}

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
PID1=$(pgrep -f "listen=${PORTBASE}801") || echo "Relay1 not yet running, which is fine"
PID2=$(pgrep -f "listen=${PORTBASE}802") || echo "Relay2 not yet running, which is fine"

if [ -z "$PID1" ]
then
    echo "Running Relay.js for port ${PORTBASE}801->${PORTBASE}443"
    pushd websocket-to-tcp-tunnel
    nohup node src/Relay.js --listen=${PORTBASE}801 --send=${PORTBASE}443 --host=localhost --name=gables --wsHeartbeat=30 --shutdownDelay=3 --tunnelInfo=false &
    popd
else
    echo "Relay is already running for port ${PORTBASE}801->${PORTBASE}443"
fi

if [ -z "$PID2" ]
then
    echo "Running Relay.js for port ${PORTBASE}802->${PORTBASE}090"
    pushd websocket-to-tcp-tunnel
    nohup node src/Relay.js --listen=${PORTBASE}802 --send=${PORTBASE}090 --host=localhost --name=gables --wsHeartbeat=30 --shutdownDelay=3 --tunnelInfo=false &
    popd
else
    echo "Relay is already running for port ${PORTBASE}802->${PORTBASE}090"
fi
