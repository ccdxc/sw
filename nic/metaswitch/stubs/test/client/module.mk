# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#
include ${MKDEFS}/pre.mk
MODULE_TARGET   = pds_ms_grpc_test.bin
MODULE_PREREQS  = metaswitch.submake
MODULE_PIPELINE = apulu
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/test_mgmt.cc)
MODULE_SOLIBS   = pdsgenproto pdsproto pdsatestcommon
MODULE_INCS     = $(TOPDIR)/nic/metaswitch/stubs/hals \
                  $(addprefix $(MS_ROOT)/,$(MS_INCLPATH))  ${MODULE_GEN_DIR}
MODULE_FLAGS    = $(addprefix -D,$(MS_COMPILATION_SWITCH))
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} z
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
include ${MKDEFS}/post.mk
