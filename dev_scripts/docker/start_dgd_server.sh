#!/bin/bash

set -e

# TODO: run tunnels and DGD via supervisord (apt package name: supervisor)

nginx || echo "NGinX already running?"
nginx -t # Make sure NGinX config is correct
nginx -s reload  # Make sure NGinX is responding

# Make sure tunnels are running, at least initially
/usr/local/websocket-to-tcp-tunnel/search-tunnel.sh

cd /var/skotos

if [ -f skotos.database ]
then
    SKOTOS_CMD="/var/dgd/bin/dgd skotos.dgd skotos.database"
else
    SKOTOS_CMD="/var/dgd/bin/dgd skotos.dgd"
fi

$SKOTOS_CMD >/var/log/dgd_server.out 2>&1
