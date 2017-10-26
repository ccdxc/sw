# Segment Configuration Spec
meta:
    id  : SEGMENT_CLASSIC_NATIVE

type        : tenant
fabencap    : vlan
native      : True
broadcast   : flood
multicast   : replicate
l4lb        : False
endpoints   :
    classic : 2
    remote  : 1
