# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpd_gft.lib
MODULE_PIPELINE = gft
MODULE_PREREQS  = gft.p4bin iris_nvme.p4bin \
                  common_p4plus_txdma.p4bin common_p4plus_rxdma.p4bin
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
				  $(wildcard ${MODULE_SRC_DIR}/../common_p4plus/*.cc)
include ${MKDEFS}/post.mk
