# Segment Configuration Spec
meta:
    id: SEGMENT_VXLAN_PROXY

type        : tenant
native      : False
fabencap    : vxlan
broadcast   : drop
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 2 # Remote TEPs
