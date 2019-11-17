# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := pcieutil.bin
MODULE_SOLIBS   := pciemgr pciemgrutils cfgspace pcieport pcietlp \
		   intrutils pal misc \
		   ${NULL}
include ${MKDEFS}/post.mk
