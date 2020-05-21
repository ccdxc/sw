# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET    = powerctl.bin
MODULE_PIPELINE  = iris
MODULE_SOLIBS   = halproto sdk_asicrw_if sensor pal \
                  logger catalog sdkfru
MODULE_LDLIBS   = :libprotobuf.so.14 ev grpc++ \
                  ${NIC_COMMON_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  ${NIC_CAPSIM_LDLIBS}
ifeq ($(ASIC),elba)
MODULE_FLAGS    = -DELBA_SW -DELBA ${NIC_CSR_FLAGS}
else
MODULE_FLAGS    = -DCAPRI_SW ${NIC_CSR_FLAGS}
endif
include ${MKDEFS}/post.mk
