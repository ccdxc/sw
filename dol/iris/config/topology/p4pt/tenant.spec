# Configuration Template.
meta:
    id: TENANT_P4PT

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_P4PT
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE
security_policy: ref://store/specs/id=SECURITY_POLICY_P4PT

sessions:
    unidest:
        ipv6:
        ipv4:
            - ref://store/specs/id=SESSION_TCP_P4PT
        mac:

lif: ref://store/specs/id=LIF_ETH
