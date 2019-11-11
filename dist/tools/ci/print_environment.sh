#!/bin/sh

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
python3 $SCRIPTPATH/print_python_versions.py
bash $SCRIPTPATH/../../../RIOT/dist/tools/ci/print_toolchain_versions.sh
