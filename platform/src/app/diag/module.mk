# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = diag_test
MODULE_SOLIBS   = diag sdkpal logger sensor sdkcapri_csrint
MODULE_LDLIBS   := dl sknobs Judy
MODULE_FLAGS    = -DCAPRI_SW

include ${MKDEFS}/post.mk
