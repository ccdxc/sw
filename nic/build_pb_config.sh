#! /bin/bash -e

set -e 

cd `dirname $0`
cd `/bin/pwd`
export NIC_DIR=$PWD
echo "NIC_DIR = $NIC_DIR"
export ASIC_SRC=$NIC_DIR/asic
export ASIC_GEN=$NIC_DIR/asic_gen
echo "ASIC_SRC = $ASIC_SRC"
echo "ASIC_GEN = $ASIC_GEN"
export PATH=$ASIC_SRC/common/tools/bin/:$PATH
echo $PATH

rm -rf $ASIC_GEN
cd $ASIC_SRC
git submodule update --init
cd $ASIC_SRC/capri/verif/top/test
gen_rtl -j4 -n -v -m sam.pf include_field_cntrl=True | tee build.log

# GEN AREA:
cd $ASIC_GEN/capri/verif/top/test
pcfg cap_top_stream_basic.cfg cap0/pb/pbc=cap_pb_base.cfg -i $ASIC_SRC/capri/verif/top/cfg $ASIC_SRC/capri/verif/common/pknobs $ASIC_SRC/capri/verif/pb/cfg/gen -o $PWD
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${ASIC_GEN}/capri/verif/common/manifest:${ASIC_GEN}/capri/model/cap_top:${ASIC_GEN}/capri/verif/apis:${ASIC_SRC}/capri/model/capsim-gen/lib"
./cap_sam +plog=info +PLOG_MAX_QUIT_COUNT=0 +model_debug=model_debug.json +CPP_TESTNAME=cap_top_stream_test -f sknobs.out +sam_only=1 +plog_add_scope=csr | tee run.log
cat run.log | perl -ne 'if (/inside cap_pb_eos_sta/) {exit;} else {print $_;}' > pre_eos.log
grep "csr.*write.*byte.*cap0.pb\|inside.*pb" pre_eos.log > $NIC_DIR/pb.log 

cd $NIC_DIR
python3 $NIC_DIR/tools/gen_init_config_bin.py -l $NIC_DIR/pb.log -m cap_pb_load_from_cfg
