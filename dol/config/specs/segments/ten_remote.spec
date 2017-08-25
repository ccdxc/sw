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
    pvlan   : 0
    direct  : 0
    remote  : 2 # Remote TEPs
