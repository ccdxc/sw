# Segment Configuration Spec
meta:
    id: SEGMENT_MCAST

type        : tenant
native      : False
broadcast   : flood
multicast   : drop
l4lb        : True
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 1 # Remote TEPs
    remote  : 2 # Remote TEPs
    remote  : 3 # Remote TEPs
