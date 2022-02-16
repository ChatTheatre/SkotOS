#!/bin/bash

set -x

# Wait until SkotOS is booted and responsive
sleep 5
while true
do
    echo "Checking if SkotOS is responsive yet. The first run can take five to ten minutes until everything is working. Curl should get a 404 (not found) from this URL until fully booted, then a 200."
    curl -v -L --fail http://localhost:11080/FullyBooted > /tmp/test_skotos.html
    if [ $? -eq 0 ]; then
        break
    fi
    sleep 20
done
