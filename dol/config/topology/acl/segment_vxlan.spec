# Segment Configuration Spec
meta:
    id: SEGMENT_VXLAN_ACL

type        : tenant
fabencap    : vxlan
native      : False
broadcast   : flood
multicast   : replicate
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 4 # 1 EP per uplink
