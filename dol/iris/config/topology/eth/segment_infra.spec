# Segment Configuration Spec
meta:
    id: SEGMENT_INFRA_ETH

type        : infra
native      : False
fabencap    : vlan
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 4 # Remote TEPs
