# Segment Configuration Spec
meta:
    id: SEGMENT_REMOTE

type        : tenant
broadcast   : drop
multicast   : drop
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 2 # Remote TEPs
