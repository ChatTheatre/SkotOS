#!/bin/bash

set -e

cd /var/skotos

if [ -f no_restart.txt ]
then
	echo "/var/skotos/no_restart.txt file detected - not restarting"
	exit
fi

if [ -f skotos.database ]
then
    SKOTOS_CMD="/var/dgd/bin/dgd skotos.dgd skotos.database"
else
    SKOTOS_CMD="/var/dgd/bin/dgd skotos.dgd"
fi

if ps aux | grep "/var/dgd/bin/dgd skotos.dgd" | grep -v grep
then
	echo "SkotOS DGD server is already running"
else
	echo "SkotOS DGD server is not running - restarting"
	$SKOTOS_CMD >>/var/log/dgd/server.out 2>&1 &
fi
