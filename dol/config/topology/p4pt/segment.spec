# Segment Configuration Spec
meta:
    id: SEGMENT_P4PT

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 2 # 1 EP per uplink
