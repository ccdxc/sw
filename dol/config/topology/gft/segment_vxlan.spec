# Segment Configuration Spec
meta:
    id: SEGMENT_GFT_VXLAN

type        : tenant
fabencap    : vxlan
native      : False
broadcast   : drop
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 4 # Remote TEPs
