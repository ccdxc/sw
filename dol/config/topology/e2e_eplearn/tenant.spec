# Configuration Template.
meta:
    id: TENANT_EPLEARN

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_EPLEARN
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

fte:
    ep_learn:
        ipv4     : True
        ipv6     : False

sessions:

lif: ref://store/specs/id=LIF_ETH
