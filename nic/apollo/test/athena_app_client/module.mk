# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = athena_client.bin
MODULE_PIPELINE = athena
MODULE_ARCH     = aarch64
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_LDFLAGS  = -L${TOPDIR}/nic/third-party/gflags/${ARCH}/lib
MODULE_LDLIBS   = zmq gflags
include ${MKDEFS}/post.mk
