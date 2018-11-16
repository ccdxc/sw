# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := x86_64
MODULE_TARGET   := test_simclient.bin
MODULE_SOLIBS   := pciemgr_if pciemgr pciemgrutils pciehdevices pcieport pcietlp cfgspace \
                    intrutils pal misc simlib simdev
MODULE_LDLIBS   := ev pthread
include ${MKDEFS}/post.mk
