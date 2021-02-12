#!/bin/bash

set -e
set -x

DGD_PID=$(ps aux | grep "/var/dgd/bin/dgd skotos.dgd" | grep -v grep | cut -c 9-14)
if [ -z "$DGD_PID" ]
then
    echo "DGD does not appear to be running. Good."
else
    echo "DGD is running with PID ${DGD_PID}. Stopping."
    kill "$DGD_PID"
fi
