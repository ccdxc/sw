# Segment Configuration Spec
meta:
    id: VXLANTOPO_SEGMENT_DEFAULT

type        : tenant
fabencap    : vlan
native      : False
broadcast   : flood
multicast   : replicate
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 1
