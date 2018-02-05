# Segment Configuration Spec
meta:
    id: SEGMENT_SPAN_ETH

type        : span
fabencap    : vlan
native      : False
broadcast   : flood
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 4
    direct  : 0
    remote  : 6
