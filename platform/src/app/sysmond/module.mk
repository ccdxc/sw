# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = sysmond.bin
MODULE_PIPELINE = iris
MODULE_SOLIBS   = sdkpal delphisdk trace halproto shmmgr sdkcapri_csrint \
                  sdkcapri_asicrw_if pdcommon hal_mock haltrace mtrack sensor pal
MODULE_LDLIBS   = rt dl pthread thread :libprotobuf.so.14 ev ${NIC_COMMON_LDLIBS} \
                  grpc++ grpc++_reflection grpc++_unsecure grpc_unsecure \
                  ${NIC_CAPSIM_LDLIBS}
MODULE_FLAGS    = -DCAPRI_SW ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
