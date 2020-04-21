# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := libsensor.lib
ifeq ($(ASIC),elba)
MODULE_SRCS     := ${MODULE_SRC_DIR}/sensor_elba.cc 
else
MODULE_SRCS     := ${MODULE_SRC_DIR}/sensor.cc 
endif

include ${MKDEFS}/post.mk
