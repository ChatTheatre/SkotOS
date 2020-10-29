#!/bin/bash

set -e

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ..

docker ps -a | grep sk && echo "You are already running a Docker container! Run stop_server.sh to remove it."

docker run -p 8080:8080 --mount type=bind,src="$(pwd)/log",target="/var/log" --detach --name sk skotos

# TODO: go through the horrible osascript magic to open a new window (not just tab) consistently:
#    https://github.com/ohmyzsh/ohmyzsh/blob/master/plugins/osx/osx.plugin.zsh

# Open iTerm/terminal window showing DGD process log
open -a $TERM_PROGRAM dev_scripts/show_dgd_logs.sh

# TODO: wait until SkotOS is booted and responsive
while true
do
    sleep 5
    echo "Checking if SkotOS is responsive yet. The first run can take awhile..."
    curl https://localhost:8080/SAM/Prop/Theatre:Web:Theatre/Index || echo "(curl failed)"
    grep "" Index && break
done

open -a $TERM_PROGRAM dev_scripts/show_post_install_instructions.sh


# TODO: open iTerm/terminal window to telnet port
# TODO: open browser window to character creation



##################
# TODO: DockerHub image: https://docs.docker.com/get-started/part3/
# REF: https://docs.docker.com/storage/bind-mounts/
