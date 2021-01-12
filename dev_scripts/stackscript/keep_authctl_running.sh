#!/bin/bash

ps aux | grep userdb-authctl | grep -v grep || /usr/bin/perl /var/skotos/dev_scripts/stackscript/userdb-authctl
