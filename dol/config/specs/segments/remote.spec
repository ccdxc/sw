# Segment Configuration Spec
meta:
    id: SEGMENT_REMOTE

type        : tenant
fabencap    : vlan
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
endpoints   :
    useg    : 0
    pvlan   : 0
    direct  : 0
    remote  : 1 # Remote TEPs
