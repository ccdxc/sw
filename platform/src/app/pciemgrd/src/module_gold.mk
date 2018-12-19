# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := pciemgrd-gold.bin
MODULE_SOLIBS   := pciemgr pciemgrutils pciehdevices pcieport pcietlp \
                   intrutils cfgspace evutils pal misc
MODULE_INCS     := ${MODULE_SRC_DIR}/../include
MODULE_FLAGS    := -DPCIEMGRD_GOLD
MODULE_SRCS     := ${MODULE_SRC_DIR}/pciemgrd.cc \
                   ${MODULE_SRC_DIR}/gold.cc \
                   ${MODULE_SRC_DIR}/logger.cc
include ${MKDEFS}/post.mk
