# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = hal.memrgns
ifeq "$(findstring $(PIPELINE),$(PIPLINES_8G))" "$(PIPELINE)"
MODULE_DEPS   = ${TOPDIR}/nic/tools/hal/mem_parser.py \
		${TOPDIR}/nic/conf/${PIPELINE}/8g/hbm_mem.json
MODULE_ARGS = ${TOPDIR}/nic/conf/${PIPELINE}/8g/hbm_mem.json \
		${BLD_GEN_DIR}/platform/mem_regions.hpp \
		${PIPELINE}
else
MODULE_DEPS   = ${TOPDIR}/nic/tools/hal/mem_parser.py \
		${TOPDIR}/nic/conf/${PIPELINE}/hbm_mem.json
MODULE_ARGS = ${TOPDIR}/nic/conf/${PIPELINE}/hbm_mem.json \
		${BLD_GEN_DIR}/platform/mem_regions.hpp \
		${PIPELINE}
endif
MODULE_BASECMD = python2 ${TOPDIR}/nic/tools/hal/mem_parser.py
include ${MKDEFS}/post.mk
