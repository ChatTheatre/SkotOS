#!/bin/bash

set -e
set -x

DGD_PID=$(pgrep -f "dgd ./skotos.dgd")
if [ -z "$DGD_PID" ]
then
    echo "DGD does not appear to be running already. Good."
else
    echo "DGD appears to be running SkotOS already with PID ${DGD_PID}."
    kill "$DGD_PID"
fi

PID1=$(pgrep -f "listen=10801")
PID2=$(pgrep -f "listen=10802")

if [ -z "$PID1" ]
then
    echo "No Relay1 running. Good."
else
    echo "Shutting down Relay1 for port 10801->10443"
    kill "$PID1"
fi

if [ -z "$PID2" ]
then
    echo "No Relay2 running. Good."
else
    echo "Shutting down Relay2 for port 10802->10090"
    kill "$PID2"
fi

DGD_PID=$(pgrep -f "dgd ./skotos.dgd")
if [ -z "$DGD_PID" ]
then
    echo "DGD has already been stopped successfully or was not running."
else
    echo "Waiting for DGD to die before killing with -9"
    sleep 5
    kill -9 "$DGD_PID"
fi

echo "Shut down DGD successfully..."
