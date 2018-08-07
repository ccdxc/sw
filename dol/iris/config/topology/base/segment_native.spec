# Segment Configuration Spec
meta:
    id  : BASE_SEGMENT_NATIVE

type        : tenant
fabencap    : vlan
native      : True
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 2 # 1 EP per uplink
