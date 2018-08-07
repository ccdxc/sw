# Segment Configuration Spec
meta:
    id: SEGMENT_MULTIDEST_DROP

type        : tenant
fabencap    : vlan
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 2
    pvlan   : 2
    direct  : 0
    remote  : 4 # 1 EP per uplink
