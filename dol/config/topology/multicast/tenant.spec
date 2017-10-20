# Configuration Template.
meta:
    id: TENANT_MULTICAST

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_MULTICAST
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=MULTICAST_TOPO_SESSION_DUMMY
        ipv6:
        mac:

lif: ref://store/specs/id=LIF_ETH
