# Configuration Spec
uplink:
    - entry:
        id      : Uplink1
        port    : 1
        mode    : PC_MEMBER
        sriov   : True
        status  : UP

    - entry:
        id      : Uplink2
        port    : 2
        mode    : PC_MEMBER
        sriov   : True
        status  : UP

    - entry: 
        id      : Uplink3
        port    : 3
        mode    : TRUNK
        sriov   : False
        status  : UP
        pc      : 1

    - entry:
        id      : Uplink4
        port    : 4
        mode    : TRUNK
        sriov   : False
        status  : UP
        pc      : 1

uplinkpc:
    - entry:
        id      : UplinkPc1
        port    : 1
        mode    : TRUNK
        members :
            - ref://store/objects/id=Uplink1
    - entry:
        id      : UplinkPc2
        port    : 2
        mode    : TRUNK
        members :
            - ref://store/objects/id=Uplink2

tenants:
    -   spec    : ref://store/specs/id=TENANT_SPAN_VLAN
        count   : 1
        lifns   : range/93/96
    #-   spec    : ref://store/specs/id=TENANT_SPAN_VXLAN
    #    count   : 1
    #    lifns   : range/97/100
    -   spec    : ref://store/specs/id=INFRA
        count   : 1
        lifns   : range/101/128
    -   spec    : ref://store/specs/id=TENANT_DEFAULT
        count   : 1
        lifns   : range/129/256
