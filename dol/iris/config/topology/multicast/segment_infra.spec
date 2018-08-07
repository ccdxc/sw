# Segment Configuration Spec
meta:
    id: SEGMENT_MULTICAST_INFRA

type        : infra
native      : False
fabencap    : vlan
broadcast   : flood
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 5 # Remote TEPs
