# Configuration Template.
meta:
    id: TELEMETRY_SPAN_VLAN

type    : span
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_SPAN_TELEMETRY
      count : 1

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : False
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : False
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : False

security_profile: ref://store/objects/id=SEC_PROF_DEFAULT

collectors:
    - spec  : ref://store/specs/id=COLLECTOR_IPFIX

span_sessions:
    - spec  : ref://store/specs/id=SPAN_TYPE_LOCAL
    - spec  : ref://store/specs/id=SPAN_TYPE_RSPAN
    - spec  : ref://store/specs/id=SPAN_TYPE_RSPAN
    - spec  : ref://store/specs/id=SPAN_TYPE_LOCAL
    - spec  : ref://store/specs/id=SPAN_TYPE_RSPAN
    - spec  : ref://store/specs/id=SPAN_TYPE_RSPAN

sessions: None

lif: ref://store/specs/id=LIF_ETH
