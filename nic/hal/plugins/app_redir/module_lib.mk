# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libplugin_app_redir.lib
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/app_redir.cc \
                  ${MODULE_SRC_DIR}/app_redir_plugin.cc \
                  ${MODULE_SRC_DIR}/app_redir_appid.cc \
                  ${MODULE_SRC_DIR}/app_redir_scanner.cc \
                  ${MODULE_SRC_DIR}/app_redir_cb_ops.cc
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
