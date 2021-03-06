# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libp4pd_pack_storage_seq.lib
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = storage.p4bin
MODULE_SRC_DIR  = ${BLD_P4GEN_DIR}/storage_seq/src/
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*p4plus_entry_packing.cc)
include ${MKDEFS}/post.mk
