# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = sysmond.bin
MODULE_PIPELINE = iris
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc ${MODULE_SRC_DIR}/event_recorder/*.cc) \
                  ${MODULE_SRC_DIR}/delphi/sysmond_delphi.cc ${MODULE_SRC_DIR}/delphi/sysmond_delphi_cb.cc \
                  ${MODULE_SRC_DIR}/delphi/asicerrord_delphi.cc ${MODULE_SRC_DIR}/cpld/*.cc)
MODULE_SOLIBS   = delphisdk trace halproto utils sdkasicpd \
                  sdk_asicrw_if sensor pal catalog logger \
                  pdcommon p4pd_${PIPELINE} sdkp4 \
                  p4pd_common_p4plus_rxdma p4pd_common_p4plus_txdma \
                  asicpd ${NIC_SDK_SOLIBS} penipc penipc_ev halproto \
                  bm_allocator sdkxcvrdriver hal_mock events_recorder \
                  eventproto eventtypes asicerror event_thread
MODULE_LDLIBS   = :libprotobuf.so.14 grpc++ \
                  ${NIC_COMMON_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${NIC_CAPSIM_LDLIBS}
MODULE_FLAGS    = -DCAPRI_SW ${NIC_CSR_FLAGS} -O2
include ${MKDEFS}/post.mk
