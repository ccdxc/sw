# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdaccel.lib
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = capri_tm_rw_gen.tenjin \
                  common_p4plus_rxdma.p4bin \
				  common_p4plus_txdma.p4bin
MODULE_DEFS     = ${NIC_CSR_DEFINES}
MODULE_INCS     = ${NIC_CSR_INCS}
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
include ${MKDEFS}/post.mk
