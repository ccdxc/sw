# Configuration Template.
meta:
    id: TENANT_L4LB

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_L4LB
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

l4lb: ref://store/specs/id=L4LB

sessions:
    l4lb:
        ipv4:
            - ref://store/specs/id=SESSION_L4LB_TCP
            - ref://store/specs/id=SESSION_L4LB_UDP

        ipv6:
            - ref://store/specs/id=SESSION_L4LB_TCP
            - ref://store/specs/id=SESSION_L4LB_UDP

    unidest:
        ipv4:
        ipv6:
        mac:

lif: ref://store/specs/id=LIF_ETH
