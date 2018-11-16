# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := captrace.bin
MODULE_SOLIBS   := sdkpal halproto logger delphisdk
MODULE_LDLIBS   := z dl pthread ${NIC_THIRDPARTY_GOOGLE_LDLIBS} ev rt
include ${MKDEFS}/post.mk
