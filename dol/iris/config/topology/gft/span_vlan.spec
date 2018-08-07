# Configuration Template.
meta:
    id: GFT_SPAN_VLAN

type    : span
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_SPAN_GFT
      count : 1

security_profile: ref://store/objects/id=SEC_PROF_DEFAULT

nfveps: 4

collectors:
    - spec  : ref://store/specs/id=COLLECTOR_IPFIX

span_sessions:
    - spec  : ref://store/specs/id=SPAN_TYPE_ERSPAN

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : False 

sessions: None

lif: ref://store/specs/id=LIF_ETH
