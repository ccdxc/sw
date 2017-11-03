# Segment Configuration Spec
meta:
    id: SEGMENT_FTE

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 8
    direct  : 0
    remote  : 2
