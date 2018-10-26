# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = p4cli.submake
#MODULE_PIPELINE = iris
MODULE_POSTGEN_MK = module_clicommon_swig.mk 
include ${MKDEFS}/post.mk
