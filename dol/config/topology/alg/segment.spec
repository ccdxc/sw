# Segment Configuration Spec
meta:
    id: SEGMENT_ALG

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 4
    direct  : 0
    remote  : 8
