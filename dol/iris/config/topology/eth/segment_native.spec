# Segment Configuration Spec
meta:
    id  : SEGMENT_NATIVE_ETH

type        : tenant
fabencap    : vlan
native      : True
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 4 # 1 EP per uplink
