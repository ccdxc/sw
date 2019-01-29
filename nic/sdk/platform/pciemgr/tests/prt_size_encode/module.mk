# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := x86_64
MODULE_TARGET   := test_prt_size_encode.bin
MODULE_SOLIBS   := misc pciemgr pciemgrutils pciehdevices pcieport pcietlp \
                    intrutils cfgspace pal misc evutils
include ${MKDEFS}/post.mk
