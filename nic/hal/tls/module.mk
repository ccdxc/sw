# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libtls_api.lib
MODULE_PIPELINE = iris gft
MODULE_INCS     = ${TOPDIR}/nic/hal/third-party/openssl/include
include ${MKDEFS}/post.mk
