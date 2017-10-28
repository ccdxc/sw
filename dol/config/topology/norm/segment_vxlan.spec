# Segment Configuration Spec
meta:
    id: NORM_SEGMENT_VXLAN

type        : tenant
fabencap    : vxlan
native      : False
broadcast   : flood
multicast   : replicate
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 4 # 1 EP per uplink
