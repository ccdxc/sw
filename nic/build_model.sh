#! /bin/bash
cd `dirname $0`
export NIC_DIR=$PWD
echo "NIC_DIR = $NIC_DIR"
export ASIC_SRC=$NIC_DIR/asic
export ASIC_GEN=$NIC_DIR/asic_gen
echo "ASIC_SRC = $ASIC_SRC"
echo "ASIC_GEN = $ASIC_GEN"
export PATH=$ASIC_SRC/common/tools/bin/:$PATH
echo $PATH
cd $ASIC_SRC/capri/model/cap_top
gen_rtl -n -v

$NIC_DIR/model_sim/cp_asic_bins.sh

