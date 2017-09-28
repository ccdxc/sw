#! /bin/bash
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`

export HAL_CONFIG_PATH=$NIC_DIR/conf/
CAPRI_MOCK_MODE=1 $NIC_DIR/../bazel-bin/nic/hal/hal -c hal.json 2>&1 | tee $NIC_DIR/hal.log
