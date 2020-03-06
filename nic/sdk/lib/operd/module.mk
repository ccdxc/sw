# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = liboperd.lib
MODULE_PIPELINE = iris apollo apulu artemis gft athena
MODULE_SOLIBS =
MODULE_LDLIBS =
MODULE_FLAGS  = -Werror
include ${MKDEFS}/post.mk
