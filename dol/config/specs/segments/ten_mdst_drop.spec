# Segment Configuration Spec
meta:
    id: SEGMENT_MDST_DROP

type        : tenant
native      : False
broadcast   : drop
multicast   : drop
l4lb        : True
endpoints   :
    useg    : 2
    pvlan   : 2
    direct  : 0
    remote  : 4 # 1 EP per uplink
