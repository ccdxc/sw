#! /bin/bash
cd `dirname $0`
export ASIC_SRC=$NIC_DIR/sdk/asic_repo/asic
export ASIC_GEN=$NIC_DIR/asic_gen
echo "ASIC_SRC = $ASIC_SRC"
echo "ASIC_GEN = $ASIC_GEN"

cp -v $ASIC_SRC/elba/model/elb_top/elb_env_base.h $NIC_DIR/sdk/model_sim/include/elb_env_base.h
cp -v $ASIC_SRC/elba/model/elb_top/elb_model_base.h $NIC_DIR/sdk/model_sim/include/elb_model_base.h
cp -v $ASIC_SRC/ip/verif/pcpp/pen_blk_env_base.h $NIC_DIR/sdk/model_sim/include/pen_blk_env_base.h
cp -v $ASIC_SRC/ip/verif/pcpp/cpu_bus_base.h $NIC_DIR/sdk/model_sim/include/cpu_bus_base.h
cp -v $ASIC_SRC/ip/verif/pcpp/cpu_bus_stub.h $NIC_DIR/sdk/model_sim/include/cpu_bus_stub.h
cp -v $ASIC_SRC/ip/verif/pcpp/cpu.h $NIC_DIR/sdk/model_sim/include/cpu.h
cp -v $ASIC_SRC/ip/verif/pcpp/HBM.h $NIC_DIR/sdk/model_sim/include/HBM.h
cp -v $ASIC_SRC/ip/verif/pcpp/HOST_MEM.h $NIC_DIR/sdk/model_sim/include/HOST_MEM.h
cp -v $ASIC_SRC/ip/verif/pcpp/pen_mem.h $NIC_DIR/sdk/model_sim/include/pen_mem.h
cp -v $ASIC_SRC/ip/verif/pcpp/pen_mem_base.h $NIC_DIR/sdk/model_sim/include/pen_mem_base.h
cp -v $ASIC_SRC/ip/verif/pcpp/pknobs.h $NIC_DIR/sdk/model_sim/include/pknobs.h
cp -v $ASIC_SRC/ip/verif/pcpp/LogMsg.h $NIC_DIR/sdk/model_sim/include/LogMsg.h
cp -v $ASIC_SRC/ip/verif/pcpp/common_dpi.h $NIC_DIR/sdk/model_sim/include/common_dpi.h
#cp -v $ASIC_SRC/capri/model/utils/cap_blk_env_base.h include/cap_blk_env_base.h -- Where is elba file
cp -v $ASIC_SRC/elba/model/elb_te/elb_te_csr.json $NIC_DIR/tools/ncc/csr_json/
cp -v $ASIC_SRC/elba/model/elb_ppa/elb_ppa_decoders.json $NIC_DIR/tools/ncc/csr_json/
cp -v $ASIC_SRC/elba/model/elb_ppa/elb_ppa_csr.json $NIC_DIR/tools/ncc/csr_json/
cp -v $ASIC_SRC/elba/model/elb_pic/elb_pict_csr.json $NIC_DIR/tools/ncc/csr_json/
cp -v $ASIC_SRC/elba/model/elb_pic/elb_pics_csr.json $NIC_DIR/tools/ncc/csr_json/
cp -v $ASIC_SRC/elba/model/elb_dpa/elb_dpr_csr.json $NIC_DIR/tools/ncc/csr_json/
cp -v $ASIC_SRC/elba/model/elb_dpa/elb_dpp_csr.json $NIC_DIR/tools/ncc/csr_json/
cp -v $ASIC_SRC/elba/design/common/elb_addr.json $NIC_DIR/tools/ncc/csr_json/
