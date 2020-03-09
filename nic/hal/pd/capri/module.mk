# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdcapri.lib
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = capri_tm_rw_gen.tenjin \
                  common_p4plus_rxdma.p4bin \
                  common_p4plus_txdma.p4bin \
                  hal.memrgns
#MODULE_LIBS 	= -lssl -lcrypto
MODULE_DEFS     = ${NIC_CSR_DEFINES}
MODULE_INCS     = ${TOPDIR}/nic/hal/third-party/openssl/include
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
