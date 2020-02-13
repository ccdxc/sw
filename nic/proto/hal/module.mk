# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = hal.proto
MODULE_PIPELINE     = iris gft
MODULE_FWTYPE       = full gold
MODULE_PREREQS      = gogo.proto
MODULE_GEN_TYPES    = CC PY DELPHI
MODULE_SRCS         := ${MODULE_DIR}/acl.proto          \
                       ${MODULE_DIR}/accel_rgroup.proto \
                       ${MODULE_DIR}/fwlog.proto        \
                       ${MODULE_DIR}/event.proto        \
                       ${MODULE_DIR}/eplearn.proto      \
                       ${MODULE_DIR}/endpoint.proto     \
                       ${MODULE_DIR}/dos.proto          \
                       ${MODULE_DIR}/debug.proto        \
                       ${MODULE_DIR}/interface.proto    \
                       ${MODULE_DIR}/l2segment.proto    \
                       ${MODULE_DIR}/kh.proto           \
                       ${MODULE_DIR}/internal.proto     \
                       ${MODULE_DIR}/nw.proto           \
                       ${MODULE_DIR}/nic.proto          \
                       ${MODULE_DIR}/nvme.proto         \
                       ${MODULE_DIR}/multicast.proto    \
                       ${MODULE_DIR}/qos.proto          \
                       ${MODULE_DIR}/port.proto         \
                       ${MODULE_DIR}/nwsec.proto        \
                       ${MODULE_DIR}/table.proto        \
                       ${MODULE_DIR}/system.proto       \
                       ${MODULE_DIR}/session.proto      \
                       ${MODULE_DIR}/vrf.proto          \
                       ${MODULE_DIR}/vmotion.proto      \
                       ${MODULE_DIR}/proxy.proto        \
                       ${MODULE_DIR}/types.proto        \
                       ${MODULE_DIR}/tcp_proxy.proto    \
                       ${MODULE_DIR}/telemetry.proto    \
                       ${MODULE_DIR}/ncsi.proto
ifeq ($(ARCH),x86_64)
MODULE_SRCS         := ${MODULE_SRCS}                   \
                       ${MODULE_DIR}/cpucb.proto        \
                       ${MODULE_DIR}/rdma.proto         \
                       ${MODULE_DIR}/l4lb.proto         \
                       ${MODULE_DIR}/ipsec.proto        \
                       ${MODULE_DIR}/nat.proto
endif
ifeq ($(PIPELINE),gft)
MODULE_SRCS         := ${MODULE_SRCS} \
                       ${MODULE_DIR}/gft.proto
endif
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/nic \
                      ${TOPDIR}/vendor/github.com/gogo/protobuf/gogoproto \
                      ${TOPDIR}/nic/hal/third-party/google/include \
                      ${TOPDIR}/nic/delphi/proto/delphi \
                      /usr/local/include
MODULE_LDLIBS       = pthread
MODULE_POSTGEN_MK   = module_protolib.mk \
                      module_svcgen.mk
MODULE_PREREQS = protoc-gen-delphi.gobin

include ${MKDEFS}/post.mk
