# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = sysmond.bin
MODULE_SOLIBS   = sdkpal delphisdk trace halproto shmmgr capri_csrint \
                  capri_csr_cpu_hal pdcommon hal_mock haltrace mtrack sensor
MODULE_LDLIBS   = rt dl pthread thread :libprotobuf.so.14 ev ${NIC_COMMON_LDLIBS} \
                  grpc++ grpc++_reflection grpc++_unsecure grpc_unsecure \
                  ${NIC_CAPSIM_LDLIBS}
MODULE_FLAGS    = -DCAPRI_SW ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
