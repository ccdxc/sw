# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpd_apollo.lib
MODULE_PIPELINE = apollo
MODULE_PREREQS  = apollo_p4.p4bin \
                  apollo_txdma.p4bin apollo_rxdma.p4bin
include ${MKDEFS}/post.mk
