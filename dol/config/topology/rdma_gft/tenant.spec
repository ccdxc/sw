# Configuration Template.
meta:
    id: TENANT_RDMA_GFT

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_RDMA_GFT
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_RDMA_GFT
        ipv6:
            - ref://store/specs/id=SESSION_UDP_RDMA_GFT
        mac: None

lif: ref://store/specs/id=LIF_ETH_RDMA
