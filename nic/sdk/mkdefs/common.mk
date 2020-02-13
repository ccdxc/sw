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

export SDK_THIRD_PARTY_INCLUDES := \
       ${SDKDIR}/third-party/googletest-release-1.8.0/googletest/include \
       ${SDKDIR}/third-party/libev/include \
       ${SDKDIR}/third-party/boost/include \
       ${SDKDIR}/third-party/zmq/include \
       ${SDKDIR}/third-party/edit/include \
       ${SDKDIR}/third-party/gmp/include \
       ${SDKDIR}/third-party/ncurses/include \
       ${SDKDIR}/third-party/openssl/include \
       ${SDKDIR}/third-party/liblmdb/include

export SDK_THIRD_PARTY_VPP_INCLUDES := \
       ${SDKDIR}/third-party/vpp-pkg/include

export SDK_THIRD_PARTY_VPP_LDPATH := \
       ${SDKDIR}/third-party/vpp-pkg/${ARCH}/lib

export SDK_THIRD_PARTY_VPP_LIBS := \
       :libvnet.so.19.01 :libvlib.so.19.01 :libvppinfra.so.19.01

export SDK_THIRDPARTY_CAPRI_LDLIBS := sdkcapri_csrint

export THIRD_PARTY_INCLUDES := ${SDK_THIRD_PARTY_INCLUDES}

export ASIC_SRC := ${SDKDIR}/asic_repo/asic

export ASIC_INCL_DIR := ${SDKDIR}/third-party/asic

export NIC_CSR_INCS := ${ASIC_INCL_DIR}/capri/model/cap_top \
    ${ASIC_INCL_DIR}/capri/model/utils \
    ${ASIC_INCL_DIR}/capri/model/cap_prd \
    ${ASIC_INCL_DIR}/capri/model/cap_ptd \
    ${ASIC_INCL_DIR}/capri/model/cap_psp \
    ${ASIC_INCL_DIR}/capri/model/cap_ppa \
    ${ASIC_INCL_DIR}/capri/model/cap_te \
    ${ASIC_INCL_DIR}/capri/model/cap_mpu \
    ${ASIC_INCL_DIR}/capri/model/cap_pic \
    ${ASIC_INCL_DIR}/capri/model/cap_pcie \
    ${ASIC_INCL_DIR}/capri/model/cap_npv \
    ${ASIC_INCL_DIR}/capri/model/cap_txs \
    ${ASIC_INCL_DIR}/capri/model/cap_pb \
    ${ASIC_INCL_DIR}/capri/model/cap_wa \
    ${ASIC_INCL_DIR}/capri/model/cap_ms \
    ${ASIC_INCL_DIR}/capri/model/cap_em \
    ${ASIC_INCL_DIR}/capri/model/cap_mc \
    ${ASIC_INCL_DIR}/capri/model/cap_he \
    ${ASIC_INCL_DIR}/capri/model/cap_dpa \
    ${ASIC_INCL_DIR}/capri/model/cap_sema \
    ${ASIC_INCL_DIR}/capri/model/cap_intr \
    ${ASIC_INCL_DIR}/capri/model/cap_mx \
    ${ASIC_INCL_DIR}/capri/model/cap_bx \
    ${ASIC_INCL_DIR}/capri/model/cap_ap \
    ${ASIC_INCL_DIR}/capri/model/cap_he/readonly \
    ${ASIC_INCL_DIR}/ip/verif/pcpp \
    ${ASIC_INCL_DIR}/capri/design/common/gen \
    ${ASIC_INCL_DIR}/capri/verif/apis \
    ${SDKDIR}/model_sim/include \
    ${ASIC_INCL_DIR}/capri/model/cap_top/csr_defines

