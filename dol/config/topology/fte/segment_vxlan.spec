# Segment Configuration Spec
meta:
    id: SEGMENT_VXLAN_FTE

type        : tenant
native      : False
fabencap    : vxlan
broadcast   : flood
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 4
    direct  : 0
    remote  : 8 # Remote TEPs
