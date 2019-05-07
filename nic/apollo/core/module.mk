# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdscore.so
MODULE_PIPELINE = apollo
MODULE_SOLIBS   = pdsnicmgr pdspciemgr pciemgrd pcieport
include ${MKDEFS}/post.mk
