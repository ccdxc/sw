# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpd_gft.so
MODULE_PIPELINE = gft
MODULE_PREREQS  = gft.p4bin \
                  common_p4plus_txdma.p4bin common_p4plus_rxdma.p4bin
MODULE_INCS     = ${NIC_CSR_INCS}
include ${MKDEFS}/post.mk
