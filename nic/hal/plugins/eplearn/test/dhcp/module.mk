# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = dhcp_test.gtest
MODULE_ARCH     = x86_64
MODULE_PIPELINE = iris gft
MODULE_SOLIBS   = ${NIC_HAL_GTEST_SOLIBS} eplearn_hal_mock
MODULE_LDLIBS   = tins ${NIC_HAL_GTEST_LDLIBS}
MODULE_INCS     = ${TOPDIR}/nic/hal/third-party/isc/include
include ${MKDEFS}/post.mk
