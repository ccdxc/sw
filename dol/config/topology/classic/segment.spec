# Segment Configuration Spec
meta:
    id: SEGMENT_CLASSIC_DEFAULT

type        : tenant
native      : False
broadcast   : flood
multicast   : flood
l4lb        : False
groups      : ref://store/specs/id=MULTICAST_GROUPS_CLASSIC
endpoints   :
    enics   : False
    classic : 2
    remote  : 1
