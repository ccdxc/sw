# Segment Configuration Spec
meta:
    id: SEGMENT_ETH_VXLAN

type        : tenant
native      : False
fabencap    : vxlan
broadcast   : drop
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 4 # Remote TEPs
