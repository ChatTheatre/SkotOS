#!/bin/bash

set -e

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ../..

DGD_PID=$(ps aux | grep "dgd ./skotos.dgd" | grep -v grep | cut -c 14-22)
if [ -z "$DGD_PID" ]
then
    echo "DGD does not appear to be running already. Good."
else
    echo "DGD appears to be running SkotOS already with PID ${DGD_PID}. Shut down that copy of DGD before running another."
    false
fi

# Start websocket-to-tcp tunnels for Orchil client and for Tree of WOE

PID1=$(ps aux | grep "listen=10801" | grep -v grep | cut -c 14-22)
PID2=$(ps aux | grep "listen=10802" | grep -v grep | cut -c 14-22)

if [ -z "$PID1" ]
then
    echo "Running Relay.js for port 10801->10443"
    pushd websocket-to-tcp-tunnel
    nohup node src/Relay.js --listen=10801 --send=10443 --host=localhost --name=gables --wsHeartbeat=30 --shutdownDelay=3 --tunnelInfo=false &
    popd
else
    echo "Relay is already running for port 10801->10443"
fi

if [ -z "$PID2" ]
then
    echo "Running Relay.js for port 10802->10090"
    pushd websocket-to-tcp-tunnel
    nohup node src/Relay.js --listen=10802 --send=10090 --host=localhost --name=gables --wsHeartbeat=30 --shutdownDelay=3 --tunnelInfo=false &
    popd
else
    echo "Relay is already running for port 10802->10090"
fi

if [ -f skotos.database ]
then
    echo "Hot-booting DGD from existing statedump..."
    dgd/bin/dgd ./skotos.dgd skotos.database >log/dgd_server.out 2>&1 &
else
    echo "Cold-booting DGD with no statedump..."
    dgd/bin/dgd ./skotos.dgd >log/dgd_server.out 2>&1 &
fi
# Open iTerm/terminal window showing DGD process log
open -a Terminal -n deploy_scripts/mac_setup/show_dgd_logs.sh

# Wait until SkotOS is booted and responsive
sleep 5
while true
do
    echo "Checking if SkotOS is responsive yet. The first run can take five to ten minutes until everything is working..."
    curl -L http://localhost:10080/SAM/Prop/Theatre:Web:Theatre/Index > /tmp/test_skotos.html || echo "(curl failed)"
    # Note: can sometimes get a "theatre not found" transient error early in bootup
    grep "Log in to Skotos" /tmp/test_skotos.html && break
    sleep 20
done

cat deploy_scripts/mac_setup/post_install_instructions.txt

open -a "Google Chrome" "http://localhost:10080/SAM/Prop/Theatre:Web:Theatre/Index"
#open -a Terminal -n "telnet localhost 10098"
# TODO: open iTerm/terminal window to telnet port
