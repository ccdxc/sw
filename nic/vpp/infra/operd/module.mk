# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET    = libpdsvpp_operd.lib
MODULE_PIPELINE  = apollo artemis apulu
MODULE_SOLIBS    = operd
MODULE_LDLIBS    = rt dl pthread
include ${MKDEFS}/post.mk
