# Segment Configuration Spec
meta:
    id: SEGMENT_INFRA

type        : infra
native      : False
broadcast   : drop
multicast   : drop
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 4 # Remote TEPs
