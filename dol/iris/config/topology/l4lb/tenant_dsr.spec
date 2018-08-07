# Configuration Template.
meta:
    id: TENANT_L4LB_DSR

type    : tenant
overlay : vxlan

segments:
    # Native L2seg
    - spec  : ref://store/specs/id=SEGMENT_L4LB
      count : 2
    # Vxlan L2seg
    - spec  : ref://store/specs/id=SEGMENT_VXLAN_L4LB
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

l4lb: ref://store/specs/id=L4LB_DSR

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
