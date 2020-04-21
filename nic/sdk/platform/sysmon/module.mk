# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsysmon.lib
MODULE_SRCS     = ${MODULE_SRC_DIR}/disk.cc  \
		  ${MODULE_SRC_DIR}/fwversion.cc  \
		  ${MODULE_SRC_DIR}/ipc.cc  \
		  ${MODULE_SRC_DIR}/memory.cc  \
		  ${MODULE_SRC_DIR}/panic.cc  \
		  ${MODULE_SRC_DIR}/pciehealth.cc  \
		  ${MODULE_SRC_DIR}/postdiag.cc  \
		  ${MODULE_SRC_DIR}/power.cc  \
		  ${MODULE_SRC_DIR}/runtime.cc  \
		  ${MODULE_SRC_DIR}/sysmon.cc  \
		  ${MODULE_SRC_DIR}/temperature.cc
ifeq ($(ASIC),elba)
MODULE_SRCS     += ${MODULE_SRC_DIR}/sysmon_monitor_elba.cc  
else
MODULE_SRCS     += ${MODULE_SRC_DIR}/sysmon_monitor.cc  
endif
MODULE_FLAGS    = -DCAPRI_SW ${NIC_CSR_FLAGS} -O2
include ${MKDEFS}/post.mk
