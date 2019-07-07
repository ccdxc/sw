# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = apollo_nvme.p4bin
MODULE_SRCS     = ${MODULE_SRC_DIR}/nvme_req_tx.p4 \
				  ${MODULE_SRC_DIR}/nvme_sess_pre_xts_tx.p4 \
				  ${MODULE_SRC_DIR}/nvme_sess_post_xts_tx.p4 \
				  ${MODULE_SRC_DIR}/nvme_sess_pre_dgst_tx.p4 \
				  ${MODULE_SRC_DIR}/nvme_sess_post_dgst_tx.p4 \
				  ${MODULE_SRC_DIR}/nvme_req_rx.p4 \
				  ${MODULE_SRC_DIR}/nvme_sess_pre_dgst_rx.p4 \
				  ${MODULE_SRC_DIR}/nvme_sess_post_dgst_rx.p4 \
				  ${MODULE_SRC_DIR}/nvme_sess_pre_xts_rx.p4 \
				  ${MODULE_SRC_DIR}/nvme_sess_post_xts_rx.p4 \
				  ${MODULE_SRC_DIR}/nvme_sess_rsrc_free_rx.p4

MODULE_PIPELINE = apollo
MODULE_NCC_OPTS = --pipeline=apollo --p4-plus --pd-gen --asm-out --no-ohi \
				  --two-byte-profile --fe-flags="-I${MODULE_SRC_DIR}/../txdma -I${MODULE_SRC_DIR}/../rxdma -I${TOPDIR} -I${SDKDIR} -DNVME_APOLLO" \
                  --gen-dir ${BLD_P4GEN_DIR}
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*.p4' -o -name '*.h') \
                  $(shell find ${TOPDIR}/nic/p4/include -name '*')
include ${MKDEFS}/post.mk
