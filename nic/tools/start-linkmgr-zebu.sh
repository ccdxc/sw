CUR_DIR=`dirname $0`
ABS_CUR_DIR=`readlink -f $CUR_DIR`
NIC_DIR=`dirname $ABS_CUR_DIR`
export ZMQ_SOC_DIR=$NIC_DIR
export MODEL_ZMQ_TYPE_TCP=1
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$NIC_DIR/conf/linkmgr:$NIC_DIR/conf/sdk/external:$NIC_DIR/conf/sdk:$NIC_DIR/lib:/usr/local/lib:$LD_LIBRARY_PATH
ulimit -c unlimited
$GDB $NIC_DIR/bin/linkmgr 2>&1
