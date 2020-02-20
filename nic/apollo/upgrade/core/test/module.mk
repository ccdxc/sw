# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET        = upgrade_fsm_test_app.bin
MODULE_PIPELINE      = apulu
MODULE_INCS          = /usr/local/include ${TOPDIR}/nic/apollo/upgrade/core
MODULE_SOLIBS        = pdsupgradecore
MODULE_SRCS          = $(wildcard ${TOPDIR}/nic/apollo/upgrade/core/test/*.cc)
include ${MKDEFS}/post.mk
