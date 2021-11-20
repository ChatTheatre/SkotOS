#!/bin/bash

# Set PORTBASE to 11 if unset
export PORTBASE=${PORTBASE:-11}

WAFER_PID=$(pgrep -f "ruby -Ilib ./exe/wafer") || echo "Wafer not running, which is fine"
if [ -z "$WAFER_PID" ]
then
    echo "Wafer does not appear to be running. Good."
else
    kill -9 "$WAFER_PID"
fi

# But Wafer also runs a sub-process which can get orphaned. Check for it.
WAFER_PID=$(pgrep -f "rackup -p 2072") || echo "Wafer's web server not running, which is fine"
if [ -z "$WAFER_PID" ]
then
    echo "Wafer's web server does not appear to be running. Good."
else
    kill -9 "$WAFER_PID"
fi

if [ -z "$DGD_PID" ]
then
    echo "DGD does not appear to be running. Good."
else
    echo "DGD appears to be running SkotOS already with PID ${DGD_PID}."
    kill "$DGD_PID"
fi

PID1=$(pgrep -f "listen=${PORTBASE}801") || echo "Relay1 not running, which is fine"
PID2=$(pgrep -f "listen=${PORTBASE}802") || echo "Relay2 not running, which is fine"

if [ -z "$PID1" ]
then
    echo "No Relay1 running. Good."
else
    echo "Shutting down Relay1 for port ${PORTBASE}801->${PORTBASE}443"
    kill "$PID1"
fi

if [ -z "$PID2" ]
then
    echo "No Relay2 running. Good."
else
    echo "Shutting down Relay2 for port ${PORTBASE}802->${PORTBASE}090"
    kill "$PID2"
fi

DGD_PID=$(pgrep -f "dgd ./skotos.dgd") || echo "DGD not running, which is fine"
if [ -z "$DGD_PID" ]
then
    echo "DGD has already been stopped successfully or was not running."
else
    echo "Waiting for DGD to die before killing with -9"
    sleep 5
    kill -9 "$DGD_PID" || echo "DGD had already stopped."
fi

echo "Shut down DGD successfully."
