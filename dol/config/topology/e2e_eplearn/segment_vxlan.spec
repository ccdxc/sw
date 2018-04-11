# Segment Configuration Spec
meta:
    id: SEGMENT_VXLAN_EPLEARN

type        : tenant
native      : False
fabencap    : vxlan
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 4
    direct  : 0
    remote  : 8 # Remote TEPs
