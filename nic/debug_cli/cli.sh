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
CURR_DIR=`dirname $0`
ABS_CURR_DIR=`readlink -f $CURR_DIR`
NIC_DIR=`dirname $ABS_CURR_DIR`

cd $NIC_DIR/gen/iris/cli
./debug_hal_cli.sh
