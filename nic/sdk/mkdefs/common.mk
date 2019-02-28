# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
NCPUS=$(shell grep processor /proc/cpuinfo | wc -l)
MAKEFLAGS += -j${NCPUS}

export NIC_CSR_DEFINES := -DEXCLUDE_PER_FIELD_CNTRL \
    -DCAPRI_HAL -DCAP_CSR_LARGE_ARRAY_THRESHOLD=1024 -DBOOST_EXCEPTION_DISABLE \
    -DCSR_NO_SHOW_IMPL -DCSR_NO_CALLBACK -DDCSR_NO_RESET_VAL \
    -DCSR_NO_INST_PATH -DCSR_NO_CTOR_DEF_NAME -DCSR_NO_BASE_NAME \
    -DCSR_NO_CSR_TYPE -DPLOG_ONLY_ERR_MODE

export NIC_CSR_FLAGS := -Wno-unused-function -Wno-unused-variable \
    -Wno-sign-compare -Wno-maybe-uninitialized -Wno-uninitialized \
    -Wno-unused-but-set-variable -fno-asynchronous-unwind-tables

export NIC_CSR_INCS := ${SDKDIR}/third-party/asic/capri/model/cap_top \
    ${SDKDIR}/third-party/asic/capri/model/utils \
    ${SDKDIR}/third-party/asic/capri/model/cap_prd \
    ${SDKDIR}/third-party/asic/capri/model/cap_ptd \
    ${SDKDIR}/third-party/asic/capri/model/cap_psp \
    ${SDKDIR}/third-party/asic/capri/model/cap_ppa \
    ${SDKDIR}/third-party/asic/capri/model/cap_te \
    ${SDKDIR}/third-party/asic/capri/model/cap_mpu \
    ${SDKDIR}/third-party/asic/capri/model/cap_pic \
    ${SDKDIR}/third-party/asic/capri/model/cap_pcie \
    ${SDKDIR}/third-party/asic/capri/model/cap_npv \
    ${SDKDIR}/third-party/asic/capri/model/cap_txs \
    ${SDKDIR}/third-party/asic/capri/model/cap_pb \
    ${SDKDIR}/third-party/asic/capri/model/cap_wa \
    ${SDKDIR}/third-party/asic/capri/model/cap_ms \
    ${SDKDIR}/third-party/asic/capri/model/cap_em \
    ${SDKDIR}/third-party/asic/capri/model/cap_mc \
    ${SDKDIR}/third-party/asic/capri/model/cap_he \
    ${SDKDIR}/third-party/asic/capri/model/cap_dpa \
    ${SDKDIR}/third-party/asic/capri/model/cap_sema \
    ${SDKDIR}/third-party/asic/capri/model/cap_intr \
    ${SDKDIR}/third-party/asic/capri/model/cap_mx \
    ${SDKDIR}/third-party/asic/capri/model/cap_bx \
    ${SDKDIR}/third-party/asic/capri/model/cap_ap \
    ${SDKDIR}/third-party/asic/capri/model/cap_he/readonly \
    ${SDKDIR}/third-party/asic/ip/verif/pcpp \
    ${SDKDIR}/third-party/asic/capri/design/common/gen \
    ${SDKDIR}/third-party/asic/capri/verif/apis \
    ${SDKDIR}/model_sim/include \
    ${SDKDIR}/third-party/asic/capri/model/cap_top/csr_defines


export SDK_THIRD_PARTY_INCLUDES := \
       ${SDKDIR}/third-party/googletest-release-1.8.0/googletest/include \
       ${SDKDIR}/third-party/libev/include \
       ${SDKDIR}/third-party/boost/include \
       ${SDKDIR}/third-party/zmq/include \
       ${SDKDIR}/third-party/edit/include \
       ${SDKDIR}/third-party/gmp/include \
       ${SDKDIR}/third-party/ncurses/include

export THIRD_PARTY_INCLUDES := ${SDK_THIRD_PARTY_INCLUDES}
