# {C} Copyright 2018-2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := pciemgrd.bin
MODULE_PIPELINE := iris
MODULE_SOLIBS   := pciemgr_if pciemgr pciemgrutils pciehdevices pcieport \
		   pcietlp intrutils cfgspace pal misc evutils \
		   delphisdk utils upgrade_app upgradeproto \
		   sdkpal logger sysmgr upgradeutils pciemgrproto pciemgrd
ifeq ($(ARCH),aarch64)
MODULE_SOLIBS   += catalog sdkfru
endif
MODULE_ARLIBS    = delphishm
MODULE_INCS     := ${MODULE_SRC_DIR}/../include \
		   ${BLD_PROTOGEN_DIR}
MODULE_LDLIBS   := rt dl pthread ev m edit ncurses \
		   ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_SRCS     := ${MODULE_SRC_DIR}/pciemgrd.cc \
                   ${MODULE_SRC_DIR}/svchandler.cc \
                   ${MODULE_SRC_DIR}/delphic.cc
include ${MKDEFS}/post.mk
