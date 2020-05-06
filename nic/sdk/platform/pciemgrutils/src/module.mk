# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := libpciemgrutils.lib
MODULE_INCS     := ${MODULE_SRC_DIR}/../include/
ifeq ($(ASIC),capri)
MODULE_DEFS     := -DASIC_CAPRI
endif
ifeq ($(ASIC),elba)
MODULE_DEFS     := -DASIC_ELBA
endif
include ${MKDEFS}/post.mk
