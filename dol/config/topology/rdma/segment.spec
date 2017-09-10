# Segment Configuration Spec
meta:
    id: SEGMENT_RDMA

type        : tenant
native      : False
broadcast   : drop
multicast   : drop
l4lb        : True
endpoints   :
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 1 # Remote TEPs
    rdma:
        pd      : ref://store/specs/id=PD_RDMA
        slab    : ref://store/specs/id=SLAB_RDMA
