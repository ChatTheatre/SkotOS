#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR
cd ../..

tail -f $(pwd)/log/dgd_server.out $(pwd)/log/wafer_log.txt
