# Segment Configuration Spec
meta:
    id: ICACHE_SEGMENT_DEFAULT

type        : tenant
fabencap    : vlan
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 2
    rdma:
        cq      : ref://store/specs/id=CQ_RDMA
        pd      : ref://store/specs/id=PD_RDMA
        slab    : ref://store/specs/id=SLAB_RDMA
