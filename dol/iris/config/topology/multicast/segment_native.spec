# Segment Configuration Spec
meta:
    id: SEGMENT_MULTICAST_NATIVE

type        : tenant
native      : True
fabencap    : vlan
broadcast   : flood
multicast   : drop
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 2
    direct  : 0
    remote  : 5 # Remote TEPs
