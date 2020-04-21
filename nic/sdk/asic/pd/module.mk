# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsdkasicpd.lib
ifeq ($(ASIC),elba)
MODULE_SRCS     := ${MODULE_SRC_DIR}/scheduler_elba.cc \
                   ${MODULE_SRC_DIR}/db.cc	\
                   ${MODULE_SRC_DIR}/pd.cc	\
                   ${MODULE_SRC_DIR}/pd_elba.cc \
                   ${MODULE_SRC_DIR}/pd_elba_accel_rgroup.cc
else
MODULE_SRCS     := ${MODULE_SRC_DIR}/scheduler_capri.cc \
                   ${MODULE_SRC_DIR}/db.cc	\
                   ${MODULE_SRC_DIR}/pd.cc	\
                   ${MODULE_SRC_DIR}/pd_capri.cc	\
                   ${MODULE_SRC_DIR}/pd_capri_accel_rgroup.cc
endif
MODULE_FLAGS    = -O3
include ${MKDEFS}/post.mk
