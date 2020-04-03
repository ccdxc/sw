# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET    = hitless_test.upgfsmgen
MODULE_DEPS      = ${SDKDIR}/upgrade/core/fsm_code_gen.py \
                   ${MODULE_SRC_DIR}/hitless_test.json
MODULE_ARGS      = -i ${MODULE_SRC_DIR}/hitless_test.json \
                   -o ${BLD_GEN_DIR}/hitless_test_upgrade.json
MODULE_BASECMD   = python2 ${SDKDIR}/upgrade/core/fsm_code_gen.py
include ${MKDEFS}/post.mk
