#! /bin/bash -e
cd `dirname $0`
cd `/bin/pwd`
export NIC_DIR=$PWD
echo "NIC_DIR = $NIC_DIR"
export ASIC_SRC=$NIC_DIR/asic
export ASIC_GEN=$NIC_DIR/asic_gen
export COVFILE=$NIC_DIR/coverage/bullseye_model.cov
BULLSEYE_PATH=/home/asic/tools/eda/bullseye/bin
echo "ASIC_SRC = $ASIC_SRC"
echo "ASIC_GEN = $ASIC_GEN"
export PATH=$ASIC_SRC/common/tools/bin/:$PATH
echo $PATH
cd $ASIC_SRC/capri/model/cap_top
if [ $# -eq 0 ]; then
    echo "Starting ASIC build"
    gen_rtl -n -v NOSKNOBS_PATH=1 OPT=-O2 -j$(grep -c processor /proc/cpuinfo)
elif [ $1 == "--coverage" ]; then
    echo "Starting ASIC build with coverage"
    $BULLSEYE_PATH/cov01 -1
    gen_rtl -n -v -- --coverage=1 NOSKNOBS_PATH=1 OPT=-O2 EXT_CC_OVERRIDE=$BULLSEYE_PATH/g++  -j $(grep -c processor /proc/cpuinfo)
    $BULLSEYE_PATH/cov01 -0
elif [ $1 == "--clean" ]; then
    echo "Starting ASIC clean build"
    gen_rtl -n -v -c
    rm -f $COVFILE
else
    echo "Invalid args. Use --coverage or --clean"
    exit
fi
$NIC_DIR/model_sim/cp_asic_bins.sh

