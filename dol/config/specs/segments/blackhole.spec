# Segment Configuration Spec
meta:
    id: SEGMENT_BLACKHOLE

type        : tenant
native      : False
blackhole   : True
broadcast   : drop
multicast   : drop
l4lb        : False
endpoints   :
    # No endpoints should be added in this segment.
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 0
