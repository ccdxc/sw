# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := pcieutil.bin
MODULE_SOLIBS   := pciemgr pciemgrutils cfgspace pcietlp pcieport intrutils pal misc
include ${MKDEFS}/post.mk
