# Segment Configuration Spec
meta:
    id: ICACHE_SEGMENT_INFRA

type        : infra
fabencap    : vlan
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 2 # Remote TEPs
