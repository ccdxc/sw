# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := pcieutil.bin
MODULE_INCS     := ${TOPDIR}/nic/asic/capri/model/cap_top \
                    ${TOPDIR}/nic/asic/capri/model/cap_top/csr_defines
MODULE_SOLIBS   := pciemgr pciemgrutils cfgspace pcietlp pcieport intrutils pal misc
include ${MKDEFS}/post.mk
