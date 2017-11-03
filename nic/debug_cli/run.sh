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

export PYTHONPATH=swig/pyfiles:../gen/proto/hal:../third-party/lib64/python2.7:$PYTHONPATH
export GRPC_SERVER="localhost:50054"
python penshell_main.py repl
