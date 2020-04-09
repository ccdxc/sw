#! /bin/bash -e
export ASIC="${ASIC:-capri}"
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`

export ZMQ_SOC_DIR=$NIC_DIR
export HAL_CONFIG_PATH=$NIC_DIR/conf/
rm -f $NIC_DIR/linkmgr.log

catalog_file=catalog.json

if [ $# -eq 1 ]; then
    if [ $1 == "serdes_test_board" ]; then
        catalog_file=catalog_serdes_test_board.json
    fi
fi

echo "catalog: $catalog_file"

$GDB $NIC_DIR/build/x86_64/iris/${ASIC}/bin/linkmgr -p $catalog_file
