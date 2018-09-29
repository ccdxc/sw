# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = nic_proxy_e2etest_ethrx.bin
MODULE_ARCH     = x86_64
MODULE_PIPELINE = iris
MODULE_SRCS     = ${MODULE_SRC_DIR}/eth_rx.cc
MODULE_SOLIBS   = e2e_driver trace print host_mem bm_allocator \
                  logger model_client halproto mtrack
MODULE_LDLIBS   = zmq pthread z m rt Judy dl \
                  ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
include ${MKDEFS}/post.mk
