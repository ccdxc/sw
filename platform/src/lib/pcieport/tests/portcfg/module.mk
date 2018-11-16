# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := x86_64
MODULE_TARGET   := test_pcieportcfg.bin
MODULE_INCS     := ${TOPDIR}/nic/asic/capri/model/cap_top \
                    ${TOPDIR}/nic/asic/capri/model/cap_top/csr_defines
MODULE_SOLIBS   := pcieport pal pciemgrutils cfgspace utils misc
MODULE_LDLIBS   := pthread
include ${MKDEFS}/post.mk
