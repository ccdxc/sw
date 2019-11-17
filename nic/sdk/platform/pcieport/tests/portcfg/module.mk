# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := x86_64
MODULE_TARGET   := test_pcieportcfg.bin
MODULE_SOLIBS   := pcieport pcietlp pal pciemgrutils cfgspace misc
MODULE_LDLIBS   := pthread
include ${MKDEFS}/post.mk
