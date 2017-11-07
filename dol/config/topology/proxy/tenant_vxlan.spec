# Configuration Template.
meta:
    id: TENANT_VXLAN_PROXY

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_VXLAN_PROXY
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv6:
            - ref://store/specs/id=SESSION_TCP_PROXY
        ipv4:
            - ref://store/specs/id=SESSION_TCP_PROXY
        mac:
            - ref://store/specs/id=SESSION_MAC_PROXY

lif: ref://store/specs/id=LIF_ETH
