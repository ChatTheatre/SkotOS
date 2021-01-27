#!/bin/bash

if ps aux | grep userdb-authctl | grep -v grep
then
	echo "userdb-authctl is already running"
else
	echo "userdb-authctl not running - restarting"
	/usr/bin/perl /var/skotos/dev_scripts/stackscript/userdb-authctl >/var/log/userdb-authctl.txt &
fi
