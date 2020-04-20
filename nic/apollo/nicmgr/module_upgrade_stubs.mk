# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

# as proto files are not build for athena pipeline requires
# to create a seperate upgrade lib with only stubs
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsupg_nicmgr_stubs.lib
MODULE_PIPELINE = athena
MODULE_SRCS     := ${MODULE_SRC_DIR}/nicmgr_upgrade_stubs.cc
include ${MKDEFS}/post.mk
