#!/bin/bash
SW_DIR=../..
NIC_DIR=$SW_DIR/nic

usage() {
        echo "Usage: $0 <iris/gft/'p4plusraw <program name>'>"
}

if [ $# -lt 1 ]
then
    usage
    exit 1
fi

# dependent modules for python
export PYTHONPATH=$PYTHONPATH:$NIC_DIR/gen/proto/hal:$NIC_DIR/gen/$1/cli:$NIC_DIR/gen/common_txdma_actions/cli:$NIC_DIR/gen/common_rxdma_actions/cli:$NIC_DIR/gen/common/cli:.

# dependent shared libs
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/gen/x86_64/lib:.

# start the debug CLI prompt
if [ "$1" = "iris" ] || [ "$1" = "" ]; then
    python3 cli/debug_cli_iris.py repl
elif [ "$1" = "gft" ]; then
    python3 cli/debug_cli_gft.py repl
elif [ "$1" = "p4plusraw" ]; then
    if [ $# -ne 2 ]; then
        usage
        exit 1
    fi
    P4PLUS_MOD_PATH=$NIC_DIR/gen/$2
    if [ ! -d $P4PLUS_MOD_PATH ]; then
        echo "Invalid P4Plus program name:" $2
        exit 1
    fi
    # Using env since click-repl prevents the module name from being passed as a command line
    export P4PLUS_MOD=$2

    # base CLI has dependencies on iris_backend
    export PYTHONPATH=$PYTHONPATH:$NIC_DIR/gen/iris/cli:$NIC_DIR/gen/
    python3 cli/debug_cli_p4plus.py repl
else
    echo "Unknown pipeline $1"
fi
