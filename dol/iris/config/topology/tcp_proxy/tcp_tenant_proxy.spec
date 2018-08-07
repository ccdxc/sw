# Configuration Template.
meta:
    id: TCP_TENANT_PROXY

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_PROXY
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_PROXY
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None
security_policy: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_TCP_PROXY

lif: ref://store/specs/id=LIF_ETH
