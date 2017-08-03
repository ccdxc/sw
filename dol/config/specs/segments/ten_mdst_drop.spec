# Segment Configuration Spec
meta:
    id: SEGMENT_MDST_DROP

type        : tenant
broadcast   : drop
multicast   : drop
endpoints   :
    useg    : 2
    pvlan   : 2
    direct  : 0
    remote  : 4 # 1 EP per uplink
