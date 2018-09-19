# Segment Configuration Spec
meta:
    id: VXLANTOPO_SEGMENT_VXLAN

type        : tenant
fabencap    : vxlan
native      : False
broadcast   : flood
multicast   : replicate
l4lb        : False
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 2
