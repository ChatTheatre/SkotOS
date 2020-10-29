#!/bin/bash

set -e

# cd to the SkotOS root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ..

docker ps -a | grep sk && echo "You are already running a Docker container! Run stop_server.sh to remove it."

docker run --mount type=bind,src="$(pwd)/log",target="/var/log" --detach --name sk skotos

# TODO: go through the horrible osascript magic to open a new window (not just tab) consistently:
#    https://github.com/ohmyzsh/ohmyzsh/blob/master/plugins/osx/osx.plugin.zsh

# Open iTerm/terminal window showing DGD process log
open -a $TERM_PROGRAM dev_scripts/show_dgd_logs.sh

# TODO: wait until SkotOS is booted and responsive
while true
do
    sleep 5
    echo "Checking if SkotOS is responsive yet. The first run can take five to ten minutes until everything is working..."
    curl http://localhost:8080/SAM/Prop/Theatre:Web:Theatre/Index > /tmp/test_skotos.html || echo "(curl failed)"
    grep -i "gables" /tmp/test_skotos.html && break
done

open -a $TERM_PROGRAM dev_scripts/show_post_install_instructions.sh

open -a "Google Chrome" "http://localhost:8080/SAM/Prop/Theatre:Web:Theatre/Index"
# TODO: open iTerm/terminal window to telnet port
# TODO: open browser window to character creation



##################
# TODO: DockerHub image: https://docs.docker.com/get-started/part3/
# REF: https://docs.docker.com/storage/bind-mounts/
