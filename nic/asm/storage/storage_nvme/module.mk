# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = storage_nvme.asmbin
MODULE_PREREQS      = storage.p4bin
MODULE_INCS         = ${BLD_GEN_DIR}/storage_nvme/asm_out \
                      ${MODULE_DIR}/../include \
                      ${TOPDIR}/nic/p4/storage/common \
                      ${TOPDIR}/nic/p4/include \
                      ${TOPDIR}/nic/include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
