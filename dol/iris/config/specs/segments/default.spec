# Segment Configuration Spec
meta:
    id: SEGMENT_DEFAULT

type        : tenant
fabencap    : vlan
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    sgenable: True
    useg    : 2
    pvlan   : 2
    direct  : 0
    remote  : 4 # 1 EP per uplink
    rdma:
        cq      : ref://store/specs/id=CQ_RDMA
        pd      : ref://store/specs/id=PD_RDMA
        slab    : ref://store/specs/id=SLAB_RDMA
