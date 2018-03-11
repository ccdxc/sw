# Configuration Template.
meta:
    id: ICACHE_TENANT_SPAN_VLAN

type    : span
overlay : vlan

segments:
    - spec  : ref://store/specs/id=ICACHE_SEGMENT_SPAN
      count : 1

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : False
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : False
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : False

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

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

lif: ref://store/specs/id=LIF_ETH_RDMA
