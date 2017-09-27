# Configuration Template.
meta:
    id: TENANT_PROXY

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

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

sessions:
    unidest:
        scale: 1
        ipv6:
            - ref://store/specs/id=SESSION_TCP_PROXY
        ipv4:
            - ref://store/specs/id=SESSION_TCP_PROXY
        mac: None

lif: ref://store/specs/id=LIF_ETH
