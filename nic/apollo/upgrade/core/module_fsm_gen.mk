# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk

MODULE_TARGET    = upgrade.upgfsmgen
MODULE_PIPELINE  = apulu
MODULE_DEPS      = ${TOPDIR}/nic/apollo/upgrade/core/fsm_code_gen.py \
                   ${TOPDIR}/nic/apollo/upgrade/core/upgrade.json
MODULE_ARGS      = -i ${TOPDIR}/nic/apollo/upgrade/core/upgrade.json \
                   -o ${BLD_PROTOGEN_DIR}/upg_fsm.hpp
MODULE_BASECMD   = python2 ${TOPDIR}/nic/apollo/upgrade/core/fsm_code_gen.py

include ${MKDEFS}/post.mk
