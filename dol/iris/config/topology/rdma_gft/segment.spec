# Segment Configuration Spec
meta:
    id: SEGMENT_RDMA_GFT

type        : tenant
native      : False
broadcast   : flood
multicast   : drop
l4lb        : True
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 2
    remote  : 4 # Remote TEPs
    rdma:
        pd      : ref://store/specs/id=PD_RDMA
        slab    : ref://store/specs/id=SLAB_RDMA
