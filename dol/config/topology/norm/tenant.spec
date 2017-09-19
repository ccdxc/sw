# Configuration Template.
meta:
    id: TENANT_NORM

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_NORM
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_NORM
            - ref://store/specs/id=SESSION_TCP_NORM
            - ref://store/specs/id=SESSION_ICMP_NORM
        ipv6:
            - ref://store/specs/id=SESSION_UDP_NORM
            - ref://store/specs/id=SESSION_TCP_NORM
            - ref://store/specs/id=SESSION_ICMPV6_NORM
        mac:

lif: ref://store/specs/id=LIF_ETH_RDMA
