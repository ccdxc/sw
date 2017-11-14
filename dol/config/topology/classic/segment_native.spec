# Segment Configuration Spec
meta:
    id  : SEGMENT_CLASSIC_NATIVE

type        : tenant
fabencap    : vlan
native      : True
broadcast   : flood
multicast   : flood
l4lb        : False
groups      : ref://store/specs/id=MULTICAST_GROUPS_CLASSIC
endpoints   :
    classic : 2
    remote  : 8
