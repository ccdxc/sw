# Segment Configuration Spec
meta:
    id: SEGMENT_FTE

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
networks    :
    sgenable: False
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 10
    direct  : 0
    remote  : 2
