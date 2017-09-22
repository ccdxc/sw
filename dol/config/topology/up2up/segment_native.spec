# Segment Configuration Spec
meta:
    id  : SEGMENT_NATIVE_UP2UP

type        : tenant
fabencap    : vlan
native      : True
broadcast   : flood
multicast   : replicate
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 8 # 1 EP per uplink
