#!/bin/bash

if [ -z "$GAME_ROOT" ]
then
    echo "You must set a GAME_ROOT to run this setup script."
    exit -1
fi

tail -f "$GAME_ROOT/log/dgd_server.out" "$GAME_ROOT/log/wafer_log.txt"
