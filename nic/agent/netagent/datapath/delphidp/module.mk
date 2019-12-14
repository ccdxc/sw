# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = agent_delphidp.submake
MODULE_PIPELINE = iris
MODULE_DIR      := ${GOPATH}/src/github.com/pensando/sw/nic/${MODULE_DIR}
MODULE_DEPS     := ${NICDIR}/proto/hal/acl.proto          \
                   ${NICDIR}/proto/hal/accel_rgroup.proto \
                   ${NICDIR}/proto/hal/fwlog.proto        \
                   ${NICDIR}/proto/hal/event.proto        \
                   ${NICDIR}/proto/hal/eplearn.proto      \
                   ${NICDIR}/proto/hal/endpoint.proto     \
                   ${NICDIR}/proto/hal/dos.proto          \
                   ${NICDIR}/proto/hal/debug.proto        \
                   ${NICDIR}/proto/hal/interface.proto    \
                   ${NICDIR}/proto/hal/hal.proto          \
                   ${NICDIR}/proto/hal/l2segment.proto    \
                   ${NICDIR}/proto/hal/kh.proto           \
                   ${NICDIR}/proto/hal/internal.proto     \
                   ${NICDIR}/proto/hal/nw.proto           \
                   ${NICDIR}/proto/hal/nic.proto          \
                   ${NICDIR}/proto/hal/nvme.proto         \
                   ${NICDIR}/proto/hal/multicast.proto    \
                   ${NICDIR}/proto/hal/qos.proto          \
                   ${NICDIR}/proto/hal/port.proto         \
                   ${NICDIR}/proto/hal/nwsec.proto        \
                   ${NICDIR}/proto/hal/table.proto        \
                   ${NICDIR}/proto/hal/system.proto       \
                   ${NICDIR}/proto/hal/session.proto      \
                   ${NICDIR}/proto/hal/vrf.proto          \
                   ${NICDIR}/proto/hal/vmotion.proto      \
                   ${NICDIR}/proto/hal/proxy.proto        \
                   ${NICDIR}/proto/hal/tcp_proxy.proto    \
                   ${NICDIR}/proto/hal/types.proto        \
                   ${NICDIR}/proto/hal/telemetry.proto
ifeq ($(ARCH),x86_64)
MODULE_DEPS     := ${MODULE_DEPS}                         \
                   ${NICDIR}/proto/hal/l4lb.proto         \
                   ${NICDIR}/proto/hal/rdma.proto         \
                   ${NICDIR}/proto/hal/cpucb.proto        \
                   ${NICDIR}/proto/hal/ipsec.proto        \
                   ${NICDIR}/proto/hal/nat.proto
endif
ifeq ($(PIPELINE),gft)
MODULE_DEPS     := ${MODULE_DEPS} \
                   ${NICDIR}/proto/hal/gft.proto
endif
include  ${MKDEFS}/post.mk
