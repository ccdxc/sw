# Configuration Template.
meta:
    id: TENANT_NVME

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_NVME
      count : 1

nfveps: 0

security_profile: None
security_policy: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_NVME

lif: ref://store/specs/id=LIF_NVME
