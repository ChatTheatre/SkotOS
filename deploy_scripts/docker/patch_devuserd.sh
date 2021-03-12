#!/bin/bash

set -e
set -x

DEVUSERD=skoot/usr/System/sys/devuserd.c
if grep -F "user_to_hash = ([ ])" $DEVUSERD
then
    # Unpatched - need to patch

    #ruby -n -e 'puts $_.gsub("user_to_hash = ([ ])", "user_to_hash = ([ \"admin\": to_hex(hash_md5(\"adminpw\")), \"bobo\": to_hex(hash_md5(\"bobopw\")) ])")' < skoot/usr/System/sys/devuserd.c
    sed 's/user_to_hash = (\[ \]);/user_to_hash = ([ "admin": to_hex(hash_md5("admin" + "adminpwd")), "bobo": to_hex(hash_md5("bobo" + "bobopwd")) ]);/g' < $DEVUSERD > /tmp/d2.c
    mv /tmp/d2.c $DEVUSERD
else
    echo "DevUserD appears to be patched already. Moving on..."
fi
