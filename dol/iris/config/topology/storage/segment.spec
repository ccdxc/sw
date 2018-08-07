# Segment Configuration Spec
meta:
    id: SEGMENT_STORAGE

type        : tenant
native      : False
broadcast   : drop
multicast   : drop
l4lb        : True
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 0 # Remote TEPs
