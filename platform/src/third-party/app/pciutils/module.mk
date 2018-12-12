# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     :=
MODULE_DEPS     := libpciemgr.so libpciemgrutils.so libpcietlp.so \
		   libcfgspace.so libpal.so libmisc.so libintrutils.so
MODULE_TARGET   := pciutils.submake
include ${MKDEFS}/post.mk
