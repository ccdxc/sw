# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := penctl.submake
MODULE_PREREQS	= agent_irisproto.submake
MODULE_PIPELINE := iris
MODULE_PREREQS	= agent_irisproto.submake agent_halproto.submake
include ${MKDEFS}/post.mk
