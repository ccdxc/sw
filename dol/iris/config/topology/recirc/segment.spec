# Segment Configuration Spec
meta:
    id: SEGMENT_RECIRC

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 2 
    direct  : 0
    remote  : 3 # Remote TEPs
