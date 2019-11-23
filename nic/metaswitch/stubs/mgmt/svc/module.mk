# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = libpdsamgmtsvc.so
MODULE_PIPELINE = apulu
MODULE_INCS     = ${MODULE_GEN_DIR} ${BLD_PROTOGEN_DIR} $(addprefix $(MS_ROOT)/,$(MS_INCLPATH))
MODULE_SOLIBS   = pdsamgmt
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
MODULE_LDLIBS   = $(MS_LD_LIBS) \
                  ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_FLAGS    = $(addprefix -D,$(MS_COMPILATION_SWITCH))
include ${MKDEFS}/post.mk
