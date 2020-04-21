# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET    = powerctl.bin
MODULE_ASIC     := capri
MODULE_PIPELINE  = iris
MODULE_SOLIBS   = halproto sdk_asicrw_if sensor pal \
                  logger catalog sdkfru
MODULE_LDLIBS   = :libprotobuf.so.14 ev grpc++ \
                  ${NIC_COMMON_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  ${NIC_CAPSIM_LDLIBS}
MODULE_FLAGS    = -DCAPRI_SW ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
