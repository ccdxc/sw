# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = netproto_delphidp.submake
MODULE_PIPELINE = iris
MODULE_DEPS     := ${NICDIR}/agent/protos/netproto/app.proto            \
                   ${NICDIR}/agent/protos/netproto/agg_watch.proto      \
                   ${NICDIR}/agent/protos/netproto/endpoint.proto       \
                   ${NICDIR}/agent/protos/netproto/flowexport.proto     \
                   ${NICDIR}/agent/protos/netproto/interface.proto      \
                   ${NICDIR}/agent/protos/netproto/ipam.proto           \
                   ${NICDIR}/agent/protos/netproto/match.proto          \
                   ${NICDIR}/agent/protos/netproto/mirror.proto         \
                   ${NICDIR}/agent/protos/netproto/namespace.proto      \
                   ${NICDIR}/agent/protos/netproto/network.proto        \
                   ${NICDIR}/agent/protos/netproto/port.proto           \
                   ${NICDIR}/agent/protos/netproto/profile.proto        \
                   ${NICDIR}/agent/protos/netproto/route.proto          \
                   ${NICDIR}/agent/protos/netproto/sgpolicy.proto       \
                   ${NICDIR}/agent/protos/netproto/secprofile.proto     \
                   ${NICDIR}/agent/protos/netproto/tenant.proto         \
                   ${NICDIR}/agent/protos/netproto/tunnel.proto         \
                   ${NICDIR}/agent/protos/netproto/types.proto          \
                   ${NICDIR}/agent/protos/netproto/vrf.proto
include ${MKDEFS}/post.mk
