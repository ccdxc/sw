# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := pciemgrd.bin
MODULE_SOLIBS   := pciemgr_if pciemgr pciemgrutils pciehdevices pcieport pcietlp \
                    intrutils cfgspace pal misc evutils delphisdk sysmgr
MODULE_INCS     := ${MODULE_SRC_DIR}/../include \
                    ${TOPDIR}/platform/src/third-party/lib/edit/include
MODULE_LDPATHS  := ${TOPDIR}/platform/src/third-party/lib/gmp/${ARCH} \
                    ${TOPDIR}/platform/src/third-party/lib/edit/${ARCH} \
                    ${TOPDIR}/platform/src/third-party/lib/ncurses/${ARCH}
MODULE_LDLIBS   := pthread ev m edit ncurses ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_SRCS     := ${MODULE_SRC_DIR}/pciemgrd.cc \
                    ${MODULE_SRC_DIR}/cli.cc \
                    ${MODULE_SRC_DIR}/server.cc \
                    ${MODULE_SRC_DIR}/gold.cc \
                    ${MODULE_SRC_DIR}/logger.cc \
                    ${MODULE_SRC_DIR}/delphic.cc
include ${MKDEFS}/post.mk
