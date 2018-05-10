#!/bin/bash
SW_DIR=../..
NIC_DIR=$SW_DIR/nic

if [ $# -ne 1 ]
then
    echo "Usage: $0 <iris/gft>"
    exit 1
fi

export TBL_DIR=$NIC_DIR/gen

# dependent modules for python
export PYTHONPATH=$PYTHONPATH:$NIC_DIR/gen/proto/hal:$NIC_DIR/gen/$1/cli:$NIC_DIR/gen/common_txdma_actions/cli:$NIC_DIR/gen/common_rxdma_actions/cli:$NIC_DIR/gen/common/cli:.

# dependent shared libs
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/gen/x86_64/lib:.

# start the debug CLI prompt
if [ $1 == 'iris' ]
then
    python3 cli/cli_main.py repl
else
    python3 cli/cli_main_gft.py repl
fi
