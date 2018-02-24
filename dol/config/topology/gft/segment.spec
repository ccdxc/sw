# Segment Configuration Spec
meta:
    id: SEGMENT_GFT

type        : tenant
native      : False
broadcast   : drop
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 4 # Remote TEPs
