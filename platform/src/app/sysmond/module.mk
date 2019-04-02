# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = sysmond.bin
MODULE_PIPELINE = iris
MODULE_SOLIBS   = sdkpal delphisdk trace halproto shmmgr \
                  sdkcapri_asicrw_if sensor pal
MODULE_LDLIBS   = :libprotobuf.so.14 ev grpc++ \
                  ${NIC_COMMON_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  ${NIC_CAPSIM_LDLIBS}
MODULE_FLAGS    = -DCAPRI_SW ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
