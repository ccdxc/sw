# Segment Configuration Spec
meta:
    id: SEGMENT_NATIVE_APP_REDIR

type        : tenant
native      : True
broadcast   : flood
multicast   : flood
l4lb        : True
endpoints   :
    sgenable: True
    useg    : 0
    pvlan   : 1
    direct  : 0
    remote  : 2 # 1 EP per uplink
