# Segment Configuration Spec
meta:
    id: BASE_SEGMENT_VXLAN

type        : tenant
fabencap    : vxlan
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 2 # 1 EP per uplink
    rdma:
        pd      : ref://store/specs/id=PD_RDMA
        slab    : ref://store/specs/id=SLAB_RDMA
