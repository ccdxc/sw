# Configuration Template.
meta:
    id: TENANT_PARSER

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_PARSER
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_PARSER
            - ref://store/specs/id=SESSION_TCP_PARSER
            - ref://store/specs/id=SESSION_PR255_PARSER
        ipv6:
            - ref://store/specs/id=SESSION_UDP_PARSER
            - ref://store/specs/id=SESSION_TCP_PARSER
            - ref://store/specs/id=SESSION_PR255_PARSER
        mac:
            - ref://store/specs/id=SESSION_MAC_PARSER

lif: ref://store/specs/id=LIF_ETH
