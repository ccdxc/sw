#!/bin/bash -e
#cd `dirname $0`        
#NIC_DIR=$PWD      
export ASIC="${ASIC:-capri}"
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
# export LD_LIBRARY_PATH=$NIC_DIR/model_sim/:$NIC_DIR/model_sim/libs:/home/asic/tools/src/0.25/x86_64/lib64:$LD_LIBRARY_PATH        
SUDO=

while getopts t:p: option
do
 case "${option}"
 in
 t) server_type=${OPTARG};;
 p) portlist=${OPTARG};;
 esac
done 

if [ "$server_type" = "tls" ]; then
    $SUDO $GDB $NIC_DIR/build/x86_64/iris/${ASIC}/bin/nic_proxy_e2etest_tls_server $portlist
else
    $SUDO $GDB $NIC_DIR/build/x86_64/iris/${ASIC}/bin/nic_proxy_e2etest_tcp_server $portlist
fi

##! /bin/bash -e
#TOOLS_DIR=`dirname $0`
#ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
#NIC_DIR=`dirname $ABS_TOOLS_DIR`
#export LD_LIBRARY_PATH=$NIC_DIR/model_sim/:$NIC_DIR/model_sim/libs:/home/asic/tools/src/0.25/x86_64/lib64:$LD_LIBRARY_PATH        
#$GDB $NIC_DIR/model_sim/build/cap_model +plog=info +model_debug=$NIC_DIR/build/iris/gen/datapath/p4/dbg_out/model_debug.json 2>&1 | tee $NIC_DIR/model.log
