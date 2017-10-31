# Segment Configuration Spec
meta:
    id: SEGMENT_VXLAN_FTE

type        : tenant
native      : False
fabencap    : vxlan
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 2 # Remote TEPs
