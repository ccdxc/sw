# Segment Configuration Spec
meta:
    id: SEGMENT_HOSTPIN

type        : tenant
native      : False
broadcast   : drop
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 2 # Remote TEPs
