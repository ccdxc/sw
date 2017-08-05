#! /bin/bash
export NIC_DIR=`dirname $0`
export ASIC_SRC=$NIC_DIR/asic
export ASIC_GEN=$NIC_DIR/asic_gen
export $ASIC_SRC/common/tools/bin/:$PATH
cd $ASIC_SRC/capri/model/cap_top
gen_rtl -n -v
