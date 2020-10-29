#!/bin/bash

set -e

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ..

DOCKER_TAG="noahgibbs/skotos:earliest"

if (docker ps -a | grep sk)
then
    # Restart the stopped container, if it's stopped
    docker start sk || echo "Maybe the Docker container is already running?"
else
    # Create a new container
    docker pull noahgibbs/skotos
    docker run -p 10800:10800 -p 10900:10900 -p 10802:10802 -p 10080:10080 -p 10443:10443 -p 10098:10098 -p 10099:10099 --mount type=bind,src="$(pwd)/log",target="/var/log" --detach --name sk noahgibbs/skotos
fi
docker ps -a | grep sk && echo "You are already running a Docker container! Run stop_server.sh to remove it."


# TODO: go through the horrible osascript magic to open a new window (not just tab) consistently:
#    https://github.com/ohmyzsh/ohmyzsh/blob/master/plugins/osx/osx.plugin.zsh

# Open iTerm/terminal window showing DGD process log
open -a Terminal -n dev_scripts/show_dgd_logs.sh

# Wait until SkotOS is booted and responsive
while true
do
    sleep 5
    echo "Checking if SkotOS is responsive yet. The first run can take five to ten minutes until everything is working..."
    curl -L http://localhost:10080/SAM/Prop/Theatre:Web:Theatre/Index > /tmp/test_skotos.html || echo "(curl failed)"
    # Note: can sometimes get a "theatre not found" transient error early in bootup
    grep "Log in to Skotos" /tmp/test_skotos.html && break
done

cat dev_scripts/post_install_instructions.txt

open -a "Google Chrome" "http://localhost:10080/SAM/Prop/Theatre:Web:Theatre/Index"
#open -a Terminal -n "telnet localhost 10098"
# TODO: open iTerm/terminal window to telnet port
