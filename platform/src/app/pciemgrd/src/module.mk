# {C} Copyright 2018-2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := pciemgrd.bin
MODULE_SOLIBS   := pciemgrd pciemgr_if pciemgr pciemgrutils pciehdevices \
                   pcieport pcietlp intrutils cfgspace pal misc evutils
MODULE_INCS     := ${MODULE_SRC_DIR}/../include
MODULE_SRCS     := ${MODULE_SRC_DIR}/pciemgrd.cc      \
                   ${MODULE_SRC_DIR}/logger.cc        \
                   ${MODULE_SRC_DIR}/cli.cc

ifneq (,$(filter $(PIPELINE),apulu))

MODULE_PIPELINE := apulu
MODULE_SRCS     += ${MODULE_SRC_DIR}/delphic_stubs.cc \
                   ${NULL}
MODULE_LDLIBS   := ev edit ncurses
ifeq ($(ARCH),aarch64)
MODULE_SOLIBS   += catalog sdkfru
endif

else

MODULE_PIPELINE := iris
MODULE_SOLIBS   += delphisdk utils upgrade_app upgradeproto       \
                   sdkpal logger sysmgr upgradeutils pciemgrproto \
                   ${NULL}
MODULE_INCS     += ${BLD_PROTOGEN_DIR}
MODULE_SRCS     += ${MODULE_SRC_DIR}/svchandler.cc \
                   ${MODULE_SRC_DIR}/delphic.cc    \
                   ${NULL}
MODULE_LDLIBS   := rt dl pthread ev m edit ncurses \
                   ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                   ${NULL}
MODULE_ARLIBS    = delphishm
ifeq ($(ARCH),aarch64)
MODULE_LDLIBS   += ${NIC_COMMON_LDLIBS}           \
                   ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                   ${NIC_CAPSIM_LDLIBS}           \
                   ${NULL}
MODULE_SOLIBS   += catalog sdkfru sensor
endif

endif

include ${MKDEFS}/post.mk
