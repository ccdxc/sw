# Segment Configuration Spec
meta:
    id: SEGMENT_ADMIN

type        : tenant
native      : False
broadcast   : drop
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 1 # Remote TEPs
