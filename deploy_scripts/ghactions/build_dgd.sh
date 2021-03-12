#!/bin/bash

set -e
set -x

cd $GITHUB_WORKSPACE
git clone https://github.com/ChatTheatre/dgd ./dgd
cd dgd/src
make DEFINES='-DUINDEX_TYPE="unsigned int" -DUINDEX_MAX=UINT_MAX -DEINDEX_TYPE="unsigned short" -DEINDEX_MAX=USHRT_MAX -DSSIZET_TYPE="unsigned int" -DSSIZET_MAX=1048576' install
