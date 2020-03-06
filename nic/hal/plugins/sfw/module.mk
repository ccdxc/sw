# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libplugin_sfw.lib
MODULE_PIPELINE = iris gft
MODULE_SRCS = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
              $(wildcard ${MODULE_SRC_DIR}/alg_utils/*.cc) \
              $(wildcard ${MODULE_SRC_DIR}/alg_dns/*.cc) \
              $(wildcard ${MODULE_SRC_DIR}/alg_ftp/*.cc) \
              $(wildcard ${MODULE_SRC_DIR}/alg_rpc/*.cc) \
              $(wildcard ${MODULE_SRC_DIR}/alg_rtsp/*.cc) \
              $(wildcard ${MODULE_SRC_DIR}/alg_sip/*.cc) \
              $(wildcard ${MODULE_SRC_DIR}/alg_tftp/*.cc)
include ${MKDEFS}/post.mk
