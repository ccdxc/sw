# Configuration Template.
meta:
    id: TENANT_UP2UP

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_UP2UP
      count : 2
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_UP2UP
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_UP2UP
            - ref://store/specs/id=SESSION_TCP_UP2UP
            - ref://store/specs/id=SESSION_ICMP_UP2UP
        ipv6:
            - ref://store/specs/id=SESSION_UDP_UP2UP
            - ref://store/specs/id=SESSION_TCP_UP2UP
        mac:
            - ref://store/specs/id=SESSION_MAC_UP2UP

lif: ref://store/specs/id=LIF_ETH_RDMA
