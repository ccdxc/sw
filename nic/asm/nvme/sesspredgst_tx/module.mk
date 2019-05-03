# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = nvme_sess_pre_dgst_tx.asmbin
MODULE_PREREQS  = iris_nvme.p4bin
MODULE_PIPELINE = iris
MODULE_INCS     = ${BLD_P4GEN_DIR}/nvme_sess_pre_dgst_tx/asm_out \
                  ${BLD_P4GEN_DIR}/nvme_sess_pre_dgst_tx/alt_asm_out \
                  ${MODULE_DIR}/../common/include \
                  ${MODULE_DIR}//include \
                  ${TOPDIR}/nic/asm/common-p4+/include \
                  ${TOPDIR}/nic/p4/include \
                  ${TOPDIR}/nic/p4/common \
                  ${TOPDIR}/nic/include \
                  ${TOPDIR}/nic
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR  = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
