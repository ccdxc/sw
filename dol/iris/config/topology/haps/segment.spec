# Segment Configuration Spec
meta:
    id: SEGMENT_HAPS

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 1 # Remote TEPs
