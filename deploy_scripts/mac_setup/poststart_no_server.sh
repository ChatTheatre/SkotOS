#!/bin/bash

set -e
set -x

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ../..
SKOTOS_ROOT="$(pwd)"

if [ -z "$GAME_ROOT" ]
then
    echo "You must set a GAME_ROOT to run this startup script."
    exit -1
fi
cd "$GAME_ROOT"

# Wait until SkotOS is booted and responsive
"$SKOTOS_ROOT/deploy_scripts/shared/wait_for_full_boot.sh"

echo "Running Wafer (auth server)"
pushd wafer
bundle
bundle exec ruby -Ilib ./exe/wafer >../log/wafer_log.txt --settings-file "$SKOTOS_ROOT/deploy_scripts/mac_setup/wafer_settings.json" 2>&1 &
popd
