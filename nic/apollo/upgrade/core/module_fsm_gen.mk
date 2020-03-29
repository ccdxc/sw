# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk

MODULE_TARGET    = upgrade.upgfsmgen
MODULE_PIPELINE  = apulu

# Real fsm
#MODULE_DEPS      = ${TOPDIR}/nic/apollo/upgrade/core/fsm_code_gen.py \
#                   ${TOPDIR}/nic/apollo/upgrade/core/graceful.json
#MODULE_ARGS      = -i ${TOPDIR}/nic/apollo/upgrade/core/graceful.json \
#                   -o ${BLD_PROTOGEN_DIR}/upg_fsm.hpp

# Test fsm
MODULE_DEPS      = ${TOPDIR}/nic/apollo/upgrade/core/fsm_code_gen.py \
                   ${TOPDIR}/nic/apollo/upgrade/test/workflow/graceful_test.json
MODULE_ARGS      = -i ${TOPDIR}/nic/apollo/upgrade/test/workflow/graceful_test.json \
                   -o ${BLD_PROTOGEN_DIR}/upg_fsm.hpp

MODULE_BASECMD   = python2 ${TOPDIR}/nic/apollo/upgrade/core/fsm_code_gen.py

include ${MKDEFS}/post.mk
