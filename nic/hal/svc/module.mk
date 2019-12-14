# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhal_svc.so
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = hal.svcgen
ifeq ($(ARCH),x86_64)
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc)
else
MODULE_SRCS     = ${MODULE_SRC_DIR}/event_svc.cc \
				  ${MODULE_SRC_DIR}/debug_svc.cc \
				  ${MODULE_SRC_DIR}/accel_rgroup_svc.cc \
				  ${MODULE_SRC_DIR}/table_svc.cc \
				  ${MODULE_SRC_DIR}/proxy_svc.cc \
				  ${MODULE_SRC_DIR}/system_svc.cc \
				  ${MODULE_SRC_DIR}/nic_svc.cc \
				  ${MODULE_SRC_DIR}/interface_svc.cc
endif
include ${MKDEFS}/post.mk
