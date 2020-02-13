# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = hal.svcgen
MODULE_PIPELINE     = iris gft
MODULE_PREREQS      = hal.proto
MODULE_DEPS         := ${MODULE_SRC_DIR}/acl.proto          \
                       ${MODULE_SRC_DIR}/accel_rgroup.proto \
                       ${MODULE_SRC_DIR}/fwlog.proto        \
                       ${MODULE_SRC_DIR}/event.proto        \
                       ${MODULE_SRC_DIR}/eplearn.proto      \
                       ${MODULE_SRC_DIR}/endpoint.proto     \
                       ${MODULE_SRC_DIR}/dos.proto          \
                       ${MODULE_SRC_DIR}/debug.proto        \
                       ${MODULE_SRC_DIR}/interface.proto    \
                       ${MODULE_SRC_DIR}/l2segment.proto    \
                       ${MODULE_SRC_DIR}/kh.proto           \
                       ${MODULE_SRC_DIR}/internal.proto     \
                       ${MODULE_SRC_DIR}/nw.proto           \
                       ${MODULE_SRC_DIR}/nic.proto          \
                       ${MODULE_SRC_DIR}/nvme.proto         \
                       ${MODULE_SRC_DIR}/multicast.proto    \
                       ${MODULE_SRC_DIR}/qos.proto          \
                       ${MODULE_SRC_DIR}/port.proto         \
                       ${MODULE_SRC_DIR}/nwsec.proto        \
                       ${MODULE_SRC_DIR}/table.proto        \
                       ${MODULE_SRC_DIR}/system.proto       \
                       ${MODULE_SRC_DIR}/session.proto      \
                       ${MODULE_SRC_DIR}/vrf.proto          \
                       ${MODULE_SRC_DIR}/vmotion.proto      \
                       ${MODULE_SRC_DIR}/proxy.proto        \
                       ${MODULE_SRC_DIR}/tcp_proxy.proto    \
                       ${MODULE_SRC_DIR}/types.proto        \
                       ${MODULE_SRC_DIR}/telemetry.proto    \
                       ${MODULE_SRC_DIR}/ncsi.proto         \
                       $(wildcard ${TOPDIR}/nic/tools/hal/*)
ifeq ($(PIPELINE),gft)
MODULE_DEPS         := ${MODULE_DEPS}                       \
                       ${MODULE_SRC_DIR}/gft.proto
endif
ifeq ($(ARCH),x86_64)
MODULE_DEPS         := ${MODULE_DEPS}                       \
                       ${MODULE_SRC_DIR}/rdma.proto         \
                       ${MODULE_SRC_DIR}/cpucb.proto        \
                       ${MODULE_SRC_DIR}/l4lb.proto         \
                       ${MODULE_SRC_DIR}/ipsec.proto        \
                       ${MODULE_SRC_DIR}/nat.proto
endif
MODULE_POSTGEN_MK   = module_libsvcgen.mk
include ${MKDEFS}/post.mk
