# Segment Configuration Spec
meta:
    id: SEGMENT_PROXY

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : True
endpoints   :
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 2 # Remote TEPs
