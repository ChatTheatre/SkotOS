#!/bin/bash

if ps aux | grep deploy_scripts/stackscript/userdb-authctl | grep -v grep
then
	echo "userdb-authctl is already running"
else
	echo "userdb-authctl not running - restarting"
	/usr/bin/perl /var/skotos/deploy_scripts/stackscript/userdb-authctl | tee -a /var/log/userdb/authctl.txt &
fi
