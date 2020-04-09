#! /bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
export ASIC="${ASIC:-capri}"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/build/x86_64/iris/${ASIC}/lib
cd /sw || exit 1
make pull-assets || exit 1
cd nic || exit 1
make bazel-build || exit 1
make bazel-gtests || exit 1
make run-gtests || exit 1
