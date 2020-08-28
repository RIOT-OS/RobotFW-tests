#!/bin/sh

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
python3 $SCRIPTPATH/env_parser.py -c -t -p -e
