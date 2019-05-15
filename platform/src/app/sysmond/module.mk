# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = sysmond.bin
MODULE_PIPELINE = iris
MODULE_SOLIBS   = sdkpal delphisdk trace halproto shmmgr sdkasicpd \
                  sdkcapri_asicrw_if sensor pal catalog sdkfru logger \
                  pdcapri pdcommon p4pd_${PIPELINE} sdkp4 sdkp4utils \
                  p4pd_common_p4plus_rxdma p4pd_common_p4plus_txdma \
                  asicpd ${NIC_HAL_PD_SOLIBS_${ARCH}} ${NIC_SDK_SOLIBS} \
                  bm_allocator mtrack sdkxcvrdriver hal_mock
MODULE_LDLIBS   = :libprotobuf.so.14 ev grpc++ crypto \
                  ${NIC_COMMON_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  ${NIC_CAPSIM_LDLIBS}
MODULE_FLAGS    = -DCAPRI_SW ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
