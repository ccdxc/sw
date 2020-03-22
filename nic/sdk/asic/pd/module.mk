# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsdkasicpd.lib
#ifeq ($(ASIC),elba)
#MODULE_SRCS     := ${MODULE_SRC_DIR}/scheduler.cc \
#                   ${MODULE_SRC_DIR}/db.cc	\
#                   ${MODULE_SRC_DIR}/pd.cc	\
#                   ${MODULE_SRC_DIR}/pd_elba.cc
#else
MODULE_SRCS     := ${MODULE_SRC_DIR}/scheduler.cc \
                   ${MODULE_SRC_DIR}/db.cc	\
                   ${MODULE_SRC_DIR}/pd.cc	\
                   ${MODULE_SRC_DIR}/pd_capri.cc
#endif
MODULE_FLAGS    = -O3
include ${MKDEFS}/post.mk
