# Segment Configuration Spec
meta:
    id: SEGMENT_CACHEPERF

type        : tenant
native      : False
broadcast   : drop
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 32 # Remote TEPs
