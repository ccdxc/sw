# Configuration Template.
meta:
    id: TENANT_HAPS

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_HAPS
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_HAPS
      count : 1
   
# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: None

sessions:
    unidest:
        ipv4:
        ipv6:
        mac:


lif: ref://store/specs/id=LIF_ETH
