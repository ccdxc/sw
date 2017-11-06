# Segment Configuration Spec
meta:
    id: SEGMENT_L4LB

type        : tenant
native      : False
broadcast   : drop
multicast   : drop
l4lb        : True
endpoints   :
    useg    : 0
    pvlan   : 4
    direct  : 0
    remote  : 4 # Remote TEPs
