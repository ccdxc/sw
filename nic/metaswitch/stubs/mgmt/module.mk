# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = libpdsamgmt.so
MODULE_PIPELINE = apollo artemis apulu
MODULE_PREREQS  = metaswitch.submake pdsgen.proto
MODULE_INCS 	= ${BLD_PROTOGEN_DIR} $(addprefix $(MS_ROOT)/,$(MS_INCLPATH))
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
MODULE_LDLIBS   = $(MS_LD_LIBS) \
                  ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_FLAGS	= $(addprefix -D,$(MS_COMPILATION_SWITCH))
MODULE_SOLIBS   = logger pdsproto
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/gen/mgmt/*.cc)
include ${MKDEFS}/post.mk
