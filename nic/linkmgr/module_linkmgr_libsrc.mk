# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblinkmgr_libsrc.so
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/linkmgr_src.cc \
                  ${MODULE_SRC_DIR}/linkmgr_ipc.cc \
                  ${MODULE_SRC_DIR}/linkmgr_event_recorder.cc \
                  ${MODULE_SRC_DIR}/linkmgr_svc.cc \
                  ${MODULE_SRC_DIR}/linkmgr_debug_svc.cc \
                  ${MODULE_SRC_DIR}/linkmgr_state.cc \
                  ${MODULE_SRC_DIR}/linkmgr_utils.cc
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
MODULE_SOLIBS   = hal_mem
include ${MKDEFS}/post.mk
