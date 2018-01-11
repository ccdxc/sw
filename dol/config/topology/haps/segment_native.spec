# Segment Configuration Spec
meta:
    id  : SEGMENT_NATIVE_HAPS

type        : tenant
fabencap    : vlan
native      : True
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 1 # 1 EP per uplink
