# Configuration Template.
meta:
    id: TENANT_MULTICAST_VXLAN

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_MULTICAST_VXLAN
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
        ipv6: None
        mac: None

lif: ref://store/specs/id=LIF_ETH
