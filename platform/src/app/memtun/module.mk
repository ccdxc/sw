# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := memtun.bin
MODULE_SRCS     := ${MODULE_SRC_DIR}/capri.c \
                    ${MODULE_SRC_DIR}/pktloop.c \
                    ${MODULE_SRC_DIR}/queue.c \
                    ${MODULE_SRC_DIR}/tun.c \
                    ${MODULE_SRC_DIR}/util.c
MODULE_LDLIBS   := pthread
MODULE_PREREQS  := hal.memrgns
include ${MKDEFS}/post.mk
