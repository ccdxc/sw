# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = hello_test.gtest
MODULE_PIPELINE = hello
MODULE_SOLIBS   = ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  hal_mock model_client halproto \
                  capri_csrint capri_csr_cpu_hal \
                  ${NIC_SDK_SOLIBS} \
                  pdcommon core fte_mock agent_api \
                  bm_allocator bitmap trace mtrack \
                  pdcapri sdkcapri p4pd_utils p4pd \
                  asicpd hal_mock pack_bytes haltrace
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${NIC_THIRDPARTY_SSL_LDLIBS} \
                  ${NIC_CAPSIM_LDLIBS} \
                  ${NIC_COMMON_LDLIBS}
include ${MKDEFS}/post.mk
