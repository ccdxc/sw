# {C} Copyright 2018-2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := pcieutil.bin
MODULE_SOLIBS   := pciemgr pciemgrutils cfgspace pcieport pcietlp \
		   intrutils pal misc \
		   ${NULL}
ifeq ($(ARCH),aarch64)
MODULE_SOLIBS   += catalog sdkfru
endif
include ${MKDEFS}/post.mk
