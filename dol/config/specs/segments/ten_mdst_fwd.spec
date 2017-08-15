# Segment Configuration Spec
meta:
    id: SEGMENT_MDST_FWD

type        : tenant
native      : False
broadcast   : flood
multicast   : replicate
endpoints   :
    useg    : 2
    pvlan   : 2
    direct  : 0
    remote  : 4 # 1 EP per uplink
