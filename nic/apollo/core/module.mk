# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdscore.so
MODULE_PIPELINE = apollo artemis apulu poseidon
ifneq ($(PIPELINE),poseidon)
MODULE_SOLIBS   = pdsnicmgr pdspciemgr pciemgrd pcieport event_thread pdslearn
else
MODULE_SOLIBS   = pdsnicmgr pdspciemgr pciemgrd pcieport event_thread
endif
include ${MKDEFS}/post.mk
