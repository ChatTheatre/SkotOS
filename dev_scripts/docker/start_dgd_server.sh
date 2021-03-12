#!/bin/bash

set -e
set -x

cd /var/skotos

if [ -f skotos.database ]
then
    SKOTOS_CMD="/var/dgd/bin/dgd skotos.dgd skotos.database"
else
    SKOTOS_CMD="/var/dgd/bin/dgd skotos.dgd"
fi

$SKOTOS_CMD >/var/log/dgd_server.out 2>&1
