#! /bin/bash -e

set -e 
set -x

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

INITBINDIR=$NIC_DIR/conf/init_bins
rm -rf $INITBINDIR/*

declare -a arr=("base;cap0/pb/pbc=cap_pb_base.cfg" )

declare -a cfgs=("8x25" "2x100" "4x50")
for cfg in "${cfgs[@]}"
do
    if [ -e "$ASIC_SRC/capri/verif/pb/cfg/gen/hbm_overflow_${cfg}_dol.cfg" ]; then
        arr+=("${cfg}_hbm;cap0/pb/pbc=cap_pb_${cfg}.cfg cap0/pb/pbc=hbm_overflow_${cfg}_dol.cfg" 
              "${cfg};cap0/pb/pbc=cap_pb_${cfg}.cfg" )
    fi
done


for i in "${arr[@]}"
do
    CFGFILES=$(echo "$i"| cut -d';' -f 2) 
    MODE=$(echo "$i"| cut -d';' -f 1) 

    DIR=$INITBINDIR/"$MODE"
    BINDIR=$DIR/init
    RUN_LOG=$DIR/run.log
    PRE_EOS_LOG=$DIR/pre_eos.log
    PB_LOG=$DIR/pb.log

    mkdir -p $DIR
    # GEN AREA:
    cd $ASIC_GEN/capri/verif/top/test
    pcfg cap_top_stream_basic.cfg $CFGFILES -i $ASIC_SRC/capri/verif/top/cfg $ASIC_SRC/capri/verif/common/pknobs $ASIC_SRC/capri/verif/pb/cfg/gen -o $PWD
    export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${ASIC_GEN}/capri/verif/common/manifest:${ASIC_GEN}/capri/model/cap_top:${ASIC_GEN}/capri/verif/apis:${ASIC_SRC}/capri/model/capsim-gen/lib"
    ./cap_sam +plog=info +PLOG_MAX_QUIT_COUNT=0 +model_debug=model_debug.json +CPP_TESTNAME=cap_top_stream_test -f sknobs.out +sam_only=1 +plog_add_scope=csr | tee $RUN_LOG
    cat $RUN_LOG | perl -ne 'if (/inside cap_pb_eos_sta/) {exit;} else {print $_;}' > $PRE_EOS_LOG
    grep "csr.*write.*byte.*cap0.pb\|inside.*pb" $PRE_EOS_LOG > $PB_LOG

    cd $NIC_DIR
    python3 $NIC_DIR/tools/gen_init_config_bin.py -l $PB_LOG -m cap_pb_load_from_cfg -b "$BINDIR"
done
