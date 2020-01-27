# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = pdsupg.bin
MODULE_PIPELINE = apulu
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_PREREQS  = pdsupgradegen.proto
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${NIC_COMMON_LDLIBS} edit ncurses
include ${MKDEFS}/post.mk
