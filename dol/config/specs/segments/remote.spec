# Segment Configuration Spec
meta:
    id: SEGMENT_REMOTE

type        : tenant
native      : False
broadcast   : drop
multicast   : drop
l4lb        : True
endpoints   :
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 1 # Remote TEPs
