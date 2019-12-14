# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := pciemgrd-gold.bin
MODULE_SOLIBS   := pciemgrd pciemgr_if pciemgr pciemgrutils pciehdevices \
		   pcieport pcietlp intrutils cfgspace pal misc evutils
ifeq ($(ARCH),aarch64)
MODULE_SOLIBS   += catalog sdkfru
endif
MODULE_LDLIBS   := ev
MODULE_INCS     := ${MODULE_SRC_DIR}/../include
MODULE_FLAGS    := -DPCIEMGRD_GOLD
MODULE_SRCS     := ${MODULE_SRC_DIR}/pciemgrd.cc \
		   ${MODULE_SRC_DIR}/gold.cc \
		   ${MODULE_SRC_DIR}/logger.cc \
		   ${MODULE_SRC_DIR}/delphic_stubs.cc \
		   ${NULL}
include ${MKDEFS}/post.mk
