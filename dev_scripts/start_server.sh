#!/bin/bash

set -e

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ..

if (docker ps -a | grep sk)
then
    # Restart the stopped container, if it's stopped
    docker start sk || echo "Maybe the Docker container is already running?"
else
    # Create a new container
    docker run -p 10800:10800 -p 10900:10900 -p 10802:10802 -p 10080:10080 -p 10443:10443 --mount type=bind,src="$(pwd)/log",target="/var/log" --detach --name sk skotos
fi
docker ps -a | grep sk && echo "You are already running a Docker container! Run stop_server.sh to remove it."


# TODO: go through the horrible osascript magic to open a new window (not just tab) consistently:
#    https://github.com/ohmyzsh/ohmyzsh/blob/master/plugins/osx/osx.plugin.zsh

# Open iTerm/terminal window showing DGD process log
open -a $TERM_PROGRAM dev_scripts/show_dgd_logs.sh

# Wait until SkotOS is booted and responsive
while true
do
    sleep 5
    echo "Checking if SkotOS is responsive yet. The first run can take five to ten minutes until everything is working..."
    curl -L http://localhost:10080/SAM/Prop/Theatre:Web:Theatre/Index > /tmp/test_skotos.html || echo "(curl failed)"
    grep "Skotos" /tmp/test_skotos.html && break
done

cat dev_scripts/post_install_instructions.txt

open -a "Google Chrome" "http://localhost:10080/SAM/Prop/Theatre:Web:Theatre/Index"
# TODO: open iTerm/terminal window to telnet port


##################
# TODO: DockerHub image: https://docs.docker.com/get-started/part3/
# REF: https://docs.docker.com/storage/bind-mounts/
