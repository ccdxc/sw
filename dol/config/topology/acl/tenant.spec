# Configuration Template.
meta:
    id: TENANT_ACL

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_ACL
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_ACL
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_ACL
            - ref://store/specs/id=SESSION_TCP_ACL
            - ref://store/specs/id=SESSION_ICMP_ACL
        ipv6:
            - ref://store/specs/id=SESSION_UDP_ACL
            - ref://store/specs/id=SESSION_TCP_ACL
        mac:
            - ref://store/specs/id=SESSION_MAC_ACL

lif: ref://store/specs/id=LIF_ETH_RDMA
