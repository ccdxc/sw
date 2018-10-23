# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblinkmgr_libsrc.so
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/linkmgr_src.cc \
                  ${MODULE_SRC_DIR}/linkmgr_svc.cc \
                  ${MODULE_SRC_DIR}/linkmgr_debug_svc.cc \
                  ${MODULE_SRC_DIR}/linkmgr_state.cc \
                  ${MODULE_SRC_DIR}/linkmgr_delphi.cc \
                  ${MODULE_SRC_DIR}/linkmgr_utils.cc
MODULES_PREREQS = libhalproto.so
include ${MKDEFS}/post.mk
