# Segment Configuration Spec
meta:
    id: SEGMENT_VXLAN_L4LB

type        : tenant
fabencap    : vxlan
native      : False
broadcast   : drop
multicast   : drop
l4lb        : True
endpoints   :
    useg    : 0
    pvlan   : 4
    direct  : 0
    remote  : 4 # 1 EP per uplink
