# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = capri_tm_rw_gen.tenjin
MODULE_BASECMD      = python2
MODULE_GENERATOR    = ${TOPDIR}/nic/tools/ncc/tenjin_wrapper.py
MODULE_TEMPLATE     = ${MODULE_SRC_DIR}/capri_tm_rw.cc.template
MODULE_OUTFILE      = ${MODULE_SRC_DIR}/capri_tm_rw.cc
MODULE_ARGS         = ${TOPDIR}/nic/asic/capri/verif/common/csr_gen/pbc.yaml
MODULE_POSTGEN_MK   = module.mk
MODULE_CLEAN_DIRS   = ${MODULE_SRC_DIR}/capri_tm_rw.cc
include ${MKDEFS}/post.mk
