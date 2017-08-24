# Configuration Spec
uplink:
    - entry:
        id      : Uplink1
        port    : 1
        mode    : TRUNK
        sriov   : True
        status  : UP
    - entry:
        id      : Uplink2
        port    : 2
        mode    : TRUNK
        sriov   : True
        status  : UP

uplinkpc: None
acls: None
tenants:
    -   spec    : ref://store/specs/id=TENANT_FP_TCP
        count   : 1
        lifns   : range/101/128
