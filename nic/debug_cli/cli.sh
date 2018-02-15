#!/bin/bash

#
# sets up python path for:
#   swig generated python files
#   grpc proto generated files
#   grpcio library
#   click library
#   click-repl library
#
# runs the penshell
#
if [ -z "$1" ]
then
    echo "Usage: ./cli.sh iris or ./cli.sh gft"
    exit
fi

CURR_DIR=`dirname $0`
ABS_CURR_DIR=`readlink -f $CURR_DIR`
NIC_DIR=`dirname $ABS_CURR_DIR`

cd $NIC_DIR/gen/$1/cli
./debug_hal_cli.sh
