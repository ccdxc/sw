# Configuration Template.
meta:
    id: TENANT_TELEMETRY

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_TELEMETRY
      count : 2
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_TELEMETRY
      count : 1

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : False 
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : False 
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : False 
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : True
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : True
    - spec  : ref://store/specs/id=TUNNEL_GRE
      local : True

collectors:
    - spec  : ref://store/specs/id=COLLECTOR_IPFIX

span_sessions:
    - spec  : ref://store/specs/id=SPAN_TYPE_LOCAL
    - spec  : ref://store/specs/id=SPAN_TYPE_RSPAN
    - spec  : ref://store/specs/id=SPAN_TYPE_RSPAN
    - spec  : ref://store/specs/id=SPAN_TYPE_LOCAL
    - spec  : ref://store/specs/id=SPAN_TYPE_RSPAN
    - spec  : ref://store/specs/id=SPAN_TYPE_RSPAN


# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 4

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_TCP_TELEMETRY
            #- ref://store/specs/id=SESSION_UDP_TELEMETRY
            #- ref://store/specs/id=SESSION_ICMP_TELEMETRY
            - ref://store/specs/id=SESSION_IPFIX
        ipv6:
        mac:

lif: ref://store/specs/id=LIF_ETH
