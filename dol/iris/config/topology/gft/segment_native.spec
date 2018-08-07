# Segment Configuration Spec
meta:
    id  : SEGMENT_NATIVE_GFT

type        : tenant
fabencap    : vlan
native      : True
broadcast   : flood
multicast   : replicate
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 2
