#!/bin/bash

set -e
set -x

DGD_PID=$(pgrep -f "bin/dgd")
if [ -z "$DGD_PID" ]
then
    echo "DGD does not appear to be running. Good."
else
    echo "DGD is running with PID ${DGD_PID}. Stopping."
    kill "$DGD_PID"
fi
