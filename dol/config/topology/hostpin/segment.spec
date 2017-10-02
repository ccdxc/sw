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
    pvlan   : 16
    direct  : 0
    remote  : 16 # Remote TEPs
