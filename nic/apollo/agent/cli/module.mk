# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = pdsctl.gobin
MODULE_PREREQS  = agent_pdsproto.submake
ifeq ($(SKIPPDSCTL), 1)
MODULE_PIPELINE = apollo artemis athena
else
MODULE_PIPELINE = apollo artemis apulu athena
endif
MODULE_FLAGS    = -ldflags="-s -w"
ifeq ($(PIPELINE), athena)
MODULE_DEPS     = $(shell find ${MODULE_SRC_DIR}/ -name 'api.go' -o -name 'clear.go' -o -name 'command.go' -o -name 'debug.go' -o -name 'interrupt.go' -o -name 'memory.go' -o -name 'port.go' -o -name 'root.go' -o -name 'show.go' -o -name 'system.go') 
else
MODULE_DEPS     = $(shell find ${MODULE_SRC_DIR}/ -name '*.go')
endif
include ${MKDEFS}/post.mk
