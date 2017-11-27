# Segment Configuration Spec
meta:
    id: SEGMENT_CLASSIC_ENIC_NATIVE

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
groups      : ref://store/specs/id=MULTICAST_GROUPS_CLASSIC
endpoints   :
    enics   : False
    access  : True
    classic : 2
    remote  : 1
