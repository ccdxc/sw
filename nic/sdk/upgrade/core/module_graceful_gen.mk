# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET    = core_graceful.upgfsmgen
MODULE_DEPS      = ${SDKDIR}/upgrade/core/fsm_code_gen.py \
                   ${SDKDIR}/upgrade/core/graceful.json
MODULE_ARGS      = -i ${SDKDIR}/upgrade/core/graceful.json \
                   -o ${BLD_GEN_DIR}/graceful_upgrade.json
MODULE_BASECMD   = python2 ${SDKDIR}/upgrade/core/fsm_code_gen.py
include ${MKDEFS}/post.mk
