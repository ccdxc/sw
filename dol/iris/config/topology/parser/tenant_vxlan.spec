# Configuration Template.
meta:
    id: PARSER_TENANT_OVERLAY_VXLAN

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=PARSER_SEGMENT_VXLAN
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 4

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

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

lif: ref://store/specs/id=LIF_ETH
