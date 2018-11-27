# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = fte_sim.bin
MODULE_SRCS     = ${MODULE_SRC_DIR}/main.cc
MODULE_SOLIBS   = fte_sim_lib halproto agent_api p4pd_mock
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${NIC_COMMON_LDLIBS}
include ${MKDEFS}/post.mk
