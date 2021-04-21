#!/bin/bash

set -e
set -x

if [ -z "$GAME_ROOT" ]
then
    echo "You must set a GAME_ROOT to run this startup script."
    exit -1
fi
cd "$GAME_ROOT"

# Wait until SkotOS is booted and responsive
./deploy_scripts/shared/wait_for_full_boot.sh

# We killed Wafer up-top, so we can just run it here.
echo "Running Wafer (auth server)"
pushd wafer
ruby -Ilib ./exe/wafer >../log/wafer_log.txt 2>&1 &
popd
