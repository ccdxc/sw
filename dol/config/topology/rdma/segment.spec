# Segment Configuration Spec
meta:
    id: SEGMENT_RDMA

type        : tenant
native      : False
broadcast   : flood
multicast   : drop
l4lb        : True
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 2 # Remote TEPs
    rdma:
        pd      : ref://store/specs/id=PD_RDMA
        slab    : ref://store/specs/id=SLAB_RDMA
