#!/bin/bash

# TODO: Do we stop NGinX? Interesting question.

DGD_PID=$(ps aux | grep "dgd ./skotos.dgd" | grep -v grep | cut -c 14-22)
if [ -z "$DGD_PID" ]
then
    echo "DGD does not appear to be running already. Good."
else
    echo "DGD appears to be running SkotOS already with PID ${DGD_PID}."
    kill "$DGD_PID"
fi

PID1=$(ps aux | grep "listen=10801" | grep -v grep | cut -c 14-22)
PID2=$(ps aux | grep "listen=10802" | grep -v grep | cut -c 14-22)

if [ -z "$PID1" ]
then
    echo "No Relay1 running. Good."
else
    echo "Shutting down Relay1 =for port 10801->10443"
    kill "$PID1"
fi

if [ -z "$PID2" ]
then
    echo "No Relay2 running. Good."
else
    echo "Shutting down Relay2 =for port 10802->10090"
    kill "$PID2"
fi

DGD_PID=$(ps aux | grep "dgd ./skotos.dgd" | grep -v grep | cut -c 14-22)
if [ -z "$DGD_PID" ]
then
    echo "DGD has already been stopped successfully or was not running."
else
    echo "Waiting for DGD to die before killing with -9"
    sleep 5
    kill -9 "$DGD_PID"
fi

echo "Shut down DGD successfully..."
