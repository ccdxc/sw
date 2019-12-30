# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = libpdsmsmgmtsvc.so
MODULE_PIPELINE = apollo artemis apulu
MODULE_PREREQS  = pdsgen.proto
MODULE_INCS     = ${MODULE_GEN_DIR} ${BLD_PROTOGEN_DIR} $(addprefix $(MS_ROOT)/,$(MS_INCLPATH))
MODULE_SOLIBS   = pdsmsmgmt
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
MODULE_LDLIBS   = $(MS_LD_LIBS) \
                  ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_FLAGS    = $(addprefix -D,$(MS_COMPILATION_SWITCH))
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/gen/svc/*.cc)
include ${MKDEFS}/post.mk
