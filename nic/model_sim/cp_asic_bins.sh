#! /bin/bash
cd `dirname $0`
MODEL_SIM_DIR=$PWD
ASIC_SRC=$MODEL_SIM_DIR/../asic/
ASIC_GEN=$MODEL_SIM_DIR/../asic_gen/
echo "ASIC_SRC = $ASIC_SRC"
echo "ASIC_GEN = $ASIC_GEN"

cp -v $ASIC_SRC/capri/model/cap_top/cap_env_base.h include/cap_env_base.h
cp -v $ASIC_SRC/capri/model/cap_top/cap_model_base.h include/cap_model_base.h
cp -v $ASIC_SRC/ip/verif/pcpp/cpu_bus_base.h include/cpu_bus_base.h
cp -v $ASIC_SRC/ip/verif/pcpp/cpu_bus_stub.h include/cpu_bus_stub.h
cp -v $ASIC_SRC/ip/verif/pcpp/cpu.h include/cpu.h
cp -v $ASIC_SRC/ip/verif/pcpp/HBM.h include/HBM.h
cp -v $ASIC_SRC/ip/verif/pcpp/HOST_MEM.h include/HOST_MEM.h
cp -v $ASIC_SRC/ip/verif/pcpp/pen_mem.h include/pen_mem.h
cp -v $ASIC_SRC/ip/verif/pcpp/pen_mem_base.h include/pen_mem_base.h
cp -v $ASIC_SRC/ip/verif/pcpp/pknobs.h include/pknobs.h
cp -v $ASIC_SRC/capri/model/utils/LogMsg.h include/LogMsg.h
cp -v $ASIC_SRC/capri/model/utils/common_dpi.h include/common_dpi.h
cp -v $ASIC_SRC/capri/model/utils/cap_blk_env_base.h include/cap_blk_env_base.h
cp -v $ASIC_GEN/capri/verif/common/manifest/libcommon_cc.so libs/libcommon_cc.so
cp -v $ASIC_GEN/capri/model/cap_top/libmodel_cc.so libs/libmodel_cc.so
cp -v $ASIC_GEN/capri/verif/common/manifest/libtop_csr_compile.so libs/libtop_csr_compile.so
cp -v $ASIC_SRC/capri/model/capsim-gen/lib/libasmsym.a libs/libasmsym.a
cp -v $ASIC_SRC/capri/model/capsim-gen/lib/libcapsim.a libs/libcapsim.a
cp -v $ASIC_SRC/capri/model/capsim-gen/lib/libisa.a libs/libisa.a
cp -v $ASIC_SRC/capri/model/capsim-gen/lib/libcapisa.a libs/libcapisa.a
cp -v $ASIC_SRC/capri/model/capsim-gen/lib/libmpuobj.a libs/libmpuobj.a

cp -v $ASIC_SRC/capri/model/cap_te/cap_te_csr.json ../tools/ncc/csr_json/
cp -v $ASIC_SRC/capri/model/cap_ppa/cap_ppa_decoders.json ../tools/ncc/csr_json/
cp -v $ASIC_SRC/capri/model/cap_ppa/cap_ppa_csr.json ../tools/ncc/csr_json/
cp -v $ASIC_SRC/capri/model/cap_pic/cap_pict_csr.json ../tools/ncc/csr_json/
cp -v $ASIC_SRC/capri/model/cap_pic/cap_pics_csr.json ../tools/ncc/csr_json/
cp -v $ASIC_SRC/capri/model/cap_dpa/cap_dpr_csr.json ../tools/ncc/csr_json/
cp -v $ASIC_SRC/capri/model/cap_dpa/cap_dpp_csr.json ../tools/ncc/csr_json/
cp -v $ASIC_SRC/capri/design/common/cap_addr.json ../tools/ncc/csr_json/
